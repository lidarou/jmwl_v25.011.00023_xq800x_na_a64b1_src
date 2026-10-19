

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define the basic struct of playlist.
 */
#ifndef __FP_PLAYLIST_H__
#define __FP_PLAYLIST_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "aic_type.h"
#include "audio_system.h"
#include "aic_list.h"


    enum media_play_mode {
        PLAY_MODE_SINGLE_LOOP,
        PLAY_MODE_SEQ_LOOP,
        PLAY_MODE_RAND,
        PLAY_MODE_SINGLE,
        };


    struct media_track {
        struct list_head list;
        u32             id;
        char *          url;
        char *          title;
        bool            title_unknown;
        char *          artist;
        bool            artist_unknown;
        char *          album;
        bool            album_unknown;
        char *          cover;
        u32             cover_size;
        u32             year;                       /* no use now */
        enum astream_format format;

        u32             duration;                   /*in seconds*/
        /* track info loaded flag */
        bool            load_flag;
        };


    struct playlist {
        struct list_head tracks;
        enum media_play_mode playmode;
        u32             track_count;
        };


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /*__FP_PLAYLIST_H__*/

