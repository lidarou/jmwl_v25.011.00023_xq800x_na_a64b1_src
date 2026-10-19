/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Pin settings.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x114a56
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#include <aic_io.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <alog_api.h>
#ifndef CONFIG_MHA_PIN
#include <pin_cfg.h>
#endif
#include <pin_mgr.h>
#include <pmic_io.h>
#include <mha.h>

extern void pincfg_jtag(void);

#ifdef CONFIG_FIX_BID012
/* pre init of banyan pin */
void fix_bid012(void)
{
    /* set 0x410803e4 bit0 to 0 before pin init */
    AIC_WRITE32(0x410823e4, BIT(0));
    return;
}
#endif

#ifdef CONFIG_FIX_BID013
/* post init of banyan pin*/
void fix_bid013(void)
{
    /* if use eiu_int6/7 as iic3, should set 0x410803e0 bit0 to 1 after pin init */
    u32 eiuint6_f, eiuint7_f;

    eiuint6_f = AIC_READ32(0x40100040) & 0x7;
    eiuint7_f = AIC_READ32(0x40100044) & 0x7;
    if((eiuint6_f == 0x5) && (eiuint7_f == 0x5))
        AIC_WRITE32(0x410813e0, BIT(0));
    return;
}
#endif

void pin_init(void)
{
    const u32 (*pin_cfg)[2] = AIC_NULL;
    u32 rb_val, size;
    dig_pin_cfg_t *p_dig_pin = AIC_NULL;

#ifdef CONFIG_MHA_PIN
    p_dig_pin = (dig_pin_cfg_t *)mha_get_dig_pin_cfg();
#else
    p_dig_pin = g_dig_pinlist0;
#endif
    if (!p_dig_pin) {
        aicx_error(0x16a0f9, "", "No suitable dig pin config found!");
        return;
    }

    pin_cfg = p_dig_pin[0].pin_cfg;
    size = p_dig_pin[0].size;

    for (s32 i = 0; i < size; i++) {
        AIC_WRITE32(pin_cfg[i][0], pin_cfg[i][1]);
        rb_val = AIC_READ32(pin_cfg[i][0]);
        if (rb_val != pin_cfg[i][1]) {
            aicx_debug(0x18ca67, "xxx", "Pin val(0x%x = 0x%x), is not as expected(0x%x)!\n",
                       pin_cfg[i][0], rb_val, pin_cfg[i][1]);
        } else {
            aicx_debug(0x1e6e18, "xx", "Pin val(0x%x = 0x%x) as expected!\n",
                       pin_cfg[i][0], rb_val);
        }
    }

    /* switch jtag pin to other functoin in user mode */
    pincfg_jtag();

#ifdef CONFIG_FIX_BID012
    fix_bid012();
#endif
#ifdef CONFIG_FIX_BID013
    fix_bid013();
#endif
}

static s32 pmic_pin_init(void)
{
#ifdef CONFIG_PMIC
    u32 pmic_id, size;
    const u32 (*pmic_pin_cfg)[2] = AIC_NULL;
    pmu_pin_cfg_t *p_pmu_pin = AIC_NULL;

#ifdef CONFIG_MHA_PIN
    p_pmu_pin = (pmu_pin_cfg_t *)mha_get_pmu_pin_cfg();
#else
    p_pmu_pin = g_pmu_pinlist0;
#endif
    if (!p_pmu_pin) {
        aicx_error(0x197090, "", "No suitable pmu pin config found!");
        return AIC_ENODEV;
    }
    pmic_id = (aic_cmdline_get_value_int(cl_key_pmic_version) >> 16) & 0xFFFF;
    if (pmic_id == 0x202) {
        pmic_pin_cfg = p_pmu_pin[0].second_pin_cfg;
        size = p_pmu_pin[0].second_size;
    } else if (pmic_id == 0x201) {
        pmic_pin_cfg = p_pmu_pin[0].first_pin_cfg;
        size = p_pmu_pin[0].first_size;
    } else {
        return AIC_ENODEV;
    }

    for (s32 i = 0; i < size; i++) {
        u32 rb_val;
        pmic_writel(pmic_pin_cfg[i][0], pmic_pin_cfg[i][1]);
        rb_val = pmic_readl(pmic_pin_cfg[i][0]);
        if (rb_val != pmic_pin_cfg[i][1]) {
            aicx_error(0x19dfbd, "xxx", "Set  pmic pin 0x%x fail! exp: 0x%x real:0x%x \n",
                pmic_pin_cfg[i][0], pmic_pin_cfg[i][1], rb_val);
            return AIC_EIO;
        }
        else {
            aicx_info(0x00f622, "xx", "Set  pmic pin 0x%x = 0x%x\n",
                pmic_pin_cfg[i][0], pmic_pin_cfg[i][1]);
        }
    }
#endif
    return AIC_SUCCESS;
}
AIC_DRV_P2_INIT_EXPORT(pmic_pin_init);
