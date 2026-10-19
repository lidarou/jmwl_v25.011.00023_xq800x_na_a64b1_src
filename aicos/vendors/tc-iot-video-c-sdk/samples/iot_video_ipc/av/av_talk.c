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
#define AIC_THIS_FILE    0x1c6707
#define LOG_MODULE    AMOD_VENDOR

#include "iv_av.h"
#include "iv_cm.h"
#include "iv_config.h"
#include "tc_iot_log.h"
#include "virtual_ipc_ops.h"

static char sg_is_voice = 0;

static void buf_free_cb(uint8_t *addr, size_t size)
{
    if (addr) {
        free(addr);
    }
}

void av_talk_get_enc_info(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type,
                          iv_cm_av_data_info_s *p_av_data_info)
{
    Log_i("visitor:%d channel:%d stream %d", visitor, channel, video_res_type);
    qcloud_av_enc_get_enc_info(channel, video_res_type, p_av_data_info);
}

void av_talk_start_real_play(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type, void *args)
{
    iv_avt_req_stream_info_s *req_args = (iv_avt_req_stream_info_s *)args;
    if (args) {
        Log_i("recv usr data:%p", args);
        if (req_args->requester) {
            Log_i("stream requester:%s", req_args->requester);
        }
        if (req_args->user_args) {
            Log_i("stream user_args:%p", req_args->user_args);
        }
    }
    Log_i("visitor:%d channel:%d stream %d", visitor, channel, video_res_type);

    if (video_res_type == IV_AVT_VIDEO_RES_PB) {
        iv_cm_time_fragment_s *pb_time = (iv_cm_time_fragment_s *)(req_args->pb_time);
        Log_i("start palyback begin time:%lld and end time %lld", pb_time->begin_time_s, pb_time->end_time_s);
        qcloud_av_enc_start(visitor, channel, video_res_type, pb_time);
    } else {
        qcloud_av_enc_start(visitor, channel, video_res_type, NULL);
    }
}

void av_talk_stop_real_play(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type)
{
    Log_i("visitor %d channel %d stream %d!", visitor, channel, video_res_type);
    qcloud_av_enc_stop(visitor, channel, video_res_type);
}

int av_talk_start_recv_stream(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type,
                              iv_cm_av_data_info_s *p_av_data_info)
{
    Log_i("visitor %d channel %d stream %d", visitor, channel, stream_type);
    if (stream_type == IV_AVT_STREAM_TYPE_AUDIO) {
        Log_i("audio: type(%d), mode(%d), bitwidth(%d), sample rate(%d) sample number(%d)", p_av_data_info->eAudioType,
              p_av_data_info->eAudioMode, p_av_data_info->eAudioBitWidth, p_av_data_info->eAudioSampleRate,
              p_av_data_info->u32SampleNumPerFrame);
    } else if (stream_type == IV_AVT_STREAM_TYPE_VIDEO) {
        Log_i("video: type(%d), width(%d), height(%d), sample rate(%d)", p_av_data_info->eVideoType,
              p_av_data_info->u32VideoWidth, p_av_data_info->u32VideoHeight, p_av_data_info->u32Framerate);
    }

    qcloud_av_dec_start(visitor, channel, stream_type, p_av_data_info);
    sg_is_voice = 1;

    return 0;
}

int av_talk_stop_recv_stream(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type)
{
    Log_i("visitor %d channel %d stream %d ", visitor, channel, stream_type);

    qcloud_av_dec_stop(visitor, channel, stream_type);
    sg_is_voice = 0;
    return 0;
}

int av_talk_recv_stream(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type, void *pStream)
{
    return qcloud_av_dec_play(visitor, channel, stream_type, pStream);
}

void av_talk_notify_process(iv_avt_event_e event, uint32_t visitor, uint32_t channel,
                            iv_avt_video_res_type_e video_res_type)
{
    Log_i("visitor %d channel %d stream %d event id %d", visitor, channel, video_res_type, event);
}

void av_talk_recv_user_data(uint32_t visitor, uint32_t channel, const char *src, uint32_t src_len, iv_cm_memory_s *dst)
{
    Log_i("visitor %d channel %d recv char = %p length = %hu\n", visitor, channel, src, src_len);
    char *val = "Command reply from IoT Video device";
    dst->buf  = (char *)malloc(strlen(val) + 1);

    if (dst->buf) {
        memcpy(dst->buf, val, (strlen(val) + 1));
        dst->size        = strlen(val) + 1;
        dst->buf_free_fn = buf_free_cb;
    }

    if (src_len < 128) {
        Log_d("user command:\n%s\n", src);
    } else {
        Log_d("user command length %u", src_len);
    }
}

int av_talk_get_dev_name_proc(uint32_t visitor, uint32_t channel, iv_cm_memory_s *dev_name, uint8_t *is_online)
{
    Log_i("visitor %d channel %d", visitor, channel);
    dev_name->buf = malloc(32);
    if (!dev_name->buf) {
        Log_e("malloc buffer failed!");
        return -1;
    }
    dev_name->size        = 32;
    dev_name->buf_free_fn = buf_free_cb;
    snprintf(dev_name->buf, 32, "test_ipc%ld", channel);
    *is_online = 1;

    return 0;
}

int av_talk_command_proc(iv_avt_command_type_e command, uint32_t visitor, uint32_t channel,
                         iv_avt_video_res_type_e video_res_type, void *args)
{
    Log_d("command %d visitor %d channel %d stream %d args %p", command, visitor, channel, video_res_type, args);
    int rc         = 0;

    switch (command) {
        case IV_AVT_COMMAND_USR_DATA: {
            iv_avt_usr_data_parm_s *usr_data = (iv_avt_usr_data_parm_s *)args;
            av_talk_recv_user_data(visitor, channel, usr_data->src, usr_data->src_len, &usr_data->dst);
            break;
        }

        case IV_AVT_COMMAND_REQ_STREAM: {
            iv_avt_req_stream_param_s *req_param = (iv_avt_req_stream_param_s *)args;
            Log_d("req stream param %d voice is %s", req_param->request_type, sg_is_voice ? "on" : "off");

            if (IV_AVT_REQUEST_SEND_STREAM == req_param->request_type) {
                // 直播
                req_param->request_result = IV_AVT_DEV_ACCEPT;
            } else {
                // 对讲
                req_param->request_result = sg_is_voice ? IV_AVT_DEV_REFUSE : IV_AVT_DEV_ACCEPT;
            }
            break;
        }

        case IV_AVT_COMMAND_CHN_NAME: {
            iv_avt_chn_name_param_s *chn_name = (iv_avt_chn_name_param_s *)args;
            rc = av_talk_get_dev_name_proc(visitor, channel, &chn_name->name, &chn_name->is_online);
            break;
        }

        case IV_AVT_COMMAND_REQ_IFRAME: {
            Log_d("visitor need idr frame!");
            break;
        }

        case IV_AVT_COMMAND_PLAYBACK_PAUSE: {
            qcloud_av_playback_pause(visitor, channel, video_res_type, 1);
            Log_d("playback pause! visitor %d res %d", visitor, video_res_type);
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_RESUME: {
            qcloud_av_playback_pause(visitor, channel, video_res_type, 0);
            Log_d("playback resume! visitor %d res %d", visitor, video_res_type);
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_QUERY_MONTH: {
            qcloud_av_playback_query(visitor, 1, args);
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_QUERY_DAY: {
            qcloud_av_playback_query(visitor, 0, args);
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_SEEK: {
            iv_cm_pb_seek_s *pb_seek = (iv_cm_pb_seek_s *)args;
            Log_d("playback seek time:%ld", pb_seek->seek_time_ms);
            rc = qcloud_av_playback_seek(visitor, channel, video_res_type, *pb_seek);
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_FF: {
            iv_cm_pb_ff_s *pb_ff = (iv_cm_pb_ff_s *)args;
            Log_d("playback fast forward:%d", pb_ff->speed);
            // TODO
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_SPEED: {
            iv_cm_pb_speed_s *pb_speed = (iv_cm_pb_speed_s *)args;
            Log_d("playback speed time_ms:%d", pb_speed->time_ms);
            // TODO
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_REWIND: {
            iv_cm_pb_rewind_s *pb_rewind = (iv_cm_pb_rewind_s *)args;
            Log_d("playback rewind begin time:%lld and end time:%lld", pb_rewind->begin_time_s, pb_rewind->end_time_s);
            // TODO
            break;
        }
        case IV_AVT_COMMAND_PLAYBACK_PROGRESS: {
            int *progress_time = (int *)args;
            *progress_time     = qcloud_av_playback_get_progress(visitor, channel, video_res_type);
            break;
        }
        case IV_AVT_COMMAND_QUERY_FILE_LIST: {
            qcloud_av_download_query(visitor, args);
            break;
        }
        case IV_AVT_COMMAND_CALL_ANSWER: {
            Log_i("receive command: call answer");
            break;
        }
        case IV_AVT_COMMAND_CALL_HANG_UP: {
            Log_i("receive command: call hang up");
            break;
        }
        case IV_AVT_COMMAND_CALL_REJECT: {
            Log_i("receive command: call reject");
            break;
        }
        case IV_AVT_COMMAND_CALL_CANCEL: {
            Log_i("receive command: call cancel");
            break;
        }
        case IV_AVT_COMMAND_CALL_BUSY: {
            Log_i("receive command: call busy");
            break;
        }
        case IV_AVT_COMMAND_CALL_TIMEOUT: {
            Log_i("receive command: call timeout");
            break;
        }

        default:
            break;
    }

    return rc;
}

int av_talk_download_proc(iv_avt_download_status_e status, uint32_t visitor, uint32_t channel, void *args)
{
    int rc = 0;

    switch (status) {
        case IV_AVT_DOWNLOAD_STATUS_START:
            Log_d("download  status %d visitor %d channel %d  args %p", status, visitor, channel, args);
            rc = qcloud_av_download_start(visitor, channel, args);
            break;
        case IV_AVT_DOWNLOAD_STATUS_RUNNING:
            rc = qcloud_av_download_running(visitor, channel, args);
            break;
        case IV_AVT_DOWNLOAD_STATUS_STOP:
            Log_d("download  status %d visitor %d channel %d  args %p", status, visitor, channel, args);
            rc = qcloud_av_download_stop(visitor, channel);
            break;
        default:
            rc = -1;
            break;
    }
    return rc;
}

int params_p2p_log_level = IV_AVT_P2P_LOG_INFO;

int av_talk_init(void)
{
    int ret = 0;

    iv_avt_init_parm_s stAvtInitParameters;
    memset(&stAvtInitParameters, 0, sizeof(iv_avt_init_parm_s));
#ifdef AVT_MJPG_ENABLED
    stAvtInitParameters.max_frame_size = 64;  // 64kB: this buffer is audio only
#else
    stAvtInitParameters.max_frame_size = 4;  // 384kB: for both audio&video
#endif
    stAvtInitParameters.max_connect_num      = MAX_CONNECT_NUM;
    stAvtInitParameters.congestion.enable    = true;
    stAvtInitParameters.congestion.low_mark  = 100 * 1024;
    stAvtInitParameters.congestion.warn_mark = 200 * 1024;
    stAvtInitParameters.congestion.high_mark = 300 * 1024;

    stAvtInitParameters.p2p_keep_alive.time_inter_s    = 0;
    stAvtInitParameters.p2p_keep_alive.max_attempt_num = 0;
    stAvtInitParameters.iv_avt_get_av_enc_info_cb      = av_talk_get_enc_info;
    stAvtInitParameters.iv_avt_start_real_play_cb      = av_talk_start_real_play;
    stAvtInitParameters.iv_avt_stop_real_play_cb       = av_talk_stop_real_play;
    stAvtInitParameters.iv_avt_start_recv_stream_cb    = av_talk_start_recv_stream;
    stAvtInitParameters.iv_avt_recv_stream_cb          = av_talk_recv_stream;
    stAvtInitParameters.iv_avt_stop_recv_stream_cb     = av_talk_stop_recv_stream;
    stAvtInitParameters.iv_avt_notify_cb               = av_talk_notify_process;
    stAvtInitParameters.iv_avt_recv_command_cb         = av_talk_command_proc;
    stAvtInitParameters.iv_avt_download_file_cb        = av_talk_download_proc;
    stAvtInitParameters.p2p_init_params.log_level      = params_p2p_log_level;
    stAvtInitParameters.p2p_init_params.log_file_path  = "/tmp";
    stAvtInitParameters.p2p_init_params.log_file_size  = 1 * 1024 * 1024;
    stAvtInitParameters.p2p_init_params.protocol       = IV_AVT_P2P_UDP;
    Log_d("avt p2p use protocol :%s", stAvtInitParameters.p2p_init_params.protocol ? "TCP" : "UDP");

    ret = iv_avt_init(&stAvtInitParameters);
    if (ret < 0) {
        Log_e("iv_avt_init error:%d", ret);
        return ret;
    }

    sg_is_voice = 0;

    return ret;
}

int av_talk_exit(void)
{
    return iv_avt_exit();
}
