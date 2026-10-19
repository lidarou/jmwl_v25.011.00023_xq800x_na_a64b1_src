/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines boot mode related infomations.
 */

#ifndef __BOOT_MODE_H__
#define __BOOT_MODE_H__

#include <aic_type.h>
#include "platform/platform.h"

#ifdef CONFIG_BOOT_MODE
void bl_reset(void);
void bl_shutdown(void);
void bl_reset_basic(void);
enum BOOT_MOD get_boot_mode(void);
BOOL is_ap_crash_mode(void);
#else
static inline void bl_reset(void){}
static inline void bl_shutdown(void){}
static inline enum BOOT_MOD get_boot_mode(void){return BOOT_MOD_MAX;}
static inline BOOL is_ap_crash_mode(void){return AIC_FALSE;}
#endif

#endif /* __BOOT_MODE_H__ */
