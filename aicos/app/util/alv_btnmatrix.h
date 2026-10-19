/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file defines the basic Application Interface (API) of a customized
 * button matrix widget(alv_btnmatrix).
 *   alv_btnmatrix inherits from lv_btnmatrix. The difference is
 *   a. Does not draw any text or bg rectangles
 */

#ifndef __ALV_BTNMATRIX_H__
#define __ALV_BTNMATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>

/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * struct alv_btnmatrix - structure of alv btnmatrix widget
 * @base: base of the widget, i.e. lv_btnmatrix_t
 */
struct alv_btnmatrix {
    lv_btnmatrix_t base;
};

/*******************************************
 *       GLOBAL VARIABLE DECLARATION
 ******************************************/
extern const lv_obj_class_t alv_btnmatrix_class;

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * alv_btnmatrix_create - Creater of alv_btnmatrix
 * @parent: [IN] parent widget
 *
 * Return: the created button, NULL for failure
 */
lv_obj_t *alv_btnmatrix_create(lv_obj_t *parent);

uint16_t alv_btnmatrix_get_btn_cnt(lv_obj_t *obj);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __ALV_BTNMATRIX_H__ */
