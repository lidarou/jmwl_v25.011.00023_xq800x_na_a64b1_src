/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "initcall.h"
#include "platform/platform.h"
#include "usb/cdc_class.h"
#include "download/download.h"

#ifdef USB_CDC_ACM_SELF_TEST
__attribute__((aligned(32))) uint8_t self_test_buffer[2048];
#endif

uint32_t usb_read(uint8_t *buf, uint32_t len, uint32_t timeout)
{
    int ret;

    if (!usb_get_status()) {
        ret = usb_start(timeout);
        if (ret)
            return 0;
    }

    return usb_data_read(buf, len, timeout);
}

uint32_t usb_write(uint8_t *buf, uint32_t len, uint32_t timeout)
{
    int ret;

    if (!usb_get_status()) {
        ret = usb_start(timeout);
        log_print("usb_write ret = %d\n", ret);
        if (ret)
            return 0;
    }

    return usb_data_write(buf, len, timeout);
}

#ifdef USB_CDC_ACM_SELF_TEST
void usb_class_cdc_acm_self_test(void)
{
    uint32_t len;
    int i;

    while (1) {
        len = usb_read(self_test_buffer, 1024, -1);
        for (i = 0; i < 10; i++)
            log_print("vcom self_test_buffer[%d]: 0x%x\n", i, self_test_buffer[i]);
        usb_write(self_test_buffer, len, -1);
        log_print("vcom len = %d\n", len);
    }
}
#endif

static int usb_class_cdc_acm_init(void)
{
#ifdef USB_CDC_ACM_SELF_TEST
    usb_class_cdc_acm_self_test();
#endif
    return 0;
}

early_init(usb_class_cdc_acm_init);
