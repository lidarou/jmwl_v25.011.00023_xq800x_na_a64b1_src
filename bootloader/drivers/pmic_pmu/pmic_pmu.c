/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is the pmic pmu operate interface.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmic_pmu.h>
#include <timer.h>

#define GP201_SETTLING_TIME 100 //us

struct pmu_info {
    char *unit_name;
    uint32_t voltage_reg;
    uint32_t voltage_msk;
    uint32_t enable_reg;
    uint32_t enable_msk;
    uint32_t cfg_update_reg;
    uint32_t cfg_update_msk;
    uint32_t voltage_min_uV;
    uint32_t voltage_max_uV;
    uint32_t voltage_step_uV;
    int settling_time_us;
    bool enalbe_bit_inverted;
};

#define PMU_UNIT_INFO(_unit_name, vol_reg, vol_msk, en_reg, en_msk, update_reg,\
                     update_msk, vol_min_uV, vol_max_uV, vol_step_uV,\
                    settling_us) {          \
         .unit_name            = #_unit_name,        \
         .voltage_reg          = vol_reg,      \
         .voltage_msk          = vol_msk,      \
         .enable_reg           = en_reg,       \
         .enable_msk           = en_msk,       \
         .cfg_update_reg       = update_reg,   \
         .cfg_update_msk       = update_msk,   \
         .voltage_min_uV       = vol_min_uV,   \
         .voltage_max_uV       = vol_max_uV,   \
         .voltage_step_uV      = vol_step_uV,  \
         .settling_time_us     = settling_us, \
         .enalbe_bit_inverted  = true,   \
    }

static struct pmu_info pmu_units_info[] = {
    /**
     *  DESCRIPTION
     *
     *  pmic pmu units for current subsys.
     *
     *   pmu_signal      BB_name
     *   BUCK0           VDDCORE
     *   BUCK1           VDDMP1V88
     *   BUCK2           VDDRFPA
     *   LDO1            AVDDRF1V8
     *   LDO2            AVDD1V8
     *   LDO3            VDDCAMIO1V8
     *   LDO4            AVDDRF1V2
     *   LDO5            DVDD3V
     *   LDO6            AVDDRF2V8
     *   LDO7            VDDBK3V3
     *   LDO8            AVDDCAM2V8
     *   LDO9            VDDSIM0
     *   LDO10           VDDSIM1
     *   LDO11           VDDUSB3V1
     *   LDO12           VDDSD3V
     *   LDO13           VDDTCXO
     *   LDO14           VDDMIC
     */
    //BUCK0
    PMU_UNIT_INFO(VDDCORE, REG_PMU_PD_BUCK_DCDC0_REG0, BIT_PMU_PD_REG2_BUCK0_18_NORM(0x3F),
                  REG_PMU_RTC_PMU_BUCK0_CTRL_REG, BIT_PMU_RTC_BUCK0_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               600000, 1230000, 10000, GP201_SETTLING_TIME),

    //BUCK1
    PMU_UNIT_INFO(VDDMP1V88, REG_PMU_PD_BUCK_DCDC1_REG0, BIT_PMU_PD_REG2_BUCK1_18(0x3F),
                  REG_PMU_RTC_PMU_BUCK1_CTRL_REG, BIT_PMU_RTC_BUCK1_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 2210000, 10000, GP201_SETTLING_TIME),

    //BUCK2
    PMU_UNIT_INFO(VDDRFPA, REG_PMU_PD_BUCK_DCDC2_REG0, BIT_PMU_PD_REG2_BUCK2_18(0x7F),
                   REG_PMU_RTC_PMU_BUCK2_CTRL_REG, BIT_PMU_RTC_BUCK2_POWER_DOWN_REG,
                REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
                500000, 3675000, 25000, GP201_SETTLING_TIME),

    //LDO1
    PMU_UNIT_INFO(AVDDRF1V8, REG_PMU_PD_LDO1_REG, BIT_PMU_PD_REG_LDO1_18(0x1F),
                  REG_PMU_RTC_PMU_LDO1_CTRL_REG, BIT_PMU_RTC_LDO1_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1530000, 1995000, 15000, GP201_SETTLING_TIME),

    //LDO2
    PMU_UNIT_INFO(AVDD1V8, REG_PMU_PD_LDO2_REG, BIT_PMU_PD_REG_LDO2_18(0x1F),
                  REG_PMU_RTC_PMU_LDO2_CTRL_REG, BIT_PMU_RTC_LDO2_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1530000, 1995000, 15000, GP201_SETTLING_TIME),

    //LDO3
    PMU_UNIT_INFO(VDDCAMIO1V8, REG_PMU_PD_LDO3_REG, BIT_PMU_PD_REG_LDO3_18(0x1F),
                  REG_PMU_RTC_PMU_LDO3_CTRL_REG, BIT_PMU_RTC_LDO3_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1530000, 1995000, 15000, GP201_SETTLING_TIME),

    //LDO4
    PMU_UNIT_INFO(AVDDRF1V2, REG_PMU_PD_LDO4_REG, BIT_PMU_PD_REG_LDO4_18(0x1F),
                  REG_PMU_RTC_PMU_LDO4_CTRL_REG, BIT_PMU_RTC_LDO4_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1025000, 1412500, 12500, GP201_SETTLING_TIME),

    //LDO5
    PMU_UNIT_INFO(DVDD3V, REG_PMU_PD_LDO5_REG, BIT_PMU_PD_REG_LDO5_18(0x3F),
                  REG_PMU_RTC_PMU_LDO5_CTRL_REG, BIT_PMU_RTC_LDO5_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO6
    PMU_UNIT_INFO(AVDDRF2V8, REG_PMU_PD_LDO6_REG, BIT_PMU_PD_REG_LDO6_18(0x3F),
                  REG_PMU_RTC_PMU_LDO6_CTRL_REG, BIT_PMU_RTC_LDO6_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO7
    PMU_UNIT_INFO(VDDBK3V3, REG_PMU_PD_LDO7_REG, BIT_PMU_PD_REG_LDO7_18(0x3F),
                  REG_PMU_RTC_PMU_LDO7_CTRL_REG, BIT_PMU_RTC_LDO7_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO8
    PMU_UNIT_INFO(AVDDCAM2V8, REG_PMU_PD_LDO8_REG, BIT_PMU_PD_REG_LDO8_18(0x3F),
                  REG_PMU_RTC_PMU_LDO8_CTRL_REG, BIT_PMU_RTC_LDO8_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO9
    PMU_UNIT_INFO(VDDSIM0, REG_PMU_PD_LDO9_REG, BIT_PMU_PD_REG_LDO9_18(0x3F),
                  REG_PMU_RTC_PMU_LDO9_CTRL_REG, BIT_PMU_RTC_LDO9_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO10
    PMU_UNIT_INFO(VDDSIM1, REG_PMU_PD_LDO10_REG, BIT_PMU_PD_REG_LDO10_18(0x3F),
                  REG_PMU_RTC_PMU_LDO10_CTRL_REG, BIT_PMU_RTC_LDO10_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO11
    PMU_UNIT_INFO(VDDUSB3V1, REG_PMU_PD_LDO11_REG, BIT_PMU_PD_REG_LDO11_18(0x3F),
                  REG_PMU_RTC_PMU_LDO11_CTRL_REG, BIT_PMU_RTC_LDO11_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO12
    PMU_UNIT_INFO(VDDSD3V, REG_PMU_PD_LDO12_REG, BIT_PMU_PD_REG_LDO12_18(0x3F),
                  REG_PMU_RTC_PMU_LDO12_CTRL_REG, BIT_PMU_RTC_LDO12_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO13
    PMU_UNIT_INFO(VDDTCXO, REG_PMU_PD_LDO13_REG, BIT_PMU_PD_REG_LDO13_18(0x3F),
                  REG_PMU_RTC_PMU_LDO13_CTRL_REG, BIT_PMU_RTC_LDO13_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),

    //LDO14
    PMU_UNIT_INFO(VDDMIC, REG_PMU_PD_LDO14_REG, BIT_PMU_PD_REG_LDO14_18(0x3F),
                  REG_PMU_RTC_PMU_LDO14_CTRL_REG, BIT_PMU_RTC_LDO14_POWER_DOWN_REG,
               REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0, BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
               1600000, 3490000, 30000, GP201_SETTLING_TIME),
};

static int pmu_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

static int pmu_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

static int pmu_sel2voltage(struct pmu_info *pinfo, int sel)
{
    int max_step_nums;

    max_step_nums = (pinfo->voltage_max_uV - pinfo->voltage_min_uV)
                    / pinfo->voltage_step_uV;

    if (sel > max_step_nums) {
        log_print("pmu voltage target sel overflow!\n");
        return AIC_EIO;
    }

    return pinfo->voltage_min_uV + sel * pinfo->voltage_step_uV;
}

static int pmu_io_update(u32 pmu_reg, u32 pmu_msk, u32 pmu_val)
{
    u32 val;
    int ret;

    if (!pmu_msk) {
        log_print("illegal mask value\n");
        ret = AIC_EIO;
        goto out;
    }

    ret = pmu_read(pmu_reg, &val);
    if (ret) {
        log_print("pmu io read fail!\n");
        ret = AIC_EIO;
        goto out;
    }

    val &= ~pmu_msk;

    pmu_val <<= aic_find_first_bit(pmu_msk);

    val |= pmu_val;

    ret = pmu_write(pmu_reg, val);
    if (ret)
        log_print("pmu io read fail!\n");

out:
    return ret;
}

static int pmu_kit_set_voltage_linear(struct pmu_info *pinfo, u32 voltage_uV)
{
    int ret, sel, final_vol;
    u32 vol_delta;

    vol_delta = voltage_uV - pinfo->voltage_min_uV;

    if (pinfo->voltage_step_uV == 0) {
        ret = AIC_EIO;
        goto out;
    }

    if (voltage_uV < pinfo->voltage_min_uV)
        voltage_uV = pinfo->voltage_min_uV;

    sel = DIV_ROUND_UP(vol_delta, pinfo->voltage_step_uV);
    if (sel < 0) {
        ret = sel;
        goto out;
    }

    /*insure final output voltage come within tolerance*/
    final_vol = pmu_sel2voltage(pinfo, sel);
    if (final_vol < 0)
        return AIC_EIO;

    ret = pmu_io_update(pinfo->voltage_reg, pinfo->voltage_msk, sel);
    if (ret) {
        log_print("%s pmu write voltage fail!\n", __func__);
        goto out;
    }

    ret = pmu_io_update(pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        log_print("%s pmu update config fail!\n", __func__);

    aic_udelay(pinfo->settling_time_us);
out:
    return ret;
}

static int pmu_kit_get_voltage_linear(struct pmu_info *pinfo, u32 *voltage_uV)
{
    int ret;
    u32 val = 0;

    if (!pinfo->voltage_msk) {
        log_print("illegal mask value\n");
        ret = AIC_EIO;
        goto out;
    }

    ret = pmu_read(pinfo->voltage_reg, &val);
    if(ret) {
        log_print("pmu io read fail!\n");
        ret = AIC_EIO;
        goto out;
    }

    val &= pinfo->voltage_msk;
    val >>= aic_find_first_bit(pinfo->voltage_msk);

    *voltage_uV = pinfo->voltage_min_uV + val * pinfo->voltage_step_uV;

out:
    return ret;
}

static int pmu_kit_is_enabled(struct pmu_info *pinfo)
{
    int ret;
    u32 val = 0;

    ret = pmu_read(pinfo->enable_reg, &val);
    if(ret) {
        log_print("pmu io read fail!\n");
        return AIC_EIO;
    }

    val &= pinfo->enable_msk;

    if (pinfo->enalbe_bit_inverted)
        return val != pinfo->enable_msk;
    else
        return val == pinfo->enable_msk;
}

static int pmu_kit_enable(struct pmu_info *pinfo)
{
    int ret;
    u32 val = 0;

    if (pinfo->enalbe_bit_inverted)
        val = 0;
    else
        val = pinfo->enable_msk;

    ret = pmu_io_update(pinfo->enable_reg, pinfo->enable_msk, val);
    if(ret) {
        log_print("%s pmu io update fail!\n", __func__);
        return ret;
    }

    ret = pmu_io_update(pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        log_print("%s pmu update config fail!\n", __func__);
    else
        aic_udelay(pinfo->settling_time_us);

    return ret;
}

static int pmu_kit_disable(struct pmu_info *pinfo)
{
    int ret, val;

    if (pinfo->enalbe_bit_inverted)
        val = pinfo->enable_msk;
    else
        val = 0;

    ret = pmu_io_update(pinfo->enable_reg, pinfo->enable_msk, val);
    if(ret) {
        log_print("%s pmu io update fail!\n", __func__);
        return ret;
    }

    ret = pmu_io_update(pinfo->cfg_update_reg, pinfo->cfg_update_msk, 0x1);
    if (ret)
        log_print("%s pmu update config fail!\n", __func__);

    return ret;
}

static struct pmu_info *find_pmu_unit(char *pmu_name)
{
    int i;

    for (i = 0; i < sizeof(pmu_units_info) / sizeof(pmu_units_info[0]); i++) {
        if (strcmp(pmu_name, pmu_units_info[i].unit_name) == 0)
            return &pmu_units_info[i];
    }

    return NULL;
}

int pmu_enable(char *pmu_name)
{
    struct pmu_info * pinfo;

    pinfo = find_pmu_unit(pmu_name);
    if(!pinfo) {
        log_print("fail!can't find pmu:%s\n", pmu_name);
        return AIC_ENOENT;
    }

    return pmu_kit_enable(pinfo);
}

int pmu_disable(char *pmu_name)
{
    struct pmu_info * pinfo;

    pinfo = find_pmu_unit(pmu_name);
    if(!pinfo) {
        log_print("fail!can't find pmu:%s\n", pmu_name);
        return AIC_ENOENT;
    }

    return pmu_kit_disable(pinfo);
}

int pmu_voltage_set(char *pmu_name, u32 voltage_mV)
{
    struct pmu_info * pinfo;

    pinfo = find_pmu_unit(pmu_name);
    if(!pinfo) {
        log_print("fail!can't find pmu:%s\n", pmu_name);
        return AIC_ENOENT;
    }

    return pmu_kit_set_voltage_linear(pinfo, voltage_mV * 1000);
}

int pmu_voltage_get(char *pmu_name, u32 *voltage_mV)
{
    struct pmu_info * pinfo;
    int ret;

    pinfo = find_pmu_unit(pmu_name);
    if(!pinfo) {
        log_print("fail!can't find pmu:%s\n", pmu_name);
        return AIC_ENOENT;
    }

    ret = pmu_kit_get_voltage_linear(pinfo, voltage_mV);
    *voltage_mV = *voltage_mV / 1000;

    return ret;
}

int pmu_is_enabled(char *pmu_name)
{
    struct pmu_info * pinfo;

    pinfo = find_pmu_unit(pmu_name);
    if(!pinfo) {
        log_print("fail!can't find pmu:%s\n", pmu_name);
        return AIC_ENOENT;
    }

    return pmu_kit_is_enabled(pinfo);
}
