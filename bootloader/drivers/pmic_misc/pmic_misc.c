/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the pmic misc init config.
*/
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>
#include <console/console.h>
#include <initcall.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <boot_mode.h>
#include <adc.h>
#include <timer.h>
#include <pmic_misc_cfg.h>
#include <charger.h>
#include <platform/aic_cmdline.h>
#include <lpr.h>
#include <pmic_efuse.h>

#define PMU_UPS_INT_EN          0x438
#define PMU_UPS_INT_CLR         0x43C
#define PMU_PD_INT_EN           0x80b4
#define PMU_PD_INT_CLR          0x80b8
#define PWR_OFF_RST_FLAG_CLR    0x3c
#define PWR_ON_RST_FLAG         0x38
#define PWRKEY_PRESS_CNT_REG    0x430

#define LONG_PRESS_MAX_TIMING  14
#define GP201_SUB_VERSION_AB 0x4

extern int pmic_status_reg_init(void);

u32 pmic_id_main, pmic_id_sub;

void get_pmic_id(void)
{
    u32 reg = 0;

    reg = pmic_readl(REG_GLB_PD_CHIP_ID_VERSION);
    pmic_id_main = ((reg >> 16) & 0xFFFF);
    pmic_id_sub = BIT_GLB_PD_CHIP_ID_SUB_VERSION(reg);
}


#ifdef CONFIG_BL
#define PWR_PRESS_CNT        (PWR_KEY_ANTI_SHAKE_TIME_MS * 1000) / 250
#define BOOT_SRC_PWR_KEY     0x1

void pwk_key_anti_shake(void)
{
    u32 press_cnt_old = 0, press_cnt_new = 0;
    u32 boot_mode;
    u32 pwr_on_rst_flag = REG_GLB_UPS_PWR_ON_RST_FLAG, lpr_press_cnt = REG_PMU_RTC_PMU_FSM_TRIGGER_REG2,
        pwr_on_rst_flag_clr = REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR, pwr_int_clr = REG_PMU_RTC_PMU_UPS_INT_CLR;

    if (pmic_id_main == 0x202) {
        pwr_on_rst_flag = PWR_ON_RST_FLAG;
        lpr_press_cnt = PWRKEY_PRESS_CNT_REG;
        pwr_on_rst_flag_clr = PWR_OFF_RST_FLAG_CLR;
        pwr_int_clr = PMU_UPS_INT_CLR;
    }

    boot_mode = pmic_readl(pwr_on_rst_flag);
    if (boot_mode != BOOT_SRC_PWR_KEY)
        return;

    while(press_cnt_new < PWR_PRESS_CNT) {
        press_cnt_new = pmic_readl(lpr_press_cnt);
        if (press_cnt_new == press_cnt_old) {
            pmic_writel(pwr_on_rst_flag_clr, 0x1);
            bl_shutdown();
        } else {
            press_cnt_old = press_cnt_new;
        }
        aic_mdelay(10);
    }
    pmic_writel(pwr_int_clr, BIT_PMU_RTC_PMU_UPS_INT_CLR(0x1));
}

static u32 is_usb_plugin(void)
{
    if (pmic_id_main == 0x201)
        return pmic_readl(REG_PMU_RTC_PMU_INTER_RTC_REG1) & BIT_PMU_RTC_CHG_INT_18R_REG;
    else if (pmic_id_main == 0x202)
        return pmic_readl(0x104) & BIT_4;
    else
        return 0;
}

static u32 pmic_pwr_flag_clr(void)
{
    if (pmic_id_main == 0x201)
        pmic_writel(REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR, 0x1);
    else if (pmic_id_main == 0x202)
        pmic_writel(0x3c, 0x1);
}

static void startup_vbat_judgment(void)
{
    u32 vbat_val;

    if (is_usb_plugin()) {
        log_print("usb plug-in, ignore vbat judgment.\n");
    } else {
        vbat_val = vbat_vol_mv_get();

        if (vbat_val < VBAT_SHUTDOWN_VAL_MV && vbat_val != 0) {
            log_print("startup vbat value:%d mv too low, shutdown!\n", vbat_val);
            pmic_pwr_flag_clr();
            bl_shutdown();
        } else {
            log_print("startup vbat value:%d mv\n", vbat_val);
        }
    }
}
#endif

static void rf_pa_power_init(void)
{
    if (pmic_id_main == 0x201) {
        /*vddpa output voltage = APT * 2.5*/
        pmic_writel(0x8a4C,0x0600);
        pmic_writel(0x8a44,0xDF610);
    } else if (pmic_id_main == 0x202) {
        pmic_writel(0x815C, 0x21D12010);
        pmic_writel(0x4238, 0x0);//enable VDDRF2V8
        pmic_writel(0x424c,0x1);//# update ldo settings
        pmic_writel(0x8158, 0x2);
    }
}

static void pmic_abnormal_cfg(void)
{
    pmic_writel(REG_PMU_RTC_PMU_UVLO_TRIG_REG, BIT_PMU_RTC_EN_UVLO_VBAT_REG);
    pmic_writel(REG_PMU_RTC_PMU_OVLO_TRIG_REG, BIT_PMU_RTC_EN_OVLO_VBAT_REG);
    pmic_writel(REG_PMU_RTC_PMU_OTP_TRIG_REG, BIT_PMU_RTC_EN_OT_DET_REG);
}

void pmic_int_init(void)
{
    if (pmic_id_main == 0x201) {
        pmic_writel(REG_PMU_RTC_PMU_RTC_INT_EN, 0);
        pmic_writel(REG_PMU_RTC_PMU_UPS_INT_EN, 0);
        pmic_writel(REG_PMU_PD_PMU_PD_INT_EN, 0);

        pmic_writel(REG_PMU_RTC_PMU_RTC_INT_CLR, 0xFF);
        pmic_writel(REG_PMU_RTC_PMU_UPS_INT_CLR, 0xFF);
        pmic_writel(REG_PMU_PD_PMU_PD_INT_CLR, 0xFF);
    } else if (pmic_id_main == 0x202){
        pmic_writel(PMU_UPS_INT_EN, 0X0);
        pmic_writel(PMU_PD_INT_EN, 0x0);

        pmic_writel(PMU_UPS_INT_CLR, 0xFF);
        pmic_writel(PMU_PD_INT_CLR, 0xFF);
    }
}

void pmic_pmu_oc_sc_init(void)
{
    pmic_writel(REG_PMU_PD_BUCK_DCDC0_REG0, pmic_readl(REG_PMU_PD_BUCK_DCDC0_REG0) | BIT(2));//BUCK0 OC
    pmic_writel(REG_PMU_PD_BUCK_DCDC1_REG0, pmic_readl(REG_PMU_PD_BUCK_DCDC1_REG0) | BIT(2));//BUCK1 OC
    pmic_writel(REG_PMU_PD_BUCK_DCDC2_REG0, pmic_readl(REG_PMU_PD_BUCK_DCDC2_REG0) | BIT(2));//BUCK2 OC
    pmic_writel(REG_PMU_PD_LDO1_REG,        pmic_readl(REG_PMU_PD_LDO1_REG)        | BIT(18) | BIT(19));//LDO1 OC & SC
    pmic_writel(REG_PMU_PD_LDO2_REG,        pmic_readl(REG_PMU_PD_LDO2_REG)        | BIT(18) | BIT(19));//LDO2 OC & SC
    pmic_writel(REG_PMU_PD_LDO3_REG,        pmic_readl(REG_PMU_PD_LDO3_REG)        | BIT(18) | BIT(19));//LDO3 OC & SC
    pmic_writel(REG_PMU_PD_LDO4_REG,        pmic_readl(REG_PMU_PD_LDO4_REG)        | BIT(18) | BIT(19));//LDO4 OC & SC
    pmic_writel(REG_PMU_PD_LDO5_REG,        pmic_readl(REG_PMU_PD_LDO5_REG)        | BIT(18));//LDO5 OC
    pmic_writel(REG_PMU_PD_LDO6_REG,        pmic_readl(REG_PMU_PD_LDO6_REG)        | BIT(16));//LDO6 OC
    pmic_writel(REG_PMU_PD_LDO7_REG,        pmic_readl(REG_PMU_PD_LDO7_REG)        | BIT(16));//LDO7 OC
    pmic_writel(REG_PMU_PD_LDO8_REG,        pmic_readl(REG_PMU_PD_LDO8_REG)        | BIT(16));//LDO8 OC
    pmic_writel(REG_PMU_PD_LDO9_REG,        pmic_readl(REG_PMU_PD_LDO9_REG)        | BIT(18));//LDO9 OC
    pmic_writel(REG_PMU_PD_LDO10_REG,       pmic_readl(REG_PMU_PD_LDO10_REG)       | BIT(18));//LDO10 OC
    pmic_writel(REG_PMU_PD_LDO11_REG,       pmic_readl(REG_PMU_PD_LDO11_REG)       | BIT(16));//LDO11 OC
    pmic_writel(REG_PMU_PD_LDO12_REG,       pmic_readl(REG_PMU_PD_LDO12_REG)       | BIT(18));//LDO12 OC
    pmic_writel(REG_PMU_PD_LDO13_REG,       pmic_readl(REG_PMU_PD_LDO13_REG)       | BIT(16));//LDO13 OC
    pmic_writel(REG_PMU_PD_LDO14_REG,       pmic_readl(REG_PMU_PD_LDO14_REG)       | BIT(16));//LDO14 OC
}

static void enable_ac_ldo13_ocp(void)
{
    u32 reg;

    reg = pmic_readl(REG_PMU_PD_LDO13_REG);
    reg &= ~ BIT(16);
    pmic_writel(REG_PMU_PD_LDO13_REG, reg);
}

#define INTLDO0_BUCK1_CALI_BLK      0x1
#define EFUSE2INTLDO0_TRIM(x)       ((x & GENMASK(13, 18)) >> 13)
#define EFUSE2BUCK1_TRIM(x)         ((x & GENMASK(25, 30)) >> 25)
#define INTLDO0_REG                 0xb0
#define INTLDO0_VOUT_TRIM_REG(x)    (((x) & 0x3f) << 4)
#define BUCK1_CFG_REG2              0x8138
#define BUCK1_VOUT_TRIM_REG(x)      (((x) & 0x3f) << 16)

static void gp202_regu_cali(void)
{
    u32 blk_val, trim_val, val;
    int ret;

    ret = aic_pmic_efuse_block_read(INTLDO0_BUCK1_CALI_BLK, &blk_val);
    if (ret)
        return;

    trim_val = EFUSE2INTLDO0_TRIM(blk_val);
    if(trim_val) {
        val = pmic_readl(INTLDO0_REG);
        val &= ~INTLDO0_VOUT_TRIM_REG(0x3f);
        val |= INTLDO0_VOUT_TRIM_REG(trim_val);
        pmic_writel(INTLDO0_REG, val);
    }

    trim_val = EFUSE2BUCK1_TRIM(blk_val);
    if(trim_val) {
        val = pmic_readl(BUCK1_CFG_REG2);
        val &= ~BUCK1_VOUT_TRIM_REG(0x3f);
        val |= BUCK1_VOUT_TRIM_REG(trim_val);
        pmic_writel(BUCK1_CFG_REG2, val);
    }
}

void gp202_misc_init(void)
{
    u32 val;

    pmic_writel(0x423c, pmic_readl(0x423c) & ~BIT(0));

    /*BUCK0/1 ripple wave optimize.*/
    val = pmic_readl(0x8114);
    val &= ~GENMASK(24, 27);
    val |= 0x8 << 24;
    pmic_writel(0x8114, val);

    val = pmic_readl(0x8118);
    val &= ~GENMASK(0, 7);
    val |= 0x1D;
    pmic_writel(0x8118, val);

    val = pmic_readl(0x8134);
    val &= ~GENMASK(24, 27);
    val |= 0x8 << 24;
    pmic_writel(0x8134, val);

    val = pmic_readl(0x8138);
    val &= ~GENMASK(0, 7);
    val |= 0x17;
    pmic_writel(0x8138, val);

    /*OVLO*/
    pmic_writel(0x410, 0x1);
    pmic_writel(0xdc, 0x0);
    pmic_writel(0x3f4, 0xfffe0000);
    val = pmic_readl(0x81a0);
    val &= ~GENMASK(10, 11);
    val |= 0x2 << 10;
    pmic_writel(0x81a0, val);

    /*UVLO*/
    pmic_writel(0x40C, 0x1);

    /* ldo0 current limit threshold scaling to 300mA*/
    val = pmic_readl(0x8154);
    val &= ~GENMASK(12, 13);
    val |= 0x3 << 12;
    pmic_writel(0x8154, val);

    /*gp202 ldo & dcdc calibrate*/
    gp202_regu_cali();
}

void pmic_compatible(void)
{
    u32 reg;
    reg = pmic_readl(REG_GLB_PD_CHIP_ID_VERSION);
    log_print("pmic id:0x%x\n", reg);
#if CONFIG_CMDLINE
    aic_cmdline_add_key_int(cl_key_pmic_version, reg);
#endif

    if (((reg >> 16) & 0xFFFF) == 0x202) {
        gp202_misc_init();
    } else {
        if (BIT_GLB_PD_CHIP_ID_SUB_VERSION(reg) > GP201_SUB_VERSION_AB) {
            pmic_writel(REG_PMU_RTC_PMU_LDO5_CTRL_REG, pmic_readl(REG_PMU_RTC_PMU_LDO5_CTRL_REG) | BIT_PMU_RTC_LDO5_POWER_DOWN_REG);
            enable_ac_ldo13_ocp();
        }
    }
}

#define GP202_PWR_OFF_RST_FLAG_CLR  0x3C
#define GP202_PWR_KEY_PRESS_CNT     0x430
#define GP202_PWR_KEY_INT_CLR       0X43C

#define PWR_LPR_TIMING_MIN 2
#define PWR_LPR_TIMING_MAX 14
#define PWR_LPR_TIMING     8

struct lpr_phy_ops {
    int (*p_lpr_phy_config)(BOOL enabled, u32 effect_time_s);
};

#define LONG_PRESS_MAX_TIMING  14

struct lpr_phy_ops *cur_lpr_phy_ops = NULL;
static int lpr_max_timing = LONG_PRESS_MAX_TIMING;

static u16 pmic_pwr_key_int_clr = REG_PMU_RTC_PMU_UPS_INT_CLR,
           pmic_pwr_key_press_cnt = REG_PMU_RTC_PMU_FSM_TRIGGER_REG2,
           pmic_pwr_off_rst_flag_clr = REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR;

u32 press_cnt_old = 0, press_cnt_new = 0;

int gp201_lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    u32 reg;
    u32 press_timing;

    reg = pmic_readl(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1);

    if (!enabled) {
        reg &= ~BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG;
        goto out;
    }
    reg |= BIT_PMU_RTC_EN_PWRKEY_SHUTDOWN_REG;
    reg &= ~BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(0xf);

    /*PWRKEY longpress reset timing*/
    if (effect_time_s > LONG_PRESS_MAX_TIMING)
        press_timing = LONG_PRESS_MAX_TIMING;
    else
        press_timing = effect_time_s;

    reg |= BIT_PMU_RTC_PWRKEY_SHUTDOWN_CNT_REG(press_timing / 2);

#ifdef LPR_HW_RST
    reg |= BIT_PMU_RTC_PWRKEY_SHUTDOWN_MODE_REG;
#endif

out:
    pmic_writel(REG_PMU_RTC_PMU_FSM_TRIGGER_REG1, reg);

    return 0;
}

struct lpr_phy_ops gp201_lpr_phy_ops = {
    .p_lpr_phy_config = gp201_lpr_phy_config,
};

#define PWR_KEY_RST_BB_EN                0x434
#define GP202_LONG_PRESS_MAX_TIMING      30
#define GP202_PWR_KEY_SHUTDOWN_CNT_REG   0x424

int gp202_lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    u32 reg;
    u32 press_timing;

    if (!enabled) {
        pmic_writel(PWR_KEY_RST_BB_EN, 0x0);
        goto out;
    }

    /*PWRKEY longpress reset timing*/
    if (effect_time_s > lpr_max_timing)
        press_timing = lpr_max_timing;
    else
        press_timing = effect_time_s;

    reg = (press_timing / 2) & 0xF;
    pmic_writel(GP202_PWR_KEY_SHUTDOWN_CNT_REG, reg);
#ifdef LPR_HW_RST
    pmic_writel(0x428, 0x1);
#endif
    pmic_writel(PWR_KEY_RST_BB_EN, 0x1);

out:
    return 0;
}

struct lpr_phy_ops gp202_lpr_phy_ops = {
    .p_lpr_phy_config = gp202_lpr_phy_config,
};

int lpr_phy_config(BOOL enabled, u32 effect_time_s)
{
    if (cur_lpr_phy_ops)
        return cur_lpr_phy_ops->p_lpr_phy_config(enabled, effect_time_s);
    else
        return 0;
}

int lpr_ops_init(void)
{
    if (pmic_id_main == 0x202) {
        lpr_max_timing = GP202_LONG_PRESS_MAX_TIMING;
        cur_lpr_phy_ops = &gp202_lpr_phy_ops;
        pmic_pwr_key_int_clr = GP202_PWR_KEY_INT_CLR;
        pmic_pwr_key_press_cnt = GP202_PWR_KEY_PRESS_CNT;
        pmic_pwr_off_rst_flag_clr = GP202_PWR_OFF_RST_FLAG_CLR;
    } else {
        lpr_max_timing = LONG_PRESS_MAX_TIMING;
        cur_lpr_phy_ops = &gp201_lpr_phy_ops;
        pmic_pwr_key_int_clr = REG_PMU_RTC_PMU_UPS_INT_CLR,
        pmic_pwr_key_press_cnt = REG_PMU_RTC_PMU_FSM_TRIGGER_REG2,
        pmic_pwr_off_rst_flag_clr = REG_GLB_UPS_PWR_OFF_RST_FLAG_CLR;
    }

    press_cnt_old = pmic_readl(pmic_pwr_key_press_cnt);
    return 0;
}

#ifdef DISABLE_CHG_BOOT_LPR
#define USB_CHG_IN_BOOT                 0x18
#define GP202_PWR_RST_FLAG              0x38
void disable_chg_boot_lpr(void)
{
    u32 reg;

    if (pmic_id_main == 0x201) {
        reg = pmic_readl(REG_GLB_UPS_PWR_ON_RST_FLAG);
        if (reg & USB_CHG_IN_BOOT)
            lpr_phy_config(true, LONG_PRESS_MAX_TIMING);
    } else if (pmic_id_main == 0x202) {
        reg = pmic_readl(GP202_PWR_RST_FLAG);
        if (reg & USB_CHG_IN_BOOT)
            pmic_writel(0x434, 0x0);
    }
}
#endif

int pmic_misc(void)
{
    get_pmic_id();

    lpr_ops_init();

    lpr_phy_config(true, LONG_PRESS_RESET_TIME);

    if (pmic_id_main == 0x201) {
        pmic_pmu_oc_sc_init();
        pmic_abnormal_cfg();
    }

#ifdef DISABLE_CHG_BOOT_LPR
    disable_chg_boot_lpr();
#endif

#ifdef CONFIG_BL
extern int pmic_status_reg_init(void);
    pmic_status_reg_init();
    startup_vbat_judgment();
    pwk_key_anti_shake();
#endif
    rf_pa_power_init();
    pmic_int_init();

#ifdef CONFIG_STOP_CHARGER
    charger_force_powerdown();
#endif

    return 0;
}
core_init(pmic_misc);

void board_id_init(void)
{
#if CONFIG_CMDLINE
        aic_cmdline_add_key_int(cl_key_boardid, -1);
#endif
}

int pmic_misc_early(void)
{
#ifndef CONFIG_MHA
    board_id_init();
#endif
    pmic_compatible();
    return 0;
}
early_init(pmic_misc_early);

#ifdef CONFIG_LPR_DISABLE
enum __lpr_state {
    LPR_MIN     = 0,
    LPR_DEFAULT = 1,
    LPR_MAX     = 2,
};
enum __lpr_state lpr_state = LPR_DEFAULT;

int lpr_disable_kick(void)
{
    press_cnt_new = pmic_readl(pmic_pwr_key_press_cnt);
    if (press_cnt_new == press_cnt_old) {
        /* either key is released or key pressed timeout (65.536s) */
            lpr_phy_config(true, PWR_LPR_TIMING_MAX);
            lpr_state = LPR_MAX;
    } else {
    /* escape LPR reset by setting compare value up and down */
        if ((press_cnt_new < (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MAX)) {
            lpr_phy_config(true, PWR_LPR_TIMING_MAX);
            lpr_state = LPR_MAX;
        } else if ((press_cnt_new >= (PWR_LPR_TIMING * 4000)) && (lpr_state != LPR_MIN)) {
            lpr_phy_config(true, PWR_LPR_TIMING_MIN);
            lpr_state = LPR_MIN;
        }
        press_cnt_old = press_cnt_new;
    }
}

void lpr_disable_loop(void)
{
    while(1) {
        lpr_disable_kick();
        aic_mdelay(10);
    };
}
#else
int lpr_disable_kick(void) {return 0;}
void lpr_disable_loop(void){}
#endif
