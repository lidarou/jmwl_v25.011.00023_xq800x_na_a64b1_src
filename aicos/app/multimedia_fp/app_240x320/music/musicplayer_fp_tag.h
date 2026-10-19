

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare the audio tag decoder APIs.
 */
#ifndef __MUSICPLAYER_FP_TAG_H__
#define __MUSICPLAYER_FP_TAG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "fp_playlist.h"

    u32 app_musicplayer_decoder_tag(struct media_track * track);
    const char * app_musicplayer_get_title(u32 track_id);
    const char * app_musicplayer_get_artist(u32 track_id);
    const char * app_musicplayer_get_genre(u32 track_id);
    u32 app_musicplayer_get_track_length(u32 track_id);
    u32 app_musicplayer_free_track_info(u32 track_id);
    uint32_t app_music_list_is_empty();

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /*__MUSICPLAYER_FP_TAG_H__*/

