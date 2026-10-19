/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis for handling AT cmds
 *    relating to usb network share.
 */

#ifndef AIC_USB_SHARE_AT_HANDLE_H
#define AIC_USB_SHARE_AT_HANDLE_H

#define ANET_WAIT_USB_INTF_COUNT 25

#ifdef CONFIG_CUSTOMER_AT
#define ANET_USB_DIAL_ENABLED 0
#define ANET_USB_DIAL_DISABLED 1
#define ANET_USB_INTF_RNDIS_MODE 0
#define ANET_USB_INTF_ECM_MODE 1
#define ANET_USB_NET_PUBLIC_IP 0
#define ANET_USB_NET_PRIVATE_IP 1

#define ANET_SET_BY_USER 1
#define ANET_NOT_SET_BY_USER 0

typedef struct CUSTOMER_USB_NET_CFG_STRUCT
{
    u8 dial_enabled;
    u8 usb_intf_mode;
    u8 ip_mode;
    u8 third_byte_set;
    u8 lan_ip_third_byte;
    u8 dhcp_start_set;
    u8 dhcp_ip_start;
    u8 dhcp_end_set;
    u8 dhcp_ip_end;
} CUSTOMER_USB_NET_CFG;
#endif

#ifdef CONFIG_CM_USB_DIAL_AT
#define AT_DIALUP_CFG "AT+MDIALUPCFG"
#define AT_DIALUP "AT+MDIALUP"

#define RESP_DIALUP "+MDIALUP: "
#define RESP_DIALUP_CFG "+MDIALUPCFG: "

#define ANET_DIAL_CONFIG_INTF_MODE 1
#define ANET_DIAL_CONFIG_AUTO_FLAG 2
#define ANET_DIAL_CONFIG_WORK_MODE 3

#define ANET_DIAL_PRIVATE_WORK_MODE 0
#define ANET_DIAL_PUBLIC_WORK_MODE 1
#define ANET_AUTO_DIAL_DISABLED 0
#define ANET_AUTO_DIAL_ENABLED 1
#define ANET_DIAL_CONNECT_ENABLED 1
#define ANET_DIAL_CONNECT_DISABLED 0

typedef struct CUSTOMER_M_DIAL_CFG_STRUCT
{
    u8 auto_flag;
    u8 intf_mode;
    u8 work_mode;
    u8 connect_enabled;
    boolean delay_report_info;
} CUSTOMER_M_DIAL_CFG;
#endif

/* Define global functions. */
#ifdef CONFIG_GX_USB_DIAL_AT
u32 aic_handle_usbtether_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_usbtether_query_cmd(AT_REQ_RESP *at_req_res_ptr);
#endif
boolean aic_is_valid_usb_intf(u8 *usb_intf_name);
void aic_wait_usb_intf_up(u8 *usb_intf, u16 *usb_intf_up);
void aic_wait_usb_intf_down(u8 *usb_intf, u8 usb_mode, u16 *usb_intf_down);
#ifdef CONFIG_CUSTOMER_AT
void aic_init_customer_net_cfg(NET_CFG *net_cfg);
u32 aic_handle_dialmode_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_dialmode_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_dialmode_query_cmd(AT_REQ_RESP *at_req_res_ptr);
boolean aic_is_usb_net_share_enabled(void);
u32 aic_handle_myconfig_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_myconfig_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_myconfig_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u8 aic_get_usb_intf_mode(void);
boolean aic_is_usb_dial_enabled(void);
boolean aic_is_usb_dial_ip_set(void);
u32 aic_handle_usbnetip_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_usbnetip_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_usbnetip_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u8 aic_get_cus_dial_ip_mode(void);
#endif

#ifdef CONFIG_CM_USB_DIAL_AT
u32 aic_handle_mdialupcfg_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_mdialupcfg_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_mdialupcfg_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_mdialup_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_mdialup_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_mdialup_query_cmd(AT_REQ_RESP *at_req_res_ptr);
void aic_auto_mdialup_start(u8 intf_mode);
boolean aic_is_mdialup_connect_enabled(void);
void aic_read_init_mdial_cfg(void);
void aic_transmit_mdialup_connect_info(u32 at_port_id, boolean is_urc);
#endif
void aic_connect_usb_intf(u8 *intf_name);
void aic_disconnect_usb_intf(u8 *intf_name);
u32 aic_usb_intf_single_get_state(u8 *intf_name);

#endif /* AIC_USB_SHARE_AT_HANDLE_H */
