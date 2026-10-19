/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16a102
#define LOG_MODULE    AMOD_ALOG

#include <aic_assert.h>
#include <adbg_cfg_api.h>
#include <AIC_DEBUG.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_module.h>
#include <alog_cfg.h>
#include <alog_inner.h>
#include <alog_api_fast.h>
#include <alog_trace_filter.h>

#define ALOG_DEFAULT_LV  ALOG_LV_INFO

#define ALOG_SET_MODE_LEVEL(m, l) \
    aic_bitmap_set_value(s_level_bitmap, (m) * ALOG_LV_BITS, ALOG_LV_BITS, (l))

#define ALOG_GET_MODE_LEVEL(m) \
    aic_bitmap_get_value(s_level_bitmap, (m) * ALOG_LV_BITS, ALOG_LV_BITS)

extern AIC_DEBUG s_debug_cfg;
static unsigned long *s_level_bitmap;

#if defined (CONFIG_SUBSYS_CP) || defined(WIN32)
/* fast log module switch define. */
u8 g_enable_phy_info_log;
u8 g_enable_L2_info_log;
u8 g_phy_log_lv;
u8 g_l2_log_lv;

void alog_set_fast_log(u8 enable) 
{
    if (enable)
        alog_en_fast_log();
    else
        alog_dis_fast_log();
}
#endif

void alog_trace_filter_init(void)
{
#if defined(WIN32)
   g_enable_phy_info_log  =  1;
   g_enable_L2_info_log  =  1;
   g_phy_log_lv = ALOG_DEFAULT_LV;
   g_l2_log_lv = ALOG_DEFAULT_LV;
#endif

    s_level_bitmap = s_debug_cfg.aic_log.level_bitmap;

    /* default enble all log. */
#if 1
    memset(s_level_bitmap, 0xff, sizeof(s_debug_cfg.aic_log.level_bitmap));
#else
    int i;

   /* init log level to default */
    for (i = 0; i < MAX_AMOD_CNT; i++) {
        ALOG_SET_MODE_LEVEL(i, ALOG_DEFAULT_LV);
        AIC_ASSERT(ALOG_GET_MODE_LEVEL(i) == ALOG_DEFAULT_LV);
    }
#endif
}

#ifdef CONFIG_SUBSYS_CP
void alog_trace_filter_init_fast(void)
{
    if (alog_is_trace_enable()) {
        g_phy_log_lv = ALOG_GET_MODE_LEVEL(AMOD_PHY);
        g_l2_log_lv = ALOG_GET_MODE_LEVEL(AMOD_EL2);
        g_enable_phy_info_log = (g_phy_log_lv > ALOG_LV_WARN);
        g_enable_L2_info_log = (g_l2_log_lv > ALOG_LV_WARN);
    }
}
#endif

static void alog_set_level(u8 level)
{
    int i;

    /* set all lever. */
    for (i = 0; i < MAX_AMOD_CNT; i++)
        ALOG_SET_MODE_LEVEL(i, level);

    adbg_save_debug_nv_cfg(TRUE);
}

u8 alog_get_log_level(u8 mod)
{
    return (u8)ALOG_GET_MODE_LEVEL(mod);
}

void alog_set_log_level(u8 mod, u8 level)
{
    if (AMOD_CNT == mod)
        alog_set_level(level);
    else
        ALOG_SET_MODE_LEVEL(mod, level);

#ifdef CONFIG_SUBSYS_CP
    if (AMOD_CNT == mod) {
        g_phy_log_lv = level;
        g_l2_log_lv = level;
        g_enable_phy_info_log = (g_phy_log_lv > ALOG_LV_WARN);
        g_enable_L2_info_log = (g_l2_log_lv > ALOG_LV_WARN);
    } else if (AMOD_PHY == mod) {
        g_phy_log_lv = level;
        g_enable_phy_info_log = (g_phy_log_lv > ALOG_LV_WARN);
    } else if (AMOD_EL2 == mod) {
        g_l2_log_lv = level;
        g_enable_L2_info_log = (g_l2_log_lv > ALOG_LV_WARN);
    }
#endif

    adbg_save_debug_nv_cfg(TRUE);
}

void alog_just_set_log_level(u8 mod, u8 level)
{
    ALOG_SET_MODE_LEVEL(mod, level);
}

bool alog_trace_filter_out(u8 mod, u8 level)
{
    return (level > ALOG_GET_MODE_LEVEL(mod));
}

void alog_set_log_level_no_save(u8 mod, u8 level)
{
    ALOG_SET_MODE_LEVEL(mod, level);
}

