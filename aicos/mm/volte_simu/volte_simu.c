/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used simulate data of VOLTE transfer between ADSP and CP.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08dc7d
#define LOG_MODULE    AMOD_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <aipc_api.h>

#include "audio_util.h"
#include "snd_ipc.h"

#define VOLTE_WAIT_TIME         40

#define VOLTE_PERIOD_SIZE       160
#define VOLTE_PERIOD_COUNT      4
#define VOLTE_CHN               1
#define VOLTE_SRATE             8000

#define VOLTE_STACK_SIZE        1024
#define VOLTE_THREAD_PRI        10

struct input_param {
    u32 period_size;
    u32 period_cnt;
    u32 channels;
    u32 srate;
};

struct volte_ul_msg {
    u32 size;
    u8 *buf;
};

static aos_thread_id g_volte_ul_th;
static aos_thread_id g_volte_dl_th;
static struct input_param g_volte_param;
static u32 g_volte_stop;

static void parse_input_cmd(int argc, char **argv, struct input_param *param)
{
    int i;
    char *p;

    /* set default at first */
    param->period_size = VOLTE_PERIOD_SIZE;
    param->period_cnt = VOLTE_PERIOD_COUNT;
    param->channels = VOLTE_CHN;
    param->srate = VOLTE_SRATE;

    for (i = 0; i < argc; i++) {
        p = argv[i];

        if (strcmp(p, "-p") == 0)
            param->period_size = atoi(argv[++i]);
        else if(strcmp(p, "-n") == 0)
            param->period_cnt = atoi(argv[++i]);
        else if(strcmp(p, "-c") == 0)
            param->channels = atoi(argv[++i]);
        else if(strcmp(p, "-r") == 0)
            param->srate = atoi(argv[++i]);
    }
}

static int volte_ul_callback(void *cb_data, u8 *buf, u32 size)
{
    struct volte_ul_msg *msg;

    ALOGW("volte ul cb: buf=0x%x, size=%d\n", buf, size);

    /* TODO: 1. copy buf to dest memory */

    /* TODO: 2. send msg to UL thread */

    /* NOTE: do not need prepare next buffer, ipc prepared auto */

    return 0;
}

void volte_ul_loop(void *data)
{
    struct input_param *param = (struct input_param *)data;
    aos_msg_ptr *msg;
    int ret, i;

    ALOGW("volte_ul_loop entry, g_volte_ul_th=%d\n", g_volte_ul_th);

    ret = snd_ipc_chn_open(SND_IPC_CH_UPLINK, param->period_size,
                           param->period_cnt, volte_ul_callback, NULL);
    if (ret < 0) {
        ALOGE("volte ul open chn failed, ret=%d\n", ret);
        return;
    }

    for (i = 0; i < param->period_cnt; i++)
        snd_ipc_prepare_recv_buff(SND_IPC_CH_UPLINK, VOLTE_WAIT_TIME);

    while (!g_volte_stop) {
        if (!g_volte_ul_th) {
            ALOGW("g_volte_ul_th is 0!\n");
            aos_sleep(20);
            continue;
        }
        msg = aos_recv_msg(g_volte_ul_th, VOLTE_WAIT_TIME);
        if (!msg)
            continue;

        /* proc msg */
        /* TODO */
        ALOGW("volte ul get data sucess\n");
        aos_sleep(20);
    };
    ALOGW("volte ul exit!\n");
}

void volte_dl_loop(void *data)
{
    struct input_param *param = (struct input_param *)data;
    int ret, i;
    char *buf;

    ALOGW("volte_dl_loop entry!\n");

    ret = snd_ipc_chn_open(SND_IPC_CH_DOWNLINK, param->period_size,
                           param->period_cnt, NULL, NULL);
    if (ret < 0) {
        ALOGE("volte dl open chn failed, ret=%d\n", ret);
        return;
    }

    while (!g_volte_stop) {
        buf = snd_ipc_buf_get(SND_IPC_CH_DOWNLINK, VOLTE_WAIT_TIME);
        if (!buf) {
            ALOGE("volte dl underrun, lost one frame\n");
            continue;
        }
        memcpy(buf, "abcd", param->period_size);
        ret = snd_ipc_send_data(SND_IPC_CH_DOWNLINK, buf, param->period_size);
        if (ret) {
            snd_ipc_buf_recycle(SND_IPC_CH_DOWNLINK);
            ALOGE("volte dl send error, ret=%d\n", ret);
        } else
            ALOGW("volte dl send data sucess\n");

        aos_sleep(20);
    }
    ALOGW("volte dl exit!\n");
}

static int volte_start(int argc, char **argv)
{
    struct input_param *param = &g_volte_param;

    ALOGW("volte_start entry!\n");

    parse_input_cmd(argc, argv, param);
    g_volte_stop = 0;
    g_volte_ul_th = aos_create_thread("volte ul",
                                      volte_ul_loop,
                                      param,
                                      VOLTE_STACK_SIZE, 5,
                                      "volte ul q",
                                      VOLTE_THREAD_PRI,
                                      AOS_AUTO_START);

    g_volte_dl_th = aos_create_thread("volte dl",
                                      volte_dl_loop,
                                      param,
                                      VOLTE_STACK_SIZE, 0,
                                      NULL,
                                      VOLTE_THREAD_PRI,
                                      AOS_AUTO_START);

    CONS_OUT("ul_th=%d, dl_th=%d\n", g_volte_ul_th, g_volte_dl_th);

    return 0;
}
CONSOLE_EXPORT_CMD(volte_start, volte_start, volte_start test);

static int volte_stop(int argc, char **argv)
{
    g_volte_stop = TRUE;

    /* wait some time */
    aos_sleep(100);

    aos_terminate_thread(g_volte_ul_th);
    aos_delete_thread(g_volte_ul_th);
    g_volte_ul_th = 0;

    aos_terminate_thread(g_volte_dl_th);
    aos_delete_thread(g_volte_dl_th);
    g_volte_dl_th = 0;

    return 0;
}
CONSOLE_EXPORT_CMD(volte_stop, volte_stop, volte_stop test);

#define INCOMING_IND    "\r\n+CMCCSI: 1,2,3,4,5,6,7,8,9,10,111111,12,13\r\n"
static int incoming_call(int argc, char **argv)
{
    ABRR_HANDLE handle = NULLPTR;
    int cnt = 0;

    handle = abuf_request_brr(AIPC_SYS_AP, AIPC_BRR_AT);
    AIC_ASSERT(handle);

    cnt = abuf_write(handle, 0, INCOMING_IND, strlen(INCOMING_IND), 20);
    abuf_release_brr(handle);

    if (cnt > 0)
        CONS_OUT("OK\n");
    else
        CONS_OUT("ERROR\n");

    return 0;
}
CONSOLE_EXPORT_CMD(incoming_call, incoming_call, incoming_call test);

