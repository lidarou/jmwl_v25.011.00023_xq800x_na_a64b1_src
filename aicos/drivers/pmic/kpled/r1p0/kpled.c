/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  kpled r1p0 driver file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d659a
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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

int kpled_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

int kpled_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

static struct pmu_ops kpled_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_current = pmu_kit_get_current_table,
        .set_current = pmu_kit_set_current_table,
};

static struct pmu_io kpled_io = {
        .pmu_read = kpled_read,
        .pmu_write = kpled_write,
};

static const uint32_t kpled_current_table[] = {
    800, 1600, 2400, 3200, 4000, 4800, 5600, 8000,
    12000, 16000, 20000, 32000, 44000, 56000, 68000,
    80000
};

static struct pmu_info kpled_info = {
    .unit_name = "kpled",
    .parent_name = "VBAT",
    .enable_reg = REG_PMU_PD_LED_REG0,
    .enable_msk = BIT_PMU_PD_LED1_PD_18_REG,
    .cfg_update_reg = REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0,
    .cfg_update_msk = BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
    .current_reg = REG_PMU_PD_LED_REG0,
    .current_msk = BIT_PMU_PD_REG1_LED1_18(0xF),
    .current_table = kpled_current_table,
    .current_step_num = ARRAY_SIZE(kpled_current_table),
    .pops = &kpled_ops
};

static void kpled_adaptive(void)
{
    int pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (pmic_id != 0x2010000)
        kpled_info.enalbe_bit_inverted = true;
}

int kpled_driver_init(void)
{
    int ret;

    kpled_adaptive();

    ret = pmu_register(&kpled_info, &kpled_io);
    if (ret)
        aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, kpled_info.unit_name);

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(kpled_driver_init);
