/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the pmic adc software driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12969f
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmic_irq.h>
#include <pinlist.h>
#include "adc_reg.h"
#include "../adc_phy.h"
#include <timer_inc.h>
#include <pmic_efuse.h>

bool adc_irq_enable = false;
u32 adc_irq_status = 0;
bool non_init = true;

#define GP202_ADC_CALI_BLK          0x0
#define GP202_ADC_LDO_TRIM_BLK      0x3
#define GP202_ADC_LDO_TRIM_SIGN     BIT(6)
#define GP202_ADC_LDO_TRIM_OFFSET(x) (((x) & 0x3f))
#define GP202_ADC_CALI_BIT          29
#define GP202_ADC_CALI_VALID_BITS   0x7
#define GP202_ADC_JOB_DONE_FLAG     0x1
#define GP202_ADC_JOB_DONE_TIMEOUT  1000
#define GP202_ADC_CALI_L7_GOLDEN    0x3c0
aos_event_ptr   adc_job_done_event = AOS_NULL;
#define GP202_ADC_CHAN_MAX          4
#define GP202_INTEL_VBAT_CHAN       8
#define GP202_ADC_LDO_STANDARD      1786
static int gp202_adc_cali_val = GP202_ADC_LDO_STANDARD;
static u32 adc_cali_efuse_val = 0;

void gp202_adc_irq_clr(void)
{
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_CLR, BIT_ADC_CTRL_GP202_ADC_INT_CLR);
}

void gp202_adc_irq_handler(void *val)
{
    aos_set_event(adc_job_done_event, GP202_ADC_JOB_DONE_FLAG, AOS_OR);
    gp202_adc_irq_clr();
}

void gp202_adc_irq_init(void)
{
    gp202_adc_irq_clr();
    register_pmic_irq(PMIC_IRQ_ADC, gp202_adc_irq_handler, (void *)NULL);
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_EN, BIT_ADC_CTRL_GP202_ADC_INT_EN);
}

int gp202_trig_adc_job_wait_done(void)
{
    u32 adc_temp, val;
    int ret = 0, i;

    gp202_adc_irq_clr();
    aos_get_event(adc_job_done_event, GP202_ADC_JOB_DONE_FLAG,
                  AOS_AND_CLEAR, &adc_temp, AOS_NO_WAIT);

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_TRIG, BIT_ADC_CTRL_GP202_ADC_TRIG);

    if (aos_is_user_thread_context()) {
        ret = aos_get_event(adc_job_done_event, GP202_ADC_JOB_DONE_FLAG,
                AOS_AND_CLEAR, &adc_temp, GP202_ADC_JOB_DONE_TIMEOUT);
    } else {
        i = 0;

        while( i < ADC_WAIT_TIME_CNT) {
            val = pmic_readl(GP202_ADC_CTRL_ADC_INT_STS_RAW) & BIT_ADC_CTRL_ADC_INT_STS_RAW;

            if (val)
                break;

            i++;
            aic_udelay(100);
        }

        if (i == ADC_WAIT_TIME_CNT) {
            aicx_warn(0x1a736d, "", "adc wait finish timeout!\n");
            ret = AIC_ETIME;
        }
    }

    return ret;
}

static int gp202_adc_ldo_trim_cali(void)
{
    int ret = 0;
    u32 val = 0;

    ret = aic_pmic_efuse_block_read(GP202_ADC_LDO_TRIM_BLK, &val);
    if (ret) {
        aicx_warn(0x1508ca, "s", "%s: read efuse failed!\n", __func__);
        goto out;
    }

    if (val & GP202_ADC_LDO_TRIM_SIGN)
        gp202_adc_cali_val = GP202_ADC_LDO_STANDARD + GP202_ADC_LDO_TRIM_OFFSET(val);
    else
        gp202_adc_cali_val = GP202_ADC_LDO_STANDARD - GP202_ADC_LDO_TRIM_OFFSET(val);

out:
    return ret;
}

int gp202_adc_init(void)
{
    int ret;

    adc_job_done_event = aos_create_event("adc_job_done_event");
    AIC_ASSERT(adc_job_done_event);

    gp202_adc_irq_init();

    ret = aic_pmic_efuse_block_read(GP202_ADC_CALI_BLK, &adc_cali_efuse_val);
    if (ret)
        goto out;

    ret = gp202_adc_ldo_trim_cali();
out:
    return 0;
}

int gp202_adc_chan_data_get(u32 chan, u32 *data)
{
    int ret = 0;
    u32 val;

    if (!adc_job_done_event)
        gp202_adc_init();

    if (!(chan <= GP202_ADC_CHAN_MAX || chan == GP202_INTEL_VBAT_CHAN)) {
        ret = AIC_EIO;
        goto out;
    }

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_AIN_SEL, BIT_ADC_CTRL_GP202_ADC_AIN_SEL(chan));

    gp202_trig_adc_job_wait_done();

    val = pmic_readl(GP202_ADC_CTRL_ADC_DATA_RPT);
    if (val & BIT_ADC_CTRL_DBG_ADC_DATA_ERR) {
        ret = AIC_EIO;
        aicx_warn(0x063209, "s", "%s: adc data err.\n", __func__);
        goto out;
    }

    *data = BIT_ADC_CTRL_ADC_DATA_RPT(val);

out:
    return ret;
}

int gp202_adc_calibrate(void)
{
    int ret = 0, data_rpt = 0;
    u32 reg = 0, val = 0;

    if (!adc_job_done_event)
        gp202_adc_init();

    val = ((adc_cali_efuse_val >> GP202_ADC_CALI_BIT) & GP202_ADC_CALI_VALID_BITS);
    if (!val) {
        ret = AIC_ENODEV;
        aicx_warn(0x03f009, "s", "%s: efuse cali val is NULL!\n", __func__);
        goto out;
    }

    reg = pmic_readl(GP202_ADC_CTRL_GP202_ADC_LDO_CFG);
    reg &= ~BIT_ADC_CTRL_GP202_ADC_LDO_TRIM(0x7);
    reg |= BIT_ADC_CTRL_GP202_ADC_LDO_TRIM(val);
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_LDO_CFG, reg);

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_EN, BIT_ADC_CTRL_GP202_ADC_INT_EN);
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_AIN_SEL, 0xF);
    /*calibrate level 7, golden value 0x3C0*/
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_CAL_CFG,
                BIT_ADC_CTRL_GP202_ADC_CAL(0x7) | BIT_ADC_CTRL_GP202_ADC_CALIBRA_EN);

    ret = gp202_trig_adc_job_wait_done();
    if (ret) {
        aicx_warn(0x12a6b9, "s", "%s: wait adc job done timeout!\n", __func__);
        goto out;
    }

    data_rpt = 0;
    data_rpt = pmic_readl(GP202_ADC_CTRL_ADC_DATA_RPT);
    if (data_rpt & BIT_ADC_CTRL_DBG_ADC_DATA_ERR) {
        ret = AIC_EIO;
        aicx_warn(0x063209, "s", "%s: adc data err.\n", __func__);
        goto out;
    }

    val = 0;
    data_rpt  = BIT_ADC_CTRL_ADC_DATA_RPT(data_rpt);
    if (data_rpt < GP202_ADC_CALI_L7_GOLDEN)
        val |= BIT_ADC_CTRL_GP202_ADC_OFFSET_MODE;

    data_rpt = abs(data_rpt - GP202_ADC_CALI_L7_GOLDEN);
    if (data_rpt > BIT_ADC_CTRL_GP202_ADC_OFFSET_VAL(0x3F)) {
        val |= BIT_ADC_CTRL_GP202_ADC_OFFSET_VAL(0x3F);
        aicx_warn(0x17d52a, "s", "%s: adc cali date out of range, use max offset.\n", __func__);
    } else {
        val |= data_rpt;
    }

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_CAL_CFG, 0x0);
    gp202_adc_irq_clr();

out:
    return ret;
}

u32 gp202_adc2voltage(u32 adc_val)
{
    return (adc_val * gp202_adc_cali_val) / 2047;
}

void gp202_adc_enable(bool enable)
{
    if (enable) {
        pmic_writel(GP202_CKM_EN_GLB_CLK_PD_ADC, BIT_CKM_EN_GLB_CLK_PD_ADC);
        pmic_writel(GP202_ADC_CTRL_ADC_EN_FORCE, BIT_ADC_CTRL_ADC_EN_FORCE_VAL);
    }
}

bool gp202_adc_is_enabled(void)
{
    u32 reg;

    reg = pmic_readl(GP202_ADC_CTRL_ADC_EN_FORCE);
    if (reg == BIT_ADC_CTRL_ADC_EN_FORCE_EB)
        return FALSE;
    else
        return TRUE;
}

void gp202_vcc5v_test_mode(bool enabled){}
void gp202_vcc5v_source_sel(bool is_vbat){}

struct adc_phy_ops gp202_adc_phy_ops = {
    .adc_get = gp202_adc_chan_data_get,
    .adc_cali = gp202_adc_calibrate,
    .adc2voltage = gp202_adc2voltage,
    .adc_enable = gp202_adc_enable,
    .adc_is_enabled = gp202_adc_is_enabled,
    .vbat_adc_chan = GP202_INTEL_VBAT_CHAN,
    .vbat_vbus_mode = gp202_vcc5v_test_mode,
    .vbat_vbus_sel = gp202_vcc5v_source_sel,
};

int adc_phy_ops_init(void)
{
    int ret = 0;

    ret = adc_phy_ops_register(&gp202_adc_phy_ops);

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(adc_phy_ops_init);
