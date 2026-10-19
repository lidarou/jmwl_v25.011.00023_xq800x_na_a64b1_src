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
#define AIC_THIS_FILE    0x1477a1
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
#include <pmic_irq.h>
#include <pmic_2nd_irq.h>
#include "../pmic_2nd_phy.h"
#include <string.h>
#include <timer_inc.h>
#include <sysoff.h>

static void PMIC_PMU_REG_AND(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) & bit_value);
}

static void PMIC_PMU_REG_OR(u16 offset, u32 bit_value)
{
    pmic_writel(offset, pmic_readl(offset) | bit_value);
}

/*gp202 pmu ipu phy*/
#define GP202_REG_PMU_UPS_INT_EN    0x438
#define GP202_REG_PMU_UPS_INT_CLR   0x43c
#define GP202_REG_PMU_UPS_INT_RAW   0x440
#define GP202_REG_PMU_UPS_INT_STA   0x444
#define GP202_REG_PMU_UPS_INT_MSK   0x448

#define GP202_REG_PMU_PD_INT_MSK    0x80b0
#define GP202_REG_PMU_PD_INT_EN     0x80b4
#define GP202_REG_PMU_PD_INT_CLR    0x80b8
#define GP202_REG_PMU_PD_INT_RAW    0x80bc
#define GP202_REG_PMU_PD_INT_STA    0x80c0

static u32 GP202_PMU_UPS_GetMaskStatus(void)
{
    return pmic_readl(GP202_REG_PMU_UPS_INT_STA);
}

static u32 GP202_PMU_UPS_GetEnStatus(void)
{
    return pmic_readl(GP202_REG_PMU_UPS_INT_EN);
}

static void GP202_PMU_UPS_IRQEnable(u32 irq_num)
{
    PMIC_PMU_REG_OR(GP202_REG_PMU_UPS_INT_EN, BIT(irq_num));
}

static void GP202_PMU_UPS_IRQDisable(u32 irq_num)
{
    PMIC_PMU_REG_AND(GP202_REG_PMU_UPS_INT_EN, ~BIT(irq_num));
}

/*PMU PD INT OPERATE GP202(20 21 22 23 24 25 26 27)*/
static u32 GP202_PMU_PD_GetMaskStatus(void)
{
    return pmic_readl(GP202_REG_PMU_PD_INT_STA);
}

static u32 GP202_PMU_PD_GetEnStatus(void)
{
    return pmic_readl(GP202_REG_PMU_PD_INT_EN);
}

static void GP202_PMU_PD_IRQEnable(u32 irq_num)
{
    PMIC_PMU_REG_OR(GP202_REG_PMU_PD_INT_EN, BIT(irq_num));
}

static void GP202_PMU_PD_IRQDisable(u32 irq_num)
{
    PMIC_PMU_REG_AND(GP202_REG_PMU_PD_INT_EN, ~BIT(irq_num));
}

int gp202_pmu_prior_irq_num_get(int irq_num)
{
    int first_order_irq;

    if (irq_num <= PMIC_PMU_UPS_IIC_ACCESS) {
        first_order_irq = PMIC_IRQ_PMU_UPS_INT;
    } else if (irq_num <= PMIC_PMU_PD_ERROR_VBAT) {
        first_order_irq = AIC_ENXIO;/*gp202 bypass these irq numbers*/
    } else {
        first_order_irq = PMIC_IRQ_PMU_PD_INT;
    }

    return first_order_irq;
}

void gp202_pmu_irq_enable(int irq_num)
{
    int irq_2nd;

    if (irq_num <= PMIC_PMU_RTC_CHG_FALL) {
        irq_2nd = irq_num - PMIC_PMU_RTC_CHG_RISE + 4;//virtual irq num remap.
        GP202_PMU_UPS_IRQEnable(irq_2nd);
    } else if (irq_num <= PMIC_PMU_UPS_IIC_ACCESS) {
        irq_2nd = irq_num - PMIC_PMU_UPS_PWRKEY_PRESS;
        GP202_PMU_UPS_IRQEnable(irq_2nd);
    } else if (irq_num <= PMIC_PMU_PD_ERROR_VBAT) {
        /*gp202 bypass these irq numbers*/
    } else {
        irq_2nd = irq_num - PMIC_PMU_PD_OT_DET1;
        GP202_PMU_PD_IRQEnable(irq_2nd);
    }
}

void gp202_pmu_irq_disable(int irq_num)
{
    int irq_2nd;

    if (irq_num <= PMIC_PMU_RTC_CHG_FALL) {
        irq_2nd = irq_num - PMIC_PMU_RTC_CHG_RISE + 4;//virtual irq num remap.
        GP202_PMU_UPS_IRQDisable(irq_2nd);
    } else if (irq_num <= PMIC_PMU_UPS_IIC_ACCESS) {
        irq_2nd = irq_num - PMIC_PMU_UPS_PWRKEY_PRESS;
        GP202_PMU_UPS_IRQDisable(irq_2nd);
    } else if (irq_num <= PMIC_PMU_PD_ERROR_VBAT) {
        /*gp202 bypass these irq numbers*/
    } else {
        irq_2nd = irq_num - PMIC_PMU_PD_OT_DET1;
        GP202_PMU_PD_IRQDisable(irq_2nd);
    }
}

void gp202_pmu_irq_disable_all(void)
{
    PMIC_PMU_REG_AND(GP202_REG_PMU_UPS_INT_EN, 0);
    PMIC_PMU_REG_AND(GP202_REG_PMU_PD_INT_EN, 0);
}

void gp202_pmu_irq_clear_all(void)
{
    pmic_writel(GP202_REG_PMU_UPS_INT_CLR, 0xFF);
    pmic_writel(GP202_REG_PMU_PD_INT_CLR, 0xFF);
}

u32 gp202_pmu_get_en_status(int irq_num)
{
    u32 val;

    if (irq_num <= PMIC_PMU_UPS_IIC_ACCESS) {
        val = GP202_PMU_UPS_GetEnStatus();
    } else if (irq_num <= PMIC_PMU_PD_ERROR_VBAT) {
        /*gp202 bypass these irq numbers*/
        val = AIC_ENXIO;
    } else {
        val = GP202_PMU_PD_GetEnStatus();
    }

    return val;
}

int append_gp202_pmu_irq_msk(int irq_msk)
{
    int value;
    u32 msk_status, msk_ops;

    value = irq_msk;

    if (irq_msk & BIT(PMIC_IRQ_PMU_UPS_INT)) {
        msk_status = GP202_PMU_UPS_GetMaskStatus();
        msk_ops = msk_status & 0x3F;
        value |= (((msk_ops >> 4) | (msk_ops <<2)) & 0x3F) << PMIC_PMU_RTC_CHG_RISE;
        value &= ~BIT(PMIC_IRQ_PMU_UPS_INT);
        pmic_writel(GP202_REG_PMU_UPS_INT_CLR, msk_status);
    }

    if (irq_msk & BIT(PMIC_IRQ_PMU_PD_INT)) {
        msk_status = GP202_PMU_PD_GetMaskStatus();
        value |= msk_status << PMIC_PMU_PD_OT_DET1;
        value &= ~BIT(PMIC_IRQ_PMU_PD_INT);
        pmic_writel(GP202_REG_PMU_PD_INT_CLR, msk_status);
    }

    value &= ~ (0x3F << PMIC_PMU_PD_BUCK0_OC);
    aic_udelay(1000);

    return value;
}

struct pmic_2nd_irq_ops pmic_gp202_pmu_irq_ops = {
    .pmic_2nd_irq_enable = gp202_pmu_irq_enable,
    .pmic_2nd_irq_disable = gp202_pmu_irq_disable,
    .pmic_1st_irq_num_get = gp202_pmu_prior_irq_num_get,
    .pmic_2nd_irq_disable_all = gp202_pmu_irq_disable_all,
    .pmic_2nd_irq_clear_all = gp202_pmu_irq_clear_all,
    .pmic_2nd_irq_en_sts_get = gp202_pmu_get_en_status,
    .pmic_2nd_irq_msk_append = append_gp202_pmu_irq_msk,
};

static int pmic_pmu_irq_init()
{
    pmic_2nd_irq_ops_register(&pmic_gp202_pmu_irq_ops);

    return AIC_SUCCESS;
}
AIC_DRV_P1_INIT_EXPORT(pmic_pmu_irq_init);
