

/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_tts.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <alog_api.h>
#include "audio_system.h"

/*********************
 *      DEFINES
 *********************/
#define VOLTE_PERIOD_SIZE       160
#define VOLTE_PERIOD_COUNT      4
#define VOLTE_CHN               1
#define VOLTE_SRATE             16000

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int play_tts(char *tts)
{
    ACLIENT handle;
    int ret;

    if (tts == NULL) {
        app_logw("please input the text you want to play\n");
        return 0;
    }

    handle = audio_open(ASTREAM_TYPE_TTS, APLAY_TYPE_TTS,
                        AUDIO_INPUT_FLAG_DIRECT, VOLTE_SRATE, 0,
                        ASTREAM_FORMAT_PCM,
                        ASTREAM_CHN_MONO,
                        ASTREAM_DIR_PLAY,
                        ASTREAM_SOURCE_DEFAULT);

    ret = audio_play_tts(handle, tts, strlen(tts), NULL, 0);

    return 0;
}

