/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_CDC_CLASS_H__
#define __USB_CDC_CLASS_H__

#include "inttypes.h"

extern uint32_t usb_read(uint8_t *buf, uint32_t len, uint32_t timeout);
extern uint32_t usb_write(uint8_t *buf, uint32_t len, uint32_t timeout);

extern uint32_t usb_data_read(uint8_t *buffer, uint32_t length, uint32_t timeout);
extern uint32_t usb_data_write(uint8_t *buffer, uint32_t length, uint32_t timeout);
extern int usb_get_status(void);
extern int usb_start(uint32_t timeout);
#endif /* __USB_CDC_CLASS_H__ */