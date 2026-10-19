/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file comp_camera_preview.h
 *
 */

#ifndef __COMP_CAMERA_PREVIEW_H__
#define __COMP_CAMERA_PREVIEW_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
* comp_camera_preview_create - 创建 Camera preview 控件实例，当前只支持同时创建一个
* @parent: [IN] preview 控件的父对象指针.
* @src_width: [IN] preview 显示的宽度.
* @src_hight: [IN] preview 显示的高度.
* @bytes_per_pixel: [IN] 每个像素占用的Bytes空间.
* Return: 返回创建好的 preview 对象指针.
*/
lv_obj_t *comp_camera_preview_create(lv_obj_t *parent, u32 src_width, u32 src_hight, u32 bytes_per_pixel);

/**
* comp_camera_preview_destroy - 销毁 Camera preview 控件实例
* @obj: [IN] 要销毁的 preview 对象指针.
* Return: 无.
*/
void comp_camera_preview_destroy(lv_obj_t *obj);

/**
* comp_camera_preview_pause - 暂停 Camera preview 控件刷新
* Return: 无.
*/
void comp_camera_preview_pause(void);

/**
* comp_camera_preview_resume - 恢复 Camera preview 控件刷新
* Return: 无.
*/
void comp_camera_preview_resume(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __COMP_CAMERA_PREVIEW_H__ */


