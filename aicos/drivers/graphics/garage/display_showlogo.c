/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is display interface & data path demo.
 *
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x149933
#define LOG_MODULE    AMOD_GPAPH

#include <aic_init.h>
#include <aic_assert.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <irq_chip.h>
#include <aic_osa.h>
#include <cache.h>
#include <display.h>
#include <aic_errno.h>
#include <fmd.h>
#include <timer_inc.h>
#include <hlzma.h>
#ifdef CONFIG_AIC_SYSOFF_CHARGE
#include <bcd.h>
#include <sysoff.h>
#endif
#include "display_cfg.h"


#define __CACHE_LINE_SIZE             32U
extern u32 csi_rgb565_dst[];

char logo_enc[] __attribute__((aligned(__CACHE_LINE_SIZE)))= {
#if (LCD_PANEL_WIDTH == 128)
    #include "./img_128_128/logo.h"
#else
    #include "./test_img_240_320/logo.h"
#endif
};

#ifdef CONFIG_AIC_SYSOFF_CHARGE
char logo_charge_enc[] __attribute__((aligned(__CACHE_LINE_SIZE)))= {
#if (LCD_PANEL_WIDTH == 128)
    #include "./img_128_128/charging_logo.h"
#else
    #include "./test_img_240_320/charging_logo.h"
#endif
};
#endif

int dec_logo(char *logo_buf, char *addr)
{
    int ret = AIC_SUCCESS;

    if (!logo_buf)
        return AIC_EPERM;

    /*make data clean*/
    dcache_invcc_by_range((u32)logo_buf, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);

    ret = hlzma_dec_polling((u32)addr, (u32)logo_buf);
    if (ret)
        aicx_warn(0x057d2d, "s", "decompress logo failed, 0x%s\n", ret);
    return ret;
}
int show_logo(void)
{
    int ret;
    char *logo_buf = NULL;

#ifdef CONFIG_CAMERA_SUPPORT
    /* use camera buffer */
    logo_buf = (char *)csi_rgb565_dst;
#else
    logo_buf = aos_dma_malloc(153600);//240*320*2 RGB565
#endif
    if (!logo_buf) {
        aicx_warn(0x0382d0, "", "allocate logo buf failed\n");
        return AIC_ENOMEM;
    }
    ret = dec_logo(logo_buf,logo_enc);
    if (ret)
        goto err_ret;
    display_refresh_sync((u32)logo_buf);

err_ret:
#ifndef CONFIG_CAMERA_SUPPORT
    aos_free(logo_buf);
#endif

    ret = aic_bl_brightness_set(80);

    return ret;
}
AIC_DRV_P2_INIT_EXPORT(show_logo);

#ifdef CONFIG_AIC_SYSOFF_CHARGE
void charge_mode_unplug_dev_tsk(void *param)
{
    if (!aic_boot_mode_is_gxem())
        sysoff_shutdown();
}

int charge_mode_plugout_init(void)
{
    struct aic_bc_device *unplug_dev;
    int ret;

    unplug_dev = aos_zalloc(sizeof(struct aic_bc_device));
    unplug_dev->bc_type = AIC_USB_UNPLUG;
    strcpy(unplug_dev->bcd_name, "unplug_dev");
    ret = aic_bcd_register(unplug_dev, charge_mode_unplug_dev_tsk, NULL);
    if(ret)
        aicx_warn(0x06cd0f, "s", "%s fail!\n",  __func__);

    if (!aic_is_usb_plugin())
        sysoff_shutdown();

    return ret;
}
AIC_CHARGE_APP_L1_INIT_EXPORT(charge_mode_plugout_init);

int show_logo_charge(void) /* TODO: delete this API and resource of charge */
{
    int ret;
    char *logo_buf = NULL;

    charge_mode_plugout_init();

#ifdef CONFIG_CAMERA_SUPPORT
        /* use camera buffer */
        logo_buf = (char *)csi_rgb565_dst;
#else
        logo_buf = aos_dma_malloc(153600);//240*320*2 RGB565
#endif

    if (!logo_buf) {
        aicx_warn(0x01845c, "", "allocate charge logo buf failed\n");
        return AIC_ENOMEM;
    }
    ret = dec_logo(logo_buf,logo_charge_enc);
    if (ret)
        goto err_ret;
    display_refresh_sync((u32)logo_buf);

err_ret:
#ifndef CONFIG_CAMERA_SUPPORT
        aos_free(logo_buf);
#endif
    return ret;
}
#endif

int backlight_init(void)
{
    int ret;

    ret = aic_bl_brightness_set(100);

    return ret;
}
AIC_APP_L1_INIT_EXPORT(backlight_init);
