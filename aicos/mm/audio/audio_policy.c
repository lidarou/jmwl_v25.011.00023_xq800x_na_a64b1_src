/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio policy main file, it contains all definition and
 *  APIs about policy, include stream select, route select, usecase conflict
 *  process etc.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d6c9c
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <alog_api.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_policy.h"
#include "audio_stream.h"
#include "audio_route.h"
#include "audio_md.h"

#define STREAM_THREAD_PRIORITY_DEFAULT  5

#define APOLICY_STREAM_MAX              5

enum apolicy_strategy {
    STRATEGY_NORMAL = 0,
    STRATEGY_CALL,
    STRATEGY_RING,
    STRATEGY_ALARM,
    STRATEGY_CAM
};

struct apolicy_runtime {
    u32 hlist[APOLICY_STREAM_MAX];
    u32 supported_out_device;
    u32 supported_in_device;
    u32 stream_type;
    u32 force_out_device;
    u32 force_in_device;
    u32 out_device;
    u32 in_device;
    u8  hp_on : 1;
    u8  hs_on : 1;
    u8  bt_sco_on : 1;
    u8  bt_a2dp_on : 1;
};

struct apolicy_runtime apolicy_runtime;

static const struct apolicy_stream_config astream_config_fast = {
    .period_size = 320,
    .period_cnt = 3,
    .start_threshold = 320,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_normal = {
    .period_size = 320,
    .period_cnt = 3,
    .start_threshold = 320,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_deepbuf = {
    .period_size = 640,
    .period_cnt = 4,
    .start_threshold = 640,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_record = {
    .period_size = 320,
    .period_cnt = 2,
    .start_threshold = 320,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_pcm_compress = {
    .period_size = 1920,
    .period_cnt = 2,
    .start_threshold = 640,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_compress = {
    .period_size = 2048,
    .period_cnt = 2,
    .start_threshold = 0,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_compress_amr_nb = {
    .period_size = 128,
    .period_cnt = 2,
    .start_threshold = 0,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_compress_amr_wb = {
    .period_size = 256,
    .period_cnt = 2,
    .start_threshold = 0,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_record_amr_nb = {
    .period_size = 64,
    .period_cnt = 2,
    .start_threshold = 160,
    .stop_threshold = 0,
};

static const struct apolicy_stream_config astream_config_record_amr_wb = {
    .period_size = 64,
    .period_cnt = 2,
    .start_threshold = 160,
    .stop_threshold = 0,
};

/* keep seqence same as type definition */
static const struct astream_property g_astream_property[ASTREAM_TYPE_MAX] = {
    {
        .type = ASTREAM_TYPE_MUSIC, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_RING, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_SYSTEM, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#ifdef CONFIG_TONE_SUPPORT
    {
        .type = ASTREAM_TYPE_TONE, .is_dynamic = 0, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#else
    {
        .type = ASTREAM_TYPE_TONE, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#endif
    {
        .type = ASTREAM_TYPE_DTMF, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#ifdef CONFIG_MCU_AUDIO
    {
        .type = ASTREAM_TYPE_VOICE, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#else
    {
        .type = ASTREAM_TYPE_VOICE, .is_dynamic = 1, .has_stream = 0,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#endif
    {
        .type = ASTREAM_TYPE_VOIP, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_FM, .is_dynamic = 1, .has_stream = 0,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_TTS, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_ALARM, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_NOTIFICATION, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
    {
        .type = ASTREAM_TYPE_RECORD, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#ifdef CONFIG_MCU_AUDIO
    {
        .type = ASTREAM_TYPE_VOICE_UL, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#else
    {
        .type = ASTREAM_TYPE_VOICE_UL, .is_dynamic = 1, .has_stream = 0,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
#endif
    {
        .type = ASTREAM_TYPE_VOIP_UL, .is_dynamic = 1, .has_stream = 1,
        .priority = STREAM_THREAD_PRIORITY_DEFAULT
    },
};

static enum apolicy_strategy get_strategy(void);

#ifdef CONFIG_MCU_AUDIO
const struct apolicy_stream_config *apolicy_get_config(
                enum astream_type type,
                enum astream_format format,
                enum astream_direct dir,
                u32 flag)
{
    switch (type) {
    case ASTREAM_TYPE_VOICE:
    case ASTREAM_TYPE_VOICE_UL:
    case ASTREAM_TYPE_VOIP:
    case ASTREAM_TYPE_VOIP_UL:
        return &astream_config_normal;
    case ASTREAM_TYPE_RECORD:
        return &astream_config_record;
    default:
        break;
    }

    switch(format) {
    case ASTREAM_FORMAT_PCM:
    case ASTREAM_FORMAT_WAV:
    case ASTREAM_FORMAT_MP3:
    case ASTREAM_FORMAT_AAC:
    case ASTREAM_FORMAT_AMR_NB:
    case ASTREAM_FORMAT_AMR_WB:
    default:
        if (flag & AUDIO_OUTPUT_FLAG_FAST)
            return &astream_config_fast;
        if (flag & AUDIO_OUTPUT_FLAG_DEEP_BUFF)
            return &astream_config_deepbuf;
        break;
    }

    return &astream_config_normal;
}
#else
const struct apolicy_stream_config *apolicy_get_config(
                enum astream_type type,
                enum astream_format format,
                enum astream_direct dir,
                u32 flag)
{
    if ((type == ASTREAM_TYPE_VOICE || type == ASTREAM_TYPE_VOICE_UL) &&
        format == ASTREAM_FORMAT_PCM)
        return &astream_config_normal;

    /* if voip, both UL and DL config use recod amr */
    if (type == ASTREAM_TYPE_VOIP || type == ASTREAM_TYPE_VOIP_UL) {
        if (format == ASTREAM_FORMAT_PCM)
            return &astream_config_normal;
        return &astream_config_record_amr_wb;
    }
    if (dir == ASTREAM_DIR_RECORD) {
        if (format == ASTREAM_FORMAT_AMR_NB)
            return &astream_config_record_amr_nb;
        else if (format == ASTREAM_FORMAT_AMR_WB)
            return &astream_config_record_amr_wb;
        else
            return &astream_config_record;
    }

    switch(format) {
    case ASTREAM_FORMAT_PCM:
    case ASTREAM_FORMAT_WAV:
        if (flag & AUDIO_OUTPUT_FLAG_COMPRESS)
            return &astream_config_pcm_compress;
        if (flag & AUDIO_OUTPUT_FLAG_FAST)
            return &astream_config_fast;
        if (flag & AUDIO_OUTPUT_FLAG_DEEP_BUFF)
            return &astream_config_deepbuf;
        break;
    case ASTREAM_FORMAT_MP3:
    case ASTREAM_FORMAT_AAC:
#ifdef CONFIG_HW_CODEC_SUPPORT
        if (!(flag & AUDIO_OUTPUT_FLAG_COMPRESS))
            return &astream_config_normal;
#endif
        return &astream_config_compress;
    case ASTREAM_FORMAT_AMR_NB:
        return &astream_config_compress_amr_nb;
    case ASTREAM_FORMAT_AMR_WB:
        return &astream_config_compress_amr_wb;
    default:
        break;
    }

    return &astream_config_normal;
}

#endif

const struct apolicy_stream_config *apolicy_get_compr_config(
                enum astream_type type,
                enum astream_format format,
                enum astream_direct dir,
                u32 flag)
{
    if (dir == ASTREAM_DIR_PLAY) {
        switch (format) {
        case ASTREAM_FORMAT_PCM:
        case ASTREAM_FORMAT_WAV:
            return &astream_config_pcm_compress;
        case ASTREAM_FORMAT_MP3:
        case ASTREAM_FORMAT_AAC:
            return &astream_config_compress;
        case ASTREAM_FORMAT_AMR_NB:
            return &astream_config_compress_amr_nb;
        case ASTREAM_FORMAT_AMR_WB:
            return &astream_config_compress_amr_wb;
        default:
            break;
        }
        return &astream_config_compress;
    } else {
        switch (format) {
        case ASTREAM_FORMAT_PCM:
        case ASTREAM_FORMAT_WAV:
            return &astream_config_pcm_compress;
        case ASTREAM_FORMAT_MP3:
        case ASTREAM_FORMAT_AAC:
            return &astream_config_compress;
        case ASTREAM_FORMAT_AMR_NB:
            return &astream_config_record_amr_nb;
        case ASTREAM_FORMAT_AMR_WB:
            return &astream_config_record_amr_wb;
        default:
            break;
        }
        return &astream_config_compress;
    }
}

struct astream_property *apolicy_get_stream_property(enum astream_type type)
{
    if (type >= ASTREAM_TYPE_MAX)
        return NULL;

    return (struct astream_property *)&g_astream_property[type];
}

/* TODO: Do not regarding stratagy and device, such as in voice call,
         or BT is connnected etc. */
#ifdef CONFIG_MCU_AUDIO
int apolicy_get_stream_chn(enum astream_type type,
                           enum astream_format format, int flag)
{
    int chn;

    if (type == ASTREAM_TYPE_VOICE) {
        if (flag & AUDIO_OUTPUT_FLAG_EXTERNAL)
            chn = STREAM_CHN_EXTV_P;
        else
            chn = STREAM_CHN_VOICE_DL_PCM;
    }
    else if (type == ASTREAM_TYPE_VOIP)
        chn = STREAM_CHN_VOICE_DL_PCM;
    else if (type == ASTREAM_TYPE_VOICE_UL) {
        if (flag & AUDIO_INPUT_FLAG_EXTERNAL)
            chn = STREAM_CHN_EXTV_C;
        else
            chn = STREAM_CHN_VOICE_UL_PCM;
    }
    else if (type == ASTREAM_TYPE_VOIP_UL)
        chn = STREAM_CHN_VOICE_UL_PCM;
    else if (type == ASTREAM_TYPE_RECORD) {
        if (flag & AUDIO_INPUT_FLAG_VOREC)
            chn = STREAM_CHN_VOICE_REC;
        else
            chn = STREAM_CHN_DIR_C;
    }
    else if (type == ASTREAM_TYPE_TONE) {
        if (get_strategy() == STRATEGY_CALL)
            chn = STREAM_CHN_VOICE_MIX;
        else {
            if (flag & AUDIO_OUTPUT_FLAG_EXTERNAL)
                chn = STREAM_CHN_EXT;
            else
                chn = STREAM_CHN_DIR_P;
        }
    }
    else
        chn = STREAM_CHN_DIR_P;

    return chn;
}
#else
int apolicy_get_stream_chn(enum astream_type type,
                           enum astream_format format, int flag)
{
    int chn;

    if (type == ASTREAM_TYPE_VOICE) {
        if (format == ASTREAM_FORMAT_PCM)
            chn = STREAM_CHN_VOICE_DL_COMPR_PCM;
        else
            chn = STREAM_CHN_VOICE_DL;
    }
    else if (type == ASTREAM_TYPE_VOIP)
        chn = STREAM_CHN_VOIP_DL;
    else if (type == ASTREAM_TYPE_VOICE_UL) {
        if (format == ASTREAM_FORMAT_PCM)
            chn = STREAM_CHN_VOICE_UL_COMPR_PCM;
        else
            chn = STREAM_CHN_VOICE_UL;
    }
    else if (type == ASTREAM_TYPE_VOIP_UL)
        chn = STREAM_CHN_VOIP_UL;
    else if (type == ASTREAM_TYPE_RECORD) {
        if (flag & AUDIO_INPUT_FLAG_COMPRESS) {
            chn = STREAM_CHN_OFFLD_REC;
            if(flag & AUDIO_INPUT_FLAG_VOREC) {
                chn = STREAM_CHN_COMPRC1;
            }
        }
        else
            chn = STREAM_CHN_DIR_C;
    }
#ifndef CONFIG_HW_CODEC_SUPPORT
    else if (format >= ASTREAM_FORMAT_OFFLD_BEGIN &&
             format <= ASTREAM_FORMAT_OFFLD_END)
        chn = STREAM_CHN_OFFLD_COMPR;
#endif
    else {
        if (flag & AUDIO_OUTPUT_FLAG_COMPRESS)
            chn = STREAM_CHN_OFFLD_PCM;
        else if (flag & AUDIO_OUTPUT_FLAG_EXTERNAL)
            chn = STREAM_CHN_EXT;
        else
            chn = STREAM_CHN_DIR_P;
    }

    return chn;
}
#endif

static void apolicy_set_stream_type(enum astream_type type, int on)
{
    /* TODO: add lock */
    if (on)
        apolicy_runtime.stream_type |= (1 << type);
    else
        apolicy_runtime.stream_type &= (~(1 << type));
}

void apolicy_add_stream_handle(u32 handle)
{
    struct audio_handle *audio_handle;
    int i;

    for (i = 0; i < APOLICY_STREAM_MAX; i++) {
        if (apolicy_runtime.hlist[i] == 0) {
            apolicy_runtime.hlist[i] = handle;
            audio_handle = (struct audio_handle *)&handle;
            apolicy_set_stream_type(audio_handle->type, TRUE);

            return;
        }
    }

    AIDXE(0x0c67bc, "x", "apolicy list full, hanlde 0x%x ignored\n", handle);
}

void apolicy_delete_stream_handle(u32 handle)
{
    struct audio_handle *audio_handle;
    int i;

    for (i = 0; i < APOLICY_STREAM_MAX; i++) {
        if (apolicy_runtime.hlist[i] == handle) {
            apolicy_runtime.hlist[i] = 0;
            audio_handle = (struct audio_handle *)&handle;
            apolicy_set_stream_type(audio_handle->type, FALSE);

            return;
        }
    }
}

#ifdef CONFIG_MCU_AUDIO
static bool need_update_music_param(void)
{
    return TRUE;
}
#else
static bool need_update_music_param(void)
{
#ifndef CONFIG_HW_CODEC_SUPPORT
    struct audio_handle *audio_handle;
    int i;

    for (i = 0; i < APOLICY_STREAM_MAX; i++) {
        audio_handle = (struct audio_handle *)&apolicy_runtime.hlist[i];
        if ((audio_handle->fmt >= ASTREAM_FORMAT_OFFLD_BEGIN &&
            audio_handle->fmt <= ASTREAM_FORMAT_OFFLD_END) ||
            audio_handle->flag & AUDIO_OUTPUT_FLAG_COMPRESS)
            return TRUE;
    }
#endif
    return FALSE;
}
#endif

static bool need_update_record_param(void)
{
    struct audio_handle *audio_handle;
    int i;

    for (i = 0; i < APOLICY_STREAM_MAX; i++) {
        audio_handle = (struct audio_handle *)&apolicy_runtime.hlist[i];
        if (audio_handle->type == ASTREAM_TYPE_RECORD)
            return TRUE;
    }
    return FALSE;
}

static u32 get_stream_type(void)
{
    return apolicy_runtime.stream_type;
}

static enum apolicy_strategy get_strategy(void)
{
    u32 stream_type = get_stream_type();

    if (stream_type & (1 << ASTREAM_TYPE_VOICE | 1 << ASTREAM_TYPE_VOIP))
        return STRATEGY_CALL;
    if (stream_type & (1 << ASTREAM_TYPE_RING))
        return STRATEGY_RING;
    if (stream_type & (1 << ASTREAM_TYPE_ALARM))
        return STRATEGY_ALARM;

    return STRATEGY_NORMAL;
}

static u32 get_out_device_by_strategy(enum apolicy_strategy strategy)
{
    u32 supported_device = apolicy_runtime.supported_out_device;
    enum audio_device device = AUDIO_DEVICE_NONE;

    switch (strategy) {
    case STRATEGY_CALL:
        if (supported_device & AUDIO_DEVICE_OUT_BT_SCO &&
            apolicy_runtime.bt_sco_on)
            device = AUDIO_DEVICE_OUT_BT_SCO;
        else if ((supported_device &
                    (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE)) &&
                 (apolicy_runtime.hp_on || apolicy_runtime.hs_on))
            device = AUDIO_DEVICE_OUT_HEADSET;
        else if (supported_device & AUDIO_DEVICE_OUT_EARPIECE)
            device = AUDIO_DEVICE_OUT_EARPIECE;
        else if (supported_device & AUDIO_DEVICE_OUT_SPEAKER)
            device = AUDIO_DEVICE_OUT_SPEAKER;
        else
            device = AUDIO_DEVICE_OUT_EARPIECE;
        break;
    case STRATEGY_RING:
        if (supported_device & AUDIO_DEVICE_OUT_BT_A2DP &&
            apolicy_runtime.bt_a2dp_on)
            device = AUDIO_DEVICE_OUT_BT_A2DP;
        else if ((supported_device &
                    (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADSET)) &&
                 (apolicy_runtime.hp_on || apolicy_runtime.hs_on))
            device = AUDIO_DEVICE_OUT_HEADSET;
        if (supported_device & AUDIO_DEVICE_OUT_SPEAKER)
            device |= AUDIO_DEVICE_OUT_SPEAKER;
        else if (supported_device & AUDIO_DEVICE_OUT_EARPIECE)
            if (!device)
                device |= AUDIO_DEVICE_OUT_EARPIECE;
        if (!device)
            device = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    case STRATEGY_ALARM:
        if (supported_device & AUDIO_DEVICE_OUT_SPEAKER)
            device = AUDIO_DEVICE_OUT_SPEAKER;
        else if (supported_device & AUDIO_DEVICE_OUT_EARPIECE)
            device = AUDIO_DEVICE_OUT_EARPIECE;
        else if ((supported_device & AUDIO_DEVICE_OUT_HEADSET) &&
            (apolicy_runtime.hp_on || apolicy_runtime.hs_on))
            device = AUDIO_DEVICE_OUT_HEADSET;
        else if (supported_device & AUDIO_DEVICE_OUT_BT_A2DP &&
            apolicy_runtime.bt_a2dp_on)
            device = AUDIO_DEVICE_OUT_BT_A2DP;
        else
            device = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    case STRATEGY_NORMAL:
    default:
        if ((supported_device & AUDIO_DEVICE_OUT_BT_A2DP) &&
            apolicy_runtime.bt_a2dp_on)
            device = AUDIO_DEVICE_OUT_BT_A2DP;
        else if ((supported_device &
                    (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE)) &&
                 (apolicy_runtime.hp_on || apolicy_runtime.hs_on))
            device = AUDIO_DEVICE_OUT_HEADSET;
        else if (supported_device & AUDIO_DEVICE_OUT_SPEAKER)
            device = AUDIO_DEVICE_OUT_SPEAKER;
        else if (supported_device & AUDIO_DEVICE_OUT_EARPIECE)
            device = AUDIO_DEVICE_OUT_EARPIECE;
        else
            device = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    }

    AIDXW(0x1a463e, "dd", "out strategy=%d, device=%d\n", strategy, device);

    return (u32)device;
}

static u32 get_in_device_by_strategy(enum apolicy_strategy
                                                   strategy)
{
    u32 supported_device = apolicy_runtime.supported_in_device;
    enum audio_device device = AUDIO_DEVICE_NONE;

    /* clear flag bit */
    supported_device &= ~AUDIO_DEVICE_BIT_IN;

    switch (strategy) {
    case STRATEGY_CALL:
        if (supported_device & AUDIO_DEVICE_IN_BT_SCO &&
            apolicy_runtime.bt_sco_on)
            device = AUDIO_DEVICE_IN_BT_SCO;
        else if ((supported_device & AUDIO_DEVICE_IN_HEADPHONE) &&
                 apolicy_runtime.hp_on)
            device = AUDIO_DEVICE_IN_HEADPHONE;
        else if (supported_device &
                 (AUDIO_DEVICE_IN_MIC | AUDIO_DEVICE_IN_BACK_MIC)) {
            device = apolicy_runtime.supported_in_device &
                        (AUDIO_DEVICE_IN_MIC | AUDIO_DEVICE_IN_BACK_MIC);
        }
        else
            device = AUDIO_DEVICE_IN_MIC;
        break;
    case STRATEGY_NORMAL:
    default:
        if ((supported_device & AUDIO_DEVICE_IN_HEADPHONE) &&
                 apolicy_runtime.hp_on)
            device = AUDIO_DEVICE_IN_HEADPHONE;
        else if (supported_device & AUDIO_DEVICE_IN_MIC)
            device = AUDIO_DEVICE_IN_MIC;
        else if (supported_device & AUDIO_DEVICE_IN_BACK_MIC)
            device = AUDIO_DEVICE_IN_BACK_MIC;
        else
            device = AUDIO_DEVICE_IN_MIC;
        break;
    }

    AIDXW(0x187b5d, "dd", "in strategy=%d, device=%d\n", strategy, device);

    return (u32)device;
}

static void get_new_device(u32 *out_device, u32 *in_device)
{
    if (apolicy_runtime.force_out_device)
        *out_device = apolicy_runtime.force_out_device;
    else
        *out_device = get_out_device_by_strategy(get_strategy());

    if (apolicy_runtime.force_in_device)
        *in_device = apolicy_runtime.force_in_device;
    else {
        *in_device = get_in_device_by_strategy(get_strategy());
    }
}

static void get_current_device(u32 *out_device, u32 *in_device)
{
    *out_device = apolicy_runtime.out_device;
    *in_device = apolicy_runtime.in_device;
}

static void set_current_device(u32 out_device, u32 in_device)
{
    apolicy_runtime.out_device = out_device;
    apolicy_runtime.in_device = in_device;
}

void apolicy_get_current_device(u32 *out_device, u32 *in_device)
{
    get_current_device(out_device, in_device);
}

void apolicy_device_update(void)
{
    u32 cur_out_device, cur_in_device, new_out_device, new_in_device;
    u32 out_device, in_device;
    int i = 0;

    /* TODO: add lock */
    get_current_device(&cur_out_device, &cur_in_device);
    get_new_device(&new_out_device, &new_in_device);

    AIDXW(0x055553, "xxxx", "audio new device: out=0x%x, in=0x%x, old: out=0x%x, in=0x%x\n",
          new_out_device, new_in_device, cur_out_device, cur_in_device);

    /* backup */
    out_device = new_out_device;
    in_device = new_in_device;

    if (cur_out_device != out_device) {
        while (cur_out_device || out_device) {
            /* if current and new both open, do nothing */
            if ((cur_out_device & 0x1) && (out_device & 0x1))
                ; /* do nothing */
            /* current open, but new not open, close it */
            else if (cur_out_device & 0x1)
                audio_route_set(1 << i, FALSE);
            /* current close, but new open, open it */
            else if (out_device & 0x1)
                audio_route_set(1 << i, TRUE);
            i++;
            cur_out_device >>= 1;
            out_device >>= 1;
        }
    }
    if (cur_in_device != in_device) {
        i = 0;
        in_device &= ~AUDIO_DEVICE_BIT_IN;
        while (cur_in_device || in_device) {
            /* if current and new both open, do nothing */
            if ((cur_in_device & 0x1) && (in_device & 0x1))
                ; /* do nothing */
            /* current open, but new not open, close it */
            else if (cur_in_device & 0x1)
                audio_route_set((1 << i) | AUDIO_DEVICE_BIT_IN, FALSE);
            /* current close, but new open, open it */
            else if (in_device & 0x1)
                audio_route_set((1 << i) | AUDIO_DEVICE_BIT_IN, TRUE);

            i++;
            cur_in_device >>= 1;
            in_device >>= 1;
        }
    }
    set_current_device(new_out_device, new_in_device);
}

void apolicy_device_standby(void)
{
    u32 out_device, in_device;
    int i = 0;

    /* TODO: add lock */
    get_current_device(&out_device, &in_device);
    while (out_device) {
        if (out_device & 0x1)
            audio_route_set(1 << i, FALSE);
        i++;
        out_device >>= 1;
    }

    i = 0;
    in_device &= ~AUDIO_DEVICE_BIT_IN;
    while (in_device) {
        if (in_device & 0x1)
            audio_route_set((1 << i) | AUDIO_DEVICE_BIT_IN, FALSE);
        i++;
        in_device >>= 1;
    }
    set_current_device(0, 0);
}

void apolicy_set_force_out_device(u32 out_device)
{
    apolicy_runtime.force_out_device = out_device;
    apolicy_device_update();
}

void apolicy_set_force_in_device(u32 in_device)
{
    apolicy_runtime.force_in_device = in_device;
    apolicy_device_update();
}

u32 apolicy_get_music_md_id(void)
{
    u32 out_device, in_device;

    get_current_device(&out_device, &in_device);

    /* spk + hp */
    if (out_device & AUDIO_DEVICE_OUT_SPEAKER &&
        (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE)))
        return AUD_MD_MUSIC_SPK_EP;
    else if (out_device & AUDIO_DEVICE_OUT_SPEAKER)
        return AUD_MD_MUSIC_SPK;
    else if (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE))
        return AUD_MD_MUSIC_EP;
    else if (out_device & AUDIO_DEVICE_OUT_EARPIECE)
        return AUD_MD_MUSIC_RCV;

    return 0;
}

u32 apolicy_get_rec_md_id(void)
{
    u32 out_device, in_device;

    get_current_device(&out_device, &in_device);

    if (in_device & AUDIO_DEVICE_IN_MIC)
        return AUD_MD_REC_MAIN_MIC;
    if (in_device & AUDIO_DEVICE_IN_HEADPHONE)
        return AUD_MD_REC_EP_MIC;

    return 0;
}

u32 apolicy_get_voice_md_id(void)
{
    u32 out_device, in_device;
    u32 stream_type = get_stream_type();

    get_current_device(&out_device, &in_device);

    if (stream_type & (1 << ASTREAM_TYPE_VOICE)) {
        /* spk + hp */
        if (out_device & AUDIO_DEVICE_OUT_SPEAKER &&
            (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE)))
            return AUD_MD_VOLTE_SPK;
        else if (out_device & AUDIO_DEVICE_OUT_SPEAKER)
            return AUD_MD_VOLTE_SPK;
        else if (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE))
            return AUD_MD_VOLTE_EP;
        else if (out_device & AUDIO_DEVICE_OUT_EARPIECE)
            return AUD_MD_VOLTE_RCV;
        else
            return AUD_MD_VOLTE_RCV;
    }
    if (stream_type & (1 << ASTREAM_TYPE_VOIP)) {
        /* spk + hp */
        if (out_device & AUDIO_DEVICE_OUT_SPEAKER &&
            (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE)))
            return AUD_MD_VOIP_SPK;
        else if (out_device & AUDIO_DEVICE_OUT_SPEAKER)
            return AUD_MD_VOIP_SPK;
        else if (out_device & (AUDIO_DEVICE_OUT_HEADSET | AUDIO_DEVICE_OUT_HEADPHONE))
            return AUD_MD_VOIP_EP;
        else if (out_device & AUDIO_DEVICE_OUT_EARPIECE)
            return AUD_MD_VOIP_RCV;
        else
            return AUD_MD_VOIP_RCV;
    }

    return 0;
}

void apolicy_param_update(void)
{
    enum apolicy_strategy strategy;
    u32 stream_type = get_stream_type();
    u32 md_id = 0;

    strategy = get_strategy();
    switch (strategy) {
    case STRATEGY_CALL:
        md_id = apolicy_get_voice_md_id();
        break;
    case STRATEGY_NORMAL:
        if (stream_type & (1 << ASTREAM_TYPE_RECORD)) {
            if (need_update_record_param()) {
                md_id = apolicy_get_rec_md_id();
                break;
            } else
                return;
        }
    case STRATEGY_RING:
    case STRATEGY_ALARM:
    case STRATEGY_CAM:
        if (!need_update_music_param())
            return;
        md_id = apolicy_get_music_md_id();
        AIDXI(0x034d7e, "d", "apolicy param update, md=%d\n", md_id);
        break;
    default:
        break;
    }
    AIDXE(0x10a57d, "dd", "strategy=%d, md_id=%d\n", strategy, md_id);
    if (md_id)
        audio_md_set(md_id);
}

const static u32 apolicy_coexist_table[] = {
    BIT(ASTREAM_TYPE_VOICE) | BIT(ASTREAM_TYPE_VOICE_UL) | BIT(ASTREAM_TYPE_TONE) | BIT(ASTREAM_TYPE_RECORD),
    BIT(ASTREAM_TYPE_VOIP) | BIT(ASTREAM_TYPE_VOIP_UL) | BIT(ASTREAM_TYPE_TONE),
    BIT(ASTREAM_TYPE_RECORD) | BIT(ASTREAM_TYPE_TONE),
    BIT(ASTREAM_TYPE_RECORD) | BIT(ASTREAM_TYPE_MUSIC),
};

int apolicy_conflict_check(enum astream_type type,
                           enum astream_format format, int flag)
{
    u32 co_type, co_bit, i;

    co_type = get_stream_type();
    AIDXD(0x007291, "xddx", "co_type=0x%x, type=%d, format=%d, flag=0x%x\n",
          co_type, type, format, flag);

    if (!co_type)
        return 0;

    if (apolicy_runtime.stream_type & BIT(type)) {
        AIDXE(0x065616, "d", "type(%d already opended\n", type);
        return AIC_EACCES;
    }

    co_bit = co_type | BIT(type);
    for (i = 0; i < ARRAY_SIZE(apolicy_coexist_table); i++)
        if ((co_bit & apolicy_coexist_table[i]) == co_bit) {
            AIDXI(0x1ab192, "dxd", "type(%d) allow coexist(0x%x), id=%d",
                  type, apolicy_coexist_table[i], i);
            return 0;
        }

    AIDXI(0x0ce829, "dx", "type(%d) not allowed coexit, cur=0x%x\n", type, co_type);

    return AIC_EPERM;
}

int apolicy_init(void)
{
    apolicy_runtime.supported_out_device = SUPPORT_AUDIO_DEVICE_OUT;
    apolicy_runtime.supported_in_device = SUPPORT_AUDIO_DEVICE_IN;

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(apolicy_init);

