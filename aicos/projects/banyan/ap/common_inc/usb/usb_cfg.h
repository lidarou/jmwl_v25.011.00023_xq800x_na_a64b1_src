/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __USB_CFG_H__
#define __USB_CFG_H__

#include <usb/debug/usb_descriptor.h>
#include <usb_componets.h>

#define COM_STRING       'G', 'e', 'n', 'e', 'r', 'i', 'c', ' ', 'S', 'e', 'r', 'i', 'a', 'l'

__attribute__((aligned(32))) UCHAR string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 0x15,
        'A', 'I', 'C', 'X', 'T', 'E', 'K', ' ', 'T', 'E', 'C', 'H', ' ', 'C', 'o', '.', ',', 'L', 't', 'd', '.',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 0x10,
        'C', 'o', 'm', 'p', 'o', 's', 'i', 't', 'e', ' ', 'D', 'e', 'v', 'i', 'c', 'e',

    /* Product string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x0E,
    COM_STRING,

    /* Product string descriptor : Index 4 */
    0x09, 0x04, 0x04, 0x0D,
    RNDIS_STRING,

    /* Product string descriptor : Index 5 */
    0x09, 0x04, 0x05, 0x0B,
    ECM_STRING,

    /* Product string descriptor : Index 6 */
    0x09, 0x04, 0x06, 0x0c,
    ECM_REMOTE_NODE_ID,

    /* Product string descriptor : Index 7 */
    0x09, 0x04, 0x07, 0x08,
    '2', '0','2', '3','0', '7', '0', '6',
};

__attribute__((aligned(32))) UCHAR language_id_framework[] = {
    /* English. */
    0x09, 0x04
};

#define UX_DEFAULT_ENUM     LOG_AT_DIAG

#if VCOM_NUM > 3
#define UX_MEMORY_SIZE     ((6 * 1024) + UX_CDC_ECM_MEM_SIZE + UX_RNDIS_MEM_SIZE + UX_MSC_MEM_SIZE)
#else
#define UX_MEMORY_SIZE     ((5 * 1024) + UX_CDC_ECM_MEM_SIZE + UX_RNDIS_MEM_SIZE + UX_MSC_MEM_SIZE)
#endif

#endif  /* __USB_CFG_H__ */
