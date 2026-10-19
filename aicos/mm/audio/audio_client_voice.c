/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is voice client file by which we can get send&receive data.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x162419
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_init.h>
#include <alog_api.h>
#include <aic_errno.h>
#include <snd_ipc.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_sink_record.h"
#include "codec_mng.h"
#include "voice_proc.h"

#define CLIENT_LOCK
#define CLIENT_SEMPH_SET(s)    if (s) aos_put_semaphore(s);
#define CLIENT_SEMPH_GET(s)    if (s) aos_get_semaphore(s, 21);

#define read_amr_fr_head(nibble)  (((nibble)>>3)&0xf)
#define read_amr_nw_head(nibble)  (((nibble)>>7)&1)
#define read_amr_Q_head(nibble)  (((nibble)>>2)&1)
#define read_amr_sid_head(nibble)  (((nibble)>>1)&1)
#define read_amr_ext_head(nibble)  (((nibble))&1)

struct client_voice {
    void *hvp;
    struct codec_info info;
    aos_semaphore_ptr   semph;
    u32 ref_cnt;
};

struct client_voice *g_client_voice;

static struct client_voice *get_client_voice(void)
{
    struct client_voice *client_voice;

    if (g_client_voice)
        return g_client_voice;

    client_voice = audio_malloc(sizeof(*client_voice));
    if (!client_voice) {
        AIDXE(0x0c77ce, "", "get client voice buf failed, no memory!\n");
        return 0;
    }
    memset(client_voice, 0, sizeof(*client_voice));

    client_voice->semph = aos_create_semaphore("client voice semph", 0);

    g_client_voice = client_voice;

    return client_voice;
}

static int voice_get_data(void *obj, u8 *buffer, u32 get_size,
                          u32 *real_size, u32 *fmt, u32 *mode)
{
    int ret;
    u8 hd_byte;

    ret = snd_ipc_read(SND_IPC_CH_DL_AP, buffer);
    if (ret < 0) {
        if (*fmt == ASTREAM_FORMAT_AMR_WB)
            buffer[0] = 0xfc;
        else
            buffer[0] = 0x7c;

        *real_size = 1;
        AIDXE(0x18973f, "d", "voice client no data, fill mute, ret=%d\n", ret);

        return 0;
    }

    hd_byte = buffer[0];
    if (hd_byte == 0xdd) {
        *fmt = ASTREAM_FORMAT_VOICE_DTMF;
        return 0;
    } else if (read_amr_ext_head(hd_byte)) {
        AIDXI(0x05d8ef, "", "voice client, switch mode\n");
        *mode = buffer[1];
        memcpy(buffer + 1, buffer + 2, ret - 2);
        ret--;
    }

    buffer[0] &= (~(BIT_7 | BIT_1 | BIT_0));
    *real_size = ret;

    if (read_amr_nw_head(hd_byte))
        *fmt = ASTREAM_FORMAT_AMR_WB;
    else
        *fmt = ASTREAM_FORMAT_AMR_NB;

    return 0;
}

int voice_send_data(void *obj, u8 *buf, u32 cnt)
{
    int ret;

    ret = snd_ipc_write(SND_IPC_CH_UL_AP, buf, cnt);
    if (ret < 0)
        AIDXE(0x0af2b3, "d", "voice client send data failed, ret=%d\n", ret);

    return ret;
}

static u32 voice_open(enum astream_format format, void *msg)
{
    struct client_voice *client_voice = get_client_voice();
    struct audio_msg_open *pmsg = msg;
    struct codec_info *info;

    if (!client_voice) {
        AIDXE(0x1d222e, "", "no voice client\n");
        return 0;
    }

    if (client_voice->hvp) {
        client_voice->ref_cnt++;
        return (u32)client_voice;
    }
    info = &client_voice->info;
    info->srate = pmsg->sample_rate;
    info->chn = pmsg->chn;
    info->fmt = format;
    info->brate = pmsg->bit_rate;
    info->wid = 2;
    info->flag = CODEC_INFO_NO_HEADER | CODEC_INFO_FRAME_MODE | CODEC_INFO_DTX;
    client_voice->hvp = voice_proc_open(client_voice, info,
                                        voice_get_data, voice_send_data);
    if (!client_voice->hvp) {
        AIDXE(0x1061c9, "", "voice client, voice process open failed\n");
        return 0;
    }

    AIDXI(0x01b4ac, "dd", "client voice open sucess, fmt=%d, srate=%d\n",
          info->fmt, info->srate);
    client_voice->ref_cnt++;

    return (u32)client_voice;
}

static int voice_close(u32 obj)
{
    struct client_voice *client_voice = (struct client_voice *)obj;

    if (!client_voice) {
        AIDXE(0x09975e, "", "client voice is null\n");

        return 0;
    }
    if (client_voice->ref_cnt)
        client_voice->ref_cnt--;

    if (client_voice->ref_cnt == 0 && client_voice->hvp) {
        voice_proc_close(client_voice->hvp);
        client_voice->hvp = 0;
    }
    return 0;
}

static int voice_start(u32 obj, void *aplay_data)
{
    return 0;
}

static int voice_stop(u32 obj)
{
    return 0;
}

static int voice_callback(u32 obj, int event, char *buffer, u32 *rw_size)
{
    struct client_voice *client_voice = (struct client_voice *)obj;
    int ret = 0;

    if (!client_voice->hvp) {
        AIDXI(0x052dea, "", "voice callback failed, no process handle\n");
        return AIC_EPIPE;
    }
    switch (event) {
    case AUDIO_CLIENT_CMD_GET_MORE:
        CLIENT_SEMPH_GET(client_voice->semph);
        ret = voice_proc_get(client_voice->hvp, buffer, rw_size);
        if (ret < 0)
            AIDXE(0x061217, "d", "voice get failed, ret=%d\n", ret);
        break;
    case AUDIO_CLIENT_CMD_DATA_NOTIFY:
        ret = voice_proc_send(client_voice->hvp, buffer, *rw_size);
        if (ret < 0)
            AIDXE(0x00d830, "d", "voice send failed, ret=%d\n", ret);
        CLIENT_SEMPH_SET(client_voice->semph);

        break;
    default:
        break;
    }

    return ret;
}

struct audio_client audio_client_voice = {
    .type = ASTREAM_TYPE_VOICE,
    .aplay_type = APLAY_TYPE_NONE,
    .open = voice_open,
    .start = voice_start,
    .stop = voice_stop,
    .close = voice_close,
    .cb = voice_callback
};

static int audio_client_voice_init(void)
{
    audio_client_register(&audio_client_voice);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(audio_client_voice_init);

