/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_DEVINFO_H_
#define _SSV_DEVINFO_H_
#include <host_apis.h>
//#include "cmd_def.h"
//#include <rtos.h>

//#ifdef ARRAY_SIZE
#define SSV_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
//#endif

#define AP_VIF_IDX 1
#define STA_VIF_IDX 0


typedef ssv_type_u32 tx_result;
#define TX_CONTINUE	((tx_result) 0u)
#define TX_DROP		((tx_result) 1u)
#define TX_QUEUED	((tx_result) 2u)


//Off->Init->Running->Pause->Off


enum TXHDR_ENCAP {
	TXREQ_ENCAP_ADDR4               =0,
	TXREQ_ENCAP_QOS                 ,
	TXREQ_ENCAP_HT                  
};


#define IS_TXREQ_WITH_ADDR4(x)          ((x)->txhdr_mask & (1<<TXREQ_ENCAP_ADDR4) )
#define IS_TXREQ_WITH_QOS(x)            ((x)->txhdr_mask & (1<<TXREQ_ENCAP_QOS)   )
#define IS_TXREQ_WITH_HT(x)             ((x)->txhdr_mask & (1<<TXREQ_ENCAP_HT)    )

#define SET_TXREQ_WITH_ADDR4(x, _tf)    (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_ADDR4))	| ((_tf)<<TXREQ_ENCAP_ADDR4) )
#define SET_TXREQ_WITH_QOS(x, _tf)      (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_QOS))	| ((_tf)<<TXREQ_ENCAP_QOS)   )
#define SET_TXREQ_WITH_HT(x, _tf)       (x)->txhdr_mask = (((x)->txhdr_mask & ~(1<<TXREQ_ENCAP_HT))		| ((_tf)<<TXREQ_ENCAP_HT)    )



typedef enum txreq_type_en {
	TX_TYPE_M0 = 0,
	TX_TYPE_M1,
	TX_TYPE_M2
} txreq_type;


#define HOST_RECOVERY_CB_NUM  1
#define HOST_DATA_CB_NUM  2
#define HOST_EVT_CB_NUM  8
#define PRMOISCUOUS_CB_NUM  1

typedef enum {
	SSV6XXX_CB_ADD		,
	SSV6XXX_CB_REMOVE	,
	SSV6XXX_CB_MOD		
} ssv6xxx_cb_action;

typedef enum vif_state_en {
    VIF_ALL_DEACTIVE = 0,
    VIF_ACTIVE_STA,
    VIF_ACTIVE_AP
} vif_state;

typedef struct StaInfo{
    ssv_type_u8* pOSMgmtframe;
	ssv_type_u8* pMgmtPkt;
    struct cfg_join_request *jreq; // Station mode used
    struct cfg_scan_request *sreq; // Station mode used
    enum conn_status    status;                 //Auth,Assoc,Eapol
#if(SW_8023TO80211==1)
    ssv_type_u16 seq_ctl[8]; //Station mode used
#endif
    void *wpactx;
    ssv_type_u8 aid;    
    ssv_type_u8 wsid;    
    ssv_type_u16 rssi;
    ssv_type_u8 ampdu_tx;    
    ssv_type_u8 ampdu_rx;    
    ssv_type_u8 ampdu_token;
    ssv_type_u16 ampdu_tx_ssn;
    ssv_type_u8 beaconmisscnt;
}StaInfo_st;

typedef struct SnifferInfo{
    ssv_type_u32 mask;
    ssv_type_u32 max_len;
    ssv_type_u32 min_len;
    ssv_type_u32 mac_addr[ETH_ALEN];
}SnifferInfo_st;


typedef struct ssv_vif_st {
    ssv6xxx_hw_mode hw_mode;
    ssv_type_u8 self_mac[ETH_ALEN];
    ssv_type_u8 idx;
    struct freq_params freq;
    struct ApInfo *APInfo; // AP mode used
    struct StaInfo *StaInfo; //STA mode used
    struct SnifferInfo SnifferInfo;
  }ssv_vif;

typedef struct DeviceInfo{

    OsMutex g_dev_info_mutex;
	txreq_type tx_type;
	ssv_type_u32 txhdr_mask;
	ETHER_ADDR addr4;
	ssv_type_u16 qos_ctrl;
	ssv_type_u32 ht_ctrl;
    //struct StaInfo *StaInfo[MAX_STA_VIF]; //STA mode used        

    ssv_vif vif[MAX_VIF_NUM];
    ssv_type_u8 used_vif;
    ssv_type_u8 ampdurx_session_cnt;
    //Set key base on VIF.

    #if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
    ssv_type_u16 cci_current_level; //Station mode used
    ssv_type_u16 cci_current_gate; //Station mode used
    #endif
    struct ssv6xxx_ieee80211_bss *ap_list; //station ap info list used (useing "g_dev_info_mutex" to protect it)
    char  alpha2[3];  //country code
    ssv_type_u16 available_2g_channel_mask; //The available channel in the regulatory domain, not hw capability
    ssv_type_u32 available_5g_channel_mask; //The available channel in the regulatory domain, not hw capability    
//-----------------------------
//Data path handler
//-----------------------------
	data_handler data_cb[HOST_DATA_CB_NUM];

//-----------------------------
//Event path handler
//-----------------------------
	evt_handler evt_cb[HOST_EVT_CB_NUM];
    void* evt_cb_priv[HOST_EVT_CB_NUM]; //private data

//-----------------------------
// promiscuous call back function
//-----------------------------
    promiscuous_data_handler promiscuous_cb[PRMOISCUOUS_CB_NUM];

//-----------------------------
// promiscuous call back function
//-----------------------------
    recovery_handler recovery_cb[HOST_RECOVERY_CB_NUM];

//-----------------------------
// fw recovery
//-----------------------------
    ssv_type_s32 reload_fw_cnt;
    ssv_type_bool recovering; //TRUE: in recovering

//timer interrupt count
    ssv_type_u32 fw_timer_interrupt;
    cali_result_info CliRes_info;

// lowpower params
    ssv_type_u8 ps_wakeuppin;
    ssv_type_u8 ps_userdtim;
    ssv_type_u8 ps_ignore_disconnection;
    ssv_type_u8 ps_wow_any_method;
    ssv_type_u32 ps_sleep_gpio_ctrl;
    ssv_type_u32 ps_dormant_gpio_ctrl;
    ssv_type_u32 ps_keepalive;
    ssv_type_u8 ps_dataretx_guard;
    ssv_type_u8 ps_beaconloss_guard;
    ssv_type_u8 ps_beaconloss_maxcnt;
    ssv_type_u8 ps_sdio_interface;
    ssv_type_u8 ps_event_cnt;

    // efuse tool
    ssv_type_u8 efuse_psk[64];
    ssv_type_u8 efuse_psk_len;
    ssv_type_u32 efuse_chip_id;
    ssv_type_u8 efuse_mac[6];
    ssv_type_u8 efuse_txpower1;
    ssv_type_u8 efuse_xtal;
    ssv_type_u8 efuse_rate_gain_b;
    ssv_type_u8 efuse_rate_gain_n40;
    ssv_type_u8 efuse_free_count;
    ssv_type_u8 efuse_ble_gain;

//hw temperature
    ssv_type_s32 hw_temperature;

//wps   
    void    *pWPSSM;
    ssv_type_u8 bWPSPBC;
    ssv_type_u8 bDoingWPS;
    ssv_type_u8 bWPSFindActivedSR;
    ssv_type_u8 bWPSPBCOverLapChecking;
    ssv_type_u8 bWPSPBCOverLap;    
    ssv_type_u8 bWPSSuccess;
    ssv_type_u8 bPushButton;
    ssv_type_u32 WPSWalkTimeStart;

//EAP
    void *pEAPSM;
    ssv_type_u8* pEAPframe;
	ssv_type_u8* pEAPPkt;

}DeviceInfo_st;

extern DeviceInfo_st *gDeviceInfo;

struct rcs_info
{
    ssv_type_u16 free_page;
    ssv_type_u16 free_id;
    ssv_type_u16 free_space; //how many space of HCI input queue for incoming packets
    ssv_type_u16 edca_fifo_cnt[4];
};

struct vif_tx_rcs
{
    ssv_type_u16 free_page;
};

extern struct rcs_info tx_rcs;
extern OsMutex txsrcMutex;

struct vif_flow_ctrl
{
    ssv_type_u32 retry_cnt:16;
    ssv_type_u32 sleep_tick:16;
    ssv_type_u32 priority:1;
    ssv_type_u32 RSVD:31;
};


struct Host_cfg {     
    //1st 
    ssv_type_u32 rate_mask:16;
    ssv_type_u32 hci_tx_aggr:1;
    ssv_type_u32 hci_rx_aggr:1;
    ssv_type_u32 force_rtscts:1;
    ssv_type_u32 force_cts2self:1;
    ssv_type_u32 rsvd10:12;

    //2nd 
    ssv_type_u32 bcn_interval:8;
    ssv_type_u32 ampdu_rx_buf_size:8;
    ssv_type_u32 ampdu_tx_buf_size:8;
    ssv_type_u32 ap_ampdu_tx_enable:1;
    ssv_type_u32 ap_ampdu_rx_enable:1;
    ssv_type_u32 sta_ampdu_tx_enable:1;
    ssv_type_u32 sta_ampdu_rx_enable:1;
    ssv_type_u32 rsvd20:4;

    //3rd 
    ssv_type_u32 pool_size:16;
    ssv_type_u32 recv_buf_size:16;

    //4th
    ssv_type_u32 trx_hdr_len:16;
    ssv_type_u32 tx_res_page:8;
    ssv_type_u32 rx_res_page:8;

    //5th
    ssv_type_u32 ap_rx_support_legacy_rate_msk:12; 
    ssv_type_u32 ap_rx_support_mcs_rate_msk:8;    
    ssv_type_u32 ap_rx_short_GI:1;
    ssv_type_u32 erp:1;
    ssv_type_u32 b_short_preamble:1;
    ssv_type_u32 tx_retry_cnt:8;
    ssv_type_u32 tx_sleep:1;
    
    //6th
    ssv_type_u32 rc_drate_endian:1;
    ssv_type_u32 volt_mode:1;
    ssv_type_u32 tx_sleep_tick:8;
    ssv_type_u32 support_ht:1;
    ssv_type_u32 support_tx_SGI:1;
    ssv_type_u32 support_rf_band:1;
    ssv_type_u32 ap_no_dfs:1;
    ssv_type_u32 rsvd60:18;    

    //8th
    ssv_type_u32 sta_no_bcn_timeout:8;
    ssv_type_u32 rxIntGPIO:8;
    ssv_type_u32 extRxInt:1;
    ssv_type_u32 AP_TimAllOne:1;
    ssv_type_u32 ApStaInactiveTime:8;
    ssv_type_u32 ap_supported_sta_num:4;
    ssv_type_u32 rsvd80:2;    

    //9th	
    ssv_type_u32 fw_tx_waitnum:8;
    ssv_type_u32 fw_tx_chkhwqnum:8;
    ssv_type_u32 xtal:8;
    ssv_type_u32 rsvd90:8;

    //10th
    ssv_type_u32 sta_scan_ch_interval:16;
    ssv_type_u32 sta_scan_off_ch_interval:16;

    //11th
    ssv_type_u32 sta_channel_mask:16;
    ssv_type_u32 hci_tx_aggr_buf_size:16;

    //12th
    ssv_type_u32 sta_5g_channel_mask;

    // lowpower params
    //13th
    ssv_type_u32 ps_wakeuppin:8;
    ssv_type_u32 ps_userdtim:8;
    ssv_type_u32 ps_ignore_disconnection:8;
    ssv_type_u32 ps_wow_any_method:8; // 1: any packet wakeup, 0: magic packet wakeup
    //14th
    ssv_type_u32 ps_sleep_gpio_ctrl;
    //15th
    ssv_type_u32 ps_dormant_gpio_ctrl;
    //16th
    ssv_type_u32 ps_keepalive;
    //17th
    ssv_type_u32 ps_dataretx_guard:8;
    ssv_type_u32 ps_beaconloss_guard:8;
    ssv_type_u32 ps_beaconloss_maxcnt:8;
    ssv_type_u32 ps_sdio_interface:1; // 1: sdio interface, 0 usb interface
    ssv_type_u32 force_custom_rftable:1;    
    ssv_type_u32 rsvd170:6;
    //18th
    ssv_type_u32 tx_trigger_qlen:8;
    ssv_type_u32 tx_trigger_pkt_size:16;
    ssv_type_u32 tx_trigger_en:1;
    ssv_type_u32 rsvd180:7;
    
    //19th
    ssv_type_u32 tx_wait_period:16;
    ssv_type_u32 rsvd190:16;

    //20th
    ssv_type_u32 mifi_idle_timeout; // mifi idle timeout, unit 100ms
    
    //HW rx aggregation
    //21th
    ssv_type_u32 hw_rx_agg_cnt:8;
    ssv_type_u32 hw_rx_agg_timer_reload:8;
    ssv_type_u32 rsvd210:16;
    //22th
    ssv_type_u32 rf_band_gain_offset;
    //23th
    ssv_type_u32 efuse_rate_gain_mask;
    //24th
    ssv_type_u32 rf_b_rate_offset;
    //25th
    ssv_type_u32 rf_g_rate_offset;
    //26th
    ssv_type_u32 rf_ht20_rate_offset;
    //27th
    ssv_type_u32 rf_ht40_rate_offset;
    //28th
    ssv_type_u32 rf_volt_regulator;
    //29th
    ssv_type_u32 rf_tx_lowpower;
    //30th
    ssv_type_u32 rf_padpd;
    //31th
    ssv_type_u32 backoff_enable;
    //32th
    ssv_type_u32 WPSWakeTimeout;
};
//AMPDU RX

#define RX_AGG_RX_BA_MAX_STATION				AMPDU_RX_MAX_STATION
#define RX_AGG_RX_BA_MAX_SESSIONS				1
#define HT_RX_REORDER_BUF_TIMEOUT               10//ms

/**
 * struct tid_ampdu_rx - TID aggregation information (Rx).
 *
 * @reorder_buf: buffer to reorder incoming aggregated MPDUs
 * @reorder_time: jiffies when skb was added
 * @session_timer: check if peer keeps Tx-ing on the TID (by timeout value)
 * @reorder_timer: releases expired frames from the reorder buffer.
 * @head_seq_num: head sequence number in reordering buffer.
 * @stored_mpdu_num: number of MPDUs in reordering buffer
 * @ssn: Starting Sequence Number expected to be aggregated.
 * @buf_size: buffer size for incoming A-MPDUs
 * @timeout: reset timer value (in TUs).
 * @dialog_token: dialog token for aggregation session
 * @rcu_head: RCU head used for freeing this struct
 * @reorder_lock: serializes access to reorder buffer, see below.
 *
 * This structure's lifetime is managed by RCU, assignments to
 * the array holding it must hold the aggregation mutex.
 *
 * The @reorder_lock is used to protect the members of this
 * struct, except for @timeout, @buf_size and @dialog_token,
 * which are constant across the lifetime of the struct (the
 * dialog token being used only for debugging).
 */
struct rx_ba_session_desc {

	OsMutex reorder_lock;
	void **reorder_buf;//[RX_AGG_RX_BA_MAX_BUF_SIZE];
	unsigned long *reorder_time;//[RX_AGG_RX_BA_MAX_BUF_SIZE];
	ssv_type_u16 head_seq_num;
	ssv_type_u16 stored_mpdu_num;
	ssv_type_u16 ssn;
	ssv_type_u16 buf_size;
};

#endif /* _SSV_DEVINFO_H_ */

