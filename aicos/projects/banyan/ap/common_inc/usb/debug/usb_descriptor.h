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

#define DESC_BASE_SIZE      9

#define VCOM_DESC_TOTAL_SIZE  (DESC_BASE_SIZE + COM_DESC_SIZE)
__attribute__((aligned(32))) UCHAR hs_vcom_desc[] = {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, (DESC_BASE_SIZE + COM_DESC_SIZE), 0x0, VCOM_NUM, 0x1, 0x0, 0xE0, 0x32,
    COM_DESC(HS),
};

__attribute__((aligned(32))) UCHAR fs_vcom_desc[]= {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, (DESC_BASE_SIZE + COM_DESC_SIZE), 0x0, VCOM_NUM, 0x1, 0x0, 0xE0, 0x32,
    COM_DESC(FS),
};

#ifdef CONFIG_RNDIS_INSTANCE
#define RNDIS_DESC_TOTAL_SIZE  (VCOM_DESC_TOTAL_SIZE + RNDIS_DESC_LEN)
#define RNDIS_INF_SUM   (RNDIS_INF_NUM + VCOM_NUM)
__attribute__((aligned(32))) UCHAR hs_rndis_desc[] = {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, RNDIS_DESC_TOTAL_SIZE, 0x0, RNDIS_INF_SUM, 0x1, 0x0, 0xE0, 0x32,
    RNDIS_DESC(HS_MAXPACKETSIZE),
    COM_DESC(HS),
};

__attribute__((aligned(32))) UCHAR fs_rndis_desc[]= {
    /* Device descriptor */
    DEVICE_DESC,
    0x9, 0x2, RNDIS_DESC_TOTAL_SIZE, 0x0, RNDIS_INF_SUM, 0x1, 0x0, 0xE0, 0x32,
    RNDIS_DESC(FS_MAXPACKETSIZE),
    COM_DESC(FS),
};
#else
__attribute__((aligned(32))) UCHAR hs_rndis_desc[] = {
    '\0'
};

__attribute__((aligned(32))) UCHAR fs_rndis_desc[]= {
    '\0'
};
#endif
#ifdef CONFIG_CDC_ECM_INSTANCE

#define ECM_DESC_TOTAL_SIZE  (VCOM_DESC_TOTAL_SIZE + ECM_DESC_LEN)
#define ECM_INF_SUM   (ECM_INF_NUM + VCOM_NUM)

__attribute__((aligned(32))) UCHAR hs_ecm_desc[] = {
     /* Device descriptor */
     DEVICE_DESC,
     0x9, 0x2, ECM_DESC_TOTAL_SIZE, 0x0, ECM_INF_SUM, 0x1, 0x0, 0xE0, 0x32,
     ECM_DESC(HS_MAXPACKETSIZE),
     COM_DESC(HS),
};

__attribute__((aligned(32))) UCHAR fs_ecm_desc[]= {
     /* Device descriptor */
     DEVICE_DESC,
     0x9, 0x2, ECM_DESC_TOTAL_SIZE, 0x0, ECM_INF_SUM, 0x1, 0x0, 0xE0, 0x32,
     ECM_DESC(FS_MAXPACKETSIZE),
     COM_DESC(FS),
};
#else
__attribute__((aligned(32))) UCHAR hs_ecm_desc[] = {
     '\0'
};

__attribute__((aligned(32))) UCHAR fs_ecm_desc[]= {
     '\0'
};
#endif
#ifdef CONFIG_USB_STORAGE_INSTANCE

#define MSC_DESC_TOTAL_SIZE  (VCOM_DESC_TOTAL_SIZE + MSC_DESC_LEN)
#define MSC_INF_SUM   (MSC_INF_NUM + VCOM_NUM)

__attribute__((aligned(32))) UCHAR hs_msc_desc[] = {
      /* Device descriptor */
      MSC_DEVICE_DESC,
      0x9, 0x2, MSC_DESC_TOTAL_SIZE, 0x0, MSC_INF_SUM, 0x1, 0x0, 0xC0, 0x32,
      MSC_DESC(HS_MAXPACKETSIZE),
      COM_DESC(HS),
};

__attribute__((aligned(32))) UCHAR fs_msc_desc[]= {
      /* Device descriptor */
      MSC_DEVICE_DESC,
      0x9, 0x2, MSC_DESC_TOTAL_SIZE, 0x0, MSC_INF_SUM, 0x1, 0x0, 0xC0, 0x32,
      MSC_DESC(FS_MAXPACKETSIZE),
      COM_DESC(FS),
};
#else
__attribute__((aligned(32))) UCHAR hs_msc_desc[] = {
      '\0'
};

__attribute__((aligned(32))) UCHAR fs_msc_desc[]= {
      '\0'
};
#endif
#endif  /* __USB_DEBUG_DESCRIPTOR_H__ */
