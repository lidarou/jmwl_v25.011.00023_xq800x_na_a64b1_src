/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Pmic clock init.
 */

#include <aic_type.h>
#include <aic_io.h>
#include <console/console.h>
#include <reg_base.h>
#include <cache.h>
#include <initcall.h>
#include <timer.h>
#include <pmic_reg_base.h>
#include <pmic_io.h>
#include <pmic_clk_cfg.h>
#include <glb_rtc.h>
#include <glb_ups.h>
#include <glb_pd.h>
#include <pmu_pd.h>
#include <pmu_rtc.h>
#include <platform/aic_cmdline.h>
#include "platform/boot.h"

#include <clk/clk.h>

static void set_26m_enter_lp(void)
{
    u32 val;

    /* decrease current from 1ma to 500ua */
    pmic_writel(0x614, 1);
    aic_udelay(100);
    pmic_writel(0x614, 0);
    aic_udelay(100);

    /* read pmic chip id */
    val = pmic_readl(0x8000);
    val = val & 0xffff;
    if (val > 4) {
        /* set LDO13 as 1.76v */
        val = pmic_readl(0x8a7c);
        val &= ~(0x7ff << 8);
        val |= ((0x646 & 0x7ff) << 8);
        pmic_writel(0x8a7c, val);
        aic_mdelay(1);
        /* set 26M work at low power in normal and deep, now hardware */
        /*val = pmic_readl(REG_GLB_RTC_REG_XC26MLPM_18);
        val |= BIT_GLB_RTC_XC26MLPM_NORM_18_REG | BIT_GLB_RTC_XC26MLPM_DEEP_18_REG;
        pmic_writel(REG_GLB_RTC_REG_XC26MLPM_18, val);
        aic_udelay(100);*/
    } else {
        /* set LDO13 as 1.76v */
        val = pmic_readl(0x8a7c);
        val &= ~(0x7ff << 8);
        val |= ((0x746 & 0x7ff) << 8);
        pmic_writel(0x8a7c, val);
    }
}
int gp201_clk_init(enum CLK_32K_SRC_SEL clk_32k_src_sel)
{
    u32 val, val2;

    val = pmic_readl(0x8000);
    if (((val >> 16) & 0xFFFF) == 0x202) {
        /*to be add */
        return 0;
    }
    if ((val & 0xffff) >= 5) {
        val = pmic_readl(0x238);
        val |= 1;
        pmic_writel(0x238, val);
    }
    /* enable rc32k in normal, deep & ups mode */
    pmic_writel(0x658, BIT(1) | BIT(0));
    aic_mdelay(10);

    switch (clk_32k_src_sel) {
    case TCXO_EXTERNAL_32K:
        pmic_writel(0x64c, BIT(0));
        val = pmic_readl(0x8000);
        if ((val & 0xffff) >= 5) {
            /* select tcxo+32k crystal */
            val = pmic_readl(0x238);
            val |= 2;
            pmic_writel(0x238, val);
            /* delay some time to wait crystal stable */
            aic_mdelay(4);
        }

    case TCXO_32K:
        if (clk_32k_src_sel == TCXO_32K)
            pmic_writel(0x654, 0xae);

        pmic_writel(0x64c, BIT(0));
        /* LDO13 must be enabled in deep */
        pmic_writel(0x96c, 0x40);
        /* DCXO should shutdown at deep and ups mode */
        pmic_writel(0x908, 0);
        /* normal sel xc32k, shutdown sel xc32k */
        val = 0x5;
        pmic_writel(0x65c, val);
        break;

    case DCXO_EXTERNAL_32K:
        pmic_writel(0x64c, BIT(0));
        val = pmic_readl(0X8000);
        if ((val & 0xffff) >= 5) {
            /* select tcxo+32k crystal */
            val = pmic_readl(0x238);
            val |= 2;
            pmic_writel(0x238, val);
            /* delay some time to wait crystal stable */
            aic_mdelay(4);
        }

    case DCXO_32K:
        if (clk_32k_src_sel == DCXO_32K)
            pmic_writel(0x654, 0xae);

        pmic_writel(0x64c, BIT(0));
        /* delay some time to wait crystal stable */
        aic_mdelay(4);
        /* normal sel xc32k, shutdown sel xc32k */
        pmic_writel(0x65c, 0x5);
        /* LDO13 must be enabled in deep */
        pmic_writel(0x96c, 0x40);
        /* DCXO should shutdown at deep and ups mode */
        pmic_writel(0x908, 0);
        set_26m_enter_lp();
        break;

    case DCXO_32KLESS:
        /* normal sel 32k-less, shutdown sel rco32k */
        pmic_writel(0x65c, 0x2);
        pmic_writel(0x96c, 0x80);
        pmic_writel(0x908, BIT(1));
        pmic_writel(0x64c, ~BIT(0));
        set_26m_enter_lp();
        break;

    default:
        /* normal sel 32k-less, shutdown sel rco32k */
        pmic_writel(0x65c, 0);
        /* DCXO should shutdown at deep and ups mode */
        pmic_writel(0x908, 0);
        /* disable 32kless gate for low power */
        pmic_writel(0x690, 0);
        /* LDO13 can be closed in deep */
        pmic_writel(0x96c, 0x40);
        pmic_writel(0x64c, ~BIT(0));
        set_26m_enter_lp();
        break;
    }

    /* enable apb auto gate */
    pmic_writel(0x6f4, 0);
    pmic_writel(0x80e0, 0);
    pmic_writel(0x8034, 1);
    val = pmic_readl(0x8a1c);
    val |= BIT(20);
    pmic_writel(0x8a1c, val);
    pmic_writel(0x624, BIT(0));
    pmic_writel(0x620, 0x1);

    /* switch to hw cali to avoid warm reset's wrong vlue */
    pmic_writel(0x664, (pmic_readl(0x664) & ~BIT(8)));
    pmic_writel(0x668, (pmic_readl(0x668) & ~BIT(12)));

    /* enable calibrate clock */
    pmic_writel(0x694, BIT(0));
    pmic_writel(0x698, BIT(0));

    /* enable ctune calibration */
    pmic_writel(0x22c, BIT(1));

    /* config cali delta and timeout num */
    val = pmic_readl(0x220);
    val &= ~(0x7f << 12);
    val |= 0x4 << 12;
    val &= ~(0xff);
    val |= 0x7f;
    pmic_writel(0x220, val);

    /* enable rco cali for ups, and timeval is 1 min */
    val = pmic_readl(0x228);
    val |= BIT(16);
    val &= ~(0x3fff);
    val |= 0x3c;
    pmic_writel(0x228, val);

    if (pmic_readl(0x65c) == 0) {
        /* manual trigger */
        val = pmic_readl(0x228);
        val &= ~BIT(17);
        pmic_writel(0x228, val);
        aic_mdelay(2);
        val = pmic_readl(0x228);
        val |= BIT(17);
        pmic_writel(0x228, val);
        aic_mdelay(5);
        val = pmic_readl(0x228);
        val &= ~BIT(17);
        pmic_writel(0x228, val);

        val = pmic_readl(0x224);
        /* wait some time to read calibration data */
        aic_mdelay(10);
        while (1) {
            val = pmic_readl(0x224);
            val2 = pmic_readl(0x230);
            if (val >= val2)
                val = val - val2;
            else
                val = val2 - val;
            val2 = pmic_readl(0x22c);
            if (val2 & BIT(0)) {
                log_print("pmic_clk: rc32k calibration failed!\n");
                break;
            }
            if (val < 4) {
                log_print("pmic_clk: rc32k calibration success!\n");
                break;
            }
        }
    }
    /*pmic_writel(REG_GLB_RTC_CKM_EN_CLK_RTC_CAL_26M, ~BIT_GLB_RTC_CKM_EN_GLB_CLK_RTC_CALIBRE_26M_RF);
    pmic_writel(REG_GLB_RTC_CKM_EN_CLK_RTC_CAL_32K, ~BIT_GLB_RTC_CKM_EN_GLB_CLK_RTC_CALIBRE_32K_RF);*/

    return 0;
}

void aic_gp202_rc32k_sw_cali_trigger(void)
{
    u32 val;

    /* switch to hw cali mode */
    pmic_writel(0x54, pmic_readl(0x54) & ~BIT(12));
    pmic_writel(0x58, pmic_readl(0x58) & ~BIT(12));

    /* switch to hw cali logic */
    val = pmic_readl(0x23c);
    val |= 0x4;
    pmic_writel(0x23c, val);

    val = pmic_readl(0x238);
    val &= ~BIT(17);
    pmic_writel(0x238, val);
    aic_udelay(500);
    val = pmic_readl(0x238);
    val |= BIT(17);
    pmic_writel(0x238, val);
    aic_udelay(500);
    val = pmic_readl(0x238);
    val &= ~BIT(17);
    pmic_writel(0x238, val);
}

int aic_gp202_rc32k_sw_cali_check(int delta)
{
    int ret;
    u32 val, val2, tmp;

    while(1) {
        val = pmic_readl(0x234);
        val2 = pmic_readl(0x240);
        log_print("rc32k golden value 0x%x, result: 0x%x\n", val, val2);
        if (val > val2)
            val = val - val2;
        else
            val = val2 - val;
        log_print("rc32k delta value 0x%x\n", val);
        val2 = pmic_readl(0x23c);
        if (val2 & BIT(0)) {
            log_print("rc32k calibration timeout!\n");
            log_print("rc32k int status 0x%x",  pmic_readl(0x244));
            tmp = pmic_readl(0x244);
            tmp |= 0x4;
            pmic_writel(0x244, tmp);
            tmp &= ~0x4;
            pmic_writel(0x244, tmp);
            ret = -1;
            break;
        }
        if (val2 & BIT(1)) {
            log_print("rc32k calibration success!\n");
            ret = 0;
            break;
        }
        aic_udelay(100);
    }

    /* switch to sw cali mode */
    pmic_writel(0x54, pmic_readl(0x54) | BIT(12));
    pmic_writel(0x58, pmic_readl(0x58) | BIT(12));
    pmic_writel(0x23c, pmic_readl(0x23c) & ~BIT(2));

    return ret;
}

int aic_gp202_rc32k_clk_cali(u32 param)
{
    int ret;
    u32 val;

    /* enable hw calibration function */
    val = pmic_readl(0x238);
    val |= BIT(16);
    /* configure cali interval to 1min */
    val &= ~0x3fff;
    val |= 0x258;
    pmic_writel(0x238, val);

    /* disable hw cali in normal and deep */
    val = pmic_readl(0x248);
    val &= ~3;
    pmic_writel(0x248, val);

    /* enable calibrantion clock */
    val = pmic_readl(0x168);
    val |= 0x1;
    pmic_writel(0x168, val);

    /* 2.6ms hw cali */
    val = pmic_readl(0x230);
    val = 0x050083ff;
    pmic_writel(0x230, val);
    val = pmic_readl(0x24c);
    val &= ~0x3;
    pmic_writel(0x24c, val);
    val = pmic_readl(0x234);
    val = 0x1045a;
    pmic_writel(0x234, val);

    aic_gp202_rc32k_sw_cali_trigger();
    ret = aic_gp202_rc32k_sw_cali_check(128);

    /* modify cali time 82ms */
    val = pmic_readl(0x24c);
    val &= ~3;
    val |= 0x3;
    pmic_writel(0x24c, val);

    /* init margin cnt */
    val = pmic_readl(0x230);
    val = 0x0f0803ff;
    pmic_writel(0x230, val);

    /* set golden value */
    val = pmic_readl(0x234);
    val = 0x208b4c;
    pmic_writel(0x234, val);
    /* enable timeout interrupt, ToDo not use timeout now */

    /* hw auto cali */
    /*val = pmic_readl(REG_GLB_UPS_RC32K_CALIBRE_EN_RF);
      val |= (BIT_GLB_UPS_RC32K_CALIBRE_EN_NORM_RF | BIT_GLB_UPS_RC32K_CALIBRE_EN_DEEP_RF);
      pmic_writel(REG_GLB_UPS_RC32K_CALIBRE_EN_RF, val);*/

    /* manual trigger */
    aic_gp202_rc32k_sw_cali_trigger();
#if 0
    ret = aic_gp202_rc32k_sw_cali_check(128);

    /* set cali step to 2.6ms for system normal run */
    val = pmic_readl(0x230);
    val = 0x0f0083ff;
    pmic_writel(0x230, val);
    val = pmic_readl(0x24c);
    val &= ~0x3;
    pmic_writel(0x24c, val);
    val = pmic_readl(0x234);
    val = 0x1045a;
    pmic_writel(0x234, val);
#endif
    return ret;
}
void gp202_26m_type_configure(enum CLK_32K_SRC_SEL clk_32k_src_sel)
{
    u32 val;
    u32 is_dcxo;

#if 0
    /* enable xc26m_en */
    pmic_writel(0x4008, 0x1);
    aic_mdelay(0x10);
#endif

    /* enable XC26M osc */
    pmic_writel(0xa4, 0x01013301);

    /* enable xc26m_x2_pullup */
    pmic_writel(0xac, 0x01010400);
    aic_mdelay(0x10);

    /* get 26m type flag: bit[6]: 1 dcxo; 0 tcxo */
    val = pmic_readl(0x104);
    if (val & 0x40) {
        pmic_writel(0xa4, 0x01013301);
        aic_mdelay(0x10);
        pmic_writel(0xac, 0x00010400);
        aic_mdelay(0x10);
        log_print("auto detect dcxo as 26m source!\n");

        pmic_writel(0x4248, 0x1);
        pmic_writel(0x424c, 0x1);

        is_dcxo = 1;
    } else {
        pmic_writel(0xa4, 0x01013111);
        aic_mdelay(0x10);
        pmic_writel(0xac, 0x00010400);
        aic_mdelay(0x10);
        log_print("auto detect tcxo as 26m source!\n");
        if (clk_32k_src_sel != TCXO_32KLESS) {
            pmic_writel(0x4248, 0x10);
            pmic_writel(0x424c, 0x1);
        }

        is_dcxo = 0;
    }

#ifdef CLK_FIX_26M_TYPE
    switch (clk_32k_src_sel) {
    case TCXO_32K:
    case TCXO_EXTERNAL_32K:
    case TCXO_RC32K:
        pmic_writel(0x4248, 0x10);
        pmic_writel(0x424c, 0x1);

        is_dcxo = 0;

        break;

    default:
        pmic_writel(0x4248, 0x1);
        pmic_writel(0x424c, 0x1);

        is_dcxo = 1;

        break;

    }
#endif

#if CONFIG_CMDLINE
    aic_cmdline_add_key_int(cl_key_dcxo, is_dcxo);
#endif

    pmic_writel(0xac, 0x00010400);
    aic_mdelay(0x10);
}
int gp202_clk_init(enum CLK_32K_SRC_SEL clk_32k_src_sel)
{
    u32 val;

    /* RC32K cpi/cmpi current, make sure RC32K jitter is good */
    pmic_writel(0x50, 0xf00631);
    /* xc26m_agc_sel */
    pmic_writel(0x88, 0x1);
    /* xc26m_ibias_ctrl_reg */
    pmic_writel(0x8c, 0x1bf);
    /* modify 26m output amplitude and phasenoise */
    pmic_writel(0xa4, 0x01013301);
    /* set wcn 32k output enable in normal state */
    val = pmic_readl(0x184);
    val |= 0x1;
    pmic_writel(0x184, val);
    /* switch gpio2 func to WCN_32K */
    //pmic_writel(0x65c, 0x1);
    /* xc32k related configure */
    pmic_writel(0x60, 0x11060111);
    aic_mdelay(10);
    /* enable rc32k in normal, deep & ups mode */
    pmic_writel(0x005c, 0x3);
    aic_mdelay(5);
    /* switch normal source to rc32k */
    pmic_writel(0x188, 0x0);
    aic_udelay(100);
    /* set 26m enter lp in deep and shutdown */
    val = pmic_readl(0x68);
    val |= 0x2;
    pmic_writel(0x68, val);

    /* read ctune/ftune to register, 0x110's value default comes from efuse */
    val = pmic_readl(0x110);
    val &= ~0xfffffe00;
    pmic_writel(0x54, val);

    val = pmic_readl(0x110);
    val &= ~0xfffc01ff;
    val >>= 9;
    pmic_writel(0x58, val);

    /* sw trigger rc32k calibration */
    aic_gp202_rc32k_clk_cali(0);
#if 0
    /* read ctune/ftune to register, 0x110's value comes from hw cali logic now */
    val = pmic_readl(0x110);
    val &= ~0xfffffe00;
    pmic_writel(0x54, BIT(12) | val);

    val = pmic_readl(0x110);
    val &= ~0xfffc01ff;
    val >>= 9;
    pmic_writel(0x58, BIT(12) | val);
#endif
    /* configure 26m depend on detect */
    gp202_26m_type_configure(clk_32k_src_sel);

    switch (clk_32k_src_sel) {
    case TCXO_32K:
        /*pmic_writel(0x4248, 0x10);
        pmic_writel(0x424c, 0x1);*/
    case DCXO_32K:
        /*if (clk_32k_src_sel == DCXO_32K) {
            pmic_writel(0x4248, 0x1);
            pmic_writel(0x424c, 0x1);
        }*/
        pmic_writel(0x64, 0xf001);
        aic_mdelay(10);
        pmic_writel(0x60, 0x11070111);
        aic_mdelay(10);

        pmic_writel(0x6c, 0);
        pmic_writel(0x164, 0);

        /* xc32k source enable */
        pmic_writel(0x158, 0x1);
        log_print("use xc32k as clock source!\n");
        /* switch to xc32k */
        pmic_writel(0x188, 0x11);

        break;

    case TCXO_EXTERNAL_32K:
        /*pmic_writel(0x4248, 0x10);
        pmic_writel(0x424c, 0x1);*/
    case DCXO_EXTERNAL_32K:
        /*if (clk_32k_src_sel == DCXO_EXTERNAL_32K) {
            pmic_writel(0x4248, 0x1);
            pmic_writel(0x424c, 0x1);
        }*/

        pmic_writel(0x64, 0xf001);
        pmic_writel(0x60, 0x11161111);
        aic_mdelay(10);
        pmic_writel(0x6c, 0);
        pmic_writel(0x164, 0);
        pmic_writel(0x158, 0);
        /* switch to xc32k */
        pmic_writel(0x188, 0x11);
        log_print("use external 32k as clock source!\n");

        break;

    /* The configure should not exist */
    /*case TCXO_32KLESS:
        pmic_writel(0x4248, 0x10);
        pmic_writel(0x424c, 0x1);*/
    case DCXO_32KLESS:
        /*if (clk_32k_src_sel == DCXO_32KLESS) {
            pmic_writel(0x4248, 0x1);
            pmic_writel(0x424c, 0x1);
        }*/

        /* Enable xc26m buffer1 in deep and normal */
        pmic_writel(0x9c, 0x3);
        /* 32kless */
        pmic_writel(0x408, 0x2);
        pmic_writel(0x6c, 0x1);
        pmic_writel(0x164, 0x1);
        pmic_writel(0x15c, 0x1);
        pmic_writel(0x188, 0x2);
        log_print("use 32kless solution!\n");

        /* disable XTAL_32K POWER */
        pmic_writel(0x64, 0xf000);
        pmic_writel(0x60, 0x11061111);

        break;

    case TCXO_RC32K:
        /*pmic_writel(0x4248, 0x10);
        pmic_writel(0x424c, 0x1);*/
    default:
        /*if (clk_32k_src_sel == DCXO_RC32K) {
            pmic_writel(0x4248, 0x1);
            pmic_writel(0x424c, 0x1);
        }*/

        /* disable XTAL_32K POWER */
        pmic_writel(0x64, 0xf000);
        pmic_writel(0x60, 0x11061111);

       /* switch normal source to rc32k */
        pmic_writel(0x188, 0x0);
        aic_udelay(100);
        log_print("use rc32k as clock source!\n");

        /* disable 32kless gate */
        pmic_writel(0x6c, 0x0);
        pmic_writel(0x164, 0x0);

        break;
    }
#if 0
    /* set the voltage of intldo0 to 1.65V to solve external 32k error check issue */
    pmic_writel(0xb0, 0x50);

    /* check 32k bound option */
    /* bit0: 1 enable XTAL_32K module and power on; bit4: 1 enable xc32k_x2_pull_up_reg */
    pmic_writel(0x64, 0xf011);
    aic_mdelay(1);
    /* bit20: 1 enable external 32k input; bit12: 1 bypass xc32k osc crystal */
    pmic_writel(0x60, 0x11161111);
    aic_mdelay(1);
    val = pmic_readl(0x104);
    /* 32k crystal */
    if (val & 0x80) {
        pmic_writel(0x64, 0xf001);
        pmic_writel(0x60, 0x11060111);
        aic_mdelay(5);
        /* enable xc32k source */
        pmic_writel(0x158, 0x1);
        aic_mdelay(1);
        /* trigger and count xc32k n*pulse */
        pmic_writel(0x8218, 0x1);
        aic_mdelay(200);
        val = pmic_readl(0x821c);
        /* crystal ok */
        if (val == 3) {
            pmic_writel(0x64, 0xf001);
            aic_mdelay(10);
            pmic_writel(0x60, 0x11060111);
            aic_mdelay(10);
            /* xc32k source enable */
            pmic_writel(0x158, 0x1);
            log_print("use xc32k as clock source!\n");
            /* switch to xc32k */
            pmic_writel(0x188, 0x1);
        } else {
            /* xc32k cannot work */
            log_print("xc32k or external 32k cannot work!\n");
            while(1);
        }
    } else {
        pmic_writel(0x64, 0xf001);
        pmic_writel(0x60, 0x11161111);
        aic_mdelay(10);
        pmic_writel(0x158, 0x1);
        pmic_writel(0x8218, 0x1);
        aic_mdelay(10);
        val = pmic_readl(0x821c);
        /* external input ok */
        if (val == 3) {
            pmic_writel(0x60, 0x11161111);
            aic_mdelay(10);
            pmic_writel(0x158, 0);
            /* switch to xc32k */
            pmic_writel(0x188, 0x1);
            log_print("use external 32k as clock source!\n");
        } else {
            /* 32kless */
            if ((clk_32k_src_sel == DCXO_32KLESS) || (clk_32k_src_sel == TCXO_32KLESS)) {
                pmic_writel(0x4248, pmic_readl(0x4248) & ~BIT(4));
                pmic_writel(0x424c, 0x1);
                pmic_writel(0x408, 0x2);
                pmic_writel(0x6c, 0x1);
                pmic_writel(0x164, 0x1);
                pmic_writel(0x15c, 0x1);
                pmic_writel(0x188, 0x2);
                log_print("use 32kless solution!\n");
            } else {    /* rc32k */
                /* switch normal source to rc32k */
                pmic_writel(0x188, 0x0);
                aic_mdelay(10);
                log_print("use rc32k as clock source!\n");
            }

            /* disable XTAL_32K POWER */
            pmic_writel(0x64, 0xf000);
            pmic_writel(0x60, 0x11061111);
        }
    }

    /* set voltage of intldo0 to 1.8v */
    pmic_writel(0xb0, 0x140);
#endif

    return 0;
}
int pmic_clk_init(enum CLK_32K_SRC_SEL clk_32k_src_sel)
{
    u32 val, main_id;

    /* read chip id */
    val = pmic_readl(0x8000);
    main_id = (val >> 16) & 0xFFFF;
    if (main_id == 0x201) {
        gp201_clk_init(clk_32k_src_sel);
    } else if (main_id == 0x202) {
        gp202_clk_init(clk_32k_src_sel);
    } else {
        /* Do nothing now */
    }

    return 0;
}

