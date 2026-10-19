/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1728b6
#define LOG_MODULE    AMOD_DRV

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_interrupt.h>
#include <aic_io.h>
#include <aic_size.h>
#include <irq.h>
#include <aic_reg_base.h>
#include <aic_auto_reg.h>
#include <cru_top.h>
#include <sys_nd_glb.h>
#include <rccalibre_pd.h>
#include <aic_at_process_common.h>
#include <chip_id.h>

#include <msrc_cfg.h>

#define RCO32K_CALI_THRD_PRIO        13

#define MSRC0_START_CALI_FLAG        0x1
#define MSRC1_START_CALI_FLAG        0x2
#define MSRC0_CALI_VALID_FLAG        0x4
#define MSRC1_CALI_VALID_FLAG        0x8
#define RC32K_START_CALI_FLAG        0x10
#define RC32K_CALI_VALID_FLAG        0x20

aos_mutex_ptr msrc_cali_mutex = AIC_NULL;
aos_timer_ptr msrc0_cali_timer = AIC_NULL;
aos_timer_ptr msrc1_cali_timer = AIC_NULL;
aos_timer_ptr rc32k_cali_timer = AIC_NULL;
aos_event_ptr msrc_cali_event_ptr = AIC_NULL;
aos_thread_id msrc0_cali_thread_id = AOS_INVALID_THREAD_ID;
aos_thread_id msrc1_cali_thread_id = AOS_INVALID_THREAD_ID;
aos_thread_id rc32k_cali_thread_id = AOS_INVALID_THREAD_ID;

static boolean rc32k_timer_enabled = AIC_FALSE;
static boolean msrc0_timer_enabled = AIC_FALSE;
static boolean use_internal_cali_step = AIC_FALSE;

enum msrc_cali_src {
    MSRC1 = 0,
    MSRC0,
    RC32K,
};

u32 msrc0_cali_exp_time = MSRC0_CALI_INTERVAL;
u32 msrc1_cali_exp_time = MSRC1_CALI_INTERVAL;
u32 rc32k_cali_exp_time = RC32K_CALI_INTERVAL;

u8 cali_step_cfg_ab[3] = {6, 9, 6};

static int msrc_cali_src_sel(enum msrc_cali_src mc_src)
{
    u32 val;

    val = AIC_READ32(REG_CRU_TOP_CLK_RCO_CFG);
    val &= ~BIT_CRU_TOP_SEL_CLK_RCO_RF(0x3);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL(mc_src);
    AIC_WRITE32(REG_CRU_TOP_CLK_RCO_CFG, val);

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL(0x3);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL(mc_src);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL, val);

    return 0;
}

static enum msrc_cali_src msrc_cali_get_src(void)
{
    return AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL) &
                      BIT_RCCALIBRE_PD_RCO_CALIBRE_SRC_SEL(0x3);
}

static int msrc_cali_auto_mode_ctl(boolean enable)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG1);

    if (enable)
        val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_AUTO_MODE;
    else
        val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_AUTO_MODE;

    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG1, val);

    return 0;
}

static int msrc_cali_rco_cycle_set(u32 rco_cycle)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_RCO_CYCLE);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_RCO_CYCLE(0xfffff);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_RCO_CYCLE(rco_cycle);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_RCO_CYCLE, val);

    return 0;
}

static int msrc_cali_golden_value_set(u32 golden_value)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_GOLDEN_VALUE);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_GOLDEN_VALUE(0xffffff);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_GOLDEN_VALUE(golden_value);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_GOLDEN_VALUE, val);

    return 0;
}

static int msrc_cali_margin_cnt_set(u16 margin_cnt)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG1);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_MARGIN_CNT(0xfff);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_MARGIN_CNT(margin_cnt);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG1, val);

    return 0;
}

static int msrc_cali_stable_num_set(u16 stable_num)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_STABLE_NUM(0x3ff);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_STABLE_NUM(stable_num);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0, val);

    return 0;
}

static int msrc_cali_step_set(u8 step)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_STEP(0xf);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_STEP(step);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0, val);

    return 0;
}

static int msrc_cali_timeout_num_set(u8 timeout)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0);
    val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_TIMEOUT_NUM(0xff);
    val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_TIMEOUT_NUM(timeout);
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CFG_REG0, val);

    return 0;
}

static int msrc_cali_ctl(u32 enable)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_RCO_CALIBRE_ENABLE);
    if (enable)
        val |= BIT_RCCALIBRE_PD_RCO_CALIBRE_ENABLE;
    else
        val &= ~BIT_RCCALIBRE_PD_RCO_CALIBRE_ENABLE;
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_ENABLE, val);

    return 0;
}

static int msrc_cali_trigger(void)
{
    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_TRIGGER, BIT_RCCALIBRE_PD_RCO_CALIBRE_TRIGGER);

    return 0;
}

/*static int msrc_cali_done_int_ctl(boolean enable)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN);
    if (enable)
        val |= BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_EN;
    else
        val &= ~BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_EN;

    AIC_WRITE32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN, val);

    return 0;
}

static int msrc_cali_timeout_int_ctl(boolean enable)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN);
    if (enable)
        val |= BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN;
    else
        val &= ~BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN;

    AIC_WRITE32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_EN, val);

    return 0;
}*/

static int msrc_cali_done_int_clr(void)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR);
    val |= BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_CLEAR;
    AIC_WRITE32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR, val);

    return 0;
}

/*static int msrc_cali_timeout_int_clr(void)
{
    u32 val;

    val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR);
    val |= BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR;
    AIC_WRITE32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR, val);

    return 0;
}*/

static int msrc_cali_done_int_status(void)
{
    return AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_STATUS) & BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_STATUS;
}

static int msrc_cali_timeout_int_status(void)
{
    return AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_STATUS) & BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_STATUS;
}

static int msrc_cali_common_cfg(enum msrc_cali_src mc_src)
{
    u8 val, cali_step;
    struct msrc_cali_cfg *mc_cfg_ptr;

    val = get_version_id();
    val &= 0xF;

    if ((mc_src ==  MSRC0) && (val < 2))
        mc_cfg_ptr = &g_msrc0_ab_cfg;
    else
        mc_cfg_ptr = &g_mc_cfg[mc_src];

    msrc_cali_golden_value_set(mc_cfg_ptr->cali_golden_value);
    msrc_cali_rco_cycle_set(mc_cfg_ptr->cali_cycle);
    msrc_cali_margin_cnt_set(mc_cfg_ptr->cali_margin_cnt);
    msrc_cali_stable_num_set(mc_cfg_ptr->cali_stable_num);

    /* use different cali step for different chip version */
    if (use_internal_cali_step)
        cali_step = cali_step_cfg_ab[mc_src];
    else
        cali_step = mc_cfg_ptr->cali_step;
    msrc_cali_step_set(cali_step);

    msrc_cali_timeout_num_set(mc_cfg_ptr->cali_timeout_val);
    msrc_cali_auto_mode_ctl(mc_cfg_ptr->cali_auto_mode);

    return AIC_SUCCESS;
}

static boolean msrc_ready(enum msrc_cali_src mc_src)
{
    boolean ready = FALSE;

    switch (mc_src) {
    case MSRC1:
        ready = AIC_READ32(REG_SYS_ND_GLB_MSRC_STATUS) & BIT_SYS_ND_GLB_MSRC1_READY;
        break;

    case MSRC0:
        ready = AIC_READ32(REG_SYS_ND_GLB_MSRC_STATUS) & BIT_SYS_ND_GLB_MSRC0_READY;
        break;

    default:
        ready = TRUE;
        break;
    }

    return ready;
}

static int msrc_cali_cfg(enum msrc_cali_src mc_src)
{
    /* should power up first */
    if (!msrc_ready(mc_src))
        return AIC_EACCES;

    switch(mc_src) {
    case MSRC1:
        /*AIC_WRITE32(REG_SYS_ND_GLB_MSRC1_CFG_REG1_CLR, BIT_SYS_ND_GLB_MSRC1_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_MSRC1_FTUNE_REG_EB);*/
        break;

    case MSRC0:
        /*AIC_WRITE32(REG_SYS_ND_GLB_MSRC0_CFG_REG1_CLR, BIT_SYS_ND_GLB_MSRC0_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_MSRC0_FTUNE_REG_EB);*/
        break;

    case RC32K:
        /*AIC_WRITE32(REG_SYS_ND_GLB_RC32_CFG_REG1_CLR,  BIT_SYS_ND_GLB_RC32K_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_RC32K_FTUNE_REG_EB);*/
        break;

    default:
        break;
    }

    /* select source first */
    msrc_cali_src_sel(mc_src);
    /* now can configure cali parameters */
    msrc_cali_common_cfg(mc_src);

    return AIC_SUCCESS;
}

/* load ate value to hw cali mode */
/*static int msrc_cali_load_value(enum msrc_cali_src mc_src)
{
    u32 val;

    switch (mc_src) {
    case MSRC1:
        val = AIC_READ32(REG_SYS_ND_GLB_MSRC1_CFG_REG0);
        val &= BIT_SYS_ND_GLB_MSRC1_CTUNE_REG(0x1ff);
        val = val >> 8;
        AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD_VALUE, val);
        break;

    case MSRC0:
        val = AIC_READ32(REG_SYS_ND_GLB_MSRC0_CFG_REG0);
        val &= BIT_SYS_ND_GLB_MSRC0_CTUNE_REG(0x1ff);
        val = val >> 8;
        AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD_VALUE, val);
        break;

    case RC32K:
        val = AIC_READ32(REG_SYS_ND_GLB_RC32_CFG_REG0);
        val &= BIT_SYS_ND_GLB_MSRC0_CTUNE_REG(0x1ff);
        val = val >> 8;
        AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD_VALUE, val);
        break;

    default:
        AIC_ASSERT(0);
    }

    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD,
                BIT_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD);

    return AIC_SUCCESS;
}*/

/* update hw cali value to sw register */
static int msrc_update_cali_value_to_sw_reg(enum msrc_cali_src mc_src)
{
    u32 val, ctune, ftune;

    switch (mc_src) {
    case MSRC1:
        /*ctune = AIC_READ32(REG_TOP_GLB_RCO_RC0_CTUNE);
        ftune = AIC_READ32(REG_TOP_GLB_RCO_RC0_FTUNE);
        val = AIC_READ32(REG_SYS_ND_GLB_MSRC1_CFG_REG0);
        val &= ~(BIT_SYS_ND_GLB_MSRC1_FTUNE_REG(0x1ff) | BIT_SYS_ND_GLB_MSRC1_CTUNE_REG(0x1ff));
        val |= BIT_SYS_ND_GLB_MSRC1_FTUNE_REG(ftune) | BIT_SYS_ND_GLB_MSRC1_CTUNE_REG(ctune);
        AIC_WRITE32(REG_SYS_ND_GLB_MSRC1_CFG_REG0, val);

        AIC_WRITE32(REG_SYS_ND_GLB_MSRC1_CFG_REG1_SET, BIT_SYS_ND_GLB_MSRC1_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_MSRC1_FTUNE_REG_EB);*/

        break;

    case MSRC0:
        /*ctune = AIC_READ32(REG_TOP_GLB_RCO_RC1_CTUNE);
        ftune = AIC_READ32(REG_TOP_GLB_RCO_RC1_FTUNE);
        val = AIC_READ32(REG_SYS_ND_GLB_MSRC0_CFG_REG0);
        val &= ~(BIT_SYS_ND_GLB_MSRC0_FTUNE_REG(0x1ff) | BIT_SYS_ND_GLB_MSRC0_CTUNE_REG(0x1ff));
        val |= BIT_SYS_ND_GLB_MSRC0_FTUNE_REG(ftune) | BIT_SYS_ND_GLB_MSRC0_CTUNE_REG(ctune);
        AIC_WRITE32(REG_SYS_ND_GLB_MSRC0_CFG_REG0, val);

        AIC_WRITE32(REG_SYS_ND_GLB_MSRC0_CFG_REG1_SET, BIT_SYS_ND_GLB_MSRC0_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_MSRC0_FTUNE_REG_EB);*/


        break;

    case RC32K:
        ctune = AIC_READ32(REG_TOP_GLB_RCO_RC2_CTUNE);
        ftune = AIC_READ32(REG_TOP_GLB_RCO_RC2_FTUNE);
        val = AIC_READ32(REG_SYS_ND_GLB_RC32_CFG_REG0);
        val &= ~(BIT_SYS_ND_GLB_RC32K_FTUNE_REG(0x1ff) | BIT_SYS_ND_GLB_RC32K_CTUNE_REG(0x1ff));
        val |= BIT_SYS_ND_GLB_RC32K_FTUNE_REG(ftune) | BIT_SYS_ND_GLB_RC32K_CTUNE_REG(ctune);
        AIC_WRITE32(REG_SYS_ND_GLB_RC32_CFG_REG0, val);

        AIC_WRITE32(REG_SYS_ND_GLB_RC32_CFG_REG1_SET,  BIT_SYS_ND_GLB_RC32K_CTUNE_REG_EB |
                                                       BIT_SYS_ND_GLB_RC32K_FTUNE_REG_EB);

        break;

    default:
        AIC_ASSERT(0);
    }

    /*AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD,
                BIT_RCCALIBRE_PD_RCO_CALIBRE_CTUNE_LOAD);*/

    return AIC_SUCCESS;
}


void msrc0_start_cali(void *data)
{
    if (!msrc_ready(MSRC0)) {
        return;
    }

    aos_set_event(msrc_cali_event_ptr, MSRC0_START_CALI_FLAG, AOS_OR);
}

void msrc1_start_cali(void *data)
{
    if (!msrc_ready(MSRC1)) {
        return;
    }

    aos_set_event(msrc_cali_event_ptr, MSRC1_START_CALI_FLAG, AOS_OR);
}

void rc32k_start_cali(void *data)
{
    aos_set_event(msrc_cali_event_ptr, RC32K_START_CALI_FLAG, AOS_OR);
}


static int msrc_cali_worker(enum msrc_cali_src mc_src, u32 request_flags)
{
    int ret;
    u32 actual_flags, wait_opt;

    if (aos_is_user_thread_context())
        wait_opt = AOS_WAIT_FOREVER;
    else
        wait_opt = AOS_NO_WAIT;

    ret = aos_get_mutex(msrc_cali_mutex, wait_opt);
    if (ret != AIC_SUCCESS) {
        return ret;
    }

    ret = msrc_cali_cfg(mc_src);
    if (ret != AIC_SUCCESS) {
        aos_put_mutex(msrc_cali_mutex);
        return ret;
    }

    msrc_cali_ctl(TRUE);

    /* trigger now */
    msrc_cali_trigger();

    /* wait cali done */
    aos_get_event(msrc_cali_event_ptr, request_flags,
                  AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);

    msrc_cali_ctl(FALSE);

    msrc_update_cali_value_to_sw_reg(mc_src);

    aos_put_mutex(msrc_cali_mutex);

    return AIC_SUCCESS;
}

void msrc0_cali_thread_entry(void *data)
{
    u32 actual_flags;

    while(1) {
        aos_get_event(msrc_cali_event_ptr, MSRC0_START_CALI_FLAG,
                      AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
        msrc_cali_worker(MSRC0, MSRC0_CALI_VALID_FLAG);
    }
}

void msrc1_cali_thread_entry(void *data)
{
    u32 actual_flags;

    while(1) {
        aos_get_event(msrc_cali_event_ptr, MSRC1_START_CALI_FLAG,
                      AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
        msrc_cali_worker(MSRC1, MSRC1_CALI_VALID_FLAG);
    }
}

void rc32k_cali_thread_entry(void *data)
{
    u32 actual_flags;

    while(1) {
        aos_get_event(msrc_cali_event_ptr, RC32K_START_CALI_FLAG,
                      AOS_AND_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
        msrc_cali_worker(RC32K, RC32K_CALI_VALID_FLAG);
    }
}

ISR_RET_E msrc_cali_irq_handler(void *param)
{
    enum msrc_cali_src mc_src;

    if (msrc_cali_timeout_int_status())
        AIC_ASSERT(0);

    mc_src = msrc_cali_get_src();
    AIC_ASSERT((mc_src == MSRC1) || (mc_src == MSRC0) || (mc_src == RC32K));

    if (msrc_cali_done_int_status()) {
        /* MSRC1 */
        if (mc_src == MSRC1)
            aos_set_event(msrc_cali_event_ptr, MSRC1_CALI_VALID_FLAG, AOS_OR);
        else if (mc_src == MSRC0)
            aos_set_event(msrc_cali_event_ptr, MSRC0_CALI_VALID_FLAG, AOS_OR);
        else
            aos_set_event(msrc_cali_event_ptr, RC32K_CALI_VALID_FLAG, AOS_OR);
    }

    msrc_cali_done_int_clr();

    return NONE_WORKER;
}

void aic_ephy_change_bb_cb(u8 ephy_status)
{
    if (AIC_READ32(REG_SYS_ND_GLB_PP_CLK_SEL_CFG) ==0) {
        /* if ephy stop */
        if (ephy_status == 0) {
           /* if cali timer is alread started, do nothing */
           if ((rc32k_timer_enabled == AIC_FALSE) && (rc32k_cali_timer))
               aos_activate_timer(rc32k_cali_timer);

           rc32k_timer_enabled = AIC_TRUE;
        } else {
           if ((rc32k_timer_enabled == AIC_TRUE) && (rc32k_cali_timer))
               aos_deactivate_timer(rc32k_cali_timer);

           rc32k_timer_enabled = AIC_FALSE;
        }
    }
}

int msrc0_power_ctl(boolean on)
{
    int ret;
    u32 val, wait_opt;

    if (aos_is_user_thread_context())
        wait_opt = AOS_WAIT_FOREVER;
    else
        wait_opt = AOS_NO_WAIT;

    ret = aos_get_mutex(msrc_cali_mutex, wait_opt);
    if (ret != AIC_SUCCESS) {
        return ret;
    }

    val = AIC_READ32(REG_SYS_ND_GLB_MSRC_CTRL);
    if (on)
        val |= BIT_SYS_ND_GLB_MSRC0_FRC_ON;
    else
        val &= ~BIT_SYS_ND_GLB_MSRC0_FRC_ON;

    AIC_WRITE32(REG_SYS_ND_GLB_MSRC_CTRL, val);

    /* wait msrc0 ready */
    if (on)
        while(!(AIC_READ32(REG_SYS_ND_GLB_MSRC_STATUS) & BIT_SYS_ND_GLB_MSRC0_READY));

    aos_put_mutex(msrc_cali_mutex);

    return AIC_SUCCESS;
}

void aic_msrc0_cali_timer_enable(boolean enable)
{
    if (!msrc0_cali_timer) {
        msrc0_timer_enabled = AIC_TRUE;
        return;
    }

    if ((enable == AIC_TRUE) && (msrc0_timer_enabled == AIC_FALSE)) {
        msrc_cali_worker(MSRC0, MSRC0_CALI_VALID_FLAG);
        aos_activate_timer(msrc0_cali_timer);
        msrc0_timer_enabled = AIC_TRUE;
    }

    if ((enable == AIC_FALSE) && (msrc0_timer_enabled == AIC_TRUE)) {
        aos_deactivate_timer(msrc0_cali_timer);
        msrc0_timer_enabled = AIC_FALSE;
    }
}

static void msrc_cali_check(void)
{
    u32 val;

    /* wait cali done */
    while(1) {
        val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_STATUS);
        if (val & BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_STATUS)
            AIC_ASSERT(0);

        if (val & BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_STATUS) {
            val = AIC_READ32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR);
            val |= BIT_RCCALIBRE_PD_INT_RCO_CALIBRE_DONE_CLEAR;
            AIC_WRITE32(REG_RCCALIBRE_PD_INT_RCO_CALIBRE_TIMEOUT_CLEAR, val);

            break;
        }
    }

    AIC_WRITE32(REG_RCCALIBRE_PD_RCO_CALIBRE_ENABLE, ~BIT_RCCALIBRE_PD_RCO_CALIBRE_ENABLE);
}

int msrc_cali_init(void)
{
    u32 auto_active;

    msrc_cali_check();

    /* create a mutex to protect msrc cali */
    msrc_cali_mutex = aos_create_mutex("msrc");
    AIC_ASSERT(msrc_cali_mutex);

    msrc_cali_event_ptr = aos_create_event("msrc");
    AIC_ASSERT(msrc_cali_event_ptr);

    if (msrc0_timer_enabled == AIC_TRUE)
        auto_active = AOS_AUTO_ACTIVATE;
    else
        auto_active = AOS_NO_ACTIVATE;

    /* create a timer for msrc0 calibration */
    msrc0_cali_timer = aos_create_timer("msrc0",
                                        msrc0_start_cali,
                                        AIC_NULL,
                                        msrc0_cali_exp_time,
                                        auto_active,
                                        AOS_BE_PERIOD);

    /* create a timer for msrc1 calibration */
    msrc1_cali_timer = aos_create_timer("msrc1",
                                        msrc1_start_cali,
                                        AIC_NULL,
                                        msrc1_cali_exp_time,
                                        AOS_NO_ACTIVATE,
                                        AOS_BE_PERIOD);

    /* create two thread to wait cali done */
    msrc0_cali_thread_id = aos_create_thread_simply("msrc0",
                                                    msrc0_cali_thread_entry, NULL,
                                                    SIZE_1K, RCO32K_CALI_THRD_PRIO);
    msrc1_cali_thread_id = aos_create_thread_simply("msrc1",
                                                    msrc1_cali_thread_entry, NULL,
                                                    SIZE_1K, RCO32K_CALI_THRD_PRIO);

    /* if use bb rc32k */
    if (AIC_READ32(REG_SYS_ND_GLB_PP_CLK_SEL_CFG) ==0) {
        /* create a timer for rc32k calibration */
        rc32k_cali_timer = aos_create_timer("rc32k",
                                            rc32k_start_cali,
                                            AIC_NULL,
                                            rc32k_cali_exp_time,
                                            AOS_NO_ACTIVATE,
                                            AOS_BE_PERIOD);
        rc32k_cali_thread_id = aos_create_thread_simply("rc32k",
                                                        rc32k_cali_thread_entry, NULL,
                                                        SIZE_1K, RCO32K_CALI_THRD_PRIO);

        /* register phy mode change callback */
        aic_register_ephystat_state_cb(aic_ephy_change_bb_cb);
    }

    /* disable ctune/ftune reg eb, configure at bootloader */
    /*AIC_WRITE32(REG_SYS_ND_GLB_MSRC0_CFG_REG1_SET, BIT_SYS_ND_GLB_MSRC0_CTUNE_REG_EB |
                                                   BIT_SYS_ND_GLB_MSRC0_FTUNE_REG_EB);
    AIC_WRITE32(REG_SYS_ND_GLB_MSRC1_CFG_REG1_SET, BIT_SYS_ND_GLB_MSRC1_CTUNE_REG_EB |
                                                   BIT_SYS_ND_GLB_MSRC1_FTUNE_REG_EB);
    AIC_WRITE32(REG_SYS_ND_GLB_RC32_CFG_REG1_SET,  BIT_SYS_ND_GLB_RC32K_CTUNE_REG_EB |
                                                   BIT_SYS_ND_GLB_RC32K_FTUNE_REG_EB);*/

    /* int config */
    /*msrc_cali_done_int_ctl(TRUE);
    msrc_cali_timeout_int_ctl(TRUE);*/

    /* register an irq handler */
    register_irq(MSRC_CALI_IRQ, msrc_cali_irq_handler, AIC_NULL, AIC_NULL, AIC_NULL);

    if (AIC_READ32(0x400c0ffc) < 0xa0000002)
        use_internal_cali_step = AIC_TRUE;

    return AIC_SUCCESS;
}

AIC_DRV_P1_INIT_EXPORT(msrc_cali_init);

