/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __USB_DESC_MSC_H__
#define __USB_DESC_MSC_H__

#define MSC_DESC(PAC_SIZE)      \
    0x09, 0x04, 0x00, 0x00, 0x02, 0x08, 0x06, 0x50, 0x00, \
    0x07, 0x05, 0x86, 0x02, PAC_SIZE, 0x00, \
    0x07, 0x05, 0x06, 0x02, PAC_SIZE, 0x00
#define MSC_DESC_LEN        (0x9 + 0x7 +0x7)

#ifdef CONFIG_USBX_DEVICE_CLASS_MSC
#define UX_MSC_MEM_SIZE    (7 * 1024)
#else
#define UX_MSC_MEM_SIZE    0
#endif

#endif  /* __USB_DESC_MSC_H__ */
