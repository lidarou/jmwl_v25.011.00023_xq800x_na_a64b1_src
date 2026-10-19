/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is midi codec adapt file, codec standard APIs are included.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d6c7b
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "eas.h"
#include "audio_util.h"
#include "codec_midi.h"
#include "codec_mng.h"

struct codec_midi_obj {
    struct list_head list;
    EAS_DATA_HANDLE handle;
    EAS_HANDLE stream;
    S_EAS_MEMFILE_DESCRIPTOR desc;
    short *buff;
    u32 buff_size;
    u32 left_samples;
    u32 left_pos;
};

struct codec_midi_runtime {
    struct list_head list;
    u32 open_count;
};

static struct codec_midi_runtime g_codec_midi_runtime;

static inline void append_obj(struct codec_midi_obj *obj)
{
    list_add(&obj->list, &g_codec_midi_runtime.list);
    g_codec_midi_runtime.open_count++;
}

static inline void delete_obj(struct codec_midi_obj *obj)
{
    list_del(&obj->list);
    if (g_codec_midi_runtime.open_count)
        g_codec_midi_runtime.open_count--;
}

static EAS_BOOL midi_eas_change(EAS_VOID_PTR pInstData,
                                S_EXT_AUDIO_PRG_CHG *pPrgChg)
{
    ALOGD("eas_change: pInstData=%p\n", pInstData);
    ALOGD("\tbank=%d, channel=%d, program=%d\n",
          pPrgChg->bank, pPrgChg->channel, pPrgChg->program);

    return TRUE;
}

static EAS_BOOL midi_eas_event_callback(EAS_VOID_PTR pInstData,
                                        S_EXT_AUDIO_EVENT *pEvent)
{
    ALOGD("event callback: pInstData=%p\n", pInstData);
    ALOGD("\tchannel=%d, note=%d, noteOn=%d, velocity=%d\n",
          pEvent->channel, pEvent->note, pEvent->noteOn, pEvent->velocity);

    return 0;
}
static u32 midi_open(u8 *buff, u32 size)
{
    struct codec_midi_obj *codec;
    S_EAS_LIB_CONFIG *config;
    EAS_I32 play_len;
    EAS_RESULT ret;
    u32 buff_size;

    if (!buff || !size) {
        ALOGE("midi open, invalid input param\n");
        return 0;
    }
    codec = audio_malloc(sizeof(*codec));

    if (!codec) {
        ALOGE("midi open, no mem\n");
        return 0;
    }
    memset(codec, 0, sizeof(*codec));
    config = (S_EAS_LIB_CONFIG *)EAS_Config();
    buff_size = config->mixBufferSize * config->numChannels * MIDI_BIT_WID;
    codec->buff = (short *)audio_malloc(buff_size);
    if (!codec->buff) {
        audio_free(codec);
        ALOGE("midi alloc failed, no mem\n");
        return 0;
    }
    codec->buff_size = buff_size;

    ret = EAS_Init(&codec->handle);
    if (ret) {
        ALOGE("midi init failed, ret=%d\n", ret);
        goto init_err;
    }

    codec->desc.dataBuffer = buff;
    codec->desc.fileSize = size;
    ret = EAS_OpenFile(codec->handle, (EAS_FILE_LOCATOR *)&codec->desc,
                       &codec->stream);
    if (ret) {
        ALOGE("midi open failed, ret=%d\n", ret);
        goto open_err;
    };
    ret = EAS_Prepare(codec->handle, codec->stream);
    if (ret) {
        ALOGE("midi prepare failed, ret=%d\n", ret);
        goto after_open_err;
    }
    ret = EAS_RegExtAudioCallback(codec->handle, codec->stream, NULL,
                                  midi_eas_change, midi_eas_event_callback);
    if (ret) {
        ALOGE("midi regiest callback failed, ret=%d\n", ret);
        goto after_open_err;
    }

    ret = EAS_SetPolyphony(codec->handle, codec->stream, MIDI_POLY_MAX);
    if (ret) {
        ALOGE("midi SetPolyphony failed, ret=%d\n", ret);
        goto after_open_err;
    }

    ret = EAS_ParseMetaData(codec->handle, codec->stream, &play_len);
    if (ret) {
        ALOGE("midi ParseMetaData failed, ret=%d\n", ret);
        goto after_open_err;
    }

    append_obj(codec);

    return (u32)codec;

after_open_err:
    EAS_CloseFile(codec->handle, codec->stream);
open_err:
    EAS_Shutdown(codec->handle);
init_err:
    audio_free(codec);

    return 0;
}

int midi_close(u32 obj)
{
    struct codec_midi_obj *codec;
    u32 ret;

    if (!obj)
        return 0;
    codec = (struct codec_midi_obj *)obj;

    EAS_CloseFile(codec->handle, codec->stream);

    ret = EAS_Shutdown(codec->handle);
    if (!ret)
        delete_obj(codec);
    if (codec->buff)
        audio_free(codec->buff);
    audio_free(codec);

    return ret;
}

static int midi_start(u32 obj)
{
    return 0;
}

static int midi_stop(u32 obj)
{
    return 0;
}

static int midi_pause(u32 obj)
{
    return 0;
}

static int midi_resume(u32 obj)
{
    return 0;
}

static int midi_set_speed(u32 obj, u32 speed)
{
    return 0;
}

static int midi_set_gain(u32 obj, u32 gain)
{
    return 0;
}

static int midi_seek(u32 obj, u32 pos)
{
    return 0;
}

/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int midi_get_data(u32 obj, short *pbuf, u32 count, u32 *gen_count)
{
    struct codec_midi_obj *codec;
    EAS_DATA_HANDLE eas_data;
    EAS_HANDLE eas_handle;
    S_EAS_LIB_CONFIG *config;
    EAS_I32 cur_pos, gen_sample;
    EAS_STATE state;
    EAS_RESULT ret;

    u32 copy_sample = 0, chn;
    short *render_buff;
    bool is_done = FALSE;

    if (!obj) {
        ALOGE("midi get data failed, obj is 0\n");
        return AIC_EIO;
    }
    codec = (struct codec_midi_obj *)obj;

    config = (S_EAS_LIB_CONFIG *)EAS_Config();
    chn = config->numChannels;

    /* copy left buff first */
    if (codec->left_samples) {
        if (codec->left_samples > count) {
            memcpy(pbuf, codec->buff + codec->left_pos * chn,
                   count * chn * MIDI_BIT_WID);
            codec->left_pos += count;
            codec->left_samples -= count;
            *gen_count = count;
            /* if buff is full, return directly */
            return count;
        } else {
            /* if buff not full, go on get data from engine */
            memcpy(pbuf, codec->buff + codec->left_pos * chn,
                   codec->left_samples * chn * MIDI_BIT_WID);
            copy_sample = codec->left_samples;
            codec->left_pos = 0;
            codec->left_samples = 0;
        }
    }

    eas_data = codec->handle;
    eas_handle = codec->stream;

    while(copy_sample < count) {
        ret = EAS_GetLocation(eas_data, eas_handle, &cur_pos);
        if (ret) {
            ALOGE("midi GetLocation failed, ret=%d\n", ret);
            break;
        }

        if (copy_sample + config->mixBufferSize < count)
            render_buff = pbuf + copy_sample;
        else
            render_buff = codec->buff;

        ret = EAS_Render(eas_data, render_buff,
                         config->mixBufferSize, &gen_sample);

        if (ret) {
            gen_sample = 0;
            ALOGE("midi rendor failed, ret=%d\n", ret);
            break;
        }

        if (copy_sample + gen_sample < count)
            copy_sample += gen_sample;
        else {
            memcpy(pbuf + copy_sample * chn,
                   codec->buff,
                   (count - copy_sample) * chn * MIDI_BIT_WID);
            codec->left_pos = count - copy_sample;
            codec->left_samples = gen_sample - codec->left_pos;
            copy_sample = count;
        }

        ret = EAS_State(eas_data, eas_handle, &state);
        if (ret) {
            ALOGE("midi get state failed, ret=%d\n", ret);
            break;
        }

        if ((state == EAS_STATE_STOPPED) || (state == EAS_STATE_ERROR) ||
                (state == EAS_STATE_EMPTY)) {
            ALOGE("midi is not running state, state=%d\n", state);
            is_done = TRUE;
            break;
        }
    }

    *gen_count = copy_sample;

    /* error */
    if (ret)
        return ret;

    /* done, return 0 */
    if (is_done)
        return 0;

    /* normal, return samples */
    return copy_sample;
}

/* do not define as const */
static struct codec_engine midi_codec = {
    .type = CODEC_TYPE_MIDI,
    .open = midi_open,
    .close = midi_close,
    .start = midi_start,
    .stop = midi_stop,
    .pause = midi_pause,
    .resume = midi_resume,
    .get_data = midi_get_data,
    .seek = midi_seek,
    .set_gain = midi_set_gain,
    .set_speed = midi_set_speed,
};

static int midi_codec_init(void)
{
    list_head_init(&g_codec_midi_runtime.list);
    codec_register(&midi_codec);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(midi_codec_init);

