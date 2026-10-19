/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_CDC_ECM_H__
#define __USB_CDC_ECM_H__

#include <ux_api.h>
#ifdef CONFIG_CDC_ECM_INSTANCE
void usb_cdc_ecm_single_connect(void);
void usb_cdc_ecm_single_disconnect(void);
u32 usb_cdc_ecm_single_get_state(void);
int usb_cdc_ecm_update_xmit_queue_room_size(u32 size);
#else
static inline void usb_cdc_ecm_single_connect(void) {}
static inline void usb_cdc_ecm_single_disconnect(void){}
static inline u32 usb_cdc_ecm_single_get_state(void) { return 0; }
static inline int usb_cdc_ecm_update_xmit_queue_room_size(u32 size)  { return 0; }
#endif /* CONFIG_CDC_ECM_INSTANCE */

#endif /* __USB_CDC_ECM_H__ */
