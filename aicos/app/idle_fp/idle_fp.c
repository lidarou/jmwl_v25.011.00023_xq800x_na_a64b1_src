
#include "lvgl.h"

#include "alv_app_mgr.h"

#include "app_log.h"
#include "backlight.h"


#include "app_msg.h"
#include "aic_srv_tele.h"
#include <stdio.h>
#include "app_audio.h"
#include "idle_fp.h"
#include "status_fp.h"
#include "obj_batch_deal.h"
#include "tts_app_player.h"

extern lv_obj_t * ui_Screenidle1_screen_init(lv_obj_t * win,void * param);
extern void ui_Screenidle1_screen_uninit(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenIdleNumWirte_screen_init(lv_obj_t * win,void * param);
extern void ui_ScreenIdleNumWirte_screen_uninit(lv_obj_t * win,void * param);
extern void ui_ScreenIdleNumWirte_screen_focus(lv_obj_t * win,void * enter_param);
extern void ui_ScreenIdleNumWirte_screen_focus(lv_obj_t * win,void * enter_param);
extern lv_obj_t * ui_ScreenIdleNumOption_screen_init(lv_obj_t * win,void * enter_param);
extern void ui_ScreenIdleNumOption_screen_uninit(lv_obj_t * win,void * enter_param);
extern lv_obj_t * ui_Screenidle_lock_screen_init(lv_obj_t * win,void * param);
extern void ui_Screenidle_lock_screen_uninit(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenImei_screen_init(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenImei_screen_init(lv_obj_t * win,void * p);
extern void ui_ScreenIdleNumWirte_screen_defocus(lv_obj_t * win,void * enter_param);
extern void ui_ScreenImei_screen_uninit(lv_obj_t * win,void * p);
extern lv_obj_t * ui_ScreenSos_screen_init(lv_obj_t * win,void * p);
extern void ui_ScreenSos_screen_uninit(lv_obj_t * win,void * p);

void idle_fp_app_focus(lv_obj_t * app,void * p_param)
{

}


void idle_fp_app_defocus(lv_obj_t * app,void * p_param)
{


}


void idle_fp_app_active(lv_obj_t * app,void * p_param)
{

}


void idle_fp_app_deacactive(lv_obj_t * app,void * p_param)
{

}


void idle_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen;

    ui_screen           = ui_Screenidle1_screen_init(win,p_param);

    lv_disp_load_scr(ui_screen);
    app_tts_stop();
}


void idle_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_Screenidle1_screen_uninit(win,p_param);

}


void idle_fp_win_active(lv_obj_t * win,void * p_param)
{

}


void idle_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
}


void idle_fp_pass_win_active(lv_obj_t * win,void * p_param)
{
}


void idle_fp_pass_win_deactive(lv_obj_t * win,void * p_param)
{

}


void idle_fp_pass_win_focus(lv_obj_t * win,void * p_param)
{
    ui_Screenidle_Pass_screen_init(p_param);
    lv_disp_load_scr(ui_ScreenidlePass);
}


void idle_fp_pass_win_defocus(lv_obj_t * win,void * p_param)
{
    if (ui_ScreenidlePass)
        lv_obj_del_delayed_custom(ui_ScreenidlePass,500);

    ui_ScreenidlePass   = NULL;
}




void idle_fp_num_write_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumWirte_screen_init(win,p_param);
}


void idle_fp_num_write_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumWirte_screen_uninit(win,p_param);
}


void idle_fp_num_write_win_focus(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumWirte_screen_focus(win,p_param);

}


void idle_fp_num_write_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumWirte_screen_defocus(win,p_param);
}




void idle_fp_num_option_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumOption_screen_init(win,p_param);
}


void idle_fp_num_option_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenIdleNumOption_screen_uninit(win,p_param);
}


void idle_fp_num_option_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen;

    ui_screen           = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}


void idle_fp_num_option_win_defocus(lv_obj_t * win,void * p_param)
{
}



void idle_lock_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen;

    ui_screen           = ui_Screenidle_lock_screen_init(win,p_param);

    lv_disp_load_scr(ui_screen);
}


void idle_lock_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    ui_Screenidle_lock_screen_uninit(win,p_param);

}


void idle_lock_fp_win_active(lv_obj_t * win,void * p_param)
{

}


void idle_lock_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
}


void power_off_fp_win_focus(lv_obj_t * win,void * p_param)
{

    ui_ScreenPowerOff_screen_init(p_param);

    lv_disp_load_scr(ui_ScreenPowerOff);
}


void power_off_fp_win_defocus(lv_obj_t * win,void * p_param)
{
    if (ui_ScreenPowerOff)
        lv_obj_del_delayed_custom(ui_ScreenPowerOff,500);

    ui_ScreenPowerOff   = NULL;

}


void power_off_fp_win_active(lv_obj_t * win,void * p_param)
{

}


void power_off_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
}




void imei_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenImei_screen_init(win,p_param);

}


void imei_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenImei_screen_uninit(win,p_param);
}


void imei_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}


void imei_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void sos_fp_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSos_screen_init(win,p_param);

}

void sos_fp_win_deacactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSos_screen_uninit(win,p_param);
}

void sos_fp_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}

void sos_fp_win_defocus(lv_obj_t * win,void * p_param)
{

}

void idle_fp_sim_pin_win_active(lv_obj_t * win,void * p_param)
{
    ui_ScreenSimPinscreen_init(win,p_param);

}

void idle_fp_sim_pin_win_deactive(lv_obj_t * win,void * p_param)
{
    ui_ScreenSimPinscreen_uninit(win,p_param);
}

void idle_fp_sim_pin_win_focus(lv_obj_t * win,void * p_param)
{
    lv_obj_t *      ui_screen = lv_obj_get_user_data(win);

    lv_disp_load_scr(ui_screen);
}

void idle_fp_sim_pin_win_defocus(lv_obj_t * win,void * p_param)
{

}

/*
watch	->idle_num_write->idle_num_option
    
*/
void idle_fp_init(void)
{

    /* Create APP for idle */
    lv_obj_t *      app_idle = alv_am_app_create(lv_scr_act(),"idle_fp");

    alv_am_app_set_active(app_idle,idle_fp_app_active);
    alv_am_app_set_deactive(app_idle,idle_fp_app_deacactive);
    alv_am_app_set_focus(app_idle,idle_fp_app_focus);
    alv_am_app_set_defocus(app_idle,idle_fp_app_defocus);

    lv_obj_t *      win_fisrt = alv_am_win_create(app_idle,"watch");

    alv_am_win_set_active(win_fisrt,idle_fp_win_active);
    alv_am_win_set_deactive(win_fisrt,idle_fp_win_deacactive);
    alv_am_win_set_focus(win_fisrt,idle_fp_win_focus);
    alv_am_win_set_defocus(win_fisrt,idle_fp_win_defocus);
    alv_am_app_set_first_win(app_idle,win_fisrt);

    lv_obj_t *      win_wacth_password = alv_am_win_create(app_idle,"poweron_password");

    alv_am_win_set_active(win_wacth_password,idle_fp_pass_win_active);
    alv_am_win_set_deactive(win_wacth_password,idle_fp_pass_win_deactive);
    alv_am_win_set_focus(win_wacth_password,idle_fp_pass_win_focus);
    alv_am_win_set_defocus(win_wacth_password,idle_fp_pass_win_defocus);



    lv_obj_t *      win_idle_num_write = alv_am_win_create(app_idle,"idle_num_write");

    alv_am_win_set_active(win_idle_num_write,idle_fp_num_write_win_active);
    alv_am_win_set_deactive(win_idle_num_write,idle_fp_num_write_win_deacactive);
    alv_am_win_set_focus(win_idle_num_write,idle_fp_num_write_win_focus);
    alv_am_win_set_defocus(win_idle_num_write,idle_fp_num_write_win_defocus);


    lv_obj_t *      win_idle_num_option = alv_am_win_create(app_idle,"idle_num_option");

    alv_am_win_set_active(win_idle_num_option,idle_fp_num_option_win_active);
    alv_am_win_set_deactive(win_idle_num_option,idle_fp_num_option_win_deacactive);
    alv_am_win_set_focus(win_idle_num_option,idle_fp_num_option_win_focus);
    alv_am_win_set_defocus(win_idle_num_option,idle_fp_num_option_win_defocus);

    lv_obj_t *      win_idle_lock = alv_am_win_create(app_idle,"idle_lock");

    alv_am_win_set_active(win_idle_lock,idle_lock_fp_win_active);
    alv_am_win_set_deactive(win_idle_lock,idle_lock_fp_win_deacactive);
    alv_am_win_set_focus(win_idle_lock,idle_lock_fp_win_focus);
    alv_am_win_set_defocus(win_idle_lock,idle_lock_fp_win_defocus);

    lv_obj_t *      win_power_off = alv_am_win_create(app_idle,"power_off");

    alv_am_win_set_active(win_power_off,power_off_fp_win_active);
    alv_am_win_set_deactive(win_power_off,power_off_fp_win_deacactive);
    alv_am_win_set_focus(win_power_off,power_off_fp_win_focus);
    alv_am_win_set_defocus(win_power_off,power_off_fp_win_defocus);

    lv_obj_t *      win_imei = alv_am_win_create(app_idle,"fp_imei");

    alv_am_win_set_active(win_imei,imei_fp_win_active);
    alv_am_win_set_deactive(win_imei,imei_fp_win_deacactive);
    alv_am_win_set_focus(win_imei,imei_fp_win_focus);
    alv_am_win_set_defocus(win_imei,imei_fp_win_defocus);

    lv_obj_t *      win_sos = alv_am_win_create(app_idle,"fp_sos");

    alv_am_win_set_active(win_sos,sos_fp_win_active);
    alv_am_win_set_deactive(win_sos,sos_fp_win_deacactive);
    alv_am_win_set_focus(win_sos,sos_fp_win_focus);
    alv_am_win_set_defocus(win_sos,sos_fp_win_defocus);

    lv_obj_t *      win_sim_pin = alv_am_win_create(app_idle,"fp_sim_pin");

    alv_am_win_set_active(win_sim_pin,idle_fp_sim_pin_win_active);
    alv_am_win_set_deactive(win_sim_pin,idle_fp_sim_pin_win_deactive);
    alv_am_win_set_focus(win_sim_pin,idle_fp_sim_pin_win_focus);
    alv_am_win_set_defocus(win_sim_pin,idle_fp_sim_pin_win_defocus);

    status_fp_init();

    fp_lv_style_font_custom_init();
}







