/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_ECM_CFG_H__
#define __USB_ECM_CFG_H__

#include <ux_api.h>
#include <ux_device_class_cdc_ecm.h>

#define VENDOR_ID   0x3563
#define PRODUCT_ID   0x2000

#define ECM_VENDOR_DESC       "AICXTEK ECM"

/* This MAC address only for demo */
uint8_t ecm_local_node_id[UX_DEVICE_CLASS_CDC_ECM_NODE_ID_LENGTH] = {
    0xB0, 0x60, 0x88, 0xd7, 0x7E, 0xb8,
};

uint8_t ecm_remote_node_id[UX_DEVICE_CLASS_CDC_ECM_NODE_ID_LENGTH] = {
    0xB0, 0x60, 0x88, 0xd7, 0x7E, 0xb9,
};


#endif  /* __USB_ECM_CFG_H__ */
