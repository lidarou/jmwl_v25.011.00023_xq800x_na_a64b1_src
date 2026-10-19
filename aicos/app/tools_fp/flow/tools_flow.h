

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
#ifndef __TOOLS_FLOW_H__
#define __TOOLS_FLOW_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*********************
     *      INCLUDES
     *********************/
#include "aic_type.h"
#include "aic_version.h"

    /*********************
     *      DEFINES
     *********************/
#define     SETTINGS_FLOW_AP_VERSION_LEN (AIC_VERSION_LEN)
#define     SETTINGS_FLOW_IMEI_LEN  (30)
#define     SETTINGS_FLOW_IMSI_LEN  (16)
#define     SETTINGS_FLOW_SN_LEN    (64)
#define     SETTINGS_FLOW_CARD_ID_MAX (2)

    /**********************
     *      TYPEDEFS
     **********************/
    typedef enum {
        SIM_EXIST_STATUS_NO,                        /* 0,no sim */
        SIM_EXIST_STATUS_SIM1,                      /* 1,sim1 exist */
        SIM_EXIST_STATUS_SIM2,                      /* 2,sim2 exist */
        SIM_EXIST_STATUS_BOTH,                      /* 3,sim1&sim2 exist */
        SIM_EXIST_STATUS_MAX
        } settings_sim_exist_status_e;


    /* factoryreset result:0,success,other is fail */
    typedef int32_t(*settings_flow_factoryreset_callback) (int32_t reset_result);

    /* flightmode on/off result:0,success,other is fail */
    typedef int32_t(*settings_flow_flightmode_callback) (int32_t flightmode_result);

    /* flightmode on/off result:0,success,other is fail */
    typedef int32_t(*settings_flow_selectsim_callback) (int32_t selectsim_result);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __TOOLS_FLOW_H__ */

