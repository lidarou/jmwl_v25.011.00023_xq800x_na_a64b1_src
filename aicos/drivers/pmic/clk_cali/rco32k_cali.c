/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13a358
#define LOG_MODULE    AMOD_CLK_CALI

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_size.h>
#ifdef CONFIG_SUBSYS_AP
#include <aic_at_process_common.h>
#include <pmic_rc32k_cfg.h>
#endif
#include <timer_inc.h>
#include <pmic_io.h>
#include <pmic_reg_base.h>
#include <glb_rtc.h>
#include <glb_ups.h>

#include <aic_cmdline.h>

#include <pmic_rco32k_cali.h>

#define RCO32K_CALI_THRD_PRIO        13

#define RCO32K_CALI_VALID_FLAG       0x1

static aos_thread_id rco32k_cali_thread_id = AOS_INVALID_THREAD_ID;
static aos_event_ptr rco32k_cali_event_ptr = AIC_NULL;

u32 rtc_sel_clk_mux = 0xFF;
u32 rc032k_cali_golden_result = 0x0;
u32 rc032k_cali_cfg3_base = 0x0;
int pmic_id;

#ifdef CONFIG_SUBSYS_AP

static boolean cali_timer_enabled = AIC_FALSE;

aos_timer_ptr rc_sw_cali_timer;
u32 rc_sw_cali_exp_time = PMIC_RC32K_CALI_INTERVAL;

int aic_rco32k_sw_cali_for_ups(void)
{
    u32 val, old_val;

    /* ToDo: if power down rco, should power up rco first*/

    /* switch source to rco32k first */
    old_val = rtc_sel_clk_mux;
    val = old_val;
    val &= ~0x3;
    if (pmic_id == 0x201) {
        pmic_writel(0x65c, val);
    } else {
        pmic_writel(0x188, val);
    }
    aic_udelay(10);

    /* sw trigger once */
    val = rc032k_cali_cfg3_base;
    val &= ~BIT(17);
    if (pmic_id == 0x201)
        pmic_writel(0x228, val);
    else
        pmic_writel(0x238, val);
    aic_udelay(100);

    val = rc032k_cali_cfg3_base;
    val |= BIT(17);
    if (pmic_id == 0x201)
        pmic_writel(0x228, val);
    else
        pmic_writel(0x238, val);
    aic_udelay(400);

    val = rc032k_cali_cfg3_base;
    val &= ~BIT(17);
    if (pmic_id == 0x201)
        pmic_writel(0x228, val);
    else
        pmic_writel(0x238, val);
    /* add some delay to make sure register write to pmic */
    aic_udelay(100);

    if (pmic_id == 0x201)
        pmic_writel(0x65c, old_val);
    else
        pmic_writel(0x188, old_val);
    aic_udelay(100);

    return AOS_SUCCESS;
}

/* interval time unit is s */
int aic_rco32k_hw_cali_interval_for_ups(u32 interval)
{
    u32 val;

    if (interval > 0x3fff)
        interval = 0x3fff;

    val = rc032k_cali_cfg3_base;
    val &= ~(0x3fff);
    val |= interval;
    if (pmic_id == 0x201)
        pmic_writel(0x228, val);
    else
        pmic_writel(0x238, val);
    /* add some delay to make sure register write to pmic */
    aic_udelay(100);

    return AOS_SUCCESS;
}
#endif

static aic_rco32k_sw_cali_cb aic_rc32k_sw_cali_prepare = AIC_NULL;

int aic_register_rco32k_sw_cali_cb(aic_rco32k_sw_cali_cb cb)
{
    aic_rc32k_sw_cali_prepare = cb;

    return AIC_SUCCESS;
}

void aic_unregister_rco32k_sw_cali_cb(void)
{
    if (AIC_NULL != aic_rc32k_sw_cali_prepare) {
        aic_rc32k_sw_cali_prepare = AIC_NULL;
    }
}

void aic_rco32k_sw_cali_clk_ctrl(u8 on)
{
    if (on) {
        /* enable calibrate clock */
        pmic_writel(0x694, BIT(0));
        pmic_writel(0x698, BIT(0));
    } else {
        /* disable calibrate clock */
        pmic_writel(0x694, ~BIT(0));
        pmic_writel(0x698, ~BIT(0));
    }
}

void aic_rco32k_sw_cali_trigger(void)
{
    u32 val;

    //val = rc032k_cali_cfg3_base;
    //val &= ~BIT_GLB_UPS_RCO_CALIBRATION_MANUAL_TRIG;
    //pmic_writel(REG_GLB_UPS_RCO_CALIBRE_CFG3, val);
    //aos_usleep(200);
    val = rc032k_cali_cfg3_base;
    val |= BIT(17);
    pmic_writel(0x228, val);
    aic_udelay(200);
    val = rc032k_cali_cfg3_base;
    val &= ~BIT(17);
    pmic_writel(0x228, val);
}

u32 aic_rco32k_sw_cali_check(void)
{
    u32 ret = RCO32K_CALI_ONGOING;
    u32 val2 = pmic_readl(0x230);
    u32 val = ((rc032k_cali_golden_result >= val2) ? (rc032k_cali_golden_result - val2) : (val2 - rc032k_cali_golden_result));

    if (val < 4) {
        ret = RCO32K_CALI_SUCCESS;
    } else if (pmic_readl(0x22c) & BIT(0)) {
        aicx_error(0x19f66f, "xdx", "rco32k calibration failed, result = 0x%x, delta = %d, golden = 0x%x\n",
                   val2, val, rc032k_cali_golden_result);
        ret = RCO32K_CALI_TIMEOUT;
    } else {
    aicx_info(0x05283b, "xdx", "rco32k calibration result = 0x%x, delta = %d, golden = 0x%x\n",
              val2, val, rc032k_cali_golden_result);
    }
    return ret;
}

void aic_gp202_rc32k_switch_to_sw_cali(void)
{
    u32 val;

    /* read ctune/ftune to register, 0x110's value comes from hw cali logic now */
    val = pmic_readl(0x110);
    val &= ~0xfffffe00;
    pmic_writel(0x54, BIT(12) | val);

    val = pmic_readl(0x110);
    val &= ~0xfffc01ff;
    val >>= 9;
    pmic_writel(0x58, BIT(12) | val);

    /* enable sw ctune eb */
    pmic_writel(0x23c, ~BIT(2));
}

void aic_gp202_rc32k_sw_cali_trigger(void)
{
    u32 val;

    /* switch to hw cali */
    val = pmic_readl(0x54);
    val &= ~BIT(12);
    pmic_writel(0x54, val);

    val = pmic_readl(0x58);
    val &= ~BIT(12);
    pmic_writel(0x58, val);

    val = pmic_readl(0x23c);
    val |= BIT(2);
    pmic_writel(0x23c, val);

    val = pmic_readl(0x238);
    val &= ~BIT(17);
    pmic_writel(0x238, val);
    aic_udelay(250);

    val = pmic_readl(0x238);
    val |= BIT(17);
    pmic_writel(0x238, val);
    aic_udelay(250);

    val = pmic_readl(0x238);
    val &= ~BIT(17);
    pmic_writel(0x238, val);
}

int aic_gp202_rc32k_sw_cali_check(void)
{
    u32 ret = RCO32K_CALI_ONGOING;
    u32 val2 = pmic_readl(0x240);
    u32 val = ((rc032k_cali_golden_result >= val2) ? (rc032k_cali_golden_result - val2) : (val2 - rc032k_cali_golden_result));

    if (pmic_readl(0x23c) & BIT(0)) {
        aicx_error(0x117748, "", "rc32k calibration timeout!\n");
        /* switch to sw cali */
        val = pmic_readl(0x54);
        val |= BIT(12);
        pmic_writel(0x54, val);

        val = pmic_readl(0x58);
        val |= BIT(12);
        pmic_writel(0x58, val);

        val = pmic_readl(0x23c);
        val &= ~BIT(2);
        pmic_writel(0x23c, val);

        ret = RCO32K_CALI_TIMEOUT;
    } else if (pmic_readl(0x23c) & BIT(1)) {
        aic_gp202_rc32k_switch_to_sw_cali();
        ret = RCO32K_CALI_SUCCESS;
    } else {
        aicx_info(0x05283b, "xdx", "rco32k calibration result = 0x%x, delta = %d, golden = 0x%x\n",
                   pmic_readl(0x240), val, rc032k_cali_golden_result);
    }

    return ret;
}

void aic_rco32k_sw_cali_internal(void *param)
{
    u32 ret = RCO32K_CALI_ONGOING;

    if (AIC_NULL != aic_rc32k_sw_cali_prepare) {
        aic_rc32k_sw_cali_prepare(0);
    }

    //aic_rco32k_sw_cali_clk_ctrl(1);

    if (pmic_id == 0x201)
        aic_rco32k_sw_cali_trigger();
    else
        aic_gp202_rc32k_sw_cali_trigger();

    /* wait some time to read calibration data */
    do {
        aos_usleep(100);
        if (pmic_id == 0x201)
            ret = aic_rco32k_sw_cali_check();
        else
            ret = aic_gp202_rc32k_sw_cali_check();
    } while (RCO32K_CALI_ONGOING == ret);

    //aic_rco32k_sw_cali_clk_ctrl(0);

    if (AIC_NULL != aic_rc32k_sw_cali_prepare) {
        aic_rc32k_sw_cali_prepare(1);
    }
}


void aic_rco32k_start_cali(void *data)
{
    if (rtc_sel_clk_mux == 0) {
        aos_set_event(rco32k_cali_event_ptr, RCO32K_CALI_VALID_FLAG, AOS_OR);
    }
}

void rco32k_cali_thread_entry(void *data)
{
    u32 actual_flags;

    while(1) {
        aos_get_event(rco32k_cali_event_ptr, RCO32K_CALI_VALID_FLAG,
                      AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
        aic_rco32k_sw_cali_internal(AIC_NULL);
    }
}

#ifdef CONFIG_SUBSYS_AP
void aic_ephy_change_cb(u8 ephy_status)
{
    if (rtc_sel_clk_mux == 0) {
        /* if ephy stop */
        if (ephy_status == 0) {
           /* if cali timer is alread started, do nothing */
           if (cali_timer_enabled == AIC_FALSE)
               aos_activate_timer(rc_sw_cali_timer);

           cali_timer_enabled = AIC_TRUE;
        } else {
           if (cali_timer_enabled == AIC_TRUE)
               aos_deactivate_timer(rc_sw_cali_timer);

           cali_timer_enabled = AIC_FALSE;
        }
        aicx_info(0x189fd4, "dd", "rco32k calibration timer %d %d!\n", ephy_status, cali_timer_enabled);
    }
}
#endif

int aic_rco32k_early_cali_init(void)
{
#ifdef CONFIG_SUBSYS_AP
    u32 val, val2, tmp;
#endif

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);
    pmic_id = (pmic_id >> 16) & 0xFFFF;

    /* only rc32k solution need following configure */
    if (pmic_id == 0x201)
        rtc_sel_clk_mux = pmic_readl(0x65c);
    else
        rtc_sel_clk_mux = pmic_readl(0x188);

    if (rtc_sel_clk_mux == 0) {
#ifdef CONFIG_SUBSYS_AP
        if (pmic_id == 0x202) {
            /* sw cali check */
            while(1) {
                val = pmic_readl(0x234);
                val2 = pmic_readl(0x240);
                aicx_info(0x1c425d, "xx", "rc32k golden value 0x%x, result: 0x%x\n", val, val2);
                aicx_info(0x026f1c, "x", "rco32k int status 0x%x\n", pmic_readl(0x23c));
                if (val > val2)
                    val = val - val2;
                else
                    val = val2 - val;

                val2 = pmic_readl(0x23c);
                if (val2 & BIT(0)) {
                    aicx_error(0x117748, "", "rc32k calibration timeout!\n");
                    tmp = pmic_readl(0x244);
                    tmp |= 0x4;
                    pmic_writel(0x244, tmp);
                    tmp &= ~0x4;
                    pmic_writel(0x244, tmp);
                    AIC_ASSERT(0);
                    return -1;
                }

                if (val2 & BIT(1)) {
                    aicx_info(0x04afdf, "", "rc32k calibration success!\n");
                    break;
                }

                aic_mdelay(90);
            }

            /* set cali step to 10.25ms for system normal run */
            val = 0x0f0103ff;
            pmic_writel(0x230, val);
            val = 0x00000011;
            pmic_writel(0x24c, val);
            val = 0x0004116a;
            pmic_writel(0x234, val);

            aic_gp202_rc32k_switch_to_sw_cali();
        }

#endif
    }

    if (pmic_id == 0x201) {
        rc032k_cali_golden_result = pmic_readl(0x224);
        rc032k_cali_cfg3_base = pmic_readl(0x228);
    } else {
        rc032k_cali_golden_result = pmic_readl(0x234);
        rc032k_cali_cfg3_base = pmic_readl(0x238);
    }

    if (rtc_sel_clk_mux == 0) {
        rco32k_cali_event_ptr = aos_create_event("rce");
        rco32k_cali_thread_id = aos_create_thread_simply("rcth",
                                                         rco32k_cali_thread_entry, NULL,
                                                         SIZE_1K, RCO32K_CALI_THRD_PRIO);

#ifdef CONFIG_SUBSYS_AP
        cali_timer_enabled = AIC_FALSE;

#ifdef REG_SYS_ND_GLB_PP_CLK_SEL_CFG
        if (!AIC_READ32(REG_SYS_ND_GLB_PP_CLK_SEL_CFG))
            cali_timer_enabled = AIC_TRUE;
#endif

        if (cali_timer_enabled == AIC_FALSE) {
            /* create  a timer but not start now */
            rc_sw_cali_timer = aos_create_timer("rct",
                                                aic_rco32k_start_cali,
                                                AIC_NULL,
                                                rc_sw_cali_exp_time,
                                                AOS_NO_ACTIVATE,
                                                AOS_BE_PERIOD);

            /* register phy mode change callback */
            aic_register_ephystat_state_cb(aic_ephy_change_cb);
        } else {
            /* create  a auto start timer */
            rc_sw_cali_timer = aos_create_timer("rct",
                                                aic_rco32k_start_cali,
                                                AIC_NULL,
                                                rc_sw_cali_exp_time,
                                                AOS_AUTO_ACTIVATE,
                                                AOS_BE_PERIOD);
        }

        if (pmic_id == 0x201) {
            /* enable calibrate clock */
            aic_rco32k_sw_cali_clk_ctrl(1);
        }

#endif
    }

    return AOS_SUCCESS;
}

AIC_DRV_P2_INIT_EXPORT(aic_rco32k_early_cali_init);

