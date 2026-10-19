/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14aed1
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_type.h>
#include <bcd.h>
#include <irq.h>
#include <usb_service.h>

#define  USB_BC_STATUS_MAX            3

struct usb_bc_dev {
    struct aic_bc_device bc;
    PMIC_BCD_FUNC func;
};

void usb_plugin_handler(void *param)
{
    u32 uevent = USB_PLUG_IN;

    aicx_info(0x0958b1, "", "usb plugin\n");
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}

void usb_plugout_handler(void *param)
{
    u32 uevent = USB_PLUG_OUT;;

    aicx_info(0x0ed0ae, "", "usb plugout\n");
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}

struct usb_bc_dev usb_bc[USB_BC_STATUS_MAX] = {
    {
       .bc = {
            .bc_type = AIC_BC_TYPE_SDP,
            .bcd_name = "usb_sdp_plugin",
        },
       .func = usb_plugin_handler,
    }, {
        .bc = {
            .bc_type = AIC_BC_TYPE_CDP,
            .bcd_name = "usb_cdp_plugin",
        },
        .func = usb_plugin_handler,
    },{
    .bc = {
        .bc_type = AIC_USB_UNPLUG,
        .bcd_name = "usb_plugout",
       },
     .func = usb_plugout_handler,
    },
};

static int usb_bcd_init(void)
{
    int i, ret;

    for (i = 0; i < USB_BC_STATUS_MAX; i++) {
        ret = aic_bcd_register(&usb_bc[i].bc, usb_bc[i].func, NULL);
        if (ret) {
            aicx_error(0x14ac1e, "sd", "%s register fail, ret = %d\n", usb_bc[i].bc.bcd_name, ret);
            while(i) {
                i--;
                aic_bcd_unregister(&usb_bc[i].bc);
            }
            return ret;
        }
    }

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(usb_bcd_init);
