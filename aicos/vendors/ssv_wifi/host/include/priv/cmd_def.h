/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _CMD_DEF_H_
#define _CMD_DEF_H_


#include "ssv_common.h"


#define u8 ssv_type_u8
#define u16 ssv_type_u16
#define u32 ssv_type_u32
#define u64 ssv_type_u64
#define s8 ssv_type_s8
#define s16 ssv_type_s16
#define s32 ssv_type_s32
#define s64 ssv_type_s64
#ifndef bool
#define bool ssv_type_bool
#endif
#define uint8_t ssv_type_u8
#define uint16_t ssv_type_u16
#define uint32_t ssv_type_u32
#define uint64_t ssv_type_u64
#define int8_t ssv_type_s8
#define int16_t ssv_type_s16
#define int32_t ssv_type_s32
#define int64_t ssv_type_s64



#include "ssv6xxx_common.h"

/************************************************************************************************************************************************/
/*                                                                Extra Info                                                                    */
/************************************************************************************************************************************************/

enum ssv6xxx_tx_extra_type
{
    SSV6XXX_TX_FRAME_TIME,
    SSV6XXX_TX_DO_RTS_CTS,
    SSV6XXX_TX_ACK_POLICY,
    SSV6XXX_TX_TX_BRUST,
    SSV6XXX_TX_TX_REPORT,
    SSV6XXX_TX_RTS_CTS_NAV,
    SSV6XXX_TX_CRATE_IDX,
    SSV6XXX_TX_DRATE_IDX,
    SSV6XXX_TX_DL_LENGTH_IDX,

    SSV6XXX_SET_INIT_BEACON,
    SSV6XXX_SET_BEACON
};

typedef void (*RxPktHdr)(void *);
typedef enum
{
    SSV_HCI_RX,
    SSV_HCI_TX
} HCI_AGGR_HW;

struct cfg_bcn_info {
    ssv_type_u16 bcn_len;
    ssv_type_u16 tim_cnt_oft;		//include mac header
    //u8  hdr_len;
};

struct cfg_bcn_param {
    ssv_type_u8                         dtim_cnt;      //DTIM=4-->dtim_cnt=3;
    ssv_type_u16          bcn_itv;     //beacon interval
};

//SSV6XXX_SET_INIT_BEACON
struct cfg_set_init_bcn {
    struct cfg_bcn_param  param;
    struct cfg_bcn_info   bcn_info;
};


/************************************************************************************************************************************************/
/*                                                                 Command                                                                      */
/************************************************************************************************************************************************/
#define SET_STA_INFO_VALID(s, v)       (s)->info = (((s)->info & ~(1<<0)) | ((v)<<0))
#define SET_STA_INFO_QOS_EN(s, v)      (s)->info = (((s)->info & ~(1<<1)) | ((v)<<1))
#define SET_STA_INFO_OP_MODE(s, v)     (s)->info = (((s)->info & ~(3<<2)) | ((v)<<2))
#define SET_STA_INFO_HT_MODE(s, v)     (s)->info = (((s)->info & ~(3<<4)) | ((v)<<4))

#define IS_STA_INFO_VALID(s)               !!((s)->info & (1<<0))

enum cfg_op_mode_type
{
    CFG_OP_MODE_STA,
    CFG_OP_MODE_AP,
    CFG_OP_MODE_AD_HOC,
    CFG_OP_MODE_WDS
};

enum cfg_sta_oper
{
    CFG_STA_DEL,
    CFG_STA_ADD,
    CFG_STA_DEL_ALL,
    CFG_STA_MODIFY
};

enum cfg_ht_type
{
    CFG_HT_NONE,
    CFG_HT_MF,
    CFG_HT_GF
};


struct cfg_wsid_info {
    //******************************************************
    //              =======IMPORTANT=======
    //Info flag is driect mapping to hw reg. DO NOT CHANGE IT.
    //******************************************************
    //HT_MODE(2)+OP_MODE(2)+QoS(1)+valid(1)
    ssv_type_u32 info;
    struct ETHER_ADDR_st addr;
    ssv_type_s32 wsid_idx;

};

struct cfg_ht_capabilities {
    ssv_type_u16 ht_capabilities_info;
    ssv_type_u8 supported_mcs_set[16];
};

struct cfg_set_sta {
    enum cfg_sta_oper sta_oper;
    struct cfg_wsid_info wsid_info;//info set to reg
    ssv_type_u16 aid;
    ssv_type_u16 non_ht_supp_rates;
    ssv_type_bool short_preamble;
    ssv_type_u8 vif_idx;
    struct cfg_ht_capabilities ht_capabilities;

};

struct cfg_set_ap_cfg {
    struct cfg_80211_ssid ssid;
    ssv6xxx_sec_type sec_type;
    ssv_type_u8 password[MAX_PASSWD_LEN+1];
};

enum cmdtype_rcinfo {
    RC_RATEMASK,
    FORCE_RTSCTS,
    FORCE_CTSTOSELF       
};

struct freq_params {
    ssv_type_u32 current_channel:8;
    ssv_type_u32 ht_enable:1;
    ssv_type_u32 sec_channel_offset:2;
    ssv_type_u32 vif_idx:1;
    ssv_type_u32 resv0:20;
};

typedef enum{
    CMD_SET_AP_CFG																		,
    CMD_SET_STA_CFG                                                                    ,
    CMD_MAXID
}ssv6xxx_cmd_id;

/************************************************************************************************************************************************/
/*                                                                Event                                                                         */
/************************************************************************************************************************************************/
struct join_res{
    ssv_type_s32     status_code;
    ssv_type_s32     aid;
    ssv_type_u32     bssid_idx;
    ssv_type_u32     wsid;
    ssv_type_u32     ch_offset;
    ssv_type_u32     assoc_req_ie_len;
    ssv_type_u32     assoc_resp_ie_len;
    ssv_type_u8     pmk[32];
    #ifdef NOT_SUPPORT_ZERO_ARRAY
    #else
    ssv_type_u8      assoc_ie_info[0];
    #endif
};
struct leave_res{
    ssv_type_s16     reason_code;
    ssv_type_u16     bssid_idx;
};

struct resp_evt_result {
    union {
        struct join_res join;
        struct leave_res leave;
        #ifdef NOT_SUPPORT_ZERO_ARRAY
        ssv_type_u8      dat[1];
        #else
        ssv_type_u8      dat[0];
        #endif
        ssv_type_u8 pmk[32];
        ssv_type_u8 wsid;
    } u;
};

typedef enum{
   EN_WPS_SUCCESS=0,
   EN_WPS_WAKE_TIME_TO,
   EN_WPS_PBC_OVERLAP,   
   EN_WPS_FAIL,
}EN_WPS_STATUS;

struct wps_res{
    EN_WPS_STATUS    status; 
    ssv_type_u8      ssid[MAX_SSID_LEN];
    ssv_type_u8      ssid_len;
    ssv_type_u8      key[MAX_PASSWD_LEN];
    ssv_type_u8      key_len;
};


typedef enum{
    EVT_JOIN_RESULT                     ,
    EVT_LEAVE_RESULT                    ,
    EVT_SCAN_DONE                       ,
    EVT_POLL_STATION                    ,
    EVT_STA_STATUS                      ,
    EVT_MLME_CMD_DONE                   ,
    EVT_WPS_RESULT                      ,
    EVT_MAXID_NONE                        // Add redundant field before remove all of this enumeration.
} ssv6xxx_event_id;

#endif//_CMD_DEF_H_

