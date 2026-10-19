/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __USB_CDC_ACM_CFG_H__
#define __USB_CDC_ACM_CFG_H__

#include <stdint.h>
#include <ux_api.h>
#include <ux_device_class_cdc_acm.h>
#include <cdc_acm.h>

#define USB_CDC_ACM_INSTANCE_MAX    3
#define APP_RX_DATA_SIZE            1048
#define APP_TX_DATA_SIZE            1048

struct usb_cdc_acm {
    uint8_t *name;
    uint32_t index;
    uint32_t if_num;
    UX_SLAVE_CLASS_CDC_ACM *cdc_acm;
    CDC_ACM_CB cb;
    enum vcom_event status;
#ifdef CDC_ACM_SELFTEST
    TX_THREAD self_test;
#endif
};

#ifdef CDC_ACM_SELFTEST
__attribute__((aligned(32))) uint8_t UserRxBuffer[APP_RX_DATA_SIZE];
__attribute__((aligned(32))) uint8_t UserTxBuffer[APP_TX_DATA_SIZE];

char usb_cdc_acm_selftest_stack[USB_CDC_ACM_INSTANCE_MAX][2048];
#endif

struct usb_cdc_acm usb_cdc_acm[USB_CDC_ACM_INSTANCE_MAX] = {
    {
        .index = 0,
        .if_num = 2,
        .name = "AIC LOG",
    } , {
        .index = 1,
        .if_num = 3,
        .name = "AIC AT",
    } , {
        .index = 2,
        .if_num = 4,
        .name = "AIC DIAG",
    } ,
};

#endif  /* __USB_CDC_ACM_CFG_H__ */
