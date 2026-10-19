/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_common.h
**文件描述:        my_common.c头文件声明
**当前版本:        V1.0
*********************************************************************/

#ifndef __MY_COMMON_H__
#define __MY_COMMON_H__

#include <aic_type.h>
#include <alog_api.h>
#include <aic_osa_ex.h>
#include <aic_init.h>
#include <aic_errno.h>

//#include "fs_api.h"

#include "nx_api.h"
#include "nx_ip.h"
#include "nxd_bsd.h"

#include <aic_gpio_pin.h>
#include <gpio_inc.h>
#include <serial_inc.h>
#include <power_supply.h>
#include <eipu.h>

// sleep lock API
#include <slp_inc.h>

//#include <aic_module_net.h>
#include <aic_net_pdp_mgr.h>
#include <aic_at_process_common.h>

#include <aic_md_api.h>
#include <aic_md_id.h>

#include <aic_srv_tele.h>
#include <aic_srv_tele_radio.h>

#include <aic_cali_band_cmd.h>

#include <usb_service.h>

// mqtt
#include <aic_mqtt_client.h>

// for debug
#define WIFISCAN_ENABLE     0

#if WIFISCAN_ENABLE
#include <aic_wifi_scan.h>
#endif

#define JM_Sleep   aos_sleep

#define malloc     aic_malloc
#define free       aic_free

typedef int  SOCKET;

typedef unsigned long long  uint64;

typedef unsigned int    uint32;
typedef signed int      int32;

typedef unsigned short  uint16;
typedef signed short    int16;

typedef unsigned char   uint8;
typedef signed char     int8;


#include "my_gpio.h"
#include "my_uart.h"
#include "my_gps.h"
#include "my_log.h"
#include "my_main.h"
#include "my_gprs.h"
#include "my_mqtt.h"
#include "my_tool.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#endif

