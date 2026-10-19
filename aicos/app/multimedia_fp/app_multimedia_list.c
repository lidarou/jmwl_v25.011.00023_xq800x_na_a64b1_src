
#include "lvgl.h"

#include "alv_app_mgr.h"
#include "backlight.h"
#include "app_msg.h"
#include <stdio.h>
#include "app_multimedia_list.h"
#include "app_log.h"
#include "musicplayer_fp_play.h"
#include "tts_app_player.h"
#include "recorder_fp_srv.h"

#define __inf                   APP_LOGI

extern void ui_ScreenMultimediaMenu_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenMultimediaMenu_screen_defocus(lv_obj_t * win,void * p);
extern void ui_ScreenFileBrowser_win_defocus(lv_obj_t * win,void * param);
extern void ui_ScreenMuiscOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenMuiscSetup_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenMuiscList_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenRecordOption_screen_focus(lv_obj_t * win,void * p);
extern void ui_ScreenRecordList_win_defocus(lv_obj_t * win,void * param);

void multimedia_fp_app_active(lv_obj_t * app,void * p_param)
{
    lv_obj_t *      frist_win;

    if (p_param == NULL) {
        frist_win           = get_win_by_name(app,"menu");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }
    else {
        frist_win           = get_win_by_name(app,"music_play");
        alv_am_app_set_first_win(app,frist_win);
        return;
        }

}


void multimedia_fp_app_deactive(lv_obj_t * app,void * p_param)
{

}





void multimedia_menu_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenMultimediaMenu_screen_init(win,p_param);

}


void multimedia_menu_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMultimediaMenu_screen_uninit(win,p_param);

}


void multimedia_menu_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenMultimediaMenu_screen_focus(win,p_param);
}


void multimedia_menu_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMultimediaMenu_screen_defocus(win,p_param);
}




void multimedia_file_browser_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenFileBrowser_screen_active(win,p_param);

}


void multimedia_file_browser_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenFileBrowser_screen_deactive(win,p_param);

}


void multimedia_file_browser_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenFileBrowser_win_focus(win,p_param);
}


void multimedia_file_browser_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenFileBrowser_win_defocus(win,p_param);
}



void multimedia_music_play_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenMuiscPlay_screen_init(win,p_param);

}


void multimedia_music_play_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscPlay_screen_uninit(win,p_param);

}


void multimedia_music_play_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscPlay_screen_focus(win,p_param);
}


void multimedia_music_play_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscPlay_screen_defocus(win,p_param);
}


void multimedia_music_option_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenMuiscOption_screen_init(win,p_param);

}


void multimedia_music_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscOption_screen_uninit(win,p_param);

}


void multimedia_music_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscOption_screen_focus(win,p_param);
}


void multimedia_music_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void multimedia_music_setup_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenMuiscSetup_screen_init(win,p_param);

}


void multimedia_music_setup_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscSetup_screen_uninit(win,p_param);

}


void multimedia_music_setup_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscSetup_screen_focus(win,p_param);
}


void multimedia_music_setup_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void multimedia_music_list_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenMuiscList_screen_init(win,p_param);

}


void multimedia_music_list_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscList_screen_uninit(win,p_param);

}


void multimedia_music_list_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenMuiscList_screen_focus(win,p_param);
}


void multimedia_music_list_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}



void multimedia_record_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenRecord_screen_init(win,p_param);

}


void multimedia_record_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecord_screen_uninit(win,p_param);

}


void multimedia_record_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecord_screen_focus(win,p_param);
}


void multimedia_record_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}


void multimedia_record_option_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenRecordOption_screen_init(win,p_param);

}


void multimedia_record_option_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordOption_screen_uninit(win,p_param);

}


void multimedia_record_option_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordOption_screen_focus(win,p_param);
}


void multimedia_record_option_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    app_tts_stop();
}


void multimedia_record_list_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenRecordList_screen_active(win,p_param);

}


void multimedia_record_list_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordList_screen_deactive(win,p_param);

}


void multimedia_record_list_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordList_win_focus(win,p_param);
}


void multimedia_record_list_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordList_win_defocus(win,p_param);
}



void multimedia_record_play_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenRecordPlay_screen_init(win,p_param);

}


void multimedia_record_play_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordPlay_screen_uninit(win,p_param);

}


void multimedia_record_play_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordPlay_screen_focus(win,p_param);
}


void multimedia_record_play_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenRecordPlay_screen_defocus(win,p_param);
}




void multimedia_file_browser_play_fp_win_active(lv_obj_t * win,void * p_param)
{

    ui_ScreenFileBrowserPlay_screen_init(win,p_param);

}


void multimedia_file_browser_play_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenFileBrowserPlay_screen_uninit(win,p_param);

}


void multimedia_file_browser_play_fp_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenFileBrowserPlay_screen_focus(win,p_param);
}


void multimedia_file_browser_play_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenFileBrowserPlay_screen_defocus(win,p_param);
}




void app_multimedia_fp_init(void)
{

    /* Create APP for Idle */
    lv_obj_t *      app_multimedia = alv_am_app_create(lv_scr_act(),MULTIMEDIA_FP_APP_NAME);

    alv_am_app_set_active(app_multimedia,multimedia_fp_app_active);
    alv_am_app_set_deactive(app_multimedia,multimedia_fp_app_deactive);


    lv_obj_t *      win_menu = alv_am_win_create(app_multimedia,"menu");

    alv_am_win_set_active(win_menu,multimedia_menu_fp_win_active);
    alv_am_win_set_deactive(win_menu,multimedia_menu_fp_win_deacactive);
    alv_am_win_set_focus(win_menu,multimedia_menu_fp_win_focus);
    alv_am_win_set_defocus(win_menu,multimedia_menu_fp_win_defocus);
    alv_am_app_set_first_win(app_multimedia,win_menu);


    lv_obj_t *      win_file_browser = alv_am_win_create(app_multimedia,"file_browser");

    alv_am_win_set_active(win_file_browser,multimedia_file_browser_fp_win_active);
    alv_am_win_set_deactive(win_file_browser,multimedia_file_browser_fp_win_deacactive);
    alv_am_win_set_focus(win_file_browser,multimedia_file_browser_fp_win_focus);
    alv_am_win_set_defocus(win_file_browser,multimedia_file_browser_fp_win_defocus);



    lv_obj_t *      win_music_play = alv_am_win_create(app_multimedia,"music_play");

    alv_am_win_set_active(win_music_play,multimedia_music_play_fp_win_active);
    alv_am_win_set_deactive(win_music_play,multimedia_music_play_fp_win_deacactive);
    alv_am_win_set_focus(win_music_play,multimedia_music_play_fp_win_focus);
    alv_am_win_set_defocus(win_music_play,multimedia_music_play_fp_win_defocus);


    lv_obj_t *      win_music_option = alv_am_win_create(app_multimedia,"music_option");

    alv_am_win_set_active(win_music_option,multimedia_music_option_fp_win_active);
    alv_am_win_set_deactive(win_music_option,multimedia_music_option_fp_win_deacactive);
    alv_am_win_set_focus(win_music_option,multimedia_music_option_fp_win_focus);
    alv_am_win_set_defocus(win_music_option,multimedia_music_option_fp_win_defocus);


    lv_obj_t *      win_music_setup = alv_am_win_create(app_multimedia,"music_setup");

    alv_am_win_set_active(win_music_setup,multimedia_music_setup_fp_win_active);
    alv_am_win_set_deactive(win_music_setup,multimedia_music_setup_fp_win_deacactive);
    alv_am_win_set_focus(win_music_setup,multimedia_music_setup_fp_win_focus);
    alv_am_win_set_defocus(win_music_setup,multimedia_music_setup_fp_win_defocus);

    lv_obj_t *      win_music_list = alv_am_win_create(app_multimedia,"music_list");

    alv_am_win_set_active(win_music_list,multimedia_music_list_fp_win_active);
    alv_am_win_set_deactive(win_music_list,multimedia_music_list_fp_win_deacactive);
    alv_am_win_set_focus(win_music_list,multimedia_music_list_fp_win_focus);
    alv_am_win_set_defocus(win_music_list,multimedia_music_list_fp_win_defocus);


    lv_obj_t *      win_record = alv_am_win_create(app_multimedia,"record");

    alv_am_win_set_active(win_record,multimedia_record_fp_win_active);
    alv_am_win_set_deactive(win_record,multimedia_record_fp_win_deacactive);
    alv_am_win_set_focus(win_record,multimedia_record_fp_win_focus);
    alv_am_win_set_defocus(win_record,multimedia_record_fp_win_defocus);


    lv_obj_t *      win_record_option = alv_am_win_create(app_multimedia,"record_option");

    alv_am_win_set_active(win_record_option,multimedia_record_option_fp_win_active);
    alv_am_win_set_deactive(win_record_option,multimedia_record_option_fp_win_deacactive);
    alv_am_win_set_focus(win_record_option,multimedia_record_option_fp_win_focus);
    alv_am_win_set_defocus(win_record_option,multimedia_record_option_fp_win_defocus);


    lv_obj_t *      win_record_list = alv_am_win_create(app_multimedia,"record_list");

    alv_am_win_set_active(win_record_list,multimedia_record_list_fp_win_active);
    alv_am_win_set_deactive(win_record_list,multimedia_record_list_fp_win_deacactive);
    alv_am_win_set_focus(win_record_list,multimedia_record_list_fp_win_focus);
    alv_am_win_set_defocus(win_record_list,multimedia_record_list_fp_win_defocus);


    lv_obj_t *      win_record_play = alv_am_win_create(app_multimedia,"record_play");

    alv_am_win_set_active(win_record_play,multimedia_record_play_fp_win_active);
    alv_am_win_set_deactive(win_record_play,multimedia_record_play_fp_win_deacactive);
    alv_am_win_set_focus(win_record_play,multimedia_record_play_fp_win_focus);
    alv_am_win_set_defocus(win_record_play,multimedia_record_play_fp_win_defocus);


    lv_obj_t *      win_file_browser_play = alv_am_win_create(app_multimedia,"file_browser_play");

    alv_am_win_set_active(win_file_browser_play,multimedia_file_browser_play_fp_win_active);
    alv_am_win_set_deactive(win_file_browser_play,multimedia_file_browser_play_fp_win_deacactive);
    alv_am_win_set_focus(win_file_browser_play,multimedia_file_browser_play_fp_win_focus);
    alv_am_win_set_defocus(win_file_browser_play,multimedia_file_browser_play_fp_win_defocus);


    app_music_player_init();
    app_tts_player_init();
    app_srv_recording_init();



}













