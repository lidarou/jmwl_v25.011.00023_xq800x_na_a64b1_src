/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_D2D_L3_MAC_H__
#define __AIC_D2D_L3_MAC_H__

/* const */
#define D2D_SESSION_KEY_LEN 16
#define D2D_MAX_QOS_NUM     9
#define D2D_ID_BYTE_LEN     4
#define D2D_BAR_FREQ_NUM    16
#define D2D_SHORT_HASH_LEN  4
#define D2D_MAX_SIG_LEN     64
#define D2D_L3_RE_EST_TMR_P 3000 /* 3s */
#define D2D_PBCH_FREQ_PATTERN_NUM   15
#define D2D_L3_BCH_MAX      32
#define D2D_MO_GUARD_TMR_P  5000
#define D2D_NO_BCH_TMR_P_MIN 8000
#define D2D_NO_BCH_TMR_P_MAX 12000
#define D2D_L3_WAIT_REL_TMR_P 500

/* thread id */
#define AOS_D2D_L3_THREAD_ID 73
#define AOS_D2D_L2_THREAD_ID 59
#define AOS_D2D_L1_THREAD_ID 80
#define AOS_D2D_TIMER_THREAD_ID 24

/* L3 <--> L3 timer  */
#define P_D2D_L3_MO_PAGING_ACK_EXP  0x4001
#define P_D2D_L3_MT_D2DID_RCV_EXP   0x4002
#define P_D2D_L3_BCH_LISTEN_EXP     0x4003
#define P_D2D_L3_INITIALISE_EXP     0x4004

#define P_D2D_L3_MO_RE_EST_TMR_EXP  0x4010
#define P_D2D_L3_BCH_REPORT_TMR_EXP 0x4011
#define P_D2D_L3_BCH_CHECK_TMR_EXP  0x4012
#define P_D2D_L3_MO_GUARD_TMR_EXP   0x4013
#define P_D2D_L3_WAIT_REL_TMR_EXP   0x4014

/* AP <--> L3 */
#define P_D2D_L3_ENABLE_REQ         0x3001
#define P_D2D_L3_ENABLE_CNF         0x3002
#define P_D2D_L3_TARGET_IND         0x3003
#define P_D2D_L3_MSISDN_REQ         0x3004
#define P_D2D_L3_MSISDN_CNF         0x3005
#define P_D2D_L3_AP_KEY_ASK         0x3006
#define P_D2D_L3_AP_KEY_RSP         0x3007
#define P_D2D_L3_CALL_REQ           0x3008
#define P_D2D_L3_ALERT_IND          0x3009
#define P_D2D_L3_DISTANCE_REQ       0x300A
#define P_D2D_L3_DISTANCE_CNF       0x300B
#define P_D2D_L3_DISABLE_REQ        0x300C
#define P_D2D_L3_DISABLE_CNF        0x300D
#define P_D2D_L3_QOS_SET_REQ        0x300E
#define P_D2D_L3_QOS_SET_CNF        0x300F
#define P_D2D_L3_RE_SEARCH_CMD      0x3010

/* L3 <--> L2 */
#define P_D2D_L2_INITIALISE_REQ          0x2001
#define P_D2D_L2_INITIALISE_CNF          0x2002
#define P_D2D_L2_STOP_REQ                0x2003
#define P_D2D_L2_STOP_CNF                0x2004
#define P_D2D_L2_RELEASE_REQ             0x2005
#define P_D2D_L2_RELEASE_CNF             0x2006
#define P_D2D_L2_DATA_REQ                0x2007
#define P_D2D_L2_DATA_IND                0x2008
#define P_D2D_L2_START_ENCRYPTION_REQ    0x2009
#define P_D2D_L2_START_ENCRYPTION_CNF    0x200A
#define P_D2D_L2_CONFIG_REQ              0x200B
#define P_D2D_L2_CONFIG_CNF              0x200C
#define P_D2D_L2_ERROR_IND               0x200D
#define P_D2D_L2_DATA_CNF                0x200E

/* L3 <--> L1 */
#define P_D2D_L1_INITIALISE_REQ       0x1001
#define P_D2D_L1_INITIALISE_CNF       0x1002
#define P_D2D_L1_STOP_REQ             0x1003
#define P_D2D_L1_STOP_CNF             0x1004
#define P_D2D_L1_PBCH_CONFIG_REQ      0x1005
#define P_D2D_L1_PBCH_CONFIG_CNF      0x1006
#define P_D2D_L1_CH_RELEASE_REQ       0x1007
#define P_D2D_L1_CH_RELEASE_CNF       0x1008
#define P_D2D_L1_CH_RELEASE_IND       0x1009
#define P_D2D_L1_PARA_SET_REQ         0x100A
#define P_D2D_L1_PARA_SET_CNF         0x100B
#define P_D2D_L1_CHNL_CODING_REQ      0x100C
#define P_D2D_L1_CHNL_CODING_CNF      0x100D
#define P_D2D_L1_L3_PAGING_IND        0x100E    // 被叫L1收到Paging，检测PSCCH A2后发送Paging ind
#define P_D2D_L3_L1_PAGING_CMD        0x100F    // 主叫L3收到Call Req，给L1发送Paging Command
#define P_D2D_L1_L3_BCH_IND           0x1010    // 收到广播
#define P_D2D_L3_L1_PAGING_ABORT_REQ  0x1011    // L3 Paging过程超时，给L1发送Paging Abort req
#define P_D2D_L1_L3_PAGING_ABORT_CNF  0x1012
#define P_D2D_L1_L3_PAGING_ABORT_IND  0x1013    // L1->L3, 当MO过程中L1检测到MT Paging，需要Abort MO
#define P_D2D_L1_PTCH_CONFIG_REQ      0x1014
#define P_D2D_L1_PTCH_CONFIG_CNF      0x1015

#define D2D_L3_PD         0x03
#define D2D_L3_HDR_LEN    0x02

#define D2D_L3_MSG_BROADCAST          0x40
#define D2D_L3_MSG_PAGING             0x41
#define D2D_L3_MSG_L3_ACK             0x42
#define D2D_L3_MSG_PUB_KEY_EXCHANGE   0x43
#define D2D_L3_MSG_RAND_EXCHANGE      0x44
#define D2D_L3_MSG_ENCRYPT_REQUEST    0x45
#define D2D_L3_MSG_ENCRYPT_RESPONSE   0x46
#define D2D_L3_MSG_DISTANCE_REQUEST   0x47
#define D2D_L3_MSG_DISTANCE_RESPONSE  0x48
#define D2D_L3_MSG_CALL_SETUP_REQUEST 0x49
#define D2D_L3_MSG_CONN_RELEASE       0x4B
#define D2D_L3_MSG_MT_DATA_ACCEPT     0x4C

#endif
