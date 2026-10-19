/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to simulate console, by which we can type charactors
 *  used as console command.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ab223
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <ctype.h>
#include <aic_osa.h>
#include <tx_api.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aipc_api.h>

#include "snd_compat.h"
#include "driver/stream/snd_stream_compr.h"
#include "audio_stream.h"

#define SND_CONSOLE_THREAD_NAME "snd console"
#define SND_CONSOLE_STACK_SIZE  2048
#define SND_CONSOLE_QUEUE_NUM   10
#define SND_CONSOLE_PRI         31
#define SND_CONSOLE_QUEUE_NAME  "snd console queue"

#define SND_CONSOLE_CMD_LEN     50

#define CONS_OUT printf

aos_thread_id g_snd_console_thread;

/* these are for stream test */
#define ASTREAM_PERIOD_SIZE          20
#define ASTREAM_PERIOD_COUNT         4
#define ASTREAM_TEST_STACK_SIZE      1024

struct astream {
    char *stack;
    TX_THREAD thread;
    int is_running;
};
static struct astream g_astream[STREAM_CHN_MAX];

const char music_file[]={
    "00000000000000000000"
    "11111111111111111111"
    "22222222222222222222"
    "33333333333333333333"
    "44444444444444444444"
    "55555555555555555555"
    "66666666666666666666"
    "77777777777777777777"
    "88888888888888888888"
    "99999999999999999999"
};


#define SND_IPC_THREAD_NAME "snd ipc"
#define SND_IPC_STACK_SIZE  2048
#define SND_IPC_QUEUE_NUM   10
#define SND_IPC_PRI         20
#define SND_IPC_QUEUE_NAME  "snd ipc queue"

#define TEST_FLAG COMPR_OUT

aos_thread_id g_snd_ipc_thread;

void aplay_thread_loop(ULONG data)
{
    struct stream_config config;
    int stream;
    int ret, send_cnt = 0;
    int stream_chn = (int)data;

    g_astream[stream_chn].is_running = TRUE;
    config.period_cnt = ASTREAM_PERIOD_COUNT;
    config.period_size = ASTREAM_PERIOD_SIZE;
    config.start_threshold = 0;
    config.stop_threshold =  0;
    config.chn = 2;
    config.width = 16;
    config.sample_rate = 16000;
    config.flag = TEST_FLAG;
    stream = audio_stream_open(stream_chn, &config, TEST_FLAG);
    if (!stream) {
        CONS_OUT("open file failed!\n");
        return;
    }

    ret = audio_stream_write(stream, (char *)music_file, ASTREAM_PERIOD_SIZE);
    if (ret < 0) {
        CONS_OUT("write error, ret=%d\n", ret);
        goto out;
    }
    send_cnt += ASTREAM_PERIOD_SIZE;

    ret = audio_stream_start(stream);
    if (ret) {
        CONS_OUT("start error, ret=%d\n", ret);
        goto out;
    }
    while (send_cnt < sizeof(music_file) && g_astream[stream_chn].is_running) {
        ret = audio_stream_write(stream, (char *)music_file + send_cnt,
                                 ASTREAM_PERIOD_SIZE);
        if (ret < 0) {
            CONS_OUT("write error, ret=%d\n", ret);
            aos_sleep(100);
            //goto out;
        }
        send_cnt += ASTREAM_PERIOD_SIZE;
        CONS_OUT("%s, after write, send_cnt=%d\n", __func__, send_cnt);
    }

    ret = audio_stream_stop(stream);
    aicx_debug(0x160318, "sd", "%s, stop ret=%d\n", __func__, ret);
    CONS_OUT("Done!\n");
out:
    ret = audio_stream_close(stream);
    aicx_debug(0x10177a, "sd", "%s, close ret=%d\n", __func__, ret);
}

static int aplay_main(int argc, char **argv)
{
    int stream_chn = 6;

    if(argc == 1) {
        if (isdigit(argv[0][0]))
            stream_chn = atoi(argv[0]);
        else {
            CONS_OUT("must be digital!\n");
            return AIC_SUCCESS;
        }
    }

    if (stream_chn >= STREAM_CHN_MAX) {
        CONS_OUT("error chn!\n");
        return AIC_SUCCESS;
    }

    g_astream[stream_chn].stack = aos_malloc(ASTREAM_TEST_STACK_SIZE);
    if (!g_astream[stream_chn].stack) {
        CONS_OUT("no memory!\n");
        return AIC_SUCCESS;
    }
    memset(g_astream[stream_chn].stack, 0, ASTREAM_TEST_STACK_SIZE);
    aicx_error(0x103a44, "sx", "%s, TX_THREAD size: %x\n", __func__, sizeof(TX_THREAD));
    return tx_thread_create(&g_astream[stream_chn].thread, "aplay test",
                           aplay_thread_loop, stream_chn,
                           g_astream[stream_chn].stack,
                           ASTREAM_TEST_STACK_SIZE, 25, 0, 0, 1);
}

void snd_console_thread_entry(void *input)
{
    char cmd_str[SND_CONSOLE_CMD_LEN];
    int ret;
    while(1) {
        scanf("%s", cmd_str);
        if (strcmp(cmd_str, "aplay") == 0) {
            aplay_main(0, NULL);
        } else
        if (strcmp(cmd_str, "aipc") == 0) {
            ret = adsp_send_cmd(1, 1, 1);
            printf("send com cmd ret=%d\n", ret);

            //ret = adsp_send_param(1, cmd_str, sizeof(cmd_str));
            //printf("send_blk ret=%d\n", ret);
        }
    }
}

void snd_ipc_thread_entry(void *input)
{
    ABRR_HANDLE handle = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_AUDIO);
    u32 link_sets;
    u8 links;
    struct snd_stream_cmd_common *pcmd;
    struct snd_stream_cmd_rsp *prsp;
    ABLK_T blk;
    int ret;

    while(1) {

        link_sets = BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4);
        links = ablk_select(handle, link_sets, AIPC_WAIT_FOREVER);

        /* sync command */
        if (links & BIT(0)) {
            links ^= BIT(0);
            ret = ablk_recv_block(handle, 0, &blk, AIPC_NO_WAIT);
            if (ret) {
               printf("adsp recv failed ret=%d\n", ret);
               continue;
            }

            pcmd = (struct snd_stream_cmd_common *)blk.addr;
            printf("ipc recv cmd=%d, stream_id=%d\n",
                   pcmd->cmd, pcmd->stream_id);
            ablk_release_block(handle, &blk);

            ret = ablk_get_block(handle, sizeof(struct snd_stream_cmd_rsp),
                                 &blk, AIPC_WAIT_FOREVER);
            if (ret) {
                printf("cannot get block\n");
                continue;
            }
            blk.size = sizeof(struct snd_stream_cmd_rsp);
            prsp = (struct snd_stream_cmd_rsp *)blk.addr;
            prsp->cmd = pcmd->cmd;
            prsp->stream_id = pcmd->stream_id;
            prsp->result = 0;

            /* send */
            ret = ablk_send_block(handle, 0, &blk, AIPC_WAIT_FOREVER);
            if (ret) {
                printf("response send failed ret=%d\n", ret);
                ablk_recycle_block(handle, &blk);
                continue;
            }
        }
        /* ASYNC command */
        if (links & BIT(1)){
            links ^= BIT(1);
        }
        if (links)
        {
            u16 *p;
            u32 data_size;
            ret = ablk_recv_block(handle, 2, &blk, AIPC_NO_WAIT);
            if (ret) {
               printf("adsp recv data failed ret=%d\n", ret);
               continue;
            }
            p = (u16 *)blk.addr;
            data_size = blk.size;
            printf("ipc recv data=0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,\n",
                   p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
            ablk_release_block(handle, &blk);

            ret = ablk_get_block(handle, 4,
                                 &blk, AIPC_WAIT_FOREVER);
            if (ret) {
                printf("cannot get block\n");
                continue;
            }

            /* Note: here we set blk.size to be data_size, just for test */
            blk.size = data_size;
            *(u32 *)blk.addr = data_size;

            /* send */
            ret = ablk_send_block(handle, 2, &blk, AIPC_WAIT_FOREVER);
            if (ret) {
                printf("response send failed ret=%d\n", ret);
                ablk_recycle_block(handle, &blk);
                continue;
            }
        }
    }
}

int snd_console_simu_init(void)
{

    g_snd_console_thread = aos_create_thread(SND_CONSOLE_THREAD_NAME,
                                             snd_console_thread_entry,
                                             NULL,
                                             SND_CONSOLE_STACK_SIZE,
                                             SND_CONSOLE_QUEUE_NUM,
                                             SND_CONSOLE_QUEUE_NAME,
                                             SND_CONSOLE_PRI,
                                             AOS_AUTO_START);

    g_snd_ipc_thread = aos_create_thread(SND_IPC_THREAD_NAME,
                                         snd_ipc_thread_entry,
                                         NULL,
                                         SND_IPC_STACK_SIZE,
                                         SND_IPC_QUEUE_NUM,
                                         SND_IPC_QUEUE_NAME,
                                         SND_IPC_PRI,
                                         AOS_AUTO_START);
    return 0;
}

