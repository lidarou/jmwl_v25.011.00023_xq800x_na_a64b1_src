/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_audio.h
 *
 */

#ifndef __APP_AUDIO_H__
#define __APP_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <aic_type.h>
#include "aic_srv_bus.h"
#include "audio_system.h"
#include "aic_list.h"

/*********************
 *      DEFINES
 *********************/
#define APP_MUSICPLAYER_FILEPATH_MAX_LENGTH (256)

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    APP_EVENT_AUDIO_PLAY_BUF_START,  //0
    APP_EVENT_AUDIO_PLAY_BUF_FAIL,
    APP_EVENT_AUDIO_PLAY_BUF_SINGLE_FINISH,
    APP_EVENT_AUDIO_PLAY_BUF_FINISH,
    APP_EVENT_AUDIO_PLAY_BUF_STOP_SUCC,  //4
    APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL,

    APP_EVENT_AUDIO_PLAY_FILE_START,  //6
    APP_EVENT_AUDIO_PLAY_FILE_FAIL,   //7
    APP_EVENT_AUDIO_PLAY_FILE_FINISH, //8
    APP_EVENT_AUDIO_PLAY_FILE_STOP_SUCC,
    APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL, //10

    APP_EVENT_AUDIO_PLAY_VOICE_START,
    APP_EVENT_AUDIO_PLAY_VOICE_FAIL,
    APP_EVENT_AUDIO_PLAY_VOICE_STOP_SUCC,
    APP_EVENT_AUDIO_PLAY_VOICE_STOP_FAIL,

    APP_EVENT_AUDIO_RECORD_START,      //15
    APP_EVENT_AUDIO_RECORD_FINISH,     //16
    APP_EVENT_AUDIO_RECORD_FAIL,       //17
    APP_EVENT_AUDIO_RECORD_STOP_SUC,   //18
    APP_EVENT_AUDIO_RECORD_STOP_FAIL,  //19

    APP_EVENT_AUDIO_MAX
}app_event_audio_e;

typedef struct
{
    app_event_audio_e event_id;
}app_event_audio_t;

typedef struct
{
    u32 flag;
    u32 sample_rate;
    u32 bit_rate;
    enum astream_format format;
    enum astream_channel channel;
}app_audio_play_buf_param_t;

typedef enum
{
    APP_AUDIO_PLAY_FILE_FORMAT_AMR_WB,
    APP_AUDIO_PLAY_FILE_FORMAT_AMR_NB,
    APP_AUDIO_PLAY_FILE_FORMAT_MP3,
    APP_AUDIO_PLAY_FILE_FORMAT_WAV,
    APP_AUDIO_PLAY_FILE_FORMAT_PCM,
    APP_AUDIO_PLAY_FILE_FORMAT_MAX,
}app_audio_play_file_format_e;

typedef enum {
    RECORD_FILE_TYPE_AMR_WB,
    RECORD_FILE_TYPE_AMR_NB,
    RECORD_FILE_TYPE_WAV,
    RECORD_FILE_TYPE_PCM,
    RECORD_FILE_TYPE_MAX,
}recorder_file_type_e;

typedef struct {
    char *file_path;
    recorder_file_type_e type;
    uint32_t rate;
}recorder_param_t;

typedef struct mp3_info {
    struct list_head node;
    char file_name[APP_MUSICPLAYER_FILEPATH_MAX_LENGTH]; /*mp3 path,eg:D:/Music/111.mp3*/
    enum astream_format format;
    u32 duration; /*in seconds*/
}mp3_info_t;

typedef int32_t (*scan_result_cb)(char *p_file_name, void *user_data);

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
* app_audio_play_buf_start
* @p_buf: [IN] the buffer data that will be played.
* @buf_len: [IN] the buffer data length.
* @duration: [IN] play time, unit: ms.
* @p_buf_para: [IN] play buffer parameter, such as sample rate, format, etc.
* @_buf_start_callback: [IN] play start callback, and it will receive the following event ids.
*                            APP_EVENT_AUDIO_PLAY_BUF_START
*                            APP_EVENT_AUDIO_PLAY_BUF_FAIL
*                            APP_EVENT_AUDIO_PLAY_BUF_SINGLE_FINISH
*                            APP_EVENT_AUDIO_PLAY_BUF_FINISH
*                            APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL
* Return: if success return 0, else return other value.
*/
int app_audio_play_buf_start(const char *p_buf, uint32_t buf_len, uint32_t duration, app_audio_play_buf_param_t *p_buf_para, srv_func_callback _buf_start_callback);

/**
* app_audio_play_buf_stop
* @_buf_stop_callback: [IN] play stop callback, and it will receive the following event ids.
*                            APP_EVENT_AUDIO_PLAY_BUF_STOP_SUCC
*                            APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL
* Return: if success return 0, else return other value.
*/
int app_audio_play_buf_stop(srv_func_callback _buf_stop_callback);

/**
* app_audio_play_file_start
* @p_file_name: [IN] the play file name, including whole path.
* @_file_start_callback: [IN] play start callback, and it will receive the following event ids.
*                             APP_EVENT_AUDIO_PLAY_FILE_START,
*                             APP_EVENT_AUDIO_PLAY_FILE_FAIL,
*                             APP_EVENT_AUDIO_PLAY_FILE_FINISH,
* Return: if success return 0, else return other value.
*/
int app_audio_play_file_start(char *p_file_name, srv_func_callback _file_start_callback);

/**
* app_audio_play_file_start
* @p_file_name: [IN] the play file name, including whole path.
* @format: [IN] the file format(this is a temporary action, such as user could set APP_AUDIO_PLAY_FILE_FORMAT_AMR_WB or APP_AUDIO_PLAY_FILE_FORMAT_AMR_NB for amr file).
* @_file_start_callback: [IN] play start callback, and it will receive the following event ids.
*                             APP_EVENT_AUDIO_PLAY_FILE_START,
*                             APP_EVENT_AUDIO_PLAY_FILE_FAIL,
*                             APP_EVENT_AUDIO_PLAY_FILE_FINISH,
* Return: if success return 0, else return other value.
*/
int app_audio_play_file_start_with_format(char *p_file_name, app_audio_play_file_format_e format, srv_func_callback _file_start_callback);

/**
* app_audio_play_file_stop
* @_file_stop_callback: [IN] play stop callback, and it will receive the following event ids.
*                             APP_EVENT_AUDIO_PLAY_FILE_STOP_SUCC,
*                             APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL,
* Return: if success return 0, else return other value.
*/
int app_audio_play_file_stop(srv_func_callback _file_stop_callback);

/**
* app_audio_record_start
* @filepath: [IN] record file save path,eg:D:/sound/111.wav,be sure folder D:/sound exist in device.
* @type: [IN] file format:wav or amr.
* @rate: record file rate:eg.16000.
* Return: record handle,if record_handle is 0,record start fail,record handle is not 0, record start success.
*/
int app_audio_record_start(recorder_param_t *p_rec_parm, srv_func_callback _rec_start_callback);
/**
* app_audio_record_stop
* @[IN] record_handle.
* Return: int,if return value is -1,stop fail,return value is 0, stop success.
*/
int app_audio_record_stop(  srv_func_callback _rec_stop_callback);

/**
* app_audio_set_master_volume
* @[IN] vol,value range is 0-15.
* Return: -1:fail,0:success.
*/
int app_audio_set_master_volume(uint8_t vol);

/**
* app_audio_get_master_volume
* @[OUT] p_vol,value range is 0-15.
* Return: -1:fail,0:success.
*/
int app_audio_get_master_volume(uint8_t *p_vol);

/**
* app_audio_get_mp3_duration
* @[IN] p_url,mp3 file path,eg,/D/Music/poweron.mp3.
* Return: mp3 duaration,uint:s.success:>0,fail:0xffff
*/
uint32_t app_audio_get_mp3_duration(char *p_file_name);

/**
 * app_audio_mp3_scan_dir_with_ext_filter - scan the dir with mp3 to find
 * all files
 * @path: [IN] dir path
 * @_result_callback: [IN] result callback
 * @usr_data: [IN] user data
 * Return: NA
 */
void app_audio_mp3_scan_dir_with_ext_filter(char *p_dir_path,            scan_result_cb _result_callback, void *p_usr_data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_XXX_H__ */

