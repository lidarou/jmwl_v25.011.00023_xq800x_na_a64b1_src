/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_srv_tele_ex.h
 *
 */

#ifndef __AIC_SRV_TELE_EX_H__
#define __AIC_SRV_TELE_EX_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aic_srv_tele_radio.h"
#include "aic_srv_tele_sim.h"
#ifdef CONFIG_TELE_DATA
#include "aic_srv_tele_data.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* aic_srv_tele_sim_get_status_sync - 同步获取sim卡状态的接口.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim卡的id，0-卡槽1，1-卡槽2，目前仅支持单卡（0）.
* @p_sim_status: [OUT] sim卡的状态.
* Return: 成功返回0，失败返回其它值.
*/
int32_t aic_srv_tele_sim_get_status_sync(uint8_t card_id, ts_sim_status_e *p_sim_status);

/**
* aic_srv_tele_sim_get_imsi_sync - synchronous get sim imsi.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim card id, 0- card 1, 1- card 2
* @p_sim_imsi: [OUT] the value of sim imsi.
* Return: 0:success;fail,other value.
*/
int32_t aic_srv_tele_sim_get_imsi_sync(uint8_t card_id, char *p_sim_imsi);

/**
* aic_srv_tele_sim_get_plmn_sync - synchronous get sim plmn.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim card id, 0- card 1, 1- card 2
* @p_plmn_mcc: [OUT] the value of sim plmn p_plmn_mcc,the length more than 3.
* @p_plmn_mnc: [OUT] the value of sim plmn p_plmn_mnc,the length more than 3.
* Return: 0:success;fail,other value.
*/
int32_t aic_srv_tele_sim_get_plmn_sync(uint8_t card_id, char *p_plmn_mcc,char *p_plmn_mnc);

/**
* aic_srv_tele_sim_get_iccid_sync - synchronous get sim iccid.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim card id, 0- card 1, 1- card 2
* @p_sim_iccid: [OUT] the value of sim iccid.
* Return: 0:success;fail,other value.
*/
int32_t aic_srv_tele_sim_get_iccid_sync(uint8_t card_id, char *p_sim_iccid);

/**
* aic_srv_tele_sim_get_msisdn_sync - synchronous get sim self phone number.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim card id, 0- card 1, 1- card 2
* @p_plmn_mcc: [OUT] the value of sim msisdn.
* Return: 0:success;fail,other value.
*/
int32_t aic_srv_tele_sim_get_msisdn_sync(uint8_t card_id, char *p_sim_msisdn);

/**
* aic_srv_tele_sim_select_card_sync - synchronous select sim card.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim card id, 0- card 1, 1- card 2
* Return: 0:success;fail,other value.
*/
int32_t aic_srv_tele_sim_select_card_sync(uint8_t card_id);

/**
* aic_srv_tele_radio_get_signal_info_sync - 同步获取信号强度等相关信息的接口.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim卡的id，0-卡槽1，1-卡槽2，目前仅支持单卡（0）.
* @p_signal_info: [OUT] 信号信息结构体.
* Return: 成功返回0，失败返回其它值.
*         如果client设置了filter属性，则返回TS_RADIO_ERR_FILTER；
*/
int32_t aic_srv_tele_radio_get_signal_info_sync(uint8_t card_id, ts_radio_signal_info_t *p_signal_info);

/**
* aic_srv_tele_radio_get_operator_info_sync - synchronous get operator information api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim id，0-sim 1，1-sim 2，and currently only support single card(0).
* @p_operator_info: [OUT] the operation information.
* Return: if success return 0, else return other value.
*/
int32_t aic_srv_tele_radio_get_operator_info_sync(uint8_t card_id, ts_radio_operator_info_t *p_operator_info);

/**
* aic_srv_tele_radio_get_reg_status_sync - synchronous get network register status
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] ID of sim card slot, 0-card slot 1, 1-card slot 2.(Only support signal card 0)
* @p_reg_status: [OUT] register status.
* Return: 0: success, other: error .
*/
int32_t aic_srv_tele_radio_get_reg_status_sync(uint8_t card_id, ts_radio_reg_status_e *p_reg_status);

/**
* aic_srv_tele_radio_set_current_slot_imei_sync - synchronous set current imei string
* [NOTE]: APIs with suffix _sync are time-consuming.
* @p_imei: [IN] pointer of imsi string want to set.
* Return: 0: success, other: error .
*/
int32_t aic_srv_tele_radio_set_current_slot_imei_sync(char *p_imei);

/**
* aic_srv_tele_radio_get_current_slot_imei_sync - synchronous get current slot imei string
* [NOTE]: APIs with suffix _sync are time-consuming.
* @p_imei: [OUT] pointer of imsi string.
* Return: 0: success, other: error .
* Note: It taskes 5 seconds from booting up to get correct imei .
*/
int32_t aic_srv_tele_radio_get_current_slot_imei_sync(char *p_imei);

/**
* aic_srv_tele_radio_get_cell_info_sync - synchronous get current service cell info
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim slotId.
* @p_cell_info: [OUT] pointer of current service cell info.
* Return: 0: success, other: error .
*/
int32_t aic_srv_tele_radio_get_cell_info_sync(uint8_t card_id, ts_radio_cell_info_t *p_cell_info);

/**
* aic_srv_tele_radio_get_neighbor_cell_info_list_sync - synchronous get neighbor cell info list
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim slotId.
* @p_ncell_info_list: [OUT] pointer of current service cell info list.
* Return: 0: success, other: error .
*/
int32_t aic_srv_tele_radio_get_neighbor_cell_info_list_sync(uint8_t card_id, ts_radio_neighbor_cell_info_list_t *p_ncell_info_list);

/**
* aic_srv_tele_radio_on_sync - the synchronous open/close radio api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim id, 0-sim 1, 1-sim2, currently only support single card(0).
* @raido_on: [IN] true-open radio, false-close radio
* Return: if success return 0, else return other value.
*/
int32_t aic_srv_tele_radio_on_sync(uint8_t card_id, bool raido_on);

/**
* aic_srv_tele_radio_is_on_sync - the synchronous get radio status api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim id, 0-sim 1, 1-sim2, currently only support single card(0).
* Return: true: is on, false: is off.
*/
bool aic_srv_tele_radio_is_on_sync(uint8_t card_id);

#ifdef CONFIG_TELE_DATA
/**
* aic_srv_tele_data_activate_pdp_sync - the synchronous activate pdp api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @card_id: [IN] sim id，0-sim 1，1-sim 2. currently only support single card(0).
* @p_pdp_act_info: [IN] pdp activate information.
* @p_pdp_id: [OUT] the pdp id.
* Return: return 0 if success, else return other value.
*/
int32_t aic_srv_tele_data_activate_pdp_sync(uint8_t card_id, ts_data_pdp_activate_info_t *p_pdp_act_info, uint32_t *p_pdp_id);

/**
* aic_srv_tele_data_deactivate_pdp_sync - the synchronous deactivate pdp api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @pdp_id: [IN] the actived pdp id.
* Return: if success, return 0; else return other value.
*/
int32_t aic_srv_tele_data_deactivate_pdp_sync(uint32_t pdp_id);

/**
* aic_srv_tele_data_get_pdp_status_sync - the synchronous get pdp status api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @pdp_id: [IN] the actived pdp id.
* @p_ip: [OUT] pdp status.
* Return: if success, return 0; else return other value.
*/
int32_t aic_srv_tele_data_get_pdp_status_sync(uint32_t pdp_id, ts_data_pdp_status_e *p_pdp_status);

/**
* aic_srv_tele_data_get_pdp_ip_sync - the synchronous get pdp ip api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @pdp_id: [IN] the actived pdp id.
* @p_ip: [OUT] pdp ip address.
* Return: if success, return 0; else return other value.
*/
int32_t aic_srv_tele_data_get_pdp_ip_sync(uint32_t pdp_id, char *p_ip);

/**
* aic_srv_tele_data_get_current_pdp_info_sync - the synchronous get current pdp information api.
* [NOTE]: APIs with suffix _sync are time-consuming.
* @p_pdp_info: [OUT] the pdp information.
* Return: if success, return 0; else return other value.
*/
int32_t aic_srv_tele_data_get_current_pdp_info_sync(ts_data_pdp_info_t *p_pdp_info);
#endif
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_SRV_TELE_EX_H__ */
