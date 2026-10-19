/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_log.h
 *
 */

#ifndef __APP_LOG_H__
#define __APP_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif
/*******************
 *      INCLUDES
 *******************/
#include <aic_osa_ex.h>
#include <lvgl.h>
#include <string.h>
/*********************
 *      DEFINES
 *********************/
#ifdef WIN32
#define APP_LOGI(fmt,...) do {\
LV_LOG_USER(fmt,##__VA_ARGS__);\
} while (0)
#else
#define APP_LOGI(fmt,...) do {\
app_logi(fmt,##__VA_ARGS__);\
} while (0)
#endif

#ifdef WIN32
#define APP_LOGE(fmt,...) do {\
LV_LOG_ERROR(fmt,##__VA_ARGS__);\
} while (0)
#else
#define APP_LOGE(fmt,...) do {\
app_loge(fmt,##__VA_ARGS__);\
} while (0)
#endif

#ifdef WIN32
#define APP_LOGW(fmt,...) do {\
LV_LOG_WARN(fmt,##__VA_ARGS__);\
} while (0)
#else
#define APP_LOGW(fmt,...) do {\
app_logw(fmt,##__VA_ARGS__);\
} while (0)
#endif

#ifdef WIN32
#define APP_LOGD(fmt,...) do {\
LV_LOG_TRACE(fmt,##__VA_ARGS__);\
} while (0)
#else
#define APP_LOGD(fmt,...) do {\
app_logd(fmt,##__VA_ARGS__);\
} while (0)
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __APP_LOG_H__ */