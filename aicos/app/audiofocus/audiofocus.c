/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the audio focus service.
 */

#include "audiofocus.h"
#include "af_ctx_matrix_cfg.h"
#include "audio_system.h"
#include "aic_list.h"

struct af_client {
    struct list_head list;
    enum app_id id;
    enum af_audio_context ctx;
    af_notify_t notify;
    void *appdata;
};

struct af_service {
    struct list_head *clients;
    struct af_client *active;
    enum af_audio_context cur_ctx;
    aos_mutex_ptr mutex;
};

/* Default Status */
static LIST_HEAD(s_af_clients_list);
static struct af_service s_af_service = {.clients = &s_af_clients_list,
                                         .active = NULL,
                                         .cur_ctx = AF_CTX_IDLE,
                                         .mutex = NULL};

static void af_default_cb(enum af_event af_event, void *appdata)
{
    if (AF_EVENT_FOCUS_LOSS == af_event) {
        /* audio app without focus callback should give its AHANDLE */
        if (audio_stop((ACLIENT)appdata)) {
            app_loge("audio focus default cb stop audio failed.\n");
            return;
        }
        if (audio_close((ACLIENT)appdata)) {
            app_loge("audio focus default cb close audio failed.\n");
            return;
        }
    } else {
        app_loge("audio focus default cb UNSPPORT Events.\n");
    }
}

u32 audio_focus_request(enum app_id id, enum af_audio_context ctx,
                        af_notify_t notify, void *appdata)
{
    u32 ret;
    enum af_context_action action;
    enum af_event event;
    struct af_client *client;

    if (NULL == s_af_service.mutex) {
        s_af_service.mutex = aos_create_mutex("audio_focus_mutex");
        if (NULL == s_af_service.mutex) {
            app_loge("audio focus create mutex failed.\n");
            return AOS_ERROR;
        }
    }
    aos_get_mutex(s_af_service.mutex, AOS_WAIT_FOREVER);

    action = s_af_context_matrix[s_af_service.cur_ctx][ctx];

    if (AF_ACT_REJECT == action || AF_ACT_INVALID == action) {
        app_logi("audio focus request forbid.\n");
        aos_put_mutex(s_af_service.mutex);
        return AOS_NOT_AVAILABLE;
    } else {
        if (AF_ACT_EXCLUSIVE == action) {
            /* TODO: later may distinguish loos and loos transient */
            event = AF_EVENT_FOCUS_LOSS;

            client = s_af_service.active;
            if (NULL != client && client->notify == NULL) {
                af_default_cb(event, client->appdata);
                aic_free(client);
            }

            if (NULL != client && client->notify != NULL) {
                client->notify(event, client->appdata);
                list_add_tail(&client->list, s_af_service.clients);
            }

            client = (struct af_client *)aic_malloc(sizeof(struct af_client));
            if (NULL == client) {
                app_loge("audio focus malloc client failed.\n");
                aos_put_mutex(s_af_service.mutex);
                return AOS_ERROR;
            }
            client->id = id;
            client->ctx = ctx;
            client->notify = notify;
            client->appdata = appdata;

            s_af_service.active = client;
            s_af_service.cur_ctx = ctx;

            app_logi("audio focus request success(EXCLUSIVE).\n");
            ret = AOS_SUCCESS;
        } else if (AF_ACT_CONCURRENT == action) {
            /*Not support now*/
            ret = AOS_ERROR;
            app_loge("audio focus CONCURRENT not support now.\n");
        } else {
            ret = AOS_ERROR;
            app_loge("audio focus error action.\n");
        }
    }

    aos_put_mutex(s_af_service.mutex);
    return ret;
}

u32 audio_focus_release(enum app_id id)
{
    struct af_client *client, *waiting = NULL, *tmp;

    aos_get_mutex(s_af_service.mutex, AOS_WAIT_FOREVER);

    if (id == s_af_service.active->id) {
        if (!list_empty(s_af_service.clients)) {
            waiting =
                list_last_entry(s_af_service.clients, struct af_client, list);
        }
        if (NULL != waiting) {
            if (waiting->notify) {
                waiting->notify(AF_EVENT_FOCUS_GAIN, waiting->appdata);
            }
            list_del(&waiting->list);
            s_af_service.active = waiting;
            s_af_service.cur_ctx = waiting->ctx;
        } else {
            aic_free(s_af_service.active);
            s_af_service.active = NULL;
            s_af_service.cur_ctx = AF_CTX_IDLE;
        }
    } else {
        list_for_each_entry_safe(client, tmp, s_af_service.clients, list,
                                 struct af_client)
        {
            if (client->id == id) {
                list_del(&client->list);
                aic_free(client);
                break;
            }
        }
    }

    aos_put_mutex(s_af_service.mutex);
    return AOS_SUCCESS;
}

/* init and deinit is not mandatory, leave here for future use */
u32 audio_focus_init(void)
{
    s_af_service.mutex = aos_create_mutex("audio_focus_mutex");
    if (NULL == s_af_service.mutex) {
        app_loge("audio focus create mutex failed.\n");
        return AOS_ERROR;
    }

    return AOS_SUCCESS;
}

u32 audio_focus_deinit(void)
{
    aos_delete_mutex(s_af_service.mutex);
    return AOS_SUCCESS;
}
