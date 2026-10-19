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

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <aic_math.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include "adc_phy.h"
#include "adc_reg.h"
#include <timer.h>
#include <pmic_efuse.h>

bool adc_irq_enable = false;
u32 adc_irq_status = 0;
bool non_init = true;

static int gp201_adc_channel_select(int channel)
{
    if (channel > ADC_CHANNEL_MAX)
        return AIC_ENODEV;

    pmic_writel(REG_ADC_CTRL_ADC_SAMP_CH, BIT_ADC_CTRL_ADC1_AIN_CH_SEL(channel));

    return 0;
}

static void gp201_adc_sample_enable(void)
{
    pmic_writel(REG_ADC_CTRL_ADC_SAMP_EN, BIT_ADC_CTRL_ADC1_AIN_SAMP_EN);
}

static void gp201_adc_sample_trigger(void)
{
    pmic_writel(REG_ADC_CTRL_ADC_TRIG, BIT_ADC_CTRL_ADC1_MANUAL_TRIG);
}

void gp201_adc_int_enable(void)
{
    pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) | BIT_ADC_CTRL_ADC_INTR_EN);
}

void gp201_adc_int_disable(void)
{
    pmic_writel(REG_ADC_CTRL_INT_EN, pmic_readl(REG_ADC_CTRL_INT_EN) & ~BIT_ADC_CTRL_ADC_INTR_EN);
}

static void gp201_adc_int_status_clear(void)
{
    pmic_writel(REG_ADC_CTRL_INT_CLR, BIT_ADC_CTRL_ADC_INTR_CLR);
}

static int gp201_adc_data_get(u32 *data)
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

    gp201_adc_int_status_clear();

    if(i == ADC_WAIT_TIME_CNT) {
        ret = AIC_ENOTSUP;
#ifdef ADC_DEBUG
        log_print("int_en:0x%x int_status:0x%x int_status_raw:0x%x ipu_status:0x%x\n",
        pmic_readl(REG_ADC_CTRL_INT_EN), pmic_readl(REG_ADC_CTRL_ADC_INT_STS),
        pmic_readl(REG_ADC_CTRL_ADC_INT_STS_RAW), pmic_readl(0xc00));
#endif
    } else {
        *data = pmic_readl(REG_ADC_CTRL_ADC_DATA);
    }

    return ret;
}

void gp201_adc_reset(void)
{
    u32 data;

    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, BIT_GLB_PD_SW_RST_GLB_CLK_APB_ADC);
    pmic_writel(REG_GLB_PD_SW_RST_GLB_CLK_APB_ADC, 0);

    data = pmic_readl(REG_ADC_CTRL_GLB_CTRL_ADC);
    data &= BIT_ADC_CTRL_SOC_HIGN_ADC_WAIT_NUM(0xF);
    data |= BIT_ADC_CTRL_SOC_HIGN_ADC_WAIT_NUM(0x4);
    pmic_writel(REG_ADC_CTRL_GLB_CTRL_ADC, data);

    gp201_adc_int_enable();
}

int gp201_adc_chan_data_get(u32 chan, u32 *data)
{
    int ret;

    /*enable adc*/
    pmic_writel(0x00008018,0x00000001);
    pmic_writel(0x00008020,0x00000001);
    pmic_writel(0x00000670,0x00000000);  //#Force turn on ADC
    pmic_writel(0x00000808,0x00000001);  //#turn on INTLDO2, enable ADC VREFP_HQ

    if (non_init) {
        non_init = false;
        gp201_adc_reset();
    }

    ret = gp201_adc_channel_select(chan);
    if (ret) {
        log_print("unsupport adc channel:%d\n", chan);
        goto out;
    }

    gp201_adc_sample_enable();

    gp201_adc_sample_trigger();

    ret = gp201_adc_data_get(data);
    if (ret < 0)
        log_print("wait adc finish timeout\n");

out:

    /*disable adc*/
    pmic_writel(0x00008018,0x00000000);
    pmic_writel(0x00008020,0x00000000);
    pmic_writel(0x00000670,0x00000002);  //#Force turn off ADC
    pmic_writel(0x00000808,0x00000000);  //#turn off INTLDO2, disable ADC VREFP_HQ

    return ret;
}

void gp201_adc_irq_handler(void *val)
{
    adc_irq_status = pmic_readl(REG_ADC_CTRL_ADC_INT_STS);

#ifdef ADC_DEBUG
    log_print("adc irq_status:0x%x\n", adc_irq_status);
#endif

    gp201_adc_int_status_clear();
}

void gp201_adc_irq_init(void)
{

#ifdef ADC_DEBUG
    log_print("%s %d\n", __func__, __LINE__);
#endif

    adc_irq_enable = true;
    gp201_adc_int_enable();
}

void gp201_adc_irq_deinit(void)
{
    gp201_adc_int_disable();
}

int gp201_adc_calibrate(void)
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
    ret = gp201_adc_chan_data_get(0, &adc_data);
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
    ret = gp201_adc_chan_data_get(0, &adc_data);
    if (ret)
        return ret;

    /*8. New_offset = 2048 - B_ADC1_18[11:0]*/
    offset = 2048 - adc_data;
    if (ABS(offset) > ADC_TOLERANCE)
        log_print("calibrate failed!!read back data:%d\n", adc_data);
    else
        log_print("calibrate success!!read back data:%d\n", adc_data);

    /*9. disable calibrate reg: REG_DISHADC1_18 = 0 */
    pmic_writel(REG_ADC_CTRL_ADC_DISH, 0);

    return ret;
}

u32 gp201_adc2voltage(u32 adc_val)
{
    return (adc_val * 3000) / 4095;
}

int adc_self_cal(void)
{
    int ret;

    ret = gp201_adc_calibrate();
    if (ret)
        log_print("adc self calibrate failed!!\n");

    return ret;
}

static void gp201_vcc5v_test_mode(bool enabled)
{
    if (enabled) {
        pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG16, BIT_PMU_RTC_EN_VCC5TST_18_REG);
    } else {
        pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG16, 0x0);
    }
}

static void gp201_vcc5v_source_sel(bool is_vbat)
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
    .adc_get = gp201_adc_chan_data_get,
    .adc2voltage = gp201_adc2voltage,
    .vbat_adc_chan = 12,
    .vbat_vbus_mode = gp201_vcc5v_test_mode,
    .vbat_vbus_sel = gp201_vcc5v_source_sel,
    .adc_calibrate = gp201_adc_calibrate,
};


#define GP202_ADC_CALI_BLK          0x0
#define GP202_ADC_CALI_BIT          29
#define GP202_ADC_CALI_VALID_BITS   0x7
#define GP202_ADC_JOB_DONE_FLAG     0x1
#define GP202_ADC_JOB_DONE_TIMEOUT  1000
#define GP202_ADC_CALI_L7_GOLDEN    0x3c0
#define GP202_ADC_CHAN_MAX          4
#define GP202_INTEL_VBAT_CHAN       8

void gp202_adc_irq_clr(void)
{
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_CLR, BIT_ADC_CTRL_GP202_ADC_INT_CLR);
}

int gp202_trig_adc_job_wait_done(void)
{
    int ret = 0, cnt = 0;

    gp202_adc_irq_clr();

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_EN, BIT_ADC_CTRL_GP202_ADC_INT_EN);
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_TRIG, BIT_ADC_CTRL_GP202_ADC_TRIG);


    while (!pmic_readl(GP202_ADC_CTRL_ADC_INT_STS_RAW) && cnt < ADC_WAIT_TIME_CNT) {
        aic_udelay(1000);
        cnt++;
    };

    if (cnt == ADC_WAIT_TIME_CNT) {
        log_print("error: wait adc finish timeout!\n");
        ret = AIC_ETIME ;
    }

    gp202_adc_irq_clr();
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_EN, 0);

    return ret;
}

int gp202_adc_chan_data_get(u32 chan, u32 *data)
{
    int ret = 0;
    u32 val;

    if (!(chan <= GP202_ADC_CHAN_MAX || chan == GP202_INTEL_VBAT_CHAN)) {
        ret = AIC_EIO;
        goto out;
    }

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_AIN_SEL, BIT_ADC_CTRL_GP202_ADC_AIN_SEL(chan));

    ret = gp202_trig_adc_job_wait_done();
    if (ret)
        goto out;

    val = pmic_readl(GP202_ADC_CTRL_ADC_DATA_RPT);
    if (val & BIT_ADC_CTRL_DBG_ADC_DATA_ERR) {
        log_print("error: adc data empty!\n");
        ret = AIC_EIO;
        goto out;
    }

    *data = BIT_ADC_CTRL_ADC_DATA_RPT(val);

out:
    return ret;
}

u32 gp202_adc2voltage(u32 adc_val)
{
    return (adc_val * 1800) / 2047;
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

void gp202_vcc5v_test_mode(bool enabled){}
void gp202_vcc5v_source_sel(bool is_vbat){}
int gp202_adc_calibrate(void)
{
    int ret = 0, data_rpt = 0;
    u32 reg = 0, val = 0;

    ret = aic_pmic_efuse_block_read(GP202_ADC_CALI_BLK, &val);
    if (ret) {
        log_print("%s: read efuse failed!\n", __func__);
        goto out;
    }

    val = ((val >> GP202_ADC_CALI_BIT) & GP202_ADC_CALI_VALID_BITS);
    if (!val) {
        ret = AIC_ENODEV;
        log_print("%s: efuse cali val is NULL!\n", __func__);
        goto out;
    }

    reg = pmic_readl(GP202_ADC_CTRL_GP202_ADC_LDO_CFG);
    reg &= ~BIT_ADC_CTRL_GP202_ADC_LDO_CTUNE(0x7);
    reg |= val;
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_LDO_CFG, reg);

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_INT_EN, BIT_ADC_CTRL_GP202_ADC_INT_EN);
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_AIN_SEL, 0xF);
    /*calibrate level 7, golden value 0x3C0*/
    pmic_writel(GP202_ADC_CTRL_GP202_ADC_CAL_CFG,
                BIT_ADC_CTRL_GP202_ADC_CAL(0x7) | BIT_ADC_CTRL_GP202_ADC_CALIBRA_EN);

    ret = gp202_trig_adc_job_wait_done();
    if (ret) {
        log_print("%s: wait adc job done timeout!\n", __func__);
        goto out;
    }

    data_rpt = 0;
    data_rpt = pmic_readl(GP202_ADC_CTRL_ADC_DATA_RPT);
    if (data_rpt & BIT_ADC_CTRL_DBG_ADC_DATA_ERR) {
        ret = AIC_EIO;
        log_print("%s: adc data err.\n", __func__);
        goto out;
    }

    val = 0;
    data_rpt  = BIT_ADC_CTRL_ADC_DATA_RPT(data_rpt);
    if (data_rpt < GP202_ADC_CALI_L7_GOLDEN)
        val |= BIT_ADC_CTRL_GP202_ADC_OFFSET_MODE;

    data_rpt = ABS(data_rpt - GP202_ADC_CALI_L7_GOLDEN);
    if (data_rpt > BIT_ADC_CTRL_GP202_ADC_OFFSET_VAL(0x3F)) {
        val |= BIT_ADC_CTRL_GP202_ADC_OFFSET_VAL(0x3F);
        log_print("%s: adc cali date out of range, use max offset.\n", __func__);
    } else {
        val |= data_rpt;
    }

    pmic_writel(GP202_ADC_CTRL_GP202_ADC_CAL_CFG, 0x0);
    gp202_adc_irq_clr();

out:
    return ret;
}

struct adc_phy_ops gp202_adc_phy_ops = {
    .adc_get = gp202_adc_chan_data_get,
    .adc2voltage = gp202_adc2voltage,
    .vbat_adc_chan = GP202_INTEL_VBAT_CHAN,
    .vbat_vbus_mode = gp202_vcc5v_test_mode,
    .vbat_vbus_sel = gp202_vcc5v_source_sel,
    .adc_calibrate = gp202_adc_calibrate,
};

int adc_phy_ops_init(void)
{
    int ret =0, pmic_id;

    pmic_id = pmic_readl(REG_GLB_PD_CHIP_ID_VERSION);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        ret = adc_phy_ops_register(&gp202_adc_phy_ops);
    else
        ret = adc_phy_ops_register(&gp201_adc_phy_ops);

    return ret;
}

