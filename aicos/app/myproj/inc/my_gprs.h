/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_gprs.h
**文件描述:        my_gprs.c头文件声明
**当前版本:        V1.0
*********************************************************************/
#ifndef __MY_GPRS_H__
#define __MY_GPRS_H__

#define ANET_PDP_ACTIVATED      1
#define ANET_PDP_DEACTIVATED    0

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

#define NW_EVENT_SIM_PIN_REQUIRED       0x000F  // 要求输入PIN码

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


typedef enum
{
    NW_REG_STATE_NOT_REGISTERED = 0,    // 网络未注册
    NW_REG_STATE_HOME_NETWORK = 1,      // 网络已注册
    NW_REG_STATE_SEARCHING = 2,         // 网络搜索中
    NW_REG_STATE_DENIED = 3,            // 网络注册被拒绝
    NW_REG_STATE_UNKNOW = 4,            // 网络状态未知
    NW_REG_STATE_ROAMING = 5,           // 网络已注册，漫游
}app_nw_reg_status_e;


#endif

