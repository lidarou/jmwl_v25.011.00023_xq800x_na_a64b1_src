/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef NET_MGR_H_
#define NET_MGR_H_

//#include "host_cmd_engine_priv.h"
#include <host_apis.h>
#include <netstack.h>

#define WLAN_IFNAME "wlan0"

//#define NET_MGR_AUTO_RETRY

//#define NET_MGR_AUTO_JOIN

typedef enum link_status_en {
    LINK_DOWN           = 0,
	LINK_UP             = 1
} link_status;

typedef enum e_netmgr_msg
{
    MSG_SCAN_REQ, // when station mode, save ap list
    MSG_JOIN_REQ, // when station mode and connect ok, if dhcpc flag is 1, netmgr will start dhcpc enable, or do nothing.
    MSG_JOIN_OTHER_REQ,
    MSG_LEAVE_REQ,
    MSG_CONTROL_REQ,
    MSG_SWITCH_REQ,
    MSG_VIF_OFF_REQ,

    MSG_SCAN_DONE,
    MSG_SCAN_RESULT, // when station mode, save ap list
    MSG_JOIN_RESULT, // when station mode and connect ok, if dhcpc flag is 1, netmgr will start dhcpc enable, or do nothing.
    MSG_LEAVE_RESULT, // when station mode
    //MSG_DHCPD_CHANGE, // when ap mode, enable or disable dhcpd.
    //MSG_DHCPC_CHANGE, // when station mode, set dhcpc flag 1 or 0.
    MSG_AP_IP_CHANGE, // when ap mode, user change ip info, netmgr need to restart dhcpd
    MSG_PS_SETUP_OK,
    MSG_PS_WAKENED,
    MSG_VIF_UP,
}netmgr_msg;

typedef enum{
	WIFI_SEC_NONE,
	WIFI_SEC_WEP,
	WIFI_SEC_WPA_PSK,		//8~63	ASCII
	WIFI_SEC_WPA2_PSK,		//8~63	ASCII
	WIFI_SEC_WPS,
	WIFI_SEC_MAX
}wifi_sec_type;

typedef struct st_ipinfo
{
    ssv_type_u32 ipv4;
    ssv_type_u32 netmask;
    ssv_type_u32 gateway;
    ssv_type_u32 dns;
}ipinfo;


typedef ssv6xxx_hw_mode wifi_mode;
typedef Ap_setting ssv_wifi_ap_cfg;
typedef Sta_setting ssv_wifi_sta_cfg;
typedef Ap_sta_status wifi_info;

typedef struct st_wifi_sta_join_cfg
{
    //wifi_sec_type    sec_type;
    struct cfg_80211_ssid ssid;
    ssv_type_u8                  password[MAX_PASSWD_LEN+1];
    ssv_type_u8      pmk_valid;
    ssv_type_u8      pmk[32];
    ssv_type_u32     def_ch_mask;
    ssv_type_u32     def_5g_ch_mask;
}wifi_sta_join_cfg;

typedef struct st_netmgr_cfg
{
    ssv_type_u32 ipaddr;
    ssv_type_u32 netmask;
    ssv_type_u32 gw;
    ssv_type_u32 dns;
    struct st_dhcps_info dhcps;
    ssv_type_bool s_dhcpd_enable;
    ssv_type_bool s_dhcpc_enable;
    
    ssv_type_bool s_dhcpd_status;
    ssv_type_bool s_dhcpc_status;
}netmgr_cfg;


typedef void (*ifup_handler)(ssv_type_u8 *ipaddr, ssv_type_u8 *mac);


extern void ssv_netmgr_init(ssv_type_bool default_cfg);
extern void netmgr_cfg_get(netmgr_cfg *p_cfg,ssv_type_u8 vif_idx);
extern void netmgr_cfg_set(netmgr_cfg *p_cfg,ssv_type_u8 vif_idx);
extern int netmgr_igmp_enable(ssv_type_bool on);
extern int netmgr_ipinfo_get(char *ifname, ipinfo *info);
extern int netmgr_ipinfo_set(ssv_type_u8 vif_idx, ipinfo *info, ssv_type_bool auto_dhcpd_info);
extern void netmgr_netif_status_set(ssv_type_bool on,ssv_type_u8 vif_idx);
extern int netmgr_dhcps_info_set(dhcps_info *if_dhcps,ssv_type_u8 vif_idx);
extern int netmgr_dhcpc_set(ssv_type_bool dhcpc_enable, ssv_type_u8 vif_idx);
extern int netmgr_dhcpd_set(ssv_type_bool dhcpd_enable, ssv_type_u8 vif_idx);
extern int netmgr_dhcp_status_get(ssv_type_bool *dhcpd_status, ssv_type_bool *dhcpc_status, ssv_type_u8 vif_idx);
extern int netmgr_dhcd_ipmac_get(dhcpdipmac *ipmac, int *size_count);
extern int netmgr_wifi_vif_set_ip_async(ssv_type_u8 vif_idx, ipinfo* ip);
extern void netmgr_netif_link_set(ssv_type_bool on,ssv_type_u8 vif_idx);

extern int netmgr_hwmac_get(char *ifname, void *mac);
extern ssv_type_bool netmgr_wifi_is_connected(ssv6xxx_hw_mode hmode);
extern int netmgr_wifi_mode_get(wifi_mode *mode, ssv_type_bool *status, ssv_type_u8 vif_idx);
extern int netmgr_wifi_info_get(Ap_sta_status *info, ssv_type_u8 vif_idx);
extern ssv_type_bool netmgr_wifi_check_mac(unsigned char * mac);
extern ssv_type_bool netmgr_wifi_check_sniffer(void);
extern ssv_type_bool netmgr_wifi_check_sta(void);
extern ssv_type_bool netmgr_wifi_check_ap(void);
extern ssv_type_bool netmgr_wifi_check_sta_connected(void);

//Join a AP that is already in the AP list
extern int netmgr_wifi_join_async(wifi_sta_join_cfg *join_cfg);

//Join a AP that is not in the AP list, this function will do the scanning first, and then join. It cause more times on running time.
extern int netmgr_wifi_join_other_async(wifi_sta_join_cfg *join_cfg);
extern int netmgr_wifi_leave_async(void);

extern int netmgr_wifi_control_async(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, ssv_wifi_sta_cfg *sta_cfg);
extern int netmgr_wifi_switch_async(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_join_cfg *join_cfg);
extern int netmgr_wifi_sta_on_async(void);
extern int netmgr_wifi_sta_off_async(void);
extern int netmgr_wifi_sniffer_on_async(void);
extern int netmgr_wifi_sniffer_off_async(void);

extern int netmgr_wifi_ap_on_async(ssv_wifi_ap_cfg *ap_cfg);
extern int netmgr_wifi_ap_off_async(void);
extern int netmgr_wifi_get_ap_cfg(ssv_wifi_ap_cfg *ap_cfg);

//If you assign th 0 to channel_mask, we use the defulat value.
//The default value is defined in DEFAULT_STA_CHANNEL_MASK
extern ssv_type_u16 g_sta_channel_mask;
extern ssv_type_u32 g_sta_5g_channel_mask;
#define SCAN_ALL_2G_CHANNEL 0xFFFF //Scan the all 2G channels that are marked in the g_sta_channel_mask
#define SCAN_ALL_5G_CHANNEL 0xFFFFFFFF //Scan the all 5G channels that are marked in the g_sta_channel_mask
extern int netmgr_wifi_scan_async(ssv_type_u16 channel_mask, char *ssids[], int ssids_count);
extern int netmgr_wifi_scan_ex_async(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask, char *ssids[], int ssids_count);
ssv_type_s32 netmgr_wifi_suspend(ssv_type_u32 ip, ssv_type_u32 powermode);
ssv_type_s32 netmgr_wifi_resume(void);

#ifdef  NET_MGR_AUTO_JOIN
extern void netmgr_apinfo_clear();
extern void netmgr_apinfo_remove(char *ssid);
extern void netmgr_apinfo_show();

#endif


void netmgr_wifi_link_register_cb(netdev_link_callback_t link_up_cb, netdev_link_callback_t link_down_cb);
ssv_type_s32 netmgr_reg_ifup_callback(ssv_type_u8 vif_idx, ifup_handler ifup_cb);
ssv_type_s32 netmgr_unreg_ifup_callback(ssv_type_u8 vif_idx);
#if(ENABLE_WPS==1)
ssv_type_s32 netmgr_wifi_wps_pbc(ssv_type_bool bEnrollee);
#endif
#endif //NET_MGR_H_
