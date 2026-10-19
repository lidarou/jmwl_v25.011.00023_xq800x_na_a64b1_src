/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines APIs about voice process between ap and cp.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19f99a
#define LOG_MODULE    AMOD_AUDIO

#include <stdlib.h>
#include <aic_errno.h>
#include <aipc_api.h>
#include <aic_init.h>
#include <aic_cmd_api.h>

#include "audio_system.h"
#include "audio_util.h"
#include "voice_mng.h"
#include "audio_service.h"

#define VOICE_MSG_RSP_OK        0x0
#define VOICE_MSG_RSP_FALSE     0xff

#define VOICE_MNG_NAME        "voice mng"
#define VOICE_MNG_STACK_SIZE  1024
#define VOICE_MNG_PRI         10
#define VOICE_MNG_QUEUE_NUM   3

struct voice_mng_ctx {
    aos_thread_id th;
};

static struct voice_mng_ctx g_voice_mng_ctx;

static int send_cp_msg(u16 cmd, u32 param)
{
    AMSG_T msg;

    AIDXW(0x110a62, "dx", "send cp cmd = %d, param = 0x%x\n", cmd, param);

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.p1 = cmd;
    msg.p2 = param;

    return amsg_send_msg(AIC_SYS_CP, (AMSG_T *)&msg, 3000);
}

static int proc_msg(struct voice_mng_ctx *pctx, AMSG_T *msg)
{
    int ret = 0;

    if (!msg)
        return AIC_EPIPE;

    switch(msg->p1) {
    case VOICE_MSG_OPEN:
        ret = audio_start_voice(TRUE, TRUE);
        send_cp_msg(VOICE_MSG_OPEN,
                    !ret ? VOICE_MSG_RSP_OK : VOICE_MSG_RSP_FALSE);
        break;
    case VOICE_MSG_CLOSE:
        ret = audio_stop_voice();
        send_cp_msg(VOICE_MSG_CLOSE,
                    !ret ? VOICE_MSG_RSP_OK : VOICE_MSG_RSP_FALSE);
        break;
    default:
        break;
    }
    AIDXW(0x03b2e4, "ddd", "cp cmd = %d, value=%d, rsp=%d\n", msg->p1, msg->p2, ret);

    return 0;
}

static void voice_mng_loop(void *data)
{
    struct voice_mng_ctx *pctx = (struct voice_mng_ctx *)data;
    void *pmsg;
    while(1) {
        pmsg = (void *)aos_recv_msg(pctx->th, -1);
        if (!pmsg)
            continue;
        proc_msg(pctx, pmsg);
    }
}

static int voice_mng_init(void)
{
    aos_thread_id th;

    th = aos_create_thread(VOICE_MNG_NAME, voice_mng_loop, &g_voice_mng_ctx,
                           VOICE_MNG_STACK_SIZE,
                           VOICE_MNG_QUEUE_NUM, VOICE_MNG_NAME,
                           VOICE_MNG_PRI, AOS_AUTO_START);
    if (th != AOS_INVALID_THREAD_ID)
        g_voice_mng_ctx.th = th;
    else
        AIDXW(0x166b0a, "", "create voice mng thread failed\n");

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(voice_mng_init);

int voice_test(int argc, char **argv)
{
    BOOL is_on;

    if (argc == 0)
        return AIC_EPERM;

    is_on = atoi(argv[0]);
    send_cp_msg(VOICE_MSG_TEST, is_on);

    return 0;
}
CONSOLE_EXPORT_CMD(voice_test, voice_test, voice_test test);

int audio_proc_cp_msg(AMSG_T *msg, void *p)
{
    struct voice_mng_ctx *pctx = &g_voice_mng_ctx;

    if (!msg)
        return AIC_EPIPE;

    aos_send_msg(pctx->th, msg, 0);

    return 0;
}

int voice_route_notify(bool is_on)
{
    return 0;
}

