

#ifndef __MUSICPLAYER_FP_PLAY_H__
#define __MUSICPLAYER_FP_PLAY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "musicplayer_fp_conf.h"
#include "fp_playservice.h"
#include "lvgl.h"


    typedef int32_t (*play_status_update_func_callback)(void *p_param,uint32_t event); // typedef srv_func_callback

    void app_musicplayer_status_set_func_callback(play_status_update_func_callback fun,void * p_param);


    void app_musicplayer_play(uint32_t id);
    void app_musicplayer_resume(void);
    void app_musicplayer_pause(void);
    void app_musicplayer_album_next(uint8_t next);

    uint32_t app_musicplayer_play_time();
    uint32_t app_musicplayer_play_duration();
    uint32_t app_musicplayer_get_play_status();
    uint8_t * app_musicplayer_get_cur_title();
    void app_musicplayer_stop(void);
    uint32_t app_musicplayer_get_vol();
    void app_musicplayer_play_pause();
    uint32_t app_musicplayer_set_cur_music_by_path(uint8_t * path);
    uint8_t app_musicplayer_get_playmode();
    uint32_t app_musicplayer_set_vol(uint32_t vol);
    void app_music_player_init();
    void app_musicplayer_set_playmode(uint8_t playmode);
    uint8_t app_musicplayer_get_all_duration();
    void app_musicplayer_stop_action(void);
    uint32_t app_musicplayer_reset_pre_playtime();
    void app_musicplayer_play_action(uint32_t id);
    void app_musicplayer_pause_action(void);
    void app_musicplayer_resume_action(void);
    void app_musicplayer_play_pause_action();
    void app_musicplayer_album_next_action(uint8_t next);
    void ps_data_msg_send_music_pending_restore(uint32_t action);
    uint32_t app_musicplayer_get_track_id();
    uint32_t app_musicplayer_set_track_id(uint32_t id);
    uint32_t app_musicplayer_set_pre_playtime(uint32_t pre_playtime);
    void app_musicplayer_set_playmode_action(uint8_t playmode);
    uint32_t app_musicplayer_set_cur_music_by_path_action(uint8_t * path);

#ifdef __cplusplus
} /* extern "C" */


#endif

#endif /*__MUSICPLAYER_FP_PLAY_H__*/

