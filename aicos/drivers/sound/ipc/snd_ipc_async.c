/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines snd ipc asyc channel proce,by which can exchange cmd
 *  between AP/CP and ADSP async.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19d803
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <aic_module.h>
#include <aic_errno.h>
#include <aic_bitmap.h>

#include "snd_compat.h"
#include "snd_ipc.h"

#define SND_IPC_ASYNC_SESSION_MAX   5

struct ipc_session {
    u32 is_used;
    snd_ipc_cb_async cb;
    u32 cb_data;
};

#ifdef CONFIG_ADSP_MNG
static struct ipc_session g_snd_ipc_async_session[SND_IPC_ASYNC_SESSION_MAX];

/* now only support one cmd */
int snd_ipc_send_cmd_async(void *cmd_ptr, u32 session_id)
{
    struct snd_ipc_msg msg;
    int ret;

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.chn = SND_IPC_CH_ASYNC;
    msg.p2 = (u32)cmd_ptr;
    msg.p1 = session_id;
    ret = amsg_send_msg(AIC_SYS_ADSP, (AMSG_T *)&msg, SND_IPC_WAIT);
    aic_udelay(10);
    snd_ipc_put_cmd_buf();

    return ret;
}

int snd_ipc_async_handle(void *cmd_ptr, u32 session_id)
{
    struct ipc_session *session;

    if (session_id < SND_IPC_ASYNC_SESSION_MAX) {
        session = &g_snd_ipc_async_session[session_id];
        if (session->is_used && session->cb)
            session->cb(cmd_ptr, session->cb_data);
    }

    return 0;
}

u32 snd_ipc_async_reg(void *cb, u32 cb_data)
{
    int i;
    struct ipc_session *session = g_snd_ipc_async_session;

    for (i = 0; i < SND_IPC_ASYNC_SESSION_MAX; i++, session++) {
        if (!session->is_used) {
            session->cb = cb;
            session->cb_data = cb_data;
            session->is_used = TRUE;
            return i;
        }
    }

    return 0;
}

void snd_ipc_async_unreg(u32 session_id)
{
    struct ipc_session *session = g_snd_ipc_async_session;

    if (session_id < SND_IPC_ASYNC_SESSION_MAX && session[session_id].is_used)
        session[session_id].is_used = 0;
}
#endif

