/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio slink client manage file, client should call register
 *  API at boot time.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17f7dd
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "audio_service.h"
#include "audio_sink.h"
#include "audio_record.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_sink_record.h"
#include "audio_client.h"

#define ACLIENT_CHECK(x)    \
    if (!x) \
        return -1;

struct audio_client_mng {
    struct list_head list;
};

struct aclient_ins {
    struct audio_client *client;
    u32 client_obj;
    AHANDLE handle;
    u32 state;
    u32 cb;
    u32 cb_data;
};

struct audio_client_mng g_client_mng;

static void *audio_client_new_instance(void)
{
    struct aclient_ins *ins;

    ins = (struct aclient_ins *)audio_malloc(sizeof(*ins));
    if (!ins) {
        AIDXE(0x13f552, "", "audio client create failed, no momory\n");
        return NULL;
    }
    memset(ins, 0, sizeof(*ins));

    return ins;
}

static void audio_client_delete_instance(struct aclient_ins *ins)
{
    if (ins)
        audio_free(ins);
}

struct audio_client *audio_client_get(enum astream_type type,
                                      enum aplay_type aplay_type)
{
    struct audio_client *client;

    list_for_each_entry(client, &g_client_mng.list, list, struct audio_client) {
        switch (type) {
        case ASTREAM_TYPE_MUSIC:
        case ASTREAM_TYPE_RECORD:
        case ASTREAM_TYPE_VOIP:
            if (client->type == type && client->aplay_type == aplay_type)
                return client;
            break;
        case ASTREAM_TYPE_VOICE:
        case ASTREAM_TYPE_VOICE_UL:
#ifdef CONFIG_MCU_AUDIO
            if (client->type == ASTREAM_TYPE_VOICE)
                return client;
#else
            if (client->type == ASTREAM_TYPE_DEFAULT)
                return client;
#endif
            break;
        default:
            if (client->type == type)
                return client;
            break;
        }
    }

    return NULL;
}

u32 audio_client_open(enum astream_type type, enum aplay_type aplay_type,
                      enum astream_format format, void *msg)
{
    struct audio_client *client = audio_client_get(type, aplay_type);
    struct audio_msg_open *pmsg = msg;
    struct audio_open_rsp rsp;
    struct aclient_ins *ins;
    u32 obj = 0;
    int ret;

    if (!client) {
        AIDXE(0x1c8e60, "dd", "Not find this client, type=%d, aplay_type=%d\n",
              type, aplay_type);
        return 0;
    }

    ins = audio_client_new_instance();
    if (!ins) {
        AIDXE(0x02818d, "", "create audio instance failed\n");
        return 0;
    }

    if (client->open)
        obj = client->open(format, msg);
    if (!obj) {
        audio_free(ins);
        AIDXE(0x0f5c51, "d", "Could not open this client, format=%d\n", format);
        return 0;
    }

    pmsg->aclient = (ACLIENT)ins;
    ret = audio_bind(msg, &rsp, -1);
    if (ret || rsp.result) {
        if (client->close)
            client->close(obj);
        audio_free(ins);
        AIDXE(0x167a18, "dd", "server open failed, ret=%d, result=%d\n", ret, rsp.result);
        return 0;
    }

    ins->handle = rsp.handle;
    ins->client_obj = obj;
    ins->client = client;

    return (ACLIENT)ins;
}

int audio_client_play(ACLIENT aclient, void *msg)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_play *pmsg = msg;
    struct audio_rsp_base rsp;
    int ret;

    ACLIENT_CHECK(ins);
    if (ins->client->start) {
        ret = ins->client->start(ins->client_obj, pmsg->aplay_data);
        if (ret) {
            AIDXE(0x041d73, "d", "client play failed, ret=%d\n", ret);
            return ret;
        }
    }
    pmsg->handle = ins->handle;
    ret = audio_bind(msg, &rsp, -1);
    if (ret || rsp.result)
        AIDXE(0x0903e8, "dd", "server play failed, ret=%d, result=%d\n", ret, rsp.result);

    return !!(ret | rsp.result);
}

int audio_client_pause(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_pause msg;
    struct audio_rsp_base rsp;
    int ret = 0;

    ACLIENT_CHECK(ins);

    msg.cmd = AUDIO_CMD_PAUSE;
    msg.handle = ins->handle;
    ret = audio_bind(&msg, &rsp, -1);
    if (ret || rsp.result) {
        AIDXE(0x1b9f40, "dd", "server pause failed, ret=%d, result=%d\n", ret, rsp.result);

        return !!(ret | rsp.result);
    }
    if (ins->client && ins->client->pause) {
        ret = ins->client->pause(ins->client_obj);
        if (ret) {
            AIDXE(0x10213d, "d", "client stop failed, ret=%d\n", ret);
            return ret;
        }
    }

    return 0;
}

int audio_client_resume(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_resume msg;
    struct audio_rsp_base rsp;
    int ret = 0;

    ACLIENT_CHECK(ins);

    if (ins->client && ins->client->resume) {
        ret = ins->client->resume(ins->client_obj);
        if (ret) {
            AIDXE(0x10213d, "d", "client stop failed, ret=%d\n", ret);
            return ret;
        }
    }
    msg.cmd = AUDIO_CMD_RESUME;
    msg.handle = ins->handle;
    ret = audio_bind(&msg, &rsp, -1);
    if (ret || rsp.result)
        AIDXE(0x0cc93d, "dd", "server resume failed, ret=%d, result=%d\n", ret, rsp.result);

    return !!(ret | rsp.result);
}

int audio_client_seek(ACLIENT aclient, u32 timestamp)
{
    int ret = 0;
    struct aclient_ins *ins = (struct aclient_ins*) aclient;

    ACLIENT_CHECK(ins);
    if(ins->client && ins->client->seek) {
        ret = ins->client->seek(ins->client_obj, timestamp);
    }
    return ret;
}

int audio_client_stop(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_stop msg;
    struct audio_rsp_base rsp;
    int ret;

    ACLIENT_CHECK(ins);
    if (ins->client->pre_stop) {
        ret = ins->client->pre_stop(ins->client_obj);
        if (ret) {
            AIDXE(0x0b4ec9, "d", "client pre_stop failed, ret=%d\n", ret);
            return ret;
        }
    }

    msg.cmd = AUDIO_CMD_STOP;
    msg.handle = ins->handle;
    ret = audio_bind(&msg, &rsp, -1);
    if (ret || rsp.result) {
        AIDXE(0x107e0d, "dd", "server stop failed, ret=%d, result=%d\n", ret, rsp.result);
        return !!(ret | rsp.result);
    }

    if (ins->client->stop) {
        ret = ins->client->stop(ins->client_obj);
        if (ret) {
            AIDXE(0x10213d, "d", "client stop failed, ret=%d\n", ret);
            return ret;
        }
    }

    return 0;
}

int audio_client_close(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_close msg;
    struct audio_rsp_base rsp;
    int ret;

    ACLIENT_CHECK(ins);

    msg.cmd = AUDIO_CMD_CLOSE;
    msg.handle = ins->handle;
    ret = audio_bind(&msg, &rsp, -1);
    if (ret || rsp.result) {
        AIDXE(0x189236, "dd", "server close failed, ret=%d, result=%d\n", ret, rsp.result);
        return !!(ret | rsp.result);
    }

    if (ins->client->close) {
        ret = ins->client->close(ins->client_obj);
        if (ret) {
            AIDXE(0x0828e8, "d", "client close failed, ret=%d\n", ret);
            return ret;
        }
    }

    audio_client_delete_instance(ins);

    return 0;
}

int audio_client_get_timestamp(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_get_timestamp msg;
    struct audio_rsp_base rsp;
    u32 timestamp = 0;
    int ret;

    ACLIENT_CHECK(ins);

    msg.cmd = AUDIO_CMD_GET_TIMESTAMP;
    msg.handle = ins->handle;
    ret = audio_bind(&msg, &rsp, -1);
    if (ret) {
        AIDXE(0x08a0c2, "d", "server get timestamp, ret=%d\n", ret);
        return ret;
    }

    if (ins->client && ins->client->get_seek_time) {
        ret = ins->client->get_seek_time(ins->client_obj, &timestamp);
        if (ret) {
            return ret;
        }
    }

    return timestamp + rsp.result;
}

int audio_client_close_async(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;
    struct audio_msg_close *msg;
    int ret;

    msg = (struct audio_msg_close *)audio_malloc(sizeof(*msg));
    if (!msg) {
        AIDXE(0x0cf2e4, "", "client send close no mem\n");
        return AIC_ENOMEM;
    }

    msg->cmd = AUDIO_CMD_CLOSE;
    msg->handle = ins->handle;
    ret = audio_send_msg(msg, NULL, -1);
    if (ret) {
        AIDXE(0x0f8739, "d", "client send close failed, ret=%d\n", ret);

        return ret;
    }
    /* try to stop in case still playing/recording and to close file in case still openned */
    if (ins->client->stop) {
        ret = ins->client->stop(ins->client_obj);
        if (ret) {
            AIDXE(0x10213d, "d", "client stop failed, ret=%d\n", ret);
        }
    }
    if (ins->client->close) {
        ret = ins->client->close(ins->client_obj);
        if (ret) {
            AIDXE(0x0828e8, "d", "client close failed, ret=%d\n", ret);

            return ret;
        }
    }

    audio_client_delete_instance(ins);

    return 0;
}

/* delete client without notify server */
int audio_client_delete(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;

    ACLIENT_CHECK(ins);

    if (ins->client->stop)
        ins->client->stop(ins->client_obj);

    if (ins->client->close)
        ins->client->close(ins->client_obj);
    audio_client_delete_instance(ins);

    return 0;
}

struct asink_buf * audio_client_get_buf(ACLIENT aclient)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;

    if (IS_REC(ins->handle))
        return audio_record_get_buf(ins->handle);

    return audio_sink_get_buf(ins->handle);
}

int audio_client_callback(ACLIENT aclient, int event, char *buff, u32 *size)
{
    struct aclient_ins *ins = (struct aclient_ins *)aclient;

    if (ins && ins->client && ins->client->cb)
        return ins->client->cb(ins->client_obj, event, buff, size);

    return 0;
}

static int audio_client_init(void)
{
    list_head_init(&g_client_mng.list);
    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(audio_client_init);

void audio_client_register(struct audio_client *client)
{
    list_add(&client->list, &g_client_mng.list);
}

