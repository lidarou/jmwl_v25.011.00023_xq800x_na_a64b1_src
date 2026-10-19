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

#ifndef __VIRTUAL_IPC_OPS_H__
#define __VIRTUAL_IPC_OPS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#include "iv_av.h"

// P2P visitor limit
#define MAX_CONNECT_NUM 4
#define MAX_CS_CHL_NUM 4

int qcloud_virtual_ipc_init(void);

int qcloud_virtual_ipc_exit(void);

// 模拟采集推流
int qcloud_av_enc_start(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type, 
                            iv_cm_time_fragment_s *pb_time);

int qcloud_av_enc_stop(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type);

void qcloud_av_enc_get_enc_info(uint32_t channel, iv_avt_video_res_type_e video_res_type,
                                iv_cm_av_data_info_s *p_av_data_info);

// 模拟收流播放
int qcloud_av_dec_start(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type,
                        iv_cm_av_data_info_s *p_av_data_info);

int qcloud_av_dec_stop(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type);

int qcloud_av_dec_play(uint32_t visitor, uint32_t channel, iv_avt_stream_type_e stream_type,
                       void *pStream);

// 模拟回放控制
int qcloud_av_playback_query(uint32_t visitor, int type, void *args);

int qcloud_av_playback_pause(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type, char pause);

int qcloud_av_playback_get_progress(uint32_t visitor, uint32_t channel, iv_avt_video_res_type_e video_res_type);

int qcloud_av_playback_seek(uint32_t visitor, uint32_t channel, 
                iv_avt_video_res_type_e video_res_type, iv_cm_pb_seek_s seek);

// 模拟文件下载
int qcloud_av_download_query(uint32_t visitor, void *args);

int qcloud_av_download_start(uint32_t visitor, uint32_t channel, void *args);

int qcloud_av_download_stop(uint32_t visitor, uint32_t channel);

int qcloud_av_download_running(uint32_t visitor, uint32_t channel, void *args);

// 模拟云存上传
int qcloud_cs_stream_start(uint32_t channel, uint32_t venc_chn);

int qcloud_cs_stream_stop(uint32_t channel, uint32_t venc_chn);

int qcloud_cs_fetch_image(int event_id, uint8_t **pic, int32_t *size);

int qcloud_cs_release_image(uint8_t **pic, int32_t err_code);

int qcloud_cs_get_enc_info(uint32_t channel, uint32_t venc_chn, iv_cm_av_data_info_s *p_av_data_info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VIRTUAL_IPC_OPS_H__ */