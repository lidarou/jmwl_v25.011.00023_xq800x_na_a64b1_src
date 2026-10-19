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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06a7a0
#define LOG_MODULE    AMOD_VENDOR

#include "iv_system.h"

#include <sys/time.h>
#include <time.h>

#include "iv_config.h"
#include "iv_sys.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_export_devinfo.h"

#define MAX_SIZE_OF_DEV_SECRET (64)
#define SYS_NULL_OF_DEV_SECRET "YOUR_DEVICE_SECRET"
static char sg_device_state = DEVICE_CONNECTING_STATUE;

int settimeofday (const struct timeval *tv, const struct timezone *tz)
{
    return 0;
}

static void device_online(uint64_t u64NetDateTime)
{
    sg_device_state = DEVICE_ONLINE_STATUE;

    Log_d("online time:%lldms", u64NetDateTime);

    // just demo how to sync local time with server time
    // system("ln -s /usr/share/zoneinfo/Asia/Shanghai /tmp/localtime");
    struct timeval stime;
    stime.tv_sec  = (u64NetDateTime / 1000);
    stime.tv_usec = ((u64NetDateTime % 1000) * 1000);

    if (0 != settimeofday(&stime, NULL)) {
        Log_e("update local time failed");
    }
}

static void device_offline(iv_sys_offline_status_type_e status)
{
    Log_d("offline %d", status);
    if (IV_SYS_DISCONNECT_STATUS == status) {
        sg_device_state = DEVICE_OFFLINE_STATUE;
    } else {
        sg_device_state = DEVICE_CONNECTING_STATUE;
    }
}

const char *params_device_info_file = "/O/device_info.json";

int sys_init(void)
{
    int eErrCode = 0;
    iv_sys_init_parm_s stSysInitParameters;
    memset(&stSysInitParameters, 0, sizeof(iv_sys_init_parm_s));

    // 无法使用device_info文件的话可以用这个硬编码方式进行测试
#if 0
    iv_sys_device_info dev_info     = {0};
    char dev_secret[MAX_SIZE_OF_DEV_SECRET] = {0};

    dev_info.product_id             = "xxx";
    dev_info.device_name            = "xxxx";
    dev_info.device_key             = SYS_NULL_OF_DEV_SECRET;
    stSysInitParameters.device_info = &dev_info;

#ifdef SYS_DEV_DYN_REG_ENABLED
    dev_info.product_secret = "xxxxxxxxx";
    if (0 == strcmp(dev_info.device_key, SYS_NULL_OF_DEV_SECRET)) {
        eErrCode = iv_sys_dyn_reg_device(&dev_info, dev_secret, sizeof(dev_secret));
        if (eErrCode < 0) {
            Log_e("dynamic register dev fail:%d", eErrCode);
            return eErrCode;
        }
    }
    dev_info.device_key = dev_secret;
    Log_i(
        "dynamic register success,productID: %s, devName: %s, "
        "device_secret: %s",
        dev_info.product_id, dev_info.device_name, dev_info.device_key);
#endif

#else
    int ret                                 = 0;
    char dev_secret[MAX_SIZE_OF_DEV_SECRET] = {0};
    DeviceInfo DevInfoTmp                   = {0};
    iv_sys_device_info sys_dev_info         = {0};

    strcpy(stSysInitParameters.sys_cache_path, "/O/video_cache");
    strcpy(stSysInitParameters.sys_store_path, "/O");

    stSysInitParameters.dev_info_path = (char*)params_device_info_file;

#ifdef SYS_DEV_DYN_REG_ENABLED
    
    ret = IOT_Get_DevInfo_From_File(stSysInitParameters.dev_info_path, &DevInfoTmp);
    if (ret) {
        Log_e("read device information faile");
        return ret;
    }

    if (0 == strcmp(DevInfoTmp.device_secret, SYS_NULL_OF_DEV_SECRET)) {
        Log_d("dev psk not exist!");
        sys_dev_info.product_id     = DevInfoTmp.product_id;
        sys_dev_info.product_secret = DevInfoTmp.product_secret;
        sys_dev_info.device_name    = DevInfoTmp.device_name;
        eErrCode                    = iv_sys_dyn_reg_device(&sys_dev_info, dev_secret, sizeof(dev_secret));
        if (eErrCode < 0) {
            Log_e("dynamic register dev fail:%d", eErrCode);
            return eErrCode;
        }

        strncpy(DevInfoTmp.device_secret, dev_secret, MAX_SIZE_OF_DEV_SECRET);
        ret = IOT_Set_DevInfo(&DevInfoTmp);
        if (QCLOUD_RET_SUCCESS != ret) {
            Log_e("devices info save fail");
            return ret;
        }

        Log_i(
            "dynamic register success,productID: %s, devName: %s, "
            "device_secret: %s",
            sys_dev_info.product_id, sys_dev_info.device_name, dev_secret);
    } else {
        Log_i("dev psk exist");
    }
#endif

#endif

    iv_sys_set_log_level(IV_eLOG_DEBUG);

    stSysInitParameters.iv_sys_online_cb    = device_online;
    stSysInitParameters.iv_sys_offline_cb   = device_offline;
    stSysInitParameters.command_timeout     = 10 * 1000;
    stSysInitParameters.keep_alive_ms       = 60 * 1000;
    stSysInitParameters.comm_use_http       = 0;
    stSysInitParameters.auto_connect_enable = 1;
    stSysInitParameters.max_channel_num     = MAX_CHANNEL_NUM;
    eErrCode                                = iv_sys_init(&stSysInitParameters);
    if (eErrCode < 0) {
        Log_e("iv_sys_init error:%d", eErrCode);
    }

    sg_device_state = DEVICE_CONNECTING_STATUE;
    return eErrCode;
}

int sys_exit(void)
{
    return iv_sys_exit();
}

char sys_get_status(void)
{
    return sg_device_state;
}
