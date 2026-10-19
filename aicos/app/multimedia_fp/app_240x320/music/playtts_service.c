

#ifdef CONFIG_AUDIO_TTS
#include "playtts_service.h"
#include "aic_bitmap.h"
#include "aic_errno.h"
#include "aic_osa.h"
#include "aic_osa_ex.h"
#include "audio_system.h"
#include "audiofocus.h"
#include <time.h>
#include <stdlib.h>
#include "fp_playservice.h"
#include "musicplayer_fp_play.h"
#include "tts_app_player.h"

#include "app_log.h"

#if 1
#define __inf                   APP_LOGI

#else

#define __inf
#endif

extern uint8_t get_incoming_call_volume(void);
extern char * yt_tts_lookup_phone_utf8_871(char * strPhoneNumber,unsigned int * pStrLen);
uint32_t tts_data_msg_send(struct tts_service * ps,tts_queue_param_t * p_app_msg,uint32_t wait_option);

static uint32_t tts_audio_play_count;


static int tts_play_cb(u32 data,enum astream_cb_event event)

#if 0
{
    struct tts_service * tts = (struct tts_service *) data;
    tts_queue_param_t msg;

    if (event == ASTREAM_CB_EVENT_FINISH) {
        msg.msg_id          = EVENT_TTS_PLAY_FINISH;
        tts_data_msg_send(tts,&msg,AOS_NO_WAIT);
        }
    else if (event == ASTREAM_CB_EVENT_BREAK) {
        // Err process
        msg.msg_id          = EVENT_TTS_PLAY_BREAK;
        tts_data_msg_send(tts,&msg,AOS_NO_WAIT);
        }


    return AOS_SUCCESS;
}


#elif 0
{
    struct tts_service * tts = (struct tts_service *) data;
    u32             flag = 0;

    if (event == ASTREAM_CB_EVENT_FINISH) {
        // setup main service event to pickup next track
        flag                = BIT(EVENT_TTS_PLAY_FINISH);
        }
    else if (event == ASTREAM_CB_EVENT_BREAK) {
        // Err process
        flag                = BIT(EVENT_TTS_PLAY_BREAK);
        }

    if (flag) {
        aos_set_event(tts->priv.event,flag,AOS_OR);
        }

    return AOS_SUCCESS;
}


#elif 1
{
    ps_queue_param_t msg;

    msg.msg_type        = QUEUE_PS_TYPE_TTS;
    msg.p_param         = (void *) data;

    if (event == ASTREAM_CB_EVENT_FINISH) {
        msg.msg_id          = QUEUE_PS_ID_FINISH;
        ps_data_msg_send(&msg,AOS_NO_WAIT);
        }
    else if (event == ASTREAM_CB_EVENT_BREAK) {
        msg.msg_id          = QUEUE_PS_ID_BREAK;

        ps_data_msg_send(&msg,AOS_NO_WAIT);
        }


    return AOS_SUCCESS;
}


#endif


static u32 tts_stop(struct tts_service * tts)
{

    __inf("tts_test tts_stop");

    if (tts->priv.status != TTS_PLAYING && tts->priv.status != TTS_PAUSE && tts->priv.status != TTS_STOP) {
        app_loge("tts_audio_stop when NOT play/pause/stop\n");
        return AOS_ERROR;
        }

    if (audio_stop(tts->priv.ahandle)) {
        app_loge("ttsservice stop failed\n");
        return AOS_ERROR;
        }

    if (audio_close(tts->priv.ahandle)) {
        app_loge("ttsservice stop/close failed\n");
        return AOS_ERROR;
        }

    tts->priv.ahandle   = 0;
    tts->priv.status    = TTS_IDLE;

    if (tts->priv.str_buf) {
        lv_mem_free(tts->priv.str_buf);
        tts->priv.str_buf   = NULL;
        }

    __inf("tts_test tts_stop1");
    audio_force_out_device(0);

    return AOS_SUCCESS;
}


static u32 tts_play(struct tts_service * tts,uint8_t * str_buf)
{
    u32             ret = AOS_ERROR;

    __inf("tts_test tts_play");

    if (str_buf == NULL) {
        __inf("please input the text you want to play\n");
        return 0;
        }


    if (tts->priv.status != TTS_IDLE) {
        if (AOS_SUCCESS != tts_stop(tts)) {
            lv_mem_free(str_buf);
            return ret;
            }
        }

    //AUDIO_INPUT_FLAG_DIRECT
    tts->priv.ahandle   = audio_open(ASTREAM_TYPE_TTS,APLAY_TYPE_TTS,
        AUDIO_OUTPUT_FLAG_COMPRESS,16000,0,
        ASTREAM_FORMAT_PCM,
        ASTREAM_CHN_MONO,
        ASTREAM_DIR_PLAY,
        ASTREAM_SOURCE_DEFAULT);


    if (tts->priv.ahandle == 0) {
        app_logw("ttsttsservice start play failed ahandle\n");
        lv_mem_free(str_buf);
        return AOS_ERROR;

        }

    __inf("tts_test audio_play_tts:%d,%s",strlen(str_buf),str_buf);
    tts_audio_play_count++;
    ret                 = audio_play_tts(tts->priv.ahandle,str_buf,strlen(str_buf),tts_play_cb,tts_audio_play_count); //tts);
    tts->priv.str_buf   = str_buf;

    if (ret) {
        app_logw("ttsttsservice start play failed\n");

        if (tts->priv.str_buf) {
            lv_mem_free(tts->priv.str_buf);
            tts->priv.str_buf   = NULL;
            }

        audio_close(tts->priv.ahandle);
        return AOS_ERROR;
        }

    tts->priv.status    = TTS_PLAYING;
    __inf("tts_test tts_play1");
    audio_force_out_device(1);

    return ret;
}


static u32 tts_getvol(struct tts_service * tts)
{
    /* Use global volume now */
    return audio_get_master_volume();
}


static u32 tts_setvol(struct tts_service * tts,u32 level)
{
    /* Use global volume now */
    return audio_set_master_volume(level);
}


static

enum tts_status tts_getstatus(struct tts_service * tts) {
    return tts->priv.status;
} static void tts_regcb(struct tts_service * ps,enum app_id id,
    tts_service_notify_t cb,void * appdata)
{
    // TODO:
    if (cb) {
        ps->priv.appnotify  = cb;
        }

    if (appdata) {
        ps->priv.appdata    = appdata;
        }
}

static void tts_unreg(struct tts_service * ps,enum app_id id)
{
    // TODO:
    ps->priv.appnotify  = NULL;
    ps->priv.appdata    = NULL;
}


static

struct tts_service_ops tts_ops = {
    .play = tts_play,
    .stop = tts_stop,
    .getvol = tts_getvol,
    .setvol = tts_setvol,
    .getstatus = tts_getstatus,
    .regcb = tts_regcb,
    .unregcb = tts_unreg,
};

uint32_t tts_data_msg_recv(struct tts_service * tts,tts_queue_param_t * p_app_msg,uint32_t wait_option)
{
    if (NULL == tts->priv.queue) {
        __inf("[%s] tele_data_queue has NOT initialized !\n",__func__);
        return - 1;
        }

    if (NULL == p_app_msg) {
        __inf("[%s] the pointer of msg is NULL !\n",__func__);
        return - 1;
        }

    return aos_recv_from_queue(tts->priv.queue,(void *) p_app_msg,wait_option);
}


uint32_t tts_data_msg_send(struct tts_service * tts,tts_queue_param_t * p_app_msg,uint32_t wait_option)
{
    if (NULL == tts->priv.queue) {
        __inf("[%s] tele_data_queue has NOT initialized !\n",__func__);
        return - 1;
        }

    if (NULL == p_app_msg) {
        __inf("[%s] the pointer of msg is NULL !\n",__func__);
        return - 1;
        }

    return aos_send_to_queue(tts->priv.queue,(void *) p_app_msg,wait_option);
}


int32_t tts_data_msg_send_play(struct tts_service * tts,uint8_t * str_buf,uint32_t str_buf_size)
{
    tts_queue_param_t cb_msg = {
        0
        };
    uint32_t        msg_send_ret = 0;


    if (0 != str_buf_size) {
        if (NULL == str_buf) {
            __inf("[%s] size is %d ,but p_param is null! \n",__func__,str_buf_size);
            return - 1;
            }

        cb_msg.p_param      = aic_malloc(str_buf_size);

        if (NULL == cb_msg.p_param) {
            __inf("[%s] cb_msg.p_param malloc fail. \n",__func__);
            return - 1;
            }

        memset(cb_msg.p_param,0,str_buf_size);
        memcpy(cb_msg.p_param,str_buf,str_buf_size);
        }
    else { /* “size == 0” indicate: p_param is data itself, NOT address of pointer */
        cb_msg.p_param      = str_buf;
        }

    cb_msg.msg_id       = EVENT_TTS_PLAY_START;
    cb_msg.size         = str_buf_size;

    msg_send_ret        = tts_data_msg_send(tts,&cb_msg,AOS_NO_WAIT);

    if (0 != msg_send_ret) {
        __inf("[%s] msg_send_ret = %d. \n",__func__,msg_send_ret);

        if ((NULL != cb_msg.p_param) && (0 != str_buf_size)) {
            aic_free(cb_msg.p_param);
            }

        return - 1;
        }

    return 0;
}


void tts_play_event_start_action(struct tts_service * tts)
{
    uint32_t        ret = 0;
    tts_queue_param_t recv_msg;


    while (1) {
        ret                 = tts_data_msg_recv(tts,&recv_msg,AOS_NO_WAIT);

        if (AOS_QUEUE_EMPTY == ret) {
            return;
            }
        else {
            switch (recv_msg.msg_id)
            {
                case EVENT_TTS_PLAY_FINISH:
                    app_logi("ttsservice: get EVENT_TTS_PLAY_FINISH\n");
                    tts->ops->stop((struct tts_service *) tts);

                    if (tts->priv.appnotify) {
                        tts->priv.appnotify(EVENT_TTS_PLAY_FINISH,
                            tts->priv.appdata);
                        }

                    break;

                case EVENT_TTS_PLAY_BREAK:
                    // Play err, notify up
                    app_logw("tts_service get EVENT_PS_PLAY_BREAK error event\n");

                    // TODO: what status audio service in?
                    tts->ops->stop((struct tts_service *) tts);

                    if (tts->priv.appnotify) {
                        tts->priv.appnotify(CLIENT_EVENT_TTS_PLAY_ERR,
                            tts->priv.appdata);
                        }

                    break;

                case EVENT_TTS_PLAY_START:
                    tts->ops->play((struct tts_service *) tts,recv_msg.p_param);
                    break;

                case EVENT_TTS_PLAY_STOP:
                    tts->ops->stop((struct tts_service *) tts);
                    break;
                }
            }
        }
}


#if 0
#define TTS_SERVICE_START_WAIT_MS 200
#define TTS_SERVICE_STACK_SIZE  2048
#define TTS_SERVICE_PRI         35


struct tts_service * tts_play_service_create(void) {
    struct tts_service * tts;
    u32             flag;
    aos_status      status;

    tts = (struct tts_service *) aic_malloc(sizeof(struct tts_service));

    if (!tts) {
        app_loge("ttsservice object malloc failed\n");
        return NULL;
        }

    memset(tts,0,sizeof(sizeof(struct tts_service)));

    tts->priv.event = aos_create_event("ttsservice event");

    if (!tts->priv.event) {
        aic_free(tts);
        app_loge("ttsservice event create failed\n");
        return NULL;
        }


    tts->priv.queue = aos_create_queue("ttsservice queue",sizeof(tts_queue_param_t),TTS_CFG_SERVER_QUEUE_NUM);

    if (!tts->priv.queue) {
        aos_delete_queue(tts);
        app_loge("ttsservice event create failed\n");
        return NULL;
        }



    tts->priv.tid = aos_create_thread("ttsservice",tts_play_service,(void *) tts,
        TTS_SERVICE_STACK_SIZE,0,NULL,
        TTS_SERVICE_PRI,AOS_AUTO_START);

    status = aos_get_event(tts->priv.event,BIT(EVENT_TTS_SERV_STARTED),
        AOS_OR_CLEAR,&flag,TTS_SERVICE_START_WAIT_MS);

    if (status) {
        aos_delete_event(tts->priv.event);
        aic_free(tts);
        app_loge("ttsservice start failed\n");
        return NULL;
        }

    tts->ops = &tts_ops;
    tts->priv.status = TTS_IDLE;
    tts->priv.str_buf = NULL;
    tts->priv.timer = NULL;
    tts->priv.str_buf_ready = NULL;



    return tts;
}

#else

#define PLACE_CALL_NAME         50
static uint8_t  place_call_name[PLACE_CALL_NAME];

static tts_place_callback_t tts_place_callback;
static void *   appdata_tts_place_notify;

void tts_play_queue_action(struct tts_service * tts,ps_queue_param_t * recv_msg)
{
    switch (recv_msg->msg_id)
    {
        case QUEUE_PS_ID_FINISH:
            app_logi("ttsservice: get EVENT_TTS_PLAY_FINISH\n");

            if ((uint32_t) (recv_msg->p_param) != tts_audio_play_count)
                break;

            tts->ops->stop(tts);

            if (tts->priv.appnotify) {
                tts->priv.appnotify(EVENT_TTS_PLAY_FINISH,
                    tts->priv.appdata);
                }

            break;

        case QUEUE_PS_ID_BREAK:
            // Play err, notify up
            if ((uint32_t) (recv_msg->p_param) != tts_audio_play_count)
                break;

            app_logw("tts_service get EVENT_PS_PLAY_BREAK error event\n");

            // TODO: what status audio service in?
            tts->ops->stop(tts);

            if (tts->priv.appnotify) {
                tts->priv.appnotify(CLIENT_EVENT_TTS_PLAY_ERR,
                    tts->priv.appdata);
                }

            break;

        case QUEUE_PS_ID_PLAY:
            if(app_fp_is_other_audio_play()){
                lv_mem_free(recv_msg->p_param);
                break;
            }

            app_fp_play_tts_get_vol();
            tts->ops->play(tts,recv_msg->p_param);
            break;

        case QUEUE_PS_ID_CALL_RING:
            if(app_fp_is_other_audio_play()){
                lv_mem_free(recv_msg->p_param);
                break;
            }

            audio_set_master_volume(get_incoming_call_volume());
            tts->ops->play(tts,recv_msg->p_param);
            break;

        case QUEUE_PS_ID_NEWS:
            if(app_fp_is_other_audio_play()){
                lv_mem_free(recv_msg->p_param);
                break;
            }

            audio_set_master_volume(get_incoming_call_volume());
            tts->ops->play(tts,recv_msg->p_param);
            break;

        case QUEUE_PS_ID_STOP:
            tts->ops->stop(tts);
            break;

        case QUEUE_PS_ID_CALL_PLACE_LOOKUP:
            {
                if (tts->priv.status != TTS_IDLE) {
                    uint8_t *       place = NULL,*num;
                    uint32_t        str_len;

                    num                 = recv_msg->p_param;
                    str_len             = recv_msg->size;

                    //place               = (uint8_t *)yt_tts_lookup_phone_utf8_871(num,(unsigned int *)&str_len);
                    if (place == NULL) {
                        memset(place_call_name,0,PLACE_CALL_NAME);
                        strncpy(place_call_name,place,PLACE_CALL_NAME - 2);

                        if (tts_place_callback) {
                            tts_place_callback(place_call_name,appdata_tts_place_notify);
                            }
                        }

                    lv_mem_free(num);
                    }
                else {
                    uint32_t        ret;

                    ret                 = audio_open(ASTREAM_TYPE_TTS,
                        APLAY_TYPE_TTS,
                        AUDIO_INPUT_FLAG_DIRECT,
                        16000,
                        0,
                        ASTREAM_FORMAT_PCM,
                        ASTREAM_CHN_MONO,
                        ASTREAM_DIR_PLAY,
                        ASTREAM_SOURCE_DEFAULT);

#if 1
                    {
                        uint8_t *       place = NULL,*num;
                        uint32_t        str_len;

                        num                 = recv_msg->p_param;
                        str_len             = recv_msg->size;

                        //place               = (uint8_t *)yt_tts_lookup_phone_utf8_871(num,(unsigned int *)&str_len);
                        if (place != NULL) {
                            memset(place_call_name,0,PLACE_CALL_NAME);
                            strncpy(place_call_name,place,PLACE_CALL_NAME - 2);

                            if (tts_place_callback) {
                                tts_place_callback(place_call_name,appdata_tts_place_notify);
                                }
                            }

                        lv_mem_free(num);
                        }

#else

                    int             ret_input;
                    uint8_t *       str = "测试";

                    ret_input           = yt_tts_input_text_mbcs_871(str,strlen(str),1,0);
                    __inf("yt_tts_input_text_mbcs_871:%d,",ret_input);


#endif

                    audio_close(ret);

                    }
                }
            break;

        case QUEUE_PS_ID_TTS_PLAY+QUEUE_PS_ID_PLAY:
        case QUEUE_PS_ID_TTS_PLAY+QUEUE_PS_ID_CALL_RING:
        case QUEUE_PS_ID_TTS_PLAY+QUEUE_PS_ID_NEWS:{
            uint32_t type;
            if(app_fp_is_other_audio_play()){
                lv_mem_free(recv_msg->p_param);
                break;
            }

            type = recv_msg->msg_id&0x0ffff;
            if(type==QUEUE_PS_ID_CALL_RING)
                audio_set_master_volume(get_incoming_call_volume());
            else if(type==QUEUE_PS_ID_NEWS)
                audio_set_master_volume(get_incoming_call_volume());
            else
                app_fp_play_tts_get_vol();

            tts->ops->play(tts,recv_msg->p_param);
        }
        break;

        default:
            break;
        }
}


struct tts_service * tts_play_service_create(void) {
    struct tts_service * tts;
    tts = (struct tts_service *) aic_malloc(sizeof(struct tts_service));

    if (!tts) {
        app_loge("ttsservice object malloc failed\n");
        return NULL;
        }

    memset(tts,0,sizeof(sizeof(struct tts_service)));


    tts->ops = &tts_ops;
    tts->priv.status = TTS_IDLE;
    tts->priv.str_buf = NULL;
    tts->priv.timer = NULL;
    tts->priv.str_buf_ready = NULL;

    tts_audio_play_count = 0;

    set_playsetvice_ps(tts,QUEUE_PS_TYPE_TTS);
    set_playsetvice_fun(tts_play_queue_action,QUEUE_PS_TYPE_TTS);

    tts_place_callback = NULL;
    appdata_tts_place_notify = NULL;
    return tts;
} uint32_t app_set_place_callback(tts_place_callback_t fun,void * user_data)
{
    tts_place_callback  = fun;
    appdata_tts_place_notify = user_data;

    return 0;
}

#endif


#endif






