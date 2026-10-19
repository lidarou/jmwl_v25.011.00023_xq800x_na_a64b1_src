/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic sn diag Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x064c22
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_md_api.h>
#include <aic_md_id.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include "SN.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum {
    SN_DIAG_READ_SUBCMD = 0,
    SN_DIAG_WRITE_SUBCMD
} SN_DIAG_SUBCMD;

typedef enum {
    SN_0 = 0,
    SN_1,
    SN_MAX
} SN_NUM;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
static u16 sn_id[SN_MAX] = {
    MD_ID_SN0,
    MD_ID_SN1
};

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_sn_diag_handle - imei diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len |data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_sn_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u16 item_id, item_size, rsp_len;
    diag_rsp_cmd_t rsp;
    u8 *ptr = NULL;
    int status;
    u8 idx;

    idx = *(u8 *)diag_cmd->data;
    if ((SN_MAX <= idx) || (!diag_cmd->data_len)) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_SN, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    item_id = sn_id[idx];
    item_size = aic_md_get_length(item_id);
    if (!item_size) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_SN, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    rsp_len = sizeof(idx);
    switch (diag_cmd->subcmd) {
        case SN_DIAG_READ_SUBCMD:
            ptr = aos_malloc(item_size + rsp_len);
            *ptr = idx;
            status = aic_md_read(item_id, item_size, ptr + rsp_len);
            rsp_len += item_size;
            break;
        case SN_DIAG_WRITE_SUBCMD:
            if (item_size < (diag_cmd->data_len - rsp_len)) {
                status = AIC_EINVAL;
                break;
            }

            ptr = aos_zalloc(item_size + rsp_len);
            aos_memcpy(ptr, (u8 *)diag_cmd->data, diag_cmd->data_len - rsp_len);
            status = aic_md_write(item_id, item_size, ptr + rsp_len);
            *ptr = idx;
            break;
        default:
            status = AIC_EINVAL;
            break;
    }

    if (AIC_SUCCESS == status) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_SN, diag_cmd->subcmd, ptr, rsp_len);
    } else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_SN, diag_cmd->subcmd, NULL, 0);
    }
    diag_cmd_response(&rsp);

    if (ptr) {
        aos_free(ptr);
    }
}

/**
 * at_sn_cmd - response at^SN=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
static u32 aic_sn_at_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"^SN: (0,1),()";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * at_sn_set_cmd - response at^SN=index or at^SN=index,value.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
static u32 aic_sn_at_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'}, *para;
    u16 count, item_id, line_cnt;
    ulong line_pointers[2];
    ulong *paras_arr;
    SN_T sn = {0};
    boolean valid;
    int status;
    u8 idx;

    do {
        line_cnt = 1;

        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT^SN=",
                                                 &count);
        if (!paras_arr || count < 1 || count > 2) {
            status = AIC_EINVAL;
            break;
        }

        para = (u8 *)paras_arr[0];
        valid = aic_is_valid_integer_para(para, strlen(para));
        if (!valid) {
            status = AIC_EINVAL;
            break;
        }

        idx = atoi((u8 *)paras_arr[0]);
        if (idx > SN_MAX) {
            status = AIC_EINVAL;
            break;
        }

        item_id = sn_id[idx];
        if (count == 1) {
            status = aic_md_read(item_id, sizeof(SN_T), &sn);
            if (status == AIC_SUCCESS) {
                snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                         "^SN=%d,%s", idx, (char *)&sn);
                line_pointers[0] = (ulong)result_line;
                line_pointers[1] = (ulong)AT_RESULT_OK;
                line_cnt = 2;
            }
        } else {
            aos_memcpy((u8 *)&sn, (u8 *)paras_arr[1], strlen((u8 *)paras_arr[1]));
            status = aic_md_write(item_id, sizeof(SN_T), &sn);
        }
    } while(0);

    if (status != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", status > 0 ? status : -status);
        line_pointers[0] = (ulong)result_line;
    } else if (line_cnt == 1) {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_cnt);
}

/*
 *  Add sn AT handle here.
 */
const static AT_CMD_HANDLE aic_sn_at_handles[] =
{
    {
        "AT^SN",
        aic_sn_at_cmd,
        NULL,
        aic_sn_at_set_cmd,
        NULL
    },
    {AIC_NULL},
};

/**
 * aic_sn_cmd_init - sn at and diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_sn_cmd_init(void)
{
    diag_register_cmd_handler(DIAG_CMD_SN, aic_sn_diag_handle, NULL);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)aic_sn_at_handles);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_sn_cmd_init);

