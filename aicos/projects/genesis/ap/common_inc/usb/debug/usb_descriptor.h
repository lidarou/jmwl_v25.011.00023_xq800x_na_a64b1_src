/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __USB_DEBUG_DESCRIPTOR_H__
#define __USB_DEBUG_DESCRIPTOR_H__

#include <usb/descriptor/vcom.h>
#include <usb/descriptor/ecm.h>
#include <usb/descriptor/rndis.h>
#include <usb/descriptor/msc.h>

#define USB_VID    0x63, 0x35
#define USB_PID    0x00, 0x20
#define USB_BCD_DEVICE    0x01, 0x00

#define FS_MAXPACKETSIZE      0x40, 0x00
#define HS_MAXPACKETSIZE      0x00, 0x02

#define DEVICE_DESC    \
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,     \
    USB_VID, USB_PID, USB_BCD_DEVICE, 0x01, 0x02,   \
    0x00, 0x01

#define MSC_DEVICE_DESC    \
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40,     \
    USB_VID, USB_PID, USB_BCD_DEVICE, 0x01, 0x02,   \
    0x07, 0x01

__attribute__((aligned(32))) UCHAR hs_vcom_desc[] = {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, 0x4e, 0x0, 0x3, 0x1, 0x0, 0xE0, 0x32,
    COM0_DESC(HS_MAXPACKETSIZE),
    COM1_DESC(HS_MAXPACKETSIZE),
    COM2_DESC(HS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR fs_vcom_desc[]= {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, 0x4e, 0x0, 0x3, 0x1, 0x0, 0xE0, 0x32,
    COM0_DESC(FS_MAXPACKETSIZE),
    COM1_DESC(FS_MAXPACKETSIZE),
    COM2_DESC(FS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR hs_rndis_desc[] = {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, 0x90, 0x0, 0x5, 0x1, 0x0, 0xE0, 0x32,
    RNDIS_DESC(HS_MAXPACKETSIZE),
    COM0_DESC(HS_MAXPACKETSIZE),
    COM1_DESC(HS_MAXPACKETSIZE),
    COM2_DESC(HS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR fs_rndis_desc[]= {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, 0x90, 0x0, 0x5, 0x1, 0x0, 0xE0, 0x32,
    RNDIS_DESC(FS_MAXPACKETSIZE),
    COM0_DESC(FS_MAXPACKETSIZE),
    COM1_DESC(FS_MAXPACKETSIZE),
    COM2_DESC(FS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR hs_ecm_desc[] = {
     /* Device descriptor */
     DEVICE_DESC,

     0x9, 0x2, 0x9D, 0x0, 0x5, 0x1, 0x0, 0xE0, 0x32,
     ECM_DESC(HS_MAXPACKETSIZE),
     COM0_DESC(HS_MAXPACKETSIZE),
     COM1_DESC(HS_MAXPACKETSIZE),
     COM2_DESC(HS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR fs_ecm_desc[]= {
     /* Device descriptor */
     DEVICE_DESC,
     0x9, 0x2, 0x9D, 0x0, 0x5, 0x1, 0x0, 0xE0, 0x32,
     ECM_DESC(FS_MAXPACKETSIZE),
     COM0_DESC(FS_MAXPACKETSIZE),
     COM1_DESC(FS_MAXPACKETSIZE),
     COM2_DESC(FS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR hs_msc_desc[] = {
      /* Device descriptor */
      MSC_DEVICE_DESC,
      0x9, 0x2, 0x65, 0x0, 0x4, 0x1, 0x0, 0xC0, 0x32,
      MSC_DESC(HS_MAXPACKETSIZE),
      COM0_DESC(HS_MAXPACKETSIZE),
      COM1_DESC(HS_MAXPACKETSIZE),
      COM2_DESC(HS_MAXPACKETSIZE),
};

__attribute__((aligned(32))) UCHAR fs_msc_desc[]= {
      /* Device descriptor */
      MSC_DEVICE_DESC,
      0x9, 0x2, 0x65, 0x0, 0x4, 0x1, 0x0, 0xC0, 0x32,
      MSC_DESC(FS_MAXPACKETSIZE),
      COM0_DESC(FS_MAXPACKETSIZE),
      COM1_DESC(FS_MAXPACKETSIZE),
      COM2_DESC(FS_MAXPACKETSIZE),
};

#endif  /* __USB_DEBUG_DESCRIPTOR_H__ */
