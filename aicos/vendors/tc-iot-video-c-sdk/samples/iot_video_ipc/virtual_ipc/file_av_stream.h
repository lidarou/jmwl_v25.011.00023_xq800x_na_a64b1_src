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

#ifndef _FILE_AV_STREAM_H_
#define _FILE_AV_STREAM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/time.h>

#include "iv_av.h"

int qcloud_get_file_stream_format(iv_avt_video_res_type_e stream_type,
                                  iv_cm_av_data_info_s *av_format);

// is_timer: 是否定时获取模拟IPC
void *qcloud_file_stream_init(iv_avt_video_res_type_e stream_type, uint8_t is_timer);

void qcloud_file_stream_exit(void *handle);

int qcloud_get_audio_from_file(void *handle, iv_cm_aenc_pack_s *aenc_packet);

int qcloud_get_pts_and_seq_from_file_handle(void *handle, uint32_t *seq, uint64_t *pts);

int qcloud_get_video_from_file(void *handle, iv_cm_venc_pack_s *venc_packet);

int qcloud_get_mjpg_from_file(void *handle, iv_cm_venc_pack_s *venc_packet);

int qcloud_update_base_time(void *handle);

int qcloud_get_stream_progress(void *handle);

int qcloud_file_seek_ms(void *handle, uint64_t seek_ms);

#if defined(__cplusplus)
}
#endif

#endif /* _AV_STREAM_PROVIDE_H_ */