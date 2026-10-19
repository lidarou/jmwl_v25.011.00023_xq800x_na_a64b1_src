/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_CDC_ACM_H__
#define __USB_CDC_ACM_H__

#include <ux_api.h>

enum vcom_event{
    VCOM_DEACTIVE,
    VCOM_ACTIVE,
};

enum USB_VCOM {
    VCOM0,
    VCOM1,
    VCOM2,
    VCOM3,
    VCOM4,
    VCOM5,
    VCOM6,
    VCOM7,
};
typedef int (*CDC_ACM_CB)(uint32_t port, enum vcom_event ev);

/**
 * usb_cdc_acm_read - Read data from USB virtrul COM.
 *
 * Parameter:
 * @handle: USB virtrul COM operation handle.
 * @buf: Read buffer ptr
 * @length:read size
 * Return:
 *  return actually read size.
 */
size_t usb_cdc_acm_read(void *handle, void *buf, size_t length);

/**
 * usb_cdc_acm_write - Write data to USB virtrul COM.
 *
 * Parameter:
 * @handle: USB virtrul COM operation handle.
 * @buf: Write buffer ptr
 * @length:Write size
 * Return:
 *  return actually Write size.
 */
size_t usb_cdc_acm_write(void *handle, const void *buf, size_t length);

/**
 * usb_cdc_acm_open - Open USB virtrul COM.
 *
 * Parameter:
 * @port: USB virtrul COM index.
 * @func: Register USB virtrul COM callback function.
 * Return:
 *  return USB virtrul COM operation handle.
 */
void *usb_cdc_acm_open(int port, CDC_ACM_CB func);

/**
 * usb_cdc_acm_close - Close USB virtrul COM.
 *
 * Parameter:
 * @handle: USB virtrul COM operation handle.
 */
void usb_cdc_acm_close(void *handle);

/**
 * usb_cdc_acm_get_state - Get USB virtrul COM state.
 *
 * Parameter:
 * @handle: USB virtrul COM operation handle.
 * Return:
 *  return USB virtrul COM state.
 */
enum vcom_event usb_cdc_acm_get_state(void *handle);

#endif /* __USB_CDC_ACM_H__ */