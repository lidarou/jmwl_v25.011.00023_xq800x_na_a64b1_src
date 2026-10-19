/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the boot mode interface.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x099c33
#define LOG_MODULE    AMOD_INIT

#include <stdlib.h>
#include <aic_errno.h>
#include <aic_assert.h>
#include <aic_boot_mode.h>
#include <aic_cmdline.h>
#include <aic_osa.h>
#include <mpu.h>
#include <subsys_api.h>

#ifdef CONFIG_FOTA_SUPPORT
#include <sys_mem_api.h>
#include <timer_inc.h>
#include <fota.h>
#endif

#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#include <sysoff.h>
#endif
#ifndef WIN32
#include <glb_ups.h>
#endif
/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/
BOOT_REASON_E aic_boot_reason = BOOT_REASON_UNKNOW;
BOOT_MODE_E aic_boot_mode = BOOT_MODE_NORMAL;
GXEM_DEV_E  aic_gxem_dev = GXEM_DEV_NULL;
#ifdef CONFIG_AIC_CTRL_D2D
bool s_reboot_hw;
#endif
bool g_ext_check_fail;

#ifdef CONFIG_FOTA_SUPPORT
bool aic_boot_ext_img_check_fail(void)
{
#ifdef  CONFIG_MULTI_BIN_SUPPORT
    addr_t addr;
    size_t size;
    u16 *check_base;
    u16 checksum, calcsum;
    u32 len, tick;

    sys_get_ap_extend_img_actual_area(&addr, &size);
    if (size == 0) {
        aicx_info(0x1d7005, "", "extend image is invalid.\n");
        return TRUE;
    }

    check_base = (u16 *)(addr + 2);
    len = size - 2;

    /* the tail has 2 byte checksum. */
    checksum = *(u16 *)(addr + size);

    calcsum = sys_get_extend_img_cheksum_in_base();
    if (checksum != calcsum) {
        aicx_info(0x17cb1b, "xx", "basic(0x%x) and extend(0x%x) is not matched.\n",
                  calcsum, checksum);
        return TRUE;
    }

    tick = aic_get_tick();
    calcsum = fota_checksum(check_base, len);
    tick = aic_get_tick() - tick;
    aicx_info(0x178a75, "xxdxd", "checksum(0x%x), calcsum(0x%x), take %d ticks, base=0x%x, len=%d.\n",
              checksum, calcsum, tick, check_base, len);

    return (calcsum != checksum);
#endif /* CONFIG_MULTI_BIN_SUPPORT */

    return FALSE;
}

bool aic_boot_is_ext_img_check_fail(void)
{
    return g_ext_check_fail;
}
#endif

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_boot_reason_get - get the boot reason.
 *
 * Parameter:
 *
 * Return:
 *      boot reson
 */
BOOT_REASON_E aic_boot_reason_get()
{
    return aic_boot_reason;
}

/**
 * aic_boot_mode_set - set boot mode.
 *
 * Parameter:
 * @boot_mode: boot mode
 *
 * Return:
 *      AIC_SUCCESS is success, other fail.
 */
int aic_boot_mode_set(BOOT_MODE_E boot_mode)
{
    aicx_info(0x0a83b8, "x", "boot mode(0x%x)!\n", boot_mode);

    if (boot_mode >= BOOT_MODE_MAX) {
        aicx_error(0x15f732, "x", "boot mode(0x%x) set error!\n", boot_mode);
        return AIC_EINVAL;
    }

    aic_boot_mode = boot_mode;

#ifdef CONFIG_AIC_CTRL_SUPPORT
    aic_ctrl_mode_notify(boot_mode);
#endif

    return AIC_SUCCESS;
}

/**
 * aic_boot_mode_get - get boot mode.
 *
 * Parameter:
 *
 * Return:
 *      boot mode
 */
BOOT_MODE_E aic_boot_mode_get()
{
    return aic_boot_mode;
}

/**
 * aic_gxem_dev_set - set gxem device.
 *
 * Parameter:
 * @gxem_dev: gxem device
 *
 * Return:
 *      AIC_SUCCESS is success, other fail.
 */
int aic_gxem_dev_set(GXEM_DEV_E gxem_dev)
{
    aicx_info(0x0be238, "x", "gxem device(0x%x)!\n", gxem_dev);

    if (gxem_dev >= GXEM_DEV_MAX) {
        aicx_error(0x149981, "x", "gxem device(0x%x) set error!\n", gxem_dev);
        return AIC_EINVAL;
    }

    aic_gxem_dev = gxem_dev;

    return AIC_SUCCESS;
}

/**
 * aic_gxem_dev_get - get gxem device.
 *
 * Parameter:
 *
 * Return:
 *      gxem device
 */
GXEM_DEV_E aic_gxem_dev_get()
{
    return aic_gxem_dev;
}

static void aic_boot_enter_basic_mode(void)
{
    aic_boot_mode = BOOT_MODE_BASIC;

#ifdef CONFIG_FOTA_SUPPORT
    sys_reconfig_extend_image();
    sys_reserve_save_basic_diff_image_ram();
#ifdef CONFIG_SUPPORT_ADSP
    /* mpu config adsp region. */
    subsys_mpu_config_sys(AIC_SYS_ADSP);
#endif
#endif
}

static void aic_boot_enter_gxem_mode(char *cmd_str)
{
    u8 mode, dev;

    mode = atoi(cmd_str);
    AICX_ASSERT_IFX(mode == 1, 0x0ce7c8, "d", "mode(%d) error", mode);
    cmd_str += 2;
    aic_boot_mode = BOOT_MODE_GXEM;

    dev = atoi(cmd_str);
    aic_gxem_dev_set(dev);
}

static void aic_boot_boot_mode_check(void)
{
    char * boot_str = aic_cmdline_get_value(cl_key_bootmode);
    if (!boot_str)
        return;

    if (0 == strcmp(boot_str, "bsc")) {
        aic_boot_enter_basic_mode();
    } else if ( 0 == strcmp(boot_str, "chg")) {
#ifdef CONFIG_AIC_SYSOFF_CHARGE
        aic_boot_mode = BOOT_MODE_CHARGE;
#else
        aic_boot_mode = BOOT_MODE_NORMAL;
#endif
        aic_boot_reason = BOOT_REASON_CHARGE;
    } else if (0 == strcmp(boot_str, "gxm")) {
        aic_boot_mode = BOOT_MODE_GXEM;
    } else {
        aic_boot_mode = BOOT_MODE_NORMAL;
        if (0 == strcmp(boot_str, "ups"))
            aic_boot_reason = BOOT_REASON_ALARM_UPS;
        else if ( 0 == strcmp(boot_str, "rtc"))
            aic_boot_reason = BOOT_REASON_ALARM_RTC;
        else
            aic_boot_reason = BOOT_REASON_UNKNOW;
    }
}

static void aic_boot_reason_check(void)
{
    int boot_mode = aic_cmdline_get_value_int(cl_key_pwr_sw);
    int poweron_sts = aic_cmdline_get_value_int(cl_key_pwr_on);

    if(aic_boot_reason == BOOT_REASON_UNKNOW)
    {
        if((boot_mode & ~(BIT_0)) || (poweron_sts & BIT_6))
            aic_boot_reason = BOOT_REASON_WARMRST;
#ifndef WIN32
        if(poweron_sts & BIT_GLB_UPS_PWR_ON_FLAG_KEY)
            aic_boot_reason = BOOT_REASON_POWERKEY;
#endif
    }
}

#ifdef CONFIG_AIC_CTRL_D2D
static void aic_boot_sw_and_pwr_check(void)
{
    /* pwr sw check. 0, is no sw reason. */
    int val = aic_cmdline_get_value_int(cl_key_pwr_sw);

    if (!val) {
        /* pwr on check. 0, is no power on reason. */
        val = aic_cmdline_get_value_int(cl_key_pwr_on);
        s_reboot_hw = (val == 0);
    }
}
#endif

/**
 * aic_boot_mode_init - boot mode init.
 *
 * Parameter:
 *
 * Return:
 *
 */
void aic_boot_mode_init()
{
    char *cmd_str;

    /* ext image check. */
#ifdef CONFIG_FOTA_SUPPORT
    if (aic_boot_ext_img_check_fail()) {
        g_ext_check_fail = TRUE;
        aic_boot_enter_basic_mode();
        goto boot_mode_print;
    }
#endif

    /* gxem check. */
    cmd_str = aic_cmdline_get_value(cl_key_gxem);
    if (cmd_str) {
        aic_boot_enter_gxem_mode(cmd_str);
        goto boot_mode_print;
    }

    #ifdef CONFIG_AIC_CTRL_D2D
    aic_boot_sw_and_pwr_check();
    #endif

    /* boot mode check. */
    aic_boot_boot_mode_check();
    aic_boot_reason_check();

boot_mode_print:
    aicx_info(0x0d4c05, "xd", "aic_boot_mode_init mode=0x%x, reason=%d\n",
              aic_boot_mode, aic_boot_reason);
}

#ifdef CONFIG_AIC_CTRL_D2D
bool aic_boot_mode_is_no_sw_no_pwr(void)
{
    return s_reboot_hw;
}
#endif
