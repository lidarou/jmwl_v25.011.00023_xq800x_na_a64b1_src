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
#define AIC_THIS_FILE    0x0effd0
#define LOG_MODULE    AMOD_VENDOR

#include "tc_iot_log.h"
#include "iv_dm.h"

// 打开该宏用于测试用户自定义物模型功能
// 需要保证云端物模型配置和代码保持一致
// #define TEST_USER_DEFINE_PROPERTY

#ifdef TEST_USER_DEFINE_PROPERTY
#include "iv_usrex.h"
#endif

/* AIC_MODIFY
static int ivm_property_report_result(void *param, iv_dm_report_result_e code)
{
    Log_i("param %s, code %d", (char *)param, code);
    return 0;
}
*/

int dm_sync(void)
{
    Log_d("data model sync called");
    int rc = 0;
#ifdef TEST_USER_DEFINE_PROPERTY
    rc = iv_dm_property_sync(5000);
    if (rc) {
        Log_e("data model sync failed %d", rc);
    } else {
        Log_d("data model sync value: %d",
              g_ivm_objs.ProWritable.m_battery_percentage);
    }
#endif
    return rc;
}


int dm_init(void)
{
    int rc = 0;
    iv_dm_init_parm_s stInitParm;
#ifdef TEST_USER_DEFINE_PROPERTY
    stInitParm.iv_dm_env_init_cb = ivm_env_init;
#else
    stInitParm.iv_dm_env_init_cb = NULL;
#endif
    rc                           = iv_dm_init(&stInitParm);
    if (rc < 0) {
        Log_e("model init failed!");
        return rc;
    }

#ifdef TEST_USER_DEFINE_PROPERTY
    ivm_ProWritable_setInt(battery_percentage, 58);
#endif
    return rc;
}


int dm_exit(void)
{
    return iv_dm_exit();
}
