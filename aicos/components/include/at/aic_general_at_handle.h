/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis for handling general AT cmds.
 */

#ifndef AIC_GENERAL_AT_HANDLE_H
#define AIC_GENERAL_AT_HANDLE_H

#include "aic_at_process_common.h"

#define AT_CALL_CLIP_ENABLED 1
#define AT_CALL_CLIP_DISABLED 0

#define AT_CALL_CCWA_DISABLED 0
#define AT_CALL_CCWA_ENABLED 1

#define AT_REG_REAL_NET 0
#define AT_REG_INSTRU_NET 1
#define AT_REG_LOW_POWER 2
#define AT_REG_CORE_NET_LOW_POWER 3

#define AT_URC_PORT_NAME_LEN 12

#define AT_URC_RI_SIGNAL_TYPE_LEN 12

#define AT_URC_RING_IND_OFF 0
#define AT_URC_RING_IND_ON 1

#define AT_URC_DEFAULT_PULSE_CNT 1
#define AT_URC_MIN_PULSE_CNT 1
#define AT_URC_MAX_PULSE_CNT 5

/* The unit is ms. */
#define AT_URC_DEFAULT_PULSE_DURATION 120
#define AT_URC_MIN_PULSE_DURATION 1
#define AT_URC_MAX_PULSE_DURATION 2000

/* The unit is ms. */
#define AT_URC_DEFAULT_DELAY_TIME 0
#define AT_URC_MIN_DELAY_TIME 0
#define AT_URC_MAX_DELAY_TIME 120

#define AT_URC_CACHE_DISABLED 0
#define AT_URC_CACHE_ENABLED 1

#define AT_URC_CACHE_MAX_LINE_NUM 20
#define AT_URC_CACHE_ONE_LINE_SIZE 40

#define AT_MSG_CONTENT_LEN SIZE_1K

#define AT_DATA_SPEED_CTRL_NUM 4

#define AT_CMD_ECHO_ENABLED 1
#define AT_CMD_ECHO_DISABLED 0

#ifdef CONFIG_D2D
#define AT_UE_MODE_D2D 1
#define AT_UE_MODE_MOBILE 0

/* 80 bit */
#define AT_D2D_DEFAULT_PKT_SIZE 80
/* 10 bytes */
#define AT_D2D_MSG_MAX_LEN 10

/* Wait for 100ms to exit data mode. */
#define AT_D2D_DATA_MODE_TIMEOUT 100

#define AT_D2D_ROLE_SEND 0
#define AT_D2D_ROLE_RECV 1
#define AT_D2D_ROLE_SEND_RECV 3

typedef struct D2D_MODE_CFG_STRUCT
{
    u8 mode;
    u8 keep_sending;
    u8 enable_pkt_dump;
    u8 enable_rep_dr;
    u32 send_pkt_num;
    u32 pkt_size;
    u32 at_com_id;
    u32 data_mode_total_len;
    u32 recved_data_len;
    u8 *total_data_ptr;
    u8 role;
    u8 is_transparent_mode;
    u8 msg_head_enabled;
    u32 urc_com_id;
    u8 at_com_type[8];
} D2D_MODE_CFG;
#endif

typedef struct URC_CFG_STRUCT
{
    u32 urc_port_id;
    u8 usb_port_name[AT_URC_PORT_NAME_LEN];
    u8 uart_port_name[AT_URC_PORT_NAME_LEN];
    u8 ri_signal_type[AT_URC_RI_SIGNAL_TYPE_LEN];
    u8 ri_enabled;
    u8 pulse_cnt;
    u16 pulse_duration;
    u8 ri_delay_time;
    u8 cache_enabled;
    u8 uart_urc_line_cnt;
    u8 usb_urc_line_cnt;
    u8 *urc_cache_ptr;
    aos_queue_ptr uart_urc_cache_queue;
    aos_queue_ptr usb_urc_cache_queue;
    aos_mutex_ptr urc_cache_mutex;
} URC_CFG;

typedef struct MSG_TEXT_STRUCT
{
    u8 *text_buf;
    u16 text_len;
} MSG_TEXT;

typedef struct DATA_SP_CTRL_STRUCT
{
    u8 active;
    u8 interface_index;
    u32 dl_max_speed;
    u32 ul_max_speed;
} DATA_SP_CTRL;

typedef struct FASTSLP_CFG_STRUCT
{
    u8 slp_enabled;
    u32 idle_time;
    u32 retry_time;
} FASTSLP_CFG;

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
typedef struct MIPCALL_CFG_STRUCT
{
    u8 mipcall_operation;
    boolean urc_via_cgev;  /* Whether invoke URC +MIPCALL via +CGEV */
    boolean act_by_mipcall;
    boolean act_as_ims;
} MIPCALL_CFG;
#endif

/* Define global functions. */
#ifndef CONFIG_AT_PROC_MINI
u32 aic_handle_ifc_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ifc_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ifc_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ipr_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ipr_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ipr_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmi_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmi_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmm_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmm_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmr_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cgmr_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ati_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ati_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cclk_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cclk_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_cclk_query_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
#ifdef CONFIG_USB_SERVICE
u32 aic_handle_usbcfg_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_usbcfg_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_usbcfg_test_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
boolean aic_is_usb_in_net_mode(void);
u32 aic_handle_regmode_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_regmode_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_regmode_test_cmd(AT_REQ_RESP *at_req_ptr);
u8 aic_get_reg_mode(void);
void aic_set_reg_mode(u8 reg_mode);
#ifndef CONFIG_AT_PROC_MINI
u32 aic_handle_autostartps_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_autostartps_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_autostartps_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_autousbnet_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_autousbnet_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_autousbnet_set_cmd(AT_REQ_RESP *at_req_ptr);
#endif
u32 aic_handle_s4_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_s3_set_cmd(AT_REQ_RESP *at_req_ptr);
#if defined CONFIG_CUSTOMER_AT || defined CONFIG_CUSTOMER_JQ_AT
u32 aic_handle_qurccfg_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_qurccfg_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_qurccfg_test_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
#ifdef CONFIG_CUSTOMER_JQ_AT
u32 aic_handle_qcfg_ri_set_cmd(AT_REQ_RESP *at_req_ptr);
void aic_send_ri_signal_for_urc(u32 urc_com_id);
#endif
#ifdef CONFIG_CUSTOMER_AT
u32 aic_handle_qiact_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qiact_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qiact_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qideact_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qideact_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qcfg_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qcfg_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qicsgp_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_qicsgp_test_cmd(AT_REQ_RESP *at_req_res_ptr);
boolean aic_is_urc_ri_enabled(void);
void aic_send_urc_gpio_signal(void);
u32 aic_handle_iprex_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_iprex_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_iprex_test_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
u32 aic_get_configed_urc_port_id(void);
#ifdef CONFIG_NET_PCAP_DUMP
u32 aic_handle_capapdata_exe_cmd(AT_REQ_RESP *at_req_resp);
#endif
#if CONFIG_CUSTOMER_AT || CONFIG_CUSTOMER_JQ_AT
void aic_init_urc_cfg(void);
#endif
u32 aic_handle_cmgs_exe_cmd(AT_REQ_RESP *at_req_resp);
#ifdef CONFIG_CUSTOMER_AT
boolean aic_is_urc_cache_enabled(void);
void aic_reset_urc_cache_mem(void);
void aic_add_urc_to_cache(u8 *urc_str);
void aic_send_urc_cached(u32 com_id);
boolean aic_need_flush_urc_cache(u32 com_id);
void aic_set_if_cache_urc(u8 enable_cache);
void aic_get_at_port_name(u32 at_port_id, u8 *port_name_buf, u8 buf_len);
u32 aic_handle_ato_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
u32 aic_handle_csub_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
#ifdef CONFIG_NET_EXTERNEL_API
u32 aic_handle_spctrl_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_spctrl_query_cmd(AT_REQ_RESP *at_req_ptr);
#endif
#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
u32 aic_handle_mipcall_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mipcall_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mipcall_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mueconfig_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mueconfig_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mtsetid_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_mtsetid_set_cmd(AT_REQ_RESP *at_req_ptr);
void aic_set_cereg_mode(u8 cereg_mode);
#endif
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
void aic_set_ipcall_act_flag(u8 pdp_cid);
#endif
#ifndef CONFIG_AT_PROC_MINI
u32 aic_handle_meminfo_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_fastslp_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_fastslp_query_cmd(AT_REQ_RESP *at_req_ptr);
void aic_init_fastslp_cfg(void);
u32 aic_handle_cardstatus_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_cardstatus_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_cardstatus_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ctzu_test_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ctzu_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ctzu_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ctcc_mode_set_cmd(AT_REQ_RESP *at_req_ptr);
#ifdef CONFIG_NX_NAT
u32 aic_handle_tcpwin_set_cmd(AT_REQ_RESP *at_req_ptr);
#endif
u32 aic_handle_cgsn_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_v0_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_q1_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_cfun_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ate_set_cmd(AT_REQ_RESP *at_req_ptr);
s32 aic_get_cclk_time_zone(void);
u32 aic_handle_cstm_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_qcellex_exe_cmd(AT_REQ_RESP *at_req_ptr);
#endif
u32 aic_commom_handle_cp_cmd(AT_REQ_RESP *at_req_ptr);
#ifdef CONFIG_GX_AT_V2
u32 aic_handle_gnetmode_set_cmd(AT_REQ_RESP *at_req_ptr);
#endif

u32 aic_handle_disable_net_copy_set_cmd(AT_REQ_RESP *at_req_ptr);
u8 aic_is_at_echo_enabled(void);
#ifdef CONFIG_D2D
u32 aic_handle_d2dbw_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_d2dbw_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dmode_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dmode_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dpktsize_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dpktsize_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dsrawdata_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dstopsend_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dsendmsg_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dpktdump_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dpktdump_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_drepfc_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_drepdr_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_drepdr_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dsvoidata_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dvoistub_exe_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_drole_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_drole_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_d2d_report_downlink_data(u8 *data_ptr, u32 data_len);
u32 aic_handle_ddbgcfg_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_ddbgcfg_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_get_d2d_urc_com_id(void);
void aic_set_d2d_urc_com_id(u32 port_id);
void aic_set_urc_msg_head_enabled(u8 enabled);
u8 aic_is_urc_msg_head_enabled(void);
#endif

#endif /* AIC_GENERAL_AT_HANDLE_H */
