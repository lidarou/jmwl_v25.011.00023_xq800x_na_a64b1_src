

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
#ifndef __CALL_FP_FLOW_H__
#define __CALL_FP_FLOW_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
    *      INCLUDES
    *********************/
#include "aic_type.h"

#include "aic_srv_tele.h"


    /*********************
    *      DEFINES
    *********************/
#define     CALL_PHONE_NUMBER_LEN_MAX TS_CALL_PHONE_NUMBER_LEN_MAX

    /**********************
    * GLOBAL PROTOTYPES
    **********************/
    typedef enum {
        /* 表示没有通话 */
        CALL_STATUS_IDLE = -1,

        /* 0.表示有正常进行中的通话 */
        CALL_STATUS_ACTIVE,

        /* 1.表示有保持的通话 */
        CALL_STATUS_HELD,

        /* 2.表示有正在拨出的通话 */
        CALL_STATUS_DIALING,

        /* 3.表示呼出的电话正在响铃 */
        CALL_STATUS_ALERTING,

        /* 4.表示有呼入的通话 */
        CALL_STATUS_INCOMING,

        /* 5.表示多通电话有来电正在等待接听 */
        CALL_STATUS_WAITING,

        /* 7.表示通话挂断 */
        CALL_STATUS_RELEASED,
        CALL_STATUS_MAX
        } call_status_e;

#define     CONTACT_NAME_LEN_MAX    (38)//(10)
#define     TELE_PHONE_NUMBER_LEN_MAX (32)


    typedef enum {
        CALL_ORIGIN_DIALDER,                        /* 0, dialer */
        CALL_ORIGIN_GLOBAL_INCOMING,                /* 1, global call */
        CALL_ORIGIN_CALLLOG,                        /* 2, calllog */
        CALL_ORIGIN_CONTACT,                        /* 3, contact */
        } call_origin_e;

    typedef struct call_time_st {
        uint16_t        year;
        uint8_t         month;
        uint8_t         wday;
        uint8_t         day;
        uint8_t         hour;
        uint8_t         minute;
        uint8_t         second;
        } call_time;

    /* TODO call_enter_param_t can be used by other app */
    typedef struct {
        call_origin_e   call_origin;
        char            phone_num[TELE_PHONE_NUMBER_LEN_MAX]; /* phone number */
        char            contact_name[CONTACT_NAME_LEN_MAX]; /* contact name*/
        uint8_t         phone_num_type;             /* 0:normal number;1:service number;3:emergency number */
        uint8_t         card_id;
        uint8_t         call_type;                  /*0:已拨打，1：未接来电，2：已接来电*/
        uint8_t         end_call_type;              /*2:对方未接听，如果有自动重拨，就要自动重拨这个号码*/
        call_time       time;
        } ui_call_enter_param_t;

    typedef struct {
        uint8_t         redial_counter;             // 自动重拨最多三次
        char            phone_num[TELE_PHONE_NUMBER_LEN_MAX]; /* phone number */
        char            contact_name[CONTACT_NAME_LEN_MAX]; /* contact name*/
        lv_timer_t *    redial_timer;
        } ui_redial_param_t;

    typedef struct {
        uint8_t         sos_start;                  // sos 开始
        uint8_t         sos_number;                 // sos 位置
        char            phone_num[TELE_PHONE_NUMBER_LEN_MAX]; /* phone number */
        char            contact_name[CONTACT_NAME_LEN_MAX]; /* contact name*/
        lv_timer_t *    sos_timer;
        } ui_sos_param_t;

    typedef struct {
        int32_t(*IncomingWinOnShow) (ui_call_enter_param_t * incoming_info);
        int32_t(*ConnectedWinOnShow) (ui_call_enter_param_t * actived_info);
        int32_t(*ReleasedWinOnShow) (ui_call_enter_param_t * call_param);
        int32_t(*ImsStatusUpdate) (bool ims_status);
        } call_flow_t;

    /* dial result:0,success,other is fail */
    typedef int32_t(*call_flow_dial_callback) (int32_t dial_result);

    /* hungup result:0,success,other is fail */
    typedef int32_t(*call_flow_hungup_callback) (int32_t hungup_result);

/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
 * call_fp_flow_is_em_enter_code - is enter engineer mode code.
 * @arg1:[in] input p_number.
 * @arg2: none.
 * Return: true,is enginner mode code.false,is not enginner mode code.
 */
bool call_fp_flow_is_em_enter_code(char *p_number);

/**
 * call_fp_flow_is_imei_enter_code - is enter imei mode code.
 * @arg1:[in] input p_number.
 * @arg2: none.
 * Return: true,is imei code;false,is not imei code.
 */
bool call_fp_flow_is_imei_enter_code(char * p_number);

    /**
     * call_fp_flow_init - the init of call flow.
     * @arg1: none.
     * @arg2: none.
     * Return: none.
     */
    void call_fp_flow_init(void);

    /**
     * call_fp_flow_register_ui_actions - register ui actions.
     * @arg1: p_ui_cbs,ui callbacks.
     * Return: none.
     */
    void call_fp_flow_register_ui_actions(const call_flow_t * p_ui_cbs);

    /**
     * call_fp_flow_get_call_status - get call status.
     * @arg1: none.
     * @arg2: none.
     * Return: call_status_e,refer enum define.
     */
    call_status_e call_fp_flow_get_call_status(void);

    /**
     * call_fp_flow_get_phonenum - get incoming/connect phone number.
     * @arg1: p_phonenum:phone number,max length 32.
     * Return: 0,success,not 0,fail.
     */
    int32_t call_fp_flow_get_phonenum(char * p_phonenum);

    /**
     * call_fp_flow_is_emergency_number - this number is emergency number.
     * @arg1: p_phonenum:phone number.
     * Return: true,is emergency number,false, not emergency number.
     */
    bool call_fp_flow_is_emergency_number(char * p_phonenum);

    /**
     * call_fp_flow_get_call_volume - get call volume value.
     * @arg1: none.
     * Return: volume value.
     */
    uint32_t call_fp_flow_get_call_volume(void);

    /**
     * call_fp_flow_set_call_volume - get call volume value.
     * @arg1: vol,volume value..
     * Return: none.
     */
    void call_fp_flow_set_call_volume(uint32_t vol);

    /**
     * call_fp_flow_get_volte_reg_status - get reg volte status.
     * @arg1: none.
     * Return: true,volte open,false,volte close.
     */
    bool call_fp_flow_get_volte_reg_status(void);

    /**
     * call_fp_flow_send_dtmf:send dtmf signal.
     * @arg1: [IN]:char dtmf string.
     * Return: none.
     */
    void call_fp_flow_send_dtmf(char * dtmf);

    /* remove to call flow begin */
    /**
     * call_fp_flow_dial.
     * @arg1: phonenum:phone number.
     * @arg2: p_flow_cb:update ui according to dial result.
     * Return: success,0,fail,not 0.
     */
    int32_t call_fp_flow_dial(char * phonenum,call_flow_dial_callback p_flow_cb);

    /**
     * inincoming call win, call_fp_flow_anwser.
     * @arg1: none.
     * Return: success,0,fail,not 0.
     */
    int32_t call_fp_flow_anwser(void);

    /**
     * inincoming call win, call_fp_flow_reject.
     * @arg1: none.
     * Return: success,0,fail,not 0.
     */
    int32_t call_fp_flow_reject(void);

    /**
    * in connected call win,hungup call, call_fp_flow_hungup.
    * @arg1: p_flow_cb,ui update callback according hungup result.
    * Return: success,0,fail,not 0.
    */
    int32_t call_fp_flow_hungup(call_flow_hungup_callback p_flow_cb);

    void call_fp_active_set_cur_name(uint8_t * name);
    ui_call_enter_param_t * get_ui_call_enter_param(void);
    void set_fp_call_relesed_now_status(uint8_t status);
    void call_fp_active_resume_other_app_action();
    uint32_t get_fp_call_relesed_now_status();
    void call_fp_active_pause_other_app_action();
    void set_fp_call_other_app_music_play_status(uint8_t music_play);
    uint32_t call_fp_flow_is_call_status(void);
    void call_fp_flow_call_status_init();

#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif /* __CALL_FP_FLOW_H__ */

