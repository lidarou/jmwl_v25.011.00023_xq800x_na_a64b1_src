/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_D2D_L3_TYPE_H__
#define __AIC_D2D_L3_TYPE_H__

#include "aic_d2d_l3_mac.h"

extern u8 *g_aic_d2d_status_str[];

typedef enum{
    D2D_LCID_CCH = 0,
    D2D_LCID_MAX = 0XFF,
} Td2d_lcid_e;

typedef enum{
    D2D_PHY_PTCH = 0,
    D2D_PHY_ALL = 0xFF,
} Td2d_phy_chnl_e;

typedef enum{
    D2D_ALGO_NULL = 0,
    D2D_ALGO_AES128 = 1,
} Td2d_algo_e;

typedef enum{
    D2D_SUCESS = 0,
    D2D_FAIL   = 1,
} Td2d_result_e;

typedef enum{
    D2D_L2_UM = 0,
    D2D_L2_AM = 1,
} Td2d_l2_mode_e;

typedef enum{
    D2D_L1_CHNL_CODING_1_2 = 0,
    D2D_L1_CHNL_CODING_1_3 = 1,
}Td2d_chnl_coding_e;

typedef enum{
    D2D_L1_DRX_1200_MS = 0,
    D2D_L1_DRX_2400_MS = 1,
    D2D_L1_DRX_4800_MS = 2,
} Td2d_drx_e;

typedef enum{
    D2D_L1_UL_DL_NO_CHANGE = 0,
    D2D_L1_UL_DL_1_1 = 1,
    D2D_L1_UL_DL_1_3 = 2,
    D2D_L1_UL_DL_1_5 = 3,
} Td2d_ul_dl_alloc_e;

typedef enum{
    D2D_L3_REASON_UNKNOW   = 0,
    D2D_L3_REASON_RLF = 1,
    D2D_L3_RLC_MAX_TRANS = 2,
} Td2d_l3_reason_e;

typedef struct{
    u32 d2d_id;
    u32 hash;
    u8  version;
    u8  slot_idx;
    u8  reserved[2];
}Td2d_l3_target;

typedef struct{
    aos_thread_id     src_tid;
    aos_thread_id     des_tid;
    u16               msg_len;
    aos_prim_id       prim_id;
} d2d_msg_header;

/* AP <--> L3 */
typedef struct{
    d2d_msg_header msg_hdr;
    u8 option;
    u8 qos_id;
    u8 mode;/*Td2d_l2_mode_e*/
    u8 service_type;/*Td2d_ie_service_type_e*/
} Td2d_ap_qos_set_req;

typedef struct{
    d2d_msg_header msg_hdr;
    u8 action;
    u8 qos_id;
    u32 d2d_id;
} Td2d_ap_call_req;

/* L3 <--> L2 */
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_lcid_e lcid;/* CCH */
    u8  *msg_ptr;
    u16 length;
    /*For 4 bytes alignment*/
    u16 msg_sn; /* L3 sig sequence number */
} Td2d_l2_sdu;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_lcid_e lcid;/* CCH */
    u16 msg_sn; /* L3 sig sequence number */
    u8 result;/* 0: ack; 1: no ack */
    u8 reserved;
} Td2d_l2_data_cnf;

typedef struct{
    d2d_msg_header msg_hdr;
    u8 session_key[D2D_SESSION_KEY_LEN];/* 长度待定 */
    Td2d_algo_e enc_algo;/*NULL/AES128*/
} Td2d_l2_sec_para;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_result_e result;/*SUCCESS/FAIL*/
} Td2d_result_para;

typedef struct{
    u16 lcid;    /* lcid */
    u16 priority;    /* priority of lcid 0 */
    u16 l2_mode;    /* l2 mode: 0:UM, 1:AM */
    u16 t_reordering;    /* t-reordering time */
    u16 t_discard;    /* t-discard time */
    u16 pollingNum;    /* polling number of PDU */
    u16 status_prohibit_time;    /* polling number of PDU */
} Td2d_l2_lcid_param;

typedef struct{
    d2d_msg_header msg_hdr;
    u8 add_mod_qos_num;
    u8 rel_qos_num;
    /*For 4 bytes alignment*/
    u16 reserved;
    Td2d_l2_lcid_param add_mod_qos_config[D2D_MAX_QOS_NUM];
    Td2d_l2_lcid_param rel_qos_config[D2D_MAX_QOS_NUM];
} Td2d_l2_cfg_para;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_l3_reason_e reason;
} Td2d_l2_error_ind;

/* L3 <--> L1 */
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_chnl_coding_e chnl_coding;/* 1/2, 1/3 */
} Td2d_l1_chnl_coding;

typedef struct{
    u8 N_user_num_TH1;
    u8 N_user_num_TH2;
    u8 N_user_num_TH3;
    u8 N_DTX_LV1;
    u8 N_DTX_LV2;
    u8 N_DTX_LV3;
    /*For 4 bytes alignment*/
    u16 reserved;
} Td2d_l1_dtx_para;

typedef struct{
    d2d_msg_header msg_hdr;
    u8 pbch_freq_pattern[D2D_PBCH_FREQ_PATTERN_NUM]; /* 15*4bits, 低4bits有效 */
    s16 tx_power;             /* 1/10 dbm, -600~350 */
    Td2d_drx_e drx; /* 1200_MS/2400_MS/4800_MS */
    Td2d_l1_dtx_para dtx;
    u32 d2d_id;
    u32 hash;
    u8 is_init_cfg; /* TRUE/FALSE */
    u8 pbch_freq_group;/*for debug*/
} Td2d_l1_pbch_cfg_para;

typedef struct{
    d2d_msg_header msg_hdr;
    u16 sub_band_bitmap;
    u32 mt_d2d_id;
    s16 tx_power; /* 1/10 dbm, -600~350 */
    Td2d_ul_dl_alloc_e ul_dl_alloc; /* 0表示不变 */
} Td2d_l1_ptch_cfg_para;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_l3_reason_e reason;
} Td2d_l1_ch_rel_ind;

typedef struct{
    d2d_msg_header msg_hdr;
    s16 rssi;
    s16 rsrp; /* 不存在 */
    s16 snr; /* 不存在 */
    s16 ta;
    s16 tx_power;
    u32 bler; /* 不存在 */
    u32 freq;
} Td2d_l1_meas_cnf;

typedef struct{
    d2d_msg_header msg_hdr;
    s16 tx_power; /* 1/10 dbm, -600~350 */
    s16 max_tx_power; /* 1/10 dbm, -600~350 */
    u32 bler_threshold;
    u32 bar_freq[D2D_BAR_FREQ_NUM];
} Td2d_l1_para_set_req;

//P_D2D_L3_L1_PAGING_CMD
typedef struct{
    d2d_msg_header msg_hdr;
    u32 mt_d2d_id;
    u8  sw_version;
    /*For 4 bytes alignment*/
    u8 reserved;
    s16 tx_power;             /*Tx power level.  -600 - 350 represent -60 - 35dbm*/
} Td2d_l3_l1_paging_cmd;

//P_D2D_L1_L3_BCH_IND
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_l3_target target;
} Td2d_l1_bch_ind;

//P_D2D_L1_L3_PAGING_IND
typedef struct{
    d2d_msg_header msg_hdr;
    u8  sw_version;
    u8  slot_idx;
    u16 paging_ack_result;   /// 0: SUCESS, 1: FAIL
    u32 mt_d2d_id;
} Td2d_l1_l3_paging_ind;

#endif

