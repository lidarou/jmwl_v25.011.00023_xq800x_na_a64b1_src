/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the auto init private api, realize the
 *    levlex emulation.
 */

#ifndef __AIC_AUTO_INIT_PRIVATE_H__
#define __AIC_AUTO_INIT_PRIVATE_H__

#ifdef CONFIG_PLAT_AUTO_INIT

#include <aic_type.h>
#include <aic_init.h>
#include <sys_mem_api.h>

/**
 * __init_aic_x_end is followed closely by the end of
 * the section .auto_initx, is used to emulate the
 * level(x + 1).
*/

#ifdef CONFIG_AUTO_INIT_HAS_NAME
#define AIC_ADD_INIT_LEVEL_END(level) \
static AUTO_INIT_T __init_aic_##level##_end \
    AIC_SECTION(".auto_init"#level".end") \
    = { NULL, NULL, 0}
#else
#define AIC_ADD_INIT_LEVEL_END(level) \
static AUTO_INIT_T __init_aic_##level##_end \
    AIC_SECTION(".auto_init"#level".end") \
    = { NULL, 0}
#endif

extern init_func   g_init_func;
extern BOOT_MODE_E aic_boot_mode;

#ifdef CONFIG_SYS_HIGH_PERF
#define AIC_LEVEL_INIT(level) \
do { \
    const AUTO_INIT_T *init; \
    \
    alog_info(#level " LV init.\n"); \
    for (init = &__init_aic_##level##_end + 1; init->func; init++) \
    { \
        if (init->boot_dis &  BIT(aic_boot_mode)) \
            continue; \
    \
        if ((aic_boot_mode == BOOT_MODE_BASIC) && \
            sys_is_belong_extend_image(init->func)) { \
            alog_error("init err: func 0x%x is in extend img.\n", init->func); \
            continue; \
        } \
    \
        g_init_func = init->func;\
        (init->func)(); \
    } \
} while (0)
#else
#define AIC_LEVEL_INIT(level) aic_levle_init(#level, &__init_aic_##level##_end)
#endif

#define AIC_EARLY_INIT()   AIC_LEVEL_INIT(0)
#define AIC_DRV_P1_INIT()  AIC_LEVEL_INIT(1)
#define AIC_DRV_P2_INIT()  AIC_LEVEL_INIT(2)
#define AIC_COMP_P1_INIT() AIC_LEVEL_INIT(3)
#define AIC_COMP_P2_INIT() AIC_LEVEL_INIT(4)
#define AIC_APP_P1_INIT()  AIC_LEVEL_INIT(5)
#define AIC_APP_P2_INIT()  AIC_LEVEL_INIT(6)
#define AIC_MODEM_INIT()   AIC_LEVEL_INIT(7)

#define AIC_DRV_L1_INIT()  AIC_LEVEL_INIT(8)
#define AIC_DRV_L2_INIT()  AIC_LEVEL_INIT(9)
#define AIC_COMP_L1_INIT() AIC_LEVEL_INIT(a)
#define AIC_COMP_L2_INIT() AIC_LEVEL_INIT(b)
#define AIC_APP_L1_INIT()  AIC_LEVEL_INIT(c)
#define AIC_APP_L2_INIT()  AIC_LEVEL_INIT(d)

/*
 * Only init the disable function of the specified mode.
 */
#define AIC_LEVEL_INIT_DIS(level, mode) \
do { \
    const AUTO_INIT_T *init; \
    \
    for (init = &__init_aic_##level##_end + 1; init->func; init++) \
    { \
        if (init->boot_dis &  BIT(mode)) \
            (init->func)(); \
    } \
} while (0)

#define AIC_EARLY_INIT_DIS(mode)   AIC_LEVEL_INIT_DIS(0, mode)
#define AIC_DRV_P1_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(1, mode)
#define AIC_DRV_P2_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(2, mode)
#define AIC_COMP_P1_INIT_DIS(mode) AIC_LEVEL_INIT_DIS(3, mode)
#define AIC_COMP_P2_INIT_DIS(mode) AIC_LEVEL_INIT_DIS(4, mode)
#define AIC_APP_P1_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(5, mode)
#define AIC_APP_P2_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(6, mode)
#define AIC_MODEM_INIT_DIS(mode)   AIC_LEVEL_INIT_DIS(7, mode)

#define AIC_DRV_L1_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(8, mode)
#define AIC_DRV_L2_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(9, mode)
#define AIC_COMP_L1_INIT_DIS(mode) AIC_LEVEL_INIT_DIS(a, mode)
#define AIC_COMP_L2_INIT_DIS(mode) AIC_LEVEL_INIT_DIS(b, mode)
#define AIC_APP_L1_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(c, mode)
#define AIC_APP_L2_INIT_DIS(mode)  AIC_LEVEL_INIT_DIS(d, mode)

#endif /* CONFIG_PLAT_AUTO_INIT */

#endif /* __AIC_AUTO_INIT_PRIVATE_H__ */

