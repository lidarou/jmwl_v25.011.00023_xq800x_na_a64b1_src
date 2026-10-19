/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines snd ipc MACRO and interface declair for snd module.
 *
 */

#ifndef __SND_IPC_H__
#define __SND_IPC_H__

#include <snd_ipc_cfg.h>

#define SND_IPC_WAIT            2000

#define SND_IPC_CMD_BASE        100

struct snd_ipc_msg {
    /* the protocol type */
    u32   ty : 8;
    /*
     * in AMSG protocol it's module id,
     * in ABLK protocol it's bearer id.
     */
    u32   id : 8;

    u32   chn : 4;
    u32   p1 : 12;
    /* parameter 2 */
    u32   p2;
};

#if defined (CONFIG_SUBSYS_AP)
enum {
    SND_IPC_CH_CMD = 0,
    SND_IPC_CH_NV,
    SND_IPC_CH_DATA,
    SND_IPC_CH_PCM = SND_IPC_CH_DATA,
    SND_IPC_CH_COMPR,
    SND_IPC_CH_REC,
    SND_IPC_CH_LOG,
    SND_IPC_CH_DUMP,
    SND_IPC_CH_DATA_E = SND_IPC_CH_DUMP,
    SND_IPC_CH_ASYNC,
    SND_IPC_CH_UL_AP,
    SND_IPC_CH_DL_AP,
    SND_IPC_CH_MAX,
};
#elif defined (CONFIG_SUBSYS_CP)
enum {
    SND_IPC_CH_CMD = 0,
    SND_IPC_CH_DATA,
    SND_IPC_CH_UPLINK = SND_IPC_CH_DATA,
    SND_IPC_CH_DOWNLINK,
    SND_IPC_CH_DATA_E = SND_IPC_CH_DOWNLINK,
    SND_IPC_CH_ASYNC,
    SND_IPC_CH_MAX,

    /* DSP send to CP */
};
#else
#error "select AP or CP side snd ipc!"
#endif

#define IS_SND_IPC_DATA_CHN(ch)  (ch >= SND_IPC_CH_DATA && ch <= SND_IPC_CH_DATA_E)

typedef int (*snd_ipc_cb)(void *cb_data, u8 *buf, u32 size);
typedef int (*snd_ipc_cb_async)(void *rsp_ptr, u32 cb_data);

/* command */
void *snd_ipc_get_cmd_buf(void);
void snd_ipc_put_cmd_buf(void);
int snd_ipc_send_cmd(void *cmd_ptr);

/* data */
int snd_ipc_chn_open(u32 chn, u32 period_size, u32 period_cnt,
                     snd_ipc_cb cb, void *cb_data);
void snd_ipc_chn_close(u32 chn);

/* buf for data */
void *snd_ipc_buf_get(u32 chn, u32 time);
void snd_ipc_buf_recycle(u32 chn);

/* send data */
int snd_ipc_send_data(u32 chn, void *buf, u32 len);

/* for receive data */
int snd_ipc_prepare_recv_buff(u32 chn, u32 time);

/* ack for both send and receive */
int snd_ipc_wait_ack(u32 chn, u32 time);

int snd_ipc_send_cmd_async(void *cmd_ptr, u32 session_id);
int snd_ipc_async_handle(void *cmd_ptr, u32 session_id);
u32 snd_ipc_async_reg(void *cb, u32 cb_data);
void snd_ipc_async_unreg(u32 session_id);

void *snd_ipc_get_buf_header(u32 chn);

/* For volte only, not lock, should be called in single thread */
int snd_ipc_read(u32 chn, void *buf);
/* For volte only, not lock, should be called in single thread */
int snd_ipc_write(u32 chn, void *buf, u16 size);

#endif /* __SND_IPC_H__ */

