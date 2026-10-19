/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines lamp driver api.
 *    The lamp api will be called by aicos user functions.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00f5ce
#define LOG_MODULE    AMOD_PMIC

#include <aic_auto_reg.h>
#include <aic_bitmap.h>
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
#include <pmu_user.h>
#include <timer_inc.h>
#include <lamp.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

int lamp_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

int lamp_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

static const struct pmu_ops led_ops = {
        .enable = pmu_kit_enable,
        .disable = pmu_kit_disable,
        .is_enabled = pmu_kit_is_enabled,
        .get_current = pmu_kit_get_current_table,
        .set_current = pmu_kit_set_current_table,
};

static struct pmu_io led_io = {
        .pmu_read = lamp_read,
        .pmu_write = lamp_write,
};

static const uint32_t lamp_rgbw_current_table[] = {
    900, 1200, 1500, 1800, 2100, 2400, 2700, 3000,
    3300, 3600, 4800, 6000, 7200, 8400, 9600, 10800,
    12000, 13200, 14400, 15600, 16800, 18000, 19200,
    20400, 21600, 22800, 24000, 25200, 26400, 27600,
    28800, 30000
};

#define LED_INFO(_unit_name, _parent_name, cur_reg, cur_msk,\
                      en_reg, en_msk) {          \
         .unit_name            = #_unit_name,        \
         .parent_name          = #_parent_name,      \
         .current_reg          = cur_reg,      \
         .current_msk          = cur_msk,      \
         .enable_reg           = en_reg,       \
         .enable_msk           = en_msk,       \
         .cfg_update_reg       = REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0,   \
         .cfg_update_msk       = BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,   \
         .current_table        = lamp_rgbw_current_table,   \
         .current_step_num     = ARRAY_SIZE(lamp_rgbw_current_table),    \
         .scaling_time_us      = 10, \
         .pd_time_us           = 200, \
         .pops                 = &led_ops,   \
    }

static struct pmu_info leds_info[] = {
#ifdef CONFIG_LED_R
    LED_INFO(LED_R, VBAT, REG_PMU_PD_LED_REG1, BIT_PMU_PD_REG1_LED2R_18(0x1F),
            REG_PMU_RTC_PMU_RTC_EB_REG07, BIT_PMU_RTC_LED2G_EN_18_REG),
#endif

#ifdef CONFIG_LED_G
    LED_INFO(LED_G, VBAT, REG_PMU_PD_LED_REG1, BIT_PMU_PD_REG1_LED2G_18(0x1F),
            REG_PMU_RTC_PMU_RTC_EB_REG08, BIT_PMU_RTC_LED2G_EN_18_REG),
#endif

#ifdef CONFIG_LED_B
    LED_INFO(LED_B, VBAT, REG_PMU_PD_LED_REG1, BIT_PMU_PD_REG1_LED2B_18(0x1F),
            REG_PMU_RTC_PMU_RTC_EB_REG09, BIT_PMU_RTC_LED2B_EN_18_REG),
#endif

#ifdef CONFIG_LED_W
    LED_INFO(LED_W, VBAT, REG_PMU_PD_LED_REG1, BIT_PMU_PD_REG1_LED2W_18(0x1F),
            REG_PMU_RTC_PMU_RTC_EB_REG10, BIT_PMU_RTC_LED2W_EN_18_REG),
#endif
};

static int brightness_init(void)
{
    int ret;

    ret = lamp_write(REG_PMU_PD_LED_REG1, ~0UL);
    if (ret) {
        aicx_warn(0x1bb494, "", "lamp init config write fail!\n");
    }

    return ret;
}

int lamp_driver_init(void)
{
    int i, ret, result = 0;
    struct pmu_user *lamp_user_pmu;

    ret = brightness_init();
    if (ret)
        return ret;

    for (i = 0; i < ARRAY_SIZE(leds_info); i++) {
        ret = pmu_register(&leds_info[i], &led_io);
        if (ret) {
            aicx_warn(0x17fe24, "ss", "%s led %s register fail!\n", __func__, leds_info[i].unit_name);
        } else {
            lamp_user_pmu = pmu_get("lamp_leds", leds_info[i].unit_name);
            if (lamp_user_pmu)
                ret = lamp_unit_register(lamp_user_pmu);
            else
                aicx_warn(0x039552, "ss", "%s get lamp led %s fail!\n", __func__, leds_info[i].unit_name);
        }
        result |= ret;
    }

    return result;
}
AIC_DRV_P2_INIT_EXPORT(lamp_driver_init);
