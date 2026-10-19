/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of statusbar.
 */

#ifndef __STATUSBAR_FLOW_H__
#define __STATUSBAR_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "aic_type.h"

typedef struct {
    void (*BatteryStatusUpdate)(uint8_t bat_capacity, uint8_t charge_status);
    void (*SimStatusUpdate)(bool is_sim_exist);
    void (*SignalLevelUpdate)(uint32_t signal_level);
    void (*OperatorNameUpdate)(char *p_opera_name);
    void (*ImsStatusUpdate)(bool ims_status);
    void (*TimeUpdate)(char *p_sys_time);
}statusbar_flow_t;


/*********************
 *      DEFINES
 *********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * statusbar_flow_register_ui_actions
 * @arg1: p_ui_cbs:update ui callbacks
 * @arg2: none.
 * Return: none.
 */
void statusbar_flow_register_ui_actions(const statusbar_flow_t *p_ui_cbs);

/**
 * statusbar_flow_set_latest_value
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void statusbar_flow_set_latest_value(void);

/**
 * statusbar_flow_init
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void statusbar_flow_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __STATUSBAR_FLOW_H__ */
