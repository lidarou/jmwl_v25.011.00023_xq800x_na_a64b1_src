/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of lcdc.
 *   It includes all the lcdc CV cases. The lcdc CV case configurations
 *   will be configued by lcdc_case_config.h in project dir.
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12bcc2
#define LOG_MODULE    AMOD_GPAPH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include "lcdc/lcdc_reg.h"
#include "lcdc.h"
#include "cache.h"
#include "lcd_panel.h"
#include <display.h>
#include <display_cfg.h>
#include <pmu_user.h>
#include <irq.h>
#include <core_star.h>

static bool lcdc_eb = false;
aos_mutex_ptr aic_display_mutex = AOS_NULL;

static LIST_HEAD(bl_monitor_list);

#define __CACHE_LINE_SIZE             32U
struct pmu_user *p_panel_power = NULL;

#ifdef DEBUG
#define AIC_PRINTF_LCDC AIC_PRINTF
#else
#define AIC_PRINTF_LCDC(fmt, ...)
#endif

struct aic_lcdc_config lcdc_config = {
    .aic_wire_type = AIC_WIRE_TYPE,
    .aic_hw_te_trigger = AIC_HW_TE_TRIGGER, // 0'sw trigger 1'hw trigger
    .cmd_para_pack = AIC_CMD_PARA_PACK, // 0'disable para pack 1'enable para pack
    .color_format = AIC_COLOR_FORMAT,
};

/*4wire8bit 3wire9bit1lane 3wire9bit2lane*/
struct wire_type_string{
    int wire_type_id;
    char wire_type_name[20];
};

struct wire_type_string wire_type_group[] ={
    {AIC_LCDC_4WIRE_8BIT, "4wire8bit"},
    {AIC_LCDC_3WIRE_9BIT_1LANE, "3wire9bit1lane"},
    {AIC_LCDC_3WIRE_9BIT_2LANE, "3wire9bit2lane"}
};

/*4wire8bit 3wire9bit1lane 3wire9bit2lane*/
struct color_format_string{
    int color_format_id;
    char color_format_name[20];
};

struct color_format_string color_format_group[] ={
    {AIC_LCDC_COLOR_FORMAT_RGB444_UNPACK, "RGB444_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB444_COMPACT, "RGB444_COMPACT"},
    {AIC_LCDC_COLOR_FORMAT_RGB565, "RGB565"},
    {AIC_LCDC_COLOR_FORMAT_RGB666_UNPACK, "RGB666_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB888_UNPACK, "RGB888_UNPACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB444_PACK, "RGB444_PACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB666_PACK, "RGB666_PACK"},
    {AIC_LCDC_COLOR_FORMAT_RGB888_PACK, "RGB888_PACK"}
};

char color_format_error[] = "unsupport color format";

struct bl_monitor_dev {
    char *name;
    bl_switch_func monitor_cb;
    struct list_head link_list;
};

static char* str_dup(const char* str)
{
    int len = 0;
    const char* pstr = str;

    while (*pstr++ != '\0') {
        len++;
    }

    char* pdest = (char*)aos_malloc(len + 1);

    int word_num = len / sizeof(size_t);
    int byte_num = len % sizeof(size_t);

    const size_t* wsrc = (size_t*)str;
    size_t* wdest = (size_t*)pdest;

    while (word_num--) {
        *wdest++ = *wsrc++;
    }

    const char* bsrc = (char*)wsrc;
    char* bdest = (char*)wdest;

    while (byte_num--) {
        *bdest++ = *bsrc++;
    }

    pdest[len] = '\0';

    return pdest;
}

char *get_color_format_name(aic_lcdc_color_format color_format)
{
    int i;

    for (i = 0; i < sizeof(color_format_group)/sizeof(color_format_group[0]); i++) {
        if (color_format == color_format_group[i].color_format_id)
            return color_format_group[i].color_format_name;
    }

    return color_format_error;
}

char te_trigger[] = "te hw trigger";
char sw_trigger[] = "sw trigger";
char trigger_fail[] = "unsupport trigger mode";

void aic_display_wait_ms(u32 wait_ms)
{
    if (aos_is_user_thread_context())
        aos_sleep(wait_ms);
    else
        aic_mdelay(wait_ms);
}

char *get_trigger_mode_name(aic_lcdc_start_mode trigger_mode)
{
    if (trigger_mode == AIC_LCDC_SW_TRIGGER)
        return sw_trigger;
    else if (trigger_mode == AIC_LCDC_TE_TRIGGER)
        return te_trigger;
    else
        return trigger_fail;
}

char pack_mode[] = "pack_mode";
char single_mode[] = "sig_mode";

char *get_cmd_para_mode_name(bool is_cmd_para_pack)
{
    if (is_cmd_para_pack)
        return pack_mode;
    else
        return single_mode;
}

char wrie_type_error[] = "unsupport wire type";

char *get_wrie_type_name(aic_lcdc_wire_type wire_type)
{
    int i;

    for (i = 0; i < sizeof(wire_type_group)/sizeof(wire_type_group[0]); i++) {
        if (wire_type == wire_type_group[i].wire_type_id)
            return wire_type_group[i].wire_type_name;
    }

    return wrie_type_error;
}

void display_lock(void)
{
    int ret;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(aic_display_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }
}

void display_unlock(void)
{
    if (aos_is_user_thread_context())
        aos_put_mutex(aic_display_mutex);
}

int aic_lcdc_panel_init(void)
{
    if (lcdc_eb)
        return 0;

    lcdc_enable();
    lcd_panel_init();
    lcdc_eb = true;

    return 0;
}

static int aic_panel_poweron(bool enabled)
{
    int ret = 0;

#ifdef CONFIG_PANEL_POWER_PMIC
    if (p_panel_power) {
        if (enabled)
            ret = pmu_enable(p_panel_power);
        else
            ret = pmu_disable(p_panel_power);
        if (ret) {
            aicx_warn(0x0cb6d0, "s", "%s enable panel power fail!\n", __func__);
        }
    } else {
        ret = AIC_ENXIO;
    }
#endif
    return ret;
}

void display_reset()
{
#ifdef CONFIG_PANEL_POWER
    int ret = 0;

    ret = pmu_reset(p_panel_power);
    if (ret)
        return;
#endif

    lcd_panel_init();
}

void display_suspend(void)
{
    int ret;

    display_lock();

    if (!lcdc_eb) {
        aicx_warn(0x024096, "", "already suspend! unbalance operate, ignore!\n");
        goto out;
    }

    /*disable irq & clear pending, make sure after lcdc disable no more lcdc operate.*/
    NVIC_DisableIRQ(IRQ_LCDC);
    NVIC_ClearPendingIRQ(IRQ_LCDC);

    lcdc_disable();
    lcdc_eb = false;

    ret = aic_panel_poweron(false);
    if (ret) {
        aicx_error(0x1c9b19, "s", "%s panel power down fail!\n", __func__);
    }

out:
    display_unlock();
}

void display_resume(void)
{
    int ret;
    display_lock();

    if (lcdc_eb) {
        aicx_warn(0x18bbae, "", "already resume! unbalance operate, ignore!\n");
        goto out;
    }

    ret = aic_panel_poweron(true);
    if (ret) {
        aicx_error(0x045981, "s", "%s panel power on fail!\n", __func__);
        goto out;
    }

    lcdc_enable();
    NVIC_EnableIRQ(IRQ_LCDC);
    aic_lcdc_panel_init();

out:
    display_unlock();
}

void display_refresh(u32 fb_addr)
{
    display_lock();

    if (!lcdc_eb) {
       display_unlock();
       return;
    }

    dcache_clean_by_range((u32)fb_addr, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES);
    lcd_panel_framebuffer_init((u32)fb_addr, LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT);
    lcd_panel_refresh();

    display_unlock();
}

void display_refresh_sync(u32 fb_addr)
{
    display_lock();

    if (!lcdc_eb) {
       display_unlock();
       return;
    }

    dcache_clean_by_range((u32)fb_addr, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * AIC_PIXEL_BYTES);
    lcd_panel_framebuffer_init((u32)fb_addr, LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT);
    lcd_panel_refresh_sync();

    display_unlock();
}

void broadcast_backlight_switch(AIC_BL_SWITCH bl_status)
{
    struct bl_monitor_dev *dev_ops;

    list_for_each_entry(dev_ops, &bl_monitor_list,
                        link_list, struct bl_monitor_dev) {
        dev_ops->monitor_cb(bl_status);
    }
}

int register_backlight_switch_monitor(char *monitor_name, bl_switch_func callback)
{
    int ret = 0;
    struct bl_monitor_dev *new_monitor_dev;

    new_monitor_dev = aos_zalloc(sizeof(struct bl_monitor_dev));
    if (!new_monitor_dev) {
        ret = AIC_ENOSPC;
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        goto out;
    }

    list_head_init(&new_monitor_dev->link_list);
    new_monitor_dev->monitor_cb = callback;
    new_monitor_dev->name = str_dup(monitor_name);
    list_add(&new_monitor_dev->link_list, &bl_monitor_list);

out:
    return ret;
}

int unregister_backlight_switch_monitor(char *monitor_name)
{
    int ret = AIC_ENXIO;
    struct bl_monitor_dev *dev_ops, *temp_ops;

    list_for_each_entry_safe(dev_ops, temp_ops, &bl_monitor_list,
                            link_list, struct bl_monitor_dev) {
        if (!strcmp(dev_ops->name, monitor_name)) {
            list_del(&dev_ops->link_list);
            ret = aos_free(dev_ops->name);
            ret |= aos_free(dev_ops);
            if (ret)
                aicx_warn(0x0be34b, "s", "%s free node fail!\n", __func__);
            break;
        }
    }

    return ret;
}

int display_panel_init(void)
{
    int ret;

    ret = aic_panel_poweron(true);
    if (ret)
        aicx_warn(0x0a996d, "s", "%s display_panel_init fail!\n", __func__);

    return ret;
}

int display_init(void)
{
    int ret;

    aic_display_mutex = aos_create_mutex("aic_display_mutex");
    AIC_ASSERT(aic_display_mutex);

#ifdef CONFIG_PANEL_POWER_PMIC
    if (NULL == p_panel_power) {
        p_panel_power = pmu_get("display_panel", CONFIG_PANEL_POWER);
        if (!p_panel_power) {
            aicx_warn(0x005ac6, "s", "%s get panel pmu operate handler fail!\n", __func__);
            return AIC_ENXIO;
        }
    }

    ret = aic_panel_poweron(true);
    if (ret)
        return ret;

#endif

    lcdc_init();
    aic_lcdc_panel_init();

    aicx_info(0x10fe0c, "", "display_init\n");

    return 0;
}
AIC_DRV_P1_INIT_EXPORT(display_init);

int display_late_init(void)
{
    int ret = 0;

    if (is_panel_init_success())
        return ret;

    /* 1. turn off backlight */
    ret = aic_bl_brightness_set(0);
    if (ret) {
        aicx_warn(0x121963, "sd", "%s turn off backlight fail! ret = %d \n", __func__,ret);
        return ret;
    }

    /* 2. suspend lcdc panel */
    display_suspend();

    return ret;
}
AIC_APP_L2_INIT_EXPORT(display_late_init);
