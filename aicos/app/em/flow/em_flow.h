/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of engineer mode.
 */

#ifndef __EM_FLOW_H__
#define __EM_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
*      INCLUDES
*********************/
#include "aic_type.h"
#include "aic_srv_tele.h"


/*********************
*      DEFINES
*********************/
#define EM_FLOW_SN_LEN               (64)

#define EM_SPEED_NO_ERROR 0
#define EM_CREATE_SOCKET_ERROR 1001

/**********************
 *      TYPEDEFS
 **********************/

typedef struct
{
    int32_t status;
    int32_t speed;  //Bytes/s
}em_network_speed_t;

/**********************
* GLOBAL PROTOTYPES
**********************/



/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
 * em_flow_get_sn0,get sn0 value.
 * @arg1: p_sn0,string of sn0.
 * Return: success,0,fail,not 0.
 */
int32_t em_flow_get_sn0(char *p_sn0);

/**
 * em_flow_get_sn1,get sn1 value.
 * @arg1: p_sn0,string of sn1.
 * Return: success,0,fail,not 0.
 */
int32_t em_flow_get_sn1(char *p_sn1);

/**
 * em_flow_get_imei,get imei value.
 * @arg1: p_sn0,string of sn1.
 * Return: success,0,fail,not 0.
 */
int32_t em_flow_get_imei(char *p_imei);

/**
 * em_flow_get_calibration_status,get sn1 value.
 * @arg1: none.
 * Return: true,has calibrated,false,not calibrated.
 */
bool em_flow_get_calibration_status(void);

/**
 * em_flow_get_sim_status,get current sim status.
 * @arg1: p_sim_status,sim status,one of value(not ready/not exist/ready/pin lock/puk lock/puk lock/pin2 lock/puk2 lock).
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_sim_status(char *p_sim_status);

/**
 * em_flow_get_sim_imsi,get current sim imsi.
 * @arg1: p_sim_imsi,sim imsi,imsi value lenght is 15.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_sim_imsi(char *p_sim_imsi);

/**
 * em_flow_get_sim_phomenum,get current sim own number.
 * @arg1: p_sim_phonenum,sim ownnumber,ownnumber value lenght is 11.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_sim_phomenum(char *p_sim_phonenum);

/**
 * em_flow_get_regstatus,get register status.
 * @arg1: p_reg_status,include no service/in service/emergency only/power off.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_regstatus(char *p_reg_status);

/**
 * em_flow_get_operatorname,get operator long name.
 * @arg1: p_opearor_long_name,China-mobile/China-unicom.
 * @arg2: p_opearor_short_name,CMCC/CUCC/CTC.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_operatorname(char *p_opearor_long_name, char *p_operator_short_name);

/**
 * em_flow_get_signalinfo,get current signal info.
 * @arg1: p_signal_info,signal stregth,signal level.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_signalinfo(ts_radio_signal_info_t *p_signal_info);

/**
 * em_flow_get_cellinfo,get cell info.
 * @arg1: p_cell_info,cell information.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_cellinfo(ts_radio_cell_info_t *p_cell_info);

/**
 * em_flow_get_neighborinfo,get neighbor cell info.
 * @arg1: p_neighbor_info_list,neighbor cell information.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_neighborinfo(ts_radio_neighbor_cell_info_list_t *p_neighbor_info_list);

/**
 * em_flow_get_pdp_status,get pdp status.
 * @arg1: p_pdp_status,active or not active.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_pdp_status(char *p_pdp_status);

/**
 * em_flow_get_ip_address,get ip adrress.
 * @arg1: p_ip_addr,ip address value.
 * Return: 0,success,not 0,fail.
 */
int32_t em_flow_get_ip_address(char *p_ip_addr);

/**
 * em_flow_sms_register,register sms moudle.
 * @arg1: none.
 * Return: sms app handler, greater 0,fail.
 */
int32_t em_flow_sms_register(void);

/**
 * em_flow_sms_unregister,unregister sms moudle.
 * @arg1: none.
 * Return:  0,success,not 0,fail.
 */
int32_t em_flow_sms_unregister(void);

/**
 * em_flow_get_smssc,get sms server center number.
 * @arg1: [out]p_smsc,smsc number.
 * Return: sms app handler, greater 0,fail.
 */
int32_t em_flow_get_smsc(char *p_smsc);

/**
 * em_flow_get_smssc,get sms server center number.
 * @arg1: [in]p_dest_num,destination number.
 * @arg2: [in]p_sms_content,sms content.
 * Return: 0,sucess,not 0,fail.
 */
int32_t em_flow_send_sms(const char *p_dest_num, const char *p_sms_content);

/**
 * em_flow_set_radio_report_status,set network info report filter.
 * @arg1: [in]report_status,false,(0xFFFFFFFF,close all network infomation)open,(0x0,normal report) .
 * Return: 0,sucess,not 0,fail.
 */
int32_t em_flow_set_radio_report_status(bool report_status);

/**
 * em_flow_get_download_speed, network download speed test.
 * @arg1: [in]srv_func_callback .
 * Return: 0,sucess,not 0,fail.
 */
int32_t em_flow_get_download_speed(srv_func_callback callback);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __EM_FLOW_H__ */

