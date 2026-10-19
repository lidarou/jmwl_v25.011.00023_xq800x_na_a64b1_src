/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used simulate data of voice transfer between ADSP and CP.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cc7e6
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>

#include "snd_ipc.h"
#include "voice_trans.h"
#include "snd_compat.h"

#define VOICE_STACK_SIZE        1024
#define VOICE_THREAD_PRI        10

static aos_thread_id g_voice_th;
static u32 g_voice_on;

static void voice_loop(void *data)
{
    int ret;
    char buf[80];

    SND_INFO("voice_loop entry!\n");

    ret = voice_chn_open();
    if (ret < 0) {
        SND_ERROR("volte dl open chn failed, ret=%d\n", ret);
        return;
    }

    SND_INFO("voice_loop chn opened!\n");

    while (g_voice_on) {
        ret = voice_get(buf, 80);
        if (ret > 0) {
            voice_send(buf, ret);
            SND_INFO("voice got, %d\n", ret);
        }
        else {
            SND_INFO("voice wait\n");
            aos_sleep(50);
        }
    }
    ret = voice_chn_close();

    SND_INFO("voice_loop chn closed, ret=%d\n", ret);
}

static int voice_start(void)
{
    SND_INFO("volte_start entry!\n");

    g_voice_on = TRUE;
    g_voice_th = aos_create_thread("voice_loop", voice_loop, 0,
                                   VOICE_STACK_SIZE, 0,
                                   NULL,
                                   VOICE_THREAD_PRI,
                                   AOS_AUTO_START);
    return 0;
}

static int voice_stop(void)
{
    if (!g_voice_on)
        return 0;

    g_voice_on = FALSE;

    /* wait some time */
    aos_sleep(100);

    aos_terminate_thread(g_voice_th);
    aos_delete_thread(g_voice_th);
    g_voice_th = 0;

    return 0;
}

void voice_test(BOOL is_on)
{
    if (is_on)
        voice_start();
    else
        voice_stop();
}

#ifdef CONFIG_AT_VOICE_TEST
static u32 atvoice_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"^VOICE=<on>";
    rsp[1] = (ulong)"OK";

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atvoice_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1], *paras_arr;
    u16 count, is_on;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT^VOICE=", &count);
    if (!paras_arr || count != 1) {
        SND_ERROR("atvoice splitting failed\n");
        return aic_fill_transmit_cmd_resp(at_req, (ulong *)"ERROR", 1);
    }

    is_on = atoi((char *)paras_arr[0]);

    if (is_on)
        ret = voice_start();
    else
        ret = voice_stop();

    if (ret)
        rsp[0] = (ulong)"ERROR";
    else
        rsp[0] = (ulong)"OK";

    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}

const static AT_CMD_HANDLE voice_at_handles[] =
{
    {
        "AT^VOICE",
        atvoice_test_cmd,
        AIC_NULL,
        atvoice_set_cmd,
        AIC_NULL
    },
    {
        AIC_NULL
    },
};

static int voice_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)voice_at_handles);

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(voice_init);

#endif

