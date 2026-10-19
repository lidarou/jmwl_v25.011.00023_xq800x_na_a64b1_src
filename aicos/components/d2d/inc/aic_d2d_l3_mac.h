/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef AIC_D2D_L3_MAC_H
#define AIC_D2D_L3_MAC_H

/* EXPORTED MACROS */
/*==========================*/


/* const */
#ifndef SUCCESS
#define SUCCESS           (0)
#endif

#ifndef FAIL
#define FAIL              (-1)
#endif


#define D2D_SESSION_KEY_LEN 16
#define D2D_MAX_QOS_NUM     9
#define D2D_ID_BYTE_LEN     4
#define D2D_BAR_FREQ_NUM    16
#define D2D_SHORT_HASH_LEN  4
#define D2D_TARGET_LIST_LEN 15
#define D2D_PBCH_FREQ_PATTERN_NUM   15
/*Wait mask for waiting confirmation from L1 and L2*/
#define NO_CNF_RCVD    0x00
#define L1_CNF_RCVD    0x01
#define L2_CNF_RCVD    0x02
/* sig message sequence number */
#define MSG_SN_INVALID 0x00
#define MSG_SN_START   0x01
#define MSG_SN_MAX     0xFFFF
#define L2ACK          0
#define L2NOACK        1


/* thread id */
#define AOS_D2D_L4_THREAD_ID 74
#define AOS_D2D_L3_THREAD_ID 73
#define AOS_D2D_L2_THREAD_ID 59
#define AOS_D2D_L1_THREAD_ID 80

/*L3<->L4*/
#define P_D2D_L3_TARGET_IND         0x5001
#define P_D2D_L3_MTCALL_IND         0x5002
#define P_D2D_L3_RELEASE_IND        0x5003

/* L3 <--> L3 timer  */
#define P_D2D_L3_MO_PAGING_ACK_EXP  0x4001
#define P_D2D_L3_MT_D2DID_RCV_EXP   0x4002
#define P_D2D_L3_BCH_LISTEN_EXP     0x4003
#define P_D2D_L3_INITIALISE_EXP     0x4004
#define P_D2D_L3_SIG_RELIND_EXP     0x4005
#define P_D2D_MT_L2_CONFIRM_EXP     0x4006


/* L3 internal message*/
#define P_D2D_L3_FULL_SCAN_IND      0x4006

/* AP <-- > L3 */
#define P_D2D_L3_INITIALISE_REQ     0x3001
#define P_D2D_L3_INITIALISE_CNF     0x3002
#define P_D2D_L3_QUERY_STATUS_REQ   0x3004
#define P_D2D_L3_QUERY_STATUS_CNF   0x3005
#define P_D2D_L3_ENCRYPT_REQ        0x3006
#define P_D2D_L3_ENCRYPT_CNF        0x3007
#define P_D2D_L3_CONNCET_SETUP_REQ  0x3008
#define P_D2D_L3_CONNCET_SETUP_CNF  0x3009
#define P_D2D_L3_DISCOVERY_REQ      0x300A
#define P_D2D_L3_DISCOVERY_CNF      0x300B
#define P_D2D_L3_SCREEN_ONOFF_REQ   0x300C
#define P_D2D_L3_SCREEN_ONOFF_CNF   0x300D
#define P_D2D_L3_CONNCET_REL_REQ    0x300E
#define P_D2D_L3_CONNCET_REL_CNF    0x300F
#define P_D2D_L3_QUERY_INFO_REQ     0x3010
#define P_D2D_L3_QUERY_INFO_CNF     0x3011
#define P_D2D_L3_STOP_REQ           0x3012
#define P_D2D_L3_STOP_CNF           0x3013
#define P_D2D_L3_TX_POWER_SET_REQ   0x3014
#define P_D2D_L3_TX_POWER_SET_CNF   0x3015


/* L3 <-- > L2 */
#define P_D2D_L2_INITIALISE_REQ        0x2001
#define P_D2D_L2_INITIALISE_CNF        0x2002
#define P_D2D_L2_STOP_REQ              0x2003
#define P_D2D_L2_STOP_CNF              0x2004
#define P_D2D_L2_RELEASE_REQ           0x2005
#define P_D2D_L2_RELEASE_CNF           0x2006
#define P_D2D_L2_DATA_REQ              0x2007
#define P_D2D_L2_DATA_IND              0x2008
#define P_D2D_L2_START_ENCRYPTION_REQ  0x2009
#define P_D2D_L2_START_ENCRYPTION_CNF  0x200A
#define P_D2D_L2_CONFIG_REQ            0x200B
#define P_D2D_L2_CONFIG_CNF            0x200C
#define P_D2D_L2_ERROR_IND             0x200D
#define P_D2D_L2_DATA_CNF              0x200E

/* L3 <-- > L1 */
#define P_D2D_L1_INITIALISE_REQ       0x1001
#define P_D2D_L1_INITIALISE_CNF       0x1002
#define P_D2D_L1_STOP_REQ             0x1003
#define P_D2D_L1_STOP_CNF             0x1004
#define P_D2D_L1_PBCH_CONFIG_REQ      0x1005
//#define P_D2D_L1_PBCH_CONFIG_CNF      0x1006
#define P_D2D_L1_CH_RELEASE_REQ       0x1007
#define P_D2D_L1_CH_RELEASE_CNF       0x1008
#define P_D2D_L1_CH_RELEASE_IND       0x1009
#define P_D2D_L1_PARA_SET_REQ         0x100A
#define P_D2D_L1_PARA_SET_CNF         0x100B
#define P_D2D_L1_CHNL_CODING_REQ      0x100C
#define P_D2D_L1_CHNL_CODING_CNF      0x100D
//////// Used @ idle state  //////
////L3 <-> L1 Paging related message
#define P_D2D_L1_L3_PAGING_IND        0x100E    // 被叫L1收到Paging，检测PSCCH A2后发送Paging ind
#define P_D2D_L3_L1_PAGING_CMD        0x100F    // 主叫L3收到Call Req，给L1发送Paging Command
#define P_D2D_L1_L3_BCH_IND           0x1010    // 收到广播
#define P_D2D_L3_L1_PAGING_ABORT_REQ  0x1011    // L3 Paging过程超时，给L1发送Paging Abort req
#define P_D2D_L1_L3_PAGING_ABORT_CNF  0x1012
#define P_D2D_L1_L3_PAGING_ABORT_IND  0x1013    // L1->L3, 当MO过程中L1检测到MT Paging，需要Abort MO


/* EXPORTED FUNCTIONS */
/*==========================*/


#endif /* AIC_D2D_L3_MAC_H */
