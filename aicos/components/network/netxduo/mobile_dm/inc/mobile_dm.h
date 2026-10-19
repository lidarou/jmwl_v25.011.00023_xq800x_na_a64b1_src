/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef __MOBILE_DM_H__
#define __MOBILE_DM_H__

#include "MOBILE_DM_CFG.h"

#define MOBILE_DM_CONFIG_GET_URL "/dm-multi-accept/multi/getMultiTerminalConfig"
#define MOBILE_DM_REGISTER_URL "/dm-multi-accept/multi/acceptMultiTerminalRegInfo"
#define MOBILE_DM_KEEP_ALIVE_URL "/dm-multi-accept/multi/acceptMultiTerminalHeartbeatInfo"

#define MOBILE_DM_HTTPS_CLIENT_NAME "MOBILE DM HTTPS Client"
#define MOBILE_DM_HTTPS_HEADER_KEY_1 "Content-Type"
#define MOBILE_DM_HTTPS_HEADER_VALUE_1 "application/json;charset=utf-8"
#define MOBILE_DM_HTTPS_HEADER_KEY_2 "cache-control"
#define MOBILE_DM_HTTPS_HEADER_VALUE_2 "no-cache"

#define MOBIIE_DM_HTTPS_URL_LEN 256

#define MOBILE_DM_BRAND "GX318"
#define MOBILE_DM_MODEL "c_h1"
#define MOBILE_DM_APPKEY "M100001088"
#define MOBILE_DM_SDK_VERSION "***"
#define MOBILE_DM_API_VERSION "4.0.1"
#define MOBILE_DM_API_TYPE "I"
#define MOBILE_DM_TEMPLATE_ID "TY000122"
#define MOBILE_DM_IMEI2 "***"
#define MOBILE_DM_CONFIG_TYPE "***"
#define MOBILE_DM_PREFIELD1 "***"
#define MOBILE_DM_PREFIELD2 "***"

#define MOBILE_DM_REGIST_TYPE "autoRegist"
#define MOBILE_DM_KEEPALIVE_TYPE "keepAlive"

#define MOBILE_DM_NET_TYPR "lte"
#define MOBILE_DM_ROM_BYTES "32K"
#define MOBILE_DM_RAM_BYTES "4M"
#define MOBILE_DM_SOFTWARE_NAME "GX318"
#define MOBILE_DM_INFO_NOT_SET "***"

#define MOBILE_DM_MAX_DEVICE_INFO_SIZE 2048
#define MOBILE_DM_MAX_CONFIG_POST_SIZE 300
#define MOBILE_DM_MAX_POST_SIZE 2348
#define MOBILE_DM_MAX_RESP_BUF_SIZE 1024
#define MOBILE_DM_MAX_ENDPOINT_SIZE 255
#define MOBILE_DM_DEVICE_INFO_COUNT 23
#define MOBILE_DM_ENCRYP_DATA_LEN 64
#define MOBILE_DM_DEVICE_INFO_LEN 32
#define MOBILE_DM_AES_KEY_BITS 256
#define MOBILE_DM_AES_IV_SIZE 16
#define MOBILE_DM_RESP_CODE_LEN 20
#define MOBILE_DM_YES 1
#define MOBILE_DM_NO 0

#define MOBILE_DM_HTTPS_CLIENT_CREATE_FAIL 1
#define MOBILE_NO_DM_HTTPS_CLIENT 2
#define MOBILE_DM_HEART_BEAT_FAIL 3
#define MOBILE_DM_HTTPS_CLIENT_DELETE_FAIL 4
#define MOBILE_DM_CONFIGURATION_GET_FAIL -1
#define MOBILE_DM_DEVICE_REGISTER_FAIL -2
#define MOBILE_DM_DEVICE_INFO_PROCESS_FAIL -3
#define MOBILE_DM_SERVICE_INIT_FAIL -4

/* Define MQTT AT thread events. */
#define MOBILE_DM_PDP_ACTIVE_EVENT 0x01
#define MOBILE_DM_KEEP_ALIVE_EVENT 0x02
#define MOBILE_DM_REPORT_TIMEOUT_EVENT 0x04

#define MOBILE_DM_ANY_EVENT 0xFF

/* dm https response code */
#define MOBILE_DM_OK              "0000"
#define MOBILE_DM_VALIDATE_ERROR  "4001"
#define MOBILE_DM_CONFIGE_ERROR   "4002"
#define MOBILE_DM_PARAM_ERROR     "4003"
#define MOBILE_DM_ENDPOINT_ERROR  "4004"
#define MOBILE_DM_SYSTEM_ERROR    "4005"

#define MOBILE_DM_FMT     \
    "\"{\\\"imsi\\\":\\\"%s\\\"," \
    "\\\"mac\\\":\\\"%s\\\"," \
    "\\\"rom\\\":\\\"%s\\\"," \
    "\\\"ram\\\":\\\"%s\\\"," \
    "\\\"cpu\\\":\\\"%s\\\"," \
    "\\\"sysVersion\\\":\\\"%s\\\"," \
    "\\\"softwareVer\\\":\\\"%s\\\"," \
    "\\\"softwareName\\\":\\\"%s\\\"," \
    "\\\"volte\\\":\\\"%s\\\"," \
    "\\\"netType\\\":\\\"%s\\\"," \
    "\\\"phoneNumber\\\":\\\"%s\\\"," \
    "\\\"routerMac\\\":\\\"%s\\\"," \
    "\\\"bluetoothMac\\\":\\\"%s\\\"," \
    "\\\"sn\\\":\\\"%s\\\"," \
    "\\\"gpu\\\":\\\"%s\\\"," \
    "\\\"board\\\":\\\"%s\\\"," \
    "\\\"resolution\\\":\\\"%s\\\"," \
    "\\\"batteryCapacity\\\":\\\"%s\\\"," \
    "\\\"screenSize\\\":\\\"%s\\\"," \
    "\\\"networkStatus\\\":\\\"%s\\\"," \
    "\\\"wearingStatus\\\":\\\"%s\\\"," \
    "\\\"appInfo\\\":\\\"%s\\\"," \
    "\\\"imsi2\\\":\\\"%s\\\"}\""

#define MOBILE_DM_ENDPOINT_FMT     \
    "{\"endpoint\":\"%s||" \
    "%s||%s||%s||%s||%s||%s||%s" \
    "||%s||%s||%s||%s\",\"configType\":\"%s\"}"

#define MOBILE_DM_REGISTER_DATA_FMT     \
    "{\"endpoint\":\"%s||" \
    "%s||%s||%s||%s||%s||%s||%s" \
    "||%s||%s||%s||%s\",\"deviceInfo\":%s}"

typedef enum {
    imsi         = 0,
    mac,
    rom,
    ram,
    cpu,
    sys_ver,
    soft_ware_ver,
    soft_ware_name,
    volte,
    net_type,
    phone_num,
    router_mac,
    bluetooth_mac,
    sn,
    gpu,
    board,
    resolution,
    battery_capacity,
    screen_size,
    network_status,
    wearing_status,
    app_info,
    imsi2,
} MOBILE_DM_CONFIG_INDEX;

typedef struct mobile_dm_device_info
{
    u8 *info;
} MOBILE_DM_DEVICE_INFO;

typedef struct mobile_dm_encryp_device_info
{
    u8 *encryp_data;
} MOBILE_DM_ENCRYP_DEVICE_INFO;

typedef struct mobile_dm_rule_config
{
    u32 report_time;
    u32 report_num;
    u32 heart_beat_time;
    u32 retry_interval;
    u32 retry_num;
} MOBILE_DM_RULE_CONFIG;

typedef struct mobile_dm_address_config
{
    u8 *address_cfg;
} MOBILE_DM_ADDRESS_CONFIG;

typedef struct mobile_dm_field_config_map
{
    u8 *mobile_md_config_str;      /* String of config    */
    u8  mobile_md_config_status;     /* 1:Y or 0:N      */
} MOBILE_DM_FIELD_CONFIG_MAP;

u32 mobile_dm_init(void);
u32 mobile_dm_stop(void);
int mobile_dm_config_write(MOBILE_DM_CFG *cfg);
int mobile_dm_config_read(MOBILE_DM_CFG *cfg);
void mobile_dm_config_enable(bool enable);
void mobile_dm_report_test(void);
#endif
