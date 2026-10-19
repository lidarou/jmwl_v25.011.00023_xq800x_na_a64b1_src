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
#include <timer.h>
#ifdef CONFIG_AIC_PMU_R2P0
#include <pmic_pinlist.h>
#include <pinlist.h>
#include <aon_pmu.h>
#include <sys_nd_glb.h>
#endif

#ifdef CONFIG_AIC_PMU_R1P0
#define BIT_PAD_SLP_TOP                 BIT_19
#define BIT_PAD_SLP_WPD                 BIT_10
#define BIT_PAD_SLP_OE                  BIT_8
#define BIT_PAD_WPD                     BIT_3
#define BIT_PAD_DRV(x)                  (((x) & 7))
#define BIT_FUNC_SEL(x)                 (((x) & 7))
#define BIT_PMIC_PAD_DRV(x)             (((x) & 1))  //PMIC PAD driver strength select
#define BIT_PMIC_PAD_PX                 0            //PMIC PAD no pull up and no pull down
#endif

static void xtl_buf_init(u32 pmic_id)
{
    if (pmic_id == 0x202) {
#ifdef CONFIG_AIC_PMU_R1P0
        /*set ext_en0 bb pin func*/
        AIC_WRITE32(0x40170054,BIT_FUNC_SEL(0));
        AIC_WRITE32(0x41220024,BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_TOP|BIT_PAD_SLP_WPD|BIT_PAD_SLP_OE);

        /*set ext_en1 bb pin func*/
        AIC_WRITE32(0x40170058,BIT_FUNC_SEL(0));
        AIC_WRITE32(0x41220028,BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_TOP|BIT_PAD_SLP_WPD|BIT_PAD_SLP_OE);

        /*set ext_en0 pmic pin func*/
        pmic_writel(0x9018,0x1);

        /*set xtl0 bound to ap/cp/dsp/modem*/
        AIC_WRITE32(0x400c0704,0x0);
        /*set xtl1 bound to ap/cp/dsp/modem*/
        AIC_WRITE32(0x400c0708,0x0);
#else
        /*set ext_en0 bb pin func*/
        AIC_WRITE32(REG_IOMUX_CLK26M_EN0,BIT_FUNC_SEL(0));
        AIC_WRITE32(REG_PG6_PAD_CTRL_CLK26M_EN0,BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_TOP|BIT_PAD_SLP_WPD|BIT_PAD_SLP_OE);

        /*set ext_en0 pmic pin func*/
        pmic_writel(GP202_IO_MUX_CLK26M_EN0,BIT_FUNC_SEL(0));
        pmic_writel(GP202_CLK26M_EN0,(1) & 3);

        /*set xtl0 bound to chipsleep*/
        //AIC_WRITE32(0x400e3048,AIC_READ32(0x400e3048)|BIT(10));
#endif
    }
#ifdef CONFIG_AIC_PMU_R1P0
    else {
        /*set ext_en0 bb pin func*/
        AIC_WRITE32(0x40170054,BIT_FUNC_SEL(0));
        AIC_WRITE32(0x41220024,BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_TOP|BIT_PAD_SLP_WPD|BIT_PAD_SLP_OE);

        /*set ext_en1 bb pin func*/
        AIC_WRITE32(0x40170058,BIT_FUNC_SEL(0));
        AIC_WRITE32(0x41220028,BIT_PAD_DRV(1)|BIT_PAD_WPD|BIT_PAD_SLP_TOP|BIT_PAD_SLP_WPD|BIT_PAD_SLP_OE);

        /*set ext_en0 pmic pin func*/
        pmic_writel(0x9048,BIT_FUNC_SEL(1));
        pmic_writel(0x9008,BIT_PMIC_PAD_DRV(1)|BIT_PMIC_PAD_PX);

        /*set ext_en1 pmic pin func*/
        pmic_writel(0x9040,BIT_FUNC_SEL(1));
        pmic_writel(0x9000,BIT_PMIC_PAD_DRV(1)|BIT_PMIC_PAD_PX);

        /*set xtl0 bound to ap/cp/dsp/modem*/
        AIC_WRITE32(0x400c0704,0x0);
        /*set xtl1 bound to ap/cp/dsp/modem*/
        AIC_WRITE32(0x400c0708,0x0);
        aic_udelay(300);             //ab version can remove this delay
        /*set 26M_buf bound to xtl0*/
        pmic_writel(0x0000061c,0x10);
        /*set xtl logic to generate 26M_buf*/
        pmic_writel(0x000009a8,0x70);

    }
#endif
}

extern int aic_pmic_efuse_block_read(int blk,u32 *data);
void slp_get_trim(u32 pmic_id)
{
#ifdef CONFIG_AIC_PMU_R2P0
    u32 intldo1_vote_efuse_val = 0;
    u32 intldo1_900mv_val = 0;
    u32 intldo1_750v_val = 0;
    u32 intldo1_700v_val = 0;
    u32 b4_val = 0;

    u32 buck0_vote_efuse_val = 0;
    u32 buck0_900mv_val = 0;
#endif

    if (pmic_id == 0x202) {
#ifdef CONFIG_AIC_PMU_R2P0
        aic_pmic_efuse_block_read(1,&intldo1_vote_efuse_val);
        intldo1_vote_efuse_val &= 0x1f80000;
        intldo1_900mv_val = intldo1_vote_efuse_val >> 16;

        if (intldo1_900mv_val > 54 || intldo1_900mv_val < 10)
            intldo1_900mv_val = 30;

        intldo1_750v_val = intldo1_900mv_val - 0xf;
        intldo1_700v_val = intldo1_750v_val - 0x5;
        b4_val = (intldo1_700v_val << 8) | (intldo1_750v_val);
        pmic_writel(0x000000b4,((pmic_readl(0x000000b4) & (~0xffff)) | b4_val));//# set intldo1 0.7v,0xf(normal 0.75v),0xa(deep 0.7v)

        aic_pmic_efuse_block_read(2,&buck0_vote_efuse_val);
        buck0_vote_efuse_val &= 0x3f00000;
        buck0_900mv_val = buck0_vote_efuse_val >> 20;

        if (buck0_900mv_val > 54 || buck0_900mv_val < 10)
            buck0_900mv_val = 30;

        pmic_writel(0x0000811c,buck0_900mv_val);//# buck0 voltage 0.9v(default)

        AIC_WRITE32(REG_SYS_ND_GLB_ND_DUMMY0,buck0_900mv_val | (b4_val << 8));
#else
        /* PMU_Normal*/
        pmic_writel(0x0000811c,0x0000a1e);//# buck0 voltage 0.9v(default)
#endif
        return;
    }
    return;
}

static void ldo_sleep_init(u32 pmic_id)
{
    if (pmic_id == 0x202) {
#ifdef CONFIG_AIC_PMU_R1P0
        //# pmic_writel(0x00004228,0x00000010);//# turn off buck0 when FSM in deepsleep,BUCK0 DEEP OFF// banyan buck0 deep turn off
        pmic_writel(0x00004228,pmic_readl(0x00004228) | BIT(5));//# turn on buck0 when FSM in deepsleep,BUCK0 DEEP ON// GX316 BUCK0 Deep turn on
        //# pmic_writel(0x0000422c,0x00000010);//# turn off buck1 when FSM in deepsleep,BUCK1 DEEP OFF//banyan buck1 deep turn off
        pmic_writel(0x0000422c,pmic_readl(0x0000422c) | BIT(5));//# turn on buck1 when FSM in deepsleep,BUCK1 DEEP ON//GX316 BUCK1 Deep turn on
        pmic_writel(0x00004230,pmic_readl(0x00004230) | BIT(4));//# turn off ldo0 when FSM in deepsleep//ldo0_bound_reg
        pmic_writel(0x00004234,pmic_readl(0x00004234) | BIT(4));//# turn off ldo1 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x00004238,pmic_readl(0x00004238) | BIT(4));//# turn off ldo2 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x0000423C,pmic_readl(0x0000423C) | BIT(4));//# turn off ldo3 when FSM in deepsleep//ldo5_bound_reg
        //pmic_writel(0x00004240,pmic_readl(0x00004240) | BIT(4));//# turn off ldo4 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x00004244,pmic_readl(0x00004244) | BIT(4));//# turn off ldo5 when FSM in deepsleep//ldo5_bound_reg
        //pmic_writel(0x00004248,pmic_readl(0x00004248) | BIT(4));//# turn off ldo6 when FSM in deepsleep//ldo6_bound_reg

        pmic_writel(0x424c,0x1);//# update ldo settings
#else
        //# pmic_writel(0x00004228,0x00000010);//# turn off buck0 when FSM in deepsleep,BUCK0 DEEP OFF// banyan buck0 deep turn off
        pmic_writel(0x00004228,pmic_readl(0x00004228) | BIT(4));//# turn on buck0 when FSM in deepsleep,BUCK0 DEEP ON// GX316 BUCK0 Deep turn on
        //# pmic_writel(0x0000422c,0x00000010);//# turn off buck1 when FSM in deepsleep,BUCK1 DEEP OFF//banyan buck1 deep turn off

        pmic_writel(0x000000f4,0x10);//# turn on sw1 when deepsleep
#ifdef CONFIG_BUCK1_ON
        pmic_writel(0x000000f8,0x3);//# turn on sw1 when deepsleep
        pmic_writel(0x000000fc,0x3);//# turn on sw1 when deepsleep
        pmic_writel(0x000000ec,0x0);//# turn on sw0 when deepsleep
        pmic_writel(0x000000f0,0x0);//# turn on sw1 when deepsleep
        //pmic_writel(0x0000422c,pmic_readl(0x0000422c) | BIT(4));//# turn on buck1 when FSM in deepsleep,BUCK1 DEEP ON//GX316 BUCK1 Deep turn on
#else
        pmic_writel(0x000000f8,0x10);//# turn on sw1 when deepsleep
        pmic_writel(0x000000fc,0x10);//# turn on sw1 when deepsleep
        pmic_writel(0x000000ec,0x2);//# turn on sw0 when deepsleep
        pmic_writel(0x000000f0,0x2);//# turn on sw1 when deepsleep
        pmic_writel(0x0000422c,pmic_readl(0x0000422c) | BIT(4));//# turn on buck1 when FSM in deepsleep,BUCK1 DEEP ON//GX316 BUCK1 Deep turn on
#endif
        pmic_writel(0x000000b8,0x3);//# switch when deepsleep

        pmic_writel(0x0000002b4,pmic_readl(0x0000002b4) | BIT(0));//# buck0 fast pwr on
        pmic_writel(0x0000002b8,pmic_readl(0x0000002b8) | BIT(0));//# buck1 fast pwr on

        pmic_writel(0x00004230,pmic_readl(0x00004230) | BIT(4));//# turn off ldo0 when FSM in deepsleep//ldo0_bound_reg
        pmic_writel(0x00004234,pmic_readl(0x00004234) | BIT(4));//# turn off ldo1 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x00004238,pmic_readl(0x00004238) | BIT(4));//# turn off ldo2 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x0000423C,pmic_readl(0x0000423C) | BIT(4));//# turn off ldo3 when FSM in deepsleep//ldo5_bound_reg
        //pmic_writel(0x00004240,pmic_readl(0x00004240) | BIT(4));//# turn off ldo4 when FSM in deepsleep//ldo5_bound_reg
        pmic_writel(0x00004244,pmic_readl(0x00004244) | BIT(4));//# turn off ldo5 when FSM in deepsleep//ldo5_bound_reg
        //pmic_writel(0x00004248,pmic_readl(0x00004248) | BIT(4));//# turn off ldo6 when FSM in deepsleep//ldo6_bound_reg

        pmic_writel(0x424c,0x1);//# update ldo settings

#endif
    }
#ifdef CONFIG_AIC_PMU_R1P0
    else {
        pmic_writel(0x00000934,pmic_readl(0x00000934) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(2));
        pmic_writel(0x00000938,pmic_readl(0x00000938) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x0000093c,pmic_readl(0x0000093c) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x0000092c,0x02);  //# configure DCDC/LDO bounding relationship sleep_en
        pmic_writel(0x00000940,pmic_readl(0x00000940) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000944,pmic_readl(0x00000944) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000948,pmic_readl(0x00000948) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
#ifndef CONFIG_LDO5_SLEEP_ON
        pmic_writel(0x0000094C,pmic_readl(0x0000094C) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
#endif
        pmic_writel(0x00000950,pmic_readl(0x00000950) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000954,pmic_readl(0x00000954) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000958,pmic_readl(0x00000958) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(2));
        pmic_writel(0x0000095C,pmic_readl(0x0000095C) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(2));
        pmic_writel(0x00000960,pmic_readl(0x00000960) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000964,pmic_readl(0x00000964) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000968,pmic_readl(0x00000968) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(2));
        //pmic_writel(0x0000096c,pmic_readl(0x0000096c) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(2));
        pmic_writel(0x00000970,pmic_readl(0x00000970) | BIT_PMU_RTC_LDO1_SLEEP_BOUND_REG(1));
        pmic_writel(0x00000928,0x1);   //# update ldo settings
    }
#endif
}

static void common_sleep_init(u32 pmic_id)
{
    if (pmic_id == 0x202) {
        /* 01_COMMON_PMU_Static_Configuration */
        pmic_writel(0x00008214,0x00000000);//# clk_apb_pd auto gating
        pmic_writel(0x00004010,0x00000000);//# clk_apb_rtc_auto gating
        pmic_writel(0x0000801c,0x00000000);//# turn off efuse function clock
        pmic_writel(0x00004020,0x00000001);//# enable deepsleep
        pmic_writel(0x000000d8,0x00000001);//# Enable ovlo when FSM in Deep & Normal//default off

        pmic_writel(0x00004224,0x00000001);//# dcdc/ldo chip sleep bound enable
        pmic_writel(0x0000424c,0x00000001);//# dcdc/ldo register configure done pulse

        /* 03_COMMON_PMU_26M_MUX_Configuration*/
        pmic_writel(0x00000408,0x00000000);//# turn off xc26m clock when FSM in DEEPSleep & shutdown
        pmic_writel(0x00004008,0x00000000);//# xc26m_en_18 is control by HW logic
        pmic_writel(0x00000154,0x00000001);//# enable signal for rco32k clock //default on
        pmic_writel(0x00000158,0x00000000);//# disable signal for xco32k clock//default off
        pmic_writel(0x0000015c,0x00000000);//# disable signal for xco32k_less clock//default off

        /* 04_COMMON_PMU_DCXO_LowPower_Setting */
        pmic_writel(0x0000009c,0x00000004);//# xtlbuf0 bound with xtl0
        pmic_writel(0x0000401c,0x00000000);//# disable xc26m_buf1_force
        pmic_writel(0x000000a0,0x00000000);//# disable xc26m_buf2

        /* 05_COMMON_PMU_Typical_Scenario_Setting */
        pmic_writel(0x00000148,0x00000000);//# turn off ups counter clock
        pmic_writel(0x00009430,0x00000001);//# turn off adc
        pmic_writel(0x00000134,0x00000000);//# turn off rtc alarm clock

#ifdef CONFIG_AIC_PMU_R2P0
        pmic_writel(0x000002a0,0x00000001);
#endif
    }
#ifdef CONFIG_AIC_PMU_R1P0
    else {
        /* 01_COMMON_PMU_Static_Configuration */
        /* Digial */
        pmic_writel(0x00008014,0x00000000);  //#Turn off apb Efuse Clock, Efuse is not used after power on
        pmic_writel(0x00008024,0x00000000);  //#Turn off Efuse Clock, Efuse is not used after power on
        pmic_writel(0x00000674,0x00000000);  //#turn off CKM_EN_FORCE1_CLK_RCO32K_PP
        pmic_writel(0x00000678,0x00000000);  //#turn off CKM_EN_FORCE1_CLK_XC32K_PP
        pmic_writel(0x0000067c,0x00000000);  //#turn off CKM_EN_FORCE1_CLK_XC32K_less_PP
        pmic_writel(0x00000904,0x00000002);  //#Gate PD Domain 26M in DS, I2C\ADC\PWM\LED can't work in DS
        pmic_writel(0x00000600,0x00000000);  //#turn off xc26m_en_force, use HW Logic
        if (pmic_readl(0x8000) >= 0x02010004)
            pmic_writel(0x00000610,0x00000000);  //#XC26M is controlled by HW logic
        /* Analog */
        pmic_writel(0x00000834,0x00000003);  //#turn on OVLO_DETconfigure EN_OVLOVBAT_18 = 0 Modify 2->3
        pmic_writel(0x000008A0,0x00382C10);  //#Config INT_LDO0 & LDO1 Low Power Mode
        pmic_writel(0x00008a2c,0x0000000a);  //#set chipsleep voltage to 0.7mv
        pmic_writel(0x00000990,0x00000001);  //#enable pmic chipsleep

        /* 03_COMMON_PMU_Typical_Scenario_Setting */
        //#Digital Module
        pmic_writel(0x000006ac,0x00000000);  //#turn off ckm_busy_ups_counter_ckmin
        pmic_writel(0x00008018,0x00000000);  //#Turn off apb adc Clock
        pmic_writel(0x00008020,0x00000000);  //#Turn off ADC Clock
        pmic_writel(0x00008028,0x00000000);  //#Turn off pwm Clock

        //#Analog Module
        //pmic_writel(0x00000814,0x00000001);  //#turn off charger enable, disable VBUS Detect
        pmic_writel(0x00000840,0x00000000);  //#turn off VCC5TST, disalbe ADC VBUS and Battery sample channel
        pmic_writel(0x00000670,0x00000002);  //#Force turn off ADC
        pmic_writel(0x00000808,0x00000000);  //#turn off INTLDO2, disable ADC VREFP_HQ

        //#update DCDC/LDO bound configure
        pmic_writel(0x00000928,0x00000001);  //#dcdc_ldo_cfg_update_reg puls
    }
#endif
}

void gp202_lp_cfg(void)
{}

int pmic_slp(void)
{
    u32 pmic_id;

    pmic_id = (pmic_readl(REG_GLB_PD_CHIP_ID_VERSION) >> 16) & 0xffff;
    xtl_buf_init(pmic_id);
    ldo_sleep_init(pmic_id);
    common_sleep_init(pmic_id);
    slp_get_trim(pmic_id);

    return 0;
}

early_init(pmic_slp);
