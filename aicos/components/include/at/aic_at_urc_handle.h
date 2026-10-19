/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for handling AT URC.
 */

#ifndef AIC_AT_URC_HANDLE_H
#define AIC_AT_URC_HANDLE_H

#define AT_MAX_URC_HANDLES 15

#define AT_INVALID_REG_STATE 0xff

#define AT_SIM_CARD_NOT_PLUGGED 0
#define AT_SIM_CARD_PLUGGED 1

/////////// Add By Nestor -- START -- 2024-04-23 ///////////
extern void my_custom_nw_evt_cb(u32 eventId, void* pData, u32 len) __attribute__((weak));

/**
 * Network Events
 */
#define NW_EVENT_SIM_PRESENT            0x0001  //sim car PRESENT
#define NW_EVENT_SIM_ABSENT             0x0002  //sim car not PRESENT

#define NW_EVENT_SIM_READY              0x0003  //sim car ready
#define NW_EVENT_SIM_NOT_READY          0x0004  //sim car not ready

#define NW_EVENT_PS_ON_RSP              0x0005  // response for PS on and off
#define NW_EVENT_PS_OFF_RSP             0x0006  // response for PS on and off

#define NW_EVENT_GPRS_ATTACH_IND        0x0007  // attach to GPRS
#define NW_EVENT_GPRS_DEATTACH_IND      0x0008  // deattach from GPRS
#define NW_EVENT_ATTACH_RSP             0x0009  // response for GPRS attach and deattach

#define NW_EVENT_PDP_ACT_IND            0x000A  // PDP is active
#define NW_EVENT_PDP_DEACT_IND          0x000B  // pdp is deactived by network

#define NW_EVENT_GSM_REGISTERED_IND     0x000C  // GSM ok
#define NW_EVENT_GSM_UNREGISTERED_IND   0x000D  // unregister from GSM

#define NW_EVENT_SYSMODE_UPDATE         0x000E

#define NW_EVENT_SIM_PIN_REQUIRED       0x000F  // “™«Û ‰»ÎPIN¬Î

#define NW_EVENT_RSSI_CHANGE_IND        0x0010  // RSSI level has changed.

#define NW_EVENT_CELL_INFO_RSP          0x0011

#define NW_EVENT_SMS_READY              0x0012  // sms ready
#define NW_EVENT_SMS_SEND_SUCCESS       0x0013  // sms send success
#define NW_EVENT_SMS_SEND_FAIL          0x0014  // sms send fail
#define NW_EVENT_SMS_READ_CNF           0x0015  // sms read result
#define NW_EVENT_SMS_IND                0x0016  // new sms

#define NW_EVENT_CALL_INCOMING_IND      0x0017  // call incoming
#define NW_EVENT_CALL_DISCONNECTED_IND  0x0018

#define NW_EVENT_LBS_GET_POS_RSP        0x0019
#define NW_EVENT_NETTIME_SYNC_IND       0x001A  // Net time has been synced
/////////// Add By Nestor -- END -- 2024-04-23 ///////////


/* Define global functions. */
u32 aic_invoke_urc_handler(u8 *urc_str);
void aic_at_urc_handle_init(void);

#endif /* AIC_AT_URC_HANDLE_H */
