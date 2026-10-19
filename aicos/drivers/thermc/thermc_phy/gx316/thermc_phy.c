/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the thermal software driver, it is therma r1p0 phy driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x174722
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include "../../therm_internal.h"
#include "thermc_reg.h"
#include <irq_chip.h>

u32 aic_thermc_irq_status = 0;
u32 cali_factor_val = 0;
u32 thermc_sig_det_temp = 0;
u32 temp_irq_status = 0;

u16 aic_thermc_stat_data_get(void)
{
    u32 val;
    u16 adc_stat_val;

    val = AIC_READ32(REG_THERM_THERM_STAT_RPT);
    adc_stat_val = BIT_THERM_THERM_STAT_DATA(val);

    return adc_stat_val;
}

void aic_calib_env_temp_set(u32 env_temp)
{
    AIC_WRITE32(REG_THERM_THERM_CALIB_TEMP, BIT_THERM_THERM_CALIB_TEMP(env_temp));
}

void aic_thermc_adc_sample_cycle(u32 sample_rate_us)
{
    u16 adc_sample_rate_cnt;

    adc_sample_rate_cnt = sample_rate_us * THERMC_ADC_CLK / 1000;

    //AIC_WRITE32(REG_THERM_THERM_STAT_CFG, BIT_THERM_THERM_STAT_CNT(adc_sample_rate_cnt));
    AIC_WRITE32(REG_THERM_THERM_STAT_CFG, 0XBB8);
}

void aic_thermc_irq_enabled(AIC_THERMC_IRQ_MSK thermc_irq_msk, bool enabled)
{
    u32 val;

    val = BIT_THERM_THERM_INT_EN(AIC_READ32(REG_THERM_THERM_INT_EN));

    if (enabled) {
        if (thermc_irq_msk == AIC_THERMC_IRQ_DET_DONE)
            thermc_sig_det_temp = 0;
        else if (thermc_irq_msk == AIC_THERMC_IRQ_CALIB_DONE)
            cali_factor_val = 0;

        AIC_WRITE32(REG_THERM_THERM_INT_EN, val | BIT_THERM_THERM_INT_EN(thermc_irq_msk));
    } else {
        AIC_WRITE32(REG_THERM_THERM_INT_EN, val & BIT_THERM_THERM_INT_EN(~ thermc_irq_msk));
    }
}

void aic_thermc_irq_clear(AIC_THERMC_IRQ_MSK thermc_irq_msk)
{
    u32 val;

    val = AIC_READ32(REG_THERM_THERM_INT_CLR);
    val |= BIT_THERM_THERM_INT_CLR(thermc_irq_msk);

    AIC_WRITE32(REG_THERM_THERM_INT_CLR, val);
}

u32 aic_thermc_irq_status_get(void)
{
#ifdef THERMC_SOC_IRQ
    return aic_thermc_irq_status;
#else
    return AIC_READ32(REG_THERM_THERM_INT_STA);
#endif
}

u16 aic_thermc_cali_factor_get(void)
{
    u32 val;

    val = AIC_READ32(REG_THERM_THERM_CALIB_RPT);

    return BIT_THERM_THERM_CALIB_FACTOR(val);
}

u16 aic_thermc_cali_factor_get_via_int(void)
{
    u32 irq_status, val = 0;
#if 0
    irq_status = aic_thermc_irq_status_get();
    if (BIT_THERM_THERM_INT_STA(irq_status) & AIC_THERMC_IRQ_CALIB_DONE)
#ifdef THERMC_SOC_IRQ
        val = cali_factor_val;
#else
        val = aic_thermc_cali_factor_get();
#endif
#else //for test
    irq_status = aic_thermc_irq_status_get();
    val = aic_thermc_cali_factor_get();
#endif
    return val;
}

u16 aic_thermc_sig_det_temp_get(void)
{
    u32 val;

    val = AIC_READ32(REG_THERM_THERM_DETECT_RPT);

    return BIT_THERM_THERM_DETECT_TEMP(val);
}

u16 aic_thermc_sig_det_temp_get_via_int(void)
{
    u32 irq_status, val = 0;

    irq_status = aic_thermc_irq_status_get();
    if (BIT_THERM_THERM_INT_STA(irq_status) & AIC_THERMC_IRQ_DET_DONE)
#ifdef THERMC_SOC_IRQ
        val = thermc_sig_det_temp;
#else
        val = aic_thermc_sig_det_temp_get();
#endif

    return val;
}

static int aic_thermc_irq_handler(void *param)
{
    u32 reg, irq_clr_msk;
    u32 temp;
    reg =  AIC_READ32(REG_THERM_THERM_INT_RAW);

#ifdef THERMC_DEBUG
    aicx_warn(0x134646, "xx", "BIT_THERM_THERM_INT_RAW:0x%x BIT_THERM_THERM_INT_STA:0x%x\n",
        BIT_THERM_THERM_INT_RAW(reg), BIT_THERM_THERM_INT_STA(reg));
#endif

    aic_thermc_irq_status = reg;
    irq_clr_msk = BIT_THERM_THERM_INT_STA(reg);
    if (irq_clr_msk & AIC_THERMC_IRQ_CALIB_DONE) {
        cali_factor_val =  aic_thermc_cali_factor_get();
        aicx_warn(0x0553e6, "x", "aic_thermc_irq_handler cali_factor_val:0x%x\n", cali_factor_val);
    }

    if (irq_clr_msk & AIC_THERMC_IRQ_DET_DONE) {
        thermc_sig_det_temp =  aic_thermc_sig_det_temp_get();
        temp = (thermc_sig_det_temp/2) - 273;
        aicx_warn(0x1d9370, "d", "aic_thermc_irq_handler thermc_sig_det_temp:%d\n", temp);
        //alog_warn("aic_thermc_irq_handler thermc_sig_det_temp:0x%x\n", thermc_sig_det_temp);
    }

    if (irq_clr_msk & AIC_THERMC_IRQ_AUTO_DET_ALRM0) {
        aicx_warn(0x030632, "", "thermc alrm0\n");
        temp_irq_status |= AIC_THERMC_IRQ_AUTO_DET_ALRM0;
    }

    if (irq_clr_msk & AIC_THERMC_IRQ_AUTO_DET_ALRM1) {
        aicx_warn(0x032a7a, "", "thermc alrm1\n");
        temp_irq_status |= AIC_THERMC_IRQ_AUTO_DET_ALRM1;
    }

    if (irq_clr_msk & AIC_THERMC_IRQ_AUTO_DET_ALRM2) {
        aicx_warn(0x014849, "", "thermc alrm2\n");
        temp_irq_status |= AIC_THERMC_IRQ_AUTO_DET_ALRM2;
    }

    reg = AIC_READ32(REG_THERM_THERM_INT_CLR);
    reg |= BIT_THERM_THERM_INT_CLR(irq_clr_msk);
    AIC_WRITE32(REG_THERM_THERM_INT_CLR, reg);

    return IRQ_ISR_ONLY;
}

#ifdef CONFIG_SUBSYS_AP
void aic_thermc_irq_init(void)
{
    aos_request_irq(IRQ_THM, aic_thermc_irq_handler, (void *)NULL);
}
#endif

void aic_thermc_cali_factor_set(u16 cali_factor_from_nv)
{
    u32 val;

    val = BIT_THERM_THERM_DETECT_FACTOR(cali_factor_from_nv);
    AIC_WRITE32(REG_THERM_THERM_DETECT_FACTOR, val);
}

u16 aic_thermc_auto_det_temp_get(void)
{
    u32 val;

    val = AIC_READ32(REG_THERM_THERM_AUTO_DETECT_RPT);
    return BIT_THERM_THERM_AUTO_DETECT_TEMP(val);
}

void aic_thermc_auto_det_cycle_set(u32 auto_det_cycle_us)
{
    u32 val;

    val = auto_det_cycle_us * THERMC_ADC_CLK / 1000;
    //AIC_WRITE32(REG_THERM_THERM_AUTO_DETECT_CFG, BIT_THERM_THERM_AUTO_DETECT_GAP(val));
    AIC_WRITE32(REG_THERM_THERM_AUTO_DETECT_CFG, 0x1000);
}

void aic_thermc_heat_thres_set(u16 irq_val, u16 reset_val)
{
    AIC_WRITE32(REG_THERM_THERM_HIGH_TEMP_ALARM0, BIT_THERM_THERM_ALARM0_THRES(irq_val));
    AIC_WRITE32(REG_THERM_THERM_HIGH_TEMP_ALARM1, BIT_THERM_THERM_ALARM1_THRES(reset_val));
}

void aic_thermc_cold_thres_set(u16 cold_thres_val)
{
    u32 val;

    val = BIT_THERM_THERM_ALARM2_THRES(cold_thres_val);

    AIC_WRITE32(REG_THERM_THERM_LOW_TEMP_ALARM, val);
}

void aic_thermc_work_mode_select(AIC_THERMC_WORK_MODE mode)
{
    AIC_WRITE32(REG_THERM_THERM_WORK_MODE, mode);
}

int aic_thermc_module_enable(void)
{
    AIC_WRITE32(REG_TOP_GLB_THMC_EB, BIT_TOP_GLB_THMC_EB);
    return 0;
}
