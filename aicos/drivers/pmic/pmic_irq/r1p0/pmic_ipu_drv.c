/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * pmic irq driver file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x03bdb3
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "pmic_ipu_reg.h"
#include "../pmic_ipu.h"
#include <string.h>

#define GP201_IPU_BASE 0x00000C00
#define GP202_IPU_BASE 0x00001000

static u32 ipu_base = GP201_IPU_BASE;

static void PMIC_IPU_REG_AND(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) & bit_value);
}

static void PMIC_IPU_REG_OR(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) | bit_value);
}

void PMIC_IPU_IRQDisAll(void)
{
    PMIC_IPU_REG_AND(ipu_base + REG_IPU_INT_REQ_ENABLE, 0x0);

    return;
}

u32 PMIC_IPU_GetRawStatus(void)
{
    return pmic_readl(ipu_base + REG_IPU_INT_REQ_RAW);
}

u32 PMIC_IPU_GetMaskStatus(void)
{
    return pmic_readl(ipu_base + REG_IPU_INT_REQ_MASK);
}

u32 PMIC_IPU_GetEnStatus(void)
{
    return pmic_readl(ipu_base + REG_IPU_INT_REQ_ENABLE);
}

void PMIC_IPU_IRQEnable(u32 irq_num)
{
    PMIC_IPU_REG_OR(ipu_base + REG_IPU_INT_REQ_ENABLE, BIT(irq_num));
}

void PMIC_IPU_IRQDisable(u32 irq_num)
{
    PMIC_IPU_REG_AND(ipu_base + REG_IPU_INT_REQ_ENABLE, ~BIT(irq_num));
}

void PMIC_IPU_IRQSoftEnable(u32 irq_num)
{
    PMIC_IPU_REG_OR(ipu_base + REG_IPU_INT_REQ_SOFT_SEL,BIT(irq_num));
}

void PMIC_IPU_IRQSoftDisable(u32 irq_num)
{
    PMIC_IPU_REG_AND(ipu_base + REG_IPU_INT_REQ_SOFT_SEL, ~BIT(irq_num));
}

void PMIC_IPU_IRQSetSoftSrc(u32 irq_num)
{
    PMIC_IPU_REG_OR(ipu_base + REG_IPU_INT_REQ_SOFT_SRC, BIT(irq_num));
}

void PMIC_IPU_IRQClrSoftSrc(u32 irq_num)
{
    PMIC_IPU_REG_AND(ipu_base + REG_IPU_INT_REQ_SOFT_SRC, ~BIT(irq_num));
}

void PMIC_IPU_IRQSetPol(u32 irq_num, bool pol)
{
    if ( pol == TRUE)
        PMIC_IPU_REG_OR(ipu_base + REG_IPU_INT_REQ_POL, BIT(irq_num));
    else if ( pol == FALSE)
        PMIC_IPU_REG_AND(ipu_base + REG_IPU_INT_REQ_POL, ~BIT(irq_num));
}

u32 PMIC_IPU_GetPol()
{
    return pmic_readl(ipu_base + REG_IPU_INT_REQ_POL);
}

int pmic_ipu_init(void)
{
    int pmic_id_main;

    pmic_id_main = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;
    if (pmic_id_main == 0x201)
        ipu_base = GP201_IPU_BASE;
    else if (pmic_id_main == 0x202)
        ipu_base = GP202_IPU_BASE;

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(pmic_ipu_init);
