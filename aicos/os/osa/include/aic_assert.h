/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the system assert Interface.
 */

#ifndef __AIC_ASSERT_H__
#define __AIC_ASSERT_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>
#include <aic_system_assert.h>
#include <aic_module.h>
#include <alog_api.h>
#include <aic_assert_def.h>

void aic_sys_assert_init(void);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AIC_ASSERT_H__ */

