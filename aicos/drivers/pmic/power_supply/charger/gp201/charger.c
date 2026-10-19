/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  chgr r1p0 driver file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x109cfa
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
#include <pmu_user.h>
#include <power_supply.h>

#define     PMIC_CURRENT_TOLERANCE      50000
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

struct pmu_user *pchg = NULL;

struct aic_psy_ops {
    bool (*psy_wrap_is_charging_complete)(void);
    int (*psy_wrap_clear_chg_complete_state)(void);
    int (*psy_wrap_cc_current_set)(u32 cc_current_mA);
    int (*psy_wrap_cc_current_get)(u32 *cc_current_mA);
    bool (*psy_wrap_is_charging_enabled)(void);
    int (*__psy_wrap_charging_enable)(bool enabled);
};

struct aic_psy_ops *cur_psy_ops;

int chgr_write(u32 reg, u32 val)
{
    return pmic_writel((u16) reg, val);
}

int chgr_read(u32 reg, u32 *val)
{
    int ret;

    ret = pmic_burst_read((u16) reg, (u8 *)val, 4);

    return ret;
}

int chgr_enable(struct pmu_unit *punit)
{
    pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG04, 0);

    return 0;
}

int chgr_disable(struct pmu_unit *punit)
{
    pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG04, BIT_PMU_RTC_PD_CHRG_18R_REG_EB | BIT_PMU_RTC_PD_CHRG_18R_REG);

    return 0;
}

int chgr_is_enabled(struct pmu_unit *punit)
{
    u32 val;
    int ret = 0;

    val = pmic_readl(REG_PMU_RTC_PMU_RTC_EB_REG04);
    if (val)
        ret = 0;
    else
        ret = 1;

    return ret;
}

static struct pmu_ops chgr_ops = {
        .enable = chgr_enable,
        .disable = chgr_disable,
        .is_enabled = chgr_is_enabled,
        .get_current = pmu_kit_get_current_table,
        .set_current = pmu_kit_set_current_table,
};

static struct pmu_io chgr_io = {
        .pmu_read = chgr_read,
        .pmu_write = chgr_write,
};

static const uint32_t chgr_current_table[] = {
    300000, 400000, 500000, 600000
};

static struct pmu_info chgr_info = {
    .unit_name = "chgr",
    .parent_name = "VBAT",
    .enable_reg = REG_PMU_PD_LINEAR_CHARGER_REG0,
    .enable_msk = BIT_PMU_PD_REG1_CHARGER_18_EB,
    .cfg_update_reg = REG_PMU_RTC_PMU_DCDC_LDO_CTRL_REG0,
    .cfg_update_msk = BIT_PMU_RTC_DCDC_LDO_CFG_UPDATE_REG,
    .current_reg = REG_PMU_PD_LINEAR_CHARGER_REG0,
    .current_msk = BIT_PMU_PD_REG2_CHARGER_18(0x3),
    .current_table = chgr_current_table,
    .current_step_num = ARRAY_SIZE(chgr_current_table),
    .boot_on = true,
    .pops = &chgr_ops
};

int chgr_driver_init(void)
{
    int ret, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) != 0x201)
        return 0;

    ret = pmu_register(&chgr_info, &chgr_io);
    if (ret)
        aicx_warn(0x0a1bd8, "ss", "%s pmu unit %s register fail!\n", __func__, chgr_info.unit_name);

    return ret;
}
DIS_GXEM_DRV_P1_INIT_EXPORT(chgr_driver_init);

bool gp201_is_charging_complete(void)
{
    if (pmic_readl(REG_PMU_PD_PMU_PD_INT_RAW) & BIT_PMU_PD_PMU_PD_INT_RAW(0x8))
        return true;
    else
        return false;
}

int gp201_clear_chg_complete_state(void)
{
    return pmic_writel(REG_PMU_PD_PMU_PD_INT_CLR, BIT_PMU_PD_PMU_PD_INT_CLR(0x8));
}

static void chg_cfg_init(void)
{
    u32 val;

    val = pmic_readl(REG_PMU_PD_LINEAR_CHARGER_REG0);
    val &= ~ BIT_PMU_PD_REG2_CHARGER_18(0xFF);
    val |= BIT_PMU_PD_REG2_CHARGER_18(0x72);
    pmic_writel(REG_PMU_PD_LINEAR_CHARGER_REG0, val);
}

int gp201_cc_current_set(u32 cc_current_mA)
{
    int ret;

    if (!pchg) {
        aicx_error(0x1cd813, "s", "%s current operate handler is NULL!\n", __func__);
        ret = AIC_ENXIO;
        goto out;
    }

    if (cc_current_mA == 0) {
        ret = pmu_disable(pchg);
        goto out;
    }

    ret = pmu_current_set(pchg, cc_current_mA * 1000, PMIC_CURRENT_TOLERANCE);
    if (ret) {
        aicx_warn(0x153865, "s", "%s set cc currnet fail!\n", __func__);
        goto out;
    }

    if (!pmu_is_enabled(pchg))
        ret = pmu_enable(pchg);

out:
    return ret;
}

int gp201_cc_current_get(u32 *cc_current_mA)
{
    int ret;
    u32 cc_current_uA;

    if (!pchg) {
        aicx_error(0x1cd813, "s", "%s current operate handler is NULL!\n", __func__);
        ret = AIC_ENXIO;
        goto out;
    }

    ret = pmu_current_get(pchg, &cc_current_uA);
    if (ret) {
        aicx_warn(0x0b1bb1, "s", "%s get cc currnet fail!\n", __func__);
        goto out;
    }

    *cc_current_mA = cc_current_uA / 1000;

out:
    return ret;
}

bool gp201_is_charging_enabled(void)
{
    u32 val;
    bool ret;

    val = pmic_readl(REG_PMU_RTC_PMU_RTC_EB_REG04);
    if (val)
        ret = false;
    else
        ret = true;

    return ret;
}

int __gp201_charging_enable(bool enabled)
{
    int ret;

    if (enabled) {
        ret = pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG04, 0);
    } else {
        ret = pmic_writel(REG_PMU_RTC_PMU_RTC_EB_REG04, BIT_PMU_RTC_PD_CHRG_18R_REG_EB | BIT_PMU_RTC_PD_CHRG_18R_REG);
    }

    return ret;
}

struct aic_psy_ops gp201_psy_ops = {
    .psy_wrap_is_charging_complete = gp201_is_charging_complete,
    .psy_wrap_clear_chg_complete_state = gp201_clear_chg_complete_state,
    .psy_wrap_cc_current_set = gp201_cc_current_set,
    .psy_wrap_cc_current_get = gp201_cc_current_get,
    .psy_wrap_is_charging_enabled = gp201_is_charging_enabled,
    .__psy_wrap_charging_enable = __gp201_charging_enable,
};

bool gp202_is_charging_complete(void) {return false;}
int gp202_clear_chg_complete_state(void) {return 0;}
int  gp202_cc_current_set(u32 cc_current_mA) {return 0;}
int gp202_cc_current_get(u32 *cc_current_mA) {return 0;}
bool gp202_is_charging_enabled(void) {return true;}
int __gp202_charging_enable(bool enabled) {return 0;}

struct aic_psy_ops gp202_psy_ops = {
    .psy_wrap_is_charging_complete = gp202_is_charging_complete,
    .psy_wrap_clear_chg_complete_state = gp202_clear_chg_complete_state,
    .psy_wrap_cc_current_set = gp202_cc_current_set,
    .psy_wrap_cc_current_get = gp202_cc_current_get,
    .psy_wrap_is_charging_enabled = gp202_is_charging_enabled,
    .__psy_wrap_charging_enable = __gp202_charging_enable,
};

bool psy_is_charging_complete(void)
{
    return cur_psy_ops->psy_wrap_is_charging_complete();
}

int psy_clear_chg_complete_state(void)
{
    return cur_psy_ops->psy_wrap_clear_chg_complete_state();
}

int psy_cc_current_set(u32 cc_current_mA)
{
    return cur_psy_ops->psy_wrap_cc_current_set(cc_current_mA);
}

int psy_cc_current_get(u32 *cc_current_mA)
{
    return cur_psy_ops->psy_wrap_cc_current_get(cc_current_mA);
}

bool psy_is_charging_enabled(void)
{
    return cur_psy_ops->psy_wrap_is_charging_enabled();
}

int __psy_charging_enable(bool enabled)
{
    return cur_psy_ops->__psy_wrap_charging_enable(enabled);
}

int psy_ops_init(void)
{
    int ret =0, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) == 0x202)
        cur_psy_ops = &gp202_psy_ops;
    else
        cur_psy_ops = &gp201_psy_ops;

    return ret;
}
AIC_DRV_P1_INIT_EXPORT(psy_ops_init);

int gp201_psy_charger_register(void)
{
    int ret, pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    if (((pmic_id >> 16) & 0xFFFF) != 0x201)
        return 0;

    pchg = pmu_get("psy_chg", "chgr");
    if (!pchg) {
        ret = AIC_ENXIO;
        aicx_warn(0x196032, "s", "%s get chg operate handle failed!\n", __func__);
        goto out;
    }

    chg_cfg_init();

out:
    return ret;
}
AIC_DRV_P2_INIT_EXPORT(gp201_psy_charger_register);
