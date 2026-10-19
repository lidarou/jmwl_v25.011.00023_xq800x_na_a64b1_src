/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines all the I2C APIs exported for other module.
 */

#ifndef __INCLUDE_PMIC_IO_H__
#define __INCLUDE_PMIC_IO_H__

#include <aic_io.h>

int pmic_writeb(u16 addr, u8 value);
int pmic_writel(u16 addr, u32 val);
u8 pmic_readb(u16 addr);
u32 pmic_readl(u16 addr);
int pmic_burst_write(u8 *buf, int num);
int pmic_burst_read(u16 addr, u8 *buf, int num);
void pmic_writeb_async(u16  addr, u8 value);
void pmic_writel_async(u16 addr, u32 val);
int pmic_polling_async_status(void);

#endif /* __INCLUDE_PMIC_IO_H__ */
