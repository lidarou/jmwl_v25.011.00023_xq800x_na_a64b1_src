/*****************************************************************************
 * Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef __IV_SYS_H__
#define __IV_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define IV_SYS_FILE_PATH_MAX_LEN 128
/********************************Macro Definition****************************/

/*Defines the certificate type*/
typedef enum
{
    IV_SYS_CERTIFICATE_TYPE_CERT = 0, /*cert file certificate*/
    IV_SYS_CERTIFICATE_TYPE_KEY  = 1, /*key file certificate*/

    IV_SYS_CERTIFICATE_TYPE_BUTT
} iv_sys_certificate_type_e;

typedef enum
{
    IV_SYS_RECONNECT_STATUS  = 0,
    IV_SYS_DISCONNECT_STATUS = 1,

    IV_SYS_INVALID_STATUS
} iv_sys_offline_status_type_e;

/**
 * SDK log print/upload level
 */
typedef enum
{
    IV_eLOG_DISABLE = 0,
    IV_eLOG_ERROR   = 1,
    IV_eLOG_WARN    = 2,
    IV_eLOG_INFO    = 3,
    IV_eLOG_DEBUG   = 4
} iv_sys_log_level_type_e;

typedef struct {
    char dev_cert_file[IV_SYS_FILE_PATH_MAX_LEN];
    char dev_pkey_file[IV_SYS_FILE_PATH_MAX_LEN];
} iv_sys_cert_info;

typedef struct {
    /*product id*/
    char *product_id;
    /*device name*/
    char *device_name;
    /* region */
    char *region;
    /* product sercert, dynamic register need */
    char *product_secret;
    /*CERTIFICATE_TYPE_CERT:cert path; CERTIFICATE_TYPE_KEY:key*/
    union {
        iv_sys_cert_info *device_cert;
        char *device_key;
    };
} iv_sys_device_info;

/*Defines the parameter of system init */
typedef struct iv_sys_init_parm_s {
    /*read-write directory, can not save after power off*/
    char sys_cache_path[IV_SYS_FILE_PATH_MAX_LEN];
    /*read-write directory, can save after power off*/
    char sys_store_path[IV_SYS_FILE_PATH_MAX_LEN];
    /*read-write directory, can save device_info.json*/
    char *dev_info_path;
    /*device information*/
    iv_sys_device_info *device_info;
    // timeout value (unit: ms) mqtt communication command, suggest value: (5 * 1000)ms
    uint32_t command_timeout;
    //  keep alive time interval in millisecond,suggest value: (240 * 1000)ms
    uint32_t keep_alive_ms;
    //  mqtt ping packet send interval, must lower than keep_alive_ms
    uint32_t mqtt_ping_interval_ms;
    // flag of auto reconnection, 1 is enable and recommended
    uint8_t auto_connect_enable;
    /*mqtt recv buffer max size, default: 2048 */
    uint32_t mqtt_recv_buf_max_size;
    /*mqtt write buffer max size, default:2048 */
    uint32_t mqtt_write_buf_max_size;
    /*max channel number, 0 : device is IPC; other:device is NVR, means max ipc number*/
    uint32_t max_channel_num;
    /*internal communicate use http or https, default 0 means use https*/
    uint32_t comm_use_http;
    /*for p2p proxy mode, set to 1, default 0 means no proxy*/
    uint32_t p2p_use_proxy;
    /*brief: the callback function of device online */
    void (*iv_sys_online_cb)(uint64_t u64NetDateTime);
    /*brief: the callback function of device offline */
    void (*iv_sys_offline_cb)(iv_sys_offline_status_type_e status);
    /*brief: the callback function of module status,
      module_status: bit 0: cs status, bit 1: ai status */
    void (*iv_func_module_status_cb)(uint32_t module_status);
} iv_sys_init_parm_s;

/********************************Macro Definition end*************************/

/********************************API declaration******************************/
/*brief: init iv system*/
int iv_sys_init(const iv_sys_init_parm_s *pstInitParm);

/*brief: exit iv system */
int iv_sys_exit(void);

/*brief: get system certificate type*/
iv_sys_certificate_type_e iv_sys_get_certificate_type(void);

/*brief: get system time, Param: ms*/
int iv_sys_get_time(uint64_t *Param);

/*brief: sync system time, Param: ms*/
// int iv_sys_sync_time(uint64_t *Param);

/*brief: set log print level*/
void iv_sys_set_log_level(iv_sys_log_level_type_e level);

/*brief: set log upload level*/
int iv_sys_set_upload_level(iv_sys_log_level_type_e level);

/*@brief: Do dynamic register/create device */
int iv_sys_dyn_reg_device(iv_sys_device_info *psys_devInfo, char *dev_psk, int dev_psk_len);

/*brief: report offline message*/
int iv_sys_active_offline(void);
/********************************API declaration end******************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IV_SYS_H__ */
