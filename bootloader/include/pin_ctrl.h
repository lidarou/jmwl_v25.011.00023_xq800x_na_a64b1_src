/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   Header file of pin control.
*/
#ifndef __PIN_CTRL_H__
#define __PIN_CTRL_H__

/**
 * pin_func_config() - Change pin function
 *
 * @pin_reg: pin register address
 * @func: pin function, only support 0 ~ 6
 */
void pin_func_config(u32 pin_reg, u32 func);

/**
 * pctl_sel_flshcs1() - Select pin REG_IOMUX_SDMMC_CMD function to 1
 *
 * For gx316, the default function of REG_IOMUX_SDMMC_CMD is 0(SDMMC_CMD).
 * Change it to 1 (nor flash cs1).
 *
 * Return: AIC_SUCCESS or errno.
 */
s32 pctl_sel_flshcs1(void);

#endif /* __PIN_CTRL_H__ */
