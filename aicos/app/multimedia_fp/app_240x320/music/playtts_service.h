

#ifdef CONFIG_AUDIO_TTS

#ifndef __PLAYTTSSERVICE_H__
#define __PLAYTTSSERVICE_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include "aic_type.h"
#include "audio_system.h"
#include "audiofocus.h"
#include "aic_list.h"


    enum tts_event {
        EVENT_TTS_PLAY_FINISH = 0,
        EVENT_TTS_PLAY_BREAK = 1,
        EVENT_TTS_SERV_STARTED,
        EVENT_TTS_PLAY_START,
        EVENT_TTS_PLAY_STOP,
        EVENT_TTS_PLAY_QUEUE,
        };


    enum tts_client_cb_event {
        /* PlayUI sync */
        CLIENT_EVENT_TTS_PLAY_NEXT = 0,

        /* Error occrured, e.g. file read err */
        CLIENT_EVENT_TTS_PLAY_ERR = 1,
        CLIENT_EVENT_TTS_PLAY_FINISH,
    CLIENT_EVENT_TTS_PLAY_TIME_SYNC, /*Resv*/
    CLIENT_EVENT_TTS_PLAY_PLASE,
        };


    typedef void(*tts_service_notify_t) (enum tts_client_cb_event event,
        void * appdata);


    enum tts_status {
        TTS_IDLE,
        TTS_PLAYING,
        TTS_PAUSE,
        TTS_STOP,
        };


    struct tts_service_priv {
        enum tts_status status;
        tts_service_notify_t appnotify;
        void *          appdata;
        ACLIENT         ahandle;
        aos_thread_id   tid;
        aos_event_ptr   event;
        aos_queue_ptr   queue;
        void *          timer;
        uint8_t *       str_buf;
        uint8_t *       str_buf_ready;
        uint32_t msg_id;
        };


    struct tts_service {
        // struct app_service base;

        /* ops will used by app, and exec in app's thread */
        struct tts_service_ops * ops;
        struct tts_service_priv priv;
        };


    struct tts_service_ops {
        u32(*play) (struct tts_service * tts,uint8_t * str_buf);
        u32(*next) (struct tts_service * tts);
        u32(*stop) (struct tts_service * tts);
        u32(*getvol) (struct tts_service * tts);
        u32(*setvol) (struct tts_service * tts,u32 level);

        void(*regcb) (struct tts_service * tts,enum app_id id,
            tts_service_notify_t cb,void * appdata);

        /*unreg when app exit, background play will not notify app after unreg*/
        void(*unregcb) (struct tts_service * tts,enum app_id id);

        /* active or idle */
        enum tts_status(*getstatus) (struct tts_service * tts);
        };


    typedef struct {
        uint32_t        msg_id;
        void *          p_param;
        uint32_t        size;
        } tts_queue_param_t;


#define     TTS_CFG_SERVER_QUEUE_NUM (5)

typedef void (*tts_place_callback_t)(uint8_t*place, void *appdata);
struct tts_service * tts_play_service_create(void);


int32_t tts_data_msg_send_play(struct tts_service * tts,uint8_t * str_buf,uint32_t str_buf_size);
uint32_t app_set_place_callback(tts_place_callback_t fun,void * user_data);

#ifdef __cplusplus
} /* extern "C" */


#endif

#endif

#endif /* __PLAYSERVICE_H__ */

