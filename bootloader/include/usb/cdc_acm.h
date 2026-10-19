/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_CDC_ACM_H__
#define __USB_CDC_ACM_H__

#include "inttypes.h"

extern uint32_t usb_data_read(uint8_t *buffer, uint32_t length);
extern uint32_t usb_data_write(uint8_t *buffer, uint32_t length);
#endif /* __USB_CDC_ACM_H__ */