/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_audio_mp3.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_osa_ex.h>
#include <alog_api.h>
#include "app_audio.h"
#include "minimp3/minimp3_ex.h"
#include "fs_api.h"
#include "stdio.h"

/*********************
 *      DEFINES
 *********************/
#define UNKNOWN_DURATION_DUMMY_VAL  (180)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *   STATIC FUNCTIONS
 **********************/

static size_t mp_mp3d_read_cb(void *buf, size_t size, void *user_data)
{
    return fread(buf, 1, size, user_data);
}

static int mp_mp3d_seek_cb(uint64_t position, void *user_data)
{
    return fseek(user_data, position, SEEK_SET);
}

static void mp_decoder_mp3_duration(mp3dec_t *dec, uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, const char *url)
{
    mp3dec_io_t io;
    FILE *file = fopen(url, "rb");
    if (!file)
        return;

    io.read = mp_mp3d_read_cb;
    io.read_data = file;
    io.seek = mp_mp3d_seek_cb;
    io.seek_data = file;

    mp3dec_load_cb(dec, &io, (uint8_t *)buf, buf_size, info, NULL, NULL);
    fclose(file);
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
uint32_t app_audio_get_mp3_duration(char *p_file_name)
{
    mp3dec_t *mp3d;
    mp3dec_file_info_t info;
    u32  mp3_duration = 0;

    if(NULL == p_file_name) {
        app_loge("[%s],p_url is null.\r\n", __func__);
        return 0xFFFF;
    }

    app_loge("[%s],p_url is %s.\r\n", __func__, p_file_name);
    /* judge this file is exsit */
    if(fs_pathisexist(p_file_name)) {
        app_loge("[%s],p_url is not exist.\r\n", __func__);
        return 0xFFFF;
    }

    // TODO :judge this file is mp3?
    memset(&info, 0, sizeof(info));
    mp3d = aic_zalloc(sizeof(mp3dec_t));
    if (!mp3d) {
        app_loge("%s: malloc failed\n", __func__);
        return 0xFFFF;
    } else {
        uint8_t *buf;
        buf = aic_malloc(MINIMP3_BUF_SIZE);
        if (buf) {
            mp_decoder_mp3_duration(mp3d, buf, MINIMP3_BUF_SIZE, &info, p_file_name);
            aic_free(buf);
        }

        app_logi("%s: p_url %s, samples: %d, chn: %d, hz: %d\n", __func__, p_file_name, info.samples,
            info.channels, info.hz);
        if (info.samples && info.channels && info.hz) {
            mp3_duration = (info.samples / info.channels) / info.hz;
        } else {
            mp3_duration = UNKNOWN_DURATION_DUMMY_VAL;
        }
        aic_free(mp3d);
    }
    app_logi("[%s]: duration: %d\n", __func__, mp3_duration);
    return mp3_duration;
}

void app_audio_mp3_scan_dir_with_ext_filter(char *p_dir_path,            scan_result_cb _result_callback, void *p_usr_data)
{
    char *url = NULL;

    if (NULL == p_dir_path) {
        app_loge("[%s],p_dir_path is null.\n", __func__);
        return;
    }

    if (_result_callback == NULL) {
        app_loge("[%s],_result_callback is null.\n", __func__);
        return;
    }

    url = aic_zalloc(APP_MUSICPLAYER_FILEPATH_MAX_LENGTH);
    if (url == NULL) {
        app_loge("[%s],zalloc failed.\n", __func__);
        return;
    }

    DIR *dir = opendir(p_dir_path);
    if (dir == NULL) {
        app_loge("[%s],Open [%s] directory failed", __func__, p_dir_path);
        aic_free(url);
        return;
    }

    struct dirent *dinfo = NULL;
    while (NULL != (dinfo = readdir(dir))) {

        /*file name is too long */
        if ((strlen(dinfo->d_name) + strlen(p_dir_path)) > APP_MUSICPLAYER_FILEPATH_MAX_LENGTH) {
            app_loge("[%s],The file name is too long, invalid path", __func__);
            continue;
        }

        /* is hidden dir */
        if (dinfo->d_name[0] == '.') {
            continue;
        }

        /* is dir file */
        if (dinfo->d_type == DT_DIR) {
            continue;
        } else {
            memset(url, 0, APP_MUSICPLAYER_FILEPATH_MAX_LENGTH);
            sprintf(url, "%s/%s", p_dir_path, dinfo->d_name);
            app_logi("[%s], file path and name is %s", __func__, url);

            char *detect = strrchr(dinfo->d_name, '.');
            if (NULL == detect) {
                continue;
            }
            detect++;
            if ((strcmp(detect, "mp3") == 0) || (strcmp(detect, "MP3") == 0)) {
                app_logi("[%s], save to list", __func__);
                _result_callback(url, p_usr_data);
            }
        }
    }
    aic_free(url);
    closedir(dir);
}
