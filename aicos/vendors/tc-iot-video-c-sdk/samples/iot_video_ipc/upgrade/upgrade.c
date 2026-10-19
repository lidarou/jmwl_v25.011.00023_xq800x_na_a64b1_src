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
#define AIC_THIS_FILE    0x124fce
#define LOG_MODULE    AMOD_VENDOR

#include "upgrade.h"
#include "iv_err.h"
#include "iv_ota.h"
#include "qcloud_iot_export.h"
#include "tc_iot_hal.h"

#define OTA_FIRMWARE_PATH    "/tmp"
#define OTA_FIRMWARE_VERSION "1.0.0"

void upgrade_firmware_update(char *firmware_path, uint32_t firmware_len)
{
    void *fp = NULL;
    Log_d("firmware file:%s,len:%d", firmware_path, firmware_len);

    fp = HAL_FileOpen(firmware_path, "rb");
    if (!fp) {
        Log_e("open firmware file:%s failed", firmware_path);
        iv_ota_update_progress(IV_OTA_PROGRESS_TYPE_FAIL, IV_OTA_FAIL_TYPE_OPEN_FILE);
        upgrade_exit();
        return;
    }
    uint32_t file_size = HAL_FileSize(fp);
    if (firmware_len != file_size) {
        Log_e("%s file size wrong, firmware_len:%u, file_size:%u",
                firmware_path, firmware_len, file_size);
        iv_ota_update_progress(IV_OTA_PROGRESS_TYPE_FAIL, IV_OTA_FAIL_TYPE_WRONG_SIZE);
        upgrade_exit();
        return;
    }
    fclose(fp);
    fp = NULL;

    iv_ota_update_progress(IV_OTA_PROGRESS_TYPE_WRITE_FLASH, 0);
    // do the upgrade job
    HAL_SleepMs(1000);
    iv_ota_update_progress(IV_OTA_PROGRESS_TYPE_SUCCESS, 0);
    upgrade_exit();
    return;
}

/*
\brief: 	upgrade init
\attention:	N/A
\param:		N/A
\return:	void
*/
int upgrade_init(void)
{
    int err_code = 0;
    iv_ota_init_parm_s stUpgradeInitParm;
    memset(&stUpgradeInitParm, 0, sizeof(iv_ota_init_parm_s));
    stUpgradeInitParm.iv_ota_firmware_update_cb = upgrade_firmware_update;
    strncpy(stUpgradeInitParm.firmware_path, OTA_FIRMWARE_PATH, sizeof(stUpgradeInitParm.firmware_path));
    strncpy(stUpgradeInitParm.firmware_version, OTA_FIRMWARE_VERSION, sizeof(stUpgradeInitParm.firmware_version));
    err_code = iv_ota_init(&stUpgradeInitParm);
    if (err_code < 0) {
        Log_e("iv_ad_ap_init error:%d", err_code);
    }

    return 0;
}

/*
\brief: 	upgrade exit
\attention:	N/A
\param:		N/A
\return:	void
*/
int upgrade_exit(void)
{
    return iv_ota_exit();
}
