/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Interacting with CP for AT cmds and Responses
 *  This is the component of sending AT cmds and receiving responses,URC
 *  from CP,via abuf link.Also,it would check if the response is complete
 *  for one AT cmd.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x043a85
#define LOG_MODULE    AMOD_AT

#include <aic_init.h>
#include <aic_type.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <aic_sys.h>
#include <subsys_api.h>
#include <aipc_api.h>
#include <connmgr_api.h>
#include <aic_boot_mode.h>
#include <slp_inc.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <sysoff.h>
#include <gpio_wakeup.h>
#include "aic_at_interact_cp.h"
#include "aic_serial_recv_send.h"
#include "aic_at_cmd_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_at_parse_utils.h"
#include "aic_at_urc_handle.h"
#include "aic_net_cfg.h"
#include "aic_general_at_handle.h"
#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif
#ifdef CONFIG_PDP_MGR
#include "aic_net_pdp_mgr.h"
#endif
#ifdef CONFIG_CMUX
#include <cmux.h>
#endif

/* Define internal vars. */
static aos_thread_id thread_send_at_to_cp;
static aos_timer_ptr at_common_timer;

static ABRR_HANDLE abuf_handle;
static subsys_state_t current_cp_state;
static aic_get_cp_urc_callback cp_urc_cbs[AT_MAX_CP_URC_CBS];
static boolean reply_to_com_inited = AIC_FALSE;
#ifdef CONFIG_CMUX
static CMUX_PORT_CFG cmux_port_cfg;
#endif

/* Define internal functions. */
static void at_send_to_cp_entry(void *thread_input);
static void at_rev_from_cp_entry(void *thread_input);
static u32 check_at_recv_msg(AT_ABUF_LINK_CFG *abuf_link_cfg, u32 msg_len);
static u8 get_idle_link(void);
static boolean is_valid_abuf_link(u8 link_id);
static u32 write_at_to_mbuf(AT_REQ_RESP *at_req_resp, u8 link_id);
static void process_at_resp_received(AT_REQ_RESP *at_ptr);
static u8 get_ready_abuf_link(AT_REQ_RESP *at_req_resp_ptr);
static u8 get_cid_from_cgdata(u8 *cgdata_cmd);
static void settle_internal_timeout_at(AT_REQ_RESP *at_req_resp);
static void settle_ext_at_timeout(AT_REQ_RESP *at_req_resp);
static void transmit_cp_urc_to_serial_ports(u8 *urc_ptr, u16 urc_len);
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
static void subsys_state_change_callback(subsys_state_t cp_state, void *p);
#endif
static s32 enable_cereg_urc(u8 cereg_mode, aic_get_cp_resp_callback cb);
static void block_sending_at_to_cp(AT_REQ_RESP *at_req_resp, u32 error_num);
static boolean is_msg_format_correct(AT_ABUF_LINK_CFG *abuf_link_cfg, s32 msg_len);
static AT_ABUF_LINK_CFG *get_abuf_cfg_by_link(u8 abuf_link_id);
static boolean at_timer_is_used(void);
static void init_send_reply_to_ucom(void);
static void send_to_diag_entry(void *thread_input);
static void send_to_com_entry(void *thread_input);
static boolean is_at_com_ppp_mode(u32 com_id);
static void invoke_cp_urc_cbs(u8 *urc_str);
#ifdef CONFIG_AIC_SYSOFF
static void wait_ps_stop(void *ptr);
#endif
static void process_medium_resp_recved(AT_REQ_RESP *at_ptr);

/* Define the global vars. */
AT_REQ_RESP *at_pending_to_send = AIC_NULL;
aos_mutex_ptr mutex_at_pending;
aos_event_ptr event_send_at;
AT_ABUF_LINK_CFG_SET abuf_link_cfg_set;
aos_event_ptr reply_to_com_event;
INIT_NET_CFG init_net_cfg;
struct list_head reply_to_com_list;
aic_flight_mode_change_cb flight_mode_cbs[AT_MAX_FLIGHT_MODE_CBS];
aos_thread_id thread_at_rev_from_cp;

/* Define external vars. */
extern boolean multi_at_com_inited;
extern struct list_head multi_at_com_list;
extern AT_PORT_INFO at_ports_info[AT_MAX_SERIAL_PORT_NUM];
extern REG_SERVICE_INFO reg_service_info;
extern s32 current_time_zone;
#ifdef CONFIG_WAKEUP_SAMPLE
extern bool wakeup_is_mcu_sleeping(void);
extern void wakeup_sample_enable_mcu_sleep(void);
extern boolean wakeup_sample_awaken_mcu(void);
extern boolean wakeup_sample_is_urc_need_wakeup(u8 *urc_str);
#endif
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
extern boolean aic_is_transparent_mode(void);
#endif
#ifdef CONFIG_D2D
extern D2D_MODE_CFG d2d_mode_cfg;
extern AT_D2D_URC_MSG_INFO d2d_urc_msg_info[];
#endif

/* Define external functions. */
//extern void test_net_share_entry(void *thread_input);

/* Init all the abuf links,the three threads and events. */
u32 aic_at_interact_cp_init(void)
{
    u8 i;
    u8 mutex_name[30] = {'\0'};
    AT_ABUF_LINK_CFG *abuf_link_cfg;

#ifdef CONFIG_D2D
    aic_set_urc_msg_head_enabled(AT_MSG_HEAD_ENABLED);
    aic_add_urc_msg_info(&d2d_urc_msg_info[0]);
#ifdef CONFIG_CMUX
    strcpy(d2d_mode_cfg.at_com_type, "cmux");
    aic_set_d2d_urc_com_id(AT_D2D_URC_CMUX_COM_ID);
#else
    if (conn_get_uart_at_conid != AIC_ENODEV) {
        aic_set_d2d_urc_com_id(conn_get_uart_at_conid);
        strcpy(d2d_mode_cfg.at_com_type, "uart");
    }
#endif
#endif
    abuf_handle = abuf_request_brr(AIC_SYS_CP, AIPC_BRR_AT);
    if (!abuf_handle)
        return AT_ERROR_NO_ABUF_HANDLE;

    for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
        abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
        aos_memset(abuf_link_cfg, 0, sizeof(AT_ABUF_LINK_CFG));
        abuf_link_cfg->link_id = AT_LINK0 + i;
        abuf_link_cfg->link_state = AT_ABUF_LINK_IDLE;
        abuf_link_cfg_set.link_set |= BIT(AT_LINK0 + i);

        snprintf(mutex_name, sizeof(mutex_name), "abuflink_%d", i);
        abuf_link_cfg->abuf_link_mutex = aos_create_mutex("abuflink");
        AIC_ASSERT(abuf_link_cfg->abuf_link_mutex);
    }

    abuf_link_cfg_set.abuf_link_recv_event = aos_create_event("abuf_recv");
    AIC_ASSERT(abuf_link_cfg_set.abuf_link_recv_event);

    event_send_at = aos_create_event("send_at");
    AIC_ASSERT(event_send_at);

    mutex_at_pending = aos_create_mutex("at_pend");
    AIC_ASSERT(mutex_at_pending);

    /* Creat thread to send AT cmds to CP. */
    thread_send_at_to_cp = aos_create_thread("send_to_cp",
                                             at_send_to_cp_entry,
                                             NULL, SIZE_2K,
                                             0, NULL, 11,
                                             AOS_AUTO_START);
    if (thread_send_at_to_cp == AOS_INVALID_THREAD_ID)
        return AOS_INVALID_THREAD_ID;

    /* Creat thread to receive AT resp and urc from CP. */
    thread_at_rev_from_cp = aos_create_thread("rev_from_cp",
                                              at_rev_from_cp_entry,
                                              NULL, SIZE_2K,
                                              0, NULL, AT_REV_FROM_CP_TASK_PRIO,
                                              AOS_AUTO_START);
    if (thread_at_rev_from_cp == AOS_INVALID_THREAD_ID)
        return AOS_INVALID_THREAD_ID;

    init_send_reply_to_ucom();
#ifdef CONFIG_AIC_SYSOFF
    sysoff_register_hook(wait_ps_stop, AIC_NULL);
#endif
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    subsys_add_state_notify(AIC_SYS_CP, subsys_state_change_callback, AIC_NULL);
#endif

    return AIC_SUCCESS;
}

boolean aic_is_uart_com(u32 com_id)
{
    if (com_id == conn_get_uart_at_conid)
        return AIC_TRUE;

    return AIC_FALSE;
}

/* to decide if it is cmux com if cmux is enabled. */
boolean aic_is_cmux_com(u32 com_id)
{
#ifdef CONFIG_CMUX
    u8 i;

    for (i = 0; i < cmux_port_cfg.port_cnt; i++) {
        if (cmux_port_cfg.port_ids && cmux_port_cfg.port_ids[i] == com_id)
            return AIC_TRUE;
    }
#endif

    return AIC_FALSE;
}

static void add_and_init_reply_com(u32 at_port_id)
{
    AT_PORT_INFO *at_port_info = AIC_NULL;

    at_port_info = aic_add_new_at_port(at_port_id);
    if (at_port_info && !at_port_info->reply_com_added) {
        aic_add_reply_to_com_list(at_port_id, 0, 0);
        at_port_info->reply_com_added = 1;
    }
}

/* Should be inited first to send URC. */
void init_send_reply_to_ucom(void)
{
#ifdef CONFIG_CMUX
    u8 cmux_port_cnt, i;
    cmux_dev_num_t cmux_ports;
#endif

    reply_to_com_event = aos_create_event("reply_to_com");
    AIC_ASSERT(reply_to_com_event);

    if (conn_get_usb_at_conid != AIC_ENODEV)
        add_and_init_reply_com(AT_USB_AT_PORT_ID);

    if (conn_get_uart_at_conid != AIC_ENODEV)
        add_and_init_reply_com(conn_get_uart_at_conid);

#ifdef CONFIG_CMUX
    /* All cmux coms use the same reply_to_com. */
    cmux_ports = cmux_get_at_sum_conid();
    cmux_port_cnt = cmux_ports.dev_sum_num;
    if (cmux_port_cnt > 0) {
        cmux_port_cfg.port_cnt = cmux_port_cnt;
        cmux_port_cfg.port_ids = (u32 *)aos_zalloc_relaxed(cmux_port_cnt * sizeof(u32));
        if (!cmux_port_cfg.port_ids)
            return;

        for (i = 0; i < cmux_port_cnt; i++) {
            *(cmux_port_cfg.port_ids + i) = cmux_ports.dev_index[i];
            aicx_info(0x03fcc9, "d", "cmux port id %d\n", *(cmux_port_cfg.port_ids + i));
            add_and_init_reply_com(cmux_port_cfg.port_ids[i]);
        }
    }
#endif
}

boolean reply_to_com_added(u32 port_id)
{
    AT_REPLY_TO_COM *reply_to_com;
    AT_REPLY_TO_COM *tmp_reply_ptr;

    if (!reply_to_com_inited)
        return AIC_FALSE;

    list_for_each_entry_safe(reply_to_com, tmp_reply_ptr, &reply_to_com_list,
                             list_key, AT_REPLY_TO_COM) {
        /* All cmux coms use the same AT_REPLY_TO_COM. */
        if (aic_is_cmux_com(port_id) && aic_is_cmux_com(reply_to_com->com_id))
            return AIC_TRUE;

        if (reply_to_com->com_id == port_id)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

/* Add list for processing at cmds received from several ports concurrently. */
void aic_add_reply_to_com_list(u32 at_com_id, u32 diag_sn, u8 diag_sys)
{
    AT_REPLY_TO_COM *reply_to_com;
    u8 at_port_index = 0, task_priority;
    u32 queue_cnt;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (!reply_to_com_inited) {
        list_head_init(&reply_to_com_list);
        reply_to_com_inited = AIC_TRUE;
    }

    /* Create only one task to reply to upper device for all cmux coms. */
    if (reply_to_com_added(at_com_id)) {
        if (at_com_id == AT_DIAG_SERIAL_PORT_ID) {
            reply_to_com = aic_get_reply_to_com_ptr(AT_DIAG_SERIAL_PORT_ID);
            reply_to_com->diag_sn = diag_sn;
            reply_to_com->diag_sys = diag_sys;
        }
        AOS_INTERRUPT_RESTORE
        return;
    }

    aicx_info(0x1c7371, "d", "add to reply com list for com %d\n", at_com_id);
    reply_to_com = aos_zalloc_relaxed(sizeof(AT_REPLY_TO_COM));
    if (!reply_to_com) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    reply_to_com->com_id = at_com_id;
    if (at_com_id == AT_DIAG_SERIAL_PORT_ID) {
        reply_to_com->diag_sn = diag_sn;
        reply_to_com->diag_sys = diag_sys;
    }

    at_port_index = aic_get_at_port_index(at_com_id);
    reply_to_com->event_id = BIT(at_port_index);

    if (at_com_id == conn_get_uart_at_conid || aic_is_cmux_com(at_com_id))
        queue_cnt = AT_REPLY_UART_CACHE_COUNT;
    else
        queue_cnt = AT_REPLY_USB_CACHE_COUNT;

    reply_to_com->reply_queue =
        aos_create_queue("reply_com", sizeof(void*), queue_cnt);
    AIC_ASSERT(reply_to_com->reply_queue);

    reply_to_com->send_mutex = aos_create_mutex("reply_com");
    AIC_ASSERT(reply_to_com->send_mutex);

#ifndef WIN32
    if (at_com_id == AT_DIAG_SERIAL_PORT_ID) {
        strcpy(reply_to_com->thread_name, "reply_diag");
        reply_to_com->reply_thread = aos_create_thread(reply_to_com->thread_name,
                                                       send_to_diag_entry,
                                                       reply_to_com, SIZE_1K, 0,
                                                       NULL, 15, AOS_AUTO_START);
    } else {
        snprintf(reply_to_com->thread_name, sizeof(reply_to_com->thread_name),
                 "reply_com_%d", at_com_id);
        if (at_com_id == conn_get_uart_at_conid || aic_is_cmux_com(at_com_id))
            task_priority = AT_SEND_TO_UART_TASK_PRIO;
        else
            task_priority = AT_SEND_TO_USB_TASK_PRIO;

        reply_to_com->reply_thread = aos_create_thread(reply_to_com->thread_name,
                                     send_to_com_entry,
                                     reply_to_com,
                                     SIZE_2K, 0, NULL,
                                     task_priority,
                                     AOS_AUTO_START);
    }

    AIC_ASSERT(reply_to_com->reply_thread);
#endif

    list_add_tail(&reply_to_com->list_key, &reply_to_com_list);
    AOS_INTERRUPT_RESTORE
}

AT_REPLY_TO_COM *aic_get_reply_to_com_ptr(u32 at_port_id)
{
    AT_REPLY_TO_COM *reply_to_com;
    AT_REPLY_TO_COM *tmp_reply_ptr;
    u32 reply_com_id;

    if (!reply_to_com_inited)
        return AIC_NULL;

    list_for_each_entry_safe(reply_to_com, tmp_reply_ptr, &reply_to_com_list,
                             list_key, AT_REPLY_TO_COM) {
        reply_com_id = reply_to_com->com_id;
        if ((aic_is_cmux_com(at_port_id) && aic_is_cmux_com (reply_com_id)) || reply_com_id == at_port_id)
            return reply_to_com;
    }

    return AIC_NULL;
}

/* Transmit the AT response and URC to upper devices. */
u32 aic_transmit_reply_to_external(AT_REQ_RESP *at_req_resp)
{
    AT_REPLY_TO_COM *resp_to_com = AIC_NULL;
    u32 len;
#ifdef CONFIG_D2D
    u8 *resave_ptr;
#endif

    resp_to_com = aic_get_reply_to_com_ptr(at_req_resp->serial_port_id);
    if (!resp_to_com)
        return AT_ERROR_NO_SERIAL_CHANNEL;

    if (at_req_resp->current_resp_len == 0)
        len = strlen(at_req_resp->at_resp);
    else
        len = at_req_resp->current_resp_len;

#ifdef CONFIG_D2D
    if (aic_at_with_msg_head(at_req_resp)) {
        resave_ptr = aos_zalloc_relaxed(len + AT_MSG_HEAD_LEN);
        if (!resave_ptr)
            goto PUT_TO_QUEUE;

        aic_at_prepend_msg_head(resave_ptr, at_req_resp, len);
        aos_memcpy((resave_ptr + AT_MSG_HEAD_LEN), at_req_resp->at_resp, len);
        aos_free(at_req_resp->at_resp);
        at_req_resp->at_resp = resave_ptr;
        len += AT_MSG_HEAD_LEN;
    }

PUT_TO_QUEUE:
#endif
    aic_put_reply_to_queue(at_req_resp->at_resp, len,
                           at_req_resp->serial_port_id,
                           AIC_TRUE, AIC_FALSE);
    aos_set_event(reply_to_com_event, resp_to_com->event_id, AOS_OR);

    return AIC_SUCCESS;
}

s32 aic_reply_to_uart_with_wakeup(u32 com_id, u8 *reply_str, u16 reply_size)
{
    AT_REPLY_TO_COM *resp_to_com = AIC_NULL;

    resp_to_com = aic_get_reply_to_com_ptr(com_id);
    if (!resp_to_com)
        return AT_ERROR_NO_SERIAL_CHANNEL;

    aic_put_reply_to_queue(reply_str, reply_size, com_id, AIC_TRUE, AIC_TRUE);
    aos_set_event(reply_to_com_event, resp_to_com->event_id, AOS_OR);

    return AIC_SUCCESS;
}

s32 aic_reply_to_certain_com(u32 com_id, u8 *reply_str, u32 reply_size)
{
    AT_REPLY_TO_COM *resp_to_com = AIC_NULL;

    resp_to_com = aic_get_reply_to_com_ptr(com_id);
    if (!resp_to_com)
        return AT_ERROR_NO_SERIAL_CHANNEL;

    aic_put_reply_to_queue(reply_str, reply_size, com_id, AIC_TRUE, AIC_FALSE);
    aos_set_event(reply_to_com_event, resp_to_com->event_id, AOS_OR);

    return AIC_SUCCESS;
}

void aic_put_reply_to_queue(u8 *resp_str, u32 len, u32 com_id,
                            boolean to_copy, boolean need_wakeup)
{
    u8 *queue_resp = AIC_NULL;
    u32 ret;
    REPLY_TO_COM_ITEM *old_item = AIC_NULL;
    aos_queue_ptr queue_ptr;
    REPLY_TO_COM_ITEM *reply_item;
    AT_REPLY_TO_COM *to_com_ptr = AIC_NULL;

    to_com_ptr = aic_get_reply_to_com_ptr(com_id);
    if (!to_com_ptr)
        return;

    queue_ptr = to_com_ptr->reply_queue;
    if (!resp_str || !queue_ptr)
        return;

    reply_item = aos_zalloc_relaxed(sizeof(REPLY_TO_COM_ITEM));
    if (!reply_item)
        return;
    reply_item->reply_len = len;
    reply_item->need_wakeup = need_wakeup;
    reply_item->at_com_id = com_id;

    if (to_copy) {
        queue_resp = aos_dma_malloc(len + 1);
        if (!queue_resp)
            return;
        aos_memset(queue_resp, 0, (len + 1));
        aos_memcpy(queue_resp, resp_str, len);
        reply_item->reply_ptr = queue_resp;
    } else {
        reply_item->reply_ptr = resp_str;
    }

    aos_get_mutex(to_com_ptr->send_mutex, AOS_WAIT_FOREVER);
    ret = aos_send_to_queue(queue_ptr, (void *)&reply_item, AOS_NO_WAIT);
    if (ret != AIC_SUCCESS) {
        /* queue is full,get and free an old response,and send again. */
        ret = aos_recv_from_queue(queue_ptr, (void*)&old_item, AOS_NO_WAIT);
        if (ret == AIC_SUCCESS) {
            ret = aos_send_to_queue(queue_ptr, (void *)&reply_item, AOS_NO_WAIT);
            if (ret)
                aicx_error(0x1072d3, "", "send reply queue error\n");
            if (old_item) {
                aos_dma_free(old_item->reply_ptr);
                aos_free(old_item);
            }
        }
    }
    aos_put_mutex(to_com_ptr->send_mutex);
}

boolean is_at_com_ppp_mode(u32 com_id)
{
    if(aic_boot_mode_is_basic())
        return AIC_FALSE;

#ifdef CONFIG_PPP_DIAL
    return aic_is_ppp_dial_data_mode(com_id);
#endif

    return AIC_FALSE;
}

void aic_send_to_at_port(u32 com_id, u8 *reply_str, u32 len)
{
    u16 count;
    u8 port_name[8] = {'\0'};
    boolean is_cmux_com = AIC_FALSE;
    u8 reply_buf[70] = {0};
    u32 info_len;

    if (reply_str) {
        if (com_id != AT_NO_SERIAL_PORT_ID && !is_at_com_ppp_mode(com_id)) {
            is_cmux_com = aic_is_cmux_com(com_id);
            if (com_id == conn_get_uart_at_conid)
                strcpy(port_name, "uart");
            else if (com_id == AT_USB_AT_PORT_ID)
                strcpy(port_name, "usb");
            else if (is_cmux_com)
                strcpy(port_name, "cmux");

            /* Print at most 70 bytes. */
            info_len = len < sizeof(reply_buf) ? len : (sizeof(reply_buf) - 1);
            aos_memcpy(reply_buf, reply_str, info_len);

#ifdef CONFIG_D2D
            if (!is_cr_lf_sign(reply_buf[0]) && !is_cr_lf_sign(reply_buf[1]))
                aicx_info(0x0fccdc, "sddxxxxxxxxs", "send to %s AT port %d,len %d,msg head 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x--%s\n",
                          port_name, com_id, len, reply_buf[0], reply_buf[1], reply_buf[2], reply_buf[3],
                          reply_buf[4], reply_buf[5], reply_buf[6], reply_buf[7], (reply_buf + 8));
            else
#endif
            aicx_info(0x1160cf, "sds", "send to %s AT port,len %d--%s\n", port_name, len, reply_buf);

            if (com_id == AT_USB_AT_PORT_ID) {
                count = conn_at_send(conn_get_usb_at_conid, reply_str, len);
            } else if (is_cmux_com) {
#ifdef CONFIG_CMUX
                count = cmux_at_send(com_id, reply_str, len);
#endif
            } else {
                count = conn_at_send(com_id, reply_str, len);
            }

            if (count != len)
                aicx_error(0x1a7bca, "xx", "send to AT port error 0x%x 0x%x\n", count, len);
        }
    }
}

void send_to_com_entry(void *thread_input)
{
    u32 events, ret = AIC_SUCCESS, at_com_id;
    AT_REPLY_TO_COM *reply_to_com = AIC_NULL;
    REPLY_TO_COM_ITEM *reply_item = AIC_NULL;
#ifdef CONFIG_WAKEUP_SAMPLE
    boolean is_mcu_sleeping = AIC_FALSE, has_wokeup = AIC_FALSE;
    boolean is_mcu_exist = AIC_TRUE;
#endif

    reply_to_com = (AT_REPLY_TO_COM *)thread_input;
    if (!reply_to_com)
        return;

    at_com_id = reply_to_com->com_id;
    while(1) {
        aos_get_event(reply_to_com_event, reply_to_com->event_id, AOS_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        if (events & reply_to_com->event_id) {
#ifdef CONFIG_CUSTOMER_AT
            /* Send urc cached first. */
            if (aic_need_flush_urc_cache(at_com_id)) {
                aic_send_urc_cached(at_com_id);
            }
#endif
            do {
                aos_get_mutex(reply_to_com->send_mutex, AOS_WAIT_FOREVER);
                ret = aos_recv_from_queue(reply_to_com->reply_queue,
                                          (void*)&reply_item, AOS_NO_WAIT);
                aos_put_mutex(reply_to_com->send_mutex);

                if (ret == AIC_SUCCESS && reply_item) {
                    if (aic_is_cmux_com(at_com_id))
                        at_com_id = reply_item->at_com_id;
#ifdef CONFIG_WAKEUP_SAMPLE
                    if (aic_is_uart_com(at_com_id)) {
                        is_mcu_sleeping = wakeup_is_mcu_sleeping();
                        aicx_info(0x126cc7, "ddd", "is MCU sleeping=%d,need wakeup=%d,is_mcu_exist=%d\n",
                                  is_mcu_sleeping, reply_item->need_wakeup, is_mcu_exist);
                        if (is_mcu_sleeping) {
                            if (reply_item->need_wakeup && is_mcu_exist) {
                                if (wakeup_sample_awaken_mcu()) {
                                    aicx_info(0x048539, "", "wakeup mcu success\n");
                                    has_wokeup = AIC_TRUE;
                                } else {
                                    is_mcu_exist = AIC_FALSE;
                                }
                            } else {
                                aos_dma_free(reply_item->reply_ptr);
                                aos_free(reply_item);
                                continue;
                            }
                        }
                    }

                    aic_send_to_at_port(at_com_id, reply_item->reply_ptr, reply_item->reply_len);
                    if (has_wokeup) {
                        wakeup_sample_enable_mcu_sleep();
                        has_wokeup = AIC_FALSE;
                    }
#else
#ifdef CONFIG_AT_WAKEUP_TARGET_MCU
                    if (aic_is_uart_com(at_com_id) && reply_item->need_wakeup) {
                        aicx_info(0x0cff7a, "", "JQ:wakeup and send\n");
                        gpio_wakeup_target_mcu(1000);
                        aic_send_to_at_port(at_com_id, reply_item->reply_ptr, reply_item->reply_len);
                        gpio_release_target_mcu();
                    } else {
                        aic_send_to_at_port(at_com_id, reply_item->reply_ptr, reply_item->reply_len);
                    }
#else
                    aic_send_to_at_port(at_com_id, reply_item->reply_ptr, reply_item->reply_len);
#endif
#endif
                    aos_dma_free(reply_item->reply_ptr);
                    aos_free(reply_item);
                }

                reply_item = 0;
            } while (ret == AIC_SUCCESS);
        }
    }
}

void send_to_diag_entry(void *thread_input)
{
    u32 events, ret = AIC_SUCCESS;
    AT_REPLY_TO_COM *reply_to_diag = AIC_NULL;
    REPLY_TO_COM_ITEM *reply_item = AIC_NULL;
    diag_rsp_cmd_t diag_rsp;

    reply_to_diag = (AT_REPLY_TO_COM *)thread_input;
    if (!reply_to_diag)
        return;

    while(1) {
        aos_get_event(reply_to_com_event, reply_to_diag->event_id, AOS_OR_CLEAR,
                      &events, AOS_WAIT_FOREVER);

        if (events & reply_to_diag->event_id) {
            do {
                aos_get_mutex(reply_to_diag->send_mutex, AOS_WAIT_FOREVER);
                ret = aos_recv_from_queue(reply_to_diag->reply_queue,
                                          (void*)&reply_item, AOS_NO_WAIT);
                aos_put_mutex(reply_to_diag->send_mutex);

                if (ret == AIC_SUCCESS && reply_item) {
                    diag_set_rsp(&diag_rsp, reply_to_diag->diag_sn,
                                 DIAG_RSP_SUCC, 0,
                                 reply_to_diag->diag_sys,
                                 DIAG_CMD_AT_CMD, 0,
                                 reply_item->reply_ptr, reply_item->reply_len);
                    diag_cmd_response(&diag_rsp);
                    aos_free(reply_item->reply_ptr);
                    aos_free(reply_item);
                }

                reply_item = 0;
            } while (ret == AIC_SUCCESS);
        }
    }
}

#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
void subsys_state_change_callback(subsys_state_t cp_state, void *p)
{
    current_cp_state = cp_state;
    aicx_info(0x18aa27, "d", "receive cp state %d\n", cp_state);

    switch (cp_state) {
        case SUBSYS_STATE_ALIVE:
            /* Abuf link is ready now. Send some init AT cmds to CP. */
            aos_set_event(event_send_at, AT_EVENT_SEND_INIT_CMD, AOS_OR);
            break;

        /* If CP assert,then it works as entering flight mode. */
        case SUBSYS_STATE_ASSERT:
            aic_enter_flight_mode();
        break;

        default:
            /* Maybe do something later. */
            break;
    }
}
#endif

subsys_state_t aic_get_cp_state(void)
{
    return current_cp_state;
}

u32 aic_send_cfun_cmd(u8 start_cp)
{
    u8 cmd_str[20] = {'\0'};

    snprintf(cmd_str, sizeof(cmd_str), "AT+CFUN=%d", start_cp);
    return aic_send_cmd_with_callback(cmd_str, AIC_NULL);
}

boolean at_timer_is_used(void)
{
    u8 i;
    AT_ABUF_LINK_CFG *abuf_link_cfg;

    for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
        abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
        if (abuf_link_cfg->link_state == AT_ABUF_LINK_BUSY)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

void at_timer_suspend(void *p)
{
    if (!at_timer_is_used()) {
        aos_deactivate_timer(at_common_timer);
    }
}

void at_timer_resume(void *p)
{
    u32 active_state;
    u32 remaining_time;
    u32 reschedule;

    /* get if timer is activated */
    aos_get_timer_info(at_common_timer, AIC_NULL, &active_state,
                       &remaining_time, &reschedule);
    if (!active_state) {
        aos_change_timer(at_common_timer, AT_TIMER_INTERVAL, AOS_BE_PERIOD);
    }
}

/* The general function to handle the regular 1s timeout. */
static void aic_check_at_resp_timeout(void *input)
{
    aos_set_event(event_send_at, AT_EVENT_REGULAR_TIMEOUT, AOS_OR);
}

void aic_create_at_timer(void)
{
    at_common_timer = aos_create_timer("check_cp_at",
                                       aic_check_at_resp_timeout,
                                       AIC_NULL,
                                       AT_TIMER_INTERVAL,
                                       AOS_AUTO_ACTIVATE,
                                       AOS_BE_PERIOD);
    if (!at_common_timer)
        return;

    slp_register_sleep_hook(at_timer_suspend, AIC_NULL,
                            at_timer_resume, AIC_NULL);
}

static boolean is_urc_transmit_disabled(u8 *urc_str)
{
    if (strcasestr(urc_str, "^SIMSTATE") || strcasestr(urc_str, "^ACT:") ||
        strcasestr(urc_str, "^EPHYSTAT") || strcasestr(urc_str, "+MATREADY"))
        return AIC_TRUE;

    if (strcasestr(urc_str, "^CARDSTATUS") && reg_service_info.cardstatus_mode == 0)
        return AIC_TRUE;

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    if (strcasestr(urc_str, "+WIFISCANBSSID"))
        return AIC_TRUE;
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
    if (strcasestr(urc_str, "+CEREG") && reg_service_info.cereg_mode == 0)
        return AIC_TRUE;

    if (strcasestr(urc_str, "+CGEV") && reg_service_info.cgerep_mode == 0)
        return AIC_TRUE;

    if ((strcasestr(urc_str, "+CTZE") || strcasestr(urc_str, "+CTZV")) &&
        reg_service_info.ctzr_mode == 0)
        return AIC_TRUE;

    if (strcasestr(urc_str, "^CALCHK") || strcasestr(urc_str, "^DESQU") ||
        strcasestr(urc_str, "^CODE") || strcasestr(urc_str, "+CGDAC") ||
        strcasestr(urc_str, "+CNEMS1") || strcasestr(urc_str, "+CIEV:"))
        return AIC_TRUE;

    /* Discard the URC if in transparent mode. */
    if ((!aic_boot_mode_is_basic() && aic_is_transparent_mode()) || aic_is_powering_off())
        return AIC_TRUE;
#endif

    return AIC_FALSE;
}

#ifdef CONFIG_CUSTOMER_JQ_AT
static boolean is_urc_need_wakeup_mcu(u8 *urc_ptr, u32 urc_len)
{
    if (urc_ptr && (strcasestr(urc_ptr, "^CARDSTATUS:") || strcasestr(urc_ptr, "RING") ||
        strcasestr(urc_ptr, "CONNECT") || strcasestr(urc_ptr, "NO CARRIER") ||
        strcasestr(urc_ptr, "^DSCI:")))
        return AIC_TRUE;

    return AIC_FALSE;
}
#endif

/* Transmit URC to uart AT port and usb AT port,not to diag port. */
void transmit_cp_urc_to_serial_ports(u8 *urc_ptr, u16 urc_len)
{
    u32 serial_port_id;
    AT_REPLY_TO_COM *reply_to_com;
    AT_REPLY_TO_COM *tmp_reply_ptr;
    u32 configed_urc_port;
#ifdef CONFIG_D2D
    u32 total_len;
    u8 *msg_urc_ptr;
#endif

    if (!urc_ptr || urc_len == 0 || !reply_to_com_inited ||
        is_urc_transmit_disabled(urc_ptr))
        return;

#ifdef CONFIG_D2D
    if (aic_is_urc_msg_head_enabled()) {
        total_len = urc_len + AT_MSG_HEAD_LEN + 1;
        msg_urc_ptr = aic_dma_malloc(total_len);
        if (!msg_urc_ptr)
            return ;

        total_len--;
        *(msg_urc_ptr + total_len) = 0;
        aos_memcpy((msg_urc_ptr + AT_MSG_HEAD_LEN), urc_ptr, urc_len);

        if (aic_urc_prepend_msg_head(msg_urc_ptr, (msg_urc_ptr + AT_MSG_HEAD_LEN), urc_len) &&
            aic_get_d2d_urc_com_id() == AT_D2D_URC_CMUX_COM_ID) {

            aos_free(msg_urc_ptr);
            return;
        }

        aic_reply_to_certain_com(aic_get_d2d_urc_com_id(), msg_urc_ptr, total_len);
        aos_free(msg_urc_ptr);
    } else {
        aic_reply_to_certain_com(aic_get_d2d_urc_com_id(), urc_ptr, urc_len);
    }

    return;
#endif

#ifdef CONFIG_CUSTOMER_AT
    if (aic_is_urc_cache_enabled()) {
        aic_add_urc_to_cache(urc_ptr);
        return;
    }

    aic_send_urc_gpio_signal();
#endif

    configed_urc_port = aic_get_configed_urc_port_id();
    list_for_each_entry_safe(reply_to_com, tmp_reply_ptr, &reply_to_com_list,
                             list_key, AT_REPLY_TO_COM) {
        serial_port_id = reply_to_com->com_id;
        if (serial_port_id == AT_DIAG_SERIAL_PORT_ID)
            continue;

        if (configed_urc_port == AT_NO_SERIAL_PORT_ID || configed_urc_port == serial_port_id) {
#ifdef CONFIG_WAKEUP_SAMPLE
            if (aic_is_uart_com(serial_port_id) && wakeup_sample_is_urc_need_wakeup(urc_ptr)) {
                aic_reply_to_uart_with_wakeup(serial_port_id, urc_ptr, urc_len);
                continue;
            }
#endif
#ifdef CONFIG_CUSTOMER_JQ_AT
            if (is_urc_need_wakeup_mcu(urc_ptr, urc_len)) {
                aic_send_ri_signal_for_urc(serial_port_id);
                if (aic_is_uart_com(serial_port_id)) {
                    aic_reply_to_uart_with_wakeup(serial_port_id, urc_ptr, urc_len);
                } else {
                    aic_reply_to_certain_com(serial_port_id, urc_ptr, urc_len);
                }

                continue;
            } else {
                return;
            }
#endif
#ifdef CONFIG_CMUX
            if (serial_port_id != conn_get_uart_at_conid && !aic_is_cmux_com(serial_port_id))
                aic_reply_to_certain_com(serial_port_id, urc_ptr, urc_len);
#else
            aic_reply_to_certain_com(serial_port_id, urc_ptr, urc_len);
#endif
        }
    }
}

/* Settle the timeout condition. The at is received from external device. */
void settle_ext_at_timeout(AT_REQ_RESP *at_req_resp)
{
    if (!at_req_resp)
        return;

    if (at_req_resp->at_timeout_handler)
        at_req_resp->at_timeout_handler(at_req_resp);

    aic_reply_error_resp(at_req_resp, AT_ERROR_HANDLE_TIMEOUT);
    aic_invoke_next_external_at(at_req_resp->serial_port_id);
    aic_free_at_req_resp(at_req_resp);
}

/** Settle the timeout condition not receiving response from CP.
 * The at is received from internal.
 */
void settle_internal_timeout_at(AT_REQ_RESP *at_req_resp)
{
    u8 resp_str[20] = {0};

    if (!at_req_resp || !at_req_resp->at_req ||
        at_req_resp->is_waiting_resp == AT_RECEIVED_CP_RESP)
        return;

    if (at_req_resp->at_timeout_handler)
        at_req_resp->at_timeout_handler(at_req_resp);

    /* If the cmd is from external,then its retry count is always 0. */
    if (at_req_resp->retry_count == 0) {
        /* Set an ERROR response. */
        if (at_req_resp->at_resp) {
            snprintf(resp_str, 20, AT_RESULT_CME_ERROR"%d", AT_ERROR_HANDLE_TIMEOUT);
            fill_head_and_end_signs(at_req_resp->at_resp, resp_str, strlen(resp_str));
            at_req_resp->resp_line_count = 1;
            at_req_resp->is_waiting_resp = AT_RECEIVED_CP_RESP;

            if (at_req_resp->at_resp_handler)
                at_req_resp->at_resp_handler(at_req_resp);

            if (at_req_resp->inform_resp_cb)
                at_req_resp->inform_resp_cb(at_req_resp->at_resp,
                                            at_req_resp->resp_line_count);
        }

        if (!at_req_resp->wait_resp_semaphore)
            aic_free_at_req_resp(at_req_resp);
        else
            aos_put_semaphore(at_req_resp->wait_resp_semaphore);
    } else if (at_req_resp->retry_count > 0) {
        aic_send_at_to_cp(at_req_resp);
        at_req_resp->retry_count--;
    }
}

static void move_to_next_internal_at(AT_REQ_RESP *at_ptr)
{
    AT_REQ_RESP *tmp_ptr;

    if (!at_ptr)
        return;

    aos_get_mutex(mutex_at_pending, AOS_WAIT_FOREVER);
    tmp_ptr = at_ptr;
    at_pending_to_send = tmp_ptr->next_ptr;
    aos_put_mutex(mutex_at_pending);
}

#ifdef CONFIG_CUSTOMER_AT
void aic_invoke_sending_urc_cached(void)
{
    u32 configed_urc_port, at_port_id;
    AT_REPLY_TO_COM *resp_to_com_ptr;
    AT_REPLY_TO_COM *tmp_reply_ptr;

    if (!reply_to_com_inited)
        return;

    configed_urc_port = aic_get_configed_urc_port_id();

    list_for_each_entry_safe(resp_to_com_ptr, tmp_reply_ptr, &reply_to_com_list,
                             list_key, AT_REPLY_TO_COM) {
        at_port_id = resp_to_com_ptr->com_id;
        if (at_port_id == AT_DIAG_SERIAL_PORT_ID)
            continue;

        if (configed_urc_port == AT_NO_SERIAL_PORT_ID || configed_urc_port == at_port_id)
            aos_set_event(reply_to_com_event, resp_to_com_ptr->event_id, AOS_OR);
    }
}

/* Not proper for +CMGR +CMT +CMGL,whose format is not regular. */
u16 aic_refit_to_special_format(u8 *resp_ptr, u16 resp_len, u8 line_cnt)
{
    u8 *tmp_ptr, *new_resp_ptr;
    u16 line_len, new_total_len = 0;
    u8 i;
    ulong *line_arr;

    if (!resp_ptr || line_cnt == 0)
        return new_total_len;

    new_resp_ptr = aos_zalloc(resp_len + 1);
    AOS_ASSERT(new_resp_ptr);
    tmp_ptr = new_resp_ptr;

    if (line_cnt == 1) {
        new_total_len = resp_len - 2;
        strncpy(tmp_ptr, (resp_ptr + 2), new_total_len);
    } else {
        /* The line has deleted leading and ending \r\n. */
        line_arr = aic_get_each_line_from_resp(resp_ptr, line_cnt);
        if (!line_arr)
            return new_total_len;

        for (i = 0; i < line_cnt; i++) {
            line_len = strlen((u8 *)line_arr[i]);
            if (i == (line_cnt - 1)) {
                *tmp_ptr = aic_get_cr_sign();
                tmp_ptr++;
                *tmp_ptr = aic_get_lf_sign();
                tmp_ptr++;
                strncat(tmp_ptr, (u8 *)line_arr[i], line_len);
                tmp_ptr += line_len;
                /* The ending <CR><LF>. */
                *tmp_ptr = aic_get_cr_sign();
                tmp_ptr++;
                *tmp_ptr = aic_get_lf_sign();

                new_total_len += (line_len + 4);
            } else {
                /* No leading \r\n. */
                strncat(tmp_ptr, (u8 *)line_arr[i], line_len);
                tmp_ptr += line_len;
                *tmp_ptr = aic_get_cr_sign();
                tmp_ptr++;
                *tmp_ptr = aic_get_lf_sign();
                tmp_ptr++;

                new_total_len += (line_len + 2);
            }
        }

        aic_free_paras_mem(line_arr, line_cnt);
    }

    aicx_info(0x0c7998, "s", "new resp %s\n", new_resp_ptr);
    aos_memset(resp_ptr, 0, resp_len);
    strncpy(resp_ptr, new_resp_ptr, new_total_len);
    aos_free(new_resp_ptr);

    return new_total_len;
}
#endif

void process_at_resp_received(AT_REQ_RESP *at_ptr)
{
#ifdef CONFIG_CUSTOMER_AT
    u16 actual_len = 0;
#endif

    if (!at_ptr || !at_ptr->abuf_link_ptr)
        return;

    if (at_ptr->is_waiting_resp == AT_WAITING_CP_RESP) {
        at_ptr->is_waiting_resp = AT_RECEIVED_CP_RESP;
    }

    if (at_ptr->at_resp_handler) {
        /* AP handle the resp. */
        at_ptr->at_resp_handler(at_ptr);
    }

    if (at_ptr->inform_resp_cb)
        at_ptr->inform_resp_cb(at_ptr->at_resp, at_ptr->resp_line_count);

    if (at_ptr->is_external_cmd == AT_FROM_EXTERNAL) {
#ifdef CONFIG_CUSTOMER_AT
        if (aic_need_refit_resp_format(at_ptr)) {
            actual_len = aic_refit_to_special_format(at_ptr->at_resp,
                                                     at_ptr->current_resp_len,
                                                     at_ptr->resp_line_count);
            at_ptr->current_resp_len = actual_len;
        }
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
        if (aic_need_refit_resp_format(at_ptr))
            aic_refit_new_format_resp(at_ptr);
#endif
        aic_transmit_reply_to_external(at_ptr);
        aic_invoke_next_external_at(at_ptr->serial_port_id);
    }

    if (!at_ptr->wait_resp_semaphore) {
        aic_free_at_req_resp(at_ptr);
    } else {
        aos_put_semaphore(at_ptr->wait_resp_semaphore);
    }
}

/* This is espially for sending and writing short message. */
void process_medium_resp_recved(AT_REQ_RESP *at_ptr)
{
    if (!at_ptr || !at_ptr->abuf_link_ptr)
        return;

    at_ptr->abuf_link_ptr->link_state = AT_ABUF_LINK_WAIT_MESSAGE;
    /* Don't invoke time expiring. Set a new timeout when send message content. */
    at_ptr->start_time = 0;

    if (at_ptr->at_resp_handler)
        at_ptr->at_resp_handler(at_ptr);

    if (at_ptr->inform_resp_cb)
        at_ptr->inform_resp_cb(at_ptr->at_resp, at_ptr->resp_line_count);

    if (at_ptr->is_external_cmd == AT_FROM_EXTERNAL)
        aic_transmit_reply_to_external(at_ptr);

    if (at_ptr->wait_resp_semaphore) {
        aos_put_semaphore(at_ptr->wait_resp_semaphore);
    }

    /* Special process for modem driver. */
    if (at_ptr->abuf_link_ptr->link_id == AT_LINK5)
        aic_free_at_req_resp(at_ptr);
}

/* Send short message content. */
u32 aic_send_msg_content_and_wait(u8 *msg_buf, u16 len, AT_RESP_FROM_CP *resp_saved)
{
    u8 i;
    AT_ABUF_LINK_CFG *abuf_link_cfg = AIC_NULL;
    AT_REQ_RESP *at_req_ptr = AIC_NULL;
    u32 ret = AIC_SUCCESS;
    s32 count;
    u8 resp_str[30] = {'\0'};

    for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
        abuf_link_cfg = &abuf_link_cfg_set.at_abuf_link_list[i];

        if (abuf_link_cfg->link_state == AT_ABUF_LINK_WAIT_MESSAGE) {
            at_req_ptr = abuf_link_cfg->at_req_resp;
            if (!at_req_ptr) {
                aicx_error(0x0529b4, "", "error,no at req for sending msg content\n");
                abuf_link_cfg->link_state = AT_ABUF_LINK_IDLE;
                ret = AT_ERROR_NO_LINK_SEND_MSG_TEXT;
                break;
            }

            /* The last char should be ctrl+z. */
            /*if (*(msg_buf + len - 1) != 0x1A) {
                block_sending_msg_content(at_req_ptr, AT_ERROR_UNCOMPLETE_MSG_TEXT);
                break;
            }*/

            /* CP is not alive,and aipc is not ready,then can't send AT cmds. */
            if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
                block_sending_msg_content(at_req_ptr, AT_ERROR_CP_NOT_ALIVE);
                ret = AT_ERROR_CP_NOT_ALIVE;
                break;
            }

            if (at_req_ptr->wait_resp_semaphore)
                aicx_error(0x01bb5f, "", "error,resp semaphore should be null\n");

            at_req_ptr->wait_resp_semaphore =
                aos_create_semaphore("wait_resp_semaphore", 0);

            *at_req_ptr->at_resp = '\0';
            at_req_ptr->current_resp_len = 0;
            at_req_ptr->resp_line_count = 0;
            at_req_ptr->timeout = AT_TIMEOUT_HALF_MINUTE;
            at_req_ptr->start_time = aos_get_tickcount();
            /* Set the timeout to make it return. */
            count = abuf_write(abuf_handle, abuf_link_cfg->link_id,
                               msg_buf, len, AT_WRITE_ABUF_WAIT_TIME);
            if ((count < 0) || (count != len))
                aicx_error(0x024c5c, "dd", "write msg content to link--%d fail,count=%d\n",
                           abuf_link_cfg->link_id, count);

            if (!aos_get_semaphore(at_req_ptr->wait_resp_semaphore,
                                    (at_req_ptr->timeout + 1000)))
                aicx_info(0x06fd81, "", "send msg content:get semaphore\n");

            if (resp_saved && resp_saved->resp_buf) {
                aos_memset(resp_saved->resp_buf, 0, resp_saved->buf_len);
                strncpy(resp_saved->resp_buf, at_req_ptr->at_resp,
                        (resp_saved->buf_len - 1));
                resp_saved->resp_line_cnt = at_req_ptr->resp_line_count;
            }

            aos_delete_semaphore(at_req_ptr->wait_resp_semaphore);
            at_req_ptr->wait_resp_semaphore = AIC_NULL;

            if (aic_contain_final_result_code(at_req_ptr->at_resp,
                                              at_req_ptr->resp_line_count)) {
                aic_free_at_req_resp(at_req_ptr);
            }

            return ret;
        }
    }

    if (resp_saved && resp_saved->resp_buf) {
        aos_memset(resp_saved->resp_buf, 0, resp_saved->buf_len);
        sprintf(resp_str, AT_RESULT_CME_ERROR"%d", ret);
        fill_head_and_end_signs(resp_saved->resp_buf, resp_str, strlen(resp_str));
        resp_saved->resp_line_cnt = 1;
    }

    return ret;
}

void block_sending_msg_content(AT_REQ_RESP *at_req_resp, u32 error_num)
{
    AT_ABUF_LINK_CFG *abuf_link_cfg = AIC_NULL;
    u8 resp_str[30] = {'\0'};

    if (!at_req_resp)
        return;

    abuf_link_cfg = at_req_resp->abuf_link_ptr;
    aos_get_mutex(abuf_link_cfg->abuf_link_mutex, AOS_WAIT_FOREVER);
    aic_set_abuf_link_idle(abuf_link_cfg);
    aos_put_mutex(abuf_link_cfg->abuf_link_mutex);

    if (at_req_resp->is_external_cmd == AT_FROM_EXTERNAL) {
        aic_reply_error_resp(at_req_resp, error_num);

        aic_invoke_next_external_at(at_req_resp->serial_port_id);
        aic_free_at_req_resp(at_req_resp);
    } else if (at_req_resp->is_external_cmd == AT_FROM_INTERNAL) {
        if (at_req_resp->at_resp) {
            *at_req_resp->at_resp = '\0';
            sprintf(resp_str, AT_RESULT_CME_ERROR"%d", error_num);
            fill_head_and_end_signs(at_req_resp->at_resp, resp_str, strlen(resp_str));
            at_req_resp->resp_line_count = 1;

            if (at_req_resp->at_resp_handler)
                at_req_resp->at_resp_handler(at_req_resp);

            if (at_req_resp->inform_resp_cb)
                at_req_resp->inform_resp_cb(at_req_resp->at_resp,
                                            at_req_resp->resp_line_count);
        }

        move_to_next_internal_at(at_req_resp);
        if (!at_req_resp->wait_resp_semaphore)
            aic_free_at_req_resp(at_req_resp);
        else
            aos_put_semaphore(at_req_resp->wait_resp_semaphore);
    }
}

u8 get_cid_from_cgdata(u8 *cgdata_cmd)
{
    ulong *paras_arr;
    u16 paras_count;
    u8 cid = ANET_DEFAULT_PDP_CID;

    paras_arr = aic_split_paras_to_str_array(cgdata_cmd, "AT+CGDATA=",
                                             &paras_count);
    if (!paras_arr || paras_count != 2)
        return cid;
    else
        cid = atoi((u8 *)paras_arr[1]);

    aic_free_paras_mem(paras_arr, paras_count);

    return cid;
}

/** The task to receive receive AT response and URC from CP via abuf links.
 * Also check if receive the final response code,if so,then set event to
 * handle the response.
 */
void at_rev_from_cp_entry(void *thread_input)
{
    s32 count;
    u8 i;
    u32 link_selected;
    AT_ABUF_LINK_CFG *abuf_link_cfg;

    abuf_link_cfg_set.at_abuf_link_list[AT_LINK0].rev_buf =
        (u8 *)aos_zalloc_relaxed(AT_MAX_RESP_TOTAL_LENGTH);
    AIC_ASSERT(abuf_link_cfg_set.at_abuf_link_list[AT_LINK0].rev_buf);

    while (1) {
        link_selected = abuf_select(abuf_handle,
                                    abuf_link_cfg_set.link_set,
                                    AIPC_WAIT_FOREVER);

        for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
            /* Msg is received from the link. */
            abuf_link_cfg = &abuf_link_cfg_set.at_abuf_link_list[i];
            if (link_selected & BIT(abuf_link_cfg->link_id)) {
                aos_get_mutex(abuf_link_cfg->abuf_link_mutex, AOS_WAIT_FOREVER);

                if (!abuf_link_cfg->rev_buf) {
                    abuf_link_cfg->rev_buf =
                        (u8 *)aos_zalloc_relaxed(AT_MAX_RESP_TOTAL_LENGTH);
                    if (!abuf_link_cfg->rev_buf) {
                        aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
                        continue;
                    }
                }

                count = abuf_read(abuf_handle,
                                  abuf_link_cfg->link_id,
                                  abuf_link_cfg->rev_buf,
                                  (AT_MAX_RESP_TOTAL_LENGTH - 1),
                                  AIPC_NO_WAIT);

                /** The count must be more than 4 according to the format.
                 * The format is \r\nxxx\r\n,or \r\n> when send short message.
                 */
                if (count <= 0 || !is_msg_format_correct(abuf_link_cfg, count)) {
                    aicx_error(0x1bfed3, "dus", "abuf link %d read error msg--count %u--%s\n",
                               abuf_link_cfg->link_id, count,
                               abuf_link_cfg->rev_buf);
                    /* Some error happened,wait a while. */
                    if (count < 0)
                        aos_sleep(100);

                    aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
                    continue;
                }

                abuf_link_cfg->rev_buf[count] = 0;
                check_at_recv_msg(abuf_link_cfg, count);
                aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
            }
        }
   }
}

boolean is_msg_format_correct(AT_ABUF_LINK_CFG *abuf_link_cfg, s32 msg_len)
{
    u8 *tmp_ptr, *recv_msg;
    AT_REQ_RESP *at_req_ptr;

    if (!abuf_link_cfg)
        return AIC_FALSE;

    recv_msg = abuf_link_cfg->rev_buf;
    if (!recv_msg)
        return AIC_FALSE;

    if (strcasestr(recv_msg, "+CMT"))
        return AIC_TRUE;

    at_req_ptr = abuf_link_cfg->at_req_resp;
    /* Not check for AT+CLAC,+CMGL and +CMGR */
    if (at_req_ptr && at_req_ptr->at_req && (strcasestr(at_req_ptr->at_req, "AT+CLAC") ||
        strcasestr(recv_msg, "+CMGL") || strcasestr(recv_msg, "+CMGR") ||
        strcasestr(at_req_ptr->at_req, "AT+CSIM") || strcasestr(at_req_ptr->at_req, "AT+CGLA")))
        return AIC_TRUE;

    tmp_ptr = recv_msg;
    if (msg_len > AT_MIN_RESP_LENGTH) {
        /* Start and end with <CR><LF>. */
        if (is_cr_lf_sign(*tmp_ptr) && is_cr_lf_sign(*(tmp_ptr + 1)) &&
            is_cr_lf_sign(*(tmp_ptr + msg_len - 2)) &&
            is_cr_lf_sign(*(tmp_ptr + msg_len - 1))) {
            return AIC_TRUE;
        }
    }

    /* medium result code for short message: \r\n>  */
    if (aic_is_message_medium_result(recv_msg, msg_len))
        return AIC_TRUE;

    return AIC_FALSE;
}

u32 check_at_recv_msg(AT_ABUF_LINK_CFG *abuf_link_cfg, u32 msg_len)
{
    AT_REQ_RESP *at_req_resp_ptr;
    u16 current_len = 0, total_buf_len;
    u8 *tmp_urc_str = AIC_NULL;
    u32 string_len;

    at_req_resp_ptr = abuf_link_cfg->at_req_resp;

    if (abuf_link_cfg->link_id == AT_LINK0 &&
        abuf_link_cfg->link_state == AT_ABUF_LINK_IDLE) {
        /* AT^DLKS=1 has been sent,and the received msg is URC. */
        aicx_info(0x18e64c, "s", "recv URC %s\n", (u8 *)(abuf_link_cfg->rev_buf + 2));

        invoke_cp_urc_cbs(abuf_link_cfg->rev_buf);
        /* Ignore the leading and ending \r\n,4 chars. */
        string_len = msg_len -4;
        tmp_urc_str = (u8 *)aos_zalloc_relaxed(string_len + 1);
        if (tmp_urc_str) {
            aic_delete_start_end_symbols(tmp_urc_str, abuf_link_cfg->rev_buf, string_len);
            aic_invoke_urc_handler(tmp_urc_str);
            aos_free(tmp_urc_str);
        }

        transmit_cp_urc_to_serial_ports(abuf_link_cfg->rev_buf, msg_len);
    } else {
        if (!at_req_resp_ptr ||
            !(at_req_resp_ptr->at_req && at_req_resp_ptr->at_resp) ||
            abuf_link_cfg->resp_complete != AT_RESP_NOT_COMPLETED ||
            at_req_resp_ptr->is_waiting_resp != AT_WAITING_CP_RESP) {
            aicx_error(0x192b2e, "ds", "abuf link %d recv invalid resp %s\n",
                       abuf_link_cfg->link_id, abuf_link_cfg->rev_buf);
            return AIC_SUCCESS;
        }

        current_len = at_req_resp_ptr->current_resp_len;
        total_buf_len = at_req_resp_ptr->resp_buf_len;
        if (msg_len < (total_buf_len - current_len)) {
            aos_memcpy((at_req_resp_ptr->at_resp + current_len), abuf_link_cfg->rev_buf, msg_len);
            at_req_resp_ptr->current_resp_len += msg_len;
            at_req_resp_ptr->resp_line_count++;
        } else {
            /* Discard it,not copy to at_resp. */
            aicx_error(0x18da03, "", "resp buffer is full or invalid reply\n");
            /* The response of AT+CMGL may be over size. */
            if (at_req_resp_ptr->is_external_cmd == AT_FROM_EXTERNAL) {
                aic_transmit_reply_to_external(at_req_resp_ptr);
                at_req_resp_ptr->current_resp_len = 0;
                at_req_resp_ptr->resp_line_count = 0;
                aos_memset(at_req_resp_ptr->at_resp, 0, total_buf_len);
                aos_memcpy(at_req_resp_ptr->at_resp, abuf_link_cfg->rev_buf, msg_len);
                at_req_resp_ptr->current_resp_len += msg_len;
                at_req_resp_ptr->resp_line_count++;
            }
        }

        aicx_info(0x12daf5, "ds", "abuf link %d receive response %s\n",
                  abuf_link_cfg->link_id, (u8 *)abuf_link_cfg->rev_buf);

        if (is_final_result_code(abuf_link_cfg->rev_buf, msg_len, AIC_TRUE)) {
            abuf_link_cfg->resp_complete = AT_RESP_COMPLETED;
            /* In case time expires before the resp is processed. */
            at_req_resp_ptr->start_time = 0;

            if (strcasestr(at_req_resp_ptr->at_req, "AT+CGDATA=") &&
                strcasestr(at_req_resp_ptr->at_resp, "CONNECT")) {
                if (abuf_link_cfg->link_id != AT_LINK0) {
                    abuf_link_cfg->at_req_resp = AIC_NULL;
                }
                abuf_link_cfg->activated_pdp_cid =
                    get_cid_from_cgdata(at_req_resp_ptr->at_req);
                abuf_link_cfg->resp_complete = AT_RESP_BLOCKED;
            } else {
                aic_set_abuf_link_idle(abuf_link_cfg);
            }

            process_at_resp_received(at_req_resp_ptr);
        } else if (aic_is_message_medium_result(abuf_link_cfg->rev_buf, msg_len)) {
            process_medium_resp_recved(at_req_resp_ptr);
        }
    }

    return AIC_SUCCESS;
}

boolean is_final_result_code(u8 *at_resp_str, u32 len, boolean has_leading_sign)
{
    boolean ret = AIC_FALSE;
    u8 *tmp_ptr;

    /* Ignor the leading \r\n. */
    if (has_leading_sign)
        tmp_ptr = at_resp_str + 2;
    else
        tmp_ptr = at_resp_str;

    /* Ignore the leading \r\n,then check if OK is appending. */
    if ((len >= strlen(AT_RESULT_OK)) && len <= (strlen(AT_RESULT_CONNECT) + 4) &&
        (strncasecmp(tmp_ptr, AT_RESULT_OK, 2) == 0 ||
        strncasecmp(tmp_ptr, AT_RESULT_CONNECT, strlen(AT_RESULT_CONNECT)) == 0)) {
        ret = AIC_TRUE;
    }

    /* Contains ERROR string. Maybe it is +CME ERROR: or ERROR. */
    if ((len >= strlen(AT_RESULT_ERROR)) && len <= 100 &&
        (strcasestr(at_resp_str, AT_RESULT_ERROR) ||
        strcasestr(at_resp_str, AT_RESULT_NO_CARRIER))) {
        ret = AIC_TRUE;
    }

    return ret;
}

boolean aic_contain_final_result_code(u8 *resp_str, u8 line_cnt)
{
    ulong *line_arr;
    u16 line_len;
    boolean ret = AIC_FALSE;

    if (!resp_str || strlen(resp_str) == 0 || line_cnt == 0)
        return AIC_FALSE;

    line_arr = aic_get_each_line_from_resp(resp_str, line_cnt);
    if (!line_arr)
        return AIC_FALSE;

    /* The last line contains the final result code. */
    line_len = strlen((u8 *)line_arr[line_cnt - 1]);
    if (is_final_result_code((u8 *)line_arr[line_cnt - 1], line_len, AIC_FALSE))
        ret = AIC_TRUE;

    aic_free_paras_mem(line_arr, line_cnt);

    return ret;
}

boolean aic_is_message_medium_result(u8 *medium_resp, u32 len)
{
    u8 *tmp_ptr = medium_resp;

    if (len != AT_MIN_RESP_LENGTH)
        return AIC_FALSE;

    if (is_cr_lf_sign(*tmp_ptr) && is_cr_lf_sign(*(tmp_ptr + 1)) &&
        *(tmp_ptr + 2) == '>' && *(tmp_ptr + 3) == ' ') {
        return AIC_TRUE;
    }

    return AIC_FALSE;
}

u32 aic_register_flight_mode_change_cb(aic_flight_mode_change_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_FLIGHT_MODE_CBS; i++) {
        if (!flight_mode_cbs[i]) {
            flight_mode_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_FLIGHT_MODE_CBS_OVERFLOW;
}

void aic_unregister_flight_mode_change_cb(aic_flight_mode_change_cb cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_FLIGHT_MODE_CBS; i++) {
        if (flight_mode_cbs[i] == cb) {
            flight_mode_cbs[i] = AIC_NULL;
            return;
        }
    }
}

void aic_at_mims_query_callback(u8 *cmd_resp, u8 line_count)
{
    ulong *line_arr;
    u8 *tmp_ptr = AIC_NULL;

    if (line_count <= 1 || !cmd_resp)
        return;

    line_arr = aic_get_each_line_from_resp(cmd_resp, line_count);
    if (line_arr) {
        tmp_ptr = strcasestr((u8 *)line_arr[0], "+MIMS:");
        if (tmp_ptr) {
            tmp_ptr += strlen("+MIMS:");
            reg_service_info.ims_enabled = atoi(tmp_ptr);
        }
    }

    aic_free_paras_mem(line_arr, line_count);
}

static boolean is_ready_to_send(AT_REQ_RESP *at_ptr, u8 *ready_link)
{
    u8 ready_link_id;
    u32 error_value = 0;

    /* The length of at cmd sent to CP can't be more than 1K. */
    if (strlen(at_ptr->at_req) > AT_MAX_CP_CMD_LENGTH) {
        error_value = AT_ERROR_INVALID_CMD;
        goto HANDLE_ERROR;
    }

    /* CP is not alive,and aipc is not ready,then can't send AT cmds. */
    if (aic_get_cp_state() != SUBSYS_STATE_ALIVE) {
        error_value = AT_ERROR_CP_NOT_ALIVE;
        goto HANDLE_ERROR;
    }

    ready_link_id = get_ready_abuf_link(at_ptr);
    if (!is_valid_abuf_link(ready_link_id)) {
        error_value = AT_ERROR_NO_IDLE_LINK;
        goto HANDLE_ERROR;
    } else {
        *ready_link = ready_link_id;
    }

    return AIC_TRUE;

HANDLE_ERROR:
    block_sending_at_to_cp(at_ptr, error_value);

    return AIC_FALSE;
}

#ifdef CONFIG_CUSTOMER_CM_AT
static u8 get_pdp_type_from_string(u8 *pdp_type)
{
    u8 len;

    if (!pdp_type || strlen(pdp_type) > 6)
        return ANET_DEFAULT_IP_TYPE_V4V6;

    len = strlen(pdp_type);
    /* IP IPV6 IPV4V6 PPP or Non-IP */
    if (len == 2 && strcasestr(pdp_type, "IP"))
        return ANET_DEFAULT_IP_TYPE_V4;

    if (strcasestr(pdp_type, "PPP"))
        return 0xff;

    if (len == 4 && strcasestr(pdp_type, "IPV6"))
        return ANET_DEFAULT_IP_TYPE_V6;

    if (len == 6 && (strcasestr(pdp_type, "IPV4V6") || strcasestr(pdp_type, "Non-IP")))
        return ANET_DEFAULT_IP_TYPE_V4V6;

    return 0xff;
}

static void set_default_custom_apn_cfg(void)
{
    u8 cmd_str[80] = {'\0'};
    u8 len1, len2, pdp_type = 0xff;

    if (!init_net_cfg.init_read_flag) {
        aos_memset(&init_net_cfg, 0, sizeof(INIT_NET_CFG));
        if (!aic_read_net_config(&init_net_cfg.net_cfg))
            init_net_cfg.init_read_flag = 1;
    }

    len1 = strlen(init_net_cfg.net_cfg.pdp_type);
    len2 = strlen(init_net_cfg.net_cfg.default_apn);
    /* AT^DDPDN=<pdp_type>,<apn>, pdp_type is interger whose default value is 3. */
    if (len1 > 0 || len2 > 0) {
        pdp_type = get_pdp_type_from_string(init_net_cfg.net_cfg.pdp_type);
        snprintf(cmd_str, sizeof(cmd_str), "AT^DDPDN=%d,\"%s\"", pdp_type,
                 init_net_cfg.net_cfg.default_apn);
        aic_send_cmd_with_callback(cmd_str, AIC_NULL);
    }
}

static boolean handle_resp_by_ap(AT_ABUF_LINK_CFG *abuf_link_cfg, AT_REQ_RESP *at_req_ptr)
{
    NET_CFG net_cfg;

    if (strcasestr(at_req_ptr->at_req, "AT+CEREG=0")) {
        reg_service_info.cereg_mode = 0;
        aos_memset(&net_cfg, 0, sizeof(NET_CFG));
        aic_read_net_config(&net_cfg);
        net_cfg.cereg_mode = reg_service_info.cereg_mode;
#ifdef CONFIG_MISC_SUPPORT
        aic_save_config_to_nv(&net_cfg);
#endif
        goto HANDLE_RESP;
    } else if (strcasestr(at_req_ptr->at_req, "AT+CGEREP=0")) {
        reg_service_info.cgerep_mode = 0;
        goto HANDLE_RESP;
    } else if (strcasestr(at_req_ptr->at_req, "AT+CTZR=0")) {
        reg_service_info.ctzr_mode = 0;
        goto HANDLE_RESP;
    } else {
        goto NO_HANDLE;
    }

 HANDLE_RESP:
    if (!at_req_ptr->at_resp) {
        at_req_ptr->at_resp = aos_zalloc_relaxed(8);
        if(!at_req_ptr->at_resp)
            goto NO_HANDLE;
    }

    at_req_ptr->abuf_link_ptr = abuf_link_cfg;
    fill_head_and_end_signs(at_req_ptr->at_resp, AT_RESULT_OK, 2);
    at_req_ptr->current_resp_len = 6;
    at_req_ptr->resp_line_count = 1;

    at_req_ptr->start_time = 0;
    abuf_link_cfg->resp_complete = AT_RESP_COMPLETED;
    if (at_req_ptr->is_external_cmd == AT_FROM_INTERNAL)
        move_to_next_internal_at(at_req_ptr);

    aic_set_abuf_link_idle(abuf_link_cfg);
    process_at_resp_received(at_req_ptr);

    return AIC_TRUE;

NO_HANDLE:
    return AIC_FALSE;
}

static void set_init_sys_time(void)
{
    aos_systime init_time;

    current_time_zone = 0;
    time_zone_in_quarters = 0;
    init_time.year = 1970;
    init_time.month = 1;
    init_time.day = 1;
    init_time.hour = 0;
    init_time.minute = 0;
    init_time.second = 0;
    aos_set_systime(&init_time);
}

/* Refit format as follows:
 * \r\nxxx\r\n
 * xxx\r\n
 * xxx\r\n
 * \r\nOK\r\n
 */
void aic_refit_new_format_resp(AT_REQ_RESP *at_req_ptr)
{
    u8 *tmp_ptr, *new_resp_ptr, *resp_ptr;
    u16 resp_len, line_len, new_total_len = 0;
    u8 i, line_cnt;
    ulong *line_arr;

    if (!at_req_ptr)
        return;

    resp_ptr = at_req_ptr->at_resp;
    resp_len = at_req_ptr->current_resp_len;
    line_cnt = at_req_ptr->resp_line_count;
    /* Not adjust if there is one or two lines. */
    if (line_cnt <= 2 || !resp_ptr || resp_len == 0)
        return;

    new_resp_ptr = aos_zalloc_relaxed(resp_len + 1);
    if (!new_resp_ptr)
        return;
    tmp_ptr = new_resp_ptr;

    /* The line has deleted leading and ending \r\n. */
    line_arr = aic_get_each_line_from_resp(resp_ptr, line_cnt);
    if (!line_arr) {
        aos_free(new_resp_ptr);
        return;
    }

    for (i = 0; i < line_cnt; i++) {
        line_len = strlen((u8 *)line_arr[i]);
        if (i == 0 || i == (line_cnt - 1)) {
            *tmp_ptr = aic_get_cr_sign();
            tmp_ptr++;
            *tmp_ptr = aic_get_lf_sign();
            tmp_ptr++;
            strncat(tmp_ptr, (u8 *)line_arr[i], line_len);
            tmp_ptr += line_len;
            /* The ending <CR><LF>. */
            *tmp_ptr = aic_get_cr_sign();
            tmp_ptr++;
            *tmp_ptr = aic_get_lf_sign();
            new_total_len += (line_len + 4);

            if (i == 0)
                tmp_ptr++;
        } else {
            /* No leading \r\n. */
            strncat(tmp_ptr, (u8 *)line_arr[i], line_len);
            tmp_ptr += line_len;
            *tmp_ptr = aic_get_cr_sign();
            tmp_ptr++;
            *tmp_ptr = aic_get_lf_sign();
            tmp_ptr++;

            new_total_len += (line_len + 2);
        }
    }

    aic_free_paras_mem(line_arr, line_cnt);
    aos_memset(resp_ptr, 0, resp_len);
    strncpy(resp_ptr, new_resp_ptr, new_total_len);
    at_req_ptr->current_resp_len = new_total_len;
    aos_free(new_resp_ptr);
}
#endif

#if CONFIG_CUSTOMER_CM_AT || CONFIG_CUSTOMER_AT
/* Check if need converting to new format.
 * Not proper for +CMGR +CMT +CMGL AT+CLAC,whose format is not regular. */
boolean aic_need_refit_resp_format(AT_REQ_RESP *at_req_ptr)
{
    if (at_req_ptr && (strcasestr(at_req_ptr->at_resp, "+CMGL:") ||
        strcasestr(at_req_ptr->at_resp, "+CMGR:") ||
        strcasestr(at_req_ptr->at_resp, "+CMT:") ||
        strcasestr(at_req_ptr->at_req, "AT+CLAC")))
        return AIC_FALSE;

    return AIC_TRUE;
}
#endif

static boolean recv_event_to_send_external_at(u32 events)
{
    AT_RECV_COM_ITEM *at_com_item = AIC_NULL;
    AT_RECV_COM_ITEM *tmp_at_com_ptr;

    if (!multi_at_com_inited)
        return AIC_FALSE;

    list_for_each_entry_safe(at_com_item, tmp_at_com_ptr, &multi_at_com_list,
                             list_key, AT_RECV_COM_ITEM) {
        if (events & at_com_item->send_to_cp_event_id)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

static u32 common_handle_send_to_cp(AT_REQ_RESP *at_to_send)
{
    AT_ABUF_LINK_CFG *abuf_link_cfg;
    u8 ready_link_id = AT_LINK_CNT;
    u32 ret;

    if (!is_ready_to_send(at_to_send, &ready_link_id))
        return AT_ERROR_HANDLE_FAILURE;

    abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[ready_link_id]);
    aos_get_mutex(abuf_link_cfg->abuf_link_mutex, AOS_WAIT_FOREVER);
    abuf_link_cfg->at_req_resp = at_to_send;
    abuf_link_cfg->link_state = AT_ABUF_LINK_BUSY;

#ifdef CONFIG_CUSTOMER_CM_AT
    if (handle_resp_by_ap(abuf_link_cfg, at_to_send)) {
        aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
        return AIC_SUCCESS;
    }
#endif

    ret = write_at_to_mbuf(at_to_send, ready_link_id);
    /* If fail to write,then don't wait to expire. */
    if (ret) {
        ret = AT_ERROR_WRITE_LINK_FAIL;
        goto SEND_BLOCK;
    }

    if (!at_to_send->at_resp) {
        if (!(strcasestr(at_to_send->at_req, "AT+CSIM") ||
            strcasestr(at_to_send->at_req, "AT+CGLA"))) {
            at_to_send->at_resp = aos_zalloc_relaxed(SIZE_1K);
            at_to_send->resp_buf_len = SIZE_1K;
        } else {
            at_to_send->at_resp = aos_zalloc_relaxed(SIZE_4K);
            at_to_send->resp_buf_len = SIZE_4K;
        }

        if (!at_to_send->at_resp) {
            ret = AT_ERROR_MALLOC_FAIL;
            goto SEND_BLOCK;
        }
    }

    at_to_send->start_time = aos_get_tickcount();
    at_to_send->abuf_link_ptr = abuf_link_cfg;
    aos_put_mutex(abuf_link_cfg->abuf_link_mutex);

    if (at_to_send->is_external_cmd == AT_FROM_INTERNAL)
        move_to_next_internal_at(at_to_send);

    return AIC_SUCCESS;

SEND_BLOCK:
    block_sending_at_to_cp(at_to_send, ret);
    abuf_link_cfg->at_req_resp = AIC_NULL;
    abuf_link_cfg->link_state = AT_ABUF_LINK_IDLE;
    aos_put_mutex(abuf_link_cfg->abuf_link_mutex);

    return ret;
}

static void send_external_at_to_cp(u32 events)
{
    AT_RECV_COM_ITEM *at_com_item;
    AT_RECV_COM_ITEM *tmp_at_com_ptr;

    list_for_each_entry_safe(at_com_item, tmp_at_com_ptr, &multi_at_com_list,
                             list_key, AT_RECV_COM_ITEM) {
        if (events & at_com_item->send_to_cp_event_id) {

            if (!at_com_item->serial_dispatch_at ||
                at_com_item->serial_dispatch_at->is_send_to_cp != AT_SEND_TO_CP ||
                at_com_item->serial_dispatch_at->abuf_link_ptr) {
                continue;
            }

            common_handle_send_to_cp(at_com_item->serial_dispatch_at);
        }
    }
}

#ifdef CONFIG_WAKEUP_SAMPLE
static void transmit_gxem_mode_urc(void)
{
    u8 boot_mode_urc[20] = {'\0'};

    if (aic_boot_mode_is_gxem())
        strcpy(boot_mode_urc, "^GXEMMODE: 1");
    else
        strcpy(boot_mode_urc, "^GXEMMODE: 0");

    aic_reply_to_uart_with_wakeup(conn_get_uart_at_conid, boot_mode_urc, strlen(boot_mode_urc));
}
#endif

void enable_cereg_urc_callback(u8 *cmd_resp, u8 line_count)
{
    u8 cmd_str[16] = {'\0'};

    /* Query if IMS if enabled when init. */
    aic_send_cmd_with_callback("AT+MIMS?", aic_at_mims_query_callback);

    if (init_net_cfg.net_cfg.cfun_pre_rst != 0xff) {
        snprintf(cmd_str, sizeof(cmd_str), "AT+CFUN=%d",
                 init_net_cfg.net_cfg.cfun_pre_rst);
        aic_send_cmd_with_callback(cmd_str, AIC_NULL);
        init_net_cfg.net_cfg.cfun_pre_rst = 0xff;
#ifdef CONFIG_MISC_SUPPORT
        aic_save_config_to_nv(&init_net_cfg.net_cfg);
#endif
    } else {
#ifdef CONFIG_TELE_CTRL_BY_AT
        /* Not send AT+CFUN=1 in gxem mode. */
        if (!aic_boot_mode_is_gxem()) {
            if (init_net_cfg.net_cfg.auto_start_ps == 1)
                aic_send_cfun_cmd(AT_START_CP);
            else if (init_net_cfg.net_cfg.auto_start_ps == 0)
                aic_send_cfun_cmd(AT_STOP_CP);
        }
#endif
    }
}

/* Enable the +CTZE URC via AT+CTZR=2. */
u32 set_local_time_urc(void)
{
    u8 at_str[16] = {'\0'};

    snprintf(at_str, sizeof(at_str), "AT+CTZR=%d", AT_CTZR_DEF_MODE);
    return aic_send_cmd_with_callback(at_str, AIC_NULL);
}

s32 aic_load_net_nv_cfg(void)
{
    s32 ret = AIC_SUCCESS;

    if (!init_net_cfg.init_read_flag) {
        aos_memset(&init_net_cfg, 0, sizeof(INIT_NET_CFG));
        ret = aic_read_net_config(&init_net_cfg.net_cfg);
        if (!ret)
            init_net_cfg.init_read_flag = 1;
    }

    return ret;
}

#ifdef CONFIG_D2D
static void init_d2d_mode_cfg(void)
{
    if (!aic_load_net_nv_cfg()) {
        d2d_mode_cfg.mode = init_net_cfg.net_cfg.d2d_lte_mode;
        d2d_mode_cfg.pkt_size = init_net_cfg.net_cfg.d2d_pkt_size;
        d2d_mode_cfg.enable_pkt_dump = init_net_cfg.net_cfg.d2d_pkt_dump;
        d2d_mode_cfg.role = init_net_cfg.net_cfg.d2d_role;
        aic_set_urc_msg_head_enabled(init_net_cfg.net_cfg.d2d_msg_head_enabled);
#ifdef CONFIG_CMUX
        strcpy(d2d_mode_cfg.at_com_type, init_net_cfg.net_cfg.d2d_com_type);
#else
        if (strcasestr(init_net_cfg.net_cfg.d2d_com_type, "cmux"))
            strcpy(d2d_mode_cfg.at_com_type, "uart");
        else
            strcpy(d2d_mode_cfg.at_com_type, init_net_cfg.net_cfg.d2d_com_type);
#endif
    }

    if (d2d_mode_cfg.pkt_size == 0)
        d2d_mode_cfg.pkt_size = AT_D2D_DEFAULT_PKT_SIZE;

    if (d2d_mode_cfg.at_com_id == AT_NO_SERIAL_PORT_ID)
        d2d_mode_cfg.at_com_id = conn_get_uart_at_conid;

#ifdef CONFIG_CMUX
    if (strcasestr(d2d_mode_cfg.at_com_type, AT_D2D_COM_TYPE_CMUX))
        aic_set_d2d_urc_com_id(AT_D2D_URC_CMUX_COM_ID);
    else
#endif
    if (strcasestr(d2d_mode_cfg.at_com_type, AT_D2D_COM_TYPE_UART))
        aic_set_d2d_urc_com_id(conn_get_uart_at_conid);
    else if (strcasestr(d2d_mode_cfg.at_com_type, AT_D2D_COM_TYPE_USB))
        aic_set_d2d_urc_com_id(AT_USB_AT_PORT_ID);

    if (d2d_mode_cfg.mode == AT_UE_MODE_D2D) {
        aic_create_at_timer();
        aicx_info(0x029363, "", "D2D mode,inform CP");
        aic_send_cmd_with_callback("AT+DMODE=1", AIC_NULL);
    }

    extern void aic_d2d_init_param(D2D_MODE_CFG *mode_cfg);
    aic_d2d_init_param(&d2d_mode_cfg);
}
#endif

void at_send_to_cp_entry(void *thread_input)
{
    u32 at_events = 0, current_time = 0, ret;
    u8 i;
    AT_ABUF_LINK_CFG *abuf_link_cfg;
    ulong at_req_resp = 0;
    AT_REQ_RESP *at_ptr;

    while (1) {
        aos_get_event(event_send_at, 0xFFFFFFFF, AOS_OR_CLEAR,
                      &at_events, AOS_WAIT_FOREVER);

        if (at_events & AT_EVENT_SEND_CMD_TO_CP) {
            while (at_pending_to_send) {
                if (at_pending_to_send->abuf_link_ptr ||
                    at_pending_to_send->is_send_to_cp != AT_SEND_TO_CP) {
                    move_to_next_internal_at(at_pending_to_send);
                    continue;
                }

                common_handle_send_to_cp(at_pending_to_send);
            }
        }

        if (recv_event_to_send_external_at(at_events)) {
            send_external_at_to_cp(at_events);
        }

        if (at_events & AT_EVENT_SEND_INIT_CMD) {
#ifdef CONFIG_D2D
            init_d2d_mode_cfg();
            continue;
#endif

#ifdef CONFIG_CUSTOMER_CM_AT
            /* Can't set sys time too early. */
            set_init_sys_time();
#endif
            /* Not update via CTZE,so set it here. */
            current_time_zone = AIC_DEFAULT_TIME_ZONE;
            aic_create_at_timer();
            set_local_time_urc();
#ifdef CONFIG_WAKEUP_SAMPLE
            transmit_gxem_mode_urc();
#endif
#ifdef CONFIG_MISC_SUPPORT
            aic_load_net_nv_cfg();
            if (init_net_cfg.net_cfg.ctzu_mode != 0xff)
                reg_service_info.ctzu_mode = init_net_cfg.net_cfg.ctzu_mode;
#else
            init_net_cfg.net_cfg.auto_start_ps = 1;
            init_net_cfg.net_cfg.cfun_pre_rst = 0xff;
#endif
#ifdef CONFIG_CUSTOMER_CM_AT
            set_default_custom_apn_cfg();
            aic_set_cereg_mode(init_net_cfg.net_cfg.cereg_mode);
            /* Send MATREADY to indicate ready to recv AT cmds. */
            aic_reply_to_certain_com(conn_get_uart_at_conid, "+MATREADY\r\n", strlen("+MATREADY\r\n"));
            if (conn_get_usb_at_conid != AIC_ENODEV)
                aic_reply_to_certain_com(AT_USB_AT_PORT_ID, "+MATREADY\r\n", strlen("+MATREADY\r\n"));
#endif
            /* basic mode meed autostart ps. */
            if (aic_boot_mode_is_basic()) {
                init_net_cfg.net_cfg.auto_start_ps = 1;
                aic_send_cfun_cmd(AT_START_CP);
                continue;
            }

            if (reg_service_info.cereg_mode == 0)
                enable_cereg_urc(AT_CEREG_DEF_MODE, enable_cereg_urc_callback);
            else
                enable_cereg_urc(reg_service_info.cereg_mode, enable_cereg_urc_callback);

#ifdef CONFIG_PA_VBAT_POWER
            aic_send_cmd_with_callback("AT+PABUCK2=0", AIC_NULL);
#endif
        }

        if (at_events & AT_EVENT_ABORT_LINK) {
            for (i = AT_LINK1; i < AT_LINK_CNT; i++) {
                abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
                if (abuf_link_cfg->link_state == AT_ABUF_LINK_RESERVED) {
                    ret = aic_assemble_and_handle_at_cmd("AT^ACLLK",
                                                         &at_req_resp,
                                                         AT_FROM_INTERNAL,
                                                         AT_NO_SERIAL_PORT_ID,
                                                         AT_RETRY_COUNT0);
                    if (!ret && at_req_resp)
                        ((AT_REQ_RESP *)at_req_resp)->assigned_link_id = i;
                }
            }
        }

        if (at_events & AT_EVENT_REGULAR_TIMEOUT) {
            for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
                abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
                if (abuf_link_cfg->link_state != AT_ABUF_LINK_BUSY &&
                    abuf_link_cfg->link_state != AT_ABUF_LINK_WAIT_MESSAGE)
                    continue;

                aos_get_mutex(abuf_link_cfg->abuf_link_mutex, AOS_WAIT_FOREVER);
                at_ptr = abuf_link_cfg->at_req_resp;

                if (at_ptr && at_ptr->timeout && at_ptr->start_time) {
                    current_time = aos_get_tickcount();
                    if ((current_time - at_ptr->start_time) >= at_ptr->timeout) {
                        abuf_link_cfg->is_link_timeout = AT_ABUF_TIMEOUT;

                        if (at_ptr->is_external_cmd == AT_FROM_INTERNAL) {
                            aicx_info(0x1d7cc1, "", "internal at timeout\n");
                            aic_set_abuf_link_idle(abuf_link_cfg);
                            settle_internal_timeout_at(at_ptr);
                        } else if (at_ptr->is_external_cmd == AT_FROM_EXTERNAL) {
                            aicx_info(0x0fb70a, "", "external at timeout\n");
                            aic_set_abuf_link_idle(abuf_link_cfg);
                            settle_ext_at_timeout(at_ptr);
                        }
                    }
                }

                aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
            }
        }
    }
}

void block_sending_at_to_cp(AT_REQ_RESP *at_req_resp, u32 error_num)
{
    AT_ABUF_LINK_CFG *abuf_link_cfg = AIC_NULL;
    u8 resp_str[30] = {0};

    if (!at_req_resp)
        return;

    if (at_req_resp->is_external_cmd == AT_FROM_EXTERNAL) {
        aic_reply_error_resp(at_req_resp, error_num);
        aicx_info(0x0e51db, "", "block sending to cp,invoke next external at\n");
        aic_invoke_next_external_at(at_req_resp->serial_port_id);
        aic_free_at_req_resp(at_req_resp);
    } else if (at_req_resp->is_external_cmd == AT_FROM_INTERNAL) {
        abuf_link_cfg = get_abuf_cfg_by_link(at_req_resp->assigned_link_id);
        if (abuf_link_cfg)
            abuf_link_cfg->link_state = AT_ABUF_LINK_IDLE;

        at_req_resp->is_waiting_resp = AT_RECEIVED_CP_RESP;
        at_req_resp->at_resp = aos_zalloc_relaxed(30);
        if (at_req_resp->at_resp) {
            snprintf(resp_str, 30, AT_RESULT_CME_ERROR"%d", error_num);
            fill_head_and_end_signs(at_req_resp->at_resp, resp_str, strlen(resp_str));
            at_req_resp->resp_line_count = 1;

            if (at_req_resp->at_resp_handler)
                at_req_resp->at_resp_handler(at_req_resp);

            if (at_req_resp->inform_resp_cb)
                at_req_resp->inform_resp_cb(at_req_resp->at_resp,
                                            at_req_resp->resp_line_count);
        }

        move_to_next_internal_at(at_req_resp);
        /* Free the current AT and move to next AT. */
        if (!at_req_resp->wait_resp_semaphore)
            aic_free_at_req_resp(at_req_resp);
        else
            aos_put_semaphore(at_req_resp->wait_resp_semaphore);
    }
}

AT_ABUF_LINK_CFG *get_abuf_cfg_by_link(u8 abuf_link_id)
{
    if (is_valid_abuf_link(abuf_link_id))
        return &abuf_link_cfg_set.at_abuf_link_list[abuf_link_id];
    else
        return AIC_NULL;
}

/* Enable the +CEREG URC via AT+CEREG=2. */
s32 enable_cereg_urc(u8 cereg_mode, aic_get_cp_resp_callback cb)
{
    u8 at_str[16] = {'\0'};

    snprintf(at_str, sizeof(at_str), "AT+CEREG=%d", cereg_mode);
    aic_send_cmd_with_callback(at_str, cb);

    return AIC_SUCCESS;
}

u8 get_ready_abuf_link(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 idle_link_id = AT_LINK_CNT, tmp_link_id = AT_LINK_CNT;

    /** Pick up the idle abuf link to send AT cmds.
     * If no link is idle,then the AT cmd is pending.
     */
    if (at_req_resp_ptr)
        tmp_link_id = at_req_resp_ptr->assigned_link_id;

    if (is_valid_abuf_link(tmp_link_id) &&
        abuf_link_cfg_set.at_abuf_link_list[tmp_link_id].link_state
        == AT_ABUF_LINK_RESERVED) {
        aicx_info(0x0cbde1, "d", "get assigned link %d\n", tmp_link_id);
        idle_link_id = tmp_link_id;
    } else {
        idle_link_id = get_idle_link();
    }

    return idle_link_id;
}

u8 aic_get_abuf_link_state(u8 link_id)
{
    return abuf_link_cfg_set.at_abuf_link_list[link_id].link_state;
}

void aic_set_abuf_link_reserved(u8 link_id)
{
    if (is_valid_abuf_link(link_id))
        abuf_link_cfg_set.at_abuf_link_list[link_id].link_state = AT_ABUF_LINK_RESERVED;
}

void aic_set_abuf_link_idle(AT_ABUF_LINK_CFG *abuf_link_cfg)
{
    u8 link_id;
    u8 is_time_out;
    AT_REQ_RESP *at_ptr;
    boolean current_cmd_is_abort = AIC_FALSE;

    if (!abuf_link_cfg || !is_valid_abuf_link(abuf_link_cfg->link_id)) {
        return;
    }

    link_id = abuf_link_cfg->link_id;
    is_time_out = abuf_link_cfg->is_link_timeout;
    at_ptr = abuf_link_cfg->at_req_resp;

    abuf_link_cfg->at_req_resp = AIC_NULL;
    abuf_link_cfg->activated_pdp_cid = 0;
    abuf_link_cfg->resp_complete = AT_RESP_NOT_COMPLETED;
    abuf_link_cfg->link_state = AT_ABUF_LINK_IDLE;
    abuf_link_cfg->is_link_timeout = AT_ABUF_NOT_TIMEOUT;
    if (link_id != AT_LINK0 && abuf_link_cfg->rev_buf) {
        aos_free(abuf_link_cfg->rev_buf);
        abuf_link_cfg->rev_buf = AIC_NULL;
    }

    /* Check if it is the abort cmd. */
    if (at_ptr && at_ptr->at_req && strcasestr(at_ptr->at_req, "AT^ACLLK"))
        current_cmd_is_abort = AIC_TRUE;

    /* If the abort cmd timeout,then not send abort cmd again. */
    if (is_time_out == AT_ABUF_TIMEOUT && !current_cmd_is_abort) {
        abuf_link_cfg->link_state = AT_ABUF_LINK_RESERVED;
        aos_set_event(event_send_at, AT_EVENT_ABORT_LINK, AOS_OR);
        return;
    }

    /* The link is idle.Ready to send the first pending at cmd. */
    if (at_pending_to_send)
        aos_set_event(event_send_at, AT_EVENT_SEND_CMD_TO_CP, AOS_OR);

    return;
}

u8 get_idle_link(void)
{
    AT_ABUF_LINK_CFG *abuf_link_cfg;
    u8 i;

    /* Exclude link 0 for URC. */
    for (i = AT_LINK1; i < AT_LINK5; i++) {
        abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
        if (abuf_link_cfg->link_state == AT_ABUF_LINK_IDLE) {
            return i;
        }
    }

    return AT_LINK_CNT;
}

boolean is_valid_abuf_link(u8 link_id)
{
    return (link_id < AT_LINK_CNT);
}

u32 write_at_to_mbuf(AT_REQ_RESP *at_req_resp, u8 link_id)
{
    s32 count;
    u32 at_len, status = AIC_SUCCESS;

    aicx_info(0x11453a, "ds", "write to abuf link %d  %s\n", link_id, at_req_resp->at_req);
    at_len = strlen(at_req_resp->at_req);
    /* Set the timeout to make it return. */
    count = abuf_write(abuf_handle, link_id,
                       at_req_resp->at_req, at_len,
                       AT_WRITE_ABUF_WAIT_TIME);
    if ((count < 0) || (count != at_len)) {
        aicx_error(0x0aaade, "dd", "write to link--%d fail,count=%d\n", link_id, count);
        status = AT_ERROR_WRITE_LINK_FAIL;
    }

   return status;
}

#ifdef CONFIG_AIC_SYSOFF
void wait_ps_stop(void *ptr)
{
    aicx_info(0x040d9c, "d", "wait ps stop:cp state %d\n", aic_get_cp_state());
    if (aic_get_cp_state() == SUBSYS_STATE_ALIVE) {
        /* Not care the resp content,so input null,not saving the resp. */
        aic_send_cmd_and_wait("AT+CFUN=0", AIC_NULL);
        aicx_info(0x134a4b, "", "cfun0 return\n");
    }
}
#endif

u32 aic_register_cp_urc_callback(aic_get_cp_urc_callback cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (!cp_urc_cbs[i]) {
            cp_urc_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return AT_ERROR_URC_CBS_OVERFLOW;
}

void aic_unregister_cp_urc_callback(aic_get_cp_urc_callback cb)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (cp_urc_cbs[i] == cb) {
            cp_urc_cbs[i] = AIC_NULL;
            return;
        }
    }
}

void invoke_cp_urc_cbs(u8 *urc_str)
{
    u8 i;

    for (i = 0; i < AT_MAX_CP_URC_CBS; i++) {
        if (cp_urc_cbs[i])
            cp_urc_cbs[i](urc_str);
    }
}

