/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file app_audio.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <alog_api.h>
#include "app_audio.h"
#include "gpio_inc.h"
#include "aic_srv_voice.h"

/*********************
 *      DEFINES
 *********************/
#define APP_AUDIO_FILE_NAME_LEN_MAX    (255)

#define APP_AUDIO_TASK_STK_SIZE        (2048u)
#define APP_AUDIO_TASK_COM_PRIO        (35u)
#define APP_AUDIO_TASK_QUEUE_NUM       (10)
#define APP_AUDIO_TASK_QUEUE_NAME      "app audio queue"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    MSG_PLAY_BUF,              /*0*/
    MSG_PLAY_BUF_CYCLE,        /*1*/
    MSG_PLAY_BUF_STOP,         /*2*/
    MSG_PLAY_BUF_STOP_BY_CUS,  /*3*/
    MSG_PLAY_FILE,             /*4*/
    MSG_PLAY_FILE_STOP,        /*5*/
    MSG_PLAY_FILE_STOP_BY_CUS, /*6*/
    MSG_PLAY_VOICE,            /*7*/
    MSG_PLAY_VOICE_STOP,       /*8*/
    MSG_PLAY_VOICE_STOP_BY_CUS,/*9*/
    MSG_RECORD,                /*10*/
    MSG_RECORD_FINISH,         /*11*/
    MSG_RECORD_FAIL,           /*12*/
    MSG_RECORD_STOP,           /*13*/
    MSG_SET_MASTER_VOLUME,     /*14*/

    MSG_MAX
}app_audio_msg_e;

typedef struct
{
    srv_func_callback p_func_callback;
    app_audio_play_buf_param_t audio_param;
    char *p_play_buf;
    uint32_t buf_len;
    uint32_t duration;
}play_buf_param_t;

typedef struct
{
    app_audio_msg_e            msg_id;
    srv_func_callback          p_func_callback;
    app_audio_play_buf_param_t audio_param;
    char                       *p_play_buf;
    uint32_t                   buf_len;
    uint32_t                   duration;
    enum astream_format        play_file_format;
    enum astream_format        record_type;
    uint32_t                   rate;
    uint8_t                    vol;
}app_audio_msg_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static aos_queue_ptr g_app_audio_queue_ptr = NULL;

/* global variables for play buffer */
static play_buf_param_t g_play_buf_param = {0};
static ACLIENT g_play_buf_audio_handle = 0;
static aos_timer_ptr g_play_buf_timer_ptr = NULL;

/* global variables for play file */
static srv_func_callback g_play_file_cb = NULL;
static ACLIENT g_play_file_audio_handle = 0;
static char g_play_file_name[APP_AUDIO_FILE_NAME_LEN_MAX + 1] = {0};

/* global variables for play voice */
static bool g_is_voice_open = false;

/* recorder handlle */
static uint32_t g_record_handle = 0;
/* recorder start callback */
static srv_func_callback g_record_start_cb = NULL;
static char g_record_file_name[APP_AUDIO_FILE_NAME_LEN_MAX + 1] = {0};
/* vol range is 0-15 */
static uint8_t g_master_vol = 0;
static aos_mutex_ptr g_audio_mutex = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/

static s32 app_audio_lock(void)
{
    if (AOS_NULL == g_audio_mutex) {
        app_loge("app audio lock g_audio_mutex is NULL!\r\n");
        return -1;
    }
    aos_get_mutex(g_audio_mutex, AOS_WAIT_FOREVER);

    return 0;
}

static s32 app_audio_unlock(void)
{
    if (AOS_NULL == g_audio_mutex) {
        app_loge("app audio unlock g_audio_mutex is NULL!\r\n");
        return -1;
    }
    aos_put_mutex(g_audio_mutex);
    return 0;
}


static void get_master_volume( void )
{
    g_master_vol = (uint8_t)audio_get_master_volume();
    app_logi("[%s] master val = %d. ", __func__, g_master_vol);
}

static void handle_audio_set_master_volume_msg(app_audio_msg_t volume_param)
{
    int ret = 0;
    uint32_t vol = 0;
    app_logi("[%s]enter. ", __func__);

    vol = (uint32_t)volume_param.vol;

    ret = audio_set_master_volume(vol);
    app_logi("[%s] result = %d,val = %d. ", __func__, ret, vol);
    if(0 == ret) {
        g_master_vol = (uint8_t)vol;
    } else {
        app_loge("[%s]audio_set_master_volume error. ", __func__);
    }
}

/********************* recorder begin*********************/
static int record_close( void )
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s], record intterrupt enter.\n", __func__);

    if(0 < g_record_handle) {
        app_audio_msg.msg_id = MSG_RECORD_STOP;
        app_audio_msg.p_func_callback = g_record_start_cb;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_RECORD_STOP)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else {
        app_logi("[%s],recorder:g_record_handle =0,no need interrupt.\n", __func__);
    }

    return 0;
}

static int play_buf_close( void )
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s], play_buf_close intterrupt enter.\n", __func__);

    if(0 < g_play_buf_audio_handle) {
        app_audio_msg.msg_id = MSG_PLAY_BUF_STOP;
        app_audio_msg.p_func_callback = g_play_buf_param.p_func_callback;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_PLAY_BUF_STOP)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else {
        app_logi("[%s],recorder:g_play_buf_param.p_play_buf is null,no need interrupt.\n", __func__);
    }

    return 0;
}

static int play_file_close( void )
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s], play_file_close intterrupt enter.\n", __func__);

    if(0 < g_play_file_audio_handle) {
        app_audio_msg.msg_id = MSG_PLAY_FILE_STOP;
        app_audio_msg.p_func_callback = g_play_file_cb;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_PLAY_FILE_STOP)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else {
        app_logi("[%s],recorder:g_play_file_audio_handle = 0,no need interrupt.\n", __func__);
    }

    return 0;
}

static int record_callback(u32 data, enum astream_cb_event event)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    if (event == ASTREAM_CB_EVENT_FINISH) {
        app_logi("[%s],recorder:  ASTREAM_CB_EVENT_FINISH\n", __func__);
        /* send MSG_RECORD_FINISH to app audio task */
        app_audio_msg.msg_id = MSG_RECORD_FINISH;
        app_audio_msg.p_func_callback = g_record_start_cb;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_RECORD_FINISH)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else if (event == ASTREAM_CB_EVENT_BREAK) {
        app_loge("[%s],recorder:  ASTREAM_CB_EVENT_BREAK\n", __func__);
        /* send MSG_RECORD_FAIL to app audio task */
        app_audio_msg.msg_id = MSG_RECORD_FAIL;
        app_audio_msg.p_func_callback = g_record_start_cb;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_RECORD_FAIL)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else {
        app_loge("[%s],recorder:  ASTREAM_NONE\n", __func__);
    }

    return 0;
}

static void handle_audio_record_msg(app_audio_msg_t record_param)
{
    int ret = -1;

    g_record_handle = audio_open_input(APLAY_TYPE_FILE, record_param.record_type,
                                     record_param.rate, ASTREAM_CHN_MONO,
                                     ASTREAM_SOURCE_MIC, 0);
    if(0 == g_record_handle) {
        app_loge("[%s],open file fail",__func__);
        ret = -1;
        return;
    }
    app_logi("[%s],recd_handle = %d",__func__, g_record_handle);
    ret = audio_play_file(g_record_handle, g_record_file_name, record_callback, 0);
    if(0 != ret) {
        audio_close(g_record_handle);
        g_record_handle = 0;
        app_loge("[%s],record fail,ret = %d",__func__, ret);
    }
}

static void handle_audio_record_finish_msg(app_audio_msg_t record_param)
{
    app_event_audio_t recrord_result = {0};
    app_logi("[%s]enter. ", __func__);

    recrord_result.event_id = APP_EVENT_AUDIO_RECORD_FINISH;
    record_param.p_func_callback(&recrord_result, sizeof(app_event_audio_t));

    /* start callback recieve stop event */
    if((NULL != g_record_start_cb) && (g_record_start_cb != record_param.p_func_callback)) {
        g_record_start_cb(&recrord_result, sizeof(app_event_audio_t));
    }
}

static void handle_audio_record_fail_msg(app_audio_msg_t record_param)
{
    app_event_audio_t recrord_result = {0};
    app_logi("[%s]enter. ", __func__);

    recrord_result.event_id = APP_EVENT_AUDIO_RECORD_FAIL;
    record_param.p_func_callback(&recrord_result, sizeof(app_event_audio_t));
    /* record fail;audio close*/
    if (audio_close(g_record_handle)) {
        recrord_result.event_id = APP_EVENT_AUDIO_RECORD_STOP_FAIL;
        record_param.p_func_callback(&recrord_result, sizeof(app_event_audio_t));
        app_loge("app_srv_recording_stop: close failed\n");
    }

    /* start callback recieve stop event */
    if((NULL != g_record_start_cb) && (g_record_start_cb != record_param.p_func_callback)) {
        g_record_start_cb(&recrord_result, sizeof(app_event_audio_t));
    }
}

static void handle_audio_record_stop_msg(app_audio_msg_t record_param)
{
    app_event_audio_t recrord_result = {0};
    app_logi("[%s] enter.", __func__);

    if (audio_stop(g_record_handle)) {
        recrord_result.event_id = APP_EVENT_AUDIO_RECORD_STOP_FAIL;
        app_loge("app record stop failed\n");
    } else {
        app_logi("app record stop success\n");
    }

    if (audio_close(g_record_handle)) {
        recrord_result.event_id = APP_EVENT_AUDIO_RECORD_STOP_FAIL;
        app_loge("app_srv_recording_stop: close failed\n");
    } else {
        g_record_handle = 0;
        recrord_result.event_id = APP_EVENT_AUDIO_RECORD_STOP_SUC;
        app_logi("app record close success\n");
    }

    if (record_param.p_func_callback) {
        record_param.p_func_callback(&recrord_result, sizeof(app_event_audio_t));
    }
    /* record start callback recieve stop event */
    if((NULL != g_record_start_cb) && (g_record_start_cb != record_param.p_func_callback)) {
        g_record_start_cb(&recrord_result, sizeof(app_event_audio_t));
    }
}


/********************* recorder end************************/

static void clear_data(void)
{
    g_play_buf_param.p_func_callback = NULL;
    g_play_buf_param.p_play_buf = NULL;
    g_play_buf_param.buf_len = 0;
    g_play_buf_param.duration = 0;
    if (g_play_buf_timer_ptr) {
        aos_delete_timer(g_play_buf_timer_ptr);
        g_play_buf_timer_ptr = NULL;
    }
    g_play_buf_audio_handle = 0;
}

static void play_buf_timer_cb(void *p_input)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]play_buf_timer_cb arrived.\r\n", __func__);
    /* send MSG_PLAY_BUF_STOP to app audio task */
    app_audio_msg.msg_id = MSG_PLAY_BUF_STOP;
    app_audio_msg.p_func_callback = p_input;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_PLAY_BUF_CYCLE)aos_status = %d.\r\n", __func__, aos_status);
    }
}

static int audio_play_buf_cb(u32 data, enum astream_cb_event event)
{
    app_event_audio_t app_event = {0};
    int ret_callback = -1;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]event = %d.\r\n", __func__, event);
    if (ASTREAM_CB_EVENT_FINISH == event) {
        /* notify event */
        if (NULL == g_play_buf_param.p_func_callback) {
            app_logi("[%s]g_play_buf_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_SINGLE_FINISH;
            ret_callback = g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]notify APP_EVENT_AUDIO_PLAY_BUF_SINGLE_FINISH(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* send MSG_PLAY_BUF_CYCLE to app audio task */
        if (0 != g_play_buf_param.duration) {
            app_audio_msg.msg_id = MSG_PLAY_BUF_CYCLE;
            app_audio_msg.p_func_callback = g_play_buf_param.p_func_callback;
            aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
            if (AOS_SUCCESS != aos_status) {
                app_loge("[%s](send MSG_PLAY_BUF_CYCLE)aos_status = %d.\r\n", __func__, aos_status);
            }
        } else {
            /* audio buf has been played finish, and need stop in app audio task */
            app_logi("[%s]audio buf has been played finish, and need stop in app audio task.\r\n", __func__);
            /* send MSG_PLAY_BUF_STOP to app audio task */
            app_audio_msg.msg_id = MSG_PLAY_BUF_STOP;
            app_audio_msg.p_func_callback = g_play_buf_param.p_func_callback;
            aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
            if (AOS_SUCCESS != aos_status) {
                app_loge("[%s](send MSG_PLAY_BUF_STOP)aos_status = %d.\r\n", __func__, aos_status);
            }
        }
    } else if (ASTREAM_CB_EVENT_BREAK == event) {
        /* notify event */
        if (NULL == g_play_buf_param.p_func_callback) {
            app_logi("[%s]g_play_buf_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_FAIL;
            ret_callback = g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]notify APP_EVENT_AUDIO_PLAY_BUF_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]no need to handle this event(%d).\n", __func__, event);
    }

    return 0;
}

static void handle_audio_play_buf_msg(app_audio_msg_t play_buf_param)
{
    int ret_audio = -1;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    /* 1. audio open first */
    g_play_buf_audio_handle = audio_open(ASTREAM_TYPE_DEFAULT,
                                         APLAY_TYPE_BUF,
                                         play_buf_param.audio_param.flag,
                                         play_buf_param.audio_param.sample_rate,
                                         play_buf_param.audio_param.bit_rate,
                                         play_buf_param.audio_param.format,
                                         play_buf_param.audio_param.channel,
                                         ASTREAM_DIR_PLAY,
                                         ASTREAM_SOURCE_DEFAULT);
    if (0 == g_play_buf_audio_handle) {
        app_loge("[%s]audio_open return error handle(%d).\r\n", __func__, g_play_buf_audio_handle);

        /* notify event */
        if (NULL == play_buf_param.p_func_callback) {
            app_logi("[%s]audio_open fail: p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_FAIL;
            ret_callback = play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_open fail: notify APP_EVENT_AUDIO_PLAY_BUF_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
        return;
    }

    /* call audio play api */
    ret_audio = audio_play_buf(g_play_buf_audio_handle,
                               play_buf_param.p_play_buf,
                               play_buf_param.buf_len,
                               audio_play_buf_cb,
                               0);
    if (ret_audio) {
        app_logi("[%s]audio_play_buf return error(%d).\r\n", __func__, ret_audio);

        /* notify event */
        if (NULL == play_buf_param.p_func_callback) {
            app_logi("[%s]audio_play_buf fail: p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_FAIL;
            ret_callback = play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_play_buf fail: notify APP_EVENT_AUDIO_PLAY_BUF_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* if play fail, do audio close */
        ret_audio = audio_close(g_play_buf_audio_handle);
        if (ret_audio) {
            app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);
        } else {
            app_logi("[%s]audio_close success.\r\n", __func__);

            /* clear data */
            clear_data();
        }
    } else {
        /* notify event */
        if (NULL == play_buf_param.p_func_callback) {
            app_logi("[%s]audio_play_buf return ok: p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_START;
            ret_callback = play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_play_buf: notify APP_EVENT_AUDIO_PLAY_BUF_START(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* create timer */
        if (g_play_buf_timer_ptr) {
            aos_delete_timer(g_play_buf_timer_ptr);
            g_play_buf_timer_ptr = NULL;
        }
        if (0 != play_buf_param.duration) {
            g_play_buf_timer_ptr = aos_create_timer("play_buf_timer",
                                                play_buf_timer_cb,
                                                play_buf_param.p_func_callback,
                                                play_buf_param.duration,
                                                AOS_AUTO_ACTIVATE,
                                                AOS_NO_PERIOD);
        } else {
            app_loge("[%s]g_play_buf_duration is 0.\r\n", __func__);
        }
    }
}

static void handle_audio_play_buf_cycle_msg(app_audio_msg_t play_buf_param)
{
    int ret_audio = -1;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    if(g_play_buf_audio_handle <= 0) {
        app_loge("[%s]g_play_buf_audio_handle error(%d).\r\n", __func__, g_play_buf_audio_handle);
        return;
    }

    /* audio stop first */
    ret_audio = audio_stop(g_play_buf_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_stop return error(%d).\r\n", __func__, ret_audio);
        return;
    }

    /* call audio play api */
    if (NULL != g_play_buf_param.p_play_buf) {
        ret_audio = audio_play_buf(g_play_buf_audio_handle,
                                   g_play_buf_param.p_play_buf,
                                   g_play_buf_param.buf_len,
                                   audio_play_buf_cb,
                                   0);
        if (ret_audio) {
            app_logi("[%s]audio_play_buf return error(%d).\r\n", __func__, ret_audio);

            /* notify event */
            if (NULL == g_play_buf_param.p_func_callback) {
                app_logi("[%s]audio_play_buf fail: g_play_buf_param.p_func_callback is NULL.\r\n", __func__);
            } else {
                app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_FAIL;
                ret_callback = g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
                app_logi("[%s]audio_play_buf fail: notify APP_EVENT_AUDIO_PLAY_BUF_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
            }
        } else {
            app_logi("[%s]play cycle: audio_play_buf success.\r\n", __func__);
        }
    } else {
        app_loge("[%s]g_play_buf_ptr is NULL.\r\n", __func__);
    }
}

static void handle_audio_play_buf_stop_msg(app_audio_msg_t app_audio_msg)
{
    int ret_audio = 0;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    app_audio_lock();

    /* audio stop */
    ret_audio = audio_stop(g_play_buf_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_stop return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_stop: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));

            if((NULL != g_play_buf_param.p_func_callback)
             && (g_play_buf_param.p_func_callback != app_audio_msg.p_func_callback)) {
                    g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_stop: notify APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_stop success.\r\n", __func__);
    }

    /* audio close */
    ret_audio = audio_close(g_play_buf_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_close fail: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            /* send to start callback */
            if((NULL != g_play_buf_param.p_func_callback)
             && (g_play_buf_param.p_func_callback != app_audio_msg.p_func_callback)) {
                g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_close success.\r\n", __func__);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_close success: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_FINISH;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            /* send to start callback */
            if((NULL != g_play_buf_param.p_func_callback)
             && (g_play_buf_param.p_func_callback != app_audio_msg.p_func_callback)) {
                g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_BUF_FINISH(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* clear data */
        clear_data();
    }

    app_audio_unlock();
}

static void handle_audio_play_buf_stop_by_cus_msg(app_audio_msg_t play_buf_stop_param)
{
    int ret_audio = 0;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    app_audio_lock();

    /* audio stop */
    ret_audio = audio_stop(g_play_buf_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_stop return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == play_buf_stop_param.p_func_callback) {
            app_logi("[%s]audio_stop: play_buf_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL;
            ret_callback = play_buf_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if((NULL != g_play_buf_param.p_func_callback)
             && (g_play_buf_param.p_func_callback != play_buf_stop_param.p_func_callback)) {
                g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_stop: notify APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_stop success.\r\n", __func__);
    }

    /* audio close */
    ret_audio = audio_close(g_play_buf_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == play_buf_stop_param.p_func_callback) {
            app_logi("[%s]audio_close fail: play_buf_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL;
            ret_callback = play_buf_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if((NULL != g_play_buf_param.p_func_callback)
             && (g_play_buf_param.p_func_callback != play_buf_stop_param.p_func_callback)) {
                g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_BUF_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_close success.\r\n", __func__);

        /* notify client event */
        if (NULL == play_buf_stop_param.p_func_callback) {
            app_logi("[%s]audio_close success: play_buf_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_BUF_STOP_SUCC;
            ret_callback = play_buf_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));

            if((NULL != g_play_buf_param.p_func_callback)
             && g_play_buf_param.p_func_callback != play_buf_stop_param.p_func_callback) {
                g_play_buf_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_BUF_STOP_SUCC(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* clear data */
        clear_data();
    }

    app_audio_unlock();
}

static void handle_audio_play_file_stop_msg(app_audio_msg_t app_audio_msg)
{
    int ret_audio = 0;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    app_audio_lock();

    /* audio stop */
    ret_audio = audio_stop(g_play_file_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_stop return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_stop: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));

            if((NULL != g_play_file_cb) && (g_play_file_cb != app_audio_msg.p_func_callback)) {
                ret_callback = g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_stop: notify APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_stop success.\r\n", __func__);
    }

    /* audio close */
    ret_audio = audio_close(g_play_file_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_close fail: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if((NULL != g_play_file_cb) && (g_play_file_cb != app_audio_msg.p_func_callback)) {
                ret_callback = g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_close success.\r\n", __func__);

        /* notify client event */
        if (NULL == app_audio_msg.p_func_callback) {
            app_logi("[%s]audio_close success: app_audio_msg.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_FINISH;
            ret_callback = app_audio_msg.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if((NULL != g_play_file_cb) && (g_play_file_cb != app_audio_msg.p_func_callback)) {
                ret_callback = g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_FILE_FINISH(ret_callback = %d).\r\n", __func__, ret_callback);
        }
        g_play_file_audio_handle = 0;
    }
    app_audio_unlock();
}

static void handle_audio_play_file_stop_by_cus_msg(app_audio_msg_t play_file_stop_param)
{
    int ret_audio = 0;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    app_audio_lock();

    /* audio stop */
    ret_audio = audio_stop(g_play_file_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_stop return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == play_file_stop_param.p_func_callback) {
            app_logi("[%s]audio_stop: play_file_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL;
            ret_callback = play_file_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_stop: notify APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_stop success.\r\n", __func__);
    }

    /* audio close */
    ret_audio = audio_close(g_play_file_audio_handle);
    if (ret_audio) {
        app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);

        /* notify client event */
        if (NULL == play_file_stop_param.p_func_callback) {
            app_logi("[%s]audio_close fail: play_file_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL;
            ret_callback = play_file_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if((NULL != g_play_file_cb) && (g_play_file_cb != play_file_stop_param.p_func_callback)) {
                g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_FILE_STOP_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]audio_close success.\r\n", __func__);

        /* notify client event */
        if (NULL == play_file_stop_param.p_func_callback) {
            app_logi("[%s]audio_close success: play_file_stop_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_FINISH;
            ret_callback = play_file_stop_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            if(NULL != g_play_file_cb && (g_play_file_cb != play_file_stop_param.p_func_callback)) {
                g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            }
            app_logi("[%s]audio_close: notify APP_EVENT_AUDIO_PLAY_FILE_FINISH(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        g_play_file_audio_handle = 0;
    }

    app_audio_unlock();
}

static int audio_play_file_cb(u32 data, enum astream_cb_event event)
{
    app_event_audio_t app_event = {0};
    int ret_callback = -1;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]event = %d.\r\n", __func__, event);
    if (ASTREAM_CB_EVENT_FINISH == event) {
        /* send MSG_PLAY_FILE_STOP to app audio task */
        app_audio_msg.msg_id = MSG_PLAY_FILE_STOP;
        app_audio_msg.p_func_callback = g_play_file_cb;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_PLAY_FILE_STOP)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else if (ASTREAM_CB_EVENT_BREAK == event) {
        /* notify event */
        if (NULL == g_play_file_cb) {
            app_logi("[%s]g_play_file_cb is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_FAIL;
            ret_callback = g_play_file_cb((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]notify APP_EVENT_AUDIO_PLAY_FILE_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    } else {
        app_logi("[%s]no need to handle this event(%d).\n", __func__, event);
    }

    return 0;
}

static void handle_audio_play_file_msg(app_audio_msg_t play_file_param)
{
    int ret_audio = -1;
    app_event_audio_t app_event = {0};
    int ret_callback = -1;

    app_logi("[%s]play_file_param.play_file_format = %d.\r\n", __func__, play_file_param.play_file_format);

    g_play_file_audio_handle = audio_open_output(ASTREAM_TYPE_MUSIC,
                                                 APLAY_TYPE_FILE,
                                                 play_file_param.play_file_format,
                                                 0);

    if (0 == g_play_file_audio_handle) {
        app_loge("[%s]audio_open_output return error handle(%d).\r\n", __func__, g_play_file_audio_handle);

        /* notify event */
        if (NULL == play_file_param.p_func_callback) {
            app_logi("[%s]audio_open_output fail: play_file_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_FAIL;
            ret_callback = play_file_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_open_output fail: notify APP_EVENT_AUDIO_PLAY_FILE_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }
        return;
    }

    /* call audio play api */
    ret_audio = audio_play_file(g_play_file_audio_handle,
                                g_play_file_name,
                                audio_play_file_cb,
                                0);
    app_logi("[%s]g_play_file_name = %s.\r\n", __func__, g_play_file_name);
    if (ret_audio) {
        app_logi("[%s]audio_play_file return error(%d).\r\n", __func__, ret_audio);

        /* notify event */
        if (NULL == play_file_param.p_func_callback) {
            app_logi("[%s]audio_play_file fail: play_file_param.p_func_callback is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_FAIL;
            ret_callback = play_file_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_play_file fail: notify APP_EVENT_AUDIO_PLAY_FILE_FAIL(ret_callback = %d).\r\n", __func__, ret_callback);
        }

        /* if play fail, do audio close */
        ret_audio = audio_close(g_play_file_audio_handle);
        if (ret_audio) {
            app_loge("[%s]audio_close return error(%d).\r\n", __func__, ret_audio);
        } else {
            app_logi("[%s]audio_close success.\r\n", __func__);
            g_play_file_audio_handle = 0;
        }
    } else {
        /* notify event */
        if (NULL == play_file_param.p_func_callback) {
            app_logi("[%s]audio_play_file: g_play_file_cb is NULL.\r\n", __func__);
        } else {
            app_event.event_id = APP_EVENT_AUDIO_PLAY_FILE_START;
            ret_callback = play_file_param.p_func_callback((void *)&app_event, sizeof(app_event_audio_t));
            app_logi("[%s]audio_play_file: notify APP_EVENT_AUDIO_PLAY_FILE_START(ret_callback = %d).\r\n", __func__, ret_callback);
        }
    }
}

static void app_audio_task_entry(void *p_thread_input)
{
    aos_status aos_status = 0;
    app_audio_msg_t app_audio_msg = {0};

    /* get master volume value when init and set this vol to global*/
    get_master_volume();

    while (1) {
        aos_status = aos_recv_from_queue(g_app_audio_queue_ptr, (app_audio_msg_t *)&app_audio_msg, AOS_WAIT_FOREVER);
        if(AOS_QUEUE_EMPTY == aos_status) {
            continue;
        } else {
            app_logi("[%s]app_audio_msg.msg_id = %d.\r\n", __func__, app_audio_msg.msg_id);
            switch (app_audio_msg.msg_id) {
                case MSG_PLAY_BUF: {
                    app_logi("[%s],receive MSG_PLAY_BUF\n", __func__);
                    handle_audio_play_buf_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_BUF_CYCLE: {
                    /* replay the buffer audio data */
                    app_logi("[%s],receive MSG_PLAY_BUF_CYCLE\n", __func__);
                    handle_audio_play_buf_cycle_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_BUF_STOP: {
                    app_logi("[%s],receive MSG_PLAY_BUF_STOP\n", __func__);
                    handle_audio_play_buf_stop_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_BUF_STOP_BY_CUS: {
                    app_logi("[%s],receive MSG_PLAY_BUF_STOP_BY_CUS\n", __func__);
                    handle_audio_play_buf_stop_by_cus_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_FILE: {
                    app_logi("[%s],receive MSG_PLAY_FILE\n", __func__);
                    handle_audio_play_file_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_FILE_STOP: {
                    app_logi("[%s],receive MSG_PLAY_FILE_STOP\n", __func__);
                    handle_audio_play_file_stop_msg(app_audio_msg);
                    break;
                }

                case MSG_PLAY_FILE_STOP_BY_CUS: {
                    app_logi("[%s],receive MSG_PLAY_FILE_STOP_BY_CUS\n", __func__);
                    handle_audio_play_file_stop_by_cus_msg(app_audio_msg);
                    break;
                }

                /* recorder part */
                case MSG_RECORD: {
                    app_logi("[%s],receive EVENT_RECORD\n", __func__);
                    handle_audio_record_msg(app_audio_msg);
                    break;
                }

                case MSG_RECORD_FINISH: {
                    app_logi("[%s],receive MSG_RECORD_FINISH\n", __func__);
                    handle_audio_record_finish_msg(app_audio_msg);
                    break;
                }

                case MSG_RECORD_FAIL: {
                    app_logi("[%s],receive MSG_RECORD_FAIL\n", __func__);
                    handle_audio_record_fail_msg(app_audio_msg);
                    break;
                }

                case MSG_RECORD_STOP: {
                    app_logi("[%s],receive MSG_RECORD_STOP \n", __func__);
                    handle_audio_record_stop_msg(app_audio_msg);
                    break;
                }

                case MSG_SET_MASTER_VOLUME: {
                    app_logi("[%s],receive MSG_SET_MASTER_VOLUME \n", __func__);
                    handle_audio_set_master_volume_msg(app_audio_msg);
                    break;
                }

                default:
                    break;
            }
        }
    }
}

static int app_audio_close_all_sync(void)
{
    int ret = 0;
    app_audio_msg_t app_audio_msg = {0};

    app_logi("[%s]enter.\r\n", __func__);

    /* 1. stop play buffer */
    if (0 < g_play_buf_audio_handle) {
        app_audio_msg.msg_id = MSG_PLAY_BUF_STOP;
        app_audio_msg.p_func_callback = g_play_buf_param.p_func_callback;
        /* call process directly */
        handle_audio_play_buf_stop_msg(app_audio_msg);
    }

    /* 2. stop play file */
    if (0 < g_play_file_audio_handle) {
        app_audio_msg.msg_id = MSG_PLAY_FILE_STOP;
        app_audio_msg.p_func_callback = g_play_file_cb;
        /* call process directly */
        handle_audio_play_file_stop_msg(app_audio_msg);
    }

    /* 3. stop record */
    if (0 < g_record_handle) {
        app_audio_msg.msg_id = MSG_RECORD_STOP;
        app_audio_msg.p_func_callback = g_record_start_cb;
        /* call process directly */
        handle_audio_record_stop_msg(app_audio_msg);
    }
    return ret;
}

static int32_t handle_voice_event(void *p_param, uint32_t size)
{
    vs_event_t *p_vs_event = NULL;

    if(NULL == p_param) {
        app_loge("[%s],p_param is null",__func__);
        return -1;
    }

    p_vs_event = (vs_event_t *)p_param;

    switch (p_vs_event->e_event) {
        case VS_EVENT_PRE_START: {
            /* close all audio */
            app_audio_close_all_sync();
            break;
        }
        default:
            app_logi("[%s] default:p_vs_event->vs_event_id = %d", __func__, p_vs_event->e_event);
            break;
    }
    return 0;
}

static int app_audio_init(void)
{
    aic_srv_bus_info_t voice_bus_info = {0};

    /* Initial app audio msg queue. */
    g_app_audio_queue_ptr = aos_create_queue(APP_AUDIO_TASK_QUEUE_NAME,
                                             sizeof(app_audio_msg_t),
                                             APP_AUDIO_TASK_QUEUE_NUM);

    g_audio_mutex = aos_create_mutex("app_audio");
    if (AOS_NULL == g_audio_mutex) {
        app_loge("g_audio_mutex is NULL!\r\n");
    }

    aos_create_thread(
        "app_audio_task",
        app_audio_task_entry,
        NULL,
        APP_AUDIO_TASK_STK_SIZE,
        0,
        NULL,
        APP_AUDIO_TASK_COM_PRIO,
        AOS_AUTO_START);

    voice_bus_info.p_client_name = "app_audio";

    aic_srv_voice_register(&voice_bus_info, handle_voice_event);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(app_audio_init);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int app_audio_play_buf_start(const char *p_buf, uint32_t buf_len, uint32_t duration, app_audio_play_buf_param_t *p_buf_para, srv_func_callback _buf_start_callback)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    /* param check */
    if (NULL == p_buf) {
        app_loge("[%s]p_buf is NULL.\r\n", __func__);
        return -1;
    }

    if (0 == buf_len) {
        app_loge("[%s]buf_len is 0.\r\n", __func__);
        return -1;
    }

    if (NULL == p_buf_para) {
        app_loge("[%s]p_buf_para is NULL.\r\n", __func__);
        return -1;
    }

    app_logi("[%s]buf_len = %d, duration = %d, flag = %d, sample_rate = %d, bit_rate = %d, format = %d, channel = %d.\r\n",
             __func__,
             buf_len,
             duration,
             p_buf_para->flag,
             p_buf_para->sample_rate,
             p_buf_para->bit_rate,
             p_buf_para->format,
             p_buf_para->channel);

    if(true == g_is_voice_open) {
        app_loge("[%s]g_is_voice_open can't play buf.\r\n", __func__);
        return -1;
    }

     /* send MSG_RECORD_STOP to app audio task */
     record_close();

     /* send MSG_PLAY_FILE_STOP to app audio task */
    play_file_close();

    /* save to global variable */
    g_play_buf_param.p_func_callback = _buf_start_callback;
    g_play_buf_param.p_play_buf = (char *)p_buf;
    g_play_buf_param.buf_len = buf_len;
    g_play_buf_param.duration = duration;

    /* 3. send event MSG_PLAY_BUF to app audio task */
    memset(&app_audio_msg, 0, sizeof(app_audio_msg_t));
    app_audio_msg.msg_id = MSG_PLAY_BUF;
    app_audio_msg.p_func_callback = _buf_start_callback;
    app_audio_msg.p_play_buf = (char *)p_buf;
    app_audio_msg.buf_len = buf_len;
    app_audio_msg.duration = duration;
    memcpy(&(app_audio_msg.audio_param), p_buf_para, sizeof(app_audio_play_buf_param_t));
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_PLAY_BUF)aos_status = %d.\r\n", __func__, aos_status);
    }

    app_logi("[%s]app_audio_play_buf_start success.\r\n", __func__);
    return 0;
}

int app_audio_play_buf_stop(srv_func_callback _buf_stop_callback)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]enter.(_buf_stop_callback = %p)\r\n", __func__, _buf_stop_callback);

    if(0 < g_play_buf_audio_handle) {
        /* send EVENT_PLAY_BUF_STOP_BY_CUS to app audio task */
        app_audio_msg.msg_id = MSG_PLAY_BUF_STOP_BY_CUS;
        app_audio_msg.p_func_callback = _buf_stop_callback;
        aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
        if (AOS_SUCCESS != aos_status) {
            app_loge("[%s](send MSG_PLAY_BUF_STOP_BY_CUS)aos_status = %d.\r\n", __func__, aos_status);
        }
    } else {
        app_logi("[%s],recorder:g_play_buf_param.p_play_buf is null,no need interrupt.\n", __func__);
    }

    return 0;
}

int app_audio_play_file_start(char *p_file_name, srv_func_callback _file_start_callback)
{
    char *p_file_ext = NULL;
    enum astream_format format = ASTREAM_FORMAT_INVALID;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    /* param check */
    if (NULL == p_file_name) {
        app_loge("[%s]p_file_name is NULL.\r\n", __func__);
        return -1;
    }

    app_logi("[%s]p_file_name = %s.\r\n", __func__, p_file_name);

    /* TODO: handle repeat call logic */
    if (NULL != g_play_file_cb) {
        app_logi("[%s]g_play_file_cb(%p) has not been handled finished.\r\n", __func__, g_play_file_cb);
    }

    if(true == g_is_voice_open) {
        app_loge("[%s]g_is_voice_open can't play buf.\r\n", __func__);
        return -1;
    }

    g_play_file_cb = _file_start_callback;

    /* send EVENT_RECORD_STOP to app audio task */
    record_close();

    /* send EVENT_PLAY_BUF_STOP to app audio task */
    play_buf_close();

    /* audio open first */
    /* according file extension to open right format */
    p_file_ext = strrchr(p_file_name, '.');
    if (p_file_ext) {
        p_file_ext++;
        if (strcmp(p_file_ext, "mp3") == 0 || strcmp(p_file_ext, "MP3") == 0) {
            format = ASTREAM_FORMAT_MP3;
        } else if (strcmp(p_file_ext, "wav") == 0 || strcmp(p_file_ext, "WAV") == 0) {
            format = ASTREAM_FORMAT_WAV;
        } else if (strcmp(p_file_ext, "amr") == 0 || strcmp(p_file_ext, "AMR") == 0) {
            // TODO: NB and WB
            format = ASTREAM_FORMAT_AMR_NB;
        } else {
            format = ASTREAM_FORMAT_INVALID;
        }
    } else {
        format = ASTREAM_FORMAT_INVALID;
    }
    app_logi("[%s]format = %d.\r\n", __func__, format);

    /* send event MSG_PLAY_FILE to app audio task */
    memset(&app_audio_msg, 0, sizeof(app_audio_msg_t));
    memset(g_play_file_name, 0, APP_AUDIO_FILE_NAME_LEN_MAX + 1);
    memcpy(g_play_file_name, p_file_name, strlen(p_file_name));
    app_audio_msg.msg_id = MSG_PLAY_FILE;
    app_audio_msg.play_file_format = format;
    app_audio_msg.p_func_callback = _file_start_callback;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_PLAY_FILE)aos_status = %d.\r\n", __func__, aos_status);
    }

    app_logi("[%s]app_audio_play_file_start success.\r\n", __func__);
    return 0;
}

int app_audio_play_file_start_with_format(char *p_file_name, app_audio_play_file_format_e format, srv_func_callback _file_start_callback)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    /* param check */
    if (NULL == p_file_name) {
        app_loge("[%s]p_file_name is NULL.\r\n", __func__);
        return -1;
    }

    app_logi("[%s]p_file_name = %s.\r\n", __func__, p_file_name);

    /* TODO: handle repeat call logic */
    if (NULL != g_play_file_cb) {
        app_logi("[%s]g_play_file_cb(%p) has not been handled finished.\r\n", __func__, g_play_file_cb);
    }

    g_play_file_cb = _file_start_callback;

    if(true == g_is_voice_open) {
        app_loge("[%s]g_is_voice_open can't play buf.\r\n", __func__);
        return -1;
    }

    /* send EVENT_PLAY_BUF_STOP to app audio task */
    play_buf_close();

    /* send EVENT_RECORD_STOP to app audio task */
    record_close();

    /* send event MSG_PLAY_FILE to app audio task */
    memset(&app_audio_msg, 0, sizeof(app_audio_msg_t));
    memset(g_play_file_name, 0, APP_AUDIO_FILE_NAME_LEN_MAX + 1);
    memcpy(g_play_file_name, p_file_name, strlen(p_file_name));
    if (APP_AUDIO_PLAY_FILE_FORMAT_AMR_WB == format) {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_AMR_WB;
    } else if (APP_AUDIO_PLAY_FILE_FORMAT_AMR_NB == format) {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_AMR_NB;
    } else if (APP_AUDIO_PLAY_FILE_FORMAT_MP3 == format) {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_MP3;
    } else if (APP_AUDIO_PLAY_FILE_FORMAT_WAV == format) {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_WAV;
    } else if (APP_AUDIO_PLAY_FILE_FORMAT_PCM == format) {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_PCM;
    } else {
        app_audio_msg.play_file_format = ASTREAM_FORMAT_INVALID;
        app_logi("[%s]format(%d) is out of range.\r\n", __func__, format);
    }
    app_logi("[%s]format = %d, app_audio_msg.play_file_format = %d.\r\n", __func__, format, app_audio_msg.play_file_format);
    app_audio_msg.msg_id = MSG_PLAY_FILE;
    app_audio_msg.p_func_callback = _file_start_callback;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_PLAY_FILE)aos_status = %d.\r\n", __func__, aos_status);
    }

    app_logi("[%s]app_audio_play_file_with_format_start success.\r\n", __func__);
    return 0;
}

int app_audio_play_file_stop(srv_func_callback _file_stop_callback)
{
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]enter.(_file_stop_callback = %p)\r\n", __func__, _file_stop_callback);

    /* send MSG_PLAY_FILE_STOP_BY_CUS to app audio task */
    app_audio_msg.msg_id = MSG_PLAY_FILE_STOP_BY_CUS;
    app_audio_msg.p_func_callback = _file_stop_callback;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_PLAY_FILE_STOP_BY_CUS)aos_status = %d.\r\n", __func__, aos_status);
    }

    return 0;
}

int app_audio_record_start(recorder_param_t *p_rec_parm, srv_func_callback _rec_start_callback)
{
    int ret = 0;
    enum astream_format record_type = ASTREAM_FORMAT_WAV;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    if(NULL == p_rec_parm) {
        app_loge("[%s],p_rec_parm is null.",__func__);
        ret= -1;
        return ret;
    }

    if(NULL == p_rec_parm->file_path) {
        app_loge("[%s],file_path is null",__func__);
        ret = -1;
        return ret;
    }

    /* if in call state, can't record */
    /* if voice record later, how to resolve*/
    if(true == g_is_voice_open) {
        app_loge("[%s], voice is open,can't record.\n", __func__);
        ret = -1;
        return ret;
    }

    /* if recording,send record stop */
    if(0 < g_record_handle) {
        app_loge("[%s], recording,can't record.\n", __func__);
        ret = -1;
        return ret;
    }
    /* stop buf & file play */
    play_buf_close();
    play_file_close();

    if(RECORD_FILE_TYPE_WAV == p_rec_parm->type) {
        record_type = ASTREAM_FORMAT_WAV;
    } else if(RECORD_FILE_TYPE_PCM == p_rec_parm->type) {
        record_type = ASTREAM_FORMAT_PCM;
    } else if(RECORD_FILE_TYPE_AMR_WB == p_rec_parm->type) {
        record_type = ASTREAM_FORMAT_AMR_WB;
    } else if(RECORD_FILE_TYPE_AMR_NB == p_rec_parm->type) {
        record_type = ASTREAM_FORMAT_AMR_NB;
    }

    g_record_start_cb = _rec_start_callback;
    app_logi("[%s],file_path = %s,type = %d,rate = %d", __func__, p_rec_parm->file_path, record_type, p_rec_parm->rate);

    /* send MSG_RECORD to app audio task */
    app_audio_msg.msg_id = MSG_RECORD;
    app_audio_msg.record_type = record_type;
    app_audio_msg.rate = p_rec_parm->rate;
    app_audio_msg.p_func_callback = _rec_start_callback;
    memset(g_record_file_name, 0, APP_AUDIO_FILE_NAME_LEN_MAX + 1);
    memcpy(g_record_file_name, p_rec_parm->file_path, strlen(p_rec_parm->file_path));
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_RECORD)aos_status = %d.\r\n", __func__, aos_status);
    }

    return ret;
}

int app_audio_record_stop(  srv_func_callback _rec_stop_callback)
{
    int ret = 0;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]enter.(_rec_stop_callback = %p)\r\n", __func__, _rec_stop_callback);

    /* send MSG_RECORD_STOP to app audio task */
    app_audio_msg.msg_id = MSG_RECORD_STOP;
    app_audio_msg.p_func_callback = _rec_stop_callback;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_RECORD_STOP)aos_status = %d.\r\n", __func__, aos_status);
    }

    return ret;
}


int app_audio_set_master_volume(uint8_t vol)
{
    int ret = 0;
    app_audio_msg_t app_audio_msg = {0};
    aos_status aos_status = 0;

    app_logi("[%s]enter. vol = %d)\r\n", __func__, vol);

    /* send MSG_SET_SYS_VOLUME to app audio task */
    app_audio_msg.msg_id = MSG_SET_MASTER_VOLUME;
    app_audio_msg.p_func_callback = NULL;
    app_audio_msg.vol = vol;
    aos_status = aos_send_to_queue(g_app_audio_queue_ptr, (void *)&app_audio_msg, AOS_NO_WAIT);
    if (AOS_SUCCESS != aos_status) {
        app_loge("[%s](send MSG_SET_MASTER_VOLUME)aos_status = %d.\r\n", __func__, aos_status);
        ret = -1;
    }

    return ret;
}

int app_audio_get_master_volume(uint8_t *p_vol)
{
    int ret = 0;

    if(NULL == p_vol) {
        app_loge("[%s],p_vol is null \r\n", __func__);
        ret = -1;
        return ret;
    }

    *p_vol = g_master_vol;
    app_logi("[%s]enter.p_vol = %d\r\n", __func__, *p_vol);

    return ret;
}

