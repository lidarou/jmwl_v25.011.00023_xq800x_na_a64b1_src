/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines snd ipc interface, by which can exchange data/cmd
 *  between AP/CP and ADSP.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06853e
#define LOG_MODULE    AMOD_SOUND

#include <aic_osa.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <aic_module.h>
#include <aic_errno.h>
#include <aic_bitmap.h>
#include <subsys_api.h>

#include "snd_compat.h"
#include "snd_ipc.h"

#define SNC_IPC_FLAG_ACK    1

#define CHK_ACK(msg)
#define SND_IPC_LOCK_GET(x)    if (x) mutex_lock(x);
#define SND_IPC_LOCK_PUT(x)    if (x) mutex_unlock(x);

struct msg_cb_info {
    void *msg;
    void *ack;
};

struct snd_ipc_buf {
    u32 rd;
    u32 wr;
    u32 period_cnt;
    u32 period_size;
    u32 address;
    u32 flag;
    u32 reserve1;
    u32 reserve2;
};

struct snd_ipc_chn_info {
    snd_ipc_cb cb;
    void *cb_data;
};

const char *snd_ipc_chn_name[] = {
    "snd_ipc_chn0",
    "snd_ipc_chn1",
    "snd_ipc_chn2",
#ifdef CONFIG_SUBSYS_AP
    "snd_ipc_chn3",
    "snd_ipc_chn4",
    "snd_ipc_chn5",
    "snd_ipc_chn6",
    "snd_ipc_chn7",
    "snd_ipc_chn8",
    "snd_ipc_chn9",
#endif
};

#if defined (CONFIG_SUBSYS_AP)
const static u32 snd_ipc_buf_arr[] = {
    SND_IPC_MEM_AP_CMD,
    SND_IPC_MEM_NV,
    SND_IPC_MEM_PCM,
    SND_IPC_MEM_COMPR,
    SND_IPC_MEM_REC,
    SND_IPC_MEM_LOG,
    SND_IPC_MEM_DUMP,
    SND_IPC_MEM_ASYNC,
    SND_IPC_MEM_UPLINK,
    SND_IPC_MEM_DOWNLINK,
};

const static u32 snd_ipc_buf_size[] = {
    SND_IPC_SIZE_AP_CMD,
    SND_IPC_SIZE_NV,
    SND_IPC_SIZE_PCM,
    SND_IPC_SIZE_COMPR,
    SND_IPC_SIZE_REC,
    SND_IPC_SIZE_LOG,
    SND_IPC_SIZE_DUMP,
    SND_IPC_SIZE_AP_CMD,
    SND_IPC_MEM_UPLINK,
    SND_IPC_MEM_DOWNLINK,
};

#elif defined (CONFIG_SUBSYS_CP)

const static u32 snd_ipc_buf_arr[] = {
    SND_IPC_MEM_CP_CMD,
    SND_IPC_MEM_UPLINK,
    SND_IPC_MEM_DOWNLINK,
};

const static u32 snd_ipc_buf_size[] = {
    SND_IPC_SIZE_CP_CMD,
    SND_IPC_SIZE_UPLINK,
    SND_IPC_SIZE_DOWNLINK,
};

#else
#error "select AP or CP side snd ipc!"
#endif

aos_event_ptr g_snd_ipc_event_ptr[SND_IPC_CH_MAX - 3];
struct snd_ipc_chn_info snd_ipc_chn_info[SND_IPC_CH_MAX];
struct msg_cb_info g_snd_ipc_cb_info;
MUTEX_PTR g_snd_ipc_lock;

void *snd_ipc_get_buf_header(u32 chn)
{
    if (chn >= sizeof(snd_ipc_buf_arr) / sizeof(u32) )
        return 0;
    return (void *)snd_ipc_buf_arr[chn];
}

void *snd_ipc_get_cmd_buf(void)
{
    SND_IPC_LOCK_GET(g_snd_ipc_lock);
    return (void *)snd_ipc_buf_arr[SND_IPC_CH_CMD];
}
void snd_ipc_put_cmd_buf(void)
{
    SND_IPC_LOCK_PUT(g_snd_ipc_lock);
}

int snd_ipc_send_cmd(void *cmd_ptr)
{
#ifdef CONFIG_ADSP_MNG
    struct snd_ipc_msg msg;
    u32 actual_flag;
    aos_status status;

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.chn = SND_IPC_CH_CMD;
    msg.p2 = (u32)cmd_ptr;

    amsg_send_msg(AIC_SYS_ADSP, (AMSG_T *)&msg, SND_IPC_WAIT);
    status = aos_get_event(g_snd_ipc_event_ptr[SND_IPC_CH_CMD], SNC_IPC_FLAG_ACK,
                           AOS_OR_CLEAR, &actual_flag, SND_IPC_WAIT);
    snd_ipc_put_cmd_buf();
    if (status == AOS_SUCCESS && actual_flag & SNC_IPC_FLAG_ACK)
        return AIC_SUCCESS;
    else {
        AICX_ASSERT_IFX(0, 0x1bfa88, "", "ADSP exception!!!\n");

        return AIC_EACCES;
    }
#else
    return 0;
#endif
}

int snd_ipc_chn_open(u32 chn, u32 period_size, u32 period_cnt,
                     snd_ipc_cb cb, void *cb_data)
{
    struct snd_ipc_buf *header;

    /* TODO: check open already */

    if (chn < SND_IPC_CH_DATA || chn >= SND_IPC_CH_MAX) {
        SND_ERROR("invalie snd icp chn = %d\n", chn);
        return AIC_EACCES;
    }

    AIC_ASSERT(period_size * period_cnt + SND_IPC_BUF_HEAD_SIZE <=
               snd_ipc_buf_size[chn]);
    if (period_size * period_cnt == 0) {
        SND_ERROR("period size or cnt could not be 0\n");
        return AIC_EIO;
    }
    header = snd_ipc_get_buf_header(chn);
    header->rd = 0;
    header->wr = 0;
    header->address = (u32)((char *)header + sizeof(struct snd_ipc_buf));
    header->period_size = period_size;
    header->period_cnt = period_cnt;
    header->reserve1 = 0;
    header->reserve2 = 0;
    snd_ipc_chn_info[chn].cb = cb;
    snd_ipc_chn_info[chn].cb_data = cb_data;

    /* TODO: set open state */

    return 0;
}

void snd_ipc_chn_close(u32 chn)
{
    struct snd_ipc_buf *header;

    if (chn < SND_IPC_CH_DATA || chn >= SND_IPC_CH_MAX)
        return;

    header = snd_ipc_get_buf_header(chn);
    header->period_size = 0;
    header->period_cnt = 0;
    header->rd = 0;
    header->wr = 0;

    /* TODO: set close state */
}

int snd_ipc_wait_ack(u32 chn, u32 time)
{
    u32 actual_flag;
    aos_status status;

    status = aos_get_event(g_snd_ipc_event_ptr[chn], SNC_IPC_FLAG_ACK,
                           AOS_OR_CLEAR, &actual_flag, time);

    if (status == AOS_SUCCESS && actual_flag & SNC_IPC_FLAG_ACK)
        return AIC_SUCCESS;
    else
        return AIC_EACCES;
}

void *snd_ipc_buf_get(u32 chn, u32 time)
{
    struct snd_ipc_buf *header;
    u32 offset, try_cnt = 0;

    /* TODO: add lock */

    header = snd_ipc_get_buf_header(chn);

    /* try twice at most, because first time ack maybe got already */
    while (header->wr - header->rd >= header->period_cnt && try_cnt++ < 2) {
        if (snd_ipc_wait_ack(chn, time) != AIC_SUCCESS)
            return NULL;
    }

    offset = (header->wr % header->period_cnt) * header->period_size;
    header->wr++;

    return (void *)(header->address + offset);
}

static void snd_ipc_buf_release(u32 chn)
{
    struct snd_ipc_buf *header;

    /* TODO: add lock */
    header = snd_ipc_get_buf_header(chn);
    if (header->wr > header->rd)
        header->rd++;
}

void snd_ipc_buf_recycle(u32 chn)
{
    struct snd_ipc_buf *header;

    header = snd_ipc_get_buf_header(chn);

    header->wr--;
}

int snd_ipc_send_data(u32 chn, void *buf, u32 len)
{
#ifdef CONFIG_ADSP_MNG
    struct snd_ipc_msg msg;

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.chn = chn;
    msg.p1 = len;
    msg.p2 = (u32)buf;

    return amsg_send_msg(AIC_SYS_ADSP, (AMSG_T *)&msg, SND_IPC_WAIT);
#else
    return 0;
#endif
}

int snd_ipc_prepare_recv_buff(u32 chn, u32 time)
{
#ifdef CONFIG_ADSP_MNG
    struct snd_ipc_msg msg;
    struct snd_ipc_buf *header;
    void *buf;

    buf = snd_ipc_buf_get(chn, time);
    if (!buf) {
        SND_ERROR("no recv buff\n");
        return AIC_ENOMEM;
    }

    header = snd_ipc_get_buf_header(chn);
    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.chn = chn;
    msg.p1 = header->period_size;
    msg.p2 = (u32)buf;

    return amsg_send_msg_nobackup(AIC_SYS_ADSP, (AMSG_T *)&msg, time);
#else
    return 0;
#endif
}

/**
 * Note: No need now, but don't delete, maybe we need it again sometime.
 */
#ifdef SND_IPC_RECV_API
/* Each event get once */
int snd_ipc_recv_data(u32 chn, void *buf)
{
    struct snd_ipc_buf *header;
    u32 actual_flag, offset;
    aos_status status;

    /* TODO: change to queue? */
    status = aos_get_event(g_snd_ipc_event_ptr[chn], SNC_IPC_FLAG_ACK,
                           AOS_OR_CLEAR, &actual_flag, SND_IPC_WAIT);

    if (status == AOS_SUCCESS && actual_flag & SNC_IPC_FLAG_ACK) {
        header = (struct snd_ipc_buf *)snd_ipc_get_buf_header(chn);

        AIC_ASSERT(header->rd < header->wr);
        offset = (header->rd % header->period_cnt) * header->period_size;
        buf = (void *)(header->address + offset);
        header->rd++;

        return header->period_size;
    }

    return AIC_EBUSY;
}
#endif

/* For volte only, not lock, should be called in single thread */
int snd_ipc_read(u32 chn, void *buf)
{
    struct snd_ipc_buf *header;
    u32 offset;
    u16 *p;

    if (!buf)
        return AIC_EPIPE;

    header = (struct snd_ipc_buf *)snd_ipc_get_buf_header(chn);
    if (!header || header->wr <= header->rd)
        return AIC_ENODATA;

    offset = (header->rd % header->period_cnt) * header->period_size;
    p = (u16 *)(header->address + offset);
    if (header->period_size) {
        AICX_ASSERT_IFX(*p <= header->period_size, 0x1177b4, "dd", "size=%d, period_size=%d\n",
                        *p, header->period_size);
        memcpy(buf, p + 1, *p);

        header->rd++;

        return *p;
    }

    return AIC_ENODATA;
}

/* For volte only, not lock, should be called in single thread */
int snd_ipc_write(u32 chn, void *buf, u16 size)
{
    struct snd_ipc_buf *header;
    u32 offset;
    u16 *p;

    header = snd_ipc_get_buf_header(chn);

    if (!header || header->wr - header->rd >= header->period_cnt)
        return AIC_ENOMEM;
    if (!buf || size > header->period_size)
        return AIC_EPIPE;

    offset = (header->wr % header->period_cnt) * header->period_size;
    p = (u16 *)(header->address + offset);
    *p = size;
    memcpy(p + 1, buf, size);

    header->wr++;

    return size;
}

int snd_ipc_msg_cb(AMSG_T *amsg, void *p)
{
    struct snd_ipc_msg *msg = (struct snd_ipc_msg *)amsg;

    switch(msg->chn) {
    case SND_IPC_CH_CMD:
        CHK_ACK(msg);
        aos_set_event(g_snd_ipc_event_ptr[SND_IPC_CH_CMD], SNC_IPC_FLAG_ACK,
                      AOS_OR);
        break;
#ifdef CONFIG_SND_IPC_AP
    case SND_IPC_CH_NV:
        break;
#endif
    case SND_IPC_CH_ASYNC:
        snd_ipc_async_handle((void *)msg->p2, msg->p1);
        break;
    default:
        /* release one buf */
        snd_ipc_buf_release(msg->chn);

        /* notify user */
        if (snd_ipc_chn_info[msg->chn].cb)
            snd_ipc_chn_info[msg->chn].cb(snd_ipc_chn_info[msg->chn].cb_data,
                                          (u8 *)msg->p2, msg->p1);

        /* set event to wake up */
        aos_set_event(g_snd_ipc_event_ptr[msg->chn], SNC_IPC_FLAG_ACK,
                      AOS_OR);
        break;
    }

    return 0;
}

int snd_ipc_init(void)
{
#ifdef CONFIG_SUBSYS_AP
    memset((void *)SND_IPC_SHMEM_BASE, 0, SND_IPC_SHMEM_SIZE);
#endif
#ifdef CONFIG_ADSP_MNG
    int i;

    for (i = 0; i < SND_IPC_CH_MAX - 3; i++)
        g_snd_ipc_event_ptr[i] = aos_create_event((char *)snd_ipc_chn_name[i]);

    amsg_register_mod_handler(AIC_SYS_ADSP, AMOD_AUDIO, snd_ipc_msg_cb,
                              (void *)&g_snd_ipc_cb_info);
    g_snd_ipc_lock = aos_create_mutex("snd_ipc");
    AIC_ASSERT(g_snd_ipc_lock);
#endif
    return 0;
}
AIC_NORMAL_DRV_P2_INIT_EXPORT(snd_ipc_init);

