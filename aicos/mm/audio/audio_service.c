/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio service main file.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07fb37
#define LOG_MODULE    AMOD_AUDIO

#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <adsp_mng.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink_record.h"
#include "audio_sink.h"
#include "audio_record.h"
#include "audio_util.h"
#include "audio_policy.h"
#include "voice_mng.h"

#define AUDIO_SERVICE_STACK_SIZE (1024 *2)
#define AUDIO_SERVICE_PRI        5
#define AUDIO_SERVICE_QUEUE_CNT  10

struct amessage_rsp {
    int     ret;
    void    *ret_data;
};

struct amessage {
    void                 *message;
    struct amessage_rsp  *message_rsp;
};

typedef int (*ASERVICE_CMD_PROC)(void *msg, void *ret_data);


static struct audio_service_info *g_audio_service_info;

static int proc_open(void *data, void *ret_data)
{
    struct audio_open_rsp *rsp = (struct audio_open_rsp *)ret_data;
    struct audio_msg_open *msg = (struct audio_msg_open *)data;

    if (apolicy_conflict_check(msg->type, msg->format, msg->flag)) {
        AIDXE(0x104580, "", "audio conflict\n");
        rsp->result = AIC_EPERM;
        return AIC_EPERM;
    }

    if (msg->dir == ASTREAM_DIR_PLAY)
        rsp->handle = audio_sink_open(msg->aclient, msg->type, msg->aplay_type,
                                      msg->flag, msg->format, msg->sample_rate,
                                      msg->bit_rate, msg->chn, msg->dir);
    else
        rsp->handle = audio_record_open(msg->aclient, msg->type, msg->aplay_type,
                                        msg->flag, msg->format, msg->sample_rate,
                                        msg->bit_rate, msg->chn, msg->dir);

    if (!rsp->handle) {
        rsp->result = AIC_EPERM;
        return AIC_EPERM;
    }

    rsp->result = 0;

    return 0;
}

static int proc_set_param(void *data, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_set_param *msg = (struct audio_msg_set_param *)data;

    rsp->result = audio_sink_set_param(msg->handle, msg->param);

    return 0;
}

static int proc_play(void *data, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_play *msg = (struct audio_msg_play *)data;

    if (IS_SINK(msg->handle))
        rsp->result = audio_sink_start(msg->handle, msg->aplay_type,
                                       msg->aplay_data);
    else
        rsp->result = audio_record_start(msg->handle, msg->aplay_type,
                                         msg->aplay_data);

    return 0;
}

static int proc_pause(void *pmsg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_pause *msg = (struct audio_msg_pause *)pmsg;

    if (IS_SINK(msg->handle))
        rsp->result = audio_sink_pause(msg->handle);
    else
        rsp->result = audio_record_pause(msg->handle);

    return 0;
}

static int proc_resume(void *pmsg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_pause *msg = (struct audio_msg_pause *)pmsg;

    if (IS_SINK(msg->handle))
        rsp->result = audio_sink_resume(msg->handle);
    else
        rsp->result = audio_record_resume(msg->handle);

    return 0;
}

static int proc_stop(void *data, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_stop *msg = (struct audio_msg_stop *)data;

    if (IS_SINK(msg->handle))
        rsp->result = audio_sink_stop(msg->handle);
    else
        rsp->result = audio_record_stop(msg->handle);

    return 0;
}

static int proc_close(void *data, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_close *msg = (struct audio_msg_close *)data;
    int ret;

    if (IS_SINK(msg->handle))
        ret = audio_sink_close(msg->handle);
    else
        ret = audio_record_close(msg->handle);

    if (rsp)
        rsp->result = ret;

    return 0;
}

static int proc_force_out_device(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_set_device *pmsg =
        (struct audio_msg_set_device *)msg;
    struct audio_service_info *info = g_audio_service_info;

    apolicy_set_force_out_device(pmsg->device);
    apolicy_param_update();
    info->force_device = pmsg->device;
    rsp->result = 0;

    return 0;
}

static int proc_force_in_device(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_set_device *pmsg =
        (struct audio_msg_set_device *)msg;
    struct audio_service_info *info = g_audio_service_info;

    apolicy_set_force_in_device(pmsg->device);
    apolicy_param_update();
    info->force_in_device = pmsg->device;
    rsp->result = 0;

    return 0;
}

static int proc_get_device(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    u32 in_dev, out_dev;

    apolicy_get_current_device(&out_dev, &in_dev);
    rsp->result = (in_dev & 0xffff) | (out_dev << 16);

    return 0;
}

static int proc_get_force_device(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;

    rsp->result = (g_audio_service_info->force_in_device & 0xffff) |
                  (g_audio_service_info->force_device << 16);

    return 0;
}

static int proc_set_volume(void *msg, void *ret_data)
{
    return 0;
}

static int proc_get_volume(void *msg, void *ret_data)
{
    return 0;
}

/**
 *   TODO: now only adsp can set volume, we should change if AP support
 *         in the future
 */
extern int adsp_set_master_vol(u32 vol);
static int proc_set_master_volume(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_set_master_volume *pmsg =
        (struct audio_msg_set_master_volume *)msg;
    struct audio_service_info *info = g_audio_service_info;
    int ret = 0;

    if (pmsg->vol != info->master_volume) {
#ifndef CONFIG_VOLUME_NO_CODEC
#ifdef CONFIG_ADSP_MNG
        ret = adsp_set_master_vol(pmsg->vol);
#endif
#endif
        info->master_volume = pmsg->vol;
    }
    rsp->result = ret;

    return ret;
}

static int proc_get_master_volume(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;

    rsp->result = g_audio_service_info->master_volume;

    return 0;
}

static int proc_mute(void *msg, void *ret_data)
{
    return 0;
}

static int proc_unmute(void *msg, void *ret_data)
{
    return 0;
}

static int proc_mute_all(void *msg, void *ret_data)
{
    return 0;
}

static int proc_unmute_all(void *msg, void *ret_data)
{
    return 0;
}

static int proc_set_voice_info(void *msg, void *ret_data)
{
    return 0;
}

static int proc_get_time_stamp(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_get_timestamp *pmsg = (struct audio_msg_get_timestamp *)msg;

    if (IS_SINK(pmsg->handle))
        rsp->result = audio_sink_get_time(pmsg->handle);
    else
        rsp->result = audio_record_get_time(pmsg->handle);

    return 0;
}

static int proc_user_cb(void *data, void *ret_data)
{
    struct audio_msg_user_cb *msg = (struct audio_msg_user_cb *)data;

    ADEBUG;

    if (IS_SINK(msg->handle))
        audio_sink_user_cb(msg->handle, msg->event);
    else
        audio_record_user_cb(msg->handle, msg->event);

    return 0;
}

static int proc_mute_ul(void *msg, void *ret_data)
{
    struct audio_rsp_base *rsp = (struct audio_rsp_base *)ret_data;
    struct audio_msg_mute_ul *pmsg =
        (struct audio_msg_mute_ul *)msg;

#ifdef CONFIG_ADSP_MNG
    rsp->result = adsp_set_mute_ul(pmsg->is_mute);

#else
    rsp->result = 0;
#endif

    if (!rsp->result)
        g_audio_service_info->mute_ul = pmsg->is_mute;

    return 0;
}

bool audio_is_mute_ul(void)
{
    return g_audio_service_info->mute_ul;
}

static const ASERVICE_CMD_PROC aservice_cmd_proc[] = {
    /* 0 - 4 */
    proc_open,
    proc_set_param,
    proc_play,
    proc_stop,
    proc_pause,
    /* 5 - 9 */
    proc_resume,
    proc_close,
    proc_force_out_device,
    proc_force_in_device,
    proc_get_device,
    /* 10 -14 */
    proc_get_force_device,
    proc_set_volume,
    proc_get_volume,
    proc_set_master_volume,
    proc_get_master_volume,
    /* 15 - 19 */
    proc_mute,
    proc_unmute,
    proc_mute_all,
    proc_unmute_all,
    proc_set_voice_info,
    /* 20 - 24 */
    proc_get_time_stamp,
    proc_user_cb,
    proc_mute_ul,
};

static void audio_bind_notify(void)
{
    struct audio_service_info *info = g_audio_service_info;

    aos_put_semaphore(info->semph);
}

static int audio_bind_wait(int wait_time)
{
    struct audio_service_info *info = g_audio_service_info;
    aos_status ret;

    ret = aos_get_semaphore(info->semph, wait_time);
    if (ret)
        AIDXE(0x125431, "d", "audio bind wait res, ret=%d\n", ret);

    return ret;
}

int audio_bind(void *msg, void *ret_data, int wait_time)
{
    struct audio_service_info *info = g_audio_service_info;
    struct amessage amessage;
    struct amessage_rsp amessage_rsp;
    int result = 0;

    if (!info || !info->mutex)
         return AIC_EPIPE;

    AASSERT(wait_time > 0);
    amessage_rsp.ret_data = ret_data;
    amessage_rsp.ret = 0;
    amessage.message_rsp = &amessage_rsp;
    amessage.message = msg;

    mutex_lock(info->mutex);
    result = aos_send_to_queue(info->queue, &amessage, wait_time);
    if (!result)
        result = audio_bind_wait(AUDIO_WAIT_FOREVER);
    mutex_unlock(info->mutex);

    if (result)
        return result;
    else
        return amessage_rsp.ret;
}

int audio_send_msg(void *msg, void *ret_data, int wait_time)
{
    struct audio_service_info *info = g_audio_service_info;
    struct amessage amessage;

    if (!info)
         return AIC_EPIPE;

    amessage.message_rsp = NULL;
    amessage.message = msg;

    aos_send_to_queue(info->queue, &amessage, (u32)wait_time);

    return 0;
}

int audio_get_voice_rate(void)
{
    return g_audio_service_info->voice_srate;
}

int audio_set_voice_rate(u32 srate, u32 brate)
{
    /* TODO: check valid */

    g_audio_service_info->voice_srate = (srate << 16) | brate;

    return 0;
}

int audio_svc_master_volume(void)
{
    return g_audio_service_info->master_volume;
}

static void audio_service_thread_loop(void *data)
{
    struct audio_service_info *info = (struct audio_service_info *)data;
    struct amessage amessage;
    struct audio_msg_base *msg;
    ASERVICE_CMD_PROC proc;
    int ret = 0, cmd = 0;

    info->semph = aos_create_semaphore("audio server semph", 0);
    if (!info->semph) {
        AIDXE(0x0c4b22, "d", "audio service event create failed, ret=%d\n", ret);
        return;
    }

    info->queue = aos_create_queue("audio service queue",
                                   sizeof(struct amessage),
                                   AUDIO_SERVICE_QUEUE_CNT);
    if (!info->queue) {
        aos_delete_semaphore(info->semph);
        AIDXE(0x1b2601, "", "audio service queue create, no memeory\n");
        return;
    }

    info->mutex = aos_create_mutex("audio service mutex");
    if (!info->mutex) {
        aos_delete_semaphore(info->semph);
        aos_delete_queue(info->queue);
        AIDXE(0x129ed1, "", "audio service mutex\n");
        return;
    }
    while(1) {
        ret = aos_recv_from_queue(info->queue, &amessage, AUDIO_WAIT_FOREVER);
        if (ret)
            continue;

        msg = (struct audio_msg_base *)amessage.message;
        cmd = msg->cmd;
        if (cmd >= 0 && cmd < AUDIO_CMD_MAX) {
            proc = aservice_cmd_proc[cmd];
            if (proc) {
                if (amessage.message_rsp) {
                    ret = proc(msg, amessage.message_rsp->ret_data);
                    if (ret)
                        amessage.message_rsp->ret = ret;
                } else
                    proc(msg, NULL);
            }
            else if (amessage.message_rsp)
                amessage.message_rsp->ret = AIC_EPERM;
        } else
            if (amessage.message_rsp)
                amessage.message_rsp->ret = AIC_EIO;

        /* bind notify, otherwise client will be blocked */
        if (amessage.message_rsp)
            audio_bind_notify();
        else
            audio_free(msg);
    }
}

int audio_service_init(void)
{
    struct audio_service_info *info;

    info = (struct audio_service_info *)
           audio_malloc(sizeof(struct audio_service_info));
    if (!info) {
        AIDXE(0x1c230f, "", "service init failed, no memory\n");
        return AIC_ENOMEM;
    }
    memset(info, 0, sizeof(*info));
    g_audio_service_info = info;
    info->thread = aos_create_thread("audio service",
                                     audio_service_thread_loop,
                                     (void *)info,
                                     AUDIO_SERVICE_STACK_SIZE,
                                     0,
                                     NULL,
                                     AUDIO_SERVICE_PRI,
                                     AOS_AUTO_START);
    info->event = aos_create_event("audio service event");
    info->voice_srate = (16000 << 16) | 7;
    info->master_volume = 10;
    amsg_register_mod_handler(AIC_SYS_CP, AMOD_AUDIO, audio_proc_cp_msg, info);

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(audio_service_init);

