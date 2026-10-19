/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic AT Cmd and Response Parsing Utils
 *  This file contains some parsing utils of AT cmds,responses and urc.
 *  For example:
 *  Extract parameters from AT cmds,responses and urc;
 *  Get all lines from AT response.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09ee76
#define LOG_MODULE    AMOD_AT

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_osa.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include "aic_at_parse_utils.h"
#include "aic_serial_recv_send.h"
#include "aic_at_process_common.h"
#include "aic_general_at_handle.h"
#include "aic_at_urc_handle.h"
#include "aic_at_interact_cp.h"
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif

/* Define internal functions. */
static u16 get_one_line_len(u8 *start_ptr);
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
extern MIPCALL_CFG mipcall_cfg[ANET_PDP_MAX_NUM];
#endif
extern s32 current_time_zone;
extern s32 time_zone_in_quarters;
extern s32 cclk_time_zone;
extern u8 time_zone_set_flag;
extern REG_SERVICE_INFO reg_service_info;
extern aos_event_ptr reply_to_com_event;
#ifdef CONFIG_WAKEUP_SAMPLE
extern boolean wakeup_sample_is_urc_need_wakeup(u8 *urc_str);
#endif

#ifdef CONFIG_D2D
static AT_D2D_URC_MSG_INFO *at_d2d_urc_msg_info_arr[AT_MAX_D2D_URC_CNT] = {0};

AT_D2D_URC_MSG_INFO d2d_urc_msg_info[] =
{
    {AT_D2D_URC_REVMSG, AT_D2D_URC_REVMSG_MSG_TYPE},
    {AT_D2D_URC_RVOIDATA, AT_D2D_URC_RVOIDATA_MSG_TYPE},
    {AT_D2D_URC_REPDR, AT_D2D_URC_REPDR_MSG_TYPE},
    {AT_D2D_URC_DATAFC, AT_D2D_URC_DATAFC_MSG_TYPE},
    {AT_D2D_URC_DSQ, AT_D2D_URC_DSQ_MSG_TYPE},
    {AIC_NULL},
};
#endif

/**
 * aic_split_paras_to_str_array - Get all the parameters in an AT cmd or
 * response or URC string.Each parameter is extracted to a string
 * without double quotations.
 *
 * @raw_str: The raw string containing several paraters.
 * @start_str: The starting key word in @raw_str,and parameters begin
 *             after @start_str.
 * @paras_cnt: The pointer to save the count of parameters.
 *
 * For example:for the following raw string,"+CTZE: " is the start_str,
 * and parameter count is 3.
 * +CTZE: <tz>,<dst>,<time>
 *
 * @Return the pointer of the pointer arrays of all parameters if split
 *  successfully,or return NULL.
 */
ulong *aic_split_paras_to_str_array(u8 *raw_str, u8 *start_str, u16 *paras_cnt)
{
    u16 para_len, raw_str_len;
    u8 cnt = 0, i, quotation_cnt = 0;
    ulong *str_arr = AIC_NULL;
    ulong str_pointer[30] = {0};
    u8 *paras_ptr, *tmp_ptr1;
    u8 *valid_str;

    if (!raw_str || !start_str)
        return AIC_NULL;

    if (strlen(raw_str) <= strlen(start_str) || !strcasestr(raw_str, start_str)) {
        *paras_cnt = 0;
        return AIC_NULL;
    }

    raw_str_len = strlen(raw_str) + 1;
    valid_str = aos_zalloc_relaxed(raw_str_len);
    if (!valid_str) {
        aicx_error(0x12f8db, "", "split paras error,malloc fail\n");
        *paras_cnt = 0;
        return AIC_NULL;
    }

    strcpy(valid_str, raw_str);
    /* The point where paras start. */
    paras_ptr = valid_str + strlen(start_str);
    tmp_ptr1 = paras_ptr;
    while (*tmp_ptr1) {
        if (*tmp_ptr1 == '(') {
            /* Track until ')' */
            tmp_ptr1++;
            paras_ptr = tmp_ptr1;
            while (*tmp_ptr1 && *tmp_ptr1 != ')') {
                tmp_ptr1++;
            }
            if (*tmp_ptr1 == ')') {
                *tmp_ptr1 = '\0';
                para_len = tmp_ptr1 - paras_ptr;
                str_pointer[cnt] = (ulong)aos_malloc_relaxed(para_len + 1);
                if (!str_pointer[cnt])
                    goto SPLIT_ERROR;
                strcpy((u8 *)str_pointer[cnt], paras_ptr);
                cnt++;
                if (cnt >= 30)
                    break;
                tmp_ptr1++;
                if (*tmp_ptr1 == ',') {
                    tmp_ptr1++;
                }
            }
        } else if (*tmp_ptr1 == '"') {
            quotation_cnt = 1;
            tmp_ptr1++;
            paras_ptr = tmp_ptr1;
            while (*tmp_ptr1) {
                if (*tmp_ptr1 != '"') {
                    tmp_ptr1++;
                    continue;
                }

                if (*tmp_ptr1 == '"') {
                    quotation_cnt++;
                    if (quotation_cnt % 2 == 0) {
                        if (*(tmp_ptr1 + 1) == ',' || is_cr_lf_sign(*(tmp_ptr1 + 1)) ||
                            *(tmp_ptr1 + 1) == 0) {
                            *tmp_ptr1 = '\0';
                            para_len = tmp_ptr1 - paras_ptr;
                            str_pointer[cnt] = (ulong)aos_malloc_relaxed(para_len + 1);
                            if (!str_pointer[cnt])
                                goto SPLIT_ERROR;

                            strcpy((u8 *)str_pointer[cnt], paras_ptr);
                            cnt++;
                            if (cnt >= 30)
                                goto SPLIT_ERROR;

                            tmp_ptr1++;
                            if (*tmp_ptr1 == ',') {
                                tmp_ptr1++;
                            }

                            break;
                        } else {
                            goto SPLIT_ERROR;
                        }
                    }
                }
            }

            if (quotation_cnt % 2 != 0) {
                aicx_error(0x083e00, "", "quotation count error\n");
                goto SPLIT_ERROR;
            }
        } else if (*tmp_ptr1 == ',') {
            /* Two continuous ',',that means there is an empty para. */
            str_pointer[cnt] = (ulong)aos_malloc_relaxed(1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;
            strcpy((u8 *)str_pointer[cnt], "");
            cnt++;
            if (cnt >= 30)
                break;

            tmp_ptr1++;
        } else {
            if (is_cr_lf_sign(*tmp_ptr1)) {
                tmp_ptr1++;
                continue;
            }

            /* Track until ',' or \r\n */
            paras_ptr = tmp_ptr1;
            while (*tmp_ptr1 && *tmp_ptr1 != ',' && !(is_cr_lf_sign(*tmp_ptr1))) {
                tmp_ptr1++;
            }

            if (*tmp_ptr1 == ',' || is_cr_lf_sign(*tmp_ptr1)) {
                *tmp_ptr1 = '\0';
                tmp_ptr1++;
            }

            para_len = tmp_ptr1 - paras_ptr;
            str_pointer[cnt] = (ulong)aos_malloc_relaxed(para_len + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;
            strcpy((u8 *)str_pointer[cnt], paras_ptr);
            cnt++;
            if (cnt >= 30)
                break;
        }
    }

    if (cnt > 0) {
        str_arr = (ulong *)aos_zalloc_relaxed(sizeof(ulong) * cnt);
        if (!str_arr)
            goto SPLIT_ERROR;

        for (i = 0; i < cnt; i++) {
            str_arr[i] = str_pointer[i];
        }
    }
    *paras_cnt = cnt;
    aos_free(valid_str);

    return str_arr;

SPLIT_ERROR:
    *paras_cnt = 0;

    aos_free(valid_str);
    for (i = 0; i < cnt; i++) {
        aos_free(str_pointer[i]);
    }

    return AIC_NULL;
}

/**
 * aic_get_each_line_from_resp - Extract each line of @at_resp received from CP.
 * The initial lines in @at_resp start and end with \r\n. Each line is
 * extracted to a string without \r\n.
 *
 * @at_resp: The response string received from CP.
 * @line_count: The number of lines.
 *
 * @Return the pointer of the pointer arrays of all lines if extract
 *  successfully,or return NULL.
 */
ulong *aic_get_each_line_from_resp(u8 *at_resp, u16 line_count)
{
    u16 i, line_len;
    ulong string_len;
    ulong *str_arr;
    u8 *start_ptr, *tmp_ptr;

    if (line_count == 0 || !at_resp)
        return AIC_NULL;

    str_arr = (ulong *)aos_malloc(sizeof(ulong) * line_count);
    if (!str_arr)
        return AIC_NULL;

    start_ptr = at_resp;
    for (i = 0; i < line_count; i++) {
        line_len = get_one_line_len(start_ptr);
        /* line_len includes the leading and ending \r\n */
        if (line_len < AT_MIN_RESP_LENGTH) {
            string_len = 0;
        } else {
            string_len = (ulong)(line_len - 4);
        }

        str_arr[i] = (ulong)aos_malloc(string_len + 1);
        if (!str_arr[i]) {
            aic_free_paras_mem(str_arr, i);
            return AIC_NULL;
        }

        if (string_len > 0) {
            /* Skip the leading \r\n. */
            tmp_ptr = start_ptr + 2;
            strncpy((u8 *)str_arr[i], tmp_ptr, string_len);
        }
        *(u8 *)(str_arr[i] + string_len) = '\0';

        start_ptr += line_len;
        /* The end. */
        if (*start_ptr == '\0') {
            break;
        }
    }

    return str_arr;
}

u16 get_one_line_len(u8 *start_ptr)
{
    u8 *tmp_ptr;
    u16 len = 0;

    if (strlen(start_ptr) < AT_MIN_RESP_LENGTH)
        return strlen(start_ptr);

    tmp_ptr = start_ptr;
    /* Start and end with \r\n,that is ASCII 0x0D 0x0A. */
    tmp_ptr += 2;
    len += 2;
    while (!(*tmp_ptr == aic_get_cr_sign() && *(tmp_ptr + 1) == aic_get_lf_sign()) &&
           *tmp_ptr != '\0') {
        tmp_ptr++;
        len++;
    }

    if (*tmp_ptr == aic_get_cr_sign() && *(tmp_ptr + 1) == aic_get_lf_sign()) {
        len += 2;  /* include \n */
    }

    return len;
}

/**
 * aic_free_paras_mem - Free all the memory pointers saved in @ptr.
 *
 * @ptr: The pointer to all memory pointers malloced.
 * @paras_cnt: The count of the memory pointers malloced.
 */
void aic_free_paras_mem(ulong *ptr, u16 paras_cnt)
{
    u16 i;

    if (!ptr) {
        return;
    }

    for (i = 0; i < paras_cnt; i++) {
        if (ptr[i]) {
            aos_free(ptr[i]);
        }
    }
    aos_free(ptr);
}

/**
 * aic_transmit_at_data - Transmit data payload to upper devices
 * via certain serial port.
 *
 * @serial_port_id: The serial port id to transmit data.
 * @data_buf: The pointer to the data buffer.
 * @data_len: The length of the data payload.
 *
 * @Return SUCCESS if transmit successfully,or return ERROR.
 */
s32 aic_transmit_at_data(u8 serial_port_id, u8 *data_buf, u16 data_len)
{
    s32 ret = AIC_SUCCESS;
#ifdef CONFIG_CUSTOMER_AT
    u8 ending_str[2];
#endif

#ifdef CONFIG_WAKEUP_SAMPLE
    /* If MCU is sleeping,wake it up before send data. */
    if (serial_port_id == conn_get_uart_at_conid)
        return aic_reply_to_uart_with_wakeup(serial_port_id, data_buf, data_len);
#endif

    ret = aic_reply_to_certain_com(serial_port_id, data_buf, data_len);

#ifdef CONFIG_CUSTOMER_AT
    ending_str[0] = aic_get_cr_sign();
    ending_str[1] = aic_get_lf_sign();
    aic_reply_to_certain_com(serial_port_id, ending_str, 2);
#endif

    return ret;
}

#ifdef CONFIG_CUSTOMER_CM_AT
static void transmit_new_format_one_line(AT_REQ_RESP *at_req_ptr, u8 *reply_ptr,
                                         u32 reply_len)
{
    boolean is_final_result = AIC_FALSE;
    u8 line_cnt;
    u32 total_len, final_len = 0;
    u8 *new_resp_ptr, *tmp_ptr;

    is_final_result = is_final_result_code(reply_ptr, reply_len, AIC_FALSE);
    at_req_ptr->resp_line_count++;
    line_cnt = at_req_ptr->resp_line_count;
    total_len = reply_len + 5;
    new_resp_ptr = (u8 *)aos_dma_malloc(total_len);
    if (!new_resp_ptr)
        return;

    aos_memset(new_resp_ptr, 0, total_len);
    tmp_ptr = new_resp_ptr;

    if (is_final_result) {
        *tmp_ptr = aic_get_cr_sign();
        tmp_ptr++;
        *tmp_ptr = aic_get_lf_sign();
        tmp_ptr++;
        final_len += 2;
    } else {
        if (line_cnt == 1) {
            *tmp_ptr = aic_get_cr_sign();
            tmp_ptr++;
            *tmp_ptr = aic_get_lf_sign();
            tmp_ptr++;
            final_len += 2;
        }
    }

    aos_memcpy(tmp_ptr, reply_ptr, reply_len);
    tmp_ptr += reply_len;

    *tmp_ptr = aic_get_cr_sign();
    tmp_ptr++;
    *tmp_ptr = aic_get_lf_sign();
    final_len += (reply_len + 2);

    if (aic_transimt_reply_no_copy(at_req_ptr->serial_port_id, new_resp_ptr, final_len))
        aos_free(new_resp_ptr);
}

static void transmit_new_format_multi_line(AT_REQ_RESP *at_req_ptr,
                                           ulong *reply_line_ptr, u16 line_cnt)
{
    u32 i, line_len;

    for (i = 0; i < line_cnt; i++) {
        line_len = strlen((u8 *)reply_line_ptr[i]);
        transmit_new_format_one_line(at_req_ptr, (u8 *)reply_line_ptr[i], line_len);
    }
}
#endif

boolean aic_is_powering_off(void)
{
    return reg_service_info.is_powering_off;
}

u8 *fill_head_and_end_signs(u8 *dst_ptr, u8 *src_str, u32 src_len)
{
    u8 *tmp_ptr;

    tmp_ptr = dst_ptr;
    *tmp_ptr = aic_get_cr_sign();
    tmp_ptr++;
    *tmp_ptr = aic_get_lf_sign();
    tmp_ptr++;

    aos_memcpy(tmp_ptr, src_str, src_len);
    tmp_ptr += src_len;

    /* The ending <CR><LF>. */
    *tmp_ptr = aic_get_cr_sign();
    tmp_ptr++;
    *tmp_ptr = aic_get_lf_sign();
    tmp_ptr++;
    *tmp_ptr = '\0';

    return tmp_ptr;
}

#ifdef CONFIG_D2D
static void aic_reply_ap_resp_msg_format(AT_REQ_RESP *at_req_ptr, u8 *reply_str, u32 reply_size)
{
    u32 total_len;

    if (!at_req_ptr->at_resp)
        at_req_ptr->at_resp = aos_zalloc(SIZE_1K);

    if (at_req_ptr->resp_line_count == 0) {
        total_len = reply_size + 4;
        fill_head_and_end_signs(at_req_ptr->at_resp + AT_MSG_HEAD_LEN,
                                reply_str, reply_size);
        total_len += AT_MSG_HEAD_LEN;
    } else {
        total_len = reply_size + 4;
        fill_head_and_end_signs(at_req_ptr->at_resp + at_req_ptr->current_resp_len,
                                reply_str, reply_size);
    }

    at_req_ptr->resp_line_count++;
    at_req_ptr->current_resp_len += total_len;
    if (is_final_result_code(reply_str, reply_size, AIC_FALSE)) {
        aicx_info(0x134de5, "dd", "reply with msg head:line cnt %d, current_resp_len %d\n",
                  at_req_ptr->resp_line_count, at_req_ptr->current_resp_len);
        aic_at_prepend_msg_head(at_req_ptr->at_resp, at_req_ptr,
                                (at_req_ptr->current_resp_len - AT_MSG_HEAD_LEN));
        aic_reply_to_certain_com(at_req_ptr->serial_port_id,
                                 at_req_ptr->at_resp, at_req_ptr->current_resp_len);
    }
}

boolean aic_at_with_msg_head(AT_REQ_RESP *at_req_ptr)
{
    if (at_req_ptr && (at_req_ptr->msg_head.msg_type ||at_req_ptr->msg_head.cmd_len))
        return AIC_TRUE;

    return AIC_FALSE;
}

void aic_at_prepend_msg_head(u8 *dst_ptr, AT_REQ_RESP *at_req_ptr, u32 resp_len)
{
    if (!dst_ptr)
        return;

    *((u16 *)dst_ptr) = at_req_ptr->msg_head.msg_type;
    *((u16 *)(dst_ptr + 2)) = (u16)resp_len;
    /* Change endian. */
    //*((u16 *)(dst_ptr + 2)) = ((*((u16 *)(dst_ptr + 2)) >> 8) |
    //                           (*((u16 *)(dst_ptr + 2)) << 8)) & 0xFFFF;
    *((u32 *)(dst_ptr + 4)) = at_req_ptr->msg_head.msg_id;
}

u32 aic_urc_prepend_msg_head(u8 *dst_ptr, u8 *urc_str, u32 urc_len)
{
    u32 ret = AIC_SUCCESS;

    if (!dst_ptr)
        return ret;

    ret = aic_get_urc_msg_type(urc_str, urc_len, (u16 *)dst_ptr);
    if (ret)
        *((u16 *)dst_ptr) = AT_D2D_URC_INVALID_MSG_TYPE;

    *((u16 *)(dst_ptr + 2)) = (u16)urc_len;
    *((u32 *)(dst_ptr + 4)) = AT_URC_MSG_ID;
    aicx_info(0x19d39d, "xxxxxxxx", "urc msg head: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
              *dst_ptr, *(dst_ptr + 1), *(dst_ptr + 2), *(dst_ptr + 3),
              *(dst_ptr + 4), *(dst_ptr + 5), *(dst_ptr + 6), *(dst_ptr + 7));

    return ret;
}

u32 aic_add_urc_msg_info(AT_D2D_URC_MSG_INFO *head_msg_info_ptr)
{
    AT_D2D_URC_MSG_INFO *tmp_info_ptr;
    u8 i, idle_index = 0;

    if (!head_msg_info_ptr)
        return AT_ERROR_INVALID_MSG_INFO;

    /* Find the idle index in the array. */
    for (i = 0; i < AT_MAX_D2D_URC_CNT; i++) {
        if (at_d2d_urc_msg_info_arr[i] == 0) {
            idle_index = i;
            break;
        }
    }

    if (i == AT_MAX_D2D_URC_CNT)
        return AT_ERROR_MSG_INFO_OVERFLOW;

    tmp_info_ptr = head_msg_info_ptr;
    while (tmp_info_ptr->urc_str != AIC_NULL && idle_index < AT_MAX_D2D_URC_CNT) {
        at_d2d_urc_msg_info_arr[idle_index] = tmp_info_ptr;
        tmp_info_ptr++;
        idle_index++;
    }

    if (idle_index == AT_MAX_D2D_URC_CNT && tmp_info_ptr->urc_str != AIC_NULL) {
        aicx_error(0x0a7c43, "", "error,too many URC msg info\n");
        return AT_ERROR_MSG_INFO_OVERFLOW;
    }

    return AIC_SUCCESS;
}

u32 aic_get_urc_msg_type(u8 *urc_ptr, u16 urc_len, u16 *msg_type_ptr)
{
    u16 i;
    u8 head_len = 0;
    u8 *tmp_ptr1 = urc_ptr, *tmp_ptr2 = AIC_NULL;
    u8 urc_head[AT_MAX_HEAD_LEN] = {0};
    AT_D2D_URC_MSG_INFO *msg_info_ptr;

    /* Skip the leading \r\n. */
    for (i = 0; i < 2; i++) {
        if (is_cr_lf_sign(*tmp_ptr1))
            tmp_ptr1++;
    }

    tmp_ptr2 = strchr(urc_ptr, ':');
    if (tmp_ptr2 && tmp_ptr2 > tmp_ptr1) {
        head_len = tmp_ptr2 - tmp_ptr1;
        if (head_len < sizeof(urc_head))
            strncpy(urc_head, tmp_ptr1, head_len);
        else {
            *msg_type_ptr = AT_D2D_URC_INVALID_MSG_TYPE;
            return AT_ERROR_CMD_TOO_LONG;
        }
    }

    for (i = 0; i < AT_MAX_D2D_URC_CNT; i++) {
        msg_info_ptr = at_d2d_urc_msg_info_arr[i];
        if (msg_info_ptr) {
            if (head_len == strlen(msg_info_ptr->urc_str) &&
                strncasecmp(msg_info_ptr->urc_str, urc_head, head_len) == 0) {
                *msg_type_ptr = msg_info_ptr->urc_msg_type;
                aicx_info(0x0b3d86, "x", "get urc msg type 0x%x\n", *msg_type_ptr);
                return AIC_SUCCESS;
            }
        } else
            break;
    }

    *msg_type_ptr = AT_D2D_URC_INVALID_MSG_TYPE;
    return AT_ERROR_CMD_NOT_FOUND;
}
#endif

/**
 * aic_format_transmit_reply - Format and transmit reply of AT cmds handled
 * by AP to the upper device.
 *
 * @serial_port_id: The serial port id to transmit the reply.
 * @reply_str: The pointer to the reply string.
 * @reply_size: The length of the reply string.
 * @is_urc: If the reply is URC or not.
 *
 * @Return SUCCESS if transmit successfully,or return ERROR.
 */
s32 aic_format_transmit_reply(u32 serial_port_id, u8 *reply_str,
                              u32 reply_size, boolean is_urc)
{
    u8 *cmd_urc_ap = AIC_NULL;
    u32 total_len;
    u32 com_id;
    AT_REQ_RESP *at_req_ptr = AIC_NULL;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;

    if (!reply_str || reply_size == 0)
        return AIC_SUCCESS;

    if (!is_urc) {
        at_com_item = aic_find_at_com_item(serial_port_id);
        if (!at_com_item || !at_com_item->serial_dispatch_at)
            return AIC_SUCCESS;

        at_req_ptr = at_com_item->serial_dispatch_at;
#ifdef CONFIG_D2D
        if (aic_at_with_msg_head(at_req_ptr)) {
            aic_reply_ap_resp_msg_format(at_req_ptr, reply_str, reply_size);
            return AIC_SUCCESS;
        }
#endif
    }
#ifdef CONFIG_D2D
else {
        if (aic_is_urc_msg_head_enabled()) {
            total_len = reply_size + AT_MSG_HEAD_LEN + 5;
            cmd_urc_ap = aic_dma_malloc(total_len);
            if (!cmd_urc_ap)
                return AT_ERROR_MALLOC_FAIL;

            total_len--;
            *(cmd_urc_ap + total_len) = 0;
            fill_head_and_end_signs((cmd_urc_ap + AT_MSG_HEAD_LEN), reply_str, reply_size);

            if (aic_urc_prepend_msg_head(cmd_urc_ap, (cmd_urc_ap + AT_MSG_HEAD_LEN),
                (reply_size + 4)) && serial_port_id == AT_D2D_URC_CMUX_COM_ID) {

                aos_free(cmd_urc_ap);
                return AIC_SUCCESS;
            }

            aic_reply_to_certain_com(serial_port_id, cmd_urc_ap, total_len);
            aos_free(cmd_urc_ap);

            return AIC_SUCCESS;
        }
    }
#endif

#ifdef CONFIG_CUSTOMER_CM_AT
    if (!is_urc) {
        transmit_new_format_one_line(at_req_ptr, reply_str, reply_size);
        return AIC_SUCCESS;
    } else {
        if (strcasestr(reply_str, "REBOOTING") || strcasestr(reply_str, "POWER OFF"))
            reg_service_info.is_powering_off = AIC_TRUE;
    }
#endif

    /* Add the leading and ending <CR><LF>. */
    total_len = reply_size + 5;
    cmd_urc_ap = (u8 *)aos_dma_malloc(total_len);
    if (!cmd_urc_ap) {
        aicx_error(0x0b9a96, "", "malloc urc from ap fail\n");
        return AIC_ENOMEM;
    }

    aos_memset(cmd_urc_ap, 0, total_len);
    fill_head_and_end_signs(cmd_urc_ap, reply_str, reply_size);
    if (is_urc) {
        com_id = aic_get_configed_urc_port_id();
        if (com_id == AT_NO_SERIAL_PORT_ID)
            com_id = serial_port_id;
        else if (com_id != serial_port_id) {
            aicx_info(0x14eacc, "d", "follow configed urc port %d\n", com_id);
            aos_free(cmd_urc_ap);
            return AIC_SUCCESS;
        }

#ifdef CONFIG_CUSTOMER_AT
        if (!aic_is_urc_cache_enabled()) {
            aic_send_urc_gpio_signal();
        } else {
            aic_add_urc_to_cache(cmd_urc_ap);
            aos_free(cmd_urc_ap);
            return AIC_SUCCESS;
        }
#endif
    } else {
        com_id = serial_port_id;
    }

    if (com_id == AT_NO_SERIAL_PORT_ID) {
        aos_free(cmd_urc_ap);
        return AT_ERROR_NO_SERIAL_CHANNEL;
    }

#ifdef CONFIG_WAKEUP_SAMPLE
    if (wakeup_sample_is_urc_need_wakeup(cmd_urc_ap) && aic_is_uart_com(com_id)) {
        aic_reply_to_uart_with_wakeup(com_id, cmd_urc_ap, (total_len - 1));
        aos_free(cmd_urc_ap);
        return AIC_SUCCESS;
    }
#endif

    if (aic_transimt_reply_no_copy(com_id, cmd_urc_ap, (total_len - 1)))
        aos_free(cmd_urc_ap);

    return AIC_SUCCESS;
}

#ifdef CONFIG_CUSTOMER_AT
static void assemble_special_format_resp(AT_REQ_RESP *at_req_resp,
                                         ulong *reply_line_ptr, u16 line_cnt)
{
    u16 len = 0, i, line_len;
    u8 *tmp_ptr;
    boolean is_final_result = AIC_FALSE;

    line_len = strlen((u8 *)reply_line_ptr[0]);
    is_final_result = is_final_result_code((u8 *)reply_line_ptr[0], line_len, AIC_FALSE);

    /* Only final result code. Only append \r\n at the end. */
    if (line_cnt == 1 && is_final_result && at_req_resp->resp_line_count == 0) {
        len = strlen((u8 *)reply_line_ptr[0]) + 3;
        at_req_resp->at_resp = (u8 *)aos_zalloc(len);
        AOS_ASSERT(at_req_resp->at_resp);
        tmp_ptr = at_req_resp->at_resp;
        strcpy(tmp_ptr, (u8 *)reply_line_ptr[0]);
        tmp_ptr += line_len;
        *tmp_ptr = aic_get_cr_sign();
        tmp_ptr++;
        *tmp_ptr = aic_get_lf_sign();
    } else if (line_cnt > 1 || at_req_resp->resp_line_count > 0 || !is_final_result) {
        for (i = 0; i < line_cnt; i++) {
            line_len = strlen((u8 *)reply_line_ptr[i]);
            if (is_final_result_code((u8 *)reply_line_ptr[i],line_len, AIC_FALSE))
                len += (line_len + 4);
            else
                len += (line_len + 2);
        }

        len ++;
        at_req_resp->at_resp = (u8 *)aos_zalloc(len);
        AOS_ASSERT(at_req_resp->at_resp);
        tmp_ptr = at_req_resp->at_resp;
        for (i = 0; i < line_cnt; i++) {
            line_len = strlen((u8 *)reply_line_ptr[i]);
            if (is_final_result_code((u8 *)reply_line_ptr[i], line_len, AIC_FALSE)) {
                *tmp_ptr = aic_get_cr_sign();
                tmp_ptr++;
                *tmp_ptr = aic_get_lf_sign();
                tmp_ptr++;
            }

            strcat(tmp_ptr, (u8 *)reply_line_ptr[i]);
            tmp_ptr += strlen((u8 *)reply_line_ptr[i]);
            /* The ending <CR><LF>. */
            *tmp_ptr = aic_get_cr_sign();
            tmp_ptr++;
            *tmp_ptr = aic_get_lf_sign();
            tmp_ptr++;
        }
    }
}
#endif

/**
 * aic_fill_transmit_cmd_resp - Fill the reply(not data payload) to certain
 * format and transmit it to the upper device.
 *
 * @at_req_resp: The pointer to the struct AT_REQ_RESP for the at command.
 * @reply_line_ptr: The pointer to all the reply line pointers.
 * @line_cnt: The count of lines to reply.
 *
 * The function is called by modules who process AT commands.
 * For example:Fill the string "OK" to the foramt below and transmit it.
 * <CR><LF>OK<CR><LF>
 *
 * @Return SUCCESS if the reply is successfully processed,or return ERROR.
 */
s32 aic_fill_transmit_cmd_resp(AT_REQ_RESP *at_req_resp,
                               ulong *reply_line_ptr, u16 line_cnt)
{
    u16 i = 0;
#ifndef CONFIG_CUSTOMER_AT
    u8 *dst_ptr;
    u16 total_len = 0;
#endif

    if (!at_req_resp || !reply_line_ptr || line_cnt == 0)
        return AT_ERROR_INVALID_RESP;

    for (i = 0; i < line_cnt; i++) {
        if (!reply_line_ptr[i])
            return AT_ERROR_INVALID_RESP;
    }

    if (at_req_resp->is_external_cmd != AT_FROM_EXTERNAL ||
        at_req_resp->serial_port_id == AT_NO_SERIAL_PORT_ID) {
        return AT_ERROR_NO_SERIAL_CHANNEL;
    }

#ifdef CONFIG_D2D
    /* For AT cmds with msg format. */
    if (aic_at_with_msg_head(at_req_resp)) {
        for (i = 0; i < line_cnt; i++) {
            aic_reply_ap_resp_msg_format(at_req_resp, (u8 *)reply_line_ptr[i],
                                         strlen((u8 *)reply_line_ptr[i]));
        }

        return AIC_SUCCESS;
    }
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
    transmit_new_format_multi_line(at_req_resp, reply_line_ptr, line_cnt);
    return AIC_SUCCESS;
#endif

    if (at_req_resp->at_resp)
        aos_free(at_req_resp->at_resp);

#ifdef CONFIG_CUSTOMER_AT
    assemble_special_format_resp(at_req_resp, reply_line_ptr, line_cnt);
#else
    for (i = 0; i < line_cnt; i++) {
        total_len += (strlen((u8 *)reply_line_ptr[i]) + 4);
    }
    total_len ++;

    at_req_resp->at_resp = (u8 *)aos_zalloc(total_len);
    if (!at_req_resp->at_resp) {
        return AIC_ENOMEM;
    }

    dst_ptr = at_req_resp->at_resp;
    for (i = 0; i < line_cnt; i++) {
        dst_ptr = fill_head_and_end_signs(dst_ptr, (u8 *)reply_line_ptr[i],
                                          strlen((u8 *)reply_line_ptr[i]));
    }
#ifdef CONFIG_WAKEUP_SAMPLE
    if (wakeup_sample_is_urc_need_wakeup(at_req_resp->at_resp) &&
        aic_is_uart_com(at_req_resp->serial_port_id)) {
        aic_reply_to_uart_with_wakeup(at_req_resp->serial_port_id, at_req_resp->at_resp,
                                      strlen(at_req_resp->at_resp));
        return AIC_SUCCESS;
    }
#endif
#endif

    at_req_resp->resp_line_count += line_cnt;
    aic_transmit_reply_to_external(at_req_resp);

    return AIC_SUCCESS;
}

/* Delete the leading and ending \r\n for one line. */
void aic_delete_start_end_symbols(u8 *dst_str, u8 *raw_str, u16 len_str)
{
    u8 *tmp_ptr;

    /* Delete the leading and ending \r\n. */
    tmp_ptr = raw_str + 2;
    strncpy(dst_str, tmp_ptr, len_str);

    *(dst_str + len_str) = '\0';
}

/**
 * Convert the string of date_time to systime.
 * The format of date_time is "YYYY/MM/DD,hh:mm:ss".
 */
u32 aic_parse_civil_time(u8 *date_time, aos_systime *current_time)
{
    u8 *time_ptr, *tmp_ptr1 = AIC_NULL, *tmp_ptr2;
    u8 date_str[15] = {'\0'};
    u8 i = 0;
    char *save_ptr;

    if (!strchr(date_time, '/') || !strchr(date_time, ':'))
        return AT_ERROR_INVALID_PARAS;

    tmp_ptr1 = strchr(date_time, ',');
    if (!tmp_ptr1)
        return AT_ERROR_INVALID_PARAS;

    strncpy(date_str, date_time, (tmp_ptr1 - date_time));
    tmp_ptr2 = strtok_r(date_str, "/", &save_ptr);
    while (tmp_ptr2) {
        i++;
        if (i == 1) {
            current_time->year = atoi(tmp_ptr2);
        } else if (i == 2) {
            current_time->month = atoi(tmp_ptr2);
        } else if (i == 3) {
            current_time->day = atoi(tmp_ptr2);
        }

        tmp_ptr2 = strtok_r(AIC_NULL, "/", &save_ptr);
    }

    i = 0;
    tmp_ptr1++;
    time_ptr = tmp_ptr1;
    tmp_ptr2 = strtok_r(time_ptr, ":", &save_ptr);
    while (tmp_ptr2) {
        i++;
        if (i == 1) {
            current_time->hour = atoi(tmp_ptr2);
        } else if (i == 2) {
            current_time->minute = atoi(tmp_ptr2);
        } else if (i == 3) {
            current_time->second = atoi(tmp_ptr2);
        }

        tmp_ptr2 = strtok_r(AIC_NULL, ":", &save_ptr);
    }

    return AIC_SUCCESS;
}

/**
 * Ajust the time with time zone and leap year.
 * The time zone is of the range -12~12.
 */
void aic_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone)
{
    u32 lastday = 0;
    s32 added_hour = 0;
    boolean leap_year;
    s32 integer_tz, half_tz, minute_shift;

    if (!current_time)
        return;

    integer_tz = time_zone / 4;
    half_tz = time_zone % 4;
    minute_shift = (60 / 4) * half_tz;
    if (time_zone < 0)
        minute_shift = -minute_shift;

    leap_year = aic_is_leap_year(current_time->year);

    if(time_zone > 0) {
        if(current_time->month == 1 || current_time->month == 3 ||
           current_time->month == 5 || current_time->month == 7 ||
           current_time->month == 8 || current_time->month == 10 ||
           current_time->month == 12) {
            lastday = 31;
        } else if (current_time->month == 4 || current_time->month == 6 ||
                   current_time->month == 9 || current_time->month == 11) {
            lastday = 30;
        } else {
            if(leap_year)
                lastday = 29;
            else
                lastday = 28;
        }

        if(current_time->hour + integer_tz >= 24 ||
           (current_time->hour + integer_tz == 23 && current_time->minute + minute_shift >= 60)) {
            current_time->hour = current_time->hour + integer_tz - 24;
            current_time->day += 1;
            if(current_time->day > lastday) {
                current_time->day -= lastday;
                current_time->month += 1;
                if(current_time->month > 12) {
                    current_time->month -= 12;
                    current_time->year += 1;
                }
            }
            current_time->wday += 1;
            if (current_time->wday > 6)
                current_time->wday = 0;
        } else {
            current_time->hour += integer_tz;
        }

        if (minute_shift) {
            if (current_time->minute + minute_shift >= 60) {
                current_time->hour += 1;
                current_time->minute = current_time->minute + minute_shift - 60;
            } else
                current_time->minute += minute_shift;
        }
    } else if (time_zone < 0) {
        added_hour = current_time->hour + integer_tz;

        if(added_hour < 0 || (added_hour == 0 && current_time->minute < minute_shift)) {
            current_time->hour = added_hour + 24;
            current_time->day -= 1;
            if(current_time->day == 0) {
                current_time->month -= 1;
                if(current_time->month == 1 || current_time->month == 3 ||
                   current_time->month == 5 || current_time->month == 7 ||
                   current_time->month == 8 || current_time->month == 10 ||
                   current_time->month == 0) {
                    lastday = 31;
                } else if (current_time->month == 4 ||
                           current_time->month == 6 ||
                           current_time->month == 9 ||
                           current_time->month == 11) {
                    lastday = 30;
                } else {
                    if(leap_year)
                        lastday = 29;
                    else
                        lastday = 28;
                }
                current_time->day += lastday;
                if(current_time->month == 0) {
                    current_time->day = 31;
                    current_time->month += 12;
                    current_time->year -= 1;
                }
            }

            if (current_time->wday >= 1)
                current_time->wday -= 1;
            else
                current_time->wday = 6;
        } else {
            current_time->hour += integer_tz;
        }

        if (minute_shift) {
            if (current_time->minute < minute_shift) {
                current_time->hour -= 1;
                current_time->minute = current_time->minute + 60 - minute_shift;
            } else
                current_time->minute -= minute_shift;
        }
    }
}

boolean aic_is_leap_year(u32 year)
{
    return (year % 4) || ((year % 100 == 0) && (year % 400)) ? 0 : 1;
}

/* The time zone may be set by different ways. */
s32 aic_get_current_time_zone(void)
{
    if (aic_is_valid_time_zone(current_time_zone)) {
        return current_time_zone;
    } else {
        return AIC_DEFAULT_TIME_ZONE;
    }
}

s32 aic_get_time_zone_in_quarters(void)
{
    if (aic_is_valid_time_zone_quarters(time_zone_in_quarters)) {
        return time_zone_in_quarters;
    } else {
        return AIC_DEFAULT_TIME_ZONE_QUARTERS;
    }
}

int aic_set_current_time_zone(s32 time_zone)
{
    if (aic_is_valid_time_zone(time_zone)) {
        current_time_zone = time_zone;
        time_zone_in_quarters = time_zone * 4;
        time_zone_set_flag = 1;
        return AIC_SUCCESS;
    } else {
        return AIC_EFAULT;
    }
}

int aic_set_time_zone_in_quarters(s32 time_zone)
{
    if (aic_is_valid_time_zone_quarters(time_zone)) {
        current_time_zone = time_zone / 4;
        time_zone_in_quarters = time_zone;
        time_zone_set_flag = 1;
        return AIC_SUCCESS;
    } else {
        return AIC_EFAULT;
    }
}

/* time_zone is in quarters of an hour. */
boolean aic_is_valid_time_zone_quarters(s32 time_zone)
{
    return (time_zone >= AIC_MIN_TIME_ZONE_QUARTERS && time_zone <= AIC_MAX_TIME_ZONE_QUARTERS);
}

/* time_zone is in unit of an hour. */
boolean aic_is_valid_time_zone(s32 time_zone)
{
    return (time_zone >= AIC_MIN_TIME_ZONE && time_zone <= AIC_MAX_TIME_ZONE);
}

boolean aic_is_valid_integer_para(u8 *para_str, u8 para_len)
{
    u8 i;

    if (!para_str || para_len == 0)
        return AIC_FALSE;

    for (i = 0; i < para_len; i++) {
        if (!(*(para_str + i) >= '0' && *(para_str + i) <= '9'))
            return AIC_FALSE;
    }

    return AIC_TRUE;
}

boolean aic_is_valid_pdp_cid(u16 cid)
{
    if (!(cid >= ANET_MIN_PDP_CID && cid <= ANET_MAX_PDP_CID)) {
        return AIC_FALSE;
    }

    return AIC_TRUE;
}

boolean aic_is_ims_pdp_cid(u8 pdp_cid)
{
    if (!reg_service_info.ims_enabled)
        return AIC_FALSE;

    if (pdp_cid == ANET_DEFAULT_IMS_PDP_CID || pdp_cid == ANET_IMS_VOICE_PDP_CID)
        return AIC_TRUE;

    return AIC_FALSE;
}

void aic_enter_flight_mode(void)
{
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    u8 i;
#endif

#if CONFIG_CUSTOMER_CM_AT
    current_time_zone = AIC_DEFAULT_TIME_ZONE;
    time_zone_in_quarters = AIC_DEFAULT_TIME_ZONE_QUARTERS;
    cclk_time_zone = AIC_DEFAULT_TIME_ZONE;
#endif
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    for (i = 0; i < ANET_PDP_MAX_NUM; i++)
        mipcall_cfg[i].act_by_mipcall = AIC_FALSE;
#endif

    /* Already in flight mode. */
    if (reg_service_info.flight_mode == ANET_IN_FLIGHT_MODE)
        return;

#ifdef CONFIG_PDP_MGR
    aic_deact_all_pdp(AIC_TRUE);
#endif

    reg_service_info.flight_mode = ANET_IN_FLIGHT_MODE;
}

void aic_set_flight_mode(u8 mode)
{
    reg_service_info.flight_mode = mode;
}

u8 aic_get_flight_mode(void)
{
    return reg_service_info.flight_mode;
}

boolean aic_is_flight_mode(void)
{
    return (reg_service_info.flight_mode == ANET_IN_FLIGHT_MODE);
}

s32 aic_transimt_reply_no_copy(u32 at_com_id, u8 *buf_ptr, u32 buf_size)
{
    AT_REPLY_TO_COM *resp_to_com = AIC_NULL;

    resp_to_com = aic_get_reply_to_com_ptr(at_com_id);
    if (!resp_to_com)
        return AT_ERROR_NO_SERIAL_CHANNEL;

    aic_put_reply_to_queue(buf_ptr, buf_size, at_com_id, AIC_FALSE, AIC_FALSE);

    aos_set_event(reply_to_com_event, resp_to_com->event_id, AOS_OR);

    return AIC_SUCCESS;
}

void aic_convert_numeric_ipv6_to_str(ulong *addr_numeric, u8 *addr_buf, u8 buf_len)
{
    if (!addr_numeric || !addr_buf)
        return;

    aos_memset(addr_buf, 0, buf_len);
    snprintf(addr_buf, buf_len, "%x:%x:%x:%x:%x:%x:%x:%x",
             (u32)addr_numeric[0] >> 16,
             (u32)addr_numeric[0] & 0xFFFF,
             (u32)addr_numeric[1] >> 16,
             (u32)addr_numeric[1] & 0xFFFF,
             (u32)addr_numeric[2] >> 16,
             (u32)addr_numeric[2] & 0xFFFF,
             (u32)addr_numeric[3] >> 16,
             (u32)addr_numeric[3] & 0xFFFF);
}

static int aic_data_is_hex(u8 *data)
{
    int num;

    if (data[0] >= '0' && data[0] <= '9')
        num = data[0] - '0';
    else if (data[0] == 'a' || data[0] == 'A')
        num = 10;
    else if (data[0] == 'b' || data[0] == 'B')
        num = 11;
    else if (data[0] == 'c' || data[0] == 'C')
        num = 12;
    else if (data[0] == 'd' || data[0] == 'D')
        num = 13;
    else if (data[0] == 'e' || data[0] == 'E')
        num = 14;
    else if (data[0] == 'f' || data[0] == 'F')
        num = 15;
    else
        num = -1;

    return num;
}

static int aic_data_is_octal(u8 *data)
{
    int num;

    if (data[0] >= '0' && data[0] <= '7')
        num = data[0] - '0';
    else
        num = -1;

    return num;
}

u32 aic_escape_data_normal(u8 *dst, u8 *src, int datalen, int *actual_len)
{
    int num1, num2, num3, len, offset = 0;
    u8 *ptr, *temp_ptr;

    temp_ptr = src;

    ptr = strchr(temp_ptr, '\\');
    if (!ptr) {
        aos_memcpy(dst, src, datalen);
        *actual_len = datalen;
    } else {
        do {
            len = ptr - temp_ptr;
            aos_memcpy(dst + offset, temp_ptr, len);
            offset += len;

            if (ptr + 1 == src + datalen) {
                dst[offset] = 0x5C;
                offset++;
                *actual_len = offset;
                return AIC_SUCCESS;
            }
            switch (*(ptr + 1)) {
            case 'a':
                dst[offset] = 0x07;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'b':
                //dst[offset] = 0x08;
                offset--;
                temp_ptr = ptr + 2;
                break;
            case 'f':
                dst[offset] = 0x0C;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'n':
                dst[offset] = 0x0A;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'r':
                dst[offset] = 0x0D;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 't':
                dst[offset] = 0x09;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'v':
                dst[offset] = 0x0B;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '\\':
                dst[offset] = 0x5C;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '\'':
                dst[offset] = 0x27;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '"':
                dst[offset] = 0x22;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '?':
                dst[offset] = 0x3F;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'x':
                num1 = aic_data_is_hex(ptr + 2);
                if (num1 >= 0) {
                    num2 = aic_data_is_hex(ptr + 3);
                    if (num2 >= 0) {
                        dst[offset] = num1 * 16 + num2;
                        offset++;
                        temp_ptr = ptr + 4;
                    } else {
                        dst[offset] = num1;
                        offset++;
                        temp_ptr = ptr + 3;
                    }
                } else {
                    dst[offset] = 0x5C;
                    dst[offset + 1] = 'x';
                    offset += 2;
                    temp_ptr = ptr + 2;
                }
                break;
            default:
                num1 = aic_data_is_octal(ptr + 1);
                if (num1 >= 0) {
                    num2 = aic_data_is_octal(ptr + 2);
                    if (num2 >= 0) {
                        num3 = aic_data_is_octal(ptr + 3);
                        if (num3 >= 0) {
                            if (num1 >= 0 && num1 < 4) {
                                dst[offset] = num1 * 8 * 8 + num2 * 8 + num3;
                                offset++;
                                temp_ptr = ptr + 4;
                            } else {
                                dst[offset] = num1 * 8 + num2;
                                offset++;
                                temp_ptr = ptr + 3;
                            }
                        } else {
                            dst[offset] = num1 * 8 + num2;
                            offset++;
                            temp_ptr = ptr + 3;
                        }
                    } else {
                        dst[offset] = num1;
                        offset++;
                        temp_ptr = ptr + 2;
                    }
                }else {
                    dst[offset] = 0x5C;
                    offset++;
                    temp_ptr = ptr + 1;
                }
                break;
            }

            ptr = strchr(temp_ptr, '\\');
            if (!ptr) {
                len = src + datalen - temp_ptr;
                aos_memcpy(dst + offset, temp_ptr, len);
                offset += len;
            }
        }while (ptr);

        *actual_len = offset;
    }

    return AIC_SUCCESS;
}
