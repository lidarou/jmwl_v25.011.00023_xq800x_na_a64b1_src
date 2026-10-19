/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is mp3 codec adapt file, codec standard APIs are defined.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e301f
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <snd_ipc.h>
#include "audio_util.h"
#include "codec_mng.h"
#include "audio_client.h"
#include "audio_stream.h"
#include "hwcodec.h"
#include "audio_policy.h"

#define HWCODEC_STACK_SIZE   4096
#define HWCODEC_THREAD_PRI   10

#define HWCODEC_EVENT_START 0x01
#define HWCODEC_EVENT_STARTED (0x01 << 2)
#define HWCODEC_EVENT_STOPED  (0x01 << 3)
#define HWCODEC_EVENT_EXIT_REQ (0x01 << 4)

#define HWCODEC_EVENT_EXIT  (0x01 << 5)

enum hwcodec_run_state {
    HWCODEC_INIT_STATE,
    HWCODEC_STARTED,
    HWCODEC_STOP_REQ,
    HWCODEC_STOPPED,
};
struct hwcodec_obj {
    aos_thread_id thread_id;
    u32 client_obj;
    u8 *src_buf;
    u32 period_size;
    s32 stream_dl_id;
    s32 stream_ul_id;
    u32 state;
    aos_mutex_ptr mutex;
    aos_mutex_ptr state_mutex;
    aos_event_ptr event_info;
    u8 exit_flag;
    codec_get fget;
    /*redunt bytes*/
    u8 *residue_data;
    u32 residue_size;
#ifdef CONFIG_SOUND_CONV
    short *rd_buf;
#endif
};

static void hwcodec_loop(void *data)
{
    struct hwcodec_obj *hwcodec = (struct hwcodec_obj*) data;
    u32 real_count , period_size , write_size , writed_size;
    int ret;
    u32 eos_flag = 0;
    u32 event_flag;

    period_size = hwcodec->period_size;

    while(1) {

        aos_get_mutex(hwcodec->mutex, AOS_WAIT_FOREVER);
        /*check whether exit*/
        if(hwcodec->exit_flag) {
            aos_put_mutex(hwcodec->mutex);
            ALOGW("hwcodec_loop exit\n");
            aos_set_event(hwcodec->event_info , HWCODEC_EVENT_EXIT , AOS_OR);
            break;
        }
        aos_put_mutex(hwcodec->mutex);

        /*if not started waite start event*/
        aos_get_mutex(hwcodec->state_mutex , AOS_WAIT_FOREVER);
        if((hwcodec->state == HWCODEC_INIT_STATE) || (hwcodec->state == HWCODEC_STOPPED)) {
            aos_put_mutex(hwcodec->state_mutex);
            aos_get_event(hwcodec->event_info , HWCODEC_EVENT_START | HWCODEC_EVENT_EXIT_REQ,
                          AOS_OR_CLEAR , &event_flag , AOS_WAIT_FOREVER);
            aos_get_mutex(hwcodec->mutex  , AOS_WAIT_FOREVER);
            /*check whether exit*/
            if(hwcodec->exit_flag) {
                aos_put_mutex(hwcodec->mutex);
                ALOGW("hwcodec_loop exit when wait start\n");
                aos_set_event(hwcodec->event_info , HWCODEC_EVENT_EXIT , AOS_OR);
                break;
            }
            aos_put_mutex(hwcodec->mutex);
            aos_get_mutex(hwcodec->state_mutex , AOS_WAIT_FOREVER);
            hwcodec->state = HWCODEC_STARTED;
            aos_put_mutex(hwcodec->state_mutex);
            aos_set_event(hwcodec->event_info , HWCODEC_EVENT_STARTED , AOS_OR);
        } else if(hwcodec->state == HWCODEC_STOP_REQ) {
            hwcodec->state = HWCODEC_STOPPED;
            aos_put_mutex(hwcodec->state_mutex);
            aos_set_event(hwcodec->event_info , HWCODEC_EVENT_STOPED , AOS_OR);
            continue;
        } else {
            aos_put_mutex(hwcodec->state_mutex);
        }
        if(hwcodec->residue_size) {
            /*send residue data*/
            real_count = period_size >= hwcodec->residue_size ? hwcodec->residue_size : period_size;
            memcpy(hwcodec->src_buf , hwcodec->residue_data , real_count);
            hwcodec->residue_data += real_count;
            hwcodec->residue_size -= real_count;
        }
        else {
            ret = hwcodec->fget(hwcodec->client_obj , hwcodec->src_buf , period_size , &real_count);
            if(ret == ASTREAM_DATA_EOS) {
                eos_flag = 1;
            }
        }

        writed_size = 0;
        while(writed_size != real_count) {
            write_size = real_count - writed_size;
            ret = audio_stream_write(hwcodec->stream_dl_id , hwcodec->src_buf , write_size);
            if(ret > 0) {
                writed_size += ret;
            }
        }
        /* end of stream */
        if(eos_flag) {
            /* drain dl buffer */
            ret = audio_stream_drain(hwcodec->stream_dl_id);
            aos_get_mutex(hwcodec->state_mutex , AOS_WAIT_FOREVER);
            /*if eos  ,  set state to init state*/
            if(hwcodec->state == HWCODEC_STARTED) {
                hwcodec->state = HWCODEC_STOPPED;
            }
            aos_put_mutex(hwcodec->state_mutex);
            continue;
        }
     }
}

static int hwcodec_thread_create(void *obj , struct hwcodec_init_param* param)
{
    struct hwcodec_obj *hwcodec = (struct hwcodec_obj *)obj;
    int ret;


    hwcodec->thread_id = aos_create_thread("mp3_hwcodec",
                                           hwcodec_loop,
                                           (void *)hwcodec,
                                           HWCODEC_STACK_SIZE, 0,
                                           NULL,
                                           HWCODEC_THREAD_PRI,
                                           AOS_AUTO_START);
    if(hwcodec->thread_id == AOS_INVALID_THREAD_ID) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_thread_create create thread failed\n");
        goto _err_thread_create;
    }

    return AIC_SUCCESS;

_err_thread_create:
    return ret;
}

static int hwcodec_thread_destroy(void *obj)
{
    struct hwcodec_obj *hwcodec = (struct hwcodec_obj *)obj;
    u32 event_flag;

    aos_get_mutex(hwcodec->mutex  , AOS_WAIT_FOREVER);
    hwcodec->exit_flag = 1;
    aos_put_mutex(hwcodec->mutex);
    aos_set_event(hwcodec->event_info , HWCODEC_EVENT_EXIT_REQ , AOS_OR);
    /* wait for hwcodec loop exit */
    aos_get_event(hwcodec->event_info , HWCODEC_EVENT_EXIT , AOS_OR_CLEAR , &event_flag , AOS_WAIT_FOREVER);

    if(hwcodec->thread_id != AOS_INVALID_THREAD_ID) {
        aos_delete_thread(hwcodec->thread_id);
    }

    return AIC_SUCCESS;
}

static int hwcodec_stream_open(void *obj , struct hwcodec_init_param *param)
{
    struct stream_config stream_cfg;
    int ret;
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*) obj;

    stream_cfg.bit_rate = param->brate;
    stream_cfg.chn = param->chn;
    stream_cfg.flag = COMPR_OUT;
    stream_cfg.fmt = param->dl_fmt;
    stream_cfg.period_cnt = param->period_cnt;
    stream_cfg.period_size = APOLICY_HWCODEC_DL_PERIOD_SIZE;
    stream_cfg.sample_rate = param->srate;
    stream_cfg.start_threshold = 0;
    stream_cfg.stop_threshold = 0;

    ret = audio_stream_open(param->dl_stream_id , &stream_cfg , COMPR_OUT);
    if(!ret) {
        ALOGE("mp3 codec hwcodec_stream_open dl failed\n");
        goto _err_dl_open;
    }
    hwcodec->stream_dl_id = ret;

    stream_cfg.bit_rate = param->brate *1000;
    stream_cfg.chn = param->chn;
    stream_cfg.flag = COMPR_IN;
    stream_cfg.fmt = param->ul_fmt;
    stream_cfg.period_cnt = param->period_cnt;
    stream_cfg.period_size = APOLICY_HWCODEC_UL_PERIOD_SIZE;
    stream_cfg.sample_rate = param->srate;
    stream_cfg.start_threshold = 0;
    stream_cfg.stop_threshold = 0;
    ret = audio_stream_open(param->ul_stream_id , &stream_cfg , COMPR_IN);
    if(!ret) {
        ALOGE("mp3 codec hwcodec_stream_open ul failed\n");
        goto _err_ul_open;
    }
    hwcodec->stream_ul_id = ret;

    return AIC_SUCCESS;
_err_ul_open:
    audio_stream_close(hwcodec->stream_dl_id);
_err_dl_open:

    return ret;
}


static int hwcodec_stream_close(void *obj)
{
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*) obj;
    int ret;

    ret = audio_stream_close(hwcodec->stream_dl_id);
    ret |= audio_stream_close(hwcodec->stream_ul_id);

    if(!ret) {
        return AIC_EFAULT;
    }
    return AIC_SUCCESS;
}

static int hwcodec_stream_start(void *obj)
{
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*) obj;
    int ret;

    ret = audio_stream_start(hwcodec->stream_dl_id);
    ret |= audio_stream_start(hwcodec->stream_ul_id);

    if(ret) {
        return AIC_EFAULT;
    }
    return AIC_SUCCESS;
}

static int hwcodec_stream_stop(void *obj)
{
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*) obj;
    int ret;

    ret = audio_stream_stop(hwcodec->stream_dl_id);
    ret |= audio_stream_stop(hwcodec->stream_ul_id);

    if(ret) {
        return AIC_EFAULT;
    }
    return AIC_SUCCESS;
}


void* hwcodec_init(struct hwcodec_init_param *param)
{
    int ret;
    struct hwcodec_obj *hwcodec = NULL;

    hwcodec = audio_malloc(sizeof(struct hwcodec_obj));
    if(!hwcodec) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_init err malloc obj\n");
        goto _err_malloc_obj;
    }
    memset(hwcodec , 0 , sizeof(struct hwcodec_obj));
    hwcodec->period_size = APOLICY_HWCODEC_DL_PERIOD_SIZE;
    hwcodec->fget = param->fget;
    hwcodec->client_obj = param->client_obj;
    hwcodec->residue_data = param->residue_data;
    hwcodec->residue_size = param->residue_size;
    //init src buf;
    hwcodec->src_buf = audio_malloc(APOLICY_HWCODEC_DL_PERIOD_SIZE);
    if(!hwcodec->src_buf) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_init err malloc\n");
        goto _err_malloc_srcbuf;
    }
#ifdef CONFIG_SOUND_CONV
    hwcodec->rd_buf = audio_malloc(APOLICY_HWCODEC_UL_PERIOD_SIZE);
    if(!hwcodec->rd_buf) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_init err malloc rd\n");
        goto _err_malloc_rdbuf;
    }
#endif
    hwcodec->mutex = aos_create_mutex("hwcodec_mutex");
    if(hwcodec->mutex == AIC_NULL) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_init err mutex\n");
        goto _err_mutex;
    }
    if(AIC_NULL == (hwcodec->state_mutex = aos_create_mutex("hwcodec_stamutex")))
    {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_init err statemutex\n");
        goto _err_stamutex;
    }

    hwcodec->event_info = aos_create_event("hwcodec_event");
    if(hwcodec->event_info == AIC_NULL) {
        ret = AIC_ENOMEM;
        ALOGE("hwcodec_event create error\n");
        goto _err_event;
    }
    /* init stream */
    ret = hwcodec_stream_open(hwcodec , param);
    if(ret) {
        ALOGE("hwcodec_init err streamopen\n");
        goto _err_streamopen;
    }
    ret = hwcodec_stream_start(hwcodec);
    if(ret) {
        ALOGE("hwcodec_stream_start failed\n");
        goto _err_stream_start;
    }
    ret = hwcodec_thread_create(hwcodec , param);
    if(ret != AIC_SUCCESS) {
        ALOGE("hwcodec_init err thread\n");
        goto _err_thread;
    }
    return hwcodec;

_err_thread:
    hwcodec_stream_stop(hwcodec);
_err_stream_start:
    hwcodec_stream_close(hwcodec);
_err_streamopen:
    aos_delete_event(hwcodec->event_info);
_err_event:
    aos_delete_mutex(hwcodec->state_mutex);
_err_stamutex:
    aos_delete_mutex(hwcodec->mutex);
_err_mutex:
#ifdef CONFIG_SOUND_CONV
    audio_free(hwcodec->rd_buf);
_err_malloc_rdbuf:
#endif
    audio_free(hwcodec->src_buf);
_err_malloc_srcbuf:
    audio_free(hwcodec);
_err_malloc_obj:

    return AIC_NULL;
}

int hwcodec_start(void *obj)
{
    u32 event_flag;
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*)obj;

    aos_get_mutex(hwcodec->state_mutex , AOS_WAIT_FOREVER);
    if(hwcodec->state == HWCODEC_INIT_STATE || hwcodec->state == HWCODEC_STOPPED) {
        aos_put_mutex(hwcodec->state_mutex);
        aos_set_event(hwcodec->event_info , HWCODEC_EVENT_START , AOS_OR);
        aos_get_event(hwcodec->event_info , HWCODEC_EVENT_STARTED , AOS_OR_CLEAR ,
                      &event_flag , AOS_WAIT_FOREVER);
        return 0;
    }
    aos_put_mutex(hwcodec->state_mutex);
    return 0;
}

int hwcodec_stop(void *obj)
{
    u32 event_flag;
    struct hwcodec_obj* hwcodec = (struct hwcodec_obj*)obj;

    aos_get_mutex(hwcodec->state_mutex , AOS_WAIT_FOREVER);
    if(hwcodec->state == HWCODEC_STARTED) {
        hwcodec->state = HWCODEC_STOP_REQ;
        aos_put_mutex(hwcodec->state_mutex);
        /*wait for stopped*/
        aos_get_event(hwcodec->event_info, HWCODEC_EVENT_STOPED,
                      AOS_OR_CLEAR, &event_flag , AOS_WAIT_FOREVER);
        return 0;
    }
    aos_put_mutex(hwcodec->state_mutex);
    return 0;
}


int hwcodec_deinit(void *obj)
{
    int ret;
    struct hwcodec_obj *hwcodec = (struct hwcodec_obj*) obj;

    ret = hwcodec_thread_destroy(obj);
    ret = hwcodec_stream_stop(obj);
    ret = hwcodec_stream_close(obj);
    aos_delete_event(hwcodec->event_info);
    aos_delete_mutex(hwcodec->state_mutex);
    /* deinit mutex */
    aos_delete_mutex(hwcodec->mutex);
    /* deinit srcbuf */
    if(hwcodec->src_buf) {
        audio_free(hwcodec->src_buf);
    }
#ifdef CONFIG_SOUND_CONV
    if (hwcodec->rd_buf)
        audio_free(hwcodec->rd_buf);
#endif
    audio_free(hwcodec);

    return ret;
}

#ifdef CONFIG_SOUND_CONV
u32 mix_audio_data(short *pbuf, short *pout, u32 byte)
{
    short *pl = pbuf, *pr = pbuf + 1;
    u32 cnt  = byte / 2;

    while(pl - pbuf < cnt) {
        *pout = (short)(((int)*pl + (int)*pr) / 2);
        pl += 2;
        pr += 2;
        pout++;
    }

    return cnt;
}
#endif

int hwcodec_get_data_inter(void* obj, short *pbuf, u32 count, u32 *real_count)
{
    int readcount;
    struct hwcodec_obj *hwcodec = (struct hwcodec_obj*)obj;

#ifdef CONFIG_SOUND_CONV
    /* read as mono, but source is dual chn, read size need double */
    readcount = audio_stream_read(hwcodec->stream_ul_id , (u8*)hwcodec->rd_buf,
                                  APOLICY_HWCODEC_UL_PERIOD_SIZE);
    readcount = mix_audio_data(hwcodec->rd_buf, pbuf, readcount);
#else
    readcount = audio_stream_read(hwcodec->stream_ul_id , (u8*)pbuf,
                                  APOLICY_HWCODEC_UL_PERIOD_SIZE);
#endif
    if (readcount > 0) {
        *real_count = readcount;
        snd_ipc_prepare_recv_buff(SND_IPC_CH_REC, (u32)-1);
        return 0;
    } else if (readcount == 0) {
        *real_count = 0;
        snd_ipc_prepare_recv_buff(SND_IPC_CH_REC, (u32)-1);
        return ASTREAM_DATA_EOS;
    } else
        return AIC_EFAULT;
}

int hwcodec_get_data(void* obj, short *pbuf, u32 count, u32 *real_count)
{
    u32 rd_count, total_count = 0;
    u8 *rd_buf = (u8 *)pbuf;
    int ret;

    AIC_ASSERT((count % APOLICY_HWCODEC_UL_PERIOD_SIZE) == 0);
    *real_count = count;
    while (total_count < count) {
        ret = hwcodec_get_data_inter(obj, (short *)(rd_buf + total_count),
                                     count - total_count, &rd_count);
        if (ret) {
            memset(rd_buf + total_count, 0, count - total_count);
            return ret;
        }
        total_count += rd_count;
        ALOGE("total=%d,rd_count=%d\n", total_count, rd_count);
    }

    return 0;
}

