/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the app id, which will be replaced by app name in future.
 */

#ifndef __APP_ID_H__
#define __APP_ID_H__

#ifdef __cplusplus
extern "C" {
#endif

enum app_id {
    APP_ID_MAIN = 0,
    APP_ID_FILE_BROWSER,
    APP_ID_CAMERA,
    APP_ID_PHOTO,
    APP_ID_MUSIC_PLAYER,
    APP_ID_RECORDER,
    APP_ID_GPS,
    APP_ID_FOTA,

    APP_ID_MAX = 32
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __APP_MGR_H__ */
