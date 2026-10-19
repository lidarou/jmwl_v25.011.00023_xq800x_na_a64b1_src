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
#define AIC_THIS_FILE    0x108307
#define LOG_MODULE    AMOD_VENDOR

#include "virtual_ipc_ops.h"

#include "file_av_stream.h"
#include "iv_av.h"
#include "iv_config.h"
#include "iv_cs.h"
#include "iv_system.h"
#include "tc_iot_hal.h"
#include "tc_iot_log.h"
#include "audio_system.h"

//////////////////////////////////////////////////////////////////////////////
// 虚拟摄像头，读取本地音视频文件模拟采集编码，并进行直播点播推流和云存上传
// 这里只考虑单个摄像头采集和播放(channel = 0)的情况，不考虑多个摄像头如NVR场景
// 单个摄像头可以输出多条流数据(流畅/标清/高清/回放/单音频)
// 在单个线程里面轮询各条流的音视频数据，并推送给对端visitor
// 直播对讲接收到对方的语音数据后保存为本地音频文件
// 本地回放支持pause/resume/seek等操作，支持通过P2P下载本地文件
//////////////////////////////////////////////////////////////////////////////

// 流畅/标清/高清/回放
#define MAX_VENC_NUM 4 //Change from 4 to 2 since only 1 is used
// video and audio
#define MAX_DEC_NUM 2

typedef struct {
    char running;
    char pause;
    char waiting_key_frame;
    uint32_t visitor;
    uint32_t channel;
    FILE *fp_save;
    uint32_t status_cnt;
    uint64_t end_time;
    uint64_t pause_time;
} av_visitor_info_s;

// 推流支持同时推给多个P2P visitor
// 后面的MAX_CS_CHL_NUM visitor用于多通道云存上传
typedef struct {
    void *handle;
    char running;
    iv_avt_video_res_type_e video_res_type;
    // MAX_CONNECT_NUM for p2p and MAX_CS_CHL_NUM for cloud storage
    av_visitor_info_s visitor_cfg[MAX_CONNECT_NUM + MAX_CS_CHL_NUM];
    int visitor_cnt;
} av_enc_handle_s;

// 对讲一次只允许一个visitor进行
typedef struct {
    char running;
    uint32_t channel;
    av_visitor_info_s visitor_cfg;
} av_dec_handle_s;

typedef struct {
    char running;
    uint32_t visitor;
    char *blk_buf;
    int blk_size;
    FILE *fp;
} av_download_info_s;

typedef struct {
    ThreadParams thread_params;
    uint8_t start;
    uint8_t end;
    av_enc_handle_s enc_handle[MAX_VENC_NUM];
    av_dec_handle_s dec_handle[MAX_DEC_NUM];

    av_download_info_s download_handle[MAX_CONNECT_NUM];
    void *av_lock;
    int total_enc_visitor;
} av_avt_info_s;

static av_avt_info_s sg_avt_info = {0};

/*only PCM For VoIP*/
ACLIENT audio_dl_handle = 0;
ACLIENT audio_ul_handle = 0;
int g_audio_fmt = ASTREAM_FORMAT_DEFAULT;

static uint64_t qcloud_get_tick_s(void)
{
    struct timeval time_val = {0};
    uint64_t time;

    gettimeofday(&time_val, NULL);
    time = time_val.tv_sec;

    return time;
}

// 在单个线程里面轮询各条流的音视频数据，并推送给对端visitor
static void virtual_ipc_proc(void *pParm)
{
//    int a_rc                       = 0;
//    int v_rc                       = 0;
    int rc                         = 0;
    iv_cm_aenc_stream_s a_stream   = {0};
    iv_cm_aenc_pack_s aenc_packet  = {0};
//    iv_cm_venc_stream_s v_stream   = {0};
//    iv_cm_venc_pack_s venc_packet  = {0};
//    iv_cm_avenc_stream_s av_stream = {0};
    uint32_t visitor               = 0;
    uint32_t channel               = 0;

    /*only PCM. For audio data transmission*/
    #define STREAM_LEN 640
    u8 audio_buf[STREAM_LEN + 4];
    int read_len = 0;

    Log_d("virtual_ipc_proc thread begin!");
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    p_avt_stream_info->end           = 0;

    while (p_avt_stream_info->start) {
        // 遍历各个流通道（流畅/标清/高清/回放/单音频）的音视频数据并推送给对端
        for (int j = 0; j < 2; j++) { //Change from MAX_VENC_NUM to 2 since only 1 is used
            av_enc_handle_s *p_enc_handle = &p_avt_stream_info->enc_handle[j];
            HAL_MutexLock(p_avt_stream_info->av_lock);
    //Log_d("enc_handle[%d]->handle: %p", j, p_enc_handle->handle);
            if (!p_enc_handle->handle) {
                HAL_MutexUnlock(p_avt_stream_info->av_lock);
                continue;
            }
#if 0//only PCM 
#ifdef AVT_MJPG_ENABLED
            if (p_enc_handle->video_res_type == IV_AVT_VIDEO_AUDIO) {
                a_rc = qcloud_get_audio_from_file(p_enc_handle->handle, &aenc_packet);
                v_rc = -1;
            } else {
                a_rc = -1;
                v_rc = qcloud_get_mjpg_from_file(p_enc_handle->handle, &venc_packet);
            }
#else
            a_rc = qcloud_get_audio_from_file(p_enc_handle->handle, &aenc_packet);
            v_rc = qcloud_get_video_from_file(p_enc_handle->handle, &venc_packet);
#endif

    //Log_d("a_rc[%d]: %d, v_rc[%d]: %d", j, a_rc, j, v_rc);
            // 音频和视频都未获取到
            if ((a_rc < 0) && (v_rc < 0)) {
                HAL_MutexUnlock(p_avt_stream_info->av_lock);
                continue;
            }
#endif
            // 遍历各个visitor并推送P2P音视频数据
            for (int k = 0; k < MAX_CONNECT_NUM; k++) {
                if (!p_enc_handle->visitor_cfg[k].running) {
                    continue;
                }
                visitor = p_enc_handle->visitor_cfg[k].visitor;
                channel = p_enc_handle->visitor_cfg[k].channel;//channel;
#if 0 //only PCM
#ifndef AVT_MJPG_ENABLED
                // 等待I帧
                if (p_enc_handle->visitor_cfg[k].waiting_key_frame) {
                    if (v_rc == 0 && venc_packet.eFrameType == IV_CM_FRAME_TYPE_I) {
                        Log_d("got key frame, start sending stream");
                        p_enc_handle->visitor_cfg[k].waiting_key_frame = 0;
                    } else {
                        // Log_d("waiting for key frame......");
                        continue;
                    }
                }
#endif
                // 本地回放通道，暂停操作
                if (p_enc_handle->video_res_type == IV_AVT_VIDEO_RES_PB && p_enc_handle->visitor_cfg[k].pause) {
                    // qcloud_update_base_time(p_enc_handle->handle);
                    // 这句先去掉，否则因为时间戳的原因暂停后seek再播放会失败
                    continue;
                }
                // 本地回放通道，结束回放
                if (p_enc_handle->video_res_type == IV_AVT_VIDEO_RES_PB && p_enc_handle->visitor_cfg[k].end_time) {
                    if (p_enc_handle->visitor_cfg[k].end_time < qcloud_get_tick_s()) {
                        Log_i("send finish stream message");
                        rc = iv_avt_send_finish_stream(visitor, channel, p_enc_handle->video_res_type);
                        if (rc) {
                            Log_e("send visitor %d channel %d stream %d finish failed %d", visitor, channel,
                                  p_enc_handle->video_res_type, rc);
                        }
                        continue;
                    }
                }
#endif
                // 获取到音频
                //if ((v_rc < 0) && (a_rc == 0)) {
                /*only PCM. Get voice data from input device and send*/
                if(p_enc_handle->visitor_cfg[k].running)
                {
                    if(audio_ul_handle == 0)
                    {
                        continue;
                    }
                    read_len = audio_read(audio_ul_handle, audio_buf, STREAM_LEN, 40);
                    if (read_len <= 0)
                    {
                        Log_e("read audio from input device fail, ret=%d", read_len);
                        continue;
                    }

                    Log_d("visitor cfg entry %d used, audio_ul_handle: %p", k, audio_ul_handle);

                    aenc_packet.pu8Addr = audio_buf;
                    aenc_packet.u32Len = read_len;
                    qcloud_get_pts_and_seq_from_file_handle(p_enc_handle->handle, &aenc_packet.u32Seq, &aenc_packet.u64PTS);
                    Log_d("PTS %llu seq %lu data read %d, sending", aenc_packet.u64PTS, aenc_packet.u32Seq, read_len);
                    a_stream.u32PackCount   = 1;
                    a_stream.pstAencPack[0] = &aenc_packet;
                    rc = iv_avt_send_stream(visitor, channel, IV_AVT_VIDEO_RES_SD, IV_AVT_STREAM_TYPE_AUDIO,
                                            &a_stream);
                    if (rc) {
                        Log_e("visitor %d channel %d stream %d send audio failed %d", visitor, channel,
                              p_enc_handle->video_res_type, rc);
                    }
                }
                //}
#if 0//only PCM
                // 获取到视频
                if ((a_rc < 0) && (v_rc == 0)) {
                    v_stream.u32PackCount   = 1;
                    v_stream.pstVencPack[0] = &venc_packet;
#ifdef AVT_MJPG_ENABLED
                    iv_avt_stream_type_e eStreamType = IV_AVT_STREAM_TYPE_MJPG;
#else
                    iv_avt_stream_type_e eStreamType = IV_AVT_STREAM_TYPE_VIDEO;
#endif
                    rc = iv_avt_send_stream(visitor, channel, p_enc_handle->video_res_type, eStreamType, &v_stream);
                    if (rc) {
                        Log_e("visitor %d channel %d stream %d send video failed %d", visitor, channel,
                              p_enc_handle->video_res_type, rc);
                    }
                }

                // 获取到视频和音频
                if ((a_rc == 0) && (v_rc == 0)) {
                    a_stream.u32PackCount   = 1;
                    a_stream.pstAencPack[0] = &aenc_packet;
                    v_stream.u32PackCount   = 1;
                    v_stream.pstVencPack[0] = &venc_packet;
                    // Log_d("visitor %d chn %d stream %d audio pts %ld and video pts %ld key %d",
                    //     visitor, channel, p_enc_handle->video_res_type,
                    //     aenc_packet.u64PTS, venc_packet.u64PTS, venc_packet.eFrameType);

                    // 有些播放器对PTS特别敏感，需要严格递增，故在此判断PTS大小;
                    if (aenc_packet.u64PTS > venc_packet.u64PTS) {
                        rc = iv_avt_send_stream(visitor, channel, p_enc_handle->video_res_type,
                                                IV_AVT_STREAM_TYPE_VIDEO, &v_stream);
                        rc |= iv_avt_send_stream(visitor, channel, p_enc_handle->video_res_type,
                                                 IV_AVT_STREAM_TYPE_AUDIO, &a_stream);
                        if (rc) {
                            Log_e("visitor %d channel %d stream %d send av failed %d", visitor, channel,
                                  p_enc_handle->video_res_type, rc);
                        }
                    } else {
                        rc = iv_avt_send_stream(visitor, channel, p_enc_handle->video_res_type,
                                                IV_AVT_STREAM_TYPE_AUDIO, &a_stream);
                        rc = iv_avt_send_stream(visitor, channel, p_enc_handle->video_res_type,
                                                IV_AVT_STREAM_TYPE_VIDEO, &v_stream);
                        if (rc) {
                            Log_e("visitor %d channel %d stream %d send av failed %d", visitor, channel,
                                  p_enc_handle->video_res_type, rc);
                        }
                    }
                }
                // 查询实时发送状态，进行拥塞控制（调整码率或帧率）
                if ((p_enc_handle->visitor_cfg[k].status_cnt++ % 200) == 0) {
                    size_t send_buf = iv_avt_get_send_stream_buf(visitor, channel, p_enc_handle->video_res_type);

                    iv_p2p_send_stats_s send_status = {0};
                    rc = iv_avt_get_send_stream_status(visitor, channel, p_enc_handle->video_res_type, &send_status);
                    Log_d(
                        "visitor %d stream %d send_buf %zu inst_net_rate %u "
                        "ave_sent_rate %u sum_sent_acked %llu link_mode %u",
                        visitor, p_enc_handle->video_res_type, send_buf, send_status.inst_net_rate,
                        send_status.ave_sent_rate, send_status.sum_sent_acked, send_status.link_mode);

                    // TODO: 直播拥塞控制
                }
#endif
            }

            HAL_MutexUnlock(p_avt_stream_info->av_lock);
# if 0//only PCM
            // 最后4个visitor用于云存上传
            for (int m = MAX_CONNECT_NUM; m < MAX_CONNECT_NUM + MAX_CS_CHL_NUM; m++) {
                if (p_enc_handle->visitor_cfg[m].running) {
                    // 等待I帧
                    if (p_enc_handle->visitor_cfg[m].waiting_key_frame) {
                        if (v_rc == 0 && venc_packet.eFrameType == IV_CM_FRAME_TYPE_I) {
                            Log_d("got key frame, start uploading stream");
                            p_enc_handle->visitor_cfg[m].waiting_key_frame = 0;
                        } else {
                            // Log_d("waiting for key frame......");
                            continue;
                        }
                    }
#ifdef CLOUD_STORAGE_ENABLED
                    // 获取到音频
                    if (a_rc == 0) {
                        rc = iv_cs_push_stream(p_enc_handle->visitor_cfg[m].channel, IV_CM_STREAM_TYPE_AUDIO,
                                               &aenc_packet);
                        if (rc) {
                            Log_e("send cs audio stream failed %d", rc);
                        }
                    }

                    // 获取到视频
                    if (v_rc == 0) {
                        rc = iv_cs_push_stream(p_enc_handle->visitor_cfg[m].channel, IV_CM_STREAM_TYPE_VIDEO,
                                               &venc_packet);
                        if (rc) {
                            Log_e("send cs video stream failed %d", rc);
                        }
                    }
#endif
                }
            }
#endif
        }
        if (p_avt_stream_info->total_enc_visitor)
            HAL_SleepMs(10);
        else
            HAL_SleepMs(100);
    }
    p_avt_stream_info->end = 1;
    Log_d("virtual_ipc_proc thread end!");
}

int qcloud_virtual_ipc_init(void)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;

    if (p_avt_stream_info->start) {
        Log_e("virtual_ipc_proc thread have already run, please exit.");
        return -1;
    }

    p_avt_stream_info->av_lock = HAL_MutexCreate();
    if (p_avt_stream_info->av_lock == NULL) {
        Log_e("creat mutex failed!");
        return -1;
    }

    p_avt_stream_info->start                     = 1;
    p_avt_stream_info->thread_params.thread_func = virtual_ipc_proc;
    p_avt_stream_info->thread_params.thread_name = "_virtual_ipc_proc";
    p_avt_stream_info->thread_params.priority    = THREAD_PRIORITY_NORMAL;
    p_avt_stream_info->thread_params.stack_size  = 4096;
    int ret                                      = HAL_ThreadCreate(&p_avt_stream_info->thread_params);
    if (ret < 0) {
        p_avt_stream_info->start = 0;
        Log_e("failed to create the virtual_ipc_proc thread.");
        return -1;
    }

    return 0;
}

int qcloud_virtual_ipc_exit(void)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;

    p_avt_stream_info->start = 0;
    while (!p_avt_stream_info->end) {
        HAL_SleepMs(10);
    }

    HAL_MutexDestroy(p_avt_stream_info->av_lock);

    memset(p_avt_stream_info, 0, sizeof(av_avt_info_s));

    return 0;
}

int qcloud_av_enc_start(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type,
                        iv_cm_time_fragment_s *pb_time)
{
    int ret = 0;

    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    uint32_t venc_chn                = (uint32_t)video_res_type;
    if (venc_chn >= MAX_VENC_NUM) {
        Log_e("invalid video res %d", video_res_type);
        return -1;
    }

    av_enc_handle_s *p_enc_handle    = &p_avt_stream_info->enc_handle[venc_chn];
    av_visitor_info_s *p_visitor_cfg = p_enc_handle->visitor_cfg;
    int i                            = 0;
    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (p_visitor_cfg[i].running && (p_visitor_cfg[i].visitor == visitor)) {
            Log_e("visitor %d venc %d has already started!", visitor, venc_chn);
            return -1;
        }
    }

    HAL_MutexLock(p_avt_stream_info->av_lock);
    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (!p_visitor_cfg[i].running) {
            p_visitor_cfg[i].running           = 1;
            p_visitor_cfg[i].visitor           = visitor;
            p_visitor_cfg[i].waiting_key_frame = 1;
            p_visitor_cfg[i].end_time          = 0;
            p_visitor_cfg[i].channel           = channel;

            Log_d("visitor cfg entry %d is used for visitor %d", i, visitor);

            // 回放时间段设置
            if (pb_time) {
                if (pb_time->end_time_s > pb_time->begin_time_s) {
                    p_visitor_cfg[i].end_time = qcloud_get_tick_s() + (pb_time->end_time_s - pb_time->begin_time_s);
                } else {
                    // 为了方便演示点播，此处即使时间不对也会发送10s的长度的音视频
                    Log_w("invalid start time %ld and end time %ld", pb_time->begin_time_s, pb_time->end_time_s);
                    p_visitor_cfg[i].end_time = qcloud_get_tick_s() + 10;
                }
            }
            break;
        }
    }

    if (i >= MAX_CONNECT_NUM) {
        Log_e("connect number has reached max value! %d", i);
        HAL_MutexUnlock(p_avt_stream_info->av_lock);
        return -1;
    }

    p_enc_handle->visitor_cnt++;
    p_avt_stream_info->total_enc_visitor++;
    if (p_enc_handle->running) {
        Log_w("venc %d has already started!", venc_chn);
        HAL_MutexUnlock(p_avt_stream_info->av_lock);
        return 0;
    }

    p_enc_handle->handle = qcloud_file_stream_init(venc_chn, 1);
    if (!p_enc_handle->handle) {
        Log_e("venc %d file stream init failed!", venc_chn);
        p_avt_stream_info->total_enc_visitor--;
        p_enc_handle->visitor_cnt--;
        memset(&p_visitor_cfg[i], 0, sizeof(av_visitor_info_s));
        HAL_MutexUnlock(p_avt_stream_info->av_lock);
        return -1;
    }

    p_enc_handle->running        = 1;
    channel                      = 0;
    p_enc_handle->video_res_type = video_res_type;
    HAL_MutexUnlock(p_avt_stream_info->av_lock);

    /*only PCM. Start sending voice to another party. support PCM format. 
      Voice data will be written to buffer for transmission when available*/

    audio_ul_handle = audio_open(ASTREAM_TYPE_VOIP_UL, APLAY_TYPE_STREAM,
                        AUDIO_INPUT_FLAG_COMPRESS, IV_CM_AENC_SAMPLE_RATE_16000, 0,
                        ASTREAM_FORMAT_PCM, ASTREAM_CHN_MONO,
                        ASTREAM_DIR_RECORD,
                        ASTREAM_SOURCE_DEFAULT);
    if (!audio_ul_handle) 
    {
        Log_e("Audio ul open failed\n");
        return -1;
    }
    ret = audio_play_stream(audio_ul_handle, 0, 0);
    if (ret) 
    {
        audio_close(audio_ul_handle);
        audio_ul_handle = 0;
        Log_e("Audio ul play failed, ret=%d\n", ret);
        return -2;
    }

    Log_d("qcloud_av_enc_start stream %d success!", video_res_type);
    return 0;
}

int qcloud_av_enc_stop(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    uint32_t venc_chn                = (uint32_t)video_res_type;
    if (venc_chn >= MAX_VENC_NUM) {
        Log_e("invalid video res %d", video_res_type);
        return -1;
    }

    av_enc_handle_s *p_enc_handle    = &p_avt_stream_info->enc_handle[venc_chn];
    av_visitor_info_s *p_visitor_cfg = p_enc_handle->visitor_cfg;
    HAL_MutexLock(p_avt_stream_info->av_lock);
    for (int i = 0; i < MAX_CONNECT_NUM; i++) {
        if (p_visitor_cfg[i].running && (p_visitor_cfg[i].visitor == visitor)) {
            // p_visitor_cfg[i].running = 0;
            // p_visitor_cfg[i].visitor = 0;
            memset(&p_visitor_cfg[i], 0, sizeof(av_visitor_info_s));
            break;
        }
    }

    if (p_enc_handle->visitor_cnt > 0) {
        p_enc_handle->visitor_cnt--;
    }

    if ((0 == p_enc_handle->visitor_cnt) && (p_enc_handle->handle)) {
        qcloud_file_stream_exit(p_enc_handle->handle);
        p_enc_handle->handle  = NULL;
        p_enc_handle->running = 0;
    }

    if (p_avt_stream_info->total_enc_visitor)
        p_avt_stream_info->total_enc_visitor--;

    HAL_MutexUnlock(p_avt_stream_info->av_lock);

    /*only PCM. Stop voice data transmission*/
    if (audio_ul_handle != 0)
    {
        audio_stop(audio_ul_handle);
        audio_close(audio_ul_handle);
        audio_ul_handle = 0;
        Log_d("UL audio transmission stopped");
    }

    Log_d("qcloud_av_enc_stop stream %d success!", video_res_type);
    return 0;
}

void qcloud_av_enc_get_enc_info(uint32_t channel, iv_avt_video_res_type_e video_res_type,
                                iv_cm_av_data_info_s *pstAvDataInfo)
{
    qcloud_get_file_stream_format(video_res_type, pstAvDataInfo);
}

////////////////////// 模拟收流播放，demo仅保存到文件
int qcloud_av_dec_start(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type,
                        iv_cm_av_data_info_s *pstAvDataInfo)
{
    int ret = 0;
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    if (stream_type > IV_AVT_STREAM_TYPE_VIDEO) {
        Log_e("invalid channel %d or type!", channel, stream_type);
        return -1;
    }

    av_dec_handle_s *p_dec_handle    = &p_avt_stream_info->dec_handle[stream_type];
    av_visitor_info_s *p_visitor_cfg = &p_dec_handle->visitor_cfg;

    if (p_visitor_cfg->running) {
        Log_e("decoder has already started! visitor %d", visitor);
        return -1;
    }

    g_audio_fmt = ASTREAM_FORMAT_DEFAULT;
    //char dec_name[64] = {0};
    if (stream_type == IV_AVT_STREAM_TYPE_AUDIO) {
        switch (pstAvDataInfo->eAudioType) {
            case IV_CM_AENC_TYPE_AAC:
                //snprintf(dec_name, sizeof(dec_name), "./visitor%d_recv_stream.aac", visitor);
                break;
            case IV_CM_AENC_TYPE_G711A:
            case IV_CM_AENC_TYPE_G711U:
                //snprintf(dec_name, sizeof(dec_name), "./visitor%d_recv_stream.g711", visitor);
                break;
            case IV_CM_AENC_TYPE_PCM:
                /*only PCM. Set audio format, PCM supported*/
                g_audio_fmt = ASTREAM_FORMAT_PCM;
                //snprintf(dec_name, sizeof(dec_name), "./visitor%d_recv_stream.pcm", visitor);
                break;
            default:
                Log_e("invalid audio format %d", pstAvDataInfo->eAudioType);
                // return -1;
        }

    } else if (stream_type == IV_AVT_STREAM_TYPE_VIDEO) {
        switch (pstAvDataInfo->eVideoType) {
            case IV_CM_VENC_TYPE_H264:
                //snprintf(dec_name, sizeof(dec_name), "./visitor%d_recv_stream.h264", visitor);
                break;
            case IV_CM_VENC_TYPE_H265:
                //snprintf(dec_name, sizeof(dec_name), "./visitor%d_recv_stream.h265", visitor);
                break;
            default:
                Log_e("invalid video format %d", pstAvDataInfo->eVideoType);
        }
    }
#if 0    
    if (strlen(dec_name)) {
        p_visitor_cfg->fp_save = fopen(dec_name, "wb");
        if (!p_visitor_cfg->fp_save) {
            Log_e("open file %s failed!", dec_name);
            return -1;
        }
    }
#endif
    p_visitor_cfg->running = 1;
    p_visitor_cfg->visitor = visitor;
    p_dec_handle->running  = 1;

    /*only PCM. Start voice play, support PCM format now. 
      Voice data will be written to buffer for play when available*/
    if (g_audio_fmt > ASTREAM_FORMAT_DEFAULT)
    {
        if (0 == audio_dl_handle) {
            Log_e("voip dl open first \n");

            audio_dl_handle = audio_open(ASTREAM_TYPE_VOIP, APLAY_TYPE_STREAM,
                                AUDIO_OUTPUT_FLAG_COMPRESS, pstAvDataInfo->eAudioSampleRate, 0,
                                g_audio_fmt, ASTREAM_CHN_MONO,
                                ASTREAM_DIR_PLAY,
                                ASTREAM_SOURCE_DEFAULT);
            if (!audio_dl_handle)
            {
                Log_e("voip dl open failed\n");
                return -2;
            }
            
            ret = audio_play_stream(audio_dl_handle, 0, 0);
            if (ret != 0) 
            {
                audio_close(audio_dl_handle);
                audio_dl_handle = 0;


                Log_e("voip dl play failed, ret=%d\n", ret);
                return -3;
            }
        } else {
            Log_e("voip dl already open\n");
            return -2;
        }
    }
    else
    {
        Log_e("Stream format not supported.");
        return -4;
    }

    //Log_d("qcloud_av_dec_start %d success! visitor %u， dec_name %s", stream_type, visitor, dec_name);
    Log_d("qcloud_av_dec_start %d success! visitor %u", stream_type, visitor);
    return 0;
}

int qcloud_av_dec_stop(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    if (stream_type > IV_AVT_STREAM_TYPE_VIDEO) {
        Log_e("invalid type %d!", channel, stream_type);
        return -1;
    }

    av_dec_handle_s *p_dec_handle    = &p_avt_stream_info->dec_handle[stream_type];
    av_visitor_info_s *p_visitor_cfg = &p_dec_handle->visitor_cfg;

    if (!p_dec_handle->running || p_visitor_cfg->visitor != visitor) {
        Log_e("decoder for visitor %u is not running!", visitor);
        return -1;
    }
#if 0
    if (p_visitor_cfg->fp_save) {
        fclose(p_visitor_cfg->fp_save);
        p_visitor_cfg->fp_save = NULL;
    }
#endif
    memset(p_visitor_cfg, 0, sizeof(av_visitor_info_s));
    p_dec_handle->running = 0;

    /*only PCM. Stop voice play*/
    if (audio_dl_handle != 0)
    {
        audio_stop(audio_dl_handle);
        audio_close(audio_dl_handle);
        audio_dl_handle = 0;
        g_audio_fmt = ASTREAM_FORMAT_DEFAULT;
        Log_d("DL audio play stopped");
    }

    Log_d("qcloud_av_dec_stop %d success! visitor %u", stream_type, visitor);
    return 0;
}

int qcloud_av_dec_play(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type, void *pStream)
{
    int ret = 0;
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    if (stream_type > IV_AVT_STREAM_TYPE_VIDEO) {
        Log_e("invalid type %d!", channel, stream_type);
        return -1;
    }

    av_dec_handle_s *p_dec_handle    = &p_avt_stream_info->dec_handle[stream_type];
    av_visitor_info_s *p_visitor_cfg = &p_dec_handle->visitor_cfg;

    if (!p_dec_handle->running || p_visitor_cfg->visitor != visitor) {
        Log_e("decoder for visitor %u is not running!", visitor);
        return -1;
    }

    iv_cm_aenc_stream_s *p_a_stream = NULL;
    //iv_cm_venc_stream_s *p_v_stream = NULL;
    switch (stream_type) {
        case IV_AVT_STREAM_TYPE_AUDIO:
            p_a_stream = (iv_cm_aenc_stream_s *)pStream;
            //if (p_visitor_cfg->fp_save) {
            if (audio_dl_handle != 0)//only PCM
            {
                for (int j = 0; j < p_a_stream->u32PackCount; j++)
                {
                    /*
                    fwrite(p_a_stream->pstAencPack[j]->pu8Addr, p_a_stream->pstAencPack[j]->u32Len, 1,
                           p_visitor_cfg->fp_save);
                    fflush(p_visitor_cfg->fp_save);
                    Log_i("Stream len %d is received!", p_a_stream->pstAencPack[j]->u32Len);
                    */

                    /*Chris. Write voice data to output audio device to play*/
                    ret = audio_write(audio_dl_handle, p_a_stream->pstAencPack[j]->pu8Addr, p_a_stream->pstAencPack[j]->u32Len, 21);
                    Log_d("audio_write, len=%d, pts=%d, seq=%d", p_a_stream->pstAencPack[j]->u32Len, p_a_stream->pstAencPack[j]->u64PTS,
                        p_a_stream->pstAencPack[j]->u32Seq);
                    if (ret < 0) 
                    {
                        Log_e("write error, ret=%d", ret);
                    }
                }
            }
            break;
        case IV_AVT_STREAM_TYPE_VIDEO:
#if 0
            p_v_stream = (iv_cm_venc_stream_s *)pStream;
            if (p_visitor_cfg->fp_save) {
                for (int j = 0; j < p_v_stream->u32PackCount; j++) {
                    fwrite(p_v_stream->pstVencPack[j]->pu8Addr, p_v_stream->pstVencPack[j]->u32Len, 1,
                           p_visitor_cfg->fp_save);
                    fflush(p_visitor_cfg->fp_save);
                }
            }
#endif
            break;
        case IV_AVT_STREAM_TYPE_AV:
            Log_e("current version don't support stream type %d", stream_type);
            break;
        default:
            break;
    }

    return 0;
}

static void av_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}

static int av_get_file_size(const char *file_name)
{
    int ret  = 0;
    FILE *fp = NULL;
    if (file_name) {
        fp = fopen(file_name, "r");
        if (!fp) {
            Log_e("invalid file path %s", file_name);
            return 0;
        }
        fseek(fp, 0, SEEK_END);
        ret = (int)ftell(fp);
        fclose(fp);
    }
    return ret;
}

////////////////////// 本地回放控制
int qcloud_av_playback_query(uint32_t visitor, int type, void *args)
{
    // 0 for query by days and 1 for query by months
    if (type) {
        iv_cm_query_rd_by_month_s *rd_month = (iv_cm_query_rd_by_month_s *)args;
        rd_month->day                       = 1;
        Log_d("query month year:%d, month:%d, day:%d", rd_month->year, rd_month->month, rd_month->day);
    } else {
        iv_cm_query_rd_by_day_s *rd_day = (iv_cm_query_rd_by_day_s *)args;
        Log_d("query day start:%lld, end:%lld, type:%d", rd_day->time_fragment.begin_time_s,
              rd_day->time_fragment.end_time_s, rd_day->time_fragment.type);
        rd_day->rd_array = (iv_cm_pb_list_s *)malloc(sizeof(iv_cm_pb_list_s) + 5 * sizeof(iv_cm_time_fragment_s));
        if (!rd_day->rd_array) {
            Log_e("malloc failed!");
            return -1;
        }
        rd_day->free_fn                          = av_free;
        rd_day->rd_array->count                  = 5;
        iv_cm_time_fragment_s vir_video_index[5] = {{0, 1626325200, 1626325800},  /*2021-07-15 13:00:00-13:10:00*/
                                                    {0, 1626326400, 1626327000},  /*2021-07-15 13:20:00-13:30:00*/
                                                    {0, 1626327000, 1626327600},  /*2021-07-15 13:30:00-13:40:00*/
                                                    {0, 1626327600, 1626328200},  /*2021-07-15 13:40:00-13:50:00*/
                                                    {0, 1626328200, 1626328800}}; /*2021-07-15 13:50:00-14:00:00*/
        for (int i = 0; i < 5; i++) {
            rd_day->rd_array->file_list[i].type         = vir_video_index[i].type;
            rd_day->rd_array->file_list[i].begin_time_s = vir_video_index[i].begin_time_s;
            rd_day->rd_array->file_list[i].end_time_s   = vir_video_index[i].end_time_s;
        }
    }

    return 0;
}

int qcloud_av_playback_pause(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type, char pause)
{
    if (video_res_type != IV_AVT_VIDEO_RES_PB) {
        Log_e("invalid res_type for playback control %d", video_res_type);
        return -1;
    }
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    uint32_t venc_chn                = video_res_type;
    av_enc_handle_s *p_enc_handle    = &p_avt_stream_info->enc_handle[venc_chn];
    av_visitor_info_s *p_visitor_cfg = p_enc_handle->visitor_cfg;
    p_visitor_cfg[visitor].pause     = pause;

    // 暂停过程中延长结束时间
    if (pause) {
        p_visitor_cfg[visitor].pause_time = qcloud_get_tick_s();
    } else {
        uint32_t interval = qcloud_get_tick_s() - p_visitor_cfg[visitor].pause_time;
        p_visitor_cfg[visitor].end_time += interval;
        Log_d("increase visitor %u playback endtime by %u", visitor, interval);
    }

    Log_d("%s stream data for visitor %u", pause ? "stop" : "resume", visitor);
    return 0;
}

int qcloud_av_playback_get_progress(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type)
{
    if (video_res_type != IV_AVT_VIDEO_RES_PB) {
        Log_e("invalid res_type for playback control %d", video_res_type);
        return -1;
    }
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    uint32_t venc_chn                = video_res_type;

    return qcloud_get_stream_progress(p_avt_stream_info->enc_handle[venc_chn].handle);
}

int qcloud_av_playback_seek(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type,
                            iv_cm_pb_seek_s seek)
{
    if (video_res_type != IV_AVT_VIDEO_RES_PB) {
        Log_e("invalid res_type for playback control %d", video_res_type);
        return -1;
    }
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    uint32_t venc_chn                = video_res_type;

    return qcloud_file_seek_ms(p_avt_stream_info->enc_handle[venc_chn].handle, seek.seek_time_ms);
}
////////////////////// 本地回放控制

////////////////////// 模拟文件下载
int qcloud_av_download_query(uint32_t visitor, void *args)
{
    if (args == NULL) {
        Log_e("invalid args");
        return -1;
    }

    iv_cm_query_file_list_s *p_file_list = (iv_cm_query_file_list_s *)args;
    Log_d("query file time begin:%" PRIu64 ", end:%" PRIu64, visitor, p_file_list->time_fragment.begin_time_s,
          p_file_list->time_fragment.end_time_s);
    p_file_list->file_array = (iv_cm_file_list_s *)malloc(sizeof(iv_cm_file_list_s) + 2 * sizeof(iv_cm_file_info_s));
    if (!p_file_list->file_array) {
        Log_e("malloc failed!");
        return -1;
    }
    memset(p_file_list->file_array, 0, sizeof(iv_cm_file_list_s) + 2 * sizeof(iv_cm_file_info_s));
    char *extra_info                    = "{\"key1\":0, \"key2\":\"value2\"}";
    p_file_list->free_fn                = av_free;
    p_file_list->file_array->count      = 2;
    iv_cm_file_info_s vir_file_index[2] = {
        {IV_CM_FILE_TYPE_VIDEO, "p2p_test_file.flv", 0, 1633060850, 1633060880, {NULL}}, /*2021-10-01
                                                                                            12:00:00-12:00:30*/
        {IV_CM_FILE_TYPE_PIC, "pic/01.jpg", 0, 1633061080, 1633061080, {NULL}}}; /*2021-10-01 12:04:40-12:05:10*/

    char path[128] = {0};
    for (int i = 0; i < p_file_list->file_array->count; i++) {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "./demo_media/%s", vir_file_index[i].file_name);
        p_file_list->file_array->file_list[i].file_type = vir_file_index[i].file_type;
        strncpy(p_file_list->file_array->file_list[i].file_name, vir_file_index[i].file_name,
                sizeof(p_file_list->file_array->file_list[i].file_name));
        p_file_list->file_array->file_list[i].file_size              = av_get_file_size(path);
        p_file_list->file_array->file_list[i].begin_time_s           = vir_file_index[i].begin_time_s;
        p_file_list->file_array->file_list[i].end_time_s             = vir_file_index[i].end_time_s;
        p_file_list->file_array->file_list[i].extra_info.buf         = extra_info;
        p_file_list->file_array->file_list[i].extra_info.size        = strlen(extra_info);
        p_file_list->file_array->file_list[i].extra_info.buf_free_fn = NULL;
    }

    return 0;
}

int qcloud_av_download_start(uint32_t visitor, uint32_t channel, void *args)
{
    if (!args) {
        Log_e("input parameter is NULL!");
        return -1;
    }

    iv_cm_download_param_s *p_download_file = (iv_cm_download_param_s *)args;
    av_download_info_s *p_download_handle   = sg_avt_info.download_handle;

    int i = 0;
    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (p_download_handle[i].running && (visitor == p_download_handle[i].visitor)) {
            Log_e("visitor %d download has already started!", visitor);
            return -1;
        }
    }

    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (!p_download_handle[i].running) {
            p_download_handle[i].running = 1;
            p_download_handle[i].visitor = visitor;
            break;
        }
    }

    if (i >= MAX_CONNECT_NUM) {
        Log_e("connect number has already max value!\n");
        return -1;
    }

    int rc         = 0;
    char path[128] = {0};
    snprintf(path, sizeof(path), "./demo_media/%s", p_download_file->file_name);
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        Log_e("open file %s failed!", path);
        return -1;  // 关闭下载
    }
    fseek(fp, p_download_file->file_offset, SEEK_SET);
    p_download_handle[i].blk_size = 4096;
    char *buf                     = malloc(p_download_handle[i].blk_size);
    if (!buf) {
        Log_e("malloc buffer %d failed!", p_download_handle[i].blk_size);
        fclose(fp);
        fp = NULL;
        rc = -1;  // 关闭下载
    }

    p_download_handle[i].fp      = fp;
    p_download_handle[i].blk_buf = buf;

    if (rc) {
        qcloud_av_download_stop(visitor, channel);
    }
    return rc;
}

int qcloud_av_download_stop(uint32_t visitor, uint32_t channel)
{
    int rc                                = 0;
    av_download_info_s *p_download_handle = sg_avt_info.download_handle;

    int i = 0;
    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (p_download_handle[i].running && (p_download_handle[i].visitor == visitor)) {
            p_download_handle[i].running = 0;
            p_download_handle[i].visitor = 0;
            break;
        }
    }

    if (i >= MAX_CONNECT_NUM) {
        Log_e("visitor %d download is invalid!", visitor);
        return -1;
    }

    if (p_download_handle[i].fp) {
        fclose(p_download_handle[i].fp);
        p_download_handle[i].fp = NULL;
    }

    if (p_download_handle[i].blk_buf) {
        free(p_download_handle[i].blk_buf);
        p_download_handle[i].blk_buf = NULL;
    }

    memset(&p_download_handle[i], 0, sizeof(av_download_info_s));
    return rc;
}

int qcloud_av_download_running(uint32_t visitor, uint32_t channel, void *args)
{
    if (!args) {
        Log_e("input parameter is NULL!");
        return -1;
    }

    iv_cm_memory_s *p_data_buf            = (iv_cm_memory_s *)args;
    av_download_info_s *p_download_handle = sg_avt_info.download_handle;
    int rc                                = 0;
    int i                                 = 0;
    for (i = 0; i < MAX_CONNECT_NUM; i++) {
        if (p_download_handle[i].running && (p_download_handle[i].visitor == visitor)) {
            break;
        }
    }

    if (i >= MAX_CONNECT_NUM) {
        Log_e("visitor %d download is invalid!", visitor);
        return -1;
    }

    if (feof(p_download_handle[i].fp)) {
        p_data_buf->buf         = NULL;
        p_data_buf->size        = -1;
        p_data_buf->buf_free_fn = NULL;
        return -1;
    }

    uint32_t read_len = 0;
    if (p_download_handle[i].fp && p_download_handle[i].blk_buf) {
        read_len = fread(p_download_handle[i].blk_buf, 1, p_download_handle[i].blk_size, p_download_handle[i].fp);
        if (read_len <= 0) {
            Log_e("read frame error!");
            p_data_buf->buf         = NULL;
            p_data_buf->size        = -1;
            p_data_buf->buf_free_fn = NULL;
            return -1;
        }
    }

    p_data_buf->buf         = p_download_handle[i].blk_buf;
    p_data_buf->size        = read_len;
    p_data_buf->buf_free_fn = NULL;

    return rc;
}
////////////////////// 模拟文件下载

////////////////////// 云存上传
int qcloud_cs_stream_start(uint32_t channel, uint32_t venc_chn)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;

    if (venc_chn >= MAX_VENC_NUM) {
        Log_e("invalid video res %d", venc_chn);
        return -1;
    }

    av_enc_handle_s *p_enc_handle    = &p_avt_stream_info->enc_handle[venc_chn];
    av_visitor_info_s *p_visitor_cfg = p_enc_handle->visitor_cfg;
    int i                            = MAX_CONNECT_NUM + channel;  // 最后两个visitor用于云存上传
    if (p_visitor_cfg[i].running) {
        Log_e("cloud storage in venc %d has already started!", venc_chn);
        return -1;
    }

    HAL_MutexLock(p_avt_stream_info->av_lock);
    p_visitor_cfg[i].running           = 1;
    p_visitor_cfg[i].visitor           = 0xcd;
    p_visitor_cfg[i].waiting_key_frame = 1;
    p_visitor_cfg[i].end_time          = 0;
    p_visitor_cfg[i].channel           = channel;

    p_enc_handle->visitor_cnt++;
    p_avt_stream_info->total_enc_visitor++;
    if (p_enc_handle->running) {
        Log_w("venc %d has already started!", venc_chn);
        HAL_MutexUnlock(p_avt_stream_info->av_lock);
        return 0;
    }

    p_enc_handle->handle = qcloud_file_stream_init(venc_chn, 1);
    if (!p_enc_handle->handle) {
        Log_e("venc %d file stream init failed!", venc_chn);
        p_avt_stream_info->total_enc_visitor--;
        p_enc_handle->visitor_cnt--;
        memset(&p_visitor_cfg[i], 0, sizeof(av_visitor_info_s));
        HAL_MutexUnlock(p_avt_stream_info->av_lock);
        return -1;
    }

    p_enc_handle->running        = 1;
    p_enc_handle->video_res_type = venc_chn;
    HAL_MutexUnlock(p_avt_stream_info->av_lock);

    Log_d("cloud storage channel[%d] visitor[%d] stream[%d] start success!", channel, i, venc_chn);
    return 0;
}

int qcloud_cs_stream_stop(uint32_t channel, uint32_t venc_chn)
{
    av_avt_info_s *p_avt_stream_info = &sg_avt_info;
    if (venc_chn >= MAX_VENC_NUM) {
        Log_e("invalid video res %d", venc_chn);
        return -1;
    }

    av_enc_handle_s *p_enc_handle    = &p_avt_stream_info->enc_handle[venc_chn];
    av_visitor_info_s *p_visitor_cfg = p_enc_handle->visitor_cfg;
    int i                            = MAX_CONNECT_NUM + channel;  // 最后一个visitor用于云存上传
    if (!p_visitor_cfg[i].running) {
        Log_e("cloud storage in venc %d is not running!", venc_chn);
        return -1;
    }

    HAL_MutexLock(p_avt_stream_info->av_lock);
    memset(&p_visitor_cfg[i], 0, sizeof(av_visitor_info_s));

    if (p_enc_handle->visitor_cnt > 0) {
        p_enc_handle->visitor_cnt--;
    }

    if ((0 == p_enc_handle->visitor_cnt) && (p_enc_handle->handle)) {
        qcloud_file_stream_exit(p_enc_handle->handle);
        p_enc_handle->handle  = NULL;
        p_enc_handle->running = 0;
    }

    if (p_avt_stream_info->total_enc_visitor)
        p_avt_stream_info->total_enc_visitor--;

    HAL_MutexUnlock(p_avt_stream_info->av_lock);

    Log_d("cloud storage stream %u stop success!", venc_chn);
    return 0;
}

int qcloud_cs_fetch_image(int event_id, uint8_t **pic, int32_t *size)
{
    char img_path[64];
    int file_size    = 0;
    int ret          = 0;
    uint8_t *img_buf = NULL;
    FILE *img_fp     = NULL;

    HAL_Snprintf(img_path, sizeof(img_path), "/O/demo_media/pic/%02d.jpg", event_id);
    img_fp = fopen(img_path, "rb");
    if (img_fp == NULL) {
        Log_e("can not open %s", img_path);
        goto err_exit;
    }

    fseek(img_fp, 0, SEEK_END);
    file_size = ftell(img_fp);
    fseek(img_fp, 0, SEEK_SET);

    Log_e("file %s size is %d Byte!", img_path, file_size);
    if (file_size == 0) {
        Log_e("file %s size is zero!", img_path);
        goto err_exit;
    }

    img_buf = (uint8_t *)malloc(file_size);
    if (img_buf == NULL) {
        Log_e("malloc img_buf size %d error", file_size);
        goto err_exit;
    }

    ret = fread(img_buf, 1, file_size, img_fp);
    if (ret != file_size) {
        Log_e("read file %s error %d", img_path, ret);
        goto err_exit;
    }

    *pic  = img_buf;
    *size = file_size;
    fclose(img_fp);
    Log_d("cloud storage get event image %p", *pic);
    return 0;

err_exit:
    if (img_fp)
        fclose(img_fp);
    if (img_buf)
        free(img_buf);
    *pic  = NULL;
    *size = 0;
    return -1;
}

int qcloud_cs_release_image(uint8_t **pic, int32_t err_code)
{
    Log_d("cloud storage release event image %p", *pic);

    if (*pic) {
        free(*pic);
    }

    return 0;
}

int qcloud_cs_get_enc_info(uint32_t channel, uint32_t venc_chn, iv_cm_av_data_info_s *p_av_data_info)
{
    return qcloud_get_file_stream_format(venc_chn, p_av_data_info);
}