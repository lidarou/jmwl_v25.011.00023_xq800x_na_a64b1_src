/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of usb.
 */

#ifndef __USB_FLOW_H__
#define __USB_FLOW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "usb_service.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    USB_STATE_PLUG_IN, /* plug in */
    USB_STATE_PLUG_OUT, /* plug out */
}usb_state_e;

typedef enum {
    USB_MODE_LOG_AT_DIAG,
    USB_MODE_RNDIS,
    USB_MODE_ECM, /* network */
    USB_MODE_MSC, /* sd card */
    USB_MODE_UNKNOW_FUNC,
}usb_mode_e;

typedef struct {
    int32_t (*OnShow)(usb_state_e usb_state);
}usb_flow_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * usb_init - Initial usb flow app.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void usb_flow_init(void);

/**
* usb_flow_get_bat_capacity
* Return: the battery capacity.
* Description: get the battery capacity.
*/
uint8_t usb_flow_get_bat_capacity(void);

/**
 * usb_flow_select_mode - select usb mode.
 * Return: none.
 * Description: select usb mode, set success, exit app; set fail, popup warn msg.
 */
int usb_flow_select_mode(usb_mode_e usb_mode);

/**
* usb_flow_get_sdcard_exist
* Return: .
* Description: true,usb card exsit, false, usb card not exsit.
*/
bool usb_flow_get_sdcard_exist(void);

/**
* usb_flow_register_ui_actions
* @arg1: p_ui_cbs,ui action callbacks.
* Return: None .
* Description: ui update action in ui cbs .
*/
void usb_flow_register_ui_actions(const usb_flow_t *p_ui_cbs);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __USB_FLOW_H__ */

