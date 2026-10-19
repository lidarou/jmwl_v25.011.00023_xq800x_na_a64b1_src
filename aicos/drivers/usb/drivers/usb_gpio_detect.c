/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0eb30d
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <gpio_inc.h>
#include <irq.h>
#include <usb_service.h>
#include <usb/usb_gpio_cfg.h>

extern void bcd_handler(void *param);
extern void usb_unplug_handler(void *param);

static void usb_hotplug_report_event(u32 pin_level)
{
    u32 uevent;

    if (pin_level) {
        aicx_info(0x0958b1, "", "usb plugin\n");
        bcd_handler(NULL);
        uevent = USB_PLUG_IN;
    } else {
        aicx_info(0x0ed0ae, "", "usb plugout\n");
        usb_unplug_handler(NULL);
        uevent = USB_PLUG_OUT;
    }
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}


static void usb_hotplug_gpio_detect_handler(void *data)
{
    u32 gpio_level;

    gpio_level = aic_gpio_level_get(usb_hotplug.gpio_num);
    usb_hotplug_report_event(gpio_level);
}

static void usb_hotplug_eipu_detect_handler(u32 id, u32 state, u32 eipu_pol)
{
    EIPU_SetPol(usb_hotplug.eipu_type, EIPU_DBNC, id, !eipu_pol);

    usb_hotplug_report_event(eipu_pol);
}

static int usb_hotplug_eipu_detect_init(void)
{
    EIPU_RegCallback(usb_hotplug.eipu_num, EIPU_DBNC, usb_hotplug_eipu_detect_handler, HIGH_LEVEL, 0, usb_hotplug.eipu_type);
    return AIC_SUCCESS;
}

static int usb_hotplug_gpio_detect_init(void)
{
    u32 gpio_level, uevent;
    int ret;

    if (usb_hotplug.eipu_mode)
        return usb_hotplug_eipu_detect_init();

    ret = aic_gpio_request(usb_hotplug.gpio_num);
    if (ret)
        return ret;
    aic_gpio_input(usb_hotplug.gpio_num, GPIO_TRIGGER_EDGE_ANY, usb_hotplug_gpio_detect_handler, NULL);
    gpio_level = aic_gpio_level_get(usb_hotplug.gpio_num);
    if (gpio_level) {
        bcd_handler(NULL);
        aicx_info(0x1a40e1, "", "usb default status: plugin\n");
        uevent = USB_PLUG_IN;
    } else {
        usb_unplug_handler(NULL);
        aicx_info(0x19f62c, "", "usb default status: plugout\n");
        uevent = USB_PLUG_DEFAULT_OUT;
    }
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
    return AIC_SUCCESS;
}

DIS_BASIC_DRV_L1_INIT_EXPORT(usb_hotplug_gpio_detect_init);
