/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_D2D_L3_INT_H__
#define __AIC_D2D_L3_INT_H__

#include "aic_list.h"

#include "aic_d2d_l3_mac.h"

extern u8 *g_aic_d2d_status_str[];

typedef enum
{
    D2D_STATUS_START_SUCCESS,
    D2D_STATUS_STOP_SUCCESS,
    D2D_STATUS_PBCH_CONFIGURED,
    D2D_STATUS_BCH_START,
    D2D_STATUS_PAGING_SEND,
    D2D_STATUS_PAGING_RECEIVED_SUCC,
    D2D_STATUS_CALL_SETUP_RECEIVED,
    D2D_STATUS_CALL_ACTIVE,
    D2D_STATUS_CALL_SETUP_SEND,
    D2D_STATUS_CONN_REL_SEND,
    D2D_STATUS_CONN_REL_RCVD,
    D2D_STATUS_CALL_END,
    D2D_STATUS_CH_RELEASE_IND,
    D2D_STATUS_PAGING_ABORT,
    D2D_STATUS_PAGING_RECEIVED_FAIL,
    D2D_STATUS_NO_BCH,
} Td2d_status_index_e;

typedef enum
{
    D2D_STATE_OFF,
    D2D_STATE_ON,
    D2D_STATE_IDLE,
    D2D_STATE_CONN,
} Td2d_state_e;

typedef enum
{
    D2D_ST_EMC = 0,
    D2D_ST_SIG = 1,
    D2D_ST_VOICE = 2,
    D2D_ST_DATA = 3,
    D2D_ST_OTHER,
} Td2d_ie_service_type_e;

typedef enum
{
    D2D_MSG_NAME_PAGING_ACK = 0,
    D2D_MSG_NAME_PUB_KEY_EXCHANGE_ACK = 1,
    D2D_MSG_NAME_RAND_EXCHANGE_ACK = 2,
    D2D_MSG_NAME_CALL_SETUP_ACK = 3,
} Td2d_ie_msg_name_e;

typedef enum
{
    D2D_REL_REASON_NORMAL = 0,
    D2D_REL_REASON_RAND_EXCHANGE_FAIL = 1,
} Td2d_ie_rel_reason_e;

typedef enum{
    D2D_L3_QOS_STATE_NULL = 0,
    D2D_L3_QOS_STATE_ACTIVE = 1,
    D2D_L3_QOS_STATE_EST = 2,
    D2D_L3_QOS_STATE_REL = 3,
    D2D_L3_QOS_STATE_WAIT_RE_EST = 4,
    D2D_L3_QOS_STATE_PAGING = 5,
    D2D_L3_QOS_STATE_CFG = 6,
} Td2d_qos_state_e;

typedef enum{
    D2D_ROLE_NULL = 0,
    D2D_ROLE_MO = 1,
    D2D_ROLE_MT = 2,
} Td2d_ue_role;

typedef struct{
    u8 qos_id;
    u8 is_audio_start;
    Td2d_qos_state_e state;
    Td2d_ie_service_type_e service_type;
    Td2d_l2_mode_e l2_mode;/* AM/UM */
} Td2d_l3_qos_info;

typedef struct{
    u16 sig_len;
    u8 *sig_ptr;
} Td2d_l3_codec;

typedef struct{
    struct list_head list_node;
    Td2d_lcid_e lcid;
    u16 msg_sn;
    u16 msg_id;
} Td2d_l3_sig_tx_node;

typedef struct{
    u8 mod_state;
    u8 is_sec_act;
    u8 pbch_freq_group;/*for debug*/
    u8 pbch_freq_pattern[D2D_PBCH_FREQ_PATTERN_NUM];
    u8 is_disable_pending;
    u8 bch_num;
    u8 bch_report_period;
    u8 Tout;
    u8 is_qos_cfg_by_usr;
    u8 is_bch_received;

    u16 lcl_msg_sn;
    s16 tx_power;
    s16 max_tx_power;

    u32 d2d_id;
    u32 hash;
    u32 mt_d2d_id;

    aos_timer_ptr re_est_tmr;
    aos_timer_ptr bch_report_tmr;
    aos_timer_ptr bch_check_tmr;
    aos_timer_ptr mo_guard_tmr;
    aos_timer_ptr l3_wait_rel_tmr;

    struct list_head l3_sig_tx_list;

    Td2d_ue_role ue_role;
    Td2d_state_e l3_state;

    Td2d_l3_qos_info qos_info[D2D_MAX_QOS_NUM];
    Td2d_l3_target bch_list[D2D_L3_BCH_MAX];
}Td2d_l3_globar_var;

#endif

