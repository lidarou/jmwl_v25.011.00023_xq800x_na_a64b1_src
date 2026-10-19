/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Receiving and Sending AT via Serial Ports
 *  This is the comonent receiving AT cmds via serial ports.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15fac8
#define LOG_MODULE    AMOD_AT

#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include "aic_serial_recv_send.h"
#include "aic_at_resp_handle.h"
#include "aic_at_cmd_handle.h"
#include "aic_at_interact_cp.h"
#include "aic_general_at_handle.h"
#ifdef CONFIG_CMUX
#include <cmux.h>
#endif

/* Define internal vars. */
AT_PORT_INFO at_ports_info[AT_MAX_SERIAL_PORT_NUM];
#ifndef CONFIG_AT_PROC_MINI
/* Save the last AT cmd for A/ cmd. */
static u8 *last_at_str = AIC_NULL;

/* Define internal functions. */
static s32 extract_single_at_cmd(u8 *raw_str, u32 serial_channel_id);
static u32 collect_complete_at(u8 *char_recved, u32 recved_len, AT_RECV_COM_ITEM *at_com_item);
static void reply_error_to_serial_port(u32 serial_port_id, u32 err_code);
static s32 process_complete_at_cmd(u8 *at_cmd_str, u32 at_len, u32 serial_port_id);
static boolean repeat_last_at_cmd(u8 *at_str);
#endif
static void replace_at_cmd_ending_sign(u8 *at_cmd_str);
static void aic_diag_at_cmd_handle(diag_send_cmd_t *diag_cmd, void *pri);

/* Define external vars. */
extern aos_event_ptr event_dispatch_at;

void aic_serial_recv_init(void)
{
    u8 i;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        at_ports_info[i].at_com_id = AT_NO_SERIAL_PORT_ID;
    }

    diag_register_cmd_handler(DIAG_CMD_AT_CMD, aic_diag_at_cmd_handle, AIC_NULL);
#ifndef CONFIG_AT_PROC_MINI
    conn_at_register_recv_cb(aic_serial_recv_callback, AIC_NULL);
    /* Retister handler to handle AT cmds of DIAG format. */
#ifdef CONFIG_CMUX
    cmux_at_register_recv_cb(aic_serial_recv_callback, AIC_NULL);
#endif
#endif
}

AT_PORT_INFO *aic_add_new_at_port(u32 port_id)
{
    u8 i;

    if (port_id == AT_NO_SERIAL_PORT_ID)
        return AIC_NULL;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        if (at_ports_info[i].at_com_id == port_id)
            return &at_ports_info[i];

        if (at_ports_info[i].at_com_id == AT_NO_SERIAL_PORT_ID) {
            at_ports_info[i].at_com_id = port_id;
            return &at_ports_info[i];
        }
    }

    return AIC_NULL;
}

u8 aic_get_at_port_index(u32 at_port_id)
{
    u8 i;

    for (i = 0; i < AT_MAX_SERIAL_PORT_NUM; i++) {
        if (at_port_id != AT_NO_SERIAL_PORT_ID &&
            at_ports_info[i].at_com_id == at_port_id)
            return i;
    }

    return AT_MAX_SERIAL_PORT_NUM;
}

void aic_diag_at_cmd_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    u8 *at_str, *tmp_ptr;
    ulong at_req_resp;
    s32 status;
    AT_PORT_INFO *at_port_info = AIC_NULL;

    /* Init thread of replying to diag port first. */
    at_port_info = aic_add_new_at_port(AT_DIAG_SERIAL_PORT_ID);
    if (at_port_info && !at_port_info->recv_com_added) {
        aic_add_to_multi_at_com_list(AT_DIAG_SERIAL_PORT_ID);
        at_port_info->recv_com_added = 1;
    }

    if (at_port_info) {
        aic_add_reply_to_com_list(AT_DIAG_SERIAL_PORT_ID, diag_cmd->sn, diag_cmd->sys);
        at_port_info->reply_com_added = 1;
    }

    at_str = aos_zalloc_relaxed(diag_cmd->data_len + 1);
    if (!at_str)
        return;
    strncpy(at_str, diag_cmd->data, diag_cmd->data_len);

    /* Only one AT cmd from DIAG port. */
    aicx_info(0x0a2f25, "s", "recv DIAG AT: %s\n", at_str);

    tmp_ptr = strchr(at_str, aic_get_cr_sign());
    if (!tmp_ptr)
        replace_at_cmd_ending_sign(at_str);

    status = aic_assemble_and_handle_at_cmd(at_str,
                                            &at_req_resp,
                                            AT_FROM_EXTERNAL,
                                            AT_DIAG_SERIAL_PORT_ID,
                                            AT_TIMIEOUT_NO_RETRY);
    if (!status) {
        ((AT_REQ_RESP *)at_req_resp)->diag_sn = diag_cmd->sn;
        ((AT_REQ_RESP *)at_req_resp)->diag_sys = diag_cmd->sys;
    }

    aos_free(at_str);
}

#ifndef CONFIG_AT_PROC_MINI
static boolean aic_is_complete_at(u8 *at_str, u32 at_len)
{
    if (!at_str)
        return AIC_FALSE;

    if (is_cr_lf_sign(*(at_str + at_len - 2)) || is_cr_lf_sign(*(at_str + at_len - 1)))
        return AIC_TRUE;

    return AIC_FALSE;
}

/**
 * When receive msg from serial ports,the function is called.
 * The received msg is AT cmd in AT_CMD_MODE;
 * the received msg is data payload in AT_DATA_MODE.
 */
void aic_serial_recv_callback(u32 conn_id, u8 *at_str, size_t len, void *p)
{
    s32 status;
    u8 at_mode = AT_CMD_MODE;
    /* Save the pointer. */
    AT_REQ_RESP *at_req_resp;
    SERIAL_DATA_MODE_AT *data_mode_at_ptr;
    u8 *at_cmd_ptr = AIC_NULL;
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;
    u8 *collected_at_ptr;
    AT_PORT_INFO *at_port_info = AIC_NULL;
    u32 at_com_id;

    if (!at_str || len == 0 || conn_id == AT_NO_SERIAL_PORT_ID) {
        aicx_error(0x0ebfd4, "u", "error AT port %ud or recv null string\n", conn_id);
        return;
    }

    if (conn_id == conn_get_usb_at_conid)
        at_com_id = AT_USB_AT_PORT_ID;
    else
        at_com_id = conn_id;

    at_port_info = aic_add_new_at_port(at_com_id);
    if (at_port_info && !at_port_info->recv_com_added) {
        aic_add_to_multi_at_com_list(at_com_id);
        at_port_info->recv_com_added = 1;
    }

    if (at_port_info && !at_port_info->reply_com_added) {
        aic_add_reply_to_com_list(at_com_id, 0, 0);
        at_port_info->reply_com_added = 1;
    }

    data_mode_at_ptr = aic_get_serial_data_mode_at(at_com_id);
    if (data_mode_at_ptr && data_mode_at_ptr->data_mode_at &&
        data_mode_at_ptr->data_mode_at->at_mode == AT_DATA_MODE)
        at_mode = AT_DATA_MODE;

    at_com_item = aic_find_at_com_item(at_com_id);
    if (!at_com_item)
        goto MEM_FREE;

    if (at_mode == AT_CMD_MODE) {
        /* Just a null character. */
        if (*at_str == 0x00)
            goto MEM_FREE;

        /* Pherhaps receive data payload in cmd mode,so make it a string. */
        at_cmd_ptr = (u8 *)aos_zalloc_relaxed(len + 1);
        if (!at_cmd_ptr)
            goto MEM_FREE;

        aos_memcpy(at_cmd_ptr, at_str, len);

        collected_at_ptr = at_com_item->collected_at_ptr;
        /* Check if it complete or part of the AT has been received. */
        if (!(collected_at_ptr && at_com_item->collected_len > 0) &&
            aic_is_complete_at(at_cmd_ptr, len)) {
            aicx_info(0x1de916, "s", "recv complete AT: %s\n", at_cmd_ptr);
            process_complete_at_cmd(at_cmd_ptr, len, at_com_id);
        } else {
            aicx_info(0x1b509a, "s", "recv part of the AT %s\n", at_cmd_ptr);

            if (!collected_at_ptr) {
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
                /* If recv data in cmd mode,then just discard it. */
                if (len >= 2 && strncasecmp(at_cmd_ptr, "AT", 2) != 0 &&
                    strncasecmp(at_str, "A/", 2) != 0) {
                    aos_free(at_cmd_ptr);
                    goto MEM_FREE;
                }
#endif
                collected_at_ptr = (u8 *)aos_zalloc_relaxed(AT_MAX_AP_CMD_LENGTH);
                if (!collected_at_ptr) {
                    aos_free(at_cmd_ptr);
                    goto MEM_FREE;
                }

                at_com_item->collected_at_ptr = collected_at_ptr;
                at_com_item->collected_len = 0;
            }

            status = collect_complete_at(at_cmd_ptr, len, at_com_item);
            /* Get a complete AT cmd. */
            if (!status) {
                aicx_info(0x05ea07, "s", "collect AT %s\n", collected_at_ptr);
                process_complete_at_cmd(collected_at_ptr, at_com_item->collected_len, at_com_id);

                /* Make it ready to to save next AT command. */
                aos_free(at_com_item->collected_at_ptr);
                at_com_item->collected_at_ptr = AIC_NULL;
                at_com_item->collected_len = 0;
            } else if (status == AT_ERROR_CMD_TOO_LONG) {
                reply_error_to_serial_port(at_com_id, (u32)status);
            }
        }

        aos_free(at_cmd_ptr);
    } else if (at_mode == AT_DATA_MODE) {
        at_req_resp = data_mode_at_ptr->data_mode_at;
        if (!at_req_resp || at_req_resp != at_com_item->serial_dispatch_at) {
            aicx_error(0x0ec1b8, "", "error,no data mode at in handle\n");
            goto MEM_FREE;
        }

        aicx_info(0x021ac2, "d", "recv data,len %d\n", len);
        aic_put_at_data_to_queue(at_com_item, at_str, len);
        aos_set_event(event_dispatch_at, at_com_item->dispatch_event_id, AOS_OR);
    }

MEM_FREE:
#ifndef WIN32
    if (!aic_is_cmux_com(at_com_id))
        aos_dma_free(at_str);
#else
    return;
#endif
}

static boolean is_cmd_with_data_para(u8 *at_str)
{
#ifdef CONFIG_CUSTOMER_CM_AT
    if (at_str && (strcasestr(at_str, "AT+MQTTCFG") || strcasestr(at_str, "AT+MQTTCONN") ||
        strcasestr(at_str, "AT+MQTTSUB") || strcasestr(at_str, "AT+MQTTPUB") ||
        strcasestr(at_str, "AT+MHTTPCFG") || strcasestr(at_str, "AT+MHTTPHEADER") ||
        strcasestr(at_str, "AT+MHTTPCONTENT") || strcasestr(at_str, "AT+MHTTPREQUEST") ||
        strcasestr(at_str, "AT+MIPSEND") || strcasestr(at_str, "AT+MIPLNOTIFY") ||
        strcasestr(at_str, "AT+MIPLREADRSP") || strcasestr(at_str, "AT+MIPLDISCOVERRSP") ||
        strcasestr(at_str, "AT+MFPUT") || strcasestr(at_str, "AT+MFWRITE")))
        return AIC_TRUE;
#endif

#ifdef CONFIG_GX_AT_V2
    if (at_str && (strcasestr(at_str, "AT+GMQTTCFG") || strcasestr(at_str, "AT+GMQTTCONN") ||
        strcasestr(at_str, "AT+GMQTTSUB") || strcasestr(at_str, "AT+GMQTTPUB") ||
        strcasestr(at_str, "AT+GHTTPCFG") || strcasestr(at_str, "AT+GHTTPHEADER") ||
        strcasestr(at_str, "AT+GHTTPCONTENT") || strcasestr(at_str, "AT+GHTTPREQUEST") ||
        strcasestr(at_str, "AT+GIPSEND") || strcasestr(at_str, "AT+GIPLNOTIFY") ||
        strcasestr(at_str, "AT+GIPLREADRSP") || strcasestr(at_str, "AT+GIPLDISCOVERRSP")))
        return AIC_TRUE;
#endif
#ifdef CONFIG_FS_AT
    if (at_str && (strcasestr(at_str, "AT+GFPUT") || strcasestr(at_str, "AT+GFWRITE")))
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

#ifdef CONFIG_D2D
static boolean aic_recv_msg_format_at(u8 *at_ptr, u32 at_cmd_len)
{
    if (at_cmd_len <= AT_MSG_HEAD_LEN)
        return AIC_FALSE;

    if ((*at_ptr == 'A' || *at_ptr == 'a') && (*(at_ptr + 1) == 'T' || *(at_ptr + 1) == 't') &&
        (*(at_ptr + 2) == '+' || *(at_ptr + 2) == '^'))
        return AIC_FALSE;

    return AIC_TRUE;
}
#endif

s32 process_complete_at_cmd(u8 *at_cmd_str, u32 at_len, u32 serial_port_id)
{
    ulong at_req_resp_ptr;
    s32 status = AIC_SUCCESS;
    u8 *tmp_ptr = at_cmd_str;
    u32 cmd_len = at_len;
#ifdef CONFIG_D2D
    AT_MSG_HEAD msg_head;
    boolean recv_msg_head = AIC_FALSE;
#endif

    if (!at_cmd_str)
        return AT_ERROR_INVALID_CMD;

#ifdef CONFIG_D2D
    aos_memset(&msg_head, 0, sizeof(AT_MSG_HEAD));
    recv_msg_head = aic_recv_msg_format_at(at_cmd_str, at_len);
    if (recv_msg_head) {
        msg_head.msg_type = *((u16 *)at_cmd_str);
        msg_head.cmd_len = *((u16 *)(at_cmd_str + 2));
        aos_memcpy((u8 *)&(msg_head.msg_id), (at_cmd_str + 4), 4);
        tmp_ptr = at_cmd_str + AT_MSG_HEAD_LEN;
        aicx_info(0x09f20a, "xdxs", "process at:msg type 0x%x,len %d,msg id 0x%x--%s\n",
                  msg_head.msg_type, msg_head.cmd_len, msg_head.msg_id, tmp_ptr);
    }
#endif

    /* If echo of AT cmds is enabled. */
    if (aic_is_at_echo_enabled())
        aic_reply_to_certain_com(serial_port_id, tmp_ptr, cmd_len);

    /* Skip the leading \r\n. */
    while ((*tmp_ptr == aic_get_cr_sign() || *tmp_ptr == aic_get_lf_sign()) &&
           *tmp_ptr != '\0') {
        tmp_ptr++;
    }
    if (*tmp_ptr == '\0')
        return AT_ERROR_INVALID_CMD;

    if (!repeat_last_at_cmd(tmp_ptr)) {
        if (last_at_str)
            aos_free(last_at_str);

        last_at_str = (u8 *)aos_zalloc_relaxed(cmd_len + 1);
        if (last_at_str)
            aos_memcpy(last_at_str, tmp_ptr, cmd_len);
    } else {
        if (!last_at_str) {
            reply_error_to_serial_port(serial_port_id, AT_ERROR_INVALID_CMD);
            return AT_ERROR_INVALID_CMD;
        }
        tmp_ptr = last_at_str;
    }

    /* Theres is a semicolon at the end for voice call. */
    if (!strchr(tmp_ptr, ';') || (strchr(tmp_ptr, ';') &&
        (strncasecmp(tmp_ptr, "ATD", 3) == 0 || is_cmd_with_data_para(tmp_ptr)))) {
#ifdef CONFIG_D2D
        if (recv_msg_head)
            status = aic_assemble_at_cmd_with_head(tmp_ptr, &msg_head,
                                                  &at_req_resp_ptr,
                                                  AT_FROM_EXTERNAL,
                                                  serial_port_id,
                                                  AT_TIMIEOUT_NO_RETRY);
        else
#endif
        status = aic_assemble_and_handle_at_cmd(tmp_ptr,
                                                &at_req_resp_ptr,
                                                AT_FROM_EXTERNAL,
                                                serial_port_id,
                                                AT_TIMIEOUT_NO_RETRY);
    } else {
        /* Several AT cmds to seperate. */
        extract_single_at_cmd(tmp_ptr, serial_port_id);
    }

    return status;
}

boolean repeat_last_at_cmd(u8 *at_str)
{
    if (aic_is_valid_at_cmd(at_str) && (strncasecmp(at_str, "A/", 2) == 0) &&
        is_cr_lf_sign(*(at_str + 2)))
        return AIC_TRUE;

    return AIC_FALSE;
}

void reply_error_to_serial_port(u32 serial_port_id, u32 err_code)
{
    u8 reply_msg[40] = {0};
    u8 error_str[30] = {0};

    snprintf(error_str, (sizeof(error_str) - 1),
             AT_RESULT_CME_ERROR"%d", err_code);
    fill_head_and_end_signs(reply_msg, error_str, strlen(error_str));
    aic_reply_to_certain_com(serial_port_id, reply_msg, strlen(reply_msg));
}

u32 aic_get_at_port_id(void)
{
    return at_ports_info[0].at_com_id;
}

/* Collect the complete at cmd util the ending signs. */
u32 collect_complete_at(u8 *char_recved, u32 recved_len, AT_RECV_COM_ITEM *at_com_item)
{
    if (!at_com_item || !at_com_item->collected_at_ptr)
        return AT_ERROR_INVALID_AT_PTR;

    if (recved_len < (AT_MAX_AP_CMD_LENGTH - at_com_item->collected_len)) {
        aos_memcpy((at_com_item->collected_at_ptr + at_com_item->collected_len),
                   char_recved, recved_len);
        at_com_item->collected_len += recved_len;
    } else {
        aicx_error(0x0b2150, "s", "error,AT cmd is too long %s\n", at_com_item->collected_at_ptr);
        aos_memset(at_com_item->collected_at_ptr, '\0', AT_MAX_AP_CMD_LENGTH);
        at_com_item->collected_len = 0;
        return AT_ERROR_CMD_TOO_LONG;
    }

    if (aic_is_complete_at(at_com_item->collected_at_ptr, at_com_item->collected_len))
        return AIC_SUCCESS;

    return AT_ERROR_INCOMPLETE_CMD;
}

/* Ectract one AT cmd from several cmds splitted by ";". */
s32 extract_single_at_cmd(u8 *raw_str, u32 serial_channel_id)
{
    u8 *start_ptr, *tmp_ptr, *actual_at_str = AIC_NULL;
    ulong at_req_resp_ptr;
    s32 status = AIC_SUCCESS;
    char *save_ptr;

    start_ptr = raw_str;
    /* Delete leading spaces if there are. */
    while (*start_ptr == ' ') {
        start_ptr++;
    }

    tmp_ptr = strtok_r(start_ptr, ";", &save_ptr);
    while (tmp_ptr) {
        /* The cmds appending the first one may ignore leading AT characters. */
        if (strncasecmp(tmp_ptr, "AT", 2) != 0) {
            actual_at_str = aos_zalloc_relaxed(strlen(tmp_ptr) + 3);
            if (!actual_at_str) {
                tmp_ptr = strtok_r(AIC_NULL, ";", &save_ptr);
                continue;
            }

            strcpy(actual_at_str, "AT");
            strncat(actual_at_str, tmp_ptr, strlen(tmp_ptr));
            tmp_ptr = actual_at_str;
        }

        status = aic_assemble_and_handle_at_cmd(tmp_ptr,
                                                &at_req_resp_ptr,
                                                AT_FROM_EXTERNAL,
                                                serial_channel_id,
                                                AT_TIMIEOUT_NO_RETRY);

        if (actual_at_str) {
            aos_free(actual_at_str);
            actual_at_str = AIC_NULL;
        }

        tmp_ptr = strtok_r(AIC_NULL, ";", &save_ptr);
    }

    return status;
}
#endif

void replace_at_cmd_ending_sign(u8 *at_cmd_str)
{
    u8 *tmp_ptr;

    tmp_ptr = at_cmd_str;
    while (*tmp_ptr != '\0') {
        if (*tmp_ptr == 0x0D || *tmp_ptr == 0x0A) {
            *tmp_ptr = aic_get_cr_sign();
            *(tmp_ptr + 1) = '\0';
            break;
        }
        tmp_ptr++;
    }
}

#ifdef WIN32
extern aos_event_ptr reply_to_com_event;

void AIC_SEND_AT(u8 *at_cmd_str)
{
    size_t len = strlen(at_cmd_str);
    u8 *at_str_ptr = aos_zalloc(len + 3);

    aicx_info(0x12f78c, "s", "[AT_SEND]: [%s]\n", at_cmd_str);
    aos_memcpy(at_str_ptr, at_cmd_str, len);

    /* add \r\n */
    *(at_str_ptr + len)='\r';
    *(at_str_ptr + len + 1)='\n';

    aic_serial_recv_callback(0, at_str_ptr, len + 2, NULL);
    aos_free(at_str_ptr);
}

aos_status AIC_RECV_AT(u8 *at_cnf_str)
{
    u32 events, ret = AIC_SUCCESS;
    AT_REPLY_TO_COM *reply_to_com = AIC_NULL;
    REPLY_TO_COM_ITEM *reply_item = AIC_NULL;
    u8* expect_data = NULL;
    u16 str_len;

    aicx_info(0x1bdef6, "s", "[AT_RECV]: expect result [%s]\n", at_cnf_str);

    reply_to_com = aic_get_reply_to_com_ptr(0);
    if (!reply_to_com)
    {
        return AOS_ERROR;
    }

    str_len = strlen(at_cnf_str);
    expect_data = aos_zalloc(str_len + 5);

    /* add \r\n */
    expect_data[0] = 0x0d;
    expect_data[1] = 0x0a;
    aos_memcpy(expect_data + 2, at_cnf_str, str_len);
    expect_data[str_len + 2] = 0x0d;
    expect_data[str_len + 3] = 0x0a;

    while(1)
    {
        aos_get_event(reply_to_com_event, reply_to_com->event_id, AOS_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        /* check event */
        if (!(events & reply_to_com->event_id))
        {
            aos_sleep(20);
            continue;
        }

        do
        {
            aos_get_mutex(reply_to_com->send_mutex, AOS_WAIT_FOREVER);
            ret = aos_recv_from_queue(reply_to_com->reply_queue,
                                      (void*)&reply_item, AOS_NO_WAIT);
            aos_put_mutex(reply_to_com->send_mutex);

            /* check queue */
            if (ret != AIC_SUCCESS || !reply_item)
            {
                aos_sleep(20);
                break;
            }

            aicx_info(0x019280, "s", "recv reply str %s\n", reply_item->reply_ptr);
            if(!strcmp(expect_data,reply_item->reply_ptr))
            {
                aos_dma_free(reply_item->reply_ptr);
                aos_free(reply_item);
                aos_free(expect_data);
                return AOS_SUCCESS;
            }

            aos_dma_free(reply_item->reply_ptr);
            aos_free(reply_item);

            reply_item = 0;
        }while(AIC_SUCCESS == ret);
    }

    aos_free(expect_data);
    return AOS_SUCCESS;
}

#endif
