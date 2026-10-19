/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aic cmd head file.
 *
 */

#ifndef __AIC_CMD_H__
#define __AIC_CMD_H__

#ifdef CONFIG_PLAT_CONSOLE

#include <aic_type.h>
#include <aic_cmd_api.h>

int aic_cons_init(void);
scall_func aic_cons_get_func(char *cmd, int size);
int aic_cmd_proc(char *cmd, size_t length);
int aic_software_version(void);

#endif /* CONFIG_PLAT_CONSOLE */

#endif /* __AIC_CMD_H__ */

