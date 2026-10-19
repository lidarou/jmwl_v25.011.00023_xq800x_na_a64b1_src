/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e0904
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <stdint.h>
#include <ux_api.h>
#include <usb_service.h>

struct usb_app {
    aos_timer_ptr timer;
    TX_THREAD test_thread;
    TX_QUEUE    message;
    u32 func_bitmap;
    int index;
};
enum usb_app_msg {
    USB_ENUM,
};

char usb_app_stack[4096];
char usb_app_queue[64];


struct usb_app usb_app_test;

void usb_app_test_entry(void* data)
{
    ULONG   message =USB_ENUM;
    UINT  status;
    status =  tx_queue_send(&usb_app_test.message, &message, TX_NO_WAIT);
    AOS_ASSERT(!status);
}

void usb_app_thread_entry(ULONG data)
{
    ULONG   received_message;
    UINT  status;
    int ret;

    usb_app_test.index = LOG_AT_DIAG;
    usb_app_test.func_bitmap = usb_service_list_support_func();

    while(1) {
        status = tx_queue_receive(&usb_app_test.message, &received_message, TX_WAIT_FOREVER);
        AOS_ASSERT(!status);

        aicx_error(0x05129a, "x", "func_bitmap = 0x%x\n", usb_app_test.func_bitmap);
        switch (received_message) {
        case USB_ENUM:
            aicx_error(0x06726a, "d", "usb_app_test_entry start index = %d \n", usb_app_test.index);
            if (!usb_get_connect_status())
                break;
            if (!(usb_app_test.func_bitmap & (1 << usb_app_test.index)))
                break;
            ret = usb_service_enum_manage(usb_app_test.index);
            aicx_error(0x0e3c16, "dd", "usb_app_test_entry end index = %d, ret =%d\n", usb_app_test.index, ret);
            if (ret)
                while(1);
            break;
        default: break;
        }
        usb_app_test.index++;
        if (usb_app_test.index == COMBO_MAX) 
          usb_app_test.index = LOG_AT_DIAG;
    }
}

int usb_app_test_init(void)
{
    UINT  status;

    usb_app_test.timer = AOS_NULL;
    usb_app_test.index = 0;

    /* Create the message queue shared by threads 1 and 2.  */
    tx_queue_create(&usb_app_test.message, "usb_app queue", TX_1_ULONG,
                   usb_app_queue, 64);

    /* Create the usbx_cdc_acm_thread_entry thread. */
    status = tx_thread_create(&usb_app_test.test_thread,
                        "usb app test",
            usb_app_thread_entry, 0,
            usb_app_stack,
            4096, 2, 2, TX_NO_TIME_SLICE,
            TX_AUTO_START);
    if (status != UX_SUCCESS) {
        aicx_error(0x123a52, "x", "usb app test create thread fail ret =0x%x.\r\n", status);
        return AIC_EIO;
    }

    usb_app_test.timer = aos_create_timer("usb_test_timer", usb_app_test_entry,
                     NULL, 20000, AOS_AUTO_ACTIVATE, AOS_TRUE); //AOS_TRUE);
    AOS_ASSERT(usb_app_test.timer);

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_L2_INIT_EXPORT(usb_app_test_init);
