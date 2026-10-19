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

#ifndef QCLOUD_IOT_EXPORT_LOG_H_
#define QCLOUD_IOT_EXPORT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_log.h"

/* Macro for debug mode */
// #define IOT_DEBUG
#ifdef IOT_DEBUG
#define IOT_FUNC_ENTRY                                              \
    {                                                               \
        HAL_Printf("FUNC_ENTRY:   %s L#%d \n", __FUNCTION__, __LINE__); \
    }
#define IOT_FUNC_EXIT                                              \
    {                                                              \
        HAL_Printf("FUNC_EXIT:   %s L#%d \n", __FUNCTION__, __LINE__); \
        return;                                                    \
    }
#define IOT_FUNC_EXIT_RC(x)                                                                     \
    {                                                                                           \
        HAL_Printf("FUNC_EXIT:   %s L#%d Return Code : %ld \n", __FUNCTION__, __LINE__, (long)(x)); \
        return x;                                                                               \
    }
#else
#define IOT_FUNC_ENTRY
#define IOT_FUNC_EXIT \
    {                 \
        return;       \
    }
#define IOT_FUNC_EXIT_RC(x) \
    {                       \
        return x;           \
    }
#endif

/* Macro for internal debug */
//#define LOG_UPLOAD_DEBUG
#ifdef LOG_UPLOAD_DEBUG
#define UPLOAD_DBG(fmt, ...) HAL_Printf(">>LOG-DBG>>%s(%d): " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define UPLOAD_DBG(...)
#endif
#define UPLOAD_ERR(fmt, ...) HAL_Printf(">>LOG-ERR>>%s(%d): " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define STRING_PTR_PRINT_SANITY_CHECK(ptr) ((ptr) ? (ptr) : "null")

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_LOG_H_ */
