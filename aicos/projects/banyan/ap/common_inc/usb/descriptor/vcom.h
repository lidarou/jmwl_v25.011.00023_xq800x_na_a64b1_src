/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __USB_DESC_VCOM_H__
#define __USB_DESC_VCOM_H__

/* Config VCOM number*/
#define VCOM_NUM        3

#define VCOM_CONFIG(n, f, s) { \
            .index = (n),   \
            .if_num = (f),  \
            .name = (s), \
        }

enum {
    COM0_IF = 2, /* 0, 1 interface for network*/
    COM1_IF,
    COM2_IF,
    COM3_IF,
    COM4_IF,
    COM5_IF,
};

#define COM_STRING       'G', 'e', 'n', 'e', 'r', 'i', 'c', ' ', 'S', 'e', 'r', 'i', 'a', 'l'
#define COM0_DESC(PAC_SIZE)    \
    0x09, 0x04, COM0_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03,   \
    0x07, 0x05, 0x83, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x03, 0x02, PAC_SIZE, 0x00

#define COM1_DESC(PAC_SIZE)    \
    0x09, 0x04, COM1_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x84, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x04, 0x02, PAC_SIZE, 0x00

#define COM2_DESC(PAC_SIZE)    \
    0x09, 0x04, COM2_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x85, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x05, 0x02, PAC_SIZE, 0x00

#define COM3_DESC(PAC_SIZE)    \
    0x09, 0x04, COM3_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03,   \
    0x07, 0x05, 0x86, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x06, 0x02, PAC_SIZE, 0x00

#define COM4_DESC(PAC_SIZE)    \
    0x09, 0x04, COM4_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x87, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x07, 0x02, PAC_SIZE, 0x00

#define COM5_DESC(PAC_SIZE)    \
    0x09, 0x04, COM5_IF, 0x00, 0x02, 0xff, 0x00, 0x00, 0x03, \
    0x07, 0x05, 0x88, 0x02, PAC_SIZE, 0x00,   \
    0x07, 0x05, 0x08, 0x02, PAC_SIZE, 0x00

#if (VCOM_NUM > 6)
#error VCOM_NUM must < = 6
#endif

#if VCOM_NUM == 1
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE)
#define USB_CDC_ACM_CONFIG      \
        VCOM_CONFIG(0, COM0_IF, "AIC LOG")

#elif VCOM_NUM == 2
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE),    \
    COM1_DESC(S##_MAXPACKETSIZE)
#define USB_CDC_ACM_CONFIG      \
            VCOM_CONFIG(0, COM0_IF, "AIC LOG"), \
            VCOM_CONFIG(1, COM1_IF, "AIC DIAG")
#elif VCOM_NUM == 3
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE),    \
    COM1_DESC(S##_MAXPACKETSIZE),    \
    COM2_DESC(S##_MAXPACKETSIZE)

#define USB_CDC_ACM_CONFIG      \
        VCOM_CONFIG(0, COM0_IF, "AIC LOG"), \
        VCOM_CONFIG(1, COM1_IF, "AIC DIAG"), \
        VCOM_CONFIG(2, COM2_IF, "AIC AT")

#elif VCOM_NUM == 4
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE),    \
    COM1_DESC(S##_MAXPACKETSIZE),    \
    COM2_DESC(S##_MAXPACKETSIZE),    \
    COM3_DESC(S##_MAXPACKETSIZE)

#define USB_CDC_ACM_CONFIG      \
            VCOM_CONFIG(0, COM0_IF, "AIC LOG"), \
            VCOM_CONFIG(1, COM1_IF, "AIC DIAG"), \
            VCOM_CONFIG(2, COM2_IF, "AIC AT"),   \
            VCOM_CONFIG(3, COM3_IF, "VCOM")

#elif VCOM_NUM == 5
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE),    \
    COM1_DESC(S##_MAXPACKETSIZE),    \
    COM2_DESC(S##_MAXPACKETSIZE),    \
    COM3_DESC(S##_MAXPACKETSIZE),    \
    COM4_DESC(S##_MAXPACKETSIZE)

#define USB_CDC_ACM_CONFIG      \
                VCOM_CONFIG(0, COM0_IF, "AIC LOG"), \
                VCOM_CONFIG(1, COM1_IF, "AIC DIAG"), \
                VCOM_CONFIG(2, COM2_IF, "AIC AT"),   \
                VCOM_CONFIG(3, COM3_IF, "VCOM"),    \
                VCOM_CONFIG(4, COM4_IF, "VCOM")
#elif VCOM_NUM == 6
#define COM_DESC(S) \
    COM0_DESC(S##_MAXPACKETSIZE),    \
    COM1_DESC(S##_MAXPACKETSIZE),    \
    COM2_DESC(S##_MAXPACKETSIZE),    \
    COM3_DESC(S##_MAXPACKETSIZE),    \
    COM4_DESC(S##_MAXPACKETSIZE),    \
    COM5_DESC(S##_MAXPACKETSIZE)

#define USB_CDC_ACM_CONFIG      \
                    VCOM_CONFIG(0, COM0_IF, "AIC LOG"), \
                    VCOM_CONFIG(1, COM1_IF, "AIC DIAG"), \
                    VCOM_CONFIG(2, COM2_IF, "AIC AT"),   \
                    VCOM_CONFIG(3, COM3_IF, "VCOM"),    \
                    VCOM_CONFIG(4, COM4_IF, "VCOM"),     \
                    VCOM_CONFIG(5, COM5_IF, "VCOM")
#endif

#define COM_DESC_SIZE (VCOM_NUM  * (9 + 7 + 7))

#endif  /* __USB_DESC_VCOM_H__ */
