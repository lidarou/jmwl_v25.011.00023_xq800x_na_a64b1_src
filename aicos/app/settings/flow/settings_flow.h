/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of settings flow.
 */
#ifndef __SETTINGS_FLOW_H__
#define __SETTINGS_FLOW_H__

#ifdef __cplusplus
    extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aic_type.h"
#include "aic_version.h"

/*********************
 *      DEFINES
 *********************/
#define SETTINGS_FLOW_AP_VERSION_LEN   (AIC_VERSION_LEN)
#define SETTINGS_FLOW_IMEI_LEN         (30)
#define SETTINGS_FLOW_IMSI_LEN         (16)
#define SETTINGS_FLOW_SN_LEN           (64)
#define SETTINGS_FLOW_CARD_ID_MAX      (2)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    SIM_EXIST_STATUS_NO,   /* 0,no sim */
    SIM_EXIST_STATUS_SIM1, /* 1,sim1 exist */
    SIM_EXIST_STATUS_SIM2, /* 2,sim2 exist */
    SIM_EXIST_STATUS_BOTH, /* 3,sim1&sim2 exist */
    SIM_EXIST_STATUS_MAX
}settings_sim_exist_status_e;

/* factoryreset result:0,success,other is fail */
typedef int32_t (*settings_flow_factoryreset_callback)(int32_t reset_result);

/* flightmode on/off result:0,success,other is fail */
typedef int32_t (*settings_flow_flightmode_callback)(int32_t flightmode_result);

/* flightmode on/off result:0,success,other is fail */
typedef int32_t (*settings_flow_selectsim_callback)(int32_t selectsim_result);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * settings_flow_dual_sim_select_card.
 * @arg1: [IN] card_id:sim slot it
 * @arg2: [IN] p_ui_cbs:update ui callback
 * Return: 0,success,is not 0,fail.
 */
 int32_t settings_flow_dual_sim_select_card(uint8_t card_id, settings_flow_selectsim_callback p_ui_cbs);

 /**
  * settings_flow_flightmode_on.
  * @arg1: [IN] cur_card_id:current card id
  * @arg2 [IN] is_on:on/off flightmode
  * @arg3 [IN] p_ui_cbs:recieve flight mode result,then update icon.
  * Return: 0:success,not 0,fail
  */
int32_t settings_flow_flightmode(uint8_t cur_card_id, bool on_off, settings_flow_flightmode_callback p_ui_cbs);

/**
 * settings_flow_get_flightmode_status.
 * Return: true,flight mode open;false,flight mode close.
 */
bool settings_flow_get_flightmode_status( void );

/**
 * settings_flow_get_volte_enabled.
 * Return: true,volte enabled by user;false,volte disabled by user.
 */
bool settings_flow_get_volte_enabled( void );
/**
 * settings_flow_set_volte_enabled.
 * @arg1: [IN] is_enable:true,enabled volte,false,disable volte.
 * Return: 0:success,not 0,fail.
 */
int32_t settings_flow_set_volte_enabled(bool is_enable);

/**
 * settings_flow_get_current_cardid.
 * Return: 0,sim1;1,sim2
 */
uint8_t settings_flow_get_current_cardid( void );

/**
 * settings_flow_get_sim_exist_status.
 * Return: settings_sim_exist_status_e,reference enum define.
 */
settings_sim_exist_status_e settings_flow_get_sim_exist_status( void );

/**
 * settings_flow_get_ap_version.
 * @arg1: [IN] p_version:ap version string
 * Return: none.
 */
int32_t settings_flow_get_ap_version(char *p_version);

/**
 * settings_flow_deep_factoryreset.
 * @arg1: [IN] p_ui_cbs:ui callback.
 * Return: none.
 */
void settings_flow_deep_factoryreset(settings_flow_factoryreset_callback p_ui_cbs);

/**
 * settings_flow_set_master_volume.
 * @arg1: [IN] vol:volume value.
 * Return: none.
 */
void settings_flow_set_master_volume(uint32_t vol);
/**
 * settings_flow_get_master_volume.
 * @arg1: none.
 * Return: volume value from audio service.
 */
uint32_t settings_flow_get_master_volume(void);

/* backlight */
/**
* settings_flow_get_backlight_value - get the backlight timeout.
* @arg1: none.
* Return: :backlight value.
* Description: get the backlight value.
*/
uint32_t settings_flow_get_backlight_value(void);

/**
* settings_flow_get_brightness - get the brightness value.
* @arg1: none.
* Return: return brightness value.
* Description: get the brightness value.
*/
uint32_t settings_flow_get_brightness(void);

/**
* settings_flow_set_backlight_value - set the backlight value.
* @arg1: [IN] backlight_value:brightness value.
* Return: if success return 0, else return other value.
* Description: set backlight value.
*/
int32_t settings_flow_set_backlight_value(uint32_t backlight_value);

/**
* settings_flow_set_brightness - set the brightness value.
* @arg1: [IN] brightness:brightness value.
* Return: if success return 0, else return other value.
* Description: set brightness value.
*/
int32_t settings_flow_set_brightness(uint32_t brightness);

/**
 settings_flow_get_sn
 * @arg1: [OUT] p_sn0:sn0 value.
 * @arg2: [OUT] p_sn1:sn1 value.
 * Return: 0,success,not 0,fail.
 */
int32_t settings_flow_get_sn(char *p_sn0, char *p_sn1);

/**
 * settings_flow_get_imei.
 * @arg1: [OUT] p_imei:device imei value.
 * Return: 0,success;1, fail.
 */
int32_t settings_flow_get_imei(char *p_imei);
/**
 * settings_flow_get_imsi.
 * @arg1: [OUT] p_imei:device imsi value,imsi maxlen is 15.
 * Return: 0,success;1, fail.
 */
int32_t settings_flow_get_imsi(char *p_imsi);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __SETTINGS_FLOW_H__ */

