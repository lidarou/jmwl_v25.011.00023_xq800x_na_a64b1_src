/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_SERVICE_H__
#define __USB_SERVICE_H__

#include <aic_osa.h>
#include <usb_componets.h>
#include <ux_api.h>

#define LISTENER_MAX_NUM       5
typedef void (* USB_LISTEN_HANDLER)(u32 event);

enum usb_network_type {
    USB_RNDIS,
    USB_ECM,
};

enum usb_detect_event {
    USB_PLUG_DEFAULT_IN,
    USB_PLUG_DEFAULT_OUT,
    USB_PLUG_IN,
    USB_PLUG_OUT,
};

struct usb_service {
    aos_thread_id id;
    aos_queue_ptr queue;
    BOOL connect;
    UEC usb_combo;
    struct usb_class_instance instance[COMBO_MAX];
    USB_LISTEN_HANDLER listener[LISTENER_MAX_NUM];
    USB_LISTEN_HANDLER network_listener;
    int listener_index;
    aos_mutex_ptr mutex;
    aos_timer_ptr enum_timer;
    int timeout_record;
};

extern struct usb_service uservice;

void usb_service_enum_timer_active(void);
void usb_service_enum_timer_deactive(void);
int usb_service_plug_event_listen_register(USB_LISTEN_HANDLER handler);
int usb_service_network_status_listen_register(USB_LISTEN_HANDLER handler);
BOOL usb_get_connect_status(void);
void usb_service_hotplug_event(u32 event);

/**
 * usb_service_enum_manage - USB enum according to combo parameter.
 *
 * Parameter:
 * @combo: USB enum mode.
 *
 * Return:
 *  return USB enum result.
 */
int usb_service_enum_manage(UEC combo);

/**
 * usb_service_get_enum_mode - Get current enum mode.
 *
 * Return:
 *  return current enum mode.
 *  UNKNOW_FUNC,
 *  LOG_AT_DIAG,
 *  RNDIS,
 *  ECM,
 *  MSC,
 */
enum USB_ENUM_COMBO usb_service_get_enum_mode(void);

/**
 * usb_service_list_support_func - List usb support function.
 *
 * Return:
 *  return usb support function bitmap.
 *  BIT1:LOG_AT_DIAG, BIT2:RNDIS, BIT3:ECM, BIT4:MSC,
 */
u32 usb_service_list_support_func(void);

/**
 * usb_service_virtual_plugin - Software virtual plugin event.
 *
 */
void usb_service_virtual_plugin(void);

/**
 * usb_service_virtual_plugout - Software virtual plugout event.
 *
 */
void usb_service_virtual_plugout(void);
#endif /* __USB_SERVICE_H__ */