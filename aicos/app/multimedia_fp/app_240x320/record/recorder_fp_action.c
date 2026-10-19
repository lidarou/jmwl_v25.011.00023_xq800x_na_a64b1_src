

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the main page ui and flow.
 */
/*******************************************
 *              INCLUDES
 ******************************************/
#include "recorder_fp_action.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef WIN32
#include "aic_osa_ex.h"
#endif

#include "fs_api.h"
#include "../music/fp_playservice.h"

#include "app_log.h"
#include "recorder_fp_srv.h"
#include "app_audio.h"
#include "tts_app_player.h"
#include "audio_system.h"

#define __inf                   APP_LOGI


extern struct recorder_service * g_recorder_srv;
extern int32_t fp_app_audio_get_file_duration(char * p_file_name,uint32_t * p_duration);

/*******************************************
 *              STATIC PROTOTYPES
 ******************************************/

/*******************************************
 *              STATIC VARIABLES
 ******************************************/

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
static void recorder_folder_init()
{
    DIR *           dir;
    int             fd;

    dir                 = opendir(APP_RECORDER_FILE_DIR);

    if (dir == NULL) {
        fd                  = mkdir(APP_RECORDER_FILE_DIR,O_RDWR | O_CREAT);
        __inf("mkdir:%s,fd:%d",APP_RECORDER_FILE_DIR,fd);
        }
    else {
        closedir(dir);
        }

}


uint32_t is_recorder_folder_empty()
{
    uint32_t        ret;
    DIR *           dir;

    struct dirent * dinfo = NULL;
    dir                 = opendir(APP_RECORDER_FILE_DIR);

    if (dir == NULL) {
        __inf("is_recorder_folder_empty dir_sms==null");
        return 1;
        }

#if 1
    dinfo               = readdir(dir);

    if (dinfo == NULL)
        ret = 1;
    else 
        ret = 0;

#else

    ret                 = -0;

    while (NULL != (dinfo = readdir(dir))) {

        __inf("sms recive file:%s",dinfo->d_name);

        ret                 = 1;

        }

#endif

    closedir(dir);

    return ret;

}






void app_recorder_resume(uint8_t * name)
{
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_RECOD;
    msg.msg_id          = QUEUE_PS_ID_RESUME;
    msg.p_param         = name;
    msg.size            = 0;
    ps_data_msg_send(&msg,AOS_NO_WAIT);
}


void app_recorder_resume_action(uint8_t * name)
{
    struct recorder_service * recs = g_recorder_srv;
    char *          url = NULL;
    aos_systime     cur_time = {
        0
        };

    if (name == NULL)
        return;

    if (recs->state == RECD_IDLE) {
        // TODO: for playnow now
        if (recs->file.url) {
            aic_free(recs->file.url);
            }

        url                 = aic_zalloc(RECORDER_URL_MAX_LENGTH);

        if (url == NULL) {
            __inf("recorder start recording failed (zalloc failed).\n");
            return;
            }

        aos_get_systime(&cur_time);

        sprintf(url,"%s/%s%d%02d%02d%02d%02d%02d%s",APP_RECORDER_FILE_DIR,name,
            cur_time.year,
            cur_time.month,
            cur_time.day,
            cur_time.hour,
            cur_time.minute,
            cur_time.second,
            APP_RECORDER_FILE_FORMAT_EXT);


        recs->file.format   = APP_RECORDER_DEFAULT_FORMAT;
        recs->file.url      = url;

        recorder_folder_init();

        if (app_srv_recording_start(recs)) {
            __inf("recorder start recording failed (srv start failed)\n");
            aic_free(url);
            recs->file.url      = NULL;
            return;
            }

        }
    else {
        app_srv_recording_resume(recs);
        }
}


void app_recorder_call_resume(uint8_t * name)
{
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_RECOD;
    msg.msg_id          = QUEUE_PS_ID_RESUME;
    msg.p_param         = name;
    msg.size            = 4;
    ps_data_msg_send(&msg,AOS_NO_WAIT);
}


void app_recorder_call_resume_action(uint8_t * name)
{
    struct recorder_service * recs = g_recorder_srv;
    char *          url = NULL;
    aos_systime     cur_time = {
        0
        };

    if (name == NULL)
        return;

    if (recs->state == RECD_IDLE) {
        // TODO: for playnow now
        if (recs->file.url) {
            aic_free(recs->file.url);
            }

        url                 = aic_zalloc(RECORDER_URL_MAX_LENGTH);

        if (url == NULL) {
            __inf("recorder start recording failed (zalloc failed).\n");
            return;
            }

        aos_get_systime(&cur_time);

        sprintf(url,"%s/%s%d%02d%02d%02d%02d%02d%s",APP_RECORDER_FILE_DIR,name,
            cur_time.year,
            cur_time.month,
            cur_time.day,
            cur_time.hour,
            cur_time.minute,
            cur_time.second,
            APP_RECORDER_FILE_FORMAT_EXT);


        recs->file.format   = APP_RECORDER_DEFAULT_FORMAT;
        recs->file.url      = url;

        recorder_folder_init();

        if (app_srv_recording_call_start(recs)) {
            __inf("recorder start recording failed (srv start failed)\n");
            aic_free(url);
            recs->file.url      = NULL;
            return;
            }

        }
    else {
        app_srv_recording_resume(recs);
        }
}


void app_recorder_pause(void)
{
    ps_data_msg_send_record(QUEUE_PS_ID_PAUSE);
}


void app_recorder_pause_action(void)
{
    struct recorder_service * recs = g_recorder_srv;

    if (app_srv_recording_pause(recs)) {
        __inf("recorder pause recording failed (pause failed)\n");
        app_srv_recording_stop(recs);
        recs->state         = RECD_IDLE;
        return;
        }

    recs->state         = RECD_PAUSE;
}


void app_recorder_stop(void)
{
    ps_data_msg_send_record(QUEUE_PS_ID_STOP);
}


void app_recorder_stop_action(void)
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs == NULL)
        return;

    if (recs->state == RECD_RECORD || recs->state == RECD_PAUSE) {

        if (app_srv_recording_stop(recs)) {
            __inf("recorder stop recording failed (pause failed)\n");

            //return;
            }
        }
    else if (recs->state == RECD_PLAY) {
        app_srv_recording_playstop(recs);
        }

    if (recs->file.url) {
        aic_free(recs->file.url);
        recs->file.url      = NULL;
        }
}


uint32_t app_recorder_get_record_status()
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs->state != RECD_IDLE)
        return 1;

    else 
        return 0;
}


void app_recorder_set_play_path(char * path)
{
    struct recorder_service * recs = g_recorder_srv;
    char *          url;

    if (path) {
        url                 = aic_zalloc(RECORDER_URL_MAX_LENGTH);
        strcpy(url,path);

        if (recs->file.url) {
            aic_free(recs->file.url);
            }

        recs->file.url      = url;
        __inf("app_recorder_set_play_path:%s",recs->file.url);
        fp_app_audio_get_file_duration(recs->file.url,(uint32_t *) &recs->file.duration);
        }

}


void app_recorder_play()
{
    ps_data_msg_send_record(QUEUE_PS_ID_PLAY);
}


void app_recorder_play_action()
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs->state != RECD_IDLE) {
        __inf("Try to play when NOT idle\n");
        return;
        }

    if (recs->file.url == NULL) {
        __inf("No recording to play\n");
        return;
        }

    if (-1 == fp_app_audio_get_amr_duration(recs->file.url,NULL))
        return;

    recs->file.format   = APP_RECORDER_DEFAULT_FORMAT;

    if (app_srv_recording_play(recs)) {
        __inf("recorder stop recording failed (pause failed)\n");
        return;
        }
}


void app_recorder_play_pause()
{
    ps_data_msg_send_record(QUEUE_PS_ID_PLAY_PAUSE);

}


void app_recorder_play_pause_action()
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs->state == RECD_IDLE) {
        app_recorder_play_action();
        }
    else if ((recs->state == RECD_PLAY) || (recs->state == RECD_PAUSE)) {
        if (AOS_ERROR == app_srv_recording_playpause(recs))
            app_recorder_stop_action();
        }
}


void app_recorder_play_pause_path(uint8_t * path)
{
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_RECOD;
    msg.msg_id          = QUEUE_PS_ID_PLAY_PAUSE_PATH;
    msg.p_param         = path;
    ps_data_msg_send(&msg,AOS_NO_WAIT);

}


void app_recorder_play_pause_path_action(uint8_t * path)
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs->state == RECD_IDLE) {
        app_recorder_set_play_path(path);
        app_recorder_play_action();
        }
    else if ((recs->state == RECD_PLAY) || (recs->state == RECD_PAUSE)) {
        if (AOS_ERROR == app_srv_recording_playpause(recs))
            app_recorder_stop_action();
        }
}


void app_recorder_fast_forward()
{
    u32             timestamp;

    struct recorder_service * recs = g_recorder_srv;

    if ((recs->state == RECD_PLAY) || (recs->state == RECD_PAUSE)) {
        timestamp           = audio_get_timestamp(recs->ahandle) + 1000;

        if (timestamp < recs->file.duration)
            audio_seek(recs->ahandle,timestamp);
        }
}


void app_recorder_fast_rewind()
{
    u32             timestamp;

    struct recorder_service * recs = g_recorder_srv;

    if ((recs->state == RECD_PLAY) || (recs->state == RECD_PAUSE)) {
        timestamp           = audio_get_timestamp(recs->ahandle);

        if (timestamp > 1000) {
            timestamp           = timestamp - 1000;
            audio_seek(recs->ahandle,timestamp);
            }
        }
}





u32 app_recorder_getvol()
{
#ifdef CONFIG_AUDIO_TTS
    /* Use global volume now */
    return app_fp_record_get_vol();
#else
    return audio_get_master_volume();
#endif
}


u32 app_recorder_setvol(u32 level)
{
    /* Use global volume now */
    uint32_t        vol = level;

    switch (vol)
    {
        case 9:
            vol++;
        case 8:
            vol++;
        case 7:
            vol++;
        case 6:
            vol++;
        case 5:
            vol++;
        case 4:
        case 3:
            vol++;
        case 2:
        case 1:
        case 0:
            break;

        default:
            break;
        }

    audio_set_master_volume(vol);

#ifdef CONFIG_AUDIO_TTS
    return app_fp_record_set_vol(level);
#else
    return vol;
#endif
}


u32 app_recorder_getplaytime()
{
    struct recorder_service * recs = g_recorder_srv;

    if ((recs->ahandle) && (recs->state != RECD_IDLE))
        return audio_get_timestamp(recs->ahandle);

    else 
        return 0;


}


u32 app_recorder_get_duration()
{
    struct recorder_service * recs = g_recorder_srv;
    return recs->file.duration;
}


uint32_t app_recorder_get_play_status()
{
    struct recorder_service * recs = g_recorder_srv;

    if (recs->state == RECD_PLAY)
        return 1;

    else 
        return 0;

}


static recorder_play_status_update_func_callback fun_callback;
static void *   func_callback_p_param;


void app_recorder_status_set_func_callback(recorder_play_status_update_func_callback fun,void * p_param)
{
    fun_callback        = fun;
    func_callback_p_param = p_param;

}







void app_recorder_service_notify_cb(uint32_t event)
{

    if (fun_callback)
        fun_callback(func_callback_p_param,event);
}









#if 1
#define AUDIO_PLAYER_MP3_ID3V1_USE
#define AUDIO_PLAYER_MP3_ID3V1_USE

#ifdef AUDIO_PLAYER_MP3_ID3V1_USE
typedef struct __AUDIO_PLAER_MP3_ID3V1_H__ {
    char            Header[3];                      /*标签头必须是"TAG"否则认为没有标签*/
    char            Title[30];                      /*标题*/
    char            Artist[30];                     /*作者*/
    char            Album[30];                      /*专集*/
    char            Year[4];                        /*出品年代*/
    char            Comment[30];                    /*备注*/
    char            Genre;                          /*类型，流派*/
} audio_mp3_id3v1_t;

#endif

#ifdef AUDIO_PLAYER_MP3_ID3V1_USE
typedef struct __AUDIO_PLAER_MP3_ID3V2_H__ {
    char            Header[3];                      /*必须为"ID3"否则认为标签不存在*/
    char            Ver;                            /*版本号ID3V2.3就记录3*/
    char            Revision;                       /*副版本号此版本记录为0*/
    char            Flag;                           /*存放标志的字节，这个版本只定义了三位，稍后详细解说*/
    char            Size[4];                        /*标签大小，包括标签头的10个字节和所有的标签帧的大小*/
} audio_mp3_id3v2_t;

typedef struct __MP3_ID3V2_FRAME_HEAD_H__ {
    char            FrameID[4];                     /*用四个字符标识一个帧，说明其内容，稍后有常用的标识对照表*/
    char            Size[4];                        /*帧内容的大小，不包括帧头，不得小于1*/
    char            Flags[2];                       /*存放标志，只定义了6位，稍后详细解说*/
} mp3_id3v2_frame_head_t;

#endif





#define AUDPLY_IS_MP3_HEAD(head) (!( (((head & 0xfff00000) != 0xfff00000) && ((head & 0xfff80000) != 0xffe00000) ) || \
        (((head>>17)&3)== 0) ||                                         \
        ( ((head>>12)&0xf) == 0xf) || ( ((head>>12)&0xf) == 0x0) ||     \
        ( ((head>>10)&0x3) == 0x3 ) ||                                  \
        ( ((head)& 0x3) == 0x2 )))


#define MP3_SKIP_ID3_FILE_LENGTH 4*1024//(512)


#include "mp3_mpeg.h"


int32_t fp_app_audio_get_mp3_duration(char * p_file_name,uint32_t * p_duration)
{
    FILE *          p_file;
    size_t          read_len;
    audio_mp3_id3v2_t mp3_id3v2;
    uint32_t        total_size_mp3_id3v2;
    uint32_t        duration;
    int32_t         ret;

    ret                 = 0;
    duration            = 0;

    p_file              = fopen(p_file_name,"rb");

    if (p_file == NULL) {
        __inf("p_file==NULL\r\n");
        return - 1;
        }

    read_len            = fread(&mp3_id3v2,1,sizeof(mp3_id3v2),p_file);

    if (read_len != sizeof(mp3_id3v2)) {
        ret                 = -1;
        goto app_audio_get_mp3_duration_failed;
        }

    if ((mp3_id3v2.Header[0] != 'I') || (mp3_id3v2.Header[1] != 'D') || (mp3_id3v2.Header[2] != '3')) {
        __inf("mp3_id3v2.Header ERR,:%d,%d,%d\r\n",mp3_id3v2.Header[0],mp3_id3v2.Header[1],mp3_id3v2.Header[2]);
        fseek(p_file,0,0);
        total_size_mp3_id3v2 = 0;
        }
    else {
        total_size_mp3_id3v2 =
             ((mp3_id3v2.Size[0] &0x7f) << 21) | ((mp3_id3v2.Size[1] &0x7f) << 14) | ((mp3_id3v2.Size[2] &0x7f) << 7) | ((mp3_id3v2.Size[3] &0x7f) << 0);

        fseek(p_file,total_size_mp3_id3v2,0);
        }

#if 0
    {
        uint8_t         version;
        uint32_t        bitrate;
        uint32_t        frame_len = 0;
        uint32_t        frame_start,read_len,file_offset;
        int             all_frame_size;
        unsigned char * temp_frame_buff,*p_tmp;
        unsigned char * str = NULL;

        /* ID3 header already skiped*/
        temp_frame_buff     = aic_zalloc(MP3_SKIP_ID3_FILE_LENGTH);

        if (temp_frame_buff == NULL) {

            __inf("[MP3]Buffer allocated failed");
            return - 1;
            }

        file_offset         = ftell(p_file);

        do {
            read_len            = fread(temp_frame_buff,1,MP3_SKIP_ID3_FILE_LENGTH,p_file);
            frame_len           = srv_mp3_search_mp3_header(temp_frame_buff,read_len,&frame_start,&version,&bitrate);

            if (frame_len != 0) {
#if 1

                //__inf("[MP3]find the next frame %d", frame_len);
                switch (version)
                {
                    case 0:
                        {
                            /* MPEG2.5 */
                            str                 = temp_frame_buff + frame_start + 4 + 17 + 9;
                            break;
                            }

                    case 1:
                        {
                            /* Reserved */
                            __inf("Wronly version happen");
                            ret                 = -1;
                            break;
                            }

                    case 2:
                        {
                            /* MPEG 2 */
                            str                 = temp_frame_buff + frame_start + 4 + 17 + 9;
                            break;
                            }

                    case 3:
                        {
                            /* MPEG 1 */
                            str                 = temp_frame_buff + frame_start + 4 + 32 + 17;
                            break;
                            }

                    default:
                        {
                            ret                 = -1;
                            __inf("Wronly version happen 2 %d",version);
                            break;
                            }
                    }

                if (strncmp((const char *) str,"Xing",4) == 0) {
                    all_frame_size      = (str[8] << 24);
                    all_frame_size      += (str[9] << 16);
                    all_frame_size      += (str[10] << 8);
                    all_frame_size      += (str[11]);

                    duration            = all_frame_size * 8 / bitrate;

                    __inf("Xing duration = %d,all_frame_size:%d,bitrate:%d\r\n",duration,all_frame_size,bitrate);

                    break;
                    }


                str                 = temp_frame_buff + frame_start + 4 + 32;
                {
                    uint32_t        temp_mpeg_header = 0;

                    p_tmp               = temp_frame_buff + frame_start;
                    temp_mpeg_header    = *p_tmp;
                    temp_mpeg_header    = temp_mpeg_header << 8 | * (p_tmp + 1);
                    temp_mpeg_header    = temp_mpeg_header << 8 | * (p_tmp + 2);
                    temp_mpeg_header    = temp_mpeg_header << 8 | * (p_tmp + 3);

                    if (0 == (temp_mpeg_header & 0x0080))
                        str = str + 2;
                    }

                if (strncmp((const char *) str,"VBRI",4) == 0) {
                    all_frame_size      = (str[10] << 24);
                    all_frame_size      += (str[11] << 16);
                    all_frame_size      += (str[12] << 8);
                    all_frame_size      += (str[13]);

                    duration            = all_frame_size * 8 / bitrate;

                    __inf("VBRI duration = %d,all_frame_size:%d,bitrate:%d\r\n",duration,all_frame_size,bitrate);


                    break;

                    }

                if (ret == 0) {
                    fseek(p_file,0,2);
                    all_frame_size      = ftell(p_file) -file_offset - frame_start - 128;

#if 0

                    if (all_frame_size > 0)
                        frame_number = all_frame_size / frame_len;
                    else 
                        frame_number = 0;

                    duration            = frame_number * 26 / 1000;
#endif

                    duration            = all_frame_size * 8 / bitrate;
                    __inf("duration = %d,all_frame_size:%d,bitrate:%d\r\n",duration,all_frame_size,bitrate);
                    }
                else {
                    duration            = 0;
                    }

                __inf("all mp3 time = %d\r\n",duration);
                break;

#else

                fseek(p_file,0,2);
                all_frame_size      = ftell(p_file) -file_offset - frame_start - 128;

                if (all_frame_size > 0)
                    frame_number = all_frame_size / frame_len;
                else 
                    frame_number = 0;

                duration            = frame_number * 26 / 1000;

                __inf("duration = %d,all_frame_size:%d,bitrate:%d\r\n",duration,all_frame_size,bitrate);

                break;
#endif
                }
            else {
                {
                    file_offset         = ftell(p_file);

                    if (file_offset > 1024 * 5) {
                        duration            = 0;
                        ret                 = -1;
                        __inf("all mp3 time file_offset:%d\r\n",file_offset);
                        break;
                        }
                    }

                while (read_len > 0)
                    ;


                aic_free(temp_frame_buff);
                }

#else

            {
                unsigned char * temp_frame_buff;
                MPEGAudioFrameInfo info;
                MPEGAudioRet    audio_ret;

                /* ID3 header already skiped*/
                temp_frame_buff     = aic_zalloc(MP3_SKIP_ID3_FILE_LENGTH);

                if (temp_frame_buff == NULL) {

                    __inf("[MP3]Buffer allocated failed");
                    return - 1;
                    }

                read_len            = fread(temp_frame_buff,1,MP3_SKIP_ID3_FILE_LENGTH,p_file);
                audio_ret           = findMpegAudioFramePos(temp_frame_buff,MP3_SKIP_ID3_FILE_LENGTH,&info,true);

                if (audio_ret.mErrCode == MPEG_AUDIO_OK) {
                    switch (info.mBitrateType)
                    {
                        case 0:
                            {
                                uint32_t        all_frame_size;

                                fseek(p_file,0,2);
                                all_frame_size      = ftell(p_file) -audio_ret.mNextPos - total_size_mp3_id3v2 - 128;
                                duration            = all_frame_size * 8 / (info.mBitrate * 1000);
                                __inf("info.mBitrateType 0,all_frame_size:%d,%d,%d",all_frame_size,info.mBitrate,
                                    info.mSamplerate);
                                }
                            break;

                        case 1:
                        case 2:
                            {
                                duration            = info.mTotalFrames * info.mSamplesPerFrame / info.mSamplerate;

                                __inf("info.mBitrateType 1,all_frame_size:%d,%d,%d",info.mTotalFrames,
                                    info.mSamplesPerFrame,info.mSamplerate);

                                //__inf("duration:%d",duration);
                                }
                            break;

                        case 3:
                            {
                                duration            = info.mTotalFrames * info.mSamplesPerFrame / info.mSamplerate;

                                __inf("info.mBitrateType 2,all_frame_size:%d,%d",info.mTotalFrames,
                                    info.mSamplesPerFrame,info.mSamplerate);
                                }
                            break;

                        default:
                            duration = 0;
                        }
                    }
                else {

                    __inf("info.mBitrateType audio_ret.mErrCode,%d",audio_ret.mErrCode);
                    duration            = 0;
                    ret                 = -1;
                    }

                aic_free(temp_frame_buff);
                }
#endif




app_audio_get_mp3_duration_failed:

            if (p_duration)
                *p_duration = duration;

            fclose(p_file);
            return ret;
            }

#else

#endif



        typedef struct {
            uint8_t         riff[4];                // "RIFF"标识符
            uint32_t        fileSize;               // 文件总大小（不包括riff和fileSize字段本身的大小）
            uint8_t         wave[4];                // "WAVE"标识符
            uint8_t         fmt[4];                 // "fmt "标识符
            uint32_t        fmtSize;                // fmt块大小
            uint16_t        audioFormat;            // 音频格式代码（例如，PCM为1）
            uint16_t        numChannels;            // 声道数
            uint32_t        sampleRate;             // 采样率
            uint32_t        byteRate;               // 每秒的字节数
            uint16_t        blockAlign;             // 数据块对齐单位（每个采样帧的字节数）
            uint16_t        bitsPerSample;          // 每个采样的位数
            uint8_t         data[4];                // "data"标识符
            uint32_t        dataSize;               // 音频数据大小
            } wav_header_t;

        int32_t fp_app_audio_get_wav_duration(char * p_file_name,uint32_t * p_duration) {
            FILE *          p_file;
            size_t          read_len;
            wav_header_t    wav_header;
            uint32_t        duration;
            int32_t         ret;

            duration            = 0;
            ret                 = 0;
            p_file              = fopen(p_file_name,"rb");

            if (p_file == NULL) {
                __inf("p_file==NULL\r\n");
                return - 1;
                }

            read_len            = fread(&wav_header,1,sizeof(wav_header),p_file);

            if (read_len != sizeof(wav_header)) {
                ret                 = -1;
                }
            else {
                if (wav_header.byteRate)
                    duration = wav_header.dataSize / wav_header.byteRate;
                }

            fclose(p_file);

            if (p_duration)
                *p_duration = duration;

            return ret;
            }


        typedef struct {
            uint8_t         head[6];
            uint8_t         frame_head;

            } amr_header_t;

        int32_t fp_app_audio_get_amr_duration(char * p_file_name,uint32_t * p_duration) {
            FILE *          p_file;
            uint32_t        all_frame_size,all_frame_count,duration;
            int32_t         ret;

            duration            = 0;
            ret                 = 0;
            p_file              = fopen(p_file_name,"rb");

            if (p_file == NULL) {
                __inf("p_file==NULL\r\n");
                return - 1;
                }


            fseek(p_file,0,2);
            all_frame_size      = ftell(p_file) - 6;
            all_frame_count     = all_frame_size / 18;

            if (all_frame_size <= sizeof(amr_header_t))
                ret = -1;

            fclose(p_file);

            if (p_duration)
                *p_duration = all_frame_count * 20 / 1000;

            return ret;
            }




        int32_t fp_app_audio_get_file_duration(char * p_file_name,uint32_t * p_duration) {
            int32_t         ret;
            char *          detect;

            if (p_file_name == NULL) {
                __inf("(p_file_name==NULL)");
                return 0;
                }

            //__inf("p_file_name:%s",p_file_name);
            detect              = strrchr(p_file_name,'.');

            if (NULL == detect) {
                return 0;
                }

            detect++;

            if (strcmp(detect,"mp3") == 0 || strcmp(detect,"MP3") == 0) {
#if 1
                ret                 = fp_app_audio_get_mp3_duration(p_file_name,p_duration);

                //ret = 0;
                //*p_duration = app_audio_get_mp3_duration1(p_file_name);
                if (p_duration) {
                    if ((ret) || (*p_duration == 0)) {
                        *p_duration         = app_audio_get_mp3_duration(p_file_name);
                        }
                    }

                return 0;

#else

                return fp_app_audio_get_mp3_duration(p_file_name,p_duration);

#endif
                }
            else if (strcmp(detect,"wav") == 0 || strcmp(detect,"WAV") == 0) {
                return fp_app_audio_get_wav_duration(p_file_name,p_duration);
                }
            else if (strcmp(detect,"amr") == 0 || strcmp(detect,"AMR") == 0) {
                // TODO: NB and WB
                return fp_app_audio_get_amr_duration(p_file_name,p_duration);
                }
            else {
                return 0;
                }



            return 0;
            }


