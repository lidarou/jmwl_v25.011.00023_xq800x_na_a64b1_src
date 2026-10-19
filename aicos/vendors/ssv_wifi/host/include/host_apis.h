/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _HOST_APIS_H_
#define _HOST_APIS_H_
//#include <porting.h>

#undef PRINTF_FORMAT

#include <host_config.h>
#include <ssv_lib.h>

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif /* _MSC_VER */
#include <ssv_common.h>
#include <ssv_ether.h>
#include <ssv_hal.h>
#include <host_apis.h>

#define SSV6XXX_APLIST_PAGE_COUNT  10

enum TX_DSCRP_CONF_TYPE {
	TX_DSCRP_SET_EXTRA_INFO    = BIT(0),
	TX_DSCRP_SET_BC_QUE        = BIT(1)
};
typedef enum{
  HOST_API_DEACTIVE,
  HOST_API_ACTIVE,
  HOST_API_PWR_SAVING
}HOST_API_STATE;

typedef enum{
  HOST_PS_SETUP,
  HOST_PS_START,
  HOST_PS_WAKEUP_OK
}HOST_PS_STATE;

typedef enum{
  HOST_PS_INFRASTRUCTURE,
  HOST_PS_AP_ECO,
  HOST_PS_MODE_END
}HOST_PS_MODE;

enum AMPDU_OPT
{
    AMPDU_TX_OPT_ENABLE,
    AMPDU_TX_OPT_SET_RETRY_MAX,
    AMPDU_RX_OPT_ENABLE,
    AMPDU_RX_OPT_BUF_SIZE    
};


//#define IS_TX_DSCRP_SET_EXTRA_INFO(x)        ((x) & (1<<TX_DSCRP_SET_EXTRA_INFO) )
//#define IS_TX_DSCRP_SET_BC_QUE(x)            ((x) & (1<<TX_DSCRP_SET_BC_QUE)   )


//#define SET_TX_DSCRP_SET_EXTRA_INFO(x, _tf)     (x) = (((x) & ~(1<<TX_DSCRP_SET_EXTRA_INFO))	| ((_tf)<<TX_DSCRP_SET_EXTRA_INFO) )
//#define SET_TX_DSCRP_SET_BC_QUE(x, _tf)      	(x) = (((x) & ~(1<<TX_DSCRP_SET_BC_QUE))	| ((_tf)<<TX_DSCRP_SET_BC_QUE)   )

#if 1
//-----------------------------------------------------------
//For Host AP API
enum conn_status {
    IDLE,
    DISCONNECT=IDLE,
    SCANNING,
    PRE_SCAN,
    AUTH,
    ASSOC,
    WPS_REGISTRATION,
    EAPOL,
    CONNECT
};

struct conn_info {
    ssv_type_u8                  Mac[ETHER_ADDR_LEN];    //00:11:22:33:44:55
    enum conn_status    status;                 //Auth,Assoc,Eapol
};
enum channel_select{
    EN_CHANNEL_0=0,
    EN_CHANNEL_1,
    EN_CHANNEL_2,
    EN_CHANNEL_3,
    EN_CHANNEL_4,
    EN_CHANNEL_5,
    EN_CHANNEL_6,
    EN_CHANNEL_7,
    EN_CHANNEL_8,
    EN_CHANNEL_9,
    EN_CHANNEL_10,
    EN_CHANNEL_11,
    EN_CHANNEL_12,
    EN_CHANNEL_13,
    EN_CHANNEL_14,
    EN_CHANNEL_15,

    EN_CHANNEL_36=36,
    EN_CHANNEL_40=40,
    EN_CHANNEL_44=44,    
    EN_CHANNEL_48=48,
    
    EN_CHANNEL_52=52,
    EN_CHANNEL_56=56,
    EN_CHANNEL_60=60,    
    EN_CHANNEL_64=64,

    EN_CHANNEL_100=100,
    EN_CHANNEL_104=104,
    EN_CHANNEL_108=108,    
    EN_CHANNEL_112=112,

    EN_CHANNEL_116=116,
    EN_CHANNEL_120=120,
    EN_CHANNEL_124=124,    
    EN_CHANNEL_128=128,

    EN_CHANNEL_132=132,    
    EN_CHANNEL_136=136,    
    EN_CHANNEL_140=140,
    EN_CHANNEL_149=149,

    EN_CHANNEL_153=153,    
    EN_CHANNEL_157=157,    
    EN_CHANNEL_161=161,
    EN_CHANNEL_165=165,
    
    EN_CHANNEL_AUTO_SELECT
};

enum channel_bandwidth{
    EN_CH_BW_AUTO=0,
    EN_CH_BW_HT20,
    EN_CH_BW_HT40
};

typedef enum t_ENANGY_THRESHOLD
{
	MINUS_88DB=0,
	MINUS_85DB,
	MINUS_82DB,
	MINUS_79DB,
	MINUS_76DB,
	MINUS_73DB,
	MINUS_72DB,
	EDCCA_OFF
} ENANGY_THRESHOLD;

typedef enum ap_setting_acl_type
{
	AP_ACL_DISABLE=0,
    AP_ACL_BLACKLIST=1,
    AP_ACL_WHITELIST=2
} AP_SETTING_ACL_TYPE;

typedef struct ap_setting_acl_mac {
    ssv_type_u8           mac[ETHER_ADDR_LEN]; 
}AP_SETTING_ACL_MAC;

struct ap_setting {
    ssv_type_u32 						proto;              //WPA_PROTO_RSN
    ssv_type_u32 						key_mgmt;           //WPA_KEY_MGMT_PSK
    ssv_type_u32 						pairwise_cipher;	//WPA_CIPHER_XXX
    ssv_type_u32 						group_cipher;   	//WPA_CIPHER_XXX
    ssv_type_bool                       status;            //on,off
    ssv_type_u8                         password[MAX_PASSWD_LEN+1];     //12345
    ssv_type_u8                         static_ip;
    ssv_type_u8                         step;
    enum channel_select                 channel;
    enum channel_bandwidth              ch_bw;
    ssv6xxx_sec_type                    security;
    struct cfg_80211_ssid               ssid;               //i-comm
    ssv_type_u8                         is_hidden;
    ssv_type_u8                         max_sta_count;
};

typedef struct ap_setting Ap_setting;
typedef struct sta_setting {
    ssv_type_bool                status;                         //on,off
    ssv6xxx_hw_mode     mode;
} Sta_setting;

struct ap_sta_status_ap{
    ssv_type_u8                          selfmac[ETHER_ADDR_LEN];       //50:11:22:33:44:55
    ssv_type_u8                          channel;
    struct cfg_80211_ssid       ssid;               //i-commap
    ssv_type_u32                         stanum;             //0,1,2
    struct conn_info            stainfo [AP_SUPPORT_CLIENT_NUM];
    ssv_type_u32                         proto;            //WPA_PROTO_RSN
    ssv_type_u32                         key_mgmt;         //WPA_KEY_MGMT_PSK
    ssv_type_u32                         pairwise_cipher;  //WPA_CIPHER_XXX
    ssv_type_u32                         group_cipher;     //WPA_CIPHER_XXX
    ap_state                    current_st;
};
struct ap_sta_status_station{
    ssv_type_u8                          selfmac[ETHER_ADDR_LEN];       //50:11:22:33:44:55
    ssv_type_u8                          channel;
    struct cfg_80211_ssid       ssid;               //i-commap
    struct conn_info            apinfo;
    ssv_type_u16                         capab_info;
    ssv_type_u32                         proto;              //WPA_PROTO_RSN
    ssv_type_u32                         key_mgmt;           //WPA_KEY_MGMT_PSK
    ssv_type_u32                         pairwise_cipher;    //WPA_CIPHER_XXX
    ssv_type_u32                         group_cipher;       //WPA_CIPHER_XXX
};

typedef struct ap_sta_status {
    ssv_type_bool          status;
    ssv6xxx_hw_mode            vif_operate[MAX_VIF_NUM];
    union {
        struct ap_sta_status_ap ap_status;
        struct ap_sta_status_station station_status;
    }vif_u[MAX_VIF_NUM];
}Ap_sta_status ;


#endif


/**
 * Define the error numbers of wifi host api:
 *
 * @ SSV6XXX_SUCCESS:
 * @ SSV6XXX_FAILED:
 */

typedef enum{
    SSV6XXX_SUCCESS                     =     0,
    SSV6XXX_FAILED                      =    -1,
    SSV6XXX_INVA_PARAM                  =    -2,
    SSV6XXX_NO_MEM                      =    -3,
    SSV6XXX_QUEUE_FULL                  =    -4,
    SSV6XXX_WRONG_HW_MODE_CMD           =    -5,
    SSV6XXX_WRONG_CHIP_ID               =    -6

}ssv6xxx_result;

struct apmode_sta_info {

    //Mac Adderss ETHER_ADDR
    ssv_type_u8 addr[ETH_ALEN];

    /* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
    ssv_type_u16 aid; 

    //ARP retry count
    ssv_type_u8 arp_retry_count;

    //record RCPI value
    ssv_type_u8 rssi;
    ssv_type_u32 prev_rssi;
};

typedef enum {
    EN_HOST_API_SNIFFER_MASK_BEACON                     = 0x00000001,
    EN_HOST_API_SNIFFER_MASK_MGMT                       = 0x00000002,
    EN_HOST_API_SNIFFER_MASK_BROADCAST_DATA             = 0x00000004,
    EN_HOST_API_SNIFFER_MASK_DATA                       = 0x00000008,
    EN_HOST_API_SNIFFER_MASK_ALL                        = 0xffffffff,
}en_host_api_sniffer_mask;

//=========================================
//                Public Command Data
//=========================================
/**
 *  struct cfg_ps_wow_request  - PWR saving request description
 *
 *  @ ipv4addr.
 *  @ ethtyp: ether type to filter
 */
struct cfg_ps_request {
    HOST_PS_STATE   host_ps_st;
    ssv_type_u32             ipv4addr;                   /*ipaddr*/
    ssv_type_u32             ethtyp[MAX_ETHTYPE_TRAP];   /* 8 ethtype filter*/
    ssv_type_u8              dtim_multiple;              /*listen dtim interval to multiple dtim period*/
    ssv_type_u8              mode;                       /*Host PS mode of AP or regular STA*/
    ssv_type_u16             RSVD;
} ;//__attribute__ ((packed));

/**
 *  struct cfg_scan_request  - scan request description
 *
 *  @ is_active: do active scan or passive scan
 *  @ n_ssids: the number of ssid to scan
 *  @ dwell_time: Time to wait for receiving beacon or probe response in each channel. (in 10ms).
 *  @ ssids: array of ssids to scan
 *  @ channel_mask: channel bit mask indicates which channels to scan
 *  @ ie_len: length of ie in octects
 *  @ ie_offset: optional information element(s) to add into probe request
 *                    frame.
 */
struct cfg_scan_request {
    ssv_type_u8      RSVD[1];
    ssv_type_bool    is_active;
    ssv_type_u8      n_ssids;
    ssv_type_u8      dwell_time;
    ssv_type_u16     channel_mask; /* channel 0 ~ channel 15 */
    ssv_type_u32     channel_5g_mask;     
    /* keep last */
    struct cfg_80211_ssid ssids[1];
} ;//__attribute__ ((packed));

//-------------------------------------------------------------------------------------------------------------------------------------------------
//HOST_CMD_JOIN

/**
 *  struct cfg_join_request - join request description
 */
struct cfg_join_request {
    ssv6xxx_sec_type    sec_type;
    int                 wep_keyidx;
    ssv_type_u8                  password[MAX_PASSWD_LEN+1];
    ssv_type_u8                  auth_alg;
#ifdef _INTERNAL_MCU_SUPPLICANT_SUPPORT_
    ssv_type_u8                  request_supplicant_bypass;
#endif
    ssv_type_u8                  no_bcn_timeout;
    struct ssv6xxx_ieee80211_bss bss;
    ssv_type_bool                ht_supported;
    ssv_type_u8                  ht_cap_ie[sizeof(struct ssv_ht_cap)]; /*STA's ht capability ie*/
    ssv_type_u32                 assoc_req_ie_len;
    ssv_type_u32                 pmk_valid;
    ssv_type_u8                 pmk[32];
#ifdef NOT_SUPPORT_ZERO_ARRAY
#else
     ssv_type_u8      assoc_req_ie[0];
#endif
} ;//__attribute__ ((packed));

//-------------------------------------------------------------------------------------------------------------------------------------------------
//HOST_CMD_LEAVE

/**
 *  struct cfg_leave_request - leave request description
 */
struct cfg_leave_request {
    //ETHER_ADDR      bssid;
    ssv_type_u16             info_len;
    ssv_type_u16             reason;
    ssv_type_u16             rsvd;
} ;//__attribute__ ((packed));

/*
  *struct cfg_addba_resp-DEL_BA cmd parameter
  */
struct cfg_delba {
    ssv_type_u16 tid;
    ssv_type_u16 initiator;
    ssv_type_u16 reason_code;
} ;//__attribute__ ((packed));

/*
  *struct cfg_addba_resp-ADDBA_RESP cmd parameter
  */
struct cfg_addba_resp {
    ssv_type_u8  dialog_token;
    ssv_type_u16 policy;
    ssv_type_u16 tid;
    ssv_type_u16 buf_size;
    ssv_type_u16 timeout;
    ssv_type_u16 status;
    ssv_type_u16 start_seq_num;
} ;//__attribute__ ((packed));


/************************************************************************************************/
/*                                 SSV  WIFI Command function							 		*/
/************************************************************************************************/


H_APIs ssv6xxx_result ssv6xxx_wifi_init(void);
H_APIs ssv6xxx_result ssv6xxx_wifi_deinit(ssv_type_u32 step);
H_APIs ssv6xxx_result ssv6xxx_wifi_pause(ssv_type_bool pause);

H_APIs ssv_type_s32 ssv6xxx_wifi_scan(struct cfg_scan_request *csreq);
H_APIs ssv_type_s32 ssv6xxx_wifi_join(struct cfg_join_request *cjreq);
H_APIs ssv_type_s32 ssv6xxx_wifi_leave(struct cfg_leave_request *clreq);
H_APIs void ssv6xxx_wifi_set_channel(ssv_type_u8 vif_idx, ssv_type_bool scan,ssv_type_bool home_ch, ssv_type_u8 ch_num, ssv_type_u8 ch_offset);
H_APIs void ssv6xxx_wifi_set_mrx_mode(ssv_type_u8 vif_idx, ssv_type_u8 mode);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_sniffer_config(ssv_type_u8 vif_idx, ssv_type_u32 mask, u32 max_len, u32 min_len, u8 *mac_addr);
H_APIs ssv6xxx_result ssv6xxx_wifi_apply_sniffer_config(ssv_type_u8 vif_idx);
//----------------------------I/O Ctrl-----------------------------



/************************************************************************************************/
/*                                  Data related function																				*/
/************************************************************************************************/

enum ssv6xxx_data_priority {
	ssv6xxx_data_priority_0,
	ssv6xxx_data_priority_1,
	ssv6xxx_data_priority_2,
	ssv6xxx_data_priority_3,
	ssv6xxx_data_priority_4,
	ssv6xxx_data_priority_5,
	ssv6xxx_data_priority_6,
	ssv6xxx_data_priority_7
};

//------------------------------------Send-------------------------------------


H_APIs ssv_type_s32 ssv6xxx_wifi_send_ethernet(ssv_type_u8 vif_idx, void *frame, ssv_type_s32 len, enum ssv6xxx_data_priority priority);
//H_APIs s32 ssv6xxx_wifi_send_80211(void *frame, s32 len);



//H_APIs s32 ssv_wifi_send(void *data, s32 len, struct cfg_host_txreq *txreq);
//time to remove
//H_APIs s32 ssv_wifi_send(void *data, s32 len, struct cfg_host_txreq *txreq);


//------------------------------------Receive-------------------------------------



typedef enum{
	SSV6XXX_DATA_ACPT		= 0,	//Accept
	SSV6XXX_DATA_CONT		= 1,	//Pass data
	SSV6XXX_DATA_QUEUED		= 2	//Data Queued
}ssv6xxx_data_result;

typedef ssv6xxx_data_result (*data_handler)(void *data, ssv_type_u32 len, ssv_type_u8 vif_idx);
typedef void (*evt_handler)(ssv_type_u32 nEvtId, void *data, ssv_type_s32 len, void* priv);
typedef void (*promiscuous_data_handler)(ssv_type_u8 channel, ssv_type_u8 *rx_buf, ssv_type_u32 len);
typedef void (*recovery_handler)(void);

//Register handler to get RX data
H_APIs ssv6xxx_result ssv6xxx_wifi_reg_rx_cb(data_handler handler);
//Register handler to get event
H_APIs ssv6xxx_result ssv6xxx_wifi_reg_evt_cb(evt_handler evtcb, void* priv);

//Register handler to get promiscuous data
H_APIs ssv6xxx_result ssv6xxx_wifi_reg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb);

//UnRegister Rx data handler
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_rx_cb(data_handler handler);

//UnRegister Rx event handler
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_evt_cb(evt_handler evtcb);

//UnRegister promiscuous call back function
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb);

//Register the recovery handler of up layer
H_APIs ssv6xxx_result ssv6xxx_wifi_reg_recovery_cb(recovery_handler recovery_cb);
//Unregister the recovery handler of up layer
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_recovery_cb(recovery_handler recovery_cb);

H_APIs ssv6xxx_result ssv6xxx_set_rc_value(enum cmdtype_rcinfo cmd, ssv_type_u32 param);

H_APIs HOST_API_STATE ssv6xxx_wifi_host_api_state(void);

H_APIs void ssv6xxx_wifi_clear_security (void);
H_APIs void ssv6xxx_wifi_apply_security (void);

H_APIs ssv_type_s32 ssv6xxx_wifi_send_addba_resp(struct cfg_addba_resp *addba_resp);
H_APIs ssv_type_s32 ssv6xxx_wifi_send_delba(struct cfg_delba *delba);


#if 0
//Data to soc
#if 1
H_APIs void* ssv_wifi_allocate(u32 size);
#else
H_APIs void* ssv_wifi_allocate(enum ssv_ts_type type,u32 size);
H_APIs struct cfg_host_txreq0* ssv_wifi_get_tx_req(void *data);
#endif
//H_APIs s32 ssv_wifi_free(void *data);
#endif

#if (CONFIG_HOST_PLATFORM == 1 )
H_APIs void ssv6xxx_wifi_apply_security_SIM (ssv_type_u8 bValue);
#endif

void ssv6xxx_send_host_cmd_done(ssv_type_u32 blocking_seq_no);
void ssv6xxx_send_host_cmd_ignore(ssv_type_bool val);
ssv_type_s32 ssv6xxx_send_host_cmd(void *pCusData, int nCuslen, 
        ssv6xxx_host_cmd_id eCmdID, ssv_type_s32 sub_h_cmd, ssv_type_bool blocking);
ssv6xxx_result ssv6xxx_send_msg_to_hcmd_eng(ssv_type_u32 cmd_id, ssv_type_u32 sub_cmd_id, void *data, ssv_type_u32 len, ssv_type_bool blocking, const ssv_type_bool mutexLock);
H_APIs ssv6xxx_result ssv6xxx_wifi_station(Sta_setting *sta_station);
H_APIs ssv6xxx_result ssv6xxx_wifi_status(struct ap_sta_status *status_info);
H_APIs ssv6xxx_result ssv6xxx_wifi_ap(Ap_setting *ap_setting,ssv_type_s32 step); //step 0: hw register and global variable; step 1: auto-channel-selection; step 2: AP start
//H_APIs ssv6xxx_result ssv6xxx_wifi_station(u8 hw_mode,Sta_setting *sta_station);
//H_APIs ssv6xxx_result ssv6xxx_wifi_status(Ap_sta_status *status_info);
H_APIs ssv_type_s32 ssv6xxx_get_rssi_by_mac(ssv_type_u8 *macaddr,ssv_type_u8 vif_idx);
H_APIs ssv_type_u32 ssv6xxx_get_aplist_info(void **ap_list);
H_APIs ssv6xxx_result ssv6xxx_get_sta_info_by_aid(struct apmode_sta_info *sta_info,ssv_type_u8 aid_num);
H_APIs ssv_type_s32 ssv6xxx_wifi_get_pwr_suspend(ssv_type_u8 *powerstatus);
H_APIs ssv_type_s32 ssv6xxx_wifi_pwr_suspend(ssv_type_u32 ip, ssv_type_u32 powermode);
H_APIs ssv_type_s32 ssv6xxx_wifi_pwr_awake(void);
H_APIs void ssv6xxx_wifi_pwr_notify_hold_on3(void);
H_APIs void ssv6xxx_wifi_pwr_notify_wake_down(void);

//------------------------------------Other Setting-------------------------------------
/* The mask of rate is depends on the sorted rates.
 * The order of sorted rates are [1M, 2M, 5.5M, 11M, 6M, 9M, 12M, 18M, 24M, 36M, 48M, 54M].
 * So the bit 0 of mask means 1M, if bit 0 = 1, 1M rate is available.
 * Example: the mask of all rates would be 0x0FFF, the mask with b mode rates only would be 0x000F.
 */
H_APIs ssv_type_s32 ssv6xxx_set_rate_mask(ssv_type_u16 mask);

//Customized
H_APIs void ssv6xxx_get_tx_info(ssv_type_u16 *tx_cnt, ssv_type_u16 *retry_cnt, ssv_type_u16 *phy_rate);

H_APIs ssv6xxx_result  ssv6xxx_wifi_set_edcca_threshold(ssv_type_u32 edcca_threshold);
H_APIs ssv_type_s32 ssv6xxx_wifi_get_mac(ssv_type_u8 *mac,ssv_type_u8 vif_idx);

//txTime Max: 200, Min: 70      qkTime Max: 50, Min: 10 ms
H_APIs ssv6xxx_result ssv6xxx_wifi_set_trx_res_page(ssv_type_u8 tx_page, ssv_type_u8 rx_page);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_tx_task_sleep(ssv_type_bool on);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_tx_task_sleep_tick(ssv_type_u8  tick);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_ap_erp(ssv_type_bool on);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_ap_short_preamble(ssv_type_bool on);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_ap_rx_support_rate(ssv_type_u16 bas_msk,ssv_type_u16 mcs_msk);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_tx_task_retry_cnt(ssv_type_u8 cnt);
H_APIs ssv6xxx_result ssv6xxx_set_voltage_mode(ssv_type_u32 mode);
H_APIs ssv_type_bool ssv6xxx_wifi_support_5g_band(void);
H_APIs ssv_type_bool ssv6xxx_wifi_enable_5g_band(ssv_type_bool en);
H_APIs int ssv6xxx_wifi_ch_to_bitmask(ssv_type_u8 ch);
H_APIs int ssv6xxx_wifi_ch_bitmask_to_num(ssv_type_bool b5GBand, ssv_type_u8 idx);
H_APIs int ssv6xxx_wifi_show_reg_list(void);
H_APIs int ssv6xxx_wifi_get_reg(char *reg, ssv_type_u8 len);
H_APIs struct ieee80211_regdomain *ssv6xxx_wifi_get_current_regdomain(void);
H_APIs int ssv6xxx_wifi_set_reg(char *reg);
H_APIs int ssv6xxx_wifi_update_available_channel(void);
H_APIs int ssv6xxx_wifi_show_available_channel(void);
H_APIs ssv_type_bool ssv6xxx_wifi_is_available_channel(ssv6xxx_hw_mode mode, ssv_type_u8 ch);
H_APIs int ssv6xxx_wifi_align_available_channel_mask(ssv6xxx_hw_mode mode,ssv_type_u16 *channel_2g_mask, ssv_type_u32 *channel_5g_mask);
H_APIs int ssv6xxx_wifi_set_ap_no_dfs(ssv_type_bool no_dfs);
H_APIs ssv_type_bool ssv6xxx_wifi_is_40MHZ_legal(ssv_type_u8 ch);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_sta_no_bcn_timeout(ssv_type_u8 value);
H_APIs HOST_API_STATE ssv6xxx_wifi_get_host_api_state(void);
H_APIs ssv_type_u8 ssv6xxx_wifi_get_sta_bw(void);
//For instance ,hwaddr:a[0]~a[5]=60:11:22:33:44:55
H_APIs ssv_type_s32 ssv6xxx_wifi_ap_del_sta(ssv_type_u8 *hwaddr);
H_APIs ssv_type_s32 ssv6xxx_wifi_krack_patch(ssv_type_bool en);
H_APIs ssv6xxx_result ssv6xxx_platform_check(void);
H_APIs struct ssv6xxx_ieee80211_bss * ssv6xxx_wifi_find_ap_ssid(struct cfg_80211_ssid *ssid);
H_APIs struct ssv6xxx_ieee80211_bss * ssv6xxx_wifi_find_wps_pbc_active_ap(void);
H_APIs ssv_type_bool ssv6xxx_wifi_is_wps_pbc_overlap(void);

H_APIs ssv6xxx_result ssv6xxx_wifi_clear_ap_acl(void);
H_APIs ssv6xxx_result ssv6xxx_wifi_get_ap_acl(AP_SETTING_ACL_TYPE *ap_acl_mode, AP_SETTING_ACL_MAC *mac_list, ssv_type_u16 *ap_acl_mac_count);
H_APIs ssv6xxx_result ssv6xxx_wifi_set_ap_acl_type(AP_SETTING_ACL_TYPE ap_acl_mode );
H_APIs ssv6xxx_result ssv6xxx_wifi_add_ap_acl(ssv_type_u8 *macaddr);
H_APIs ssv6xxx_result ssv6xxx_wifi_del_ap_acl(ssv_type_u8 *macaddr);
H_APIs ssv_type_s32 ssv6xxx_aplist_get_count_by_channel(ssv_type_u8 channel);

ssv_type_s32 netmgr_wifi_wps_push_button(void);
extern void send_deauth_and_remove_all(void);

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */

#endif /* _HOST_APIS_H_ */



