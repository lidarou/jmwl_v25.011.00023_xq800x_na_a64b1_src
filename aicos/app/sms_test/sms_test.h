/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is entry of sms test
 */

#ifndef __SMS_TEST_H__
#define __SMS_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/**********************
 * GLOBAL PROTOTYPES
 **********************/
 /**
 * app_sms_test_register - register sms test app into app manager
 *
 * Return: NA
 */
void app_sms_test_register(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __SMS_TEST_H__ */