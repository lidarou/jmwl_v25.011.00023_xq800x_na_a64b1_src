/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

#ifndef __USB_GPIO_CFG_H__
#define __USB_GPIO_CFG_H__

#include <eipu.h>

#define  USB_EIUINT2_DETECT            2

struct usb_plug_dectet {
    u32 eipu_mode;
    u32 gpio_num;
    u32 eipu_num;
    u32 eipu_type;
};

struct usb_plug_dectet usb_hotplug = {
    .eipu_mode = 1,
    .eipu_num = USB_EIUINT2_DETECT,
    .eipu_type = ND_MODE,
};

#endif  /* __USB_GPIO_CFG_H__ */
