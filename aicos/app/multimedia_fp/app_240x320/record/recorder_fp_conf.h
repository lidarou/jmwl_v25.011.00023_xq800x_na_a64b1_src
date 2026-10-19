

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the basic recorder app configs.
 */
#ifndef __RECORDER_FP_CONF_H__
#define __RECORDER_FP_CONF_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*******************************************
     *              INCLUDES
     ******************************************/
#include "lvgl.h"

    /*******************************************
     *              DEFINES
     ******************************************/
#define     RECORDER_HANDLE_SIZE    15
#define     RECORDER_URL_MAX_LENGTH 128
#define     APP_RECORDER_FILE_DIR   "D:/sound"
#define     APP_RECORDER_FILE_FORMAT_EXT (".amr")//(".wav")
#define     APP_RECORDER_DEFAULT_FORMAT ASTREAM_FORMAT_AMR_WB//ASTREAM_FORMAT_WAV
#define     APP_RECORDER_SAMPLE_RATE 16000

#ifdef __cplusplus
} /* extern "C" */


#endif

#endif /*__RECORDER_FP_CONF_H__*/

