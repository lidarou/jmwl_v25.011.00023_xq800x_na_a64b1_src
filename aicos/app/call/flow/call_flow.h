/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of call.
 */

#ifndef __CALL_FLOW_H__
#define __CALL_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
*      INCLUDES
*********************/
#include "aic_type.h"

#include "aic_srv_tele.h"

#include "call.h"

/*********************
*      DEFINES
*********************/
#define CALL_PHONE_NUMBER_LEN_MAX               TS_CALL_PHONE_NUMBER_LEN_MAX

/**********************
* GLOBAL PROTOTYPES
**********************/
typedef enum
{
    /* 没有通话 */
    CALL_STATUS_IDLE = TS_CALL_STATUS_IDLE,

    /* 有正常进行中的通话 */
    CALL_STATUS_ACTIVE = TS_CALL_STATUS_ACTIVE,

    /* 有保持的通话 */
    CALL_STATUS_HELD = TS_CALL_STATUS_HELD,

    /* 有正在拨出的通话 */
    CALL_STATUS_DIALING = TS_CALL_STATUS_DIALING,

    /* 呼出的电话正在响铃 */
    CALL_STATUS_ALERTING = TS_CALL_STATUS_ALERTING,

    /* 有呼入的通话 */
    CALL_STATUS_INCOMING = TS_CALL_STATUS_INCOMING,

    /* 多通电话有来电正在等待接听 */
    CALL_STATUS_WAITING = TS_CALL_STATUS_WAITING,

    /* 通话挂断 */
    CALL_STATUS_RELEASED = TS_CALL_STATUS_RELEASED,

    CALL_STATUS_MAX
}call_status_e;

typedef struct {
    int32_t (*IncomingWinOnShow)(call_enter_param_t incoming_info);
    int32_t (*ConnectedWinOnShow)(uint8_t call_idx, char *p_phone_num);
    int32_t (*ReleasedWinOnShow)(uint32_t reason);
    int32_t (*ImsStatusUpdate)(bool ims_status);
}call_flow_t;

/* dial result:0,success,other is fail */
typedef int32_t (*call_flow_dial_callback)(int32_t dial_result);
/* hungup result:0,success,other is fail */
typedef int32_t (*call_flow_hungup_callback)(int32_t hungup_result);

/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
 * call_flow_init - the init of call flow.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void call_flow_init(void);

/**
 * call_flow_init - the init of call flow.
 * @arg1: p_ui_cbs,ui callbacks.
 * Return: none.
 */
void call_flow_register_ui_actions(const call_flow_t *p_ui_cbs);

/**
 * call_flow_get_call_status - get call status.
 * @arg1: none.
 * @arg2: none.
 * Return: call_status_e,refer enum define.
 */
call_status_e call_flow_get_call_status(void);
/**
 * call_flow_get_phonenum - get incoming/connect phone number.
 * @arg1: p_phonenum:phone number,max length 32.
 * Return: 0,success,not 0,fail.
 */
int32_t call_flow_get_phonenum(char *p_phonenum);

/**
 * call_flow_is_emergency_number - this number is emergency number.
 * @arg1: p_phonenum:phone number.
 * Return: true,is emergency number,false, not emergency number.
 */
bool call_flow_is_emergency_number(char *p_phonenum);

/**
 * call_flow_get_call_volume - get call volume value.
 * @arg1: none.
 * Return: volume value.
 */
uint32_t call_flow_get_call_volume(void);
/**
 * call_flow_get_call_volume - get call volume value.
 * @arg1: vol,volume value..
 * Return: none.
 */
void call_flow_set_call_volume(uint32_t vol);

/**
 * call_flow_get_volte_reg_status - get reg volte status.
 * @arg1: none.
 * Return: true,volte open,false,volte close.
 */
bool call_flow_get_volte_reg_status(void);

/**
 * call_flow_send_dtmf:send dtmf signal.
 * @arg1: [IN]:char dtmf string.
 * Return: none.
 */
void call_flow_send_dtmf(char *dtmf);

/* remove to call flow begin */
/**
 * call_flow_dial.
 * @arg1: phonenum:phone number.
 * @arg2: p_flow_cb:update ui according to dial result.
 * Return: success,0,fail,not 0.
 */
int32_t call_flow_dial(char *phonenum, call_flow_dial_callback p_flow_cb);

/**
 * inincoming call win, call_flow_anwser.
 * @arg1: none.
 * Return: success,0,fail,not 0.
 */
int32_t call_flow_anwser(void);

/**
 * inincoming call win, call_flow_reject.
 * @arg1: none.
 * Return: success,0,fail,not 0.
 */
int32_t call_flow_reject(void);

 /**
 * in connected call win,hungup call, call_flow_hungup.
 * @arg1: p_flow_cb,ui update callback according hungup result.
 * Return: success,0,fail,not 0.
 */
int32_t call_flow_hungup(call_flow_hungup_callback p_flow_cb);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __CALL_FLOW_H__ */

