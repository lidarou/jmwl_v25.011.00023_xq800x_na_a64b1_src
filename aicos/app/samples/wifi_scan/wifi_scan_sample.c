/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file wifi_scan_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>

#include "aic_wifi_scan.h"
#ifdef CONFIG_APP
#include "app_msg.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define WIFI_SCAN_SAMPLE_TASK_STK_SIZE       (4096u)
#define WIFI_SCAN_SAMPLE_TASK_COM_PRIO       (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

#ifdef CONFIG_APP
void start_wifi_scan_in_wifi_callback(void);
#endif
void start_wifi_scan_in_other_task(void);
void stop_wifi_scan(void);


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static int32_t  wfs_scan_result_handler(void *p_param, uint32_t size)
{
    u32 i = 0;
    wfs_scan_result_t *p_wfs_result = NULL;
    uint32_t total_ap_num = 0;

    p_wfs_result = (wfs_scan_result_t *)p_param;
    total_ap_num = p_wfs_result->total_ap_num;

    app_logi("[wifi_scan sample]wfs_scan_result_handler \r\n");

    for (i=0; i<total_ap_num; i++ ) {
        /* package scan result at string */
        /* urc eg. +WIFISCAN:1,1,1A:2B:3C:4D:5E:6F,-30,-,-*/
        app_logi("[wifi_scan sample]1,%d,%s,%d,-,-  \r\n",p_wfs_result->p_ap_array[i].channel,p_wfs_result->p_ap_array[i].mac,p_wfs_result->p_ap_array[i].rssi);
    }

    /* need start next wifi scan in this callback*/
    //start_wifi_scan_in_wifi_callback();
    return 0;
}

#ifdef CONFIG_APP

/* NOTE:this sample need open CONFIG_APP */

__APP_CALLBACK_FUNCTION__(wfs_scan_result_handler);

void start_wifi_scan_in_wifi_callback(void)
{
    int32_t ret = 0;
    wfs_scan_config_t scan_cfg = {0};

    app_logi("[wifi_scan sample]wifi_scan \r\n");

    scan_cfg.max_ap_num = 10;
    scan_cfg.time_out = 5000;
    scan_cfg.scan_channel[0] = 0;/* scan all chennel */
    scan_cfg.ssid_enable = true;/* parse ssid enable */

    ret = aic_srv_wifi_scan_start(&scan_cfg, __APP_CALLBACK_NAME__(wfs_scan_result_handler));
    if (ret != 0) {
         app_logi("[wifi_scan sample]: wifi scan start fail ret(%d).r\n",ret);
    }
    return;
}

void stop_wifi_scan_in_wifi_callback(void)
{
    aic_srv_wifi_scan_stop(__APP_CALLBACK_NAME__(wfs_scan_result_handler));
}
#endif

void start_wifi_scan_in_other_task(void)
{
    int32_t ret = 0;
    wfs_scan_config_t scan_cfg = {0};

    app_logi("[wifi_scan sample]wifi_scan \r\n");

    scan_cfg.max_ap_num = 10;
    scan_cfg.time_out = 5000;
    scan_cfg.scan_channel[0] = 0;/* scan all chennel */
    scan_cfg.ssid_enable = true;/* parse ssid enable */

    ret = aic_srv_wifi_scan_start(&scan_cfg, (srv_func_callback)wfs_scan_result_handler);
    if (ret != 0) {
         app_logi("[wifi_scan sample]: wifi scan start fail ret(%d).r\n",ret);
    }
    return;
}

void stop_wifi_scan_in_other_task(void)
{
    aic_srv_wifi_scan_stop((srv_func_callback)wfs_scan_result_handler);
}

static void WifiScanSampleEntry(void *thread_input)
{
    while (1) {
        app_logi("wifi_scan sample entry point!\n");

        /* 10 seconds */
        aos_sleep(10000);
#ifdef CONFIG_APP
        /* NOTE:this sample need open CONFIG_APP */
        start_wifi_scan_in_wifi_callback();  /* the sample code for start wifiscan in wifi call back*/

        /* 100 seconds */
        aos_sleep(100000);
        stop_wifi_scan_in_wifi_callback();  /* the sample code for stop wifiscan */
#endif
        /* 10 seconds */
        aos_sleep(10000);
        start_wifi_scan_in_other_task();  /* the sample code for start wifiscan in another task */

        /* 100 seconds */
        aos_sleep(100000);
        stop_wifi_scan_in_other_task();  /* the sample code for stop wifiscan */

    }
}

int wifi_scan_init_sample(void)
{
    aos_create_thread(
        "wifi_scan_sample_task",
        WifiScanSampleEntry,
        NULL,
        WIFI_SCAN_SAMPLE_TASK_STK_SIZE,
        0,
        NULL,
        WIFI_SCAN_SAMPLE_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}


AIC_NORMAL_APP_P2_INIT_EXPORT(wifi_scan_init_sample);

