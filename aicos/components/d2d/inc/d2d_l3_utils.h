/*==============================================================================

 Copyright (c) 2024-2028 OPLUS,  All Rights  Reserved.  OPLUS Proprietary.

-------------------------------------------------------------------------------
 @file      d2d_l3_utils.h
 @brief     This header file declare the L3 common functions    
 History

 when       who     what, where, why
 --------   ---     ---------------------------------------------------------
 20240515   haizhou  initial version 

==============================================================================*/

#ifndef D2D_L3_UTILS_H
#define D2D_L3_UTILS_H

/* INCLUDE FILES */
/*==========================*/
#include "aic_osa.h"
#include "aic_osa_ex.h"
#include "aic_d2d_l3_type.h"  
#include "aic_d2d_l3_mac.h"
#include "d2d_l3_fsm_tables.h"
#include "D2D_L3_CONFIG.h"
#include "aic_d2d_l3_func.h"

/* EXPORTED MACROS */
/*==========================*/

#define DURATION_TIMER_MO_PAGINGACK 3000
#define DURATION_TIMER_MT_D2DIDRCV  15000
#define DURATION_TIMER_PBCH_LISTEM  6000
#define DURATION_TIMER_INITIALISE  1000
#define DURATION_PBCH_LISTEN_RANDOM  3000
#define DURATION_TIMER_SIG_RELIND   500
#define DURATION_TIMER_MT_L2_CONFIRM 250

#define INVALID_D2D  0x00000000
#define VALID_LCID_NUM   5

/* EXPORTED TYPES  */
/*==========================*/

typedef enum
{
    L3_TIMER_BEGIN    = 0,
    L3_TIMER_MO_PAGING_ACK = L3_TIMER_BEGIN, /// Timer to control wating duration for mo user receiveing the paging ack form mt user
    L3_TIMER_MT_PAGING_ACK_CNF,              /// Timer to control wating duration for mt user receiveing the d2d-id of mo user
    L3_TIMER_PBCH_LISTEN,                    /// TImer for PBCH listening duration
    L3_TIMER_INITIALISE,          /// Timer for waiting L1/L2 initialise confirmation
	L3_TIMER_SIG_RELIND,		             /// Timer for MO waiting L2 sends out successfully or L2 data_cnf
	L3_TIMER_MT_L2_CONFIRM,					 /// Timer for MT waiting L2 sends out successfully or L2 data_cnf
    L3_TIMER_TOTALNUM,
}l3TimerId_e;    

typedef enum
{
		L3_TIMER_INACTIVE = 0,
		L3_TIMER_RUNNING = 1,
		L3_TIMER_STOP = 2,
}l3TimerStatus_e;


/** @brief Structure of all parameters regarding one L3 timer */
typedef struct
{
		l3TimerId_e 			  timerId;		  ///< timer ID
		aos_timer_ptr             timerptr;       
		//u32						  remDuration;	  ///< remaining uncomsumed duration

}l3Timer_t;

typedef struct
{
	l3TimerId_e timerId;		///< timer ID
    u32         time_out;       /* timer expiration */
    bool        is_repeat;      /* is repeat */
    char        *p_name;        /* timer name */
    aos_prim_id prim_id;	    /* timer expiry message prim_id  */
} l3TimerParaTable_t;

/** @brief The structure to specify L3 information database structure. */
typedef struct
{
    /** @brief Is wait mask for message from other modules */
    u8                       waitMask;
    /** @brief phone's d2d id */
    u32                       d2d_id;
	/*@brief phone's hash value*/
	u32                       hash;
	/*@brief phone's version*/
    u8                        version;
	/*@brief current message sequence number between L3 and L2 sdu */
    u16                      l3SigMsgSn;
    /** @brief L3 role, MO or MT user */
    l3Role_e                 userRole;
    /** @brief L3 current state */
    l3FsmState_e                       curState;
    /** @brief Is paging ACK received as MO user */
    l3Flag_e                       pagingFlag;
    /** @brief Is screen off  */
    bool                       isScreenOff;
    /** @brief MO: Is release ind sig sent out already  */
    bool                     isRelSigSent;
    /** @brief MT Is release ind sig received already  */
    bool                     isRelSigReceived;
    /*PBCH Configuration read from NV*/
	D2D_L3_CONFIG     l3_config;
	u8 pbch_freq_pattern[D2D_PBCH_FREQ_PATTERN_NUM];
	//////// data for idle ///// 
	Td2d_l3_target remote_user_info;
    /** @brief Is release mask for message from L1 and L2 modules */
    u8                       releaseMask;
    /** @brief Trigger module of connection release */
    l3RelTrigger_e           releaseTrigger;
	boolean first_init;

} l3DbCommon_t;


/* EXPORTED DATA */
/*==========================*/

/* EXPORTED FUNCTIONS  */
/*==========================*/

//aos_status fd2d_l3_send_msg_to_app_queue(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);

//aos_status fd2d_l3_send_msg_to_l2(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);

//aos_status fd2d_l3_send_sig(Td2d_lcid_e lcid, u16 sig_len, u8 *sig_ptr);

//aos_status fd2d_l3_send_msg_to_l1(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
//aos_status fd2d_l3_send_msg_to_l3(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
//aos_status fd2d_l3_send_msg_to_l4(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);

//void fd2d_l3_send_common_cnf_to_app_queue(aos_prim_id prim_id, boolean result);

aos_status l3TimerInit(void);
void l3TimerDestroy(void);
void l3TimerDeactiveAll();


void fshuffle(u8 *array, u8 num);
u16 fshuffle_num(u16 num);

u8 fcch_package_parse(Td2d_l2_cch_sdu* cch_sdu_ptr);

void fsend_connect_setup_cnf_to_ap(Td2d_result_e setup_result, l3_fail_reason_e fail_reason);
void fsend_para_set_to_l1(s16 temp_Txpower);


#endif /*D2D_L3_UTILS_H */

