/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the boot mode interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_BOOT_MODE_H__
#define __AIC_BOOT_MODE_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_bitmap.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
/*
 * boot reason define, please refence the define in file
 * bootloader\include\platform.h
 */
typedef enum {
    BOOT_REASON_UNKNOW = 0,
    BOOT_REASON_WARMRST,//software reset at^reset/assert reset
    BOOT_REASON_POWERKEY,//key reset
    BOOT_REASON_CHARGE,
    BOOT_REASON_ALARM_UPS,
    BOOT_REASON_ALARM_RTC,
    BOOT_REASON_LONG_PRESS,
    BOOT_REASON_MAX
} BOOT_REASON_E;

/*
 * boot mode define
 * @BOOT_MODE_NORMAL: normal boot mode.
 * @BOOT_MODE_BASIC:  basic boot mode, only boot basic sys.
 * @BOOT_MODE_CHARGE: charge boot mode.
 * @BOOT_MODE_GXEM:   guixin engineering mode.
 */
typedef enum {
    BOOT_MODE_NORMAL = 0,
    BOOT_MODE_BASIC,
    BOOT_MODE_CHARGE,
    BOOT_MODE_GXEM,
    BOOT_MODE_MAX,
} BOOT_MODE_E;

/*
 * guixin engineering mode detect device define
 */
typedef enum {
    GXEM_DEV_NULL = 0,
    GXEM_DEV_USB,
    GXEM_DEV_UART,
    GXEM_DEV_FACTORY,
    GXEM_DEV_KEY,
    GXEM_DEV_MAX
} GXEM_DEV_E;

/**
 * aic_boot_reason_get - get the boot reason.
 *
 * Parameter:
 *
 * Return:
 *      boot reson
 */
BOOT_REASON_E aic_boot_reason_get();

/**
 * aic_boot_mode_set - set boot mode.
 *
 * Parameter:
 * @boot_mode: boot mode
 *
 * Return:
 *      AIC_SUCCESS is success, other fail.
 */
int aic_boot_mode_set(BOOT_MODE_E boot_mode);

/**
 * aic_boot_mode_get - get boot mode.
 *
 * Parameter:
 *
 * Return:
 *      boot mode
 */
BOOT_MODE_E aic_boot_mode_get();

#define aic_boot_mode_is_normal()   (BOOT_MODE_NORMAL == aic_boot_mode_get())
#define aic_boot_mode_is_basic()    (BOOT_MODE_BASIC == aic_boot_mode_get())
#define aic_boot_mode_is_gxem()     (BOOT_MODE_GXEM == aic_boot_mode_get())

#ifdef CONFIG_AIC_SYSOFF_CHARGE
#define aic_boot_mode_is_charge()   (BOOT_MODE_CHARGE == aic_boot_mode_get())
#else
#define aic_boot_mode_is_charge()   (FALSE)
#endif

#ifdef CONFIG_AIC_CTRL_D2D
bool aic_boot_mode_is_no_sw_no_pwr(void);
#endif

/**
 * aic_gxem_dev_set - set gxem device.
 *
 * Parameter:
 * @gxem_dev: gxem device
 *
 * Return:
 *      AIC_SUCCESS is success, other fail.
 */
int aic_gxem_dev_set(GXEM_DEV_E gxem_dev);

/**
 * aic_gxem_dev_get - get gxem device.
 *
 * Parameter:
 *
 * Return:
 *      gxem device
 */
GXEM_DEV_E aic_gxem_dev_get();

/**
 * aic_boot_mode_init - boot mode init.
 *
 * Parameter:
 *
 * Return:
 *
 */
void aic_boot_mode_init();

/**
 * aic_boot_ext_img_check_fail
 *
 * Parameter:
 *
 * Return: if TRUE, check fail, can't boot to normal mode.
 *
 */
bool aic_boot_ext_img_check_fail(void);

/**
 * aic_boot_is_ext_img_check_fail, just return the check result.
 *
 * Parameter:
 *
 * Return: if TRUE, check fail, else check succ.
 *
 */
bool aic_boot_is_ext_img_check_fail(void);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_BOOT_MODE_H__ */

