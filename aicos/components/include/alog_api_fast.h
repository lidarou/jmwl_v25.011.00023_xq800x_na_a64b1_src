/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines fast log api of the the alog.
 */
#ifndef __ALOG_API_FAST_H__
#define __ALOG_API_FAST_H__

#include <aic_type.h>
#include <aic_module.h>
#include <alog_api.h>

#if defined (CONFIG_SUBSYS_CP) || defined(WIN32)
/*
 * ALOG_FAST_API_FEATURE
 * In cp open it to provide fast log api for phy and l2 log.
 */
#define ALOG_FAST_API_FEATURE
#endif

/*
 * extern or fast log module switch.
 */
extern u8 g_enable_phy_info_log;
extern u8 g_enable_L2_info_log;
extern u8 g_phy_log_lv;
extern u8 g_l2_log_lv;

#define alog_dis_phy_fast_info_log  do {g_enable_phy_info_log = 0;} while (0)
#define alog_en_phy_fast_info_log   do { \
    if (g_phy_log_lv > ALOG_LV_WARN) \
        g_enable_phy_info_log = 1; \
} while (0)

#define alog_dis_l2_fast_info_log   do {g_enable_L2_info_log = 0;} while (0)
#define alog_en_l2_fast_info_log   do { \
    if (g_l2_log_lv > ALOG_LV_WARN) \
        g_enable_L2_info_log = 1; \
} while (0)

#define alog_dis_fast_log()         do { \
    g_l2_log_lv = 0; \
    g_phy_log_lv = 0; \
    g_enable_L2_info_log = 0; \
    g_enable_phy_info_log = 0; \
} while (0)

#define alog_en_fast_log()          do { \
    g_l2_log_lv = (u8)ALOG_GET_MODE_LEVEL(AMOD_EL2); \
    g_phy_log_lv = (u8)ALOG_GET_MODE_LEVEL(AMOD_PHY); \
    alog_en_l2_fast_info_log; \
    alog_en_phy_fast_info_log; \
} while (0)

#ifdef ALOG_FAST_API_FEATURE
/*
 * fast log api.
 */
PLAT_TCM_CODE void alog_output_fast(u8 mod, const char *fmt, ...);
PLAT_TCM_CODE void alog_idx_output_fast(u8 mod, u32 fmt, const char *pf, ...);
#else
#define alog_output_fast(mod, fmt, ...)     alog_output(mod, ALOG_LV_INFO, fmt, ##__VA_ARGS__)
#define alog_idx_output_fast(mod, fmt, ...) alog_idx_output(mod, ALOG_LV_INFO, fmt, ##__VA_ARGS__)
#endif /* ALOG_FAST_API_FEATURE */

/*
 * phy fast api define.
 */
#define PHY_FAST_DEBUG(...)

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define PHY_FAST_INFO(...)
#define PHY_FAST_IFID(...)
#else
#define PHY_FAST_INFO(fmt, ...) do { \
    if (g_enable_phy_info_log) {\
        alog_output_fast(AMOD_PHY, fmt, ##__VA_ARGS__);\
    } \
} while (0)

#define PHY_FAST_IFID(file, fmt, pf, ...) do { \
    if (g_enable_phy_info_log) {\
        alog_idx_output_fast(AMOD_PHY, fmt, pf, ##__VA_ARGS__);\
    } \
} while (0)
#endif

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define PHY_FAST_ERROR(...)
#define PHY_FAST_ERRID(...)
#else
#define PHY_FAST_ERROR(fmt, ...) do { \
    if (g_phy_log_lv > ALOG_LV_NONE) \
        alog_output_fast(AMOD_PHY, fmt, ##__VA_ARGS__);\
} while (0)

#define PHY_FAST_ERRID(file, fmt, pf, ...) do { \
    if (g_phy_log_lv > ALOG_LV_NONE) \
        alog_idx_output_fast(AMOD_PHY, fmt, pf, ##__VA_ARGS__);\
} while (0)
#endif

/*
 * L2 fast api define.
 */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define L2_FAST_INFO(...)
#define L2_FAST_IFID(...)
#else
#define L2_FAST_INFO(fmt, ...) do { \
    if (g_enable_L2_info_log) {\
        alog_output_fast(AMOD_EL2, fmt, ##__VA_ARGS__);\
    } \
} while (0)

#define L2_FAST_IFID(file, fmt, pf, ...) do { \
    if (g_enable_L2_info_log) {\
        alog_idx_output_fast(AMOD_EL2, fmt, pf, ##__VA_ARGS__);\
    } \
} while (0)
#endif

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define L2_FAST_ERROR(...)
#define L2_FAST_ERRID(...)
#else
#define L2_FAST_ERROR(fmt, ...) do { \
    if (g_l2_log_lv > ALOG_LV_NONE) \
        alog_output_fast(AMOD_EL2, fmt, ##__VA_ARGS__);\
} while (0)

#define L2_FAST_ERRID(file, fmt, pf, ...) do { \
    if (g_l2_log_lv > ALOG_LV_NONE) \
        alog_idx_output_fast(AMOD_EL2, fmt, pf, ##__VA_ARGS__);\
} while (0)
#endif

#ifdef CONFIG_ALOG_TRACE_IDX
#define phy_fast_debug(id, xxx, fmt, ...)   aidx_debug(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define phy_fast_info(id, xxx, fmt, ...)    PHY_FAST_IFID(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define phy_fast_error(id, xxx, fmt, ...)   PHY_FAST_ERRID(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)

#define l2_fast_debug(id, xxx, fmt, ...)    aidx_debug(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define l2_fast_info(id, xxx, fmt, ...)     L2_FAST_IFID(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define l2_fast_error(id, xxx, fmt, ...)    L2_FAST_ERRID(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#else
#define phy_fast_debug(id, xxx, fmt, ...)   alog_debug(fmt, ##__VA_ARGS__)
#define phy_fast_info(id, xxx, fmt, ...)    PHY_FAST_INFO(fmt, ##__VA_ARGS__)
#define phy_fast_error(id, xxx, fmt, ...)   PHY_FAST_ERROR(fmt, ##__VA_ARGS__)

#define l2_fast_debug(id, xxx, fmt, ...)    alog_debug(fmt, ##__VA_ARGS__)
#define l2_fast_info(id, xxx, fmt, ...)     L2_FAST_INFO(fmt, ##__VA_ARGS__)
#define l2_fast_error(id, xxx, fmt, ...)    L2_FAST_ERROR(fmt, ##__VA_ARGS__)

#endif

#endif /* __ALOG_API_FAST_H__ */

