/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic imei at Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e76b4
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <aic_math.h>
#include <aic_convert.h>
#include <subsys_api.h>
#include <aic_at_interact_cp.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
typedef enum {
    IMEI_0 = 0,
    IMEI_1,
    IMEI_MAX
} IMEI_NUM;

typedef enum {
    IMEI_DIAG_READ_SUBCMD = 0,
    IMEI_DIAG_WRITE_SUBCMD
} IMEI_DIAG_SUBCMD;

#define IMEI_READ_EVENT_FLAG          BIT_0
#define IMEI_WRITE_EVENT_FLAG         BIT_1

#ifdef IMEI_AT_CHECKDIGIT
#define IMEI_STRING_MAX_LEN           14
#else
#define IMEI_STRING_MAX_LEN           15
#endif

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
aos_semaphore_ptr s_imei_semp_ptr;
AT_REQ_RESP *s_imei_at_req;
ulong s_imei_line_pointers[2];
u8 s_imei_line_cnt;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
#ifdef IMEI_AT_CHECKDIGIT
/**
 * aic_imei_checkdigit_get - get imei check digit.
 *
 * Parameter:
 * @imei: input imei ptr.
 *
 * Return:
 */
char aic_imei_checkdigit_get(char *imei)
{
    int i, sum1 = 0, sum2 = 0, total = 0, temp = 0;

    for (i = 0; i < IMEI_STRING_MAX_LEN; i++) {
        if ((i % 2) == 0) {
            sum1 += imei[i] - '0';
        } else {
            temp = (imei[i] - '0') * 2;
            if (temp < 10) {
                sum2 += temp;
            } else {
                sum2 += 1 + temp - 10;
            }
        }
    }

    total = sum1 + sum2;
    temp = total % 10;
    if (temp == 0) {
        return '0';
    } else {
        return (char)(10 - temp + '0');
    }
}
#endif

/**
 * aic_imei_diag_handle - imei diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len | idx | data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_imei_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u8 *result_line, idx;

    do {
        result_line = aos_zalloc(AT_MAX_AP_RESP_LENGTH);

        s_imei_line_cnt = 1;
        if (diag_cmd->err_no) {
            snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                     "+CME ERROR: %d", -AIC_EACCES);
            s_imei_line_pointers[0] = (ulong)result_line;
            break;
        }

        switch (diag_cmd->subcmd) {
            case IMEI_DIAG_READ_SUBCMD:
                idx = *(u8 *)diag_cmd->data;
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH, "^IMEI=%d,", idx);
                hex2string((u8 *)diag_cmd->data + sizeof(idx),
                           result_line + strlen(result_line),
                           diag_cmd->data_len - sizeof(idx));
                /* last char set 0. */
                result_line[strlen(result_line) - 1] = '\0';
                s_imei_line_pointers[0] = (ulong)result_line;
                s_imei_line_pointers[1] = (ulong)AT_RESULT_OK;
                s_imei_line_cnt = 2;
                break;
            case IMEI_DIAG_WRITE_SUBCMD:
                aos_memcpy(result_line, AT_RESULT_OK,
                           strlen(AT_RESULT_OK));
                s_imei_line_pointers[0] = (ulong)result_line;
                break;
            default:
                AIC_ASSERT(0);
                break;
        }
    } while(0);

    aos_put_semaphore(s_imei_semp_ptr);
}


/**
 * at_imei_cmd - response at^IMEI=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_imei_at_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"^IMEI: (index),(imei)";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * at_imei_set_cmd - process at^IMEI=index or at^IMEI=index,value.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_imei_at_set_cmd(AT_REQ_RESP *at_req)
{
    u8 idx, *result_line, checkdigit, *para;
    boolean flag, valid;
    aos_status status;
    ulong *paras_arr;
    u16 count, len;
    int error_code;

    do {
        aos_memset(s_imei_line_pointers, 0, sizeof(s_imei_line_pointers));
        checkdigit = 0;

        result_line = aos_zalloc(AT_MAX_AP_RESP_LENGTH);

        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT^IMEI=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 2) {
            error_code = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            error_code = AIC_EINVAL;
            break;
        }

        idx = atoi((u8 *)paras_arr[0]);
        if (idx >= IMEI_MAX) {
            error_code = AIC_EINVAL;
            break;
        }

        if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
            error_code = -(AT_ERROR_CP_NOT_ALIVE);
            break;
        }

        if (count == 1) {
            ap_send_diag_cmd_to_cp(DIAG_CMD_IMEI, IMEI_DIAG_READ_SUBCMD, &idx,
                                   sizeof(idx));
        } else {
            if (strlen((u8 *)paras_arr[1]) != IMEI_STRING_MAX_LEN) {
                error_code =  AIC_ENOEXEC;
                break;
            }
            *result_line = idx;
            flag = string2hex((u8 *)paras_arr[1], result_line + sizeof(idx),
                              strlen((u8 *)paras_arr[1]));
            if (!flag) {
                error_code = AIC_EOVERFLOW;
                break;
            }

#ifdef IMEI_AT_CHECKDIGIT
            checkdigit = aic_imei_checkdigit_get((u8 *)paras_arr[1]);
            *(result_line + sizeof(idx) + IMEI_STRING_MAX_LEN / 2) =  (checkdigit << 4) & 0xFF;
            len = sizeof(idx) + IMEI_STRING_MAX_LEN / 2 + 1;
#else
            len = sizeof(idx) + ALIGN(strlen((u8 *)paras_arr[1]) / 2, 2);
#endif
            ap_send_diag_cmd_to_cp(DIAG_CMD_IMEI, IMEI_DIAG_WRITE_SUBCMD,
                                   result_line, len);
        }

        status = aos_get_semaphore(s_imei_semp_ptr, AOS_WAIT_FOREVER);
        AIC_ASSERT(status == AOS_SUCCESS);

        error_code = AIC_SUCCESS;
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", -error_code);
        s_imei_line_pointers[0] = (ulong)result_line;
        s_imei_line_cnt = 1;
    }

    aic_free_paras_mem(paras_arr, count);
    error_code = aic_fill_transmit_cmd_resp(at_req, s_imei_line_pointers,
                                            s_imei_line_cnt);

    if (s_imei_line_pointers[0] &&
        s_imei_line_pointers[0] != (ulong)result_line) {
        aos_free((u8 *)s_imei_line_pointers[0]);
    }

    if (result_line)
        aos_free(result_line);

    return error_code;
}

/*
 *  Add imei AT handle here.
 */
const static AT_CMD_HANDLE aic_imei_at_handles[] =
{
    {
        "AT^IMEI",
        aic_imei_at_cmd,
        NULL,
        aic_imei_at_set_cmd,
        NULL
    },
    {AIC_NULL},
};

/**
 * aic_imei_cmd_init - imei at and diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_imei_cmd_init(void)
{

    s_imei_semp_ptr = aos_create_semaphore("IMEI", 0);
    AIC_ASSERT(s_imei_semp_ptr);

    diag_register_cmd_handler(DIAG_CMD_IMEI, aic_imei_diag_handle, NULL);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)aic_imei_at_handles);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_imei_cmd_init);

