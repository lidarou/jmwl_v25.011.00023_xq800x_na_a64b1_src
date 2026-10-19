#ifndef AIC_D2D_L3_TYPE_H
#define AIC_D2D_L3_TYPE_H

#include "aic_osa.h"
#include "aic_d2d_l3_mac.h"
#include "D2D_L3_CONFIG.h"

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
    D2D_FAIL = 1,
} Td2d_result_e;

typedef enum{
    D2D_L2_UM = 0,
    D2D_L2_AM = 1,
} Td2d_l2_mode_e;

typedef enum{
    D2D_L1_CHNL_CODING_1_2 = 0,
    D2D_L1_CHNL_CODING_1_3 = 1,
} Td2d_chnl_coding_e;

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
    D2D_L3_UNKNOW_ROLE = 0,
    D2D_L3_MO_USER = 1,
    D2D_L3_MT_USER = 2,
} l3Role_e;

typedef enum{
    D2D_L3_UNKNOW_PAGING = 0,
    D2D_L3_PAGING_SEND = 1,
    D2D_L3_PAGING_ACK = 2,
} l3Flag_e;


typedef enum{
    D2D_L3_UNKNOW_REASON = 0,
    D2D_L3_RLF = 1,
	D2D_L3_RLC_MAX_TRANS = 2,
    D2D_L3_NORMAL_RELEASE = 3,
} l3Release_reason_e;


typedef enum{
    D2D_L3_REL_UNKNOW_TRIGGER = 0,
    D2D_L3_REL_AP_TRIGGER = 1, // user abort
    D2D_L3_REL_L1_TRIGGER = 2, // RLF
	D2D_L3_REL_L3_TRIGGER =3, // L3 timeeoute
	D2D_L3_REL_L2_TRIGGER =4, // RLC reach the max trans
	D2D_L3_REL_PEER_TRIGGER =5, // received rel ind message from peer UE
} l3RelTrigger_e;

typedef enum{
    D2D_L3_OFFLINE = 0,
    D2D_L3_ONLINE = 1,
} Td2d_l3_status_e;


typedef enum{
	D2D_NOT_FAIL = 0,
	D2D_FAIL_RLF = 1, // both L1 RLF and L2 max trans, report this reason to AP
	D2D_FAIL_PAGING_EXP = 2,
	D2D_FAIL_PAGING_CONFLIT = 3,
	D2D_FAIL_AP_CANCEL = 4,
	D2D_FAIL_SIGNALING_CONFICT = 5,
	D2D_FAIL_STATE_MISMATCH = 6,
	D2D_FAIL_UNKNOW	= 0xFF,
}l3_fail_reason_e;

typedef enum{
    D2D_SCREEN_OFF = 0,
	D2D_SCREEN_ON = 1,
}l3_screen_onoff_e;


typedef enum{
    CCH_PAGING_ACK = 0,
	CCH_RELEASE_IND = 1,
	DEFAULT_CCH_MSG = 2,
}l3_cch_msg_type_e;

typedef struct{
    u32 d2d_id;
	u32 hash;
	u8  version;
	u8  slot_idx;
	u8  reserved[2];
}Td2d_l3_target;

typedef struct{
    aos_thread_id src_tid;
    aos_thread_id des_tid;
	u16 msg_len;
    aos_prim_id prim_id;
} d2d_msg_header;

/* L3 <-> L2: P_D2D_L2_DATA_REQ/P_D2D_L2_DATA_IND*/

// L2 : CCH_SDU (CCH ACK)
typedef struct{
    l3_cch_msg_type_e msg_type;
}Td2d_l2_cch_sdu;

//P_D2D_L2_DATA_REQ, P_D2D_L2_DATA_IND
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_lcid_e lcid;
    u8  *msg_ptr;
	u16 length;
	/*For 4 bytes alignment*/
	u16 msg_sn; /* L3 sig sequence number */
} Td2d_l2_sdu;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_lcid_e lcid; /* CCH */
	u16 msg_sn; /* L3 sig sequence number */
    u8  result; /* 0: ack, 1: no ack */
	u8  reserved;
} Td2d_l2_data_cnf;

//P_D2D_L2_START_ENCRYPTION_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    u8 session_key[D2D_SESSION_KEY_LEN];
    Td2d_algo_e enc_algo;
} Td2d_l2_sec_para;

// common cnf message, only result parameter.
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_result_e result;
} Td2d_result_para;

/*L3<->L2: P_D2D_L2_CONFIG_REQ*/
typedef struct{
    d2d_msg_header msg_hdr;
    u8 add_mod_qos_num;
	u8  rel_qos_num;
	/*For 4 bytes alignment*/
	u16 reserved;
	/*qos 0 represents CCH, qos 1-8 represent traffic qos of AP side*/
    LCID_PARAM add_mod_qos_config[D2D_MAX_QOS_NUM];
	LCID_PARAM rel_qos_config[D2D_MAX_QOS_NUM];
} Td2d_l2_cfg_para;

//P_D2D_L2_ERROR_IND
typedef struct{
    d2d_msg_header msg_hdr;
    l3Release_reason_e reason;
} Td2d_l2_error_ind;


/* L3 <-- > L1 */
//P_D2D_L1_CHNL_CODING_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_chnl_coding_e chnl_coding;
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
//P_D2D_L1_PBCH_CONFIG_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    u8 pbch_freq_pattern[D2D_PBCH_FREQ_PATTERN_NUM];
	s16 tx_power;             /*Tx power level.  -600 - 350 represent -60 - 35dbm*/
    Td2d_drx_e drx;
    Td2d_l1_dtx_para dtx;    //read from NV
    u32 d2d_id;              //read from NV provided by GX
    u32 hash;
    boolean is_init_cfg;     //is pbch config the first timer after initialization
} Td2d_l1_pbch_cfg_para;

//P_D2D_L1_CH_RELEASE_IND
typedef struct{
    d2d_msg_header msg_hdr;
    l3Release_reason_e reason;
} Td2d_l1_ch_rel_ind;
//P_D2D_L1_PARA_SET_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    s16 tx_power;
	s16 max_tx_power;
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



/* L4 <-- L3 */

// P_D2D_L3_TARGET_IND
typedef struct{
    d2d_msg_header msg_hdr;
	Td2d_l3_target broad_target;
} Td2d_l3_target_ind;

//P_D2D_L3_DISCOVERY_REQ
typedef struct{
    d2d_msg_header msg_hdr;
	u8 version;
    u32 hash;
} Td2d_l3_discovery_req;

//P_D2D_L3_SCREEN_ONOFF_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    l3_screen_onoff_e screen_mode;
} Td2d_l3_screen_onoff_req;

//P_D2D_L3_ENCRYPT_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    u8 session_key[D2D_SESSION_KEY_LEN];
    Td2d_algo_e enc_algo;
} Td2d_l3_AP_encrypt_req;

//P_D2D_L3_CONNCET_SETUP_REQ
typedef struct{
    d2d_msg_header msg_hdr;
	u32 mt_d2d_id;
} Td2d_l3_connect_setup_req;

typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_result_e  setup_rlt;
    l3_fail_reason_e  fail_reason;
	/////  if remote D2D id needed?
} Td2d_l3_connect_setup_cnf;

typedef struct{
    d2d_msg_header msg_hdr;
    l3Release_reason_e reason;
} Td2d_l3_rel_ind;

// Td2d_l3_connect_release_req; allways release all, empty message.

typedef struct{
    d2d_msg_header msg_hdr;
    boolean result;
} Td2d_l3_ap_common_cnf;
// discover_cnf, screen_cnf, encrypt_cnf,release_cnf

//P_D2D_L3_QUERY_STATUS_CNF
typedef struct{
    d2d_msg_header msg_hdr;
    Td2d_l3_status_e status;
} Td2d_l3_query_status_cnf;

//P_D2D_L3_QUERY_INFO_CNF
typedef struct{
    d2d_msg_header msg_hdr;
    u32 d2d_id;
	u16 drx_cycle;
} Td2d_l3_query_info_cnf;


//P_D2D_L3_TX_POWER_SET_REQ
typedef struct{
    d2d_msg_header msg_hdr;
    s16 tx_power;      //1/10dbm, -600-350
	s16 max_tx_power;   //1/10dbm, -600-350, invalid value:0x7FFF
} Td2d_l3_tx_power_set_req;




// 1. CP call AT URC for +rssi,rsrp,snr,ta,tx_power,bler,freq
//    mt role cannot meas TA report as 0, it get TA by distance signalling procedre, move to oppo AP.

// 2. CP call AT URC for +buffer status

// 3. mo altering and mt answer procedure move to oppo AP

// 4. distance signalling procedure move to oppo AP, but meas requirment still need to support

// 5. msisdn request procedure move to oppo AP


#endif
