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
#define AIC_THIS_FILE    0x010e81
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

static int aic_adc_channel_select(int channel)
{
    if (channel > ADC_CHANNEL_MAX || channel < 0)
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_ADC_SAMP_CH, BIT_ADC_CTRL_ADC1_AIN_CH_SEL(channel));

    return 0;
}

static void aic_adc_sample_enable(void)
{
    pmic_writel(REG_ADC_CTRL_ADC_SAMP_EN, BIT_ADC_CTRL_ADC1_AIN_SAMP_EN);
}

static void aic_adc_sample_trigger(void)
{
    pmic_writel(REG_ADC_CTRL_INT_CLR, BIT_ADC_CTRL_ADC_INTR_CLR);
    pmic_writel(REG_ADC_CTRL_ADC_TRIG, BIT_ADC_CTRL_ADC1_MANUAL_TRIG);
}

void aic_adc_int_enable(void)
{
    pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) | BIT_ADC_CTRL_ADC_INTR_EN);
}

void aic_adc_int_disable(void)
{
    pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) & ~BIT_ADC_CTRL_ADC_INTR_EN);
}

static void aic_adc_int_status_clear(void)
{
    pmic_writel(REG_ADC_CTRL_INT_CLR, BIT_ADC_CTRL_ADC_INTR_CLR);
}

static int aic_adc_data_get(u32 *data)
{
    int i, ret;
    u32 val;

    ret = 0;
    i = 0;

    while( i < ADC_WAIT_TIME_CNT) {
        i ++;
        if (adc_irq_enable)
            val = adc_irq_status & BIT_ADC_CTRL_ADC_INTR_STS;
        else
            val = pmic_readl(REG_ADC_CTRL_ADC_INT_STS) & BIT_ADC_CTRL_ADC_INTR_STS;

        if (val)
            break;

        aic_udelay(100);
    }

    if (adc_irq_enable) {
        adc_irq_status = 0;
    }

    aic_adc_int_status_clear();

    if(i == ADC_WAIT_TIME_CNT) {
        ret = AIC_ENOTSUP;
#ifdef ADC_DEBUG
        aicx_warn(0x1df7cf, "xxxx", "int_en:0x%x int_status:0x%x int_status_raw:0x%x ipu_status:0x%x\n",
        pmic_readl(REG_ADC_CTRL_INT_EN), pmic_readl(REG_ADC_CTRL_ADC_INT_STS),
        pmic_readl(REG_ADC_CTRL_ADC_INT_STS_RAW), pmic_readl(0xc00));
#endif
    } else {
        *data = pmic_readl(REG_ADC_CTRL_ADC_DATA);
    }

    return ret;
}

void aic_adc_reset(void)
{
    u32 data;

    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, BIT_GLB_PD_SW_RST_GLB_CLK_APB_ADC);
    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, 0);

    data = pmic_readl(REG_ADC_CTRL_GLB_CTRL_ADC);
    data &= BIT_ADC_CTRL_SOC_HIGN_ADC_WAIT_NUM(0xF);
    data |= BIT_ADC_CTRL_SOC_HIGN_ADC_WAIT_NUM(0x4);
    pmic_writel(REG_ADC_CTRL_GLB_CTRL_ADC, data);

    aic_adc_int_enable();
}

void aic_adc_enable(bool enable)
{
    if (enable) {
        /*enable adc*/
        pmic_writel(0x00008018,0x00000001);
        pmic_writel(0x00008020,0x00000001);
        pmic_writel(0x00000670,0x00000000);  //#Force turn on ADC
        pmic_writel(0x00000808,0x00000001);  //#turn on INTLDO2, enable ADC VREFP_HQ
    } else {
        /*disable adc*/
        pmic_writel(0x00008018,0x00000000);
        pmic_writel(0x00008020,0x00000000);
        pmic_writel(0x00000670,0x00000002);  //#Force turn off ADC
        pmic_writel(0x00000808,0x00000000);  //#turn off INTLDO2, disable ADC VREFP_HQ
    }
}

bool aic_adc_is_enabled(void)
{
    return pmic_readl(0x00008018);
}

int aic_adc_chan_data_get(u32 chan, u32 *data)
{
    int ret;

    if (non_init) {
        non_init = false;
        aic_adc_reset();
    }

    ret = aic_adc_channel_select(chan);
    if (ret) {
        aicx_warn(0x0b5702, "d", "unsupport adc channel:%d\n", chan);
        goto out;
    }

    aic_adc_sample_enable();

    aic_adc_sample_trigger();

    ret = aic_adc_data_get(data);
    if (ret < 0)
        aicx_warn(0x0ae47a, "", "wait adc finish timeout\n");

out:
    return ret;
}

void aic_adc_irq_handler(void *val)
{
    adc_irq_status = pmic_readl(REG_ADC_CTRL_ADC_INT_STS);

#ifdef ADC_DEBUG
    aicx_warn(0x1abdcd, "x", "adc irq_status:0x%x\n", adc_irq_status);
#endif

    aic_adc_int_status_clear();
}

void aic_adc_irq_init(void)
{

#ifdef ADC_DEBUG
    aicx_warn(0x0692b4, "sd", "%s %d\n", __func__, __LINE__);
#endif

    adc_irq_enable = true;
    register_pmic_irq(PMIC_IRQ_ADC, aic_adc_irq_handler, (void *)NULL);
    aic_adc_int_enable();
}

void aic_adc_irq_deinit(void)
{
    aic_adc_int_disable();
    unregister_pmic_irq(PMIC_IRQ_ADC);
}

int aic_adc_calibrate(void)
{
    int val, ret, offset;
    u32 adc_data;

    /*1. config calibrate value reg: REG_OFFSET_ADC1_18[5:0] = 0*/
    val = pmic_readl(REG_ADC_CTRL_ADC_OFFSET);
    val &= ~ 0x3F;
    pmic_writel(REG_ADC_CTRL_ADC_OFFSET, val);

    /*2. config calibrate enable reg: REG_DISHADC1_18 = 1*/
    pmic_writel(REG_ADC_CTRL_ADC_DISH, BIT_ADC_CTRL_REG_DISHADC1_18);

    /*3. normal hw adc mode, trigger adc sample get 12bit adc value B_ADC1_18[11:0]*/
    ret = aic_adc_chan_data_get(0, &adc_data);
    if (ret)
        return ret;

    /*4. New_offset = 2048 - B_ADC1_18[11:0]*/
    offset = 2048 - adc_data;

    /*5. if New_offset < 0, VAL = New_offset & 0x1F | BIT(5), else VAL = New_offset & 0x1F*/
    if (offset < 0) {
        offset &= 0x1F;
        offset |= BIT(5);
    } else {
        offset &= 0x1F;
    }

    /*6. VAL set to REG_OFFSET_ADC1_18[5:0]*/
    val |= offset;
    pmic_writel(REG_ADC_CTRL_ADC_OFFSET, val);

    /*7. read back check calibrate config done*/
    ret = aic_adc_chan_data_get(0, &adc_data);
    if (ret)
        return ret;

    /*8. New_offset = 2048 - B_ADC1_18[11:0]*/
    offset = 2048 - adc_data;
    if (abs(offset) > ADC_TOLERANCE)
        aicx_warn(0x14df5f, "d", "calibrate failed!!read back data:%d\n", adc_data);

    /*9. disable calibrate reg: REG_DISHADC1_18 = 0 */
    pmic_writel(REG_ADC_CTRL_ADC_DISH, 0);

    return ret;
}

u32 aic_adc2voltage(u32 adc_val)
{
    return (adc_val * 3000) / 4095;
}

void vcc5v_test_mode(bool enabled)
{
    if (enabled) {
        pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG16, BIT_PMU_RTC_EN_VCC5TST_18_REG);
    } else {
        pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG16, 0x0);
    }
}

void vcc5v_source_sel(bool is_vbat)
{
    u32 reg;

    reg = pmic_readl(REG_PMU_PD_PMU_INTER_REG3);

    if (is_vbat)
        reg &= ~ BIT_PMU_PD_REG_VCC5SEL_18;
    else
        reg |= BIT_PMU_PD_REG_VCC5SEL_18;

    pmic_writel(REG_PMU_PD_PMU_INTER_REG3, reg);
}

struct adc_phy_ops gp201_adc_phy_ops = {
    .adc_get = aic_adc_chan_data_get,
    .adc_cali = aic_adc_calibrate,
    .adc2voltage = aic_adc2voltage,
    .adc_enable = aic_adc_enable,
    .adc_is_enabled = aic_adc_is_enabled,
    .vbat_adc_chan = 12,
    .vbat_vbus_mode = vcc5v_test_mode,
    .vbat_vbus_sel = vcc5v_source_sel,
};

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
        pmic_writel(GP202_ADC_CTRL_ADC_EN_FORCE, BIT_ADC_CTRL_ADC_EN_FORCE_VAL | BIT_ADC_CTRL_ADC_EN_FORCE_EB);
    } else {
        pmic_writel(GP202_ADC_CTRL_ADC_EN_FORCE, BIT_ADC_CTRL_ADC_EN_FORCE_EB);
        pmic_writel(GP202_CKM_EN_GLB_CLK_PD_ADC, 0x0);
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
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        ret = adc_phy_ops_register(&gp202_adc_phy_ops);
    else
        ret = adc_phy_ops_register(&gp201_adc_phy_ops);

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(adc_phy_ops_init);

#ifndef CONFIG_TSC
int adc_phy_init(void)
{
    pmic_writel(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO8, BIT_FUNC_SEL(0));
    pmic_writel(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO9, BIT_FUNC_SEL(0));
    pmic_writel(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO10, BIT_FUNC_SEL(0));
    pmic_writel(PMIC_PG0_PAD_CTRL_IO_MUX_GPIO11, BIT_FUNC_SEL(0));
    return 0;
}
AIC_DRV_L1_INIT_EXPORT(adc_phy_init);
#endif
