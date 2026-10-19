/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  gp202 pmu driver file.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fcc0a
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <pmu_driver.h>
#include <pmu_kit.h>
#include <pmic_efuse.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define BIT_GLB_PD_BUCK0_VOUT_TRIM_NORM_REG(x)                                  (((x) & 0x3f))
#define BIT_GLB_PD_BUCK1_VOUT_TRIM_REG(x)                                       (((x) & 0x3f) << 16)
#define BIT_GLB_PD_LDO_VSEL_REG(x, y)                                           (((x) & y) << 20)
#define BIT_PMU_RTC_BUCK_LDO_POWER_DOWN_REG                                     BIT(0)

#define GP201_SCALING_SETTLING_TIME 10 //us
#define GP201_PD_SETTLING_TIME 2000 //us

int gp202_pmu_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

int gp202_pmu_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

static struct pmu_ops gp202_pmu_gen_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = pmu_kit_get_voltage_linear,
        .set_voltage = pmu_kit_set_voltage_linear,
};

static struct pmu_io gp202_pmu_io = {
        .pmu_read = gp202_pmu_read,
        .pmu_write = gp202_pmu_write,
};

#define PMU_UNIT_INFO(_unit_name, _parent_name, vol_reg, vol_msk, en_reg, en_msk,\
                                          update_reg, update_msk, vol_min_uV, vol_max_uV, vol_step_uV,\
                                         scaling_settling_us, pd_settling_us, punit_always_on, \
                                         punit_boot_on, punit_bypass, punit_scaling_boot_voltage) {          \
                              .unit_name            = #_unit_name,        \
                              .parent_name          = #_parent_name,      \
                              .voltage_reg          = vol_reg,      \
                              .voltage_msk          = vol_msk,      \
                              .enable_reg           = en_reg,       \
                              .enable_msk           = en_msk,       \
                              .cfg_update_reg       = update_reg,   \
                              .cfg_update_msk       = update_msk,   \
                              .voltage_min_uV       = vol_min_uV,   \
                              .voltage_max_uV       = vol_max_uV,   \
                              .voltage_step_uV      = vol_step_uV,  \
                              .scaling_time_us      = scaling_settling_us, \
                              .pd_time_us           = pd_settling_us, \
                              .scaling_boot_voltage = punit_scaling_boot_voltage, \
                              .always_on            = punit_always_on,        \
                              .boot_on              = punit_boot_on,          \
                              .bypass               = punit_bypass,  \
                              .pops                 = &gp202_pmu_gen_ops,   \
                              .enalbe_bit_inverted  = true,   \
                         }

#define PMU_UNIT_INFO_OPS(_unit_name, _parent_name, vol_reg, vol_msk, en_reg, en_msk,\
                     update_reg, update_msk, vol_min_uV, vol_max_uV, vol_step_uV,\
                    scaling_settling_us, pd_settling_us, punit_always_on, \
                    punit_boot_on, punit_bypass, pmu_ops, punit_scaling_boot_voltage) {\
         .unit_name            = #_unit_name,        \
         .parent_name          = #_parent_name,      \
         .voltage_reg          = vol_reg,      \
         .voltage_msk          = vol_msk,      \
         .enable_reg           = en_reg,       \
         .enable_msk           = en_msk,       \
         .cfg_update_reg       = update_reg,   \
         .cfg_update_msk       = update_msk,   \
         .voltage_min_uV       = vol_min_uV,   \
         .voltage_max_uV       = vol_max_uV,   \
         .voltage_step_uV      = vol_step_uV,  \
         .scaling_time_us      = scaling_settling_us, \
         .pd_time_us           = pd_settling_us, \
         .scaling_boot_voltage = punit_scaling_boot_voltage, \
         .always_on            = punit_always_on,        \
         .boot_on              = punit_boot_on,          \
         .bypass               = punit_bypass,  \
         .pops                 = pmu_ops,   \
         .enalbe_bit_inverted  = true,   \
    }

int gp202_ldo0_1_set_voltage(struct pmu_unit *punit, u32 voltage_uV,
                                      u32 tolerance_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret;
    u32 voltage_base = pinfo->voltage_min_uV;

    if (voltage_uV > pinfo->voltage_max_uV) {
        ret = pmu_io_update(pio, pinfo->voltage_reg, 0x1, 0x1);
        pinfo->voltage_min_uV += 600000;
    } else {
        ret = pmu_io_update(pio, pinfo->voltage_reg, 0x1, 0x0);
    }

    if (ret) {
        aicx_warn(0x10a379, "s", "%s pmu switch power range fail!\n", __func__);
        goto out;
    }

    ret = pmu_kit_set_voltage_linear(punit, voltage_uV, tolerance_uV);
    if (ret)
        aicx_warn(0x083906, "s", "%s pmu scaling voltage fail!\n", __func__);
out:
    pinfo->voltage_min_uV = voltage_base;
    return ret;
}

int gp202_ldo0_1_get_voltage(struct pmu_unit *punit, u32 *voltage_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    u32 voltage_base = pinfo->voltage_min_uV;
    int ret;
    u32 val;

    ret= pio->pmu_read(pinfo->voltage_reg, &val);
    if (ret) {
        aicx_warn(0x050912, "s", "%s get power range fail!\n", __func__);
        goto out;
    }

    if (val & 0x1)
        pinfo->voltage_min_uV += 600000;

    ret = pmu_kit_get_voltage_linear(punit, voltage_uV);
    if (ret)
        aicx_warn(0x15d79f, "s", "%s pmu get voltage fail!\n", __func__);

out:
    pinfo->voltage_min_uV = voltage_base;
    return ret;
}

static const struct pmu_ops gp202_pmu_ldo0_1_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = gp202_ldo0_1_get_voltage,
        .set_voltage = gp202_ldo0_1_set_voltage,
};

int gp202_ldo_gen_set_voltage(struct pmu_unit *punit, u32 voltage_uV,
                                      u32 tolerance_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret, sel;

    if (voltage_uV > 3100000) {
        pinfo->voltage_min_uV = 3100000;
        sel = 0x3;
    } else if (voltage_uV > 2800000) {
        pinfo->voltage_min_uV = 2800000;
        sel = 0x2;
    } else if (voltage_uV > 2600000) {
        pinfo->voltage_min_uV = 2600000;
        sel = 0x1;
    } else {
        pinfo->voltage_min_uV = 1600000;
        sel = 0;
    }

    ret = pmu_io_update(pio, pinfo->voltage_reg, 0x3 << 25, sel);
    if (ret) {
        aicx_warn(0x10a379, "s", "%s pmu switch power range fail!\n", __func__);
        goto out;
    }

    ret = pmu_kit_set_voltage_linear(punit, voltage_uV, tolerance_uV);
    if (ret)
        aicx_warn(0x083906, "s", "%s pmu scaling voltage fail!\n", __func__);
out:
    return ret;
}

int gp202_ldo_gen_get_voltage(struct pmu_unit *punit, u32 *voltage_uV)
{
    struct pmu_info *pinfo = punit->pinfo;
    struct pmu_io *pio= punit->pmu_io;
    int ret, sel;
    u32 val;

    ret= pio->pmu_read(pinfo->voltage_reg, &val);
    if (ret) {
        aicx_warn(0x050912, "s", "%s get power range fail!\n", __func__);
        goto out;
    }

    sel = (val >> 25) & 0x3;

    switch (sel) {
    case 0x3:
        pinfo->voltage_min_uV = 3100000;
        break;
    case 0x2:
        pinfo->voltage_min_uV = 2800000;
        break;
    case 0x1:
        pinfo->voltage_min_uV = 2600000;
        break;
    case 0x0:
    default:
        pinfo->voltage_min_uV = 1600000;
    }

    ret = pmu_kit_get_voltage_linear(punit, voltage_uV);
    if (ret)
        aicx_warn(0x15d79f, "s", "%s pmu get voltage fail!\n", __func__);

out:
    return ret;
}

static const struct pmu_ops gp202_pmu_ldo_gen_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_voltage = gp202_ldo_gen_get_voltage,
        .set_voltage = gp202_ldo_gen_set_voltage,
};

struct pmu_info gp202_pmu_units_info[] = {
    #include <gp202_pmu_units.h>
};

int pmic_pmu_init(void)
{
    int ret, i, result = 0;

    for (i = 0; i < ARRAY_SIZE(gp202_pmu_units_info); i++) {
        ret = pmu_register(&gp202_pmu_units_info[i], &gp202_pmu_io);
        if (ret)
            aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, gp202_pmu_units_info[i].unit_name);
            result |= ret;
    }

    return result;
}
AIC_DRV_P1_INIT_EXPORT(pmic_pmu_init);
