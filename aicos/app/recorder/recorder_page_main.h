/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare the main page create API.
 */


#ifndef __RECORDER_PAGE_MAIN_H__
#define __RECORDER_PAGE_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder_conf.h"
#include "recorder_srv.h"

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
lv_obj_t *app_recorder_main_create(lv_obj_t *parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*__RECORDER_PAGE_MAIN_H__*/
