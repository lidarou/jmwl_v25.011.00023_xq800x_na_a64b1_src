/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file entry demo apps without GUI for aicos
 */

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>
#include "app_msg.h"
#include "aic_gpio_pin.h"

#define APP_CFG_TASK_STK_SIZE       (4096u)
#define APP_CFG_TASK_COM_PRIO       (35u)
typedef int32_t (*srv_func_callback)(void *p_param, uint32_t size); // TODO: temp and common define this

/* Entry point for application, custom this function to meet your requirement */
static void AppEntryNoGUI(void *thread_input)
{
    /* Initial API of APP, you can add your code here. */
    //app_demo_init();
    uint32_t ret = 0;
    app_callback_msg_t recv_msg = {0};

    while (1) {
        app_logi("Application entry point!\n");

        /* 15 seconds */
        ret = app_msg_recv(&recv_msg, 15000);
        if( AOS_QUEUE_EMPTY == ret ) {
            continue;
        }
        else {
            switch (recv_msg.msg_id) {
                case 0xAAAAAAAA:
                    {
                        srv_func_callback p_cb = (srv_func_callback)recv_msg.p_func_callback;
                        if(NULL != p_cb) {
                            p_cb(recv_msg.p_param, recv_msg.size);
                        }

                        /* free recieved msg */
                        if(NULL != recv_msg.p_param) {
                            aic_free(recv_msg.p_param);
                        }
                    }
                    break;
                default:
                    app_logi("Recive a APP msg which is NOT known. \n");
                    break;
            }
        }
    }
}

int app_init(void)
{
    // A64A2 PCB stable BLE RESET Felix. 2026-10-10
    aic_gpio_output(GPIO_GPO_1, 0);

#if 0
    /* Initial app msg queue for extern event handle and dispatch. */
    app_msg_init();

    aos_create_thread(
        "app_no_gui_task",
        AppEntryNoGUI,
        NULL,
        APP_CFG_TASK_STK_SIZE,
        0,
        NULL,
        APP_CFG_TASK_COM_PRIO,
        AOS_AUTO_START);
#else
    extern void my_gps_main(void);
    my_gps_main();
#endif
    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(app_init);

