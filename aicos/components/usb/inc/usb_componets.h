/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_COMPONETS_H__
#define __USB_COMPONETS_H__

#include <ux_api.h>

typedef int (*USB_CLASS_INIT)(void);
typedef int (*USB_CLASS_UNINIT)(void);
struct usb_class_instance {
    char *name;
    int status;
    USB_CLASS_INIT init;
    USB_CLASS_UNINIT uninit;
};

typedef enum USB_ENUM_COMBO
{
    LOG_AT_DIAG,
    RNDIS,
    ECM,
    MSC,
    UNKNOW_FUNC,
    COMBO_MAX = UNKNOW_FUNC,
} UEC;

void usb_service_network_status_report(u32 event);
void usb_service_register_instance(UEC class,
                       struct usb_class_instance instance);
UEC usbx_get_combo_default(void);
int usbx_uninit(void);
void usbx_descriptor_build(UEC combo);
int usbx_device_stack_init(void);
int usbx_init(void);
void usb_connect(void);
void usb_disconnect(void);
void usb_plugin(void);
void usb_plugout(void);
void usb_clk_enable();
void usb_clk_disable();
void usb_driver_get_mutex(void);
void usb_driver_put_mutex(void);
int usb_direct_enum(void);

#endif /* __USB_COMPONETS_H__ */
