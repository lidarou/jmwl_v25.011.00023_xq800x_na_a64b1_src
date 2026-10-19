/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __USB_DESC_VCOM_H__
#define __USB_DESC_VCOM_H__

#define COM_STRING       'G', 'e', 'n', 'e', 'r', 'i', 'c', ' ', 'S', 'e', 'r', 'i', 'a', 'l'
#define COM0_DESC(PAC_SIZE)    \
    0x09, 0x04, 0x02, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03,   \
    0x07, 0x05, 0x83, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x03, 0x02, PAC_SIZE, 0x00

#define COM1_DESC(PAC_SIZE)    \
    0x09, 0x04, 0x03, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x84, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x04, 0x02, PAC_SIZE, 0x00

#define COM2_DESC(PAC_SIZE)    \
    0x09, 0x04, 0x04, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x85, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x05, 0x02, PAC_SIZE, 0x00

#endif  /* __USB_DESC_VCOM_H__ */
