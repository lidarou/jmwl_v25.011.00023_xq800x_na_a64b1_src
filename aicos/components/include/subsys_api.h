
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the api header file of the subsys.
 */
#ifndef __SUBSYS_API_H__
#define __SUBSYS_API_H__

#include <aic_sys.h>

/* subsys define */
#define SUBSYS_START (AIC_SYS_CP)
#define SUBSYS_END (AIC_SYS_NR - 1)

/*
 * The feature CONFIG_CP_RESET_ONLY_CP.
 * open it if cp assert will just reset cp firstly,
 * if can't reset succ, will reset the chip.
 */
#define CONFIG_CP_RESET_ONLY_CP     0

/*
 * subsys state define:
 * Init: The defaul state before boot.
 * BOOTING: Is be boot, but not receive the 'OK'response.
 * ALIVE: Received the 'OK' response.
 * ASSERT: Received the ASSERT response.
 * HUNG: Received the watchdog interrupt.
 * BLOCK: Ril not respond at command, notify subsys.
 */
typedef enum {
    SUBSYS_STATE_INIT = 0,
    SUBSYS_STATE_BOOTING,
    SUBSYS_STATE_ALIVE,
    SUBSYS_STATE_ASSERT,
    SUBSYS_STATE_RESET,
    SUBSYS_STATE_HUNG,
    SUBSYS_STATE_BLOCK,
    SUBSYS_STATE_WDG,
    SUBSYS_STATE_STOP,
    SUBSYS_STATE_POWER_OFF
} subsys_state_t;

/* subsys state channe notify function define */
typedef void (*subsys_state_nf)(subsys_state_t state, void *p);

enum {
    SUBSYS_MAIN_MSG_ALIVE = 0,
    SUBSYS_MAIN_MSG_ASSERT,
    SUBSYS_MAIN_MSG_RESET,
    SUBSYS_MAIN_MSG_WDG,
    SUBSYS_MAIN_MSG_BLOCK,
    SUBSYS_MAIN_MSG_STOP,
    SUBSYS_MAIN_MSG_START,
    SUBSYS_MAIN_MSG_POWER_OFF,
    SUBSYS_MAIN_MSG_POWER_ON,
    SUBSYS_MAIN_MSG_DUMP
};

void subsys_main_send_msg(u16 sys, u16 action);

/**
 * subsys_stop - Stop the specified sys.
 *
 * @sys: subsys id.
 */
#define subsys_stop(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_STOP)

/**
 * subsys_start - Start the specified sys.
 *
 * @sys: subsys id.
 */
#define subsys_start(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_START)

/**
 * subsys_block - notify the subsys man that the subsys is blocked.
 * such as cp sys, ril send at to cp can't get response.
 *
 * @sys: subsys id.
 */
#define subsys_block(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_BLOCK)

/**
 * subsys_power_off - power off the sys.
 *
 * @sys: subsys id.
 */
#define subsys_power_off(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_POWER_OFF)

/**
 * subsys_power_on - power on the sys.
 *
 * @sys: subsys id.
 */
#define subsys_power_on(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_POWER_ON)

/**
 * subsys_dump - dump the  on the sys.
 *
 * @sys: subsys id.
 */
#define subsys_dump(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_DUMP)

/**
 * subsys_set_reset_state - Set sys to reset state..
 *
 * @sys: subsys id.
 */
#define subsys_set_reset_state(sys)    subsys_main_send_msg(sys, SUBSYS_MAIN_MSG_RESET)

/**
 * subsys_wait_poweroff - wait the sys power off.
 *
 * @sys: subsys id.
 * @timeout: max wait time(ms), timeout will reboot chip.
 */
void subsys_wait_poweroff(u8 sys, int timeout);

/**
 * subsys_add_state_notify - Add a notify callback function.
 * When the state of specified sys change, will call it.
 *
 * @sys: subsys id.
 * @state_cb: the callback function and data pointer.
 */
void subsys_add_state_notify(u8 sys, subsys_state_nf nf, void *p);

/**
 * subsys_get_state - Get the specify sys state.
 *
 * @sys: subsys id.
 */
u8 subsys_get_state(u8 sys);

/**
 * subsys_mpu_config_sys - mpu dynamic config the specify sys psram region.
 *
 * @sys: subsys id.
 * @return: the mpu index. >=0 succ, can use it to unconfig the mpu.
 */
int subsys_mpu_config_sys(u8 sys);

/**
 * subsys_mpu_config_region - mpu dynamic config the specify region attr.
 *
 * @region_attr: the attribution of the region..
 * @return: the mpu index. >=0 succ, can use it to unconfig the mpu.
 */
int subsys_mpu_config_region(u32 region_attr);

/**
 * subsys_mpu_unconfig_region - mpu dynamic unconfig the specify index.
 *
 * @index: the index the return by subsys_mpu_config_region.
 */

#ifdef WIN32
#define subsys_mpu_unconfig_region(i)
#else
#include <mpu.h>
#define subsys_mpu_unconfig_region(i) mpu_dynamic_unconf_region(i, FALSE)
#endif

#endif /* __SUBSYS_API_H__ */

