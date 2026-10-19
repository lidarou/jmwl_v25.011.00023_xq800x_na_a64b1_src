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
#define AIC_THIS_FILE    0x0d8b4d
#define LOG_MODULE    AMOD_VENDOR

#include <sys/time.h>

#include "iv_av.h"
#include "iv_cs.h"
#include "iv_sys.h"
#include "iv_system.h"
#include "tc_iot_hal.h"
#include "tc_iot_log.h"
#include "virtual_ipc_ops.h"

typedef struct {
    ThreadParams params;
    uint8_t start;
    uint8_t running;
    uint8_t end;
} iv_cs_thread_s;

static iv_cs_thread_s sg_event_thread = {0};

// IPC场景，channel = 0
#define CS_CHANNEL_ID 0
// IV_AVT_VIDEO_RES_SD
#define CS_STREAM_ID 1
// 支持多通道
#define CS_CHANNEL_NUM CS_CH_NUM

#define STR_N_CMP(str1, str2) strncmp(str1, str2, sizeof(str2) - 1)

// 简单事件云存测试序列
// start,事件序号
// stop,事件序号
// end 结束测试
// reset 返回第一行重新开始运行
// sleep,延时时间（单位秒）
// event,事件序号,事件持续时间（单位秒）
static const char *event_test_script[] = {"start,2,0",
                                          "sleep,3",   "start,3,0", "sleep,10", "stop,2,0", "sleep,2",
                                          "stop,3,0",  "reset",       NULL};

static void cs_event_proc(void *param)
{
    int32_t chl             = 1;
    int32_t count_down_ms   = 0;
    int32_t event_sleep     = 0;
    int32_t event_id        = 0;
    int32_t event_duration  = 0;
    int32_t event_pre_time  = 0;
    uint32_t event_end_time = 0;
    char event_script_buf[256];
    char event_action_buf[256];
    iv_cs_thread_s *p_event_thread = &sg_event_thread;
    //int cnt                        = 0;

    Log_d("%s thread begin!\n", __func__);
    p_event_thread->end = 0;

    memset(event_script_buf, 0, sizeof(event_script_buf));
    memset(event_action_buf, 0, sizeof(event_action_buf));

    int script_cnt = 0;
    while (p_event_thread->start) {
        //iv_cs_balance_info_s cs_balance = {0};

        if (DEVICE_ONLINE_STATUE != sys_get_status()) {
            HAL_SleepMs(500);
            continue;
        }
#if 0
        int rc = iv_cs_get_balance_info(1, &cs_balance, 0);
        if (rc) {
            printf("get cs balance info failed!\n");
            HAL_SleepMs(2000);
            continue;
        }

        if (0 == cs_balance.cs_switch) {
            if (cnt > 50) {
                printf("cs balance is invalid!\n");
                cnt = 0;
            } else {
                cnt++;
            }
            HAL_SleepMs(1000);
            continue;
        }
#endif
        const char *script = event_test_script[script_cnt++];
        if (script == NULL) {
            printf("end of event test script\n");
            break;
        }
        memset(event_script_buf, 0, sizeof(event_script_buf));
        strncpy(event_script_buf, script, sizeof(event_script_buf) - 1);

        if (event_script_buf[0] == '#' || event_script_buf[0] == '\r' || event_script_buf[0] == '\n') {
            continue;
        }

        if (STR_N_CMP(event_script_buf, "end") == 0) {
            Log_d("event test stop\n");
            break;
        } else if (STR_N_CMP(event_script_buf, "reset") == 0) {
            script_cnt = 0;
            Log_d("event test reset\n");
        } else if (STR_N_CMP(event_script_buf, "start") == 0) {
            sscanf(event_script_buf, "%[^,],%d,%d", event_action_buf, &event_id, &chl);
            Log_d("channel[%d] event[%d] start\n", chl, event_id);
            iv_cs_event_start(chl, event_id);
        } else if (STR_N_CMP(event_script_buf, "stop") == 0) {
            sscanf(event_script_buf, "%[^,],%d,%d", event_action_buf, &event_id, &chl);
            Log_d("channel[%d] event[%d] stop\n", chl, event_id);
            iv_cs_event_stop(chl, event_id);
        } else if (STR_N_CMP(event_script_buf, "event") == 0) {
            sscanf(event_script_buf, "%[^,],%d,%d", event_action_buf, &event_id, &event_duration);
            Log_d("event id: %d, duration: %d\n", event_id, event_duration);
            count_down_ms = event_duration * 1000;

            iv_cs_event_start(chl, event_id);
            while (count_down_ms) {
                count_down_ms -= 100;
                HAL_SleepMs(100);
            }
            iv_cs_event_stop(chl, event_id);
        } else if (STR_N_CMP(event_script_buf, "sleep") == 0) {
            sscanf(event_script_buf, "%[^,],%d", event_action_buf, &event_sleep);
            Log_d("event sleep %d second\n", event_sleep);
            HAL_SleepMs(event_sleep * 1000);
        } else if (STR_N_CMP(event_script_buf, "pre_event") == 0) {
            // 事件指定事件触发开始结束
            sscanf(event_script_buf, "%[^,],%d,%d,%d,%d", event_action_buf, &event_id, &event_duration, &event_pre_time,
                   &chl);
            printf("event id: %ld, duration: %ld, per_time: %ld\n", event_id, event_duration, event_pre_time);
            count_down_ms = (event_duration + event_pre_time) * 1000;

            struct timeval tv;
            memset(&tv, 0, sizeof(tv));
            gettimeofday(&tv, NULL);

            HAL_SleepMs(event_pre_time * 1000);
            iv_cs_event_start_ext(chl, event_id, tv.tv_sec, tv.tv_sec);

            while (count_down_ms) {
                count_down_ms -= 100;
                HAL_SleepMs(100);
            }
            event_end_time = tv.tv_sec + event_duration + event_pre_time;
            iv_cs_event_stop_ext(chl, event_id, event_end_time, event_end_time);
        } else {
            // error
            printf("unknown script %s\n", event_script_buf);
        }
        HAL_SleepMs(40);
    }

    p_event_thread->end   = 1;
    p_event_thread->start = 0;
    printf("%s thread end!\n", __func__);
}

int cloud_storage_stop_event_test(void)
{
    iv_cs_thread_s *p_event_thread = &sg_event_thread;

    if (p_event_thread->start) {
        p_event_thread->start = 0;
        while (!p_event_thread->end) {
            HAL_SleepMs(100);
        }
        printf("%s success!\n", __func__);
    }

    memset(p_event_thread, 0, sizeof(iv_cs_thread_s));
    return 0;
}

// 启动一个线程来测试云存
int cloud_storage_start_event_test(void)
{
    iv_cs_thread_s *p_event_thread = &sg_event_thread;
    if (p_event_thread->start) {
        printf("event test is running\n");
        return -1;
    }

    p_event_thread->start              = 1;
    p_event_thread->end                = 1;
    p_event_thread->params.thread_func = cs_event_proc;
    p_event_thread->params.thread_name = "_cs_event_proc";
    p_event_thread->params.priority    = THREAD_PRIORITY_NORMAL;
    p_event_thread->params.stack_size  = 8192;
    int ret                            = HAL_ThreadCreate(&p_event_thread->params);
    if (ret < 0) {
        p_event_thread->start = 0;
        printf("failed to create the cs_event_proc thread.\n");
        return -1;
    }

    printf("%s success!\n", __func__);
    return 0;
}

static void utc_to_localtime_ms(int64_t time_ms, char *time_buf, int buf_len)
{
    struct tm ltime;
    time_t time_sec = time_ms / 1000;
    localtime_r(&time_sec, &ltime);
    snprintf(time_buf, buf_len, "%04d%02d%02d %02d:%02d:%02d:%03d", ltime.tm_year + 1900, ltime.tm_mon + 1,
             ltime.tm_mday, ltime.tm_hour, ltime.tm_min, ltime.tm_sec, (int)(time_ms % 1000));
}

static int cloudStorage_upload_state(iv_cs_chn_e channel, iv_cs_upload_info_s *info)
{
    int i = 0;
    printf("cloud storage upload state:\n");
    for (i = 0; i < info->num; i++) {
        printf("state %d, size %ld / %ld, frame %ld, %ld, pts %lld" PRIu64 ", %lld" PRIu64 ", utc %lld" PRIu64 ", %lld" PRIu64 "\n",
               info->slice_info[i].state, info->slice_info[i].upload_size, info->slice_info[i].total_size,
               info->slice_info[i].frame_seq_a, info->slice_info[i].frame_seq_b, info->slice_info[i].pts_ms_a,
               info->slice_info[i].pts_ms_b, info->slice_info[i].utc_sec_a, info->slice_info[i].utc_sec_b);
    }
    printf("\n");
    return 0;
}

static int cloudStorage_notify_msg(iv_cs_chn_e channel, iv_cs_notify_msg_type_e notify_msg_type,
                                   iv_cs_notify_msg_data *pst_notify_data)
{
    Log_d("channel[%d] cloud storage notify msg type:%d\n", channel, notify_msg_type);

    switch (notify_msg_type) {
        case IV_CS_AV_UPLOAD_STATE_MSG:
            cloudStorage_upload_state(channel, pst_notify_data->av_result_info);
        default:
            break;
    }

    return 0;
}

static int cloudStorage_image(iv_cs_chn_e channel, int32_t event_id, uint8_t **pic, int32_t *size)
{
    if (event_id > 32 || event_id < 1) {
        printf("event_id(%ld) invalid\n", event_id);
        *pic  = NULL;
        *size = 0;
        return -1;
    }

    Log_d("%s channel[%d], event id:[%d]\n", __func__, channel, event_id);
    return qcloud_cs_fetch_image(event_id, pic, size);
}

static int cloudStorage_image_result(iv_cs_chn_e channel, uint8_t **pic, int32_t err_code)
{
    return qcloud_cs_release_image(pic, err_code);
}

static int cloudStorage_event_result(iv_cs_chn_e channel, iv_cs_event_result_info *pst_result_info)
{
    Log_d("channel:%d cloud storage event[%d] result:%d\n", channel, pst_result_info->event_id,
           pst_result_info->result_code);
    return 0;
}

static int cloudStorage_start(iv_cs_chn_e channel)
{
    Log_d("channel[%d] cloud storage start\n", channel);

    int rc          = 0;
    int64_t time    = 0;
    char buf[256]   = {0};

    rc = iv_sys_get_time(&time);
    if (0 == rc) {
        utc_to_localtime_ms(time, buf, sizeof(buf));
        Log_d("sys get system time is [%s]\n", buf);
    } else {
        Log_d("sys get system time failed!\n");
    }

    return qcloud_cs_stream_start(channel, CS_STREAM_ID);
}

static int cloudStorage_stop(iv_cs_chn_e channel)
{
    Log_d("channel[%d] cloud storage stop\n", channel);
    return qcloud_cs_stream_stop(channel, CS_STREAM_ID);
}

void cloudStorage_ai_service_notify(iv_cs_chn_e channel, unsigned int u32AiServerType, unsigned long long u64UtcExpire)
{
    printf("channel:%d u32AiServerType: %d, u64UtcExpire: %lld\n", channel, u32AiServerType, u64UtcExpire);
}

// 初始化云存服务
int cloud_storage_init(void)
{
    int err_code = 0;
    int chl_id   = 0;
    iv_cs_init_parm_s stCsInitParm;
    iv_cs_channel_params_s stChanParams[CS_CHANNEL_NUM];

    memset(&stCsInitParm, 0, sizeof(iv_cs_init_parm_s));
    memset(&stChanParams, 0, sizeof(iv_cs_channel_params_s) * CS_CHANNEL_NUM);

    stCsInitParm.channel_num = 1;

    for (int i = 0; i < stCsInitParm.channel_num; i++) {
        chl_id = stCsInitParm.channel_num == 1 ? 0 : i + 1;
        // 获取当前通道的音视频编码信息
        if (qcloud_cs_get_enc_info(chl_id, CS_STREAM_ID, &stChanParams[i].av_fmt)) {
            Log_d("get av enc format failed\n");
            return -1;
        }

        if (stChanParams[i].av_fmt.eAudioType == IV_CM_AENC_TYPE_AAC) {
            stChanParams[i].av_fmt.u32AudioCodecOption = IV_CM_AAC_TYPE_LC;
        }

        stChanParams[i].channel_id    = chl_id;
        stChanParams[i].u32MaxGopSize = 512 * 1024;
        stChanParams[i].cs_fmt        = IV_CS_FORMAT_TS;
    }

    stCsInitParm.func_cb.iv_cs_push_stream_start_cb     = cloudStorage_start;
    stCsInitParm.func_cb.iv_cs_push_stream_stop_cb      = cloudStorage_stop;
    stCsInitParm.func_cb.iv_cs_event_capture_picture_cb = cloudStorage_image;
    stCsInitParm.func_cb.iv_cs_event_picture_result_cb  = cloudStorage_image_result;
    stCsInitParm.func_cb.iv_cs_event_report_result_cb   = cloudStorage_event_result;
    stCsInitParm.func_cb.iv_cs_notify_cb                = cloudStorage_notify_msg;
    stCsInitParm.func_cb.iv_cs_ai_service_notify_cb     = cloudStorage_ai_service_notify;
    stCsInitParm.ch_params                              = stChanParams;

    err_code = iv_cs_init(&stCsInitParm);
    if (err_code < 0) {
        printf("iv_cs_init error:%d", err_code);
        return err_code;
    }

    // ! test
    HAL_SleepMs(2000);
    cloud_storage_start_event_test();
    return err_code;
}

int cloud_storage_exit(void)
{
    cloud_storage_stop_event_test();
    return iv_cs_exit();
}
