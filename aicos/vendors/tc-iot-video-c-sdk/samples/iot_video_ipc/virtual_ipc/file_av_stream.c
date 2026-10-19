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
#define AIC_THIS_FILE    0x029403
#define LOG_MODULE    AMOD_VENDOR

#include "file_av_stream.h"

#include "iv_config.h"
#include "tc_iot_hal.h"
#include "tc_iot_log.h"

typedef struct {
    int num;
    unsigned int frame_len;
    unsigned int offset;
    unsigned int timestamp;
    char type;
} media_index_info;

typedef struct {
    FILE *fp_data;
    FILE *fp_index;
    media_index_info index_info;
    uint64_t seq;
    uint64_t pts;
    uint64_t base_time_ms;  // 每个视频文件或音频文件读完后，重新获取基准时间，base_time_ms
                            // + timestamp作为帧的时间戳
} StreamStatusInfo;

typedef struct {
    int wait_for_i_frame;
    StreamStatusInfo a_stream_status;
    StreamStatusInfo v_stream_status;
    uint8_t is_timer;
    iv_cm_av_data_info_s av_info;
} AvFileStruct;

#define MAX_FILE_NAME_LEN  256
//#define MAX_AUDIO_BUF_SIZE (1024 * 128)
//#define MAX_VIDEO_BUF_SIZE (1024 * 256)

#define INDEX_SUFFIX "_index.txt"

/* remove audio/video buffer since we don't read audio/video file*/
//static uint8_t audio_frame_buf[MAX_AUDIO_BUF_SIZE];
//static uint8_t video_frame_buf[MAX_VIDEO_BUF_SIZE]; 

// 从文件中读取音视频数据来模拟IPC
// 不同流类型使用的音视频文件，根据iv_avt_video_res_type_e区分
// for IV_AVT_VIDEO_RES_FL
static iv_cm_av_data_info_s sg_res_fl_info = {
    .eAudioBitWidth       = IV_CM_AENC_BIT_WIDTH_16,
    .eAudioMode           = IV_CM_AENC_MODE_STEREO,
    .eAudioType           = IV_CM_AENC_TYPE_AAC,
    .eAudioSampleRate     = IV_CM_AENC_SAMPLE_RATE_44100,
    .u32AudioCodecOption  = IV_CM_AAC_TYPE_LC,
    .u32SampleNumPerFrame = 1024,

#ifdef AVT_MJPG_ENABLED
    .eVideoType     = IV_CM_VENC_TYPE_MJPG,
    .u32VideoWidth  = 640,
    .u32VideoHeight = 360,
    .u32Framerate   = 15,
#else
    .eVideoType     = IV_CM_VENC_TYPE_H264,
    .u32VideoWidth  = 160,
    .u32VideoHeight = 90,
    .u32Framerate   = 25,
#endif
};
#ifdef AVT_MJPG_ENABLED
//static const char *sg_video_res_fl_file = "/O/demo_media/video_size160x90_gop50_fps25.h264";
#else
//static const char *sg_video_res_fl_file = "/O/demo_media/video_size160x90_gop50_fps25.h264";
#endif
//static const char *sg_audio_res_fl_file = "/O/demo_media/audio_sample44100_stereo_96kbps.aac";

/* modified for voip*/
// for IV_AVT_VIDEO_RES_SD
static iv_cm_av_data_info_s sg_res_sd_info = {
    .eAudioBitWidth       = IV_CM_AENC_BIT_WIDTH_16,
    .eAudioMode           = IV_CM_AENC_MODE_MONO, //IV_CM_AENC_MODE_STEREO,
    .eAudioType           = IV_CM_AENC_TYPE_PCM, //IV_CM_AENC_TYPE_AAC,
    .eAudioSampleRate     = IV_CM_AENC_SAMPLE_RATE_16000, //IV_CM_AENC_SAMPLE_RATE_44100,
    .u32AudioCodecOption  = IV_CM_AAC_TYPE_LC,
    .u32SampleNumPerFrame = 320, //1024,

#ifdef AVT_MJPG_ENABLED
    .eVideoType     = IV_CM_VENC_TYPE_MJPG,
    .u32VideoWidth  = 640,
    .u32VideoHeight = 360,
    .u32Framerate   = 15,
#else
    .eVideoType     = IV_CM_VENC_TYPE_H264,
    .u32VideoWidth  = 160,
    .u32VideoHeight = 90,
    .u32Framerate   = 25,
#endif
};
#ifdef AVT_MJPG_ENABLED
//static const char *sg_video_res_sd_file = "/O/demo_media/video_size160x90_gop50_fps25.h264";
#else
//static const char *sg_video_res_sd_file = "/O/demo_media/video_size160x90_gop50_fps25.h264";
#endif
//static const char *sg_audio_res_sd_file = "/O/demo_media/audio_sample44100_stereo_96kbps.aac";

// for IV_AVT_VIDEO_RES_HD
static iv_cm_av_data_info_s sg_res_hd_info = {
    .eAudioBitWidth       = IV_CM_AENC_BIT_WIDTH_16,
    .eAudioMode           = IV_CM_AENC_MODE_STEREO,
    .eAudioType           = IV_CM_AENC_TYPE_AAC,
    .eAudioSampleRate     = IV_CM_AENC_SAMPLE_RATE_44100,
    .u32AudioCodecOption  = IV_CM_AAC_TYPE_LC,
    .u32SampleNumPerFrame = 1024,

    .eVideoType     = IV_CM_VENC_TYPE_H264,
    .u32VideoWidth  = 160,
    .u32VideoHeight = 90,
    .u32Framerate   = 25,
};
//static const char *sg_video_res_hd_file = "/O/demo_media/video_size160x90_gop50_fps25.h264";
//static const char *sg_audio_res_hd_file = "/O/demo_media/audio_sample44100_stereo_96kbps.aac";

// for IV_AVT_VIDEO_AUDIO
static iv_cm_av_data_info_s sg_audio_only_info = {
    .eAudioBitWidth       = IV_CM_AENC_BIT_WIDTH_16,
    .eAudioMode           = IV_CM_AENC_MODE_STEREO,
    .eAudioType           = IV_CM_AENC_TYPE_AAC,
    .eAudioSampleRate     = IV_CM_AENC_SAMPLE_RATE_44100,
    .u32AudioCodecOption  = IV_CM_AAC_TYPE_LC,
    .u32SampleNumPerFrame = 1024,

    .eVideoType     = 0,
    .u32VideoWidth  = 0,
    .u32VideoHeight = 0,
    .u32Framerate   = 0,
};
//static const char *sg_audio_only_file = "/O/demo_media/audio_sample44100_stereo_96kbps.aac";

#if 0
static const char *get_audio_file_path(iv_avt_video_res_type_e stream_type)
{
    const char *path = NULL;
    switch (stream_type) {
        case IV_AVT_VIDEO_RES_FL:
            path = sg_audio_res_fl_file;
            break;
        // 这里设置本地回放跟SD一样
        case IV_AVT_VIDEO_RES_PB:
        case IV_AVT_VIDEO_RES_SD:
            path = sg_audio_res_sd_file;
            break;
        case IV_AVT_VIDEO_RES_HD:
            path = sg_audio_res_hd_file;
            break;
        case IV_AVT_VIDEO_AUDIO:
            path = sg_audio_only_file;
            break;
        default:
            printf("invalid stream type %d!\n", stream_type);
            return NULL;
    };

    return path;
}

static const char *get_video_file_path(iv_avt_video_res_type_e stream_type)
{
    const char *path = NULL;
    switch (stream_type) {
        case IV_AVT_VIDEO_RES_FL:
            path = sg_video_res_fl_file;
            break;
        // 这里设置本地回放跟SD一样
        case IV_AVT_VIDEO_RES_PB:
        case IV_AVT_VIDEO_RES_SD:
            path = sg_video_res_sd_file;
            break;
        case IV_AVT_VIDEO_RES_HD:
            path = sg_video_res_hd_file;
            break;
        case IV_AVT_VIDEO_AUDIO:
            break;
        default:
            printf("invalid stream type %d!\n", stream_type);
            return NULL;
    };

    return path;
}
#endif
static uint64_t qcloud_get_tick_ms(void)
{
    struct timeval time_val = {0};
    uint64_t time_ms;

    gettimeofday(&time_val, NULL);
    time_ms = time_val.tv_sec * 1000 + time_val.tv_usec / 1000;

    return time_ms;

}

static int parse_index(FILE *fp, media_index_info *index)
{
    char line[128] = "";
    char *tmp      = fgets(line, sizeof(line), fp);
    if (tmp != NULL) {
        sscanf(line, "%d,%c,%d,%d,%d,", &index->num, &index->type, &index->offset, &index->timestamp,
               &index->frame_len);

        // printf(line);
        // printf("index %d %d %d %d %c \n", index->num, index->frame_len, index->offset,
        // index->timestamp,
        //       index->type);
        return 0;
    }
    return 1;
}

#if 0
static int open_av_file(StreamStatusInfo *av_stream, const char *path)
{
    int file_name_len = 0;
    char index_name[MAX_FILE_NAME_LEN];

    file_name_len = strlen(path);
    if (file_name_len + sizeof(INDEX_SUFFIX) + 1 > MAX_FILE_NAME_LEN) {
        printf("file name or index file name larger than 256\n");
        return 1;
    }

    strcpy(index_name, path);
    char *dot_char_ptr = strrchr(index_name, '.');
    *dot_char_ptr      = '_';
    strcat(index_name, INDEX_SUFFIX);

    dot_char_ptr++;
    if (strstr(dot_char_ptr, "h264") || strstr(dot_char_ptr, "h265") || strstr(dot_char_ptr, "aac")) {
        av_stream->fp_index = fopen(index_name, "r");
        if (av_stream->fp_index == NULL) {
            printf("open file %s error!\n", path);
            return 1;
        }

    } else {
        av_stream->fp_index = NULL;
    }

    av_stream->fp_data = fopen(path, "rb");
    if (av_stream->fp_data == NULL) {
        printf("open file %s error!\n", path);
        if (av_stream->fp_index) {
            fclose(av_stream->fp_index);
            av_stream->fp_index = NULL;
        }
        return 1;
    }

    return 0;
}
#endif
int qcloud_get_file_stream_format(iv_avt_video_res_type_e stream_type, iv_cm_av_data_info_s *av_format)
{
    if (av_format == NULL) {
        printf("invalid params!\n");
        return -1;
    }

    switch (stream_type) {
        case IV_AVT_VIDEO_RES_FL:
            memcpy(av_format, &sg_res_fl_info, sizeof(iv_cm_av_data_info_s));
            break;
        // 这里设置本地回放跟SD一样
        case IV_AVT_VIDEO_RES_PB:
        case IV_AVT_VIDEO_RES_SD:
            memcpy(av_format, &sg_res_sd_info, sizeof(iv_cm_av_data_info_s));
            break;
        case IV_AVT_VIDEO_RES_HD:
            memcpy(av_format, &sg_res_hd_info, sizeof(iv_cm_av_data_info_s));
            break;
        case IV_AVT_VIDEO_AUDIO:
            // only audio
            memcpy(av_format, &sg_audio_only_info, sizeof(iv_cm_av_data_info_s));
            break;
        default:
            printf("invalid stream type %d!\n", stream_type);
            return -1;
    };

    return 0;
}

void *qcloud_file_stream_init(iv_avt_video_res_type_e stream_type, uint8_t is_timer)
{
    //int ret = 0;
    //printf("qcloud_file_stream_init type %d\n", stream_type);
	Log_d("qcloud_file_stream_init type %d\n", stream_type);
    AvFileStruct *p_av_file_handle = NULL;

    p_av_file_handle = malloc(sizeof(AvFileStruct));
    if (!p_av_file_handle) {
        //printf("malloc memory %ld failed!\n", sizeof(AvFileStruct));
        return p_av_file_handle;
    }

    memset(p_av_file_handle, 0, sizeof(AvFileStruct));
    p_av_file_handle->is_timer = is_timer;

    do {
/* We don't need audio/video file*/
#if 0
        const char *audio_name = get_audio_file_path(stream_type);
        ret                    = open_av_file(&p_av_file_handle->a_stream_status, audio_name);
        if (ret) {
            break;
        }
        printf("test audio file:%s\n", audio_name);
        Log_d("test audio file:%s\n", audio_name);

        if (stream_type != IV_AVT_VIDEO_AUDIO) {
            const char *video_name = get_video_file_path(stream_type);
            ret                    = open_av_file(&p_av_file_handle->v_stream_status, video_name);
            if (ret) {
                break;
            }
            printf("test video file:%s\n", video_name);
            Log_d("test video file:%s\n", video_name);
        }
#endif
        qcloud_get_file_stream_format(stream_type, &p_av_file_handle->av_info);
        return p_av_file_handle;
    } while (0);

    printf("init file av reader failed\n");
    qcloud_file_stream_exit(p_av_file_handle);
    return NULL;
}

void qcloud_file_stream_exit(void *handle)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;

    if (p_av_file_handle) {
        if (p_av_file_handle->a_stream_status.fp_data) {
            fclose(p_av_file_handle->a_stream_status.fp_data);
            p_av_file_handle->a_stream_status.fp_data = NULL;
        }
        if (p_av_file_handle->a_stream_status.fp_index) {
            fclose(p_av_file_handle->a_stream_status.fp_index);
            p_av_file_handle->a_stream_status.fp_index = NULL;
        }
        if (p_av_file_handle->v_stream_status.fp_data) {
            fclose(p_av_file_handle->v_stream_status.fp_data);
            p_av_file_handle->v_stream_status.fp_data = NULL;
        }
        if (p_av_file_handle->v_stream_status.fp_index) {
            fclose(p_av_file_handle->v_stream_status.fp_index);
            p_av_file_handle->v_stream_status.fp_index = NULL;
        }

        free(p_av_file_handle);
        printf("file stream close!\n");
    }
}

int get_aac_data_from_file(void *handle, uint8_t *buf_addr, int32_t buf_len)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    if (!(handle)) {
        return -1;
    }

    media_index_info *index_info = &p_av_file_handle->a_stream_status.index_info;

    fseek(p_av_file_handle->a_stream_status.fp_data, index_info->offset, SEEK_SET);
    if (index_info->frame_len > buf_len) {
        printf("audio frame too large (%d)\n", index_info->frame_len);
        return -1;
    }

    uint32_t read_len = 0;
    read_len =
        (uint32_t)fread(buf_addr, sizeof(uint8_t), index_info->frame_len, p_av_file_handle->a_stream_status.fp_data);
    if (read_len != index_info->frame_len) {
        printf("read aac frame error\n");
        return -1;
    }
    return read_len;
}

#if 0 /* We don't need audio/video file*/
int qcloud_get_audio_from_file(void *handle, iv_cm_aenc_pack_s *aenc_packet)
{
    FILE *fpa                        = NULL;
    FILE *fp_index                   = NULL;
    int size                         = 0;
    uint64_t interval                = 0;
    uint8_t *audio_buf               = audio_frame_buf;
    media_index_info *pst_index_info = NULL;
    AvFileStruct *p_av_file_handle   = (AvFileStruct *)handle;

    if (!(handle && aenc_packet)) {
        return -1;
    }

    fpa = p_av_file_handle->a_stream_status.fp_data;
    if (!(fpa)) {
        return -1;
    }

    if (IV_CM_AENC_TYPE_AAC == p_av_file_handle->av_info.eAudioType) {
        fp_index = p_av_file_handle->a_stream_status.fp_index;
        if (!(fp_index)) {
            return -1;
        }
    }

    pst_index_info    = &p_av_file_handle->a_stream_status.index_info;
    uint64_t cur_time = qcloud_get_tick_ms();
    // 定时获取模拟IPC
    if (p_av_file_handle->is_timer) {
        if (p_av_file_handle->a_stream_status.base_time_ms) {
            if (p_av_file_handle->a_stream_status.base_time_ms + pst_index_info->timestamp > cur_time) {
                return -1;
            }
        } else {
            p_av_file_handle->a_stream_status.base_time_ms = cur_time;

            if (IV_CM_AENC_TYPE_AAC == p_av_file_handle->av_info.eAudioType) {
                memset(pst_index_info, 0, sizeof(media_index_info));
                fseek(fp_index, 0, SEEK_SET);
                parse_index(fp_index, pst_index_info);
                p_av_file_handle->a_stream_status.pts = pst_index_info->timestamp;
            }
        }
    }

    switch (p_av_file_handle->av_info.eAudioType) {
        case IV_CM_AENC_TYPE_PCM:
            // 读取pcm音频并编码
            size     = fread(audio_buf, 1, 1024 * (p_av_file_handle->av_info.eAudioMode + 1) * 2, fpa);
            interval = 1000 * size / (p_av_file_handle->av_info.eAudioMode + 1) /
                       (p_av_file_handle->av_info.eAudioBitWidth + 1) / p_av_file_handle->av_info.eAudioSampleRate;
            break;
        case IV_CM_AENC_TYPE_G711A:
        case IV_CM_AENC_TYPE_G711U:
            size     = fread(audio_buf, 1, 1024 * (p_av_file_handle->av_info.eAudioMode + 1) * 1, fpa);
            interval = 1000 * size / (p_av_file_handle->av_info.eAudioMode + 1) / 1 /
                       p_av_file_handle->av_info.eAudioSampleRate;
            break;
        case IV_CM_AENC_TYPE_AAC:
            size = get_aac_data_from_file(p_av_file_handle, audio_buf, MAX_AUDIO_BUF_SIZE);
            break;
        default:
            printf("don't support audio format %d!\n", p_av_file_handle->av_info.eAudioType);
            break;
    }

    if (size <= 0) {
        fseek(fpa, 0L, SEEK_SET);
        if (IV_CM_AENC_TYPE_AAC == p_av_file_handle->av_info.eAudioType) {
            fseek(fp_index, 0, SEEK_SET);
        }
        p_av_file_handle->a_stream_status.base_time_ms = 0;

        printf("audio read fail\n");
        return -1;
    }

    // printf("audio frame time interal %ldms,pts %ld, size %d\n",
    // p_av_file_handle->a_stream_status.pts,
    //       aenc_packet->u64PTS, size);

    aenc_packet->pu8Addr = audio_frame_buf;
    aenc_packet->u32Len  = size;
    aenc_packet->u64PTS  = p_av_file_handle->a_stream_status.pts + pst_index_info->timestamp;
    aenc_packet->u32Seq  = p_av_file_handle->a_stream_status.seq;
    p_av_file_handle->a_stream_status.seq++;

    if (IV_CM_AENC_TYPE_AAC == p_av_file_handle->av_info.eAudioType) {
        if (feof(fp_index)) {
            fseek(fp_index, 0, SEEK_SET);
            p_av_file_handle->a_stream_status.base_time_ms = cur_time;
            p_av_file_handle->a_stream_status.pts += pst_index_info->timestamp;
        }
        parse_index(fp_index, pst_index_info);
    } else {
        if (feof(fpa)) {
            fseek(fpa, 0, SEEK_SET);
            p_av_file_handle->a_stream_status.base_time_ms = cur_time;
            p_av_file_handle->a_stream_status.pts          = 0;
        }
        pst_index_info->timestamp += interval;
    }

    return 0;
}
#endif

/// @brief only PCM Get pts and seq for stream
/// @param handle input reuse file info handle
/// @param seq output seq
/// @param pts out put pts
/// @return 0 if OK
int qcloud_get_pts_and_seq_from_file_handle(void *handle, uint32_t *seq, uint64_t *pts)
{
    AvFileStruct *p_av_file_handle   = (AvFileStruct *)handle;

    if (!handle) {
        return -1;
    }

    *seq  = p_av_file_handle->a_stream_status.seq;
    *pts  = p_av_file_handle->a_stream_status.pts;
    p_av_file_handle->a_stream_status.seq++;
    p_av_file_handle->a_stream_status.pts+=20;

    return 0;
}

#if 0 /* We don't need audio/video file*/
int qcloud_get_video_from_file(void *handle, iv_cm_venc_pack_s *venc_packet)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    if (!(handle && venc_packet)) {
        return -1;
    }

    if ((!p_av_file_handle->v_stream_status.fp_data) || (!p_av_file_handle->v_stream_status.fp_index)) {
        return -1;
    }

    media_index_info *index_info = &p_av_file_handle->v_stream_status.index_info;

    uint64_t cur_time = qcloud_get_tick_ms();
    if (p_av_file_handle->is_timer) {
        if (p_av_file_handle->v_stream_status.base_time_ms) {
            // 根据索引中的时间戳控制帧率
            if (p_av_file_handle->v_stream_status.base_time_ms + index_info->timestamp > cur_time) {
                return -1;
            }
        } else {
            p_av_file_handle->v_stream_status.base_time_ms = cur_time;
            memset(index_info, 0, sizeof(media_index_info));
            fseek(p_av_file_handle->v_stream_status.fp_index, 0, SEEK_SET);
            parse_index(p_av_file_handle->v_stream_status.fp_index, index_info);
        }
    }

    fseek(p_av_file_handle->v_stream_status.fp_data, index_info->offset, SEEK_SET);
    if (index_info->frame_len > MAX_VIDEO_BUF_SIZE) {
        printf("video frame too large (%d)\n", index_info->frame_len);
        return -1;
    }

    uint32_t read_len = 0;
    read_len          = (uint32_t)fread(video_frame_buf, sizeof(uint8_t), index_info->frame_len,
                                        p_av_file_handle->v_stream_status.fp_data);
    if (read_len != index_info->frame_len) {
        printf("read video frame error\n");
        return -1;
    }

    venc_packet->pu8Addr = video_frame_buf;
    venc_packet->u32Len  = index_info->frame_len;
    venc_packet->u64PTS  = p_av_file_handle->v_stream_status.pts + index_info->timestamp;
    venc_packet->u32Seq  = p_av_file_handle->v_stream_status.seq;
    if (index_info->type == 'P') {
        venc_packet->eFrameType = IV_CM_FRAME_TYPE_P;
    } else if (index_info->type == 'I') {
        venc_packet->eFrameType = IV_CM_FRAME_TYPE_I;
    } else {
        venc_packet->eFrameType = IV_CM_FRAME_TYPE_BUTT;
    }

    p_av_file_handle->v_stream_status.seq += 1;
    if (feof(p_av_file_handle->v_stream_status.fp_index)) {
        fseek(p_av_file_handle->v_stream_status.fp_index, 0, SEEK_SET);
        p_av_file_handle->v_stream_status.base_time_ms = cur_time;
        p_av_file_handle->v_stream_status.pts += index_info->timestamp;
    }

    memset(index_info, 0, sizeof(media_index_info));
    parse_index(p_av_file_handle->v_stream_status.fp_index, index_info);

    return 0;
}
#endif

int qcloud_update_base_time(void *handle)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    if (!(handle)) {
        return -1;
    }

    if ((!p_av_file_handle->v_stream_status.fp_data) || (!p_av_file_handle->v_stream_status.fp_index)) {
        return -1;
    }

    uint64_t cur_time                              = qcloud_get_tick_ms();
    p_av_file_handle->v_stream_status.base_time_ms = cur_time;
    p_av_file_handle->a_stream_status.base_time_ms = cur_time;

    return 0;
}

// 设备端使用一个文件循环播放进行模拟，这里只考虑在一个文件内查询播放进度的情况，联调验证无误就行
// 超过一个文件的时长计算比较复杂，暂不考虑
int qcloud_get_stream_progress(void *handle)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    if (!(handle)) {
        return 0;
    }
    printf("progress time ms %d \n", p_av_file_handle->v_stream_status.index_info.timestamp);
    return p_av_file_handle->v_stream_status.index_info.timestamp;

    // printf("progress seq %d frame rate %d %p\n", p_av_file_handle->v_stream_status.seq,
    //       p_av_file_handle->av_info.u32Framerate, handle);
    // return (p_av_file_handle->v_stream_status.seq * (1000 / p_av_file_handle->av_info.u32Framerate));
}

// seek 到指定时间戳之前的最近一个I帧
// 重新调整时间戳，否则上面的计时器会限制，导致不读视频帧
int qcloud_file_seek_ms(void *handle, uint64_t seek_ms)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    int64_t v_index_seq_new        = 0;
    int64_t v_index_seq_old        = 0;
    int64_t v_index_ms_old         = 0;
    int64_t seek_delta_ms          = 0;
    media_index_info index_info    = {0};
    int32_t ret                    = 0;
    int64_t a_index_old = 0, v_index_old = 0;

    int64_t last_i_frame_pts_ms       = 0;
    int64_t now_i_frame_pts_ms        = 0;
    int64_t last_i_frame_index_offset = 0;
    int64_t now_i_frame_index_offset  = 0;

    int64_t last_a_frame_pts_ms       = 0;
    int64_t now_a_frame_pts_ms        = 0;
    int64_t last_a_frame_index_offset = 0;
    int64_t now_a_frame_index_offset  = 0;

    parse_index(p_av_file_handle->v_stream_status.fp_index, &index_info);
    v_index_seq_old = index_info.num;
    v_index_ms_old  = index_info.timestamp;

    a_index_old = ftell(p_av_file_handle->a_stream_status.fp_index);
    v_index_old = ftell(p_av_file_handle->v_stream_status.fp_index);

    fseek(p_av_file_handle->v_stream_status.fp_index, 0, SEEK_SET);
    while (!feof(p_av_file_handle->v_stream_status.fp_index)) {
        now_i_frame_index_offset = ftell(p_av_file_handle->v_stream_status.fp_index);
        ret                      = parse_index(p_av_file_handle->v_stream_status.fp_index, &index_info);
        if (ret) {
            break;
        }

        if (index_info.type != 'I') {
            continue;
        } else {
            last_i_frame_pts_ms       = now_i_frame_pts_ms;
            now_i_frame_pts_ms        = index_info.timestamp;
            last_i_frame_index_offset = now_i_frame_index_offset;
        }

        if (seek_ms >= last_i_frame_pts_ms && seek_ms < now_i_frame_pts_ms) {
            v_index_seq_new = index_info.num;
            fseek(p_av_file_handle->v_stream_status.fp_index, last_i_frame_index_offset, SEEK_SET);
            parse_index(p_av_file_handle->v_stream_status.fp_index, &p_av_file_handle->v_stream_status.index_info);
            break;
        }
    }

    fseek(p_av_file_handle->a_stream_status.fp_index, 0, SEEK_SET);
    while (!feof(p_av_file_handle->a_stream_status.fp_index)) {
        now_a_frame_index_offset = ftell(p_av_file_handle->a_stream_status.fp_index);
        ret                      = parse_index(p_av_file_handle->a_stream_status.fp_index, &index_info);
        if (ret) {
            break;
        }

        last_a_frame_pts_ms       = now_a_frame_pts_ms;
        now_a_frame_pts_ms        = index_info.timestamp;
        last_a_frame_index_offset = now_a_frame_index_offset;

        // 以I帧的时间戳为准seek音频时间戳
        if (last_i_frame_pts_ms >= last_a_frame_pts_ms && last_i_frame_pts_ms < now_a_frame_pts_ms) {
            fseek(p_av_file_handle->a_stream_status.fp_index, last_a_frame_index_offset, SEEK_SET);
            parse_index(p_av_file_handle->a_stream_status.fp_index, &p_av_file_handle->a_stream_status.index_info);
            break;
        }
    }

    // 更新时间戳 base_time
    seek_delta_ms = last_i_frame_pts_ms - v_index_ms_old;
    p_av_file_handle->v_stream_status.base_time_ms -= seek_delta_ms;
    p_av_file_handle->a_stream_status.base_time_ms -= seek_delta_ms;

    printf("seek time ms %d \n", p_av_file_handle->v_stream_status.index_info.timestamp);

    if (ret) {
        // 还原之前的index位置
        fseek(p_av_file_handle->a_stream_status.fp_index, a_index_old, SEEK_SET);
        fseek(p_av_file_handle->v_stream_status.fp_index, v_index_old, SEEK_SET);
        return -1;
    }

    return 0;
}

#if 0 /* We don't need audio/video file*/
static uint32_t get_frame_from_file(const char *file_name, uint8_t *frame, size_t max_len)
{
    if (!file_name || !frame) {
        Log_e("invalid params");
        return 0;
    }

    FILE *f = fopen(file_name, "rb");
    if (f == NULL) {
        Log_e("open file %s error", file_name);
        return 0;
    }

    fseek(f, 0L, SEEK_END);
    size_t length = ftell(f);
    rewind(f);

    if (length == 0 || length > max_len) {
        Log_e("file %s length is zero or too big!", file_name);
        fclose(f);
        return 0;
    }

    size_t ret = fread(frame, 1, length, f);
    fclose(f);

    if (ret == 0) {
        Log_e("read file %s %zu return zero", file_name, length);
        return 0;
    }

    return (uint32_t)ret;
}

// mjpg图片流文件基于h264文件生成
// cd mjpg-input
// ffmpeg -i ../video_size640x360_gop50_fps25.h264 -vf fps=25 t%04d.jpg
// 因此可以复用h264的索引文件来进行帧率控制
#define MJPG_FILES_PATH "/O/demo_media/mjpg-input/t%04d.jpg"

int qcloud_get_mjpg_from_file(void *handle, iv_cm_venc_pack_s *venc_packet)
{
    AvFileStruct *p_av_file_handle = (AvFileStruct *)handle;
    if (!(handle && venc_packet)) {
        return -1;
    }

    // reuse h264 the index file for fps control
    if (!p_av_file_handle->v_stream_status.fp_index) {
        return -1;
    }

    media_index_info *index_info = &p_av_file_handle->v_stream_status.index_info;

    uint64_t cur_time = qcloud_get_tick_ms();
    if (p_av_file_handle->is_timer) {
        if (p_av_file_handle->v_stream_status.base_time_ms != 0) {
            // 根据索引中的时间戳控制帧率
            if (p_av_file_handle->v_stream_status.base_time_ms + index_info->timestamp > cur_time) {
                return -1;
            }
        } else {
            p_av_file_handle->v_stream_status.base_time_ms = qcloud_get_tick_ms();
            memset(index_info, 0, sizeof(media_index_info));
            fseek(p_av_file_handle->v_stream_status.fp_index, 0, SEEK_SET);
            parse_index(p_av_file_handle->v_stream_status.fp_index, index_info);
        }
    }

    char file_name[64];
    HAL_Snprintf(file_name, sizeof(file_name), MJPG_FILES_PATH, index_info->num + 1);

    uint32_t ret = get_frame_from_file(file_name, video_frame_buf, MAX_VIDEO_BUF_SIZE);
    if (ret == 0) {
        Log_e("get jpg frame from file %s error", file_name);
        return -1;
    }
    // Log_d("get jpg file %s ret %u", file_name, ret);
    venc_packet->pu8Addr = video_frame_buf;
    venc_packet->u32Len  = ret;

    venc_packet->u64PTS     = p_av_file_handle->v_stream_status.base_time_ms + index_info->timestamp;
    venc_packet->u32Seq     = p_av_file_handle->v_stream_status.seq;
    venc_packet->eFrameType = IV_CM_FRAME_TYPE_I;

    p_av_file_handle->v_stream_status.seq += 1;
    if (feof(p_av_file_handle->v_stream_status.fp_index)) {
        fseek(p_av_file_handle->v_stream_status.fp_index, 0, SEEK_SET);
        p_av_file_handle->v_stream_status.base_time_ms = qcloud_get_tick_ms();
    }

    memset(index_info, 0, sizeof(media_index_info));
    parse_index(p_av_file_handle->v_stream_status.fp_index, index_info);

    return 0;
}
#endif
