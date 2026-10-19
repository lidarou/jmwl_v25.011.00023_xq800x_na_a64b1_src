

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare the playlist operation APIs.
 */
#ifndef __MUSICPLAYER_FP_PLAYLIST_H__
#define __MUSICPLAYER_FP_PLAYLIST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "fp_playlist.h"

    void app_musicplayer_playlist_update(void);
    void app_musicplayer_playlist_release(struct playlist * playlist);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /*__MUSICPLAYER_FP_PLAYLIST_H__*/

