/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA ext Interface. It extend the
 * assert apis, log apis and malloc apis. The new api don't
 * need AIC_THIS_FILE param, replace it with __FILE__.
 * But __FILE__ will takes more code space.
 */

#ifndef __AIC_OSA_EX_H__
#define __AIC_OSA_EX_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG_MODULE
#define LOG_MODULE  AMOD_APP
#endif

#include <aic_math.h>
#include <aic_osa.h>
#include <timer_inc.h>

/* redefine some api.*/
#define aos_msleep          aos_sleep

#define sprintf_s(str, size, fmt, ...)      snprintf(str, size, fmt, ##__VA_ARGS__)
#define vsprintf_s(str, size, fmt, ...)     vsnprintf(str, size, fmt, ##__VA_ARGS__)
#define strcasecmp(str1,str2)               strncasecmp(str1, str2, strlen(str2))

#ifdef CONFIG_RTOS_LITEOS_M
#include "securec.h"
#else
int strcpy_s(char *dst, size_t n, const char * src);
int strcat_s(char *dst, size_t n, const char * src);
int memset_s(void *buf, size_t bufsz, int ch, size_t count);
int memmove_s(void *dst, size_t dstsz, const void *src, size_t srcsz);
#endif

/* Local macro define */
#define AIC_MALLOC_POS  (filename_t)__FILE__, __LINE__

/* The assert api define */
/**
 * aic_assert(info) - When the conditon is not satisfied, will crash here,
 * and will record the assert file and assert line.
 *
 * @cond: A conditon expression.
 */
#define aic_assert      AIC_ASSERT_S

/**
 * aic_assert_info(cond, info) - When the conditon is not satisfied, will crash here,
 * and will record the assert file and assert line,
 * and record a assert info that sprintf by the format string.
 *
 * @cond: A conditon expression.
 * @info
 */
#define aic_assert_info     AIC_ASSERT_INFO_S

/* The memory api define */
/**
* aic_malloc - Allocate memory.
* @size: memory size to alloc.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
#define aic_malloc(size)\
    aos_malloc_debug((u32)(size), AOS_FALSE, AIC_MALLOC_POS)

/**
* aic_zalloc() - Allocate memory and init to zero.
* @size: memory size to alloc.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
#define aic_zalloc(size)\
    aos_malloc_debug((u32)(size), AOS_TRUE, AIC_MALLOC_POS)

/**
* aic_free() - Free the mem ptr that alloc by aic_malloc or aic_zalloc.
* @mem_ptr: memory to free
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
#define aic_free(mem_ptr)\
    aos_free_debug((u8 *)(mem_ptr), AIC_MALLOC_POS)

/**
* aic_dma_malloc - Allocate memory amd makesure the memory is cacheline-aligned,
* @size: memory size to alloc.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
#define aic_dma_malloc(size)\
    aos_dma_malloc_debug((u32)(size), AIC_MALLOC_POS)


/**
* aic_dma_free() - Free the mem ptr that alloc by aic_dma_malloc.
* @mem_ptr: memory to free
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
#define aic_dma_free(mem_ptr)\
    aos_dma_free_debug((u8 *)(mem_ptr), AIC_MALLOC_POS)

/* log api */
/**
* aic_logx - the output log apis.
* x: the loglevel of the log, d < i < w < e.
* if the log level is < default static log fiter leve,
* the api alog_xxx will not compile.
* The default static log fiter level is ALOG_LV_INFO.
*
*
* @mod: the module number of the log.
* @fmt: the format string of the log.
*/

/**
* app_logx - the app default output log apis.
* will use AMOD_APP pass to the log module param.
*
* @fmt: the format string of the log.
*/

/**
* drv_logx - the diver default output log apis.
* will use AMOD_DRV_COMM pass to the log module param.
*
* @fmt: the format string of the log.
*/

/* xxx_logd define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_DEBUG)
#define aic_logd(...)
#define app_logd(...)
#define drv_logd(...)
#else
#define aic_logd(mod, fmt, ...) do { \
        alog_output(mod, ALOG_LV_DEBUG, fmt, ##__VA_ARGS__);\
    } while (0)
#define app_logd(fmt, ...) do { \
        alog_output(AMOD_APP, ALOG_LV_DEBUG, fmt, ##__VA_ARGS__);\
    } while (0)
#define drv_logd(fmt, ...) do { \
        alog_output(AMOD_DRV_COMM, ALOG_LV_DEBUG, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* xxx_logi define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define aic_logi(...)
#define app_logi(...)
#define drv_logi(...)
#else
#define aic_logi(mod, fmt, ...) do { \
        alog_output(mod, ALOG_LV_INFO, fmt, ##__VA_ARGS__);\
    } while (0)
#define app_logi(fmt, ...) do { \
        alog_output(AMOD_APP, ALOG_LV_INFO, fmt, ##__VA_ARGS__);\
    } while (0)
#define drv_logi(fmt, ...) do { \
        alog_output(AMOD_DRV_COMM, ALOG_LV_INFO, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* xxx_logw define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_WARN)
#define aic_logw(...)
#define app_logw(...)
#define drv_logw(...)
#else
#define aic_logw(mod, fmt, ...) do { \
        alog_output(mod, ALOG_LV_WARN, fmt, ##__VA_ARGS__);\
    } while (0)
#define app_logw(fmt, ...) do { \
        alog_output(AMOD_APP, ALOG_LV_WARN, fmt, ##__VA_ARGS__);\
    } while (0)
#define drv_logw(fmt, ...) do { \
        alog_output(AMOD_DRV_COMM, ALOG_LV_WARN, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* xxx_logw define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define aic_loge(...)
#define app_loge(...)
#define drv_loge(...)
#else
#define aic_loge(mod, fmt, ...) do { \
        alog_output(mod, ALOG_LV_ERROR, fmt, ##__VA_ARGS__);\
    } while (0)
#define app_loge(fmt, ...) do { \
        alog_output(AMOD_APP, ALOG_LV_ERROR, fmt, ##__VA_ARGS__);\
    } while (0)
#define drv_loge(fmt, ...) do { \
        alog_output(AMOD_DRV_COMM, ALOG_LV_ERROR, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AIC_OSA_EX_H__ */

