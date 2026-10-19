/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the basic mac diag Application Interface (API)
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cae5e
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
#include <aic_convert.h>
#include "MAC.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define MAC_SIZE            6

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum {
    MAC_DIAG_READ_SUBCMD = 0,
    MAC_DIAG_WRITE_SUBCMD
} MAC_DIAG_SUBCMD;

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_mac_diag_handle - mac diag handle.
 *  diag cmd:
 *  magic | sn | sys | errno | cmd | subcmd | len |data | tail
 *
 * Parameter:
 * @diag_cmd: receive diag parameter.
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_mac_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u16 item_id, item_size, rsp_len;
    diag_rsp_cmd_t rsp;
    u8 *ptr = NULL;
    int status;

    item_id = MD_ID_MAC;
    item_size = aic_md_get_length(item_id);
    if (!item_size) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_MAC, diag_cmd->subcmd, NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    rsp_len = 0;
    switch (diag_cmd->subcmd) {
        case MAC_DIAG_READ_SUBCMD:
            ptr = aos_malloc(item_size);
            status = aic_md_read(item_id, item_size, ptr);
            rsp_len += MAC_SIZE;
            break;
        case MAC_DIAG_WRITE_SUBCMD:
            if (MAC_SIZE != diag_cmd->data_len) {
                status = AIC_EINVAL;
                break;
            }
            ptr = aos_malloc(item_size);
            aos_memcpy(ptr, (u8 *)diag_cmd->data, MAC_SIZE);
            status = aic_md_write(item_id, item_size, ptr);
            break;
        default:
            status = AIC_EINVAL;
            break;
    }

    if (AIC_SUCCESS == status) {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_MAC, diag_cmd->subcmd,
                     ptr, rsp_len);
    } else {
        diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD, diag_cmd->m2,
                     diag_cmd->sys, DIAG_CMD_MAC, diag_cmd->subcmd, NULL, 0);
    }
    diag_cmd_response(&rsp);

    if (ptr) {
        aos_free(ptr);
    }
}

/**
 * at_mac_cmd - response at^MAC=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_mac_at_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[2];

    line_pointers[0] = (ulong)"^MAC: (mac)";
    line_pointers[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);
}

/**
 * at_mac_cmd - response at^MAC?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_mac_at_query_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    u16 line_cnt;
    int status;
    MAC_T mac;

    status = aic_md_read(MD_ID_MAC, sizeof(MAC_T), &mac);
    if (status == AIC_SUCCESS) {
        strcpy(result_line, "^MAC: ");
        hex2string(mac.MAC, result_line + strlen(result_line), MAC_SIZE);

        /* Fill response,two lines. */
        line_pointers[0] = (ulong)&result_line[0];
        line_pointers[1] = (ulong)AT_RESULT_OK;
        line_cnt = 2;
    } else {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", status > 0 ? status : -status);
        line_pointers[0] = (ulong)result_line;
        line_cnt = 1;
    }

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, line_cnt);
}

/**
 * aic_mac_at_set_cmd - set mac address, at^MAC=value.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_mac_at_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    boolean flag;
    int status;
    MAC_T mac;
    u16 count;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT^MAC=",
                                                 &count);
        if (!paras_arr || count != 1) {
            status = AIC_EINVAL;
            break;
        }

        if (strlen((u8 *)paras_arr[0]) != (MAC_SIZE * 2)) {
            status =  AIC_ENOEXEC;
            break;
        }

        flag = string2hex((u8 *)paras_arr[0], mac.MAC,
                          strlen((u8 *)paras_arr[0]));
        if (!flag) {
            status = AIC_EOVERFLOW;
            break;
        }

        status = aic_md_write(MD_ID_MAC, sizeof(MAC_T), &mac);

    } while(0);

    if (status != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", status > 0 ? status : -status);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add mac AT handle here.
 */
const static AT_CMD_HANDLE aic_mac_at_handles[] =
{
    {
        "AT^MAC",
        aic_mac_at_cmd,
        aic_mac_at_query_cmd,
        aic_mac_at_set_cmd,
        NULL
    },
    {AIC_NULL},
};

/**
 * aic_mac_diag_cmd_init - mac diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_mac_diag_cmd_init(void)
{
    diag_register_cmd_handler(DIAG_CMD_MAC, aic_mac_diag_handle, NULL);

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)aic_mac_at_handles);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_mac_diag_cmd_init);

