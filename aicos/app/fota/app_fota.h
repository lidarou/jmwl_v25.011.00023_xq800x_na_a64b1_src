/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) of file
 *    browser(FB) app.  All app prototypes and data structure
 *    definitions are defined in this file.
 */

#ifndef __APP_FOTA_H__
#define __APP_FOTA_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>
#ifndef WIN32
#include <aic_osa_ex.h>
#endif

/*******************************************
 *              DEFINES
 ******************************************/
#ifndef FOTA_APP_NAME
#define FOTA_APP_NAME "app_fota"
#endif

#ifdef CONFIG_FOTA_APP_DEBUG_MEM /*debug only*/
extern void *fota_app_malloc_ex(size_t x, const char *name, uint32_t line);
extern void fota_app_free_ex(void *x, const char *name, uint32_t line);
#define fota_app_malloc(x) fota_app_malloc_ex(x, __FILE__, __LINE__)
#define fota_app_free(x) fota_app_free_ex(x, __FILE__, __LINE__)
#else
#define fota_app_malloc(x) lv_mem_alloc((size_t)(x))
#define fota_app_free(x) lv_mem_free((void *)(x))
#endif

#ifdef WIN32
#define app_logi printf
#define app_logd printf
#define app_loge printf
#endif
/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * aic_fota_app_entry - Entry of app fota
 *
 * Return: the page of current page
 */
lv_obj_t *aic_fota_app_entry(void);

/**
 * aic_fota_app_exit - Exit of app fota
 *
 * Return: NA
 */
void aic_fota_app_exit(void);

/**
 * fota_app_service_create - create fota app service
 * @notify: [IN] notify callback
 *
 * Return: NA
 * Note: used in ui.c
 */
void *fota_app_service_create(void (*notify)(void *srv, int evt_id,
                                             void *param));

/**
 * fota_srv_destory - destory fota app service
 * @gracefully: [IN] gracefully destory
 *
 * Return: NA
 * Note: used in ui.c
 */
void fota_srv_destory(bool gracefully);

/**
 * register_app_fota - register fota app into app manager
 *
 * Return: NA
 */
void register_app_fota(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __APP_FOTA_H__ */
