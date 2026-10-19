/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  Netx DHCP Server Test Component                                      */
/**                                                                       */
/**  This is a small demo of DHCP Server. Start and configure DHCP Server */
/**  to assign IPv4 address and DNS server address etc. for PC via USB.   */
/**  After the addresses are assigned, they can transmit data packets.    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x157ec6
#define LOG_MODULE    AMOD_AT

#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_size.h>
#include <aic_errno.h>
#include <connmgr_api.h>
#include "aic_at_cmd_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_net_at_resp_handle.h"
#include "aic_at_interact_cp.h"
#include "aic_serial_recv_send.h"
#include "aic_at_parse_utils.h"

/* Define internal functions. */
static s32 get_resp_handle_index(AT_REQ_RESP *at_req_resp_ptr);

static AT_RESP_FROM_CP_HANDLE *at_resp_handle_list[AT_MAX_RESP_HANDLES] = {0};

/* Define the internal vars. */
static AT_RESP_FROM_CP_HANDLE general_at_resp_handles[] =
{
    {"AT+COPS", AIC_NULL, aic_handle_cops_query_resp,
     AIC_NULL, AIC_NULL, AIC_NULL},

    {"AT+CFUN", AIC_NULL, AIC_NULL, AIC_NULL, aic_handle_cfun_exe_resp, AIC_NULL},

#ifdef CONFIG_CUSTOMER_CM_AT
    {"AT+CEREG", AIC_NULL, aic_handle_cereg_query_resp,
     aic_handle_cereg_set_resp, AIC_NULL, AIC_NULL},

    {"AT+CGEREP", AIC_NULL, aic_handle_cgerep_query_resp,
     aic_handle_cgerep_set_resp, AIC_NULL, AIC_NULL},

    {"AT+CTZR", AIC_NULL, aic_handle_ctzr_query_resp,
     aic_handle_ctzr_set_resp, AIC_NULL, AIC_NULL},
#endif

    {AIC_NULL},
};

/** If the AT cmds is sent to CP,and AP needs to handle the response
 * received from CP.Then put the handle functions in at_resp_handle_list.
 * And get the response handling index from at_resp_handle_list.
 */
s32 get_resp_handle_index(AT_REQ_RESP *at_req_resp_ptr)
{
    s32 i = 0;
    s32 index = AT_NO_RESP_HANDLE_INDEX;
    AT_RESP_FROM_CP_HANDLE *resp_handle_ptr;
    u8 at_cmd_head[AT_MAX_HEAD_LEN] = {'\0'};
    u8 len1, len2, cmp_len;

    get_at_head_str(at_req_resp_ptr->at_req, at_cmd_head);

    for (i = 0; i < AT_MAX_RESP_HANDLES; i++) {
        resp_handle_ptr = at_resp_handle_list[i];

        if (resp_handle_ptr) {
            len1 = strlen(at_cmd_head);
            len2 = strlen(resp_handle_ptr->at_cmd);
            cmp_len = len1 >= len2 ? len1 : len2;
            if (!strncasecmp(at_cmd_head, resp_handle_ptr->at_cmd, cmp_len))
                return i;
        } else {
            /* No handle remains. */
            break;
        }
    }

    return index;
}

u32 aic_set_resp_handler(AT_REQ_RESP *at_req_resp)
{
    s32 index = -1;
    u8 cmd_type;

    index = get_resp_handle_index(at_req_resp);
    if (index == AT_NO_RESP_HANDLE_INDEX) {
        at_req_resp->at_resp_handler = AIC_NULL;
        return AIC_SUCCESS;
    }

    /* There are handlers in at_resp_handle_list. */
    cmd_type = at_req_resp->at_cmd_type;
    /* Query cmd. */
    if (cmd_type == AT_CMD_TYPE_QUERY) {
        at_req_resp->at_resp_handler =
            at_resp_handle_list[index]->at_query_resp_handler;
    } else if (cmd_type == AT_CMD_TYPE_TEST) {
        /* Test cmd. */
        at_req_resp->at_resp_handler =
            at_resp_handle_list[index]->at_test_resp_handler;
    } else {
        /* Set or exe cmd. */
        if (at_resp_handle_list[index]->at_set_resp_handler) {
            at_req_resp->at_resp_handler =
                at_resp_handle_list[index]->at_set_resp_handler;
        } else if (at_resp_handle_list[index]->at_exe_resp_handler) {
            at_req_resp->at_resp_handler =
                at_resp_handle_list[index]->at_exe_resp_handler;
        } else {
            at_req_resp->at_resp_handler = AIC_NULL;
        }
    }

    return AIC_SUCCESS;
}

u32 aic_set_timeout_handler(AT_REQ_RESP *at_req_resp)
{
    s32 index = -1;

    /* AP not care the response. */
    index = get_resp_handle_index(at_req_resp);
    if (index == AT_NO_RESP_HANDLE_INDEX) {
        if (at_req_resp->at_req)
        /* The default timeout handler. */
        at_req_resp->at_timeout_handler = AIC_NULL;
    } else {
        at_req_resp->at_timeout_handler =
            at_resp_handle_list[index]->at_resp_timeout_handler;
    }

    return AIC_SUCCESS;
}

/**
 * aic_add_at_resp_handle - Add AT_RESP_FROM_CP_HANDLE for processing
 * response received from CP.
 *
 * @head_handle_ptr: The pointer to the first resp handle.
 *
 * If the response received from CP needed to be parsed and handled,
 * then their resp handles should be added to at_resp_handle_list
 * by the function.
 *
 * @Return SUCCESS if the resp handles are successfully added to
 *  at_resp_handle_list. Return ERROR if at_resp_handle_list is full.
 */
u32 aic_add_at_resp_handle(AT_RESP_FROM_CP_HANDLE *head_handle_ptr)
{
    AT_RESP_FROM_CP_HANDLE *tmp_handle_ptr;
    u16 i, idle_index = 0;

    if (!head_handle_ptr)
        return AT_ERROR_INVALID_AT_HANDLE;

    /* Find the idle index in the array. */
    for (i = 0; i < AT_MAX_RESP_HANDLES; i++) {
        if (at_resp_handle_list[i] == 0) {
            idle_index = i;
            break;
        }
    }
    if (i == AT_MAX_RESP_HANDLES)
        goto HANDLE_ERROR;

    tmp_handle_ptr = head_handle_ptr;
    while (tmp_handle_ptr->at_cmd != AIC_NULL && idle_index < AT_MAX_RESP_HANDLES) {
        at_resp_handle_list[idle_index] = tmp_handle_ptr;
        tmp_handle_ptr++;
        idle_index++;
    }
    if (idle_index == AT_MAX_RESP_HANDLES && tmp_handle_ptr->at_cmd != AIC_NULL)
        goto HANDLE_ERROR;

    return AIC_SUCCESS;

HANDLE_ERROR:
    aicx_error(0x05582c, "", "error,too many AT resp handles\n");
    return AT_ERROR_AT_RESP_HANDLES_OVERFLOW;
}

void aic_net_at_resp_init(void)
{
    aic_add_at_resp_handle(&general_at_resp_handles[0]);
}

