


#include <stdio.h>
#include "playtts_service.h"
#include "aic_bitmap.h"
#include "aic_errno.h"
#include "aic_osa.h"
#include "aic_osa_ex.h"
#include "audio_system.h"
#include "audiofocus.h"
#include <time.h>
#include <stdlib.h>
#include "lvgl.h"
#include "stdint.h"
#include "tts_app_player.h"
#include "app_log.h"
#include "alarm_manager.h"
#include "fp_playservice.h"
#include "aic_md_api.h"
#include "aic_md_id.h"
#include "playtts_service.h"
#include "charset.h"
#include "musicplayer_fp_play.h"
#include "call_fp_flow.h"
#include "app_msg.h"


#if 1
#define __inf                   APP_LOGI

#else

#define __inf
#endif

#ifdef CONFIG_AUDIO_TTS
extern uint8_t get_incoming_call_volume(void);

void app_fp_play_tts_time_auto_enable();
void app_fp_play_tts_time_auto_disable();

struct tts_service * g_tts_service;
static tts_app_param_t tts_app_param;
static tts_notify_fun_user_t notify_fun_user;
static void *   notify_user_data;


static void app_tts_service_notify(enum tts_client_cb_event event,void * appdata)
{

    switch (event)
    {
        case EVENT_TTS_PLAY_FINISH:
        case EVENT_TTS_PLAY_BREAK:
            if (notify_fun_user)
                notify_fun_user(event,notify_user_data);

            break;

        default:
            break;
        }
}


int32_t app_fp_play_tts_save_to_nv(tts_app_param_t * app_param)
{
    int32_t         ret;

    ret                 = aic_md_write(MD_ID_TTS_PARAM,sizeof(tts_app_param_t),(u8 *) app_param);

    if (ret)
        __inf("app_fp_play_tts_save_to_nv error 0x%x\n",ret);

    return ret;

}


void reset_app_fp_play_tts_param_data()
{
    tts_app_param.vol   = 12;
    tts_app_param.on_off = 0xffffffff;
    tts_app_param.time_auto_mode = 0;
    tts_app_param.vol_call = 8;
    tts_app_param.vol_music = 7;
    tts_app_param.vol_record = 7;

    app_fp_play_tts_save_to_nv(&tts_app_param);
}


void app_fp_play_tts_param_init()
{

    int32_t         ret;

    ret                 = aic_md_read(MD_ID_TTS_PARAM,sizeof(tts_app_param_t),(u8 *) &tts_app_param);

    if (ret) {
        __inf("app_fp_play_tts_param_init error 0x%x\n",ret);
        reset_app_fp_play_tts_param_data();

        //app_fp_play_tts_set_time_mode(1);
        }
}


void app_tts_player_init()
{
    notify_fun_user     = NULL;
    notify_user_data    = NULL;

    g_tts_service       = (struct tts_service *) tts_play_service_create();
    app_fp_play_tts_param_init();

    g_tts_service->ops->regcb((struct tts_service *) g_tts_service,0,app_tts_service_notify,g_tts_service);

    app_fp_play_tts_set_time_mode(tts_app_param.time_auto_mode);
}

static void tts_player_timer_handle_cb(lv_timer_t * t)
{
    struct tts_service * tts = t->user_data;
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.msg_id          = QUEUE_PS_ID_TTS_PLAY + tts->priv.msg_id;
    msg.p_param         = tts->priv.str_buf_ready;
    ps_data_msg_send(&msg,AOS_NO_WAIT);


    tts->priv.str_buf_ready = NULL;
    lv_timer_pause(t);
}

static void tts_play_send_msg_timer(struct tts_service * tts,uint32_t msg_id,uint8_t * str_buf)
{
    if (tts->priv.timer) {
        lv_timer_reset(tts->priv.timer);
        lv_timer_resume(tts->priv.timer);

        if (tts->priv.str_buf_ready)
            lv_mem_free(tts->priv.str_buf_ready);

        tts->priv.str_buf_ready = str_buf;
        }
    else {
        tts->priv.timer     = lv_timer_create(tts_player_timer_handle_cb,300,tts);
        tts->priv.str_buf_ready = str_buf;
        }

    tts->priv.msg_id    = msg_id;
}

static void tts_stop_send_msg_timer(struct tts_service * tts)
{
    if (tts->priv.timer) {
        lv_timer_pause(tts->priv.timer);

        if (tts->priv.str_buf_ready)
            lv_mem_free(tts->priv.str_buf_ready);

        tts->priv.str_buf_ready = NULL;
        }

    ps_data_msg_send_tts(QUEUE_PS_ID_STOP);
}

void app_tts_play(uint8_t * str_buf)
{
    struct tts_service * tts = g_tts_service;

#if 0
    tts->ops->play(tts,str_buf);
#elif 0

    if (tts->priv.timer) {
        lv_timer_reset(tts->priv.timer);
        lv_timer_resume(tts->priv.timer);

        if (tts->priv.str_buf_ready)
            lv_mem_free(tts->priv.str_buf_ready);

        tts->priv.str_buf_ready = str_buf;

        __inf("app_tts_play:%d",tts->priv.status);

        if (tts->priv.status != TTS_IDLE) {
            u32             flag = BIT(EVENT_TTS_PLAY_STOP);

            aos_set_event(tts->priv.event,flag,AOS_OR);
            }
        }
    else {
        tts->priv.timer     = lv_timer_create(app_tts_player_timer_handle_cb,300,tts);
        tts->priv.str_buf_ready = str_buf;
        }

#elif 0
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.msg_id          = QUEUE_PS_ID_PLAY;
    msg.p_param         = str_buf;
    ps_data_msg_send(&msg,AOS_NO_WAIT);
#elif 1
    tts_play_send_msg_timer(tts,QUEUE_PS_ID_PLAY,str_buf);
#endif
}

void app_tts_play_call(uint8_t * str_buf)
{
    struct tts_service * tts = g_tts_service;

#if 0
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.msg_id          = QUEUE_PS_ID_CALL_RING;
    msg.p_param         = str_buf;
    ps_data_msg_send(&msg,AOS_NO_WAIT);

#else

    tts_play_send_msg_timer(tts,QUEUE_PS_ID_CALL_RING,str_buf);
#endif
}

void app_tts_play_new(uint8_t * str_buf)
{
    struct tts_service * tts = g_tts_service;

#if 0
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.msg_id          = QUEUE_PS_ID_NEWS;
    msg.p_param         = str_buf;
    ps_data_msg_send(&msg,AOS_NO_WAIT);

#else

    tts_play_send_msg_timer(tts,QUEUE_PS_ID_NEWS,str_buf);
#endif
}

void app_tts_stop()
{
    struct tts_service * tts = g_tts_service;

#if 0
    tts->ops->stop(tts);
#elif 0
    u32             flag = BIT(EVENT_TTS_PLAY_STOP);

    //tts_queue_param_t msg;
    //msg.msg_id = BIT(EVENT_TTS_PLAY_STOP);
    //tts_data_msg_send(tts,&msg,AOS_NO_WAIT);
    if (tts->priv.timer) {
        lv_timer_pause(tts->priv.timer);

        if (tts->priv.str_buf_ready)
            lv_mem_free(tts->priv.str_buf_ready);

        tts->priv.str_buf_ready = NULL;

        }

    aos_set_event(tts->priv.event,flag,AOS_OR);
#elif 0
    ps_data_msg_send_tts(QUEUE_PS_ID_STOP);
#elif 1
    tts_stop_send_msg_timer(tts);
#endif
}


void app_tts_stop_force()
{
    struct tts_service * tts = g_tts_service;

#if 1
    tts->ops->stop((struct tts_service *) tts);
#elif 0

    //u32 flag =  BIT(EVENT_TTS_PLAY_STOP);
    //tts_queue_param_t msg;
    //msg.msg_id = BIT(EVENT_TTS_PLAY_STOP);
    //tts_data_msg_send(tts,&msg,AOS_NO_WAIT);
    if (tts->priv.timer) {
        lv_timer_pause(tts->priv.timer);

        if (tts->priv.str_buf_ready)
            lv_mem_free(tts->priv.str_buf_ready);

        tts->priv.str_buf_ready = NULL;

        }

    tts->ops->stop(tts);
#elif 1
    ps_data_msg_send_tts(QUEUE_PS_ID_STOP);
#endif
}


uint32_t app_tts_get_vol(uint8_t * str_buf)
{
    struct tts_service * tts = g_tts_service;
    return tts->ops->getvol((struct tts_service *) tts);
}


uint32_t app_tts_set_vol(uint32_t vol)
{
    struct tts_service * tts = g_tts_service;
    return tts->ops->setvol((struct tts_service *) tts,vol);
}


uint32_t app_tts_set_notify_fun(tts_notify_fun_user_t fun,void * user_data)
{
    notify_fun_user     = fun;
    notify_user_data    = user_data;

    return 0;
}

void app_tts_get_place(uint8_t * num_buf,uint32_t num_len)
{

    ps_queue_param_t msg;
    uint8_t *       num_buf1;


    num_buf1            = lv_mem_alloc(num_len + 1);

    if (num_buf1 == NULL)
        return;

    strncpy(num_buf1,num_buf,num_len);
    num_buf1[num_len]   = 0;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.msg_id          = QUEUE_PS_ID_CALL_PLACE_LOOKUP;
    msg.p_param         = num_buf1;
    msg.p_param         = (void *) num_len;
    ps_data_msg_send(&msg,AOS_NO_WAIT);
}

uint32_t app_fp_record_get_vol()
{

    //audio_set_master_volume(tts_app_param.vol_music);
    return tts_app_param.vol_record;
}

uint32_t app_fp_record_set_vol(uint8_t vol)
{
    if (tts_app_param.vol_record == vol)
        return tts_app_param.vol_record;

    tts_app_param.vol_record = vol;


    app_fp_play_tts_save_to_nv(&tts_app_param);

    return tts_app_param.vol_record;
}

#else

//static tts_app_param_t tts_app_param;
void app_tts_player_init()
{

}

void app_tts_play(uint8_t * str_buf)
{


}

void app_tts_stop(void)
{

}

void app_tts_stop_force()
{

}

uint32_t app_tts_get_vol(uint8_t * str_buf)
{


    return 0;
}


uint32_t app_tts_set_vol(uint32_t vol)
{
    return 0;
}

void app_fp_play_tts_menu(uint8_t * str)
{

}

uint32_t app_fp_play_tts_news(uint8_t * str)
{
    return 0;
}

void app_fp_play_tts_charge(uint8_t * str)
{

}

void app_fp_play_tts_low_bat(uint8_t * str)
{

}

uint32_t app_fp_play_tts_call(uint8_t * str)
{
    return 0;
}

void app_fp_play_tts_sms(uint8_t * str)
{

}

void app_fp_play_tts_phonebook(uint8_t * str)
{

}

void app_fp_play_tts_key(uint8_t * str)
{

}

uint32_t app_fp_call_set_vol(uint8_t vol)
{
    u32             out_device;
    uint32_t        ret = 0;

    audio_get_device(NULL,&out_device);

    __inf("audio_get_device:%d",out_device);

    if (out_device != 2) {
        ret                 = vol;
        }
    else {
        vol                 = vol + 5;
        ret                 = vol - 5;
        }

    audio_set_master_volume(vol);

    return ret;
}

uint32_t app_fp_call_get_vol()
{
    return audio_get_master_volume();
}

void app_fp_play_tts_lock_lcd(uint8_t * str)
{

}

void app_fp_play_tts_once_time(uint8_t * str)
{

}

uint32_t app_tts_set_notify_fun(tts_notify_fun_user_t fun,void * user_data)
{
    return 0;
}

void reset_app_fp_play_tts_param_data()
{

}

uint32_t app_fp_record_get_vol()
{
    return audio_get_master_volume();

    ;
}

uint32_t app_fp_record_set_vol(uint8_t vol)
{
    audio_set_master_volume(vol);

    return vol;
}

uint32_t get_app_fp_play_tts_news_is_enable()
{
    return 0;
}

#endif


#ifdef CONFIG_AUDIO_TTS

uint8_t return_tts_play_status(void)
{
    struct tts_service * tts = g_tts_service;

    if (TTS_IDLE == tts->ops->getstatus(tts))
        return 0;

    else 
        return 1;
}

//play_type 0:普通播放，
void fp_play_tts(uint8_t * str,uint32_t play_type) //src urf8格式
{
    uint32_t        str_len;
    uint8_t *       unicode,*gb2312str;
    uint32_t        pos;

    if (str == NULL)
        return;

    str_len             = strlen(str) * 2 + 2;
    unicode             = lv_mem_alloc(str_len);
    gb2312str           = lv_mem_alloc(str_len);

    if ((unicode == NULL) || (gb2312str == NULL)) {
        if (unicode)
            lv_mem_free(unicode);

        if (gb2312str)
            lv_mem_free(gb2312str);

        return;
        }

    memset(unicode,0,str_len);
    memset(gb2312str,0,str_len);
    chset_utf8_to_ucs2_string(unicode,str_len,str);
    chset_ucs2_to_text_str_ex(gb2312str,str_len,unicode,1,(unsigned int *) (&pos));

    if (play_type == tts_app_on_off_call)
        app_tts_play_call(gb2312str);
    else if (play_type == tts_app_on_off_news)
        app_tts_play_new(gb2312str);
    else 
        app_tts_play(gb2312str);

    __inf("fp_ui_focus_event_actions:%s",str);


    if (unicode)
        lv_mem_free(unicode);

    //if(gb2312str)
    //	lv_mem_free(gb2312str);
}

uint32_t app_fp_is_other_audio_play()
{
    if (app_musicplayer_get_play_status())
        return 1;

    if (call_fp_flow_is_call_status())
        return 1;

    if (get_fp_call_relesed_now_status())
        return 1;

    if (get_ring_play_status())
        return 1;

    return 0;
}

void app_fp_play_tts(uint8_t * str,uint8_t type)
{
    uint32_t        on_off;

    if (get_incoming_call_volume() == 0x80) // 静音模式
        return;

    if (app_fp_is_other_audio_play())
        return;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_all);

    if (on_off == 0)
        return;

    switch (type)
    {
        case tts_app_on_off_key: //按键报号
        case tts_app_on_off_menu: //菜单朗读
        case tts_app_on_off_time_auto: //整点报时
        case tts_app_on_off_once_time: //一键报时
        case tts_app_on_off_call: //来电报号
        case tts_app_on_off_phonebook: //电话本朗读
        case tts_app_on_off_news: //新短信提醒
        case tts_app_on_off_sms: //短选信息朗读
        case tts_app_on_off_charge: //从电器插拔提醒
        case tts_app_on_off_low_bat: //低电量提醒
        case tts_app_on_off_lock_lcd: //解锁屏提醒
            on_off = tts_app_param.on_off & BIT(type);
            break;

        default:
            on_off = 0;
        }

    if (on_off == 0)
        return;

    app_fp_play_tts_get_vol();

    fp_play_tts(str,type);
}


void app_fp_play_tts_menu(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_menu);
}


void app_fp_play_tts_time_auto(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_time_auto);
}


void app_fp_play_tts_once_time(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_once_time);
}


uint32_t app_fp_play_tts_call(uint8_t * str)
{
    uint32_t        on_off;

    if (get_incoming_call_volume() == 0x80) // 静音模式
        return 1;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_all);

    if (on_off == 0)
        return 0;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_call);

    if (on_off == 0)
        return 0;

    fp_play_tts(str,tts_app_on_off_call);

    return 1;
}


void app_fp_play_tts_sms(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_sms);
}


void app_fp_play_tts_phonebook(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_phonebook);
}


void app_fp_play_tts_key(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_key);
}

uint32_t get_app_fp_play_tts_news_is_enable()
{
    uint32_t        on_off;

    if (get_incoming_call_volume() == 0x80) // 静音模式
        return 0;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_all);

    if (on_off == 0)
        return 0;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_news);

    if (on_off == 0)
        return 0;

    return 1;
}

uint32_t app_fp_play_tts_news(uint8_t * str)
{
    uint32_t        on_off;

    if (get_incoming_call_volume() == 0x80) // 静音模式
        return 1;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_all);

    if (on_off == 0)
        return 0;

    on_off              = tts_app_param.on_off & BIT(tts_app_on_off_news);

    if (on_off == 0)
        return 0;


    fp_play_tts(str,tts_app_on_off_news);

    //app_fp_play_tts(str,tts_app_on_off_news);
    return 1;
}


void app_fp_play_tts_charge(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_charge);
}


void app_fp_play_tts_low_bat(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_low_bat);
}


void app_fp_play_tts_lock_lcd(uint8_t * str)
{
    app_fp_play_tts(str,tts_app_on_off_lock_lcd);
}



uint32_t app_fp_play_tts_enable_on_off(uint8_t type)
{
    uint32_t        on_off;

    on_off              = tts_app_param.on_off;
    tts_app_param.on_off = on_off | BIT(type);

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return tts_app_param.on_off;
}


uint32_t app_fp_play_tts_disable_on_off(uint8_t type)
{
    uint32_t        on_off;

    on_off              = tts_app_param.on_off;
    tts_app_param.on_off = on_off & (~(BIT(type)));

    app_fp_play_tts_save_to_nv(&tts_app_param);
    return tts_app_param.on_off;
}


uint32_t app_fp_play_tts_get_on_off(uint8_t type)
{
    uint32_t        on_off;

    on_off              = tts_app_param.on_off & BIT(type);

    return on_off;
}


uint32_t app_fp_play_tts_get_vol()
{

    audio_set_master_volume(tts_app_param.vol);

    return tts_app_param.vol;
}


uint32_t app_fp_play_tts_set_vol(uint8_t vol)
{
    tts_app_param.vol   = vol;

    audio_set_master_volume(vol);

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return tts_app_param.vol;
}

uint32_t app_fp_play_tts_get_vol1()
{


    return tts_app_param.vol;
}

uint32_t app_fp_play_tts_set_vol1(uint8_t vol)
{
    tts_app_param.vol   = vol;

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return tts_app_param.vol;
}


uint32_t app_fp_call_get_vol()
{

    //audio_set_master_volume(tts_app_param.vol_call);
    return tts_app_param.vol_call;
}


uint32_t app_fp_call_set_vol(uint8_t vol)
{
    u32             out_device;

    if (tts_app_param.vol_call != vol) {
        tts_app_param.vol_call = vol;
        app_fp_play_tts_save_to_nv(&tts_app_param);
        }

    audio_get_device(NULL,&out_device);

    __inf("audio_get_device:%d",out_device);

    if (out_device != 2) {

        }
    else {
        vol                 = vol + 5;
        }

    audio_set_master_volume(vol);


    return tts_app_param.vol_call;
}


uint32_t app_fp_music_get_vol()
{

    //audio_set_master_volume(tts_app_param.vol_music);
    return tts_app_param.vol_music;
}


uint32_t app_fp_music_set_vol(uint8_t vol)
{
    tts_app_param.vol_music = vol;

    audio_set_master_volume(vol);

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return tts_app_param.vol_music;
}


uint32_t app_fp_play_tts_set_time_auto(uint16_t start_time,uint16_t end_time)
{
    tts_app_param.time_auto_start = start_time;
    tts_app_param.time_auto_end = end_time;

    tts_app_param.time_auto_mode = 4;
    app_fp_play_tts_enable_on_off(tts_app_on_off_time_auto);
    app_fp_play_tts_time_auto_enable();

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return 0;
}


uint32_t app_fp_play_tts_set_time_mode(uint8_t mode) //1:白天模式，2：夜晚模式，3：全天模式，4：自定义模式
{
    switch (mode)
    {
        case 1:
            tts_app_param.time_auto_start = 8 * 60;
            tts_app_param.time_auto_end = 18 * 60;
            break;

        case 2:
            tts_app_param.time_auto_start = 18 * 60;
            tts_app_param.time_auto_end = 23 * 60;
            break;

        case 3:
            tts_app_param.time_auto_start = 0;
            tts_app_param.time_auto_end = 0;
            break;

        case 4:
            break;

        default:
            mode = 0;
            break;
        }

    tts_app_param.time_auto_mode = mode;

    if (mode) {
        app_fp_play_tts_enable_on_off(tts_app_on_off_time_auto);
        app_fp_play_tts_time_auto_enable();
        }
    else {
        app_fp_play_tts_disable_on_off(tts_app_on_off_time_auto);
        app_fp_play_tts_time_auto_disable();

        }

    app_fp_play_tts_save_to_nv(&tts_app_param);

    return 0;
}


uint32_t app_fp_play_tts_get_time_auto_mode()
{
    return tts_app_param.time_auto_mode;
}


uint16_t app_fp_play_tts_get_time_auto_start()
{
    return tts_app_param.time_auto_start;
}


uint16_t app_fp_play_tts_get_time_auto_end()
{
    return tts_app_param.time_auto_end;
}


static int32_t app_fp_play_tts_time_auto_action(void * p_param,uint32_t size)
{
    aos_systime     cur_time = {
        0
        };
    uint8_t         str_buf[50];

    memset(str_buf,0,50);
    aos_get_systime(&cur_time);
    sprintf(str_buf,"现在是北京时间%02d:%02d",cur_time.hour,cur_time.minute);
    app_fp_play_tts_time_auto(str_buf);

    app_fp_play_tts_time_auto_enable();

    return 0;
}


void on_alarm_expiring_tts_time(uint16_t index,uint32_t * user_data)
{

    app_msg_send_callback(app_fp_play_tts_time_auto_action,NULL,0);

}



void app_fp_play_tts_time_auto_enable()
{
    alarm_manager_time_t cur_time,incrase_time,alarm_time;

    if (tts_app_param.time_auto_mode == 0)
        return;

    get_rtc_time_alarm(&cur_time);
    memset(&incrase_time,0,sizeof(alarm_manager_time_t));

#if 1
    cur_time.minute     = 0;
    cur_time.second     = 0;

    if (tts_app_param.time_auto_start == tts_app_param.time_auto_end) {
        incrase_time.hour   = 1;
        }
    else if (tts_app_param.time_auto_start > tts_app_param.time_auto_end) {
        if (cur_time.hour < tts_app_param.time_auto_end / 60) {
            incrase_time.hour   = 1;
            }
        else if (cur_time.hour >= tts_app_param.time_auto_start / 60) {
            incrase_time.hour   = 1;
            }
        else {
            incrase_time.hour   = tts_app_param.time_auto_start / 60 - incrase_time.hour;
            }
        }
    else {
        if (cur_time.hour < tts_app_param.time_auto_start / 60) {
            incrase_time.hour   = tts_app_param.time_auto_start / 60 - incrase_time.hour;
            }
        else if (cur_time.hour >= tts_app_param.time_auto_end / 60) {
            incrase_time.hour   = tts_app_param.time_auto_start / 60 - incrase_time.hour;
            }
        else {
            incrase_time.hour   = 1;
            }
        }

#else

    incrase_time.minute = 2;
#endif

    dt_increase_time(&cur_time,&incrase_time,&alarm_time);

    add_alarm(ALM_TYPE_CUSTOM,0,&alarm_time,on_alarm_expiring_tts_time,NULL);

}


void app_fp_play_tts_time_auto_disable()
{
    alarm_manager_time_t alarm_time;

    cancel_alarm(ALM_TYPE_CUSTOM,0,&alarm_time,on_alarm_expiring_tts_time,NULL);
}

#endif

