/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file entry apps with GUI for aicos
 */

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>

#include <lvgl.h>
#include "alv_app_mgr.h"
#include "app_msg.h"
#include "em.h"
#include "settings.h"
#include "idle.h"
#include "call.h"
#include "camera.h"
#include "aic_srv_backlight.h"
#include "statusbar.h"
#include "softkey.h"
#include "aic_boot_mode.h"
#include "app_log.h"
#include "display.h"
#include "bootanimation.h"
#include "pwr_key.h"
#include "backlight.h"
#include "aic_cmdline.h"
#include "notify_center.h"
#include "lv_port_indev_keypad.h"
#include "kpc.h"
#include "usb.h"
#ifdef CONFIG_FP
#include "aic_md_id.h"
#include "aic_md_api.h"
#endif


#ifdef CONFIG_APP_PHOTO
#include "photo.h"
#endif
#ifdef CONFIG_APP_FILEBROWSER
#include "filebrowser.h"
#endif
#ifdef CONFIG_APP_RECORDER
#include "recorder.h"
#endif
#ifdef CONFIG_APP_FOTA
#include "app_fota.h"
#endif
#ifdef CONFIG_APP_SMS_TEST
#include "sms_test.h"
#endif
#ifdef CONFIG_TELE_SERVICE
#include "aic_srv_tele.h"
#endif
#include "aic_srv_tmr.h"

#ifdef CONFIG_AUDIO_SUPPORT
#include "app_audio.h"
#endif

#ifdef CONFIG_APP_REMIND_FP
#include "app_remind_list.h"
#endif

#ifdef CONFIG_FP
#include "obj_batch_deal.h"
#endif

#ifdef CONFIG_APP_IDLE_FP
extern void idle_fp_init(void);
#endif
#ifdef CONFIG_APP_HOME_FP
extern void home_fp_init(void);
#endif
#ifdef CONFIG_APP_PHONEBOOK_FP
extern void app_phonebook_fp_init(void);
extern void app_blacklist_fp_init(void);
#endif

#ifdef CONFIG_APP_SMS_FP
extern void app_sms_fp_init(void);
#endif

#ifdef CONFIG_APP_SETTINGS_FP
extern void settings_fp_init(void);
#endif

#ifdef CONFIG_APP_CALLLOG_FP
extern void app_calllog_fp_init(void);
#endif

#ifdef CONFIG_APP_CALL_FP
extern void app_call_fp_init(void);
#endif
#ifdef CONFIG_APP_TOOLS_FP
extern void tools_fp_init(void);
#endif

#ifdef CONFIG_APP_MULTIMEDIA_FP
extern void app_multimedia_fp_init(void);
#endif


#ifdef CONFIG_APP_REMIND_FP
extern void remind_fp_init();
#endif


#ifdef CONFIG_APP_ENTRY_LVGL_DEMO
#define APP_CFG_TASK_COM_PRIO       (5u)/* TODO: temp for LVGL Benchmark, move to Benchmark APP later. */
#else
#define APP_CFG_TASK_COM_PRIO       (35u)
#endif
#define APP_CFG_TASK_STK_SIZE       (4096u)
#define APP_CFG_MSG_RCV_TIMEOUT     (5)
#define APP_WDT_TIMEOUT             (120000)

uint32_t g_app_msg_recv_timeout = APP_CFG_MSG_RCV_TIMEOUT;

#ifdef CONFIG_AUDIO_SUPPORT
/* power on ring data */
static const char g_power_on_ring_data[] = {
    #include "power_on.h"
};
#endif

#ifdef CONFIG_FP
typedef struct _POWER_ON_INIT_STATE_PARAM_H_
{
    uint8_t init[4];
}poweron_init_state_param_t;
#endif

static aos_timer_ptr app_wdt_timer = NULL;

/* app watch dog begin */
static void app_wdt_kick(void)
{
    aos_change_timer(app_wdt_timer, APP_WDT_TIMEOUT, AOS_BE_PERIOD);
}
static void app_wdt_timeout(void *param)
{
    aic_assert(NULL);
}
static void app_wdt_init(void)
{
    app_wdt_timer = aos_create_timer("app_wdt_timeout",
                                         app_wdt_timeout, NULL,
                                         APP_WDT_TIMEOUT,
                                         AOS_AUTO_ACTIVATE, AOS_BE_PERIOD);
    aic_assert(app_wdt_timer);
}
/* app watch dog end */


/* TODO:move to key external msg */
#ifdef CONFIG_KPC
/* TODO delete macro KPC control */
static int32_t kpc_callback(void *p_param, uint32_t size)
{
    u32 brt_perc = 0;

    /* param check */
    if (NULL == p_param) {
        APP_LOGE("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    aic_bl_brightness_get(&brt_perc);
    if(brt_perc != 0) {/*Avoiding the error of keypad write when the screen is off (app sleep) , eg.not passing key values, passing incorrect key values*/
        /*write keypad when the screen is on*/
#ifdef CONFIG_FP
        if(keypad_write(p_param) == 1)
        {
            if(false == alv_am_app_is_exist("idle_fp")) {
                lv_obj_t * frist_win;
                lv_obj_t *app;
                app = get_app_by_name("idle_fp");
                frist_win = get_win_by_name(app,"power_off");
                alv_am_app_set_first_win(app, frist_win);

                alv_am_app_enter_action(NULL, "idle_fp", NULL);
            } else {
                alv_am_win_jump_action(NULL, "power_off", NULL);
            }
        }
#else
        keypad_write(p_param);
#endif
    }

    /*key wakeup*/
    backlight_turn_on();

    return 0;
}

__APP_CALLBACK_FUNCTION__(kpc_callback);
#endif
#ifdef CONFIG_FP
extern void power_on_alarm(void);
#endif
int32_t entry_first_app(void *para, uint32_t size)
{
    srv_tmr_cb_param_t *p_tmr_cb_param = NULL;

    if(NULL == para) {
        return -1;
    }

    p_tmr_cb_param = (srv_tmr_cb_param_t *)para;
    if(NULL != p_tmr_cb_param->tmr_handle) {
        aic_srv_tmr_destroy(p_tmr_cb_param->tmr_handle);
    }

#ifdef CONFIG_APP_ENTRY_LVGL_DEMO
    extern void lv_demo_app_init(void);
    lv_demo_app_init();
#endif

#ifdef CONFIG_APP_ENTRY_EM
    alv_am_app_enter(NULL, "em", NULL);
#endif

#ifdef CONFIG_APP_ENTRY_IDLE
    alv_am_app_enter(NULL, "idle", NULL);
#endif

#ifdef CONFIG_APP_ENTRY_IDLE_FP
    {
        lv_obj_t * frist_win;
        lv_obj_t * idle_app;

        extern uint8_t get_security_state(void);
        if(get_security_state())
        {
            idle_app = get_app_by_name("idle_fp");
            frist_win = get_win_by_name(idle_app,"poweron_password");
            alv_am_app_set_first_win(idle_app, frist_win);
        } else {
            extern void ts_sim_pin_param_init();
            ts_sim_pin_param_init();
        }
    }
    //alv_am_app_enter(NULL, "home_fp", NULL);
    alv_am_app_enter(NULL, "idle_fp", NULL);
#ifdef CONFIG_APP_REMIND_FP
    {
        extern uint8_t get_bat_capacity();
        extern uint8_t get_bat_chg_status();
        if((get_bat_capacity() <= 2) && (get_bat_chg_status() == 2))// POWER_SUPPLY_STATUS_DISCHARGING 开机电池电量低且没有充电，就立即自动关机
        {
            box_bat_remind_show(3,0);
        }
        else if((get_bat_capacity() <= 15) && (get_bat_chg_status() == 2))// POWER_SUPPLY_STATUS_DISCHARGING 开机电池电量低
        {
            box_bat_remind_show(2,0);
        }
        if(get_bat_chg_status() == 1 || get_bat_chg_status() == 3 || get_bat_chg_status() == 4)
        {
            extern void change_usb_charger_status(uint8_t status);
            change_usb_charger_status(1);
        }
        else
        {
            extern void change_usb_charger_status(uint8_t status);
            change_usb_charger_status(0);
        }
    }
#endif

#endif
#ifdef CONFIG_FP
    power_on_alarm();
#endif
    backlight_turn_on();

    return AIC_SUCCESS;
}
__APP_CALLBACK_FUNCTION__(entry_first_app);
#ifdef CONFIG_FP
uint8_t play_power_on_off_voice(srv_func_callback _buf_start_callback)
{
#ifdef CONFIG_AUDIO_SUPPORT
    /* play power on ring */
    extern uint8_t power_on_off_sound_status(void);
    extern uint32_t app_musicplayer_get_play_status();
    extern void app_musicplayer_stop();
    if(app_musicplayer_get_play_status())
        app_musicplayer_stop();

    if(power_on_off_sound_status())
    {
        app_audio_play_buf_param_t buf_param = {0};

        audio_set_master_volume(10);
        buf_param.flag = AUDIO_OUTPUT_FLAG_COMPRESS;
        buf_param.sample_rate = 44100;
        buf_param.bit_rate = 0;
        buf_param.format = ASTREAM_FORMAT_MP3;
        buf_param.channel = ASTREAM_CHN_STEREO;
        app_audio_play_buf_start(g_power_on_ring_data,
                                 sizeof(g_power_on_ring_data),
                                 0,
                                 &buf_param,
                                 _buf_start_callback);
        return 1;
    }
    else
        return 0;
#endif

}

void fp_play_power_off_ring_action()
{
#ifdef CONFIG_AUDIO_SUPPORT
    /* play power on ring */
    extern uint8_t power_on_off_sound_status(void);

    if(power_on_off_sound_status())
    {
        app_audio_play_buf_param_t buf_param = {0};
        audio_set_master_volume(10);

        buf_param.flag = AUDIO_OUTPUT_FLAG_COMPRESS;
        buf_param.sample_rate = 44100;
        buf_param.bit_rate = 0;
        buf_param.format = ASTREAM_FORMAT_MP3;
        buf_param.channel = ASTREAM_CHN_STEREO;
        app_audio_play_buf_start(g_power_on_ring_data,
                                 sizeof(g_power_on_ring_data),
                                 0,
                                 &buf_param,
                                 NULL);
        return;
    }
    else
        return;
#endif
}

uint8_t power_on_type =0; // 正常开机为0；充电开机为1
#endif
static void AppTaskGUI(void *thread_input)
{
    uint32_t ret = 0;
    app_callback_msg_t recv_msg = {0};

    /* Judage Boot mode for whether entry APP */
    if(aic_boot_mode_is_gxem()) {
        return;
    }
    else if (aic_boot_mode_is_charge() ) {
#ifdef CONFIG_FP
        extern void get_display(unsigned char * display);
        uint32_t time_value[5] = {10000,20000,40000,60000,UINT32_MAX};
        backlight_config_t backlight;
        unsigned char display[2];
        get_display(display);
        backlight.brightness_on = display[0];
        backlight.turn_off_timeout = time_value[display[1]];
        backlight.half_off_timeout = 0;
        backlight.brightness_half_on = 0;
        backlight_set_config(backlight);
#endif
        APP_LOGI("@@@@@@@@@@ [%s] Show charge animation @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n", __func__);
#ifdef CONFIG_FP
        fp_lv_style_font_custom_init();
#endif
#ifdef CONFIG_CHARGE_ANIMATION
        show_charge_animation();
#endif
#ifdef CONFIG_FP
        power_on_type = 1;
        backlight_turn_on();
#ifdef CONFIG_KPC
        /* TODO delete macro KPC control */
        aic_srv_kpc_register("app", __APP_CALLBACK_NAME__(kpc_callback));
#endif
#endif

        goto LV_MAIN_LOOP;
    }

#ifdef CONFIG_FP
    extern void alarm_data_init();
    alarm_data_init();
    power_on_type = 0;
#endif
    /* Show boot animation */
    show_boot_animation();

    srv_tmr_param_t timer_param = {0};
    timer_param.time_out = 6500;
    timer_param.is_repeat = false;
    timer_param.p_name = "boot_animation";
    timer_param.p_usr_data = NULL;
    aic_srv_tmr_create(&timer_param, __APP_CALLBACK_NAME__(entry_first_app));

#ifdef CONFIG_FP
    s32 ret1;
    poweron_init_state_param_t phone_init;
    ret1 = aic_md_read(MD_ID_PHONE_INITED, sizeof(poweron_init_state_param_t), &phone_init);
    if (ret1) {
        APP_LOGI("read security cfg error 0x%x\n", ret);
    }
    if(phone_init.init[0] != 0x55 || phone_init.init[1] != 0xaa || phone_init.init[2] != 0x55 || phone_init.init[3] != 0xaa)
    {
        phone_init.init[0] = 0x55;
        phone_init.init[1] = 0xaa;
        phone_init.init[2] = 0x55;
        phone_init.init[3] = 0xaa;
        ret = aic_md_write(MD_ID_PHONE_INITED, sizeof(poweron_init_state_param_t), &phone_init);
        if (ret) {
            APP_LOGI("read security cfg error 0x%x\n", ret);
        }
        if(1)
        {
            extern void reset_family(void);
            extern void reset_phone(void);
            extern void reset_display(void);
            extern void reset_sos(void);
            extern void reset_net(void);
            extern void reset_security(void);
            extern void reset_theme(void);
            extern void reset_date(void);
            extern void reset_comm(void);
            extern void reset_alarm(void);
            extern void reset_todo(void);
            extern void reset_power_on_alarm_id(void);
            extern void reset_app_fp_play_tts_param_data();
            extern void reset_tele_phonebook_nv_data();
            reset_family();
            reset_phone();
            reset_display();
            reset_sos();
            reset_net();
            reset_security();
            reset_theme();
            reset_date();
            reset_comm();
            reset_alarm();
            reset_todo();
            reset_power_on_alarm_id();
            reset_app_fp_play_tts_param_data();
            reset_tele_phonebook_nv_data();
        }
    }
    // 复位初始化后，设置背光时长和亮度
    if(1)
    {
        extern void get_display(unsigned char * display);
        extern void alarm_para_init(void);
        extern void settings_flow_get_datetime_dec(uint8_t *date, uint8_t *time);
        extern void settings_flow_set_datetime(uint8_t *date, uint8_t *time);
        uint8_t date[4];
        uint8_t time[2];
        uint32_t time_value[5] = {10000,20000,40000,60000,UINT32_MAX};
        backlight_config_t backlight;
        unsigned char display[2];
        get_display(display);
        backlight.brightness_on = display[0];
        backlight.turn_off_timeout = time_value[display[1]];
        backlight.half_off_timeout = 0;
        backlight.brightness_half_on = 0;
        backlight_set_config(backlight);
        alarm_para_init();
        settings_flow_get_datetime_dec(date,time);
        if(date[0]  <= 0)
        {
            date[0] = 2024 -1970;
            date[1] = 1;
            date[2] = 1;
            time[0] = 0;
            time[1] = 0;
            settings_flow_set_datetime(date,time);
        }
    }
    play_power_on_off_voice(NULL);
#else
#ifdef CONFIG_AUDIO_SUPPORT
    /* play power on ring */
    app_audio_play_buf_param_t buf_param = {0};
    buf_param.flag = AUDIO_OUTPUT_FLAG_COMPRESS;
    buf_param.sample_rate = 44100;
    buf_param.bit_rate = 0;
    buf_param.format = ASTREAM_FORMAT_MP3;
    buf_param.channel = ASTREAM_CHN_STEREO;
    app_audio_play_buf_start(g_power_on_ring_data,
                             sizeof(g_power_on_ring_data),
                             0,
                             &buf_param,
                             NULL);
#endif
#endif

    /*************************************************************************/
    /* The Initial of Telephone MUST be done before Other APPs Inital */
    /*************************************************************************/
#ifdef CONFIG_TELE_SERVICE
    /* Power on radio (SimCard 1) when boot in NOT fly mode */
    aic_srv_tele_radio_on(TS_CARD_ID_0, true, NULL);
    /* Set VOLTE enable (SimCard 1). The default value is Eable if not set */
    aic_srv_tele_call_volte_enable(TS_CARD_ID_0, true, NULL);
#endif


    /* TODO:move to key external msg */
#ifdef CONFIG_KPC
    /* TODO delete macro KPC control */
    aic_srv_kpc_register("app", __APP_CALLBACK_NAME__(kpc_callback));
#endif
#ifdef CONFIG_COMP_STATUSBAR
    comp_statusbar_init();
#endif

#ifdef CONFIG_APP_CALL
    app_call_init();
#endif

#ifdef CONFIG_APP_EM
    em_init();
#endif

#ifdef CONFIG_APP_SETTINGS
    settings_init();
#endif

#ifdef CONFIG_APP_PHOTO
    app_photo_init();
#endif

#ifdef CONFIG_APP_FILEBROWSER
    register_app_filebrowser();
#endif

#ifdef CONFIG_APP_RECORDER
    app_recorder_register();
#endif

#ifdef CONFIG_APP_SMS_TEST
    app_sms_test_register();
#endif

#ifdef CONFIG_APP_FOTA
    register_app_fota();
#endif

#ifdef CONFIG_APP_CAMERA
    app_camera_init(0);
#endif

#ifdef CONFIG_APP_IDLE
    idle_init();
#endif

#ifdef CONFIG_APP_NOTIFY_CENTER
    notify_center_init();
#endif

#ifdef CONFIG_APP_USB
    app_usb_init();
#endif

#ifdef CONFIG_APP_IDLE_FP
    idle_fp_init();
#endif

#ifdef CONFIG_APP_HOME_FP
    home_fp_init();
#endif

#ifdef CONFIG_APP_PHONEBOOK_FP
    app_phonebook_fp_init();
    app_blacklist_fp_init();
#endif

#ifdef CONFIG_APP_SMS_FP
    app_sms_fp_init();
#endif

#ifdef CONFIG_APP_SETTINGS_FP
    settings_fp_init();
#endif

#ifdef CONFIG_APP_CALLLOG_FP
    app_calllog_fp_init();
#endif

#ifdef CONFIG_APP_CALL_FP
    app_call_fp_init();
#endif
#ifdef CONFIG_APP_TOOLS_FP
    tools_fp_init();
#endif

#ifdef CONFIG_APP_MULTIMEDIA_FP
    app_multimedia_fp_init();
#endif

#ifdef CONFIG_APP_REMIND_FP
    remind_fp_init();
#endif

LV_MAIN_LOOP:
if (aic_is_user_mode()) {
    app_wdt_init();
}

    while (1) {

        if(aic_is_user_mode()) {
            app_wdt_kick();
        }
        lv_timer_handler();
        //aos_sleep(5);

        ret = app_msg_recv(&recv_msg, g_app_msg_recv_timeout);
        if( AOS_QUEUE_EMPTY == ret ) {
            continue;
        }
        else {
            switch (recv_msg.msg_id) {
                case 0xAAAAAAAA:
                    {
                        srv_func_callback p_cb = (app_msg_func_callback)recv_msg.p_func_callback;
                        if(NULL != p_cb) {
                            p_cb(recv_msg.p_param, recv_msg.size);
                        }

                        /* free recieved msg */
                        if(NULL != recv_msg.p_param) {
                            aic_free(recv_msg.p_param);
                        }
                    }
                    break;
                default:
                    APP_LOGI("Recive a APP msg which is NOT known. \n");
                    break;
            }
        }
    }
}

int app_init(void)
{
    APP_LOGI("OOOOOOOOOO [%s] Enter OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO \n", __func__);

    /* Initial app msg queue for extern event handle and dispatch. */
    app_msg_init();

    aos_create_thread(
        "app_task",
        AppTaskGUI,
        NULL,
        APP_CFG_TASK_STK_SIZE,
        0,
        NULL,
        APP_CFG_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(app_init);

int app_charge_mode_init(void)
{
    APP_LOGI("@@@@@@@@@@ [%s] Enter @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n", __func__);
#ifdef CONFIG_CHARGE_ANIMATION
    return app_init();
#else
    return show_charge_picture();
#endif
}

AIC_CHARGE_APP_L1_INIT_EXPORT(app_charge_mode_init);

