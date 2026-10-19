/*****************************************************************************/
/*                                                                           */
/*         Copyright 2023 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifdef __SUBSYS_DRV_CFG_H__
#error redefine __SUBSYS_DRV_CFG_H__
#else
#define __SUBSYS_DRV_CFG_H__

#include <aic_auto_reg.h>
#include <cru_cp.h>
#include <aic_iot_clk.h>
#include <aic_clk_map.h>

/*
 * REG_PMU_CTRL_CP_SLEEP_CTRL:
 * BIT_PMU_CTRL_PD_CP_AUTO_SHUTDOWN_EN is 0, force shutdown can be valid.
 * The default value is 1, so we just need clear the force deeep sleep bit
 * to power on cp.
 */
#define CP_SLEEP_CTRL_REG       REG_PMU_CTRL_CP_SLEEP_CTRL
#define CP_SLEEP_CTRL_MASK      BIT_PMU_CTRL_CP_FORCE_DEEP_SLEEP

/*
 * REG_PMU_CTRL_PD_CP_STATE:
 * 0    -- complete power on.
 * 0x1f -- complete power off.
 */
#define CP_POWER_ON_STATE       REG_PMU_CTRL_PD_CP_STATE
#define POWER_ON_STATE          0x00
#define POWER_OFF_STATE         0x1f


/*
 * REG_PMU_CTRL_POWER_DOMAIN_SOFT_RESET:
 * BIT2: 0 -- release, 1 -- hold
 */
#define CP_SYS_RESET_REG        REG_PMU_CTRL_POWER_DOMAIN_SOFT_RESET
#define CP_SYS_RESET_MASK       BIT_PMU_CTRL_CP_SOFT_RESET

/*
 * REG_PMU_CTRL_AUDIO_SLEEP_CTRL:
 * BIT_PMU_CTRL_AUDIO_FORCE_DEEP_SLEEP set 0 en to enable adsp autu sleep.
 */
#define ADSP_SLEEP_CTRL_REG     REG_PMU_CTRL_AUDIO_SLEEP_CTRL

/*
 * REG_PMU_CTRL_AUDIO_POWER_CTRL:
 *
 */
#define ADSP_POWER_ON_REG       REG_PMU_CTRL_AUDIO_POWER_CTRL

/*
 * REG_PMU_CTRL_PD_AUDIO_STATE:
 *
 */
#define ADSP_POWER_ON_STATE     REG_PMU_CTRL_PD_AUDIO_STATE
#define ADSP_POWER_WAIT_MASK    0x1f

/*
 * REG_TOP_GLB_EXTERNAL_WAIT:
 *
 */
#define ADSP_CORE_RESET_REG     REG_TOP_GLB_EXTERNAL_WAIT
#define ADSP_CORE_RESET_MASK    BIT(0)

#ifdef CONFIG_SUPPORT_ADSP
static void subsys_adsp_auto_deepsleep(void)
{
    AIC_REG_CLRBIT(ADSP_SLEEP_CTRL_REG, BIT_PMU_CTRL_AUDIO_FORCE_DEEP_SLEEP);
}
static void subsys_adsp_hold_core(void)
{
    AIC_REG_SETBIT(ADSP_CORE_RESET_REG, ADSP_CORE_RESET_MASK);
}

static void subsys_adsp_release_core(void)
{
    AIC_REG_CLRBIT(ADSP_CORE_RESET_REG, ADSP_CORE_RESET_MASK);
}

static void subsys_adsp_pwron(void)
{
    alog_info("adsp power off.\n");
#ifndef WIN32
    clk_set_parent_comp(CLK_BLK_DSP_CORE, 2);

    /* adsp not deep */
    subsys_adsp_auto_deepsleep();
    /* hold core before power on */
    subsys_adsp_hold_core();
    /*
     * clear auto shutdown en to disable auto shutdown
     * and clear force shutdown.to power on adsp.
     */
    AIC_REG_CLRBIT(ADSP_POWER_ON_REG, BIT_PMU_CTRL_PD_AUDIO_AUTO_SHUTDOWN_EN);
    AIC_REG_CLRBIT(ADSP_POWER_ON_REG, BIT_PMU_CTRL_PD_AUDIO_FRC_SHUTDOWN);

    /* wait power on state to 0. */
    while(BIT_PMU_CTRL_PD_AUDIO_STATE(AIC_READ32(ADSP_POWER_ON_STATE)));

#ifdef CONFIG_CHECK_APLL
    extern void aic_check_apll(void);
    aic_check_apll();
#endif
    clk_set_parent_comp(CLK_BLK_DSP_CORE, 0);
#endif
}

static void subsys_adsp_pwroff(void)
{
    alog_info("adsp power off.\n");
    subsys_adsp_hold_core();
#ifndef WIN32
    /*
     * clear auto shutdown en to disable auto shutdown
     * and clear force shutdown.to powen adsp.
     */
    AIC_REG_SETBIT(ADSP_POWER_ON_REG, BIT_PMU_CTRL_PD_AUDIO_FRC_SHUTDOWN);

    /* wait power off state to POWER_OFF_STATE. */
   // while(POWER_OFF_STATE == BIT_PMU_CTRL_PD_AUDIO_STATE(AIC_READ32(ADSP_POWER_ON_STATE)));
#endif
}

static bool subsys_adsp_is_power_off(void)
{
#ifndef WIN32
    return (POWER_OFF_STATE == BIT_PMU_CTRL_PD_AUDIO_STATE(AIC_READ32(ADSP_POWER_ON_STATE)));
#endif
}

static void subsys_adsp_stop(void)
{
    alog_debug("adsp stop\n");

    subsys_adsp_hold_core();
}

static void subsys_adsp_start(void)
{
    alog_debug("adsp start\n");

    subsys_adsp_release_core();
}
#endif

static void subsys_cp_hold_sys(void)
{
    AIC_REG_SETBIT(CP_SYS_RESET_REG, CP_SYS_RESET_MASK);
}

static void subsys_cp_release_sys(void)
{
    AIC_REG_CLRBIT(CP_SYS_RESET_REG, CP_SYS_RESET_MASK);
}

static void subsys_cp_pwron(void)
{
    alog_debug("cp power on.\n");

#ifdef CONFIG_CHECK_APLL
    AIC_WRITE32(REG_CRU_CP_CLK_CP_CORE_CFG, BIT_CRU_CP_SEL_CLK_CP_CORE_RF(3) |
                                            BIT_CRU_CP_DIV_CFG_CLK_CP_CORE_RF(0));
#endif

    /* clear cp force deep sleep to poweron cp. */
    AIC_REG_CLRBIT(CP_SLEEP_CTRL_REG, CP_SLEEP_CTRL_MASK);

    /* wait power on state */
    while (POWER_ON_STATE != BIT_PMU_CTRL_PD_CP_STATE(AIC_READ32(CP_POWER_ON_STATE)));

#ifdef CONFIG_CHECK_APLL
    extern void aic_check_apll(void);
    aic_check_apll();

    AIC_WRITE32(REG_CRU_CP_CLK_CP_CORE_CFG, BIT_CRU_CP_SEL_CLK_CP_CORE_RF(0) |
                                            BIT_CRU_CP_DIV_CFG_CLK_CP_CORE_RF(0));
#endif /* CONFIG_CHECK_APLL */
}

static void subsys_cp_pwroff(void)
{
    alog_debug("cp power off.\n");
    subsys_cp_hold_sys();

   /* clear cp force deep sleep to poweoff cp. */
    AIC_REG_SETBIT(CP_SLEEP_CTRL_REG, CP_SLEEP_CTRL_MASK);
}

static bool subsys_cp_is_power_off(void)
{
    return (POWER_OFF_STATE == BIT_PMU_CTRL_PD_CP_STATE(AIC_READ32(CP_POWER_ON_STATE)));
}

#endif /* __SUBSYS_DRV_CFG_H__ */

