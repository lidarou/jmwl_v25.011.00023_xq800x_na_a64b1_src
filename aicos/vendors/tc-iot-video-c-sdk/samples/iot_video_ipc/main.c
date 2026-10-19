/*****************************************************************************
 * Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1159cf
#define LOG_MODULE    AMOD_VENDOR

#include "av_talk.h"
#include "data_model.h"
#include "iv_config.h"
#include "iv_system.h"
#ifdef CLOUD_STORAGE_ENABLED
#include "cloud_storage.h"
#endif
#include "iv_voip.h"
#include "tc_iot_hal.h"
#include "tc_iot_platform_inc.h"
#include "upgrade.h"
#include "virtual_ipc_ops.h"
#include "tc_iot_log.h"


#include <aic_osa_ex.h>
#include <aic_init.h>
#include <aic_size.h>

extern int params_p2p_log_level;
extern const char *params_device_info_file;
extern wxvoip_os_impl_t os_impl;

#include "getopt.h"
static uint32_t process_exit = 0;
#if 0
static char *usage = "./iot_video_demo -d [device_info_file] -p [p2p log level]";
static int _parse_arguments(int argc, char **argv)
{
    char *optstring = "d:p:";
    int opt;

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'd':
                params_device_info_file = optarg;
                break;
            case 'p':
                params_p2p_log_level = atoi(optarg);
                break;
            default:
                printf("usage:\n%s\n", usage);
                return -1;
        }
    }

    return 0;
}

// Linux平台上利用信号来模拟事件触发和云存上报
// "kill -10 pid" to trigger msg send
static void handle_user_signal(int sig)
{
    uint64_t time = 0;

    if (DEVICE_ONLINE_STATUE != sys_get_status()) {
        printf("device off line");
        return;
    }

    int rc = iv_sys_get_time(&time);
    if (rc) {
        printf("sys get system time failed!\n");
        return;
    }

    printf("trigger msg send\n");
    iv_msg_send_notice(6, time / 1000);

#ifdef CLOUD_STORAGE_ENABLED
    printf("trigger cs event test\n");
    cloud_storage_start_event_test();
#endif
}

static void main_exit(int sig)
{
    printf("vIpcExit by signal:%d\n", sig);
    process_exit = 1;
}

#endif
/* return: net status
    0:net is err, 
    1:net is ok*/
int wait_net_ok(void)
{
    int cnt_time = 0;
    int pdp_cnt = 0;
    while(cnt_time < 25)
    {
        aos_sleep(2000);
        if(TRUE == aic_is_default_pdp_activated())
        {
            pdp_cnt++;
            HAL_Printf("-------net is ok[%d]\n", pdp_cnt);
        }else {
            HAL_Printf("-------net is err\n");
        }

        if(pdp_cnt > 1)
        {
            break;
        }
        
        cnt_time++;
    }

    if(TRUE == aic_is_default_pdp_activated())
        return 1;
    else 
        return 0;

}

void voip_test()
{
     HAL_SleepMs(1000 * 10);
    int rc = -1;
     Log_d("voip begin init:%d rocky ready", rc);
    rc = iv_avt_voip_init(
        &os_impl,
        VOIP_WXA_TYPE_RELEASE,
        "/O/voip/",
        "xxxxxxxxxx",
        "xxxxxxxxx",
        "xxxxxxxxxxx",
        "abc"
    );

     Log_d("voip init:%d rocky ready", rc);

    HAL_SleepMs(1000 * 10);
    voip_video_info_s voip_info = {IV_CM_VENC_TYPE_H264, IV_CM_VENC_TYPE_H264, IV_CM_PIXEL_480x640};
    rc = iv_avt_voip_call(
        IV_CM_STREAM_TYPE_VIDEO,
        "xxxxxxx",
        "xxxxxxxxx",
        "xxxxxxxxxx",
        "xxxxxxxxxxx",
        voip_info,
        1, 1
    );
     Log_d("voip_call:%d rocky ready", rc);
    HAL_SleepMs(1000 * 300);
    iv_avt_voip_hang_up();

}

int iv_demo_main(void)
{
    app_logi("enter iv_demo_main rockyshen  ok");

#ifndef WIN32
    app_logi("rockyshen AICOS rtos PALTFORM");
#else
    app_logi("rockyshen win32 platform");
#endif
    process_exit = 0;
    // signal(SIGTERM, main_exit);
    // signal(SIGINT, main_exit);
    // signal(SIGUSR1, handle_user_signal);
//    _parse_arguments(argc, argv);

    
    if(wait_net_ok() == 0)
        return 0;


    int rc = 0;
    // sys模块和dm模块需要先初始化
    rc = sys_init();
    if (rc < 0) {
        printf("sys_init failed\n");
        return -1;
    }   
    app_logi("sys_init rockyshen  ok");

    rc = dm_init(); 
    if (rc < 0) {
        sys_exit();
        printf("dm_init failed\n");
        Log_e("dm_init failed\n");
        return -1;
    }

    app_logi("dm_init rockyshen ok");

    // 初始化音视频P2P模块
#ifdef AVT_P2P_ENABLED  
    av_talk_init();
    app_logi("av_talk_init rockyshen ok");
#endif
    

    // 初始化虚拟IPC，从文件读取音视频数据
#if defined(AVT_P2P_ENABLED) || defined(CLOUD_STORAGE_ENABLED)
    qcloud_virtual_ipc_init();
    app_logi("qcloud_virtual_ipc_init rockyshen ok");
#endif
    

    while (DEVICE_ONLINE_STATUE != sys_get_status()) {
        printf("wait for device online status...\n");
        HAL_SleepMs(1000);
    }

    app_logi("sys_get_status_init rockyshen ok");

    // 云存模块及OTA必须上线后才能初始化成功;
#ifdef CLOUD_STORAGE_ENABLED
    Log_d("cloud_storage_init rocky ready");
    //cloud_storage_init();
    //app_logi("cloud_storage_init rockyshen ok");
#endif

   voip_test();

#ifdef OTA_COMM_ENABLED
    // upgrade_init();
    app_logi("upgrade_init rockyshen ok");
#endif

    while (!process_exit) {
        sleep(1);
    }

    Log_d("will to exit main");
#ifdef OTA_COMM_ENABLED
    // upgrade_exit(); 
#endif

#ifdef CLOUD_STORAGE_ENABLED
    //cloud_storage_exit();
#endif

#ifdef AVT_P2P_ENABLED
    av_talk_exit();
#endif

#if defined(AVT_P2P_ENABLED) || defined(CLOUD_STORAGE_ENABLED)
    qcloud_virtual_ipc_exit();
#endif

    dm_exit();
    sys_exit();

    return 0;
}


static VOID iot_video_mid_demo(void *thread_input)
{
    NX_PARAMETER_NOT_USED(thread_input);
    HAL_Printf("start rockyshen  002 ok");
    iv_demo_main();

    return ;
}

#define DEMO_STACK_SIZE (11 * 1024)
#define IOT_VIDEO_TASK_PRIORITY    35

static aos_thread_id iot_video_thread;
int iot_video_main_thread(void)
{
    app_logi("start rockyshen  001 ok aic sample,%s %s", __DATE__, __TIME__);
    Log_d("stack size:%d", DEMO_STACK_SIZE);   
    iot_video_thread = aos_create_thread("iot_video",
                                          iot_video_mid_demo,
                                          NULL, DEMO_STACK_SIZE, 0, NULL,
                                          IOT_VIDEO_TASK_PRIORITY,
                                          AOS_AUTO_START);
    
     

    return NX_SUCCESS;
}


// AIC_NORMAL_APP_P2_INIT_EXPORT(iot_video_main_thread);
AIC_APP_L1_INIT_EXPORT(iot_video_main_thread);