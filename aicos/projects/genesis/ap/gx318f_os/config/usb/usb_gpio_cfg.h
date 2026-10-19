/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

#ifndef __USB_GPIO_CFG_H__
#define __USB_GPIO_CFG_H__

#include <eipu.h>

#define  USB_GPIO_DETECT            GPIO_AON_13

struct usb_plug_dectet {
    u32 eipu_mode;
    u32 gpio_num;
    u32 eipu_num;
    u32 eipu_type;
};

struct usb_plug_dectet usb_hotplug = {
    .gpio_num = USB_GPIO_DETECT,
};

#endif  /* __USB_GPIO_CFG_H__ */
