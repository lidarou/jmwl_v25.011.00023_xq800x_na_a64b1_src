/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_audio_mp3_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>
#include "aic_list.h"
#include "app_audio.h"
#ifdef CONFIG_APP
#include "app_msg.h"
#endif
#include "fs_api.h"

/*********************
 *      DEFINES
 *********************/
#define APP_AUDIO_MP3_SAMPLE_TASK_STK_SIZE       (4096u)
#define APP_AUDIO_MP3_SAMPLE_TASK_COM_PRIO       (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

LIST_HEAD(mp3_files_list);

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int app_audio_play_mp3_file_cb(void *p_param, uint32_t size)
{
    app_event_audio_t *p_event = NULL;
    if(NULL == p_param) {
        APP_LOGE("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }
    p_event = (app_event_audio_t *)(p_param);
    APP_LOGI("[%s]event id = %d.\r\n", __func__, p_event->event_id);
    return 0;
}
__APP_CALLBACK_FUNCTION__(app_audio_play_mp3_file_cb);

static int32_t get_mp3_file_list_test(char *p_full_file_name,void *mp3_list)
{
    mp3_info_t *p_mp3_info = NULL;

    if(NULL == p_full_file_name) {
        app_loge("[%s],p_full_file_name error.\n", __func__);
        return -1;
    }

    p_mp3_info = aic_zalloc(sizeof(mp3_info_t));
    if (NULL == p_mp3_info) {
        app_loge("[%s],p_mp3_info malloc failed.\n", __func__);
        return -1;
    }

    aos_memcpy(p_mp3_info->file_name, p_full_file_name, strlen(p_full_file_name));
    char *detect = strrchr(p_full_file_name, '.');
    if (detect) {
        detect++;
        if (strcmp(detect, "mp3") == 0 || strcmp(detect, "MP3") == 0) {
            p_mp3_info->format = ASTREAM_FORMAT_MP3;
        } else {
            p_mp3_info->format = ASTREAM_FORMAT_INVALID;
        }
    } else {
        p_mp3_info->format = ASTREAM_FORMAT_INVALID;
    }


    list_add(&p_mp3_info->node, &mp3_files_list);

    return 0;
}


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static void AppAudioMp3SampleEntry(void *thread_input)
{
    char *p_file_path = "/D/Music/2";
    mp3_info_t *p_mp3_info = NULL;
    struct list_head *pos, *next;

    while (1) {
        app_logi("app_auido_mp3 sample entry point!\n");

        /* 10 seconds */
        aos_sleep(10000);
#ifdef CONFIG_APP
        app_audio_mp3_scan_dir_with_ext_filter(p_file_path, get_mp3_file_list_test, (void *)&mp3_files_list);

        list_for_each_safe(pos, next, &mp3_files_list) {
            p_mp3_info = list_entry(pos, mp3_info_t, node);
            if(NULL != p_mp3_info) {
                APP_LOGI("[%s]file_name = %s.\r\n", __func__, p_mp3_info->file_name);
                app_audio_play_file_stop(__APP_CALLBACK_NAME__(app_audio_play_mp3_file_cb));
                app_audio_play_file_start(p_mp3_info->file_name, __APP_CALLBACK_NAME__(app_audio_play_mp3_file_cb));
            }
        }
#endif
    }
}


int app_audio_mp3_init_sample(void)
{
    aos_create_thread(
        "app_audio_mp3_sample_task",
        AppAudioMp3SampleEntry,
        NULL,
        APP_AUDIO_MP3_SAMPLE_TASK_STK_SIZE,
        0,
        NULL,
        APP_AUDIO_MP3_SAMPLE_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}


AIC_NORMAL_APP_P2_INIT_EXPORT(app_audio_mp3_init_sample);


