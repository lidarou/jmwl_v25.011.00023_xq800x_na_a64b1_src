/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of pmic ipu module.
 *   It includes all the pmic ipu phy common function api.
*/

#ifndef __PMIC_IPU_H__
#define __PMIC_IPU_H__

u32 PMIC_IPU_GetRawStatus(void);
u32 PMIC_IPU_GetMaskStatus(void);
u32 PMIC_IPU_GetEnStatus(void);
u32 PMIC_IPU_GetPol(void);
void PMIC_IPU_IRQDisAll(void);
void PMIC_IPU_IRQEnable(u32 irq_num);
void PMIC_IPU_IRQDisable(u32 irq_num);
void PMIC_IPU_IRQSoftEnable(u32 irq_num);
void PMIC_IPU_IRQSetSoftSrc(u32 irq_num);
void PMIC_IPU_IRQSetPol(u32 irq_num, bool pol);
void PMIC_IPU_IRQClrSoftSrc(u32 irq_num);
void PMIC_IPU_IRQSoftDisable(u32 irq_num);

#endif /* __PMIC_IPU_H__ */