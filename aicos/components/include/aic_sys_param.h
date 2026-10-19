/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aic system parameter Interface.
 */

#ifndef __AIC_SYS_PARAM_H__
#define __AIC_SYS_PARAM_H__

#include <aic_type.h>

bool aic_sys_is_32kless(void);
int aic_sys_get_crystal_type(void);
int aic_sys_get_board_id(void);
int aic_sys_get_boot_cnt(void);

#endif /* __AIC_SYS_PARAM_H__ */

