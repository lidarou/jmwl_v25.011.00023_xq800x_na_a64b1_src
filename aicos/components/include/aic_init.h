/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the auto init Interface.
 */


#ifndef __AIC_INIT_H__
#define __AIC_INIT_H__

#include <aic_type.h>
#include <aic_boot_mode.h>
#include <alog_api.h>

/*
 * this below macros define must consitant with
 * the BOOT_MODE_E define in aic_boot_mode.h.
 */
#define BOOT_MASK       0xFF
#define ENABLE_ALL      0x00

#define DIS_NORMAL      BIT_0
#define DIS_BASIC       BIT_1
#define DIS_CHARGE      BIT_2
#define DIS_GXEM        BIT_3

#define NORMAL_ONLY     (BOOT_MASK) & (~DIS_NORMAL)
#define BASIC_ONLY      (BOOT_MASK) & (~DIS_BASIC)
#define CHARGE_ONLY     (BOOT_MASK) & (~DIS_CHARGE)
#define GXEM_ONLY       (BOOT_MASK) & (~DIS_GXEM)

#ifdef CONFIG_PLAT_AUTO_INIT
/* Define auto init function type. */
typedef int (*init_func)(void);

#ifdef CONFIG_AUTO_INIT_HAS_NAME
typedef struct AUTO_INIT_DATA {
    init_func   func;
    char        *name;
    u32          boot_dis; /* disable some boot mode. */
} AUTO_INIT_T;

/*
 * Define a function pointer local in a .auto_initx section.
 * will init in all boot mode.
 */
#define AIC_INIT_FUNC_DEFINE(level, func) \
static AUTO_INIT_T __init_##func       AIC_SECTION(".auto_init"level) = { \
        func,  #func, ENABLE_ALL\
    }

/*
 * Define a function pointer local in a .auto_initx section. 
 * will init disable some boot mode as the dis param.
 */
#define AIC_INIT_EX_FUNC_DEFINE(level, func, dis) \
static AUTO_INIT_T __init_##func       AIC_SECTION(".auto_init"level) = { \
        func,  #func, dis\
    }
#else
typedef struct AUTO_INIT_DATA {
    init_func   func;
    u32          boot_dis; /* disable some boot mode. */
} AUTO_INIT_T;

/*
 * Define a function pointer local in a .auto_initx section.
 * will init in all boot mode.
 */
#define AIC_INIT_FUNC_DEFINE(level, func) \
static AUTO_INIT_T __init_##func       AIC_SECTION(".auto_init"level) = { \
        func, ENABLE_ALL\
    }

/*
 * Define a function pointer local in a .auto_initx section. 
 * will init disable some boot mode as the dis param.
 */
#define AIC_INIT_EX_FUNC_DEFINE(level, func, dis) \
static AUTO_INIT_T __init_##func       AIC_SECTION(".auto_init"level) = { \
        func, dis\
    }

#endif

void aic_delete_init_task(void);

#else
/* dummy auto init macors. */
#define aic_delete_init_task()
#define AIC_INIT_FUNC_DEFINE(level, func)
#define AIC_INIT_EX_FUNC_DEFINE(level, func, dis)
#endif /* CONFIG_PLAT_AUTO_INIT */

/*
 * All the normal init level are be called int init context,
 * In these function, can't called apis that cause system scheduing.
 */

/* Some early public module(uart, log, mbox, watchdog). */
#define AIC_EARLY_INIT_EXPORT(func)             AIC_INIT_FUNC_DEFINE("1", func)
#define AIC_EARLY_INIT_EX_EXPORT(func, dis)     AIC_INIT_EX_FUNC_DEFINE("1", func, dis)

/* Some device drivers(int, gpio) that are dependent on by other drivers.*/
#define AIC_DRV_P1_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("2", func)
#define AIC_DRV_P1_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("2", func, dis)

/* Normal device drivers.*/
#define AIC_DRV_P2_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("3", func)
#define AIC_DRV_P2_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("3", func, dis)

/* Some componets(log, nv, mux, aipc) that are dependent on by other componets.*/
#define AIC_COMP_P1_INIT_EXPORT(func)           AIC_INIT_FUNC_DEFINE("4", func)
#define AIC_COMP_P1_INIT_EX_EXPORT(func, dis)   AIC_INIT_EX_FUNC_DEFINE("4", func, dis)

/* Normal  componets.*/
#define AIC_COMP_P2_INIT_EXPORT(func)           AIC_INIT_FUNC_DEFINE("5", func)
#define AIC_COMP_P2_INIT_EX_EXPORT(func, dis)   AIC_INIT_EX_FUNC_DEFINE("5", func, dis)

/* Some app that are dependent on by other app.*/
#define AIC_APP_P1_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("6", func)
#define AIC_APP_P1_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("6", func, dis)

/* Normal app.*/
#define AIC_APP_P2_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("7", func)
#define AIC_APP_P2_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("7", func, dis)

/* modem init. */
#define AIC_MODEM_INIT_EXPORT(func)             AIC_INIT_FUNC_DEFINE("8", func)
#define AIC_MODEM_INIT_EX_EXPORT(func, dis)     AIC_INIT_EX_FUNC_DEFINE("8", func, dis)

/*
 * All the later init level are  task context init, can use any api,
 * but can't be blocked to long, it will block other module init.
 */

/* later drv init p1.*/
#define AIC_DRV_L1_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("9", func)
#define AIC_DRV_L1_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("9", func, dis)

/* later drv init p2.*/
#define AIC_DRV_L2_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("a", func)
#define AIC_DRV_L2_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("a", func, dis)

/* later components init p1.*/
#define AIC_COMP_L1_INIT_EXPORT(func)           AIC_INIT_FUNC_DEFINE("b", func)
#define AIC_COMP_L1_INIT_EX_EXPORT(func, dis)   AIC_INIT_EX_FUNC_DEFINE("b", func, dis)

/* later components init p2.*/
#define AIC_COMP_L2_INIT_EXPORT(func)           AIC_INIT_FUNC_DEFINE("c", func)
#define AIC_COMP_L2_INIT_EX_EXPORT(func, dis)   AIC_INIT_EX_FUNC_DEFINE("c", func, dis)

/* later app init p1.*/
#define AIC_APP_L1_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("d", func)
#define AIC_APP_L1_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("d", func, dis)

/* later app init p2.*/
#define AIC_APP_L2_INIT_EXPORT(func)            AIC_INIT_FUNC_DEFINE("e", func)
#define AIC_APP_L2_INIT_EX_EXPORT(func, dis)    AIC_INIT_EX_FUNC_DEFINE("e", func, dis)

/***************************************************************************************
 * more extend macros define
 ***************************************************************************************/
/* only normal mode init define.*/
#define AIC_NORMAL_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_NORMAL_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)

/* disable normal mode init define.*/
#define DIS_NORMAL_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, DIS_NORMAL)
#define DIS_NORMAL_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, DIS_NORMAL)

/* only basic mode init define.*/
#define AIC_BASIC_EARLY_INIT_EXPORT(func)       AIC_EARLY_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_DRV_P1_INIT_EXPORT(func)      AIC_DRV_P1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_DRV_P2_INIT_EXPORT(func)      AIC_DRV_P2_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_COMP_P1_INIT_EXPORT(func)     AIC_COMP_P1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_COMP_P2_INIT_EXPORT(func)     AIC_COMP_P2_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_APP_P1_INIT_EXPORT(func)      AIC_APP_P1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_APP_P2_INIT_EXPORT(func)      AIC_APP_P2_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_DRV_L1_INIT_EXPORT(func)      AIC_DRV_L1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_DRV_L2_INIT_EXPORT(func)      AIC_DRV_L2_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_COMP_L1_INIT_EXPORT(func)     AIC_COMP_L1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_COMP_L2_INIT_EXPORT(func)     AIC_COMP_L2_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_APP_L1_INIT_EXPORT(func)      AIC_APP_L1_INIT_EX_EXPORT(func, BASIC_ONLY)
#define AIC_BASIC_APP_L2_INIT_EXPORT(func)      AIC_APP_L2_INIT_EX_EXPORT(func, BASIC_ONLY)

/* disable basic mode init define.*/
#define DIS_BASIC_EARLY_INIT_EXPORT(func)       AIC_EARLY_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_DRV_P1_INIT_EXPORT(func)      AIC_DRV_P1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_DRV_P2_INIT_EXPORT(func)      AIC_DRV_P2_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_COMP_P1_INIT_EXPORT(func)     AIC_COMP_P1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_COMP_P2_INIT_EXPORT(func)     AIC_COMP_P2_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_APP_P1_INIT_EXPORT(func)      AIC_APP_P1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_APP_P2_INIT_EXPORT(func)      AIC_APP_P2_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_DRV_L1_INIT_EXPORT(func)      AIC_DRV_L1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_DRV_L2_INIT_EXPORT(func)      AIC_DRV_L2_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_COMP_L1_INIT_EXPORT(func)     AIC_COMP_L1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_COMP_L2_INIT_EXPORT(func)     AIC_COMP_L2_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_APP_L1_INIT_EXPORT(func)      AIC_APP_L1_INIT_EX_EXPORT(func, DIS_BASIC)
#define DIS_BASIC_APP_L2_INIT_EXPORT(func)      AIC_APP_L2_INIT_EX_EXPORT(func, DIS_BASIC)

/* only charge init define.*/
#define AIC_CHARGE_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, CHARGE_ONLY)
#define AIC_CHARGE_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, CHARGE_ONLY)

/* disable charge init define.*/
#define DIS_CHARGE_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, DIS_CHARGE)
#define DIS_CHARGE_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, DIS_CHARGE)

/* only gxem init define.*/
#define AIC_GXEM_EARLY_INIT_EXPORT(func)        AIC_EARLY_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_DRV_P1_INIT_EXPORT(func)       AIC_DRV_P1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_DRV_P2_INIT_EXPORT(func)       AIC_DRV_P2_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_COMP_P1_INIT_EXPORT(func)      AIC_COMP_P1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_COMP_P2_INIT_EXPORT(func)      AIC_COMP_P2_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_APP_P1_INIT_EXPORT(func)       AIC_APP_P1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_APP_P2_INIT_EXPORT(func)       AIC_APP_P2_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_DRV_L1_INIT_EXPORT(func)       AIC_DRV_L1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_DRV_L2_INIT_EXPORT(func)       AIC_DRV_L2_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_COMP_L1_INIT_EXPORT(func)      AIC_COMP_L1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_COMP_L2_INIT_EXPORT(func)      AIC_COMP_L2_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_APP_L1_INIT_EXPORT(func)       AIC_APP_L1_INIT_EX_EXPORT(func, GXEM_ONLY)
#define AIC_GXEM_APP_L2_INIT_EXPORT(func)       AIC_APP_L2_INIT_EX_EXPORT(func, GXEM_ONLY)

/* disable gxem init define.*/
#define DIS_GXEM_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, DIS_GXEM)
#define DIS_GXEM_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, DIS_GXEM)

/* extend image init define, = normal only.*/
#define AIC_EXTIMG_EARLY_INIT_EXPORT(func)      AIC_EARLY_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_DRV_P1_INIT_EXPORT(func)     AIC_DRV_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_DRV_P2_INIT_EXPORT(func)     AIC_DRV_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_COMP_P1_INIT_EXPORT(func)    AIC_COMP_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_COMP_P2_INIT_EXPORT(func)    AIC_COMP_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_APP_P1_INIT_EXPORT(func)     AIC_APP_P1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_APP_P2_INIT_EXPORT(func)     AIC_APP_P2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_DRV_L1_INIT_EXPORT(func)     AIC_DRV_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_DRV_L2_INIT_EXPORT(func)     AIC_DRV_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_COMP_L1_INIT_EXPORT(func)    AIC_COMP_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_COMP_L2_INIT_EXPORT(func)    AIC_COMP_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_APP_L1_INIT_EXPORT(func)     AIC_APP_L1_INIT_EX_EXPORT(func, NORMAL_ONLY)
#define AIC_EXTIMG_APP_L2_INIT_EXPORT(func)     AIC_APP_L2_INIT_EX_EXPORT(func, NORMAL_ONLY)

void aic_init_charge_dis(void);
#endif /* __AIC_INIT_H__ */

