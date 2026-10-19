/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    voice_trans.c: this file defines APIs about voice data transfer.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08f496
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_cmd_api.h>
#include <alog_api.h>
#include <aipc_api.h>

#include "snd_compat.h"
#include "snd_ipc.h"

#define MODULE_NAME             "voice trans"

#define VOICE_PERIOD_CNT        6
#define VOICE_PERIOD_SIZE       80

#define VOICE_TRANS_STACK_SIZE  1024
#define VOICE_TRANS_PRI         10

#define VOICE_MSG_VOICE_SRATE   1
#define VOICE_MSG_OPEN          2
#define VOICE_MSG_CLOSE         3
#define VOICE_MSG_TEST          4

#define VOICE_MSG_RSP_OK        0x0
#define VOICE_MSG_RSP_FALSE     0xff

#define EV_OPEN_OK              BIT_0
#define EV_OPEN_FALSE           BIT_1
#define EV_CLOSE_OK             BIT_2
#define EV_CLOSE_FALSE          BIT_3

/* runtimes context */
struct voice_trans_ctx {
    /* aos_thread_id th; */
    EVENT_PTR event;
    MUTEX_PTR mutex;

    BOOL is_init;
    BOOL is_open;
};

static struct voice_trans_ctx *g_voice_pctx;
extern void voice_test(BOOL is_on);

static u32 wait_event(aos_event_ptr event, u32 ev)
{
    u32 actual_flags = 0;
    aos_status sts;

    sts = aos_get_event(event, ev, AOS_OR_CLEAR, &actual_flags, 1000);
    if (!sts)
        return actual_flags;

    return 0;
}

static u32 send_msg_wait(u16 cmd, u32 param, u32 ev)
{
    struct voice_trans_ctx *ctx = g_voice_pctx;
    AMSG_T msg;
    int ret;

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.p1 = cmd;
    msg.p2 = param;

    SND_INFO("voice msg = %d, value = %d\n", msg.p1, msg.p2);

    ret = amsg_send_msg(AIC_SYS_AP, (AMSG_T *)&msg, 3000);
    if (ret) {
        SND_ERROR("voice msg send failed, ret = %d\n", ret);
        return 0;
    }

    return wait_event(ctx->event, ev);
}

static int proc_msg(AMSG_T *msg, void *p)
{
    struct voice_trans_ctx *ctx = (struct voice_trans_ctx *)p;

    if (!msg)
        return AIC_EPIPE;

    SND_INFO("cp cmd = %d, value = %d\n", msg->p1, msg->p2);

    switch(msg->p1) {
    case VOICE_MSG_OPEN:
        if (msg->p2 == VOICE_MSG_RSP_OK)
            aos_set_event(ctx->event, EV_OPEN_OK, AOS_OR);
        else
            aos_set_event(ctx->event, EV_OPEN_FALSE, AOS_OR);
        break;
    case VOICE_MSG_CLOSE:
        if (msg->p2 == VOICE_MSG_RSP_OK)
            aos_set_event(ctx->event, EV_CLOSE_OK, AOS_OR);
        else
            aos_set_event(ctx->event, EV_CLOSE_FALSE, AOS_OR);
        break;
    case VOICE_MSG_TEST:
        voice_test(msg->p2);
    default:
        SND_INFO("unknown voice msg(%d)!\n", msg->p1);
        break;
    }

    return 0;
}

/**
 * voice_chn_open - Open voice transfer channel.
 *
 * No input param.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_chn_open(void)
{
    u32 ev;
    int ret;

    ev = send_msg_wait(VOICE_MSG_OPEN, 0, EV_OPEN_FALSE | EV_OPEN_OK);
    if ((!ev) || (ev & EV_OPEN_FALSE)) {
        SND_ERROR("voice chn open failed!\n");
        return AIC_EPIPE;
    }

    ret = snd_ipc_chn_open(SND_IPC_CH_UPLINK, VOICE_PERIOD_SIZE,
                           VOICE_PERIOD_CNT, 0, 0);
    if (ret) {
        SND_ERROR("snd ipc ul open failed, ret=%d\n", ret);
        return ret;
    }

    ret = snd_ipc_chn_open(SND_IPC_CH_DOWNLINK, VOICE_PERIOD_SIZE,
                           VOICE_PERIOD_CNT, 0, 0);
    if (ret) {
        SND_ERROR("snd ipc dl open failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

/**
 * voice_chn_close - Close voice transfer channel.
 *
 * No input param.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_chn_close(void)
{
    u32 ev;

    ev = send_msg_wait(VOICE_MSG_CLOSE, 0, EV_CLOSE_FALSE | EV_CLOSE_OK);
    if ((!ev) || (ev & EV_CLOSE_FALSE)) {
        SND_ERROR("voice chn close failed!\n");
    }
    snd_ipc_chn_close(SND_IPC_CH_UPLINK);
    snd_ipc_chn_close(SND_IPC_CH_DOWNLINK);

    return 0;
}

/**
 * voice_send - Close voice transfer channel.
 *
 * @buf: voice data buffer pointer.
 * @len: data length.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_send(u8 *buf, u32 len)
{
    return snd_ipc_write(SND_IPC_CH_DOWNLINK, buf, len);
}

/**
 * voice_get - Close voice transfer channel.
 *
 * @buf: voice data buffer pointer.
 * @len: buffer length.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int voice_get(u8 *buf, u32 len)
{
    if (len > VOICE_PERIOD_SIZE)
        return AIC_EPERM;

    return snd_ipc_read(SND_IPC_CH_UPLINK, buf);
}

static int voice_trans_init(void)
{
    struct voice_trans_ctx *pctx;

    pctx = (struct voice_trans_ctx *)
           snd_malloc(sizeof(struct voice_trans_ctx));
    if (!pctx) {
        SND_ERROR("voice trans init failed, no memory\n");
        return AIC_ENOMEM;
    }
    memset(pctx, 0, sizeof(*pctx));

    pctx->event = aos_create_event(MODULE_NAME);
    if (!pctx->event)
        goto err;
    pctx->mutex = aos_create_mutex(MODULE_NAME);
    if (!pctx->mutex)
        goto err;

#if 0
    pctx->th = aos_create_thread(MODULE_NAME, voice_trans_th_loop,
                                 (void *)pctx, VOICE_TRANS_STACK_SIZE, 0, NULL,
                                 VOICE_TRANS_PRI, AOS_AUTO_START);
    if (!pctx->th)
        goto err;
#endif

    pctx->is_init = TRUE;
    amsg_register_mod_handler(AIC_SYS_AP, AMOD_AUDIO, proc_msg, pctx);

    g_voice_pctx = pctx;

    return 0;

err:
    if (pctx->mutex)
        aos_delete_mutex(pctx->mutex);
    if (pctx->event)
        aos_delete_event(pctx->event);
    if (pctx)
        snd_free(pctx);

    return AIC_EPIPE;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(voice_trans_init);

