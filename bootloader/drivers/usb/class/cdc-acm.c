/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "initcall.h"
#include "platform/platform.h"
#include "usb/cdc_acm.h"
#include "download/download.h"

static uint32_t data_len = 0;
static uint32_t data_index = 0;
__attribute__((aligned(32))) uint8_t usb_buffer[32];
__attribute__((aligned(32))) uint8_t self_test_buffer[2048];

uint32_t usb_class_cdc_acm_read(void *data, uint8_t *buf, uint32_t len)
{
    return usb_data_read(buf, len);
}

uint32_t usb_class_cdc_acm_write(void *data, uint8_t *buf, uint32_t len)
{
    return usb_data_write(buf, len);
}

void usb_class_cdc_acm_self_test(void *data)
{
    uint32_t len;
    int i;

    while (1) {
        len = usb_class_cdc_acm_read(data, self_test_buffer, 1024);
        for (i = 0; i < 10; i++)
            log_print("vcom self_test_buffer[%d]: 0x%x\n", i, self_test_buffer[i]);
        usb_class_cdc_acm_write(data, self_test_buffer, len);
        log_print("vcom len = %d", len);
    }
}

int usb_class_cdc_acm_get_char_blocking(void *data)
{
    char ch;

    if (!data_index)
        data_len = usb_data_read(usb_buffer, 32);
    ch = usb_buffer[data_index];
    data_index++;
    if (data_index == data_len) {
       data_len = 0;
       data_index = 0;
    }
    return ch;

}

static struct download usb_download = {
    .loop_self = usb_class_cdc_acm_self_test,
    .write = usb_class_cdc_acm_write,
    .read = usb_class_cdc_acm_read,
    .get_char_blocking = usb_class_cdc_acm_get_char_blocking,
};

static int usb_class_cdc_acm_init(void)
{
    download_driver_register(DL_USB, &usb_download);

    return 0;
}

driver_init(usb_class_cdc_acm_init);
