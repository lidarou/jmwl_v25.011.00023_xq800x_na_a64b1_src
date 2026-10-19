/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of eipu module.
 *   It includes all the eipu common driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13ccad
#define LOG_MODULE    AMOD_DRV

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aon_glb.h>
#include <top_glb.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <ipu.h>
#include "ipu_reg.h"
#include <timer_inc.h>
#include <ipu_cfg.h>
#include <ipu_inc.h>

static void IPU_CLRBIT(u32 offset, u32 value, IPU_TYPE_E index)
{
    u32 base_addr = 0;
    u32 ipu_max = sizeof(ipu_bd_cfg) / sizeof(*ipu_bd_cfg);

    if (ipu_max == 1) {
        if ((index == IPU_MCU) || (index == IPU_CP))
            base_addr = ipu_bd_cfg[0].base;
    }
    else if (ipu_max == 2) {
        if ((index == IPU_MCU) || (index == IPU_CP))
            base_addr = ipu_bd_cfg[0].base;
        else
            base_addr = ipu_bd_cfg[1].base;
    }

    AIC_REG_CLRBIT(base_addr + offset, value);
}

static void IPU_SETBIT(u32 offset, u32 value, IPU_TYPE_E index)
{
    u32 base_addr = 0;
    u32 ipu_max = sizeof(ipu_bd_cfg) / sizeof(*ipu_bd_cfg);

    if (ipu_max == 1) {
        if ((index == IPU_MCU) || (index == IPU_CP))
            base_addr = ipu_bd_cfg[0].base;
    }
    else if (ipu_max == 2) {
        if ((index == IPU_MCU) || (index == IPU_CP))
            base_addr = ipu_bd_cfg[0].base;
        else
            base_addr = ipu_bd_cfg[1].base;
    }

    AIC_REG_SETBIT(base_addr + offset, value);
}

void IPU_IRQEnable(u32 value,IPU_TYPE_E index)
{
    IPU_SETBIT(INT_IRQ_EN, value ,index);
}

void IPU_IRQDisable(u32 value,IPU_TYPE_E index)
{
    IPU_CLRBIT(INT_IRQ_EN, value, index);
}

void IPU_IRQSetPol(u32 irq_num, bool pol, IPU_TYPE_E index)
{
    if ( pol == TRUE)
        IPU_SETBIT(INT_IRQ_POL, BIT(irq_num), index);
    else if ( pol == FALSE)
        IPU_CLRBIT(INT_IRQ_POL, BIT(irq_num), index);
}

u32 IPU_IRQGetPol(u32 irq_num)
{
    u32 base_addr = ipu_bd_cfg[0].base;

    return (AIC_READ32(base_addr + INT_IRQ_POL) & BIT(irq_num));
}

void IPU_IRQDisAll(IPU_TYPE_E index)
{
    IPU_CLRBIT(INT_IRQ_EN, 0xffffffff, index);

    return;
}

int IPU_Init(void)
{
    u32 i;
    u32 ipu_max = sizeof(ipu_bd_cfg) / sizeof(*ipu_bd_cfg);

    for (i = 0; i < ipu_max; i++) {
        if (ipu_bd_cfg[i].aon_eb != 0) {
            AIC_WRITE32(ipu_bd_cfg[i].aon_eb, 1);
            aic_udelay(100);
        }

        IPU_IRQEnable(ipu_bd_cfg[i].eb_bits, ipu_bd_cfg[i].type);
    }

    return AIC_SUCCESS;
}

AIC_DRV_P1_INIT_EXPORT(IPU_Init);
