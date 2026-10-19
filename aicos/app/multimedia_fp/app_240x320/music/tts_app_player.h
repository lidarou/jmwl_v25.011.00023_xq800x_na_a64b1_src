

//#ifdef CONFIG_AUDIO_TTS

#ifndef __TTS_APP_PLAYER_H__
#define __TTS_APP_PLAYER_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include "aic_type.h"
#include "audio_system.h"
#include "audiofocus.h"
#include "aic_list.h"



#define     TTS_APP_ON_OFF_ALL      0x00000001
#define     TTS_APP_ON_OFF_KEY      0x00000002
#define     TTS_APP_ON_OFF_MENU     0x00000004
#define     TTS_APP_ON_OFF_TIME_AUTO 0x00000008
#define     TTS_APP_ON_OFF_TIME_ONCE 0x00000010
#define     TTS_APP_ON_OFF_CALL     0x00000020
#define     TTS_APP_ON_OFF_PHONEBOOK 0x00000040
#define     TTS_APP_ON_OFF_NEWS     0x00000080
#define     TTS_APP_ON_OFF_SMS      0x00000100
#define     TTS_APP_ON_OFF_CHARGE   0x00000200
#define     TTS_APP_ON_OFF_LOW_BAT  0x00000400
#define     TTS_APP_ON_OFF_LOCK_LCD 0x00000800


    enum {
        tts_app_on_off_all = 0,                     //总开关
        tts_app_on_off_key,                         //按键报号
        tts_app_on_off_menu,                        //菜单朗读
        tts_app_on_off_time_auto,                   //整点报时
        tts_app_on_off_once_time,                   //一键报时
        tts_app_on_off_call,                        //来电报号
        tts_app_on_off_phonebook,                   //电话本朗读
        tts_app_on_off_news,                        //新短信提醒
        tts_app_on_off_sms,                         //短选信息朗读
        tts_app_on_off_charge,                      //从电器插拔提醒
        tts_app_on_off_low_bat,                     //低电量提醒
        tts_app_on_off_lock_lcd,                    //解锁屏提醒
        };

    typedef struct {
        uint32_t        on_off;                     //开关
        uint8_t         vol;                        //音量
        uint8_t         time_auto_mode;             //1:白天模式，2：夜晚模式，3：全天模式，4：自定义模式
        uint16_t        time_auto_start;            //整点报时开始时间
        uint16_t        time_auto_end;              //整点报时结束时间
        uint8_t         vol_call;                   //电话音量
        uint8_t         vol_music;                  //音乐音量
        uint8_t         vol_record;                 //录音音量
        } tts_app_param_t;

    typedef void(*tts_notify_fun_user_t) (uint32_t event,void * userdata);

    uint32_t app_fp_play_tts_get_vol();
    void app_tts_stop();
    void app_fp_play_tts(uint8_t * str,uint8_t type);
    void app_fp_play_tts_menu(uint8_t * str);
    void app_fp_play_tts_time_auto(uint8_t * str);
    void app_fp_play_tts_once_time(uint8_t * str);
    uint32_t app_fp_play_tts_call(uint8_t * str);
    void app_fp_play_tts_sms(uint8_t * str);
    void app_fp_play_tts_phonebook(uint8_t * str);
    void app_fp_play_tts_key(uint8_t * str);
    uint32_t app_fp_play_tts_news(uint8_t * str);
    void app_fp_play_tts_charge(uint8_t * str);
    void app_fp_play_tts_low_bat(uint8_t * str);
    void app_fp_play_tts_lock_lcd(uint8_t * str);
    uint32_t app_fp_call_get_vol();
    uint32_t app_fp_call_set_vol(uint8_t vol);
    void app_tts_stop_force();
    uint32_t app_tts_set_notify_fun(tts_notify_fun_user_t fun,void * user_data);
    uint32_t app_fp_play_tts_get_on_off(uint8_t type);
    uint32_t app_fp_play_tts_enable_on_off(uint8_t type);
    uint32_t app_fp_play_tts_disable_on_off(uint8_t type);
    uint32_t app_fp_play_tts_get_time_auto_mode();
    uint32_t app_fp_play_tts_set_time_mode(uint8_t mode); //1:白天模式，2：夜晚模式，3：全天模式，4：自定义模式
    void app_tts_player_init();
    uint16_t app_fp_play_tts_get_time_auto_start();
    uint16_t app_fp_play_tts_get_time_auto_end();
    uint32_t app_fp_play_tts_set_time_auto(uint16_t start_time,uint16_t end_time);
    uint32_t app_fp_play_tts_set_vol(uint8_t vol);
    uint32_t app_fp_record_get_vol();
    uint32_t app_fp_record_set_vol(uint8_t vol);
    void app_fp_play_tts_time_auto_disable();
    uint32_t app_fp_play_tts_get_vol1();
    uint32_t app_fp_play_tts_set_vol1(uint8_t vol);
    uint32_t app_fp_is_other_audio_play();
    uint32_t get_app_fp_play_tts_news_is_enable();

#ifdef __cplusplus
} /* extern "C" */

#endif

#endif

//#endif /* __PLAYSERVICE_H__ */

