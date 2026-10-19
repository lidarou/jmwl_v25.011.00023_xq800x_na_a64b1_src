/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio system main file, it provides all audio APIs for
 *  APP or other module.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04c426
#define LOG_MODULE    AMOD_AUDIO

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_cmd_api.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink_record.h"
#include "audio_sink.h"
#include "audio_util.h"
#include "voice_mng.h"

struct audio_system_ctx {
    aos_mutex_ptr mutex;
    ACLIENT tone_handle;

    u8 *voip_buf;
    u8 lp_state;
    aos_thread_id lp_th;
};

#define CONFIG_LOOP_TEST_VOIP       1

#define ALOCK       mutex_lock(g_audio_system_ctx.mutex);
#define AUNLOCK     mutex_unlock(g_audio_system_ctx.mutex);

#define C           g_audio_system_ctx

static struct audio_system_ctx g_audio_system_ctx;
u32 is_voice_stream_on(void);

static int audio_user_callback_default(u32 data, enum astream_cb_event event)
{
    return audio_client_close_async((ACLIENT)data);
}

ACLIENT audio_open(enum astream_type type, enum aplay_type aplay_type, u32 flag,
                   u32 sample_rate, u32 bit_rate, enum astream_format format,
                   enum astream_channel chn,
                   enum astream_direct dir, enum astream_source source)
{
    struct audio_msg_open msg;

    msg.cmd = AUDIO_CMD_OPEN;
    msg.type = type;
    msg.aplay_type = aplay_type;
    msg.format = format;
    msg.chn = chn;
    msg.source = source;
    msg.dir = dir;
    msg.flag = flag;
    msg.sample_rate = sample_rate;
    msg.bit_rate = bit_rate;
#ifdef CONFIG_MCU_AUDIO
    if (dir == ASTREAM_DIR_RECORD)
        msg.flag &= ~AUDIO_INPUT_FLAG_COMPRESS;
    else
        msg.flag &= ~AUDIO_OUTPUT_FLAG_COMPRESS;
#endif
    return audio_client_open(type, aplay_type, format, &msg);
}

ACLIENT audio_open_output(enum astream_type type, enum aplay_type aplay_type,
                          enum astream_format format, u32 flag)
{
#ifndef CONFIG_HW_CODEC_SUPPORT
    if (format >= ASTREAM_FORMAT_OFFLD_BEGIN &&
            format <= ASTREAM_FORMAT_OFFLD_END)
        flag |= AUDIO_OUTPUT_FLAG_COMPRESS;
#endif
    return audio_open(type, aplay_type, flag, 0, 0,
                      format, 0, ASTREAM_DIR_PLAY, 0);
}

ACLIENT audio_open_input(enum aplay_type aplay_type, enum astream_format format,
                         u32 sample_rate, enum astream_channel chn,
                         enum astream_source source, u32 flag)
{
    if (format >= ASTREAM_FORMAT_OFFLD_BEGIN &&
            format <= ASTREAM_FORMAT_OFFLD_END)
        flag |= AUDIO_INPUT_FLAG_COMPRESS;

    return audio_open(ASTREAM_TYPE_RECORD, aplay_type, flag, sample_rate, 0,
                      format, chn, ASTREAM_DIR_RECORD, source);
}

ACLIENT audio_open_output_pcm(enum astream_type type, enum aplay_type aplay_type,
                              u32 flag, u32 sample_rate, enum astream_channel chn,
                              enum astream_direct dir)
{
    return audio_open(type, aplay_type, flag, sample_rate, 0,
                      ASTREAM_FORMAT_PCM, chn, ASTREAM_DIR_PLAY, 0);
}

int audio_play(ACLIENT handle)
{
    struct audio_msg_play msg;

    msg.cmd = AUDIO_CMD_PLAY;
    msg.aplay_type = APLAY_TYPE_NONE;

    return audio_client_play(handle, &msg);
}

/* char *buf, buflen, void *cb, cb_data */
int audio_play_buf(ACLIENT handle, char *buf, u32 buf_len,
                   void *user_cb, u32 cb_data)
{
    struct audio_msg_play msg;
    struct audio_play_sub_buf sub;

    msg.cmd = AUDIO_CMD_PLAY;
    msg.aplay_type = APLAY_TYPE_BUF;
    msg.aplay_data = &sub;

    memset(&sub, 0, sizeof(sub));
    sub.buf = buf;
    sub.buf_len = buf_len;
    if (user_cb) {
        sub.user_cb = user_cb;
        sub.cb_data = cb_data;
    } else {
        sub.user_cb = audio_user_callback_default;
        sub.cb_data = handle;
    }
    return audio_client_play(handle, &msg);
}

int audio_play_file(ACLIENT handle, char *file_name, void *user_cb, u32 cb_data)
{
    struct audio_msg_play msg;
    struct audio_play_sub_file sub;

    msg.cmd = AUDIO_CMD_PLAY;
    msg.aplay_type = APLAY_TYPE_FILE;
    msg.aplay_data = &sub;

    memset(&sub, 0, sizeof(sub));
    sub.file_name = file_name;
    if (user_cb) {
        sub.user_cb = user_cb;
        sub.cb_data = cb_data;
    } else {
        sub.user_cb = audio_user_callback_default;
        sub.cb_data = handle;
    }

    return audio_client_play(handle, &msg);
}

/* TBD */
int audio_play_tts(ACLIENT handle, char *buf, u32 buf_len,
                   void *user_cb, u32 cb_data)
{
    struct audio_msg_play msg;
    struct audio_play_sub_tts sub;

    msg.cmd = AUDIO_CMD_PLAY;
    msg.aplay_type = APLAY_TYPE_TTS;
    msg.aplay_data = &sub;

    sub.buf = buf;
    sub.buf_len = buf_len;
    if (user_cb) {
        sub.user_cb = user_cb;
        sub.cb_data = cb_data;
    } else {
        sub.user_cb = audio_user_callback_default;
        sub.cb_data = handle;
    }

    return audio_client_play(handle, &msg);
}

/* TBD: online play */
int audio_play_stream(ACLIENT handle, void *user_cb, u32 cb_data)
{
    struct audio_msg_play msg;
    struct audio_play_sub_stream sub;

    msg.cmd = AUDIO_CMD_PLAY;
    msg.aplay_type = APLAY_TYPE_STREAM;
    msg.aplay_data = &sub;

    memset(&sub, 0, sizeof(sub));
    if (user_cb) {
        sub.user_cb = user_cb;
        sub.cb_data = cb_data;
    } else {
        sub.user_cb = audio_user_callback_default;
        sub.cb_data = handle;
    }
    sub.abuf = audio_client_get_buf(handle);

    return audio_client_play(handle, &msg);
}

int audio_play_tone_ex(enum tone_type tone_type, u32 duation, u32 flag)
{
    struct audio_msg_play msg;
    struct audio_play_sub_tone sub;
    int ret;

    ALOCK
    if (!C.tone_handle) {
        C.tone_handle = audio_open(ASTREAM_TYPE_TONE, APLAY_TYPE_TONE,
                                   flag, 16000, 0,
                                   ASTREAM_FORMAT_PCM, ASTREAM_CHN_MONO,
                                   ASTREAM_DIR_PLAY, ASTREAM_SOURCE_DEFAULT);
        if (C.tone_handle == 0) {
            AIDXE(0x1bcbaa, "", "play tone failed, create handle failed\n");
            AUNLOCK

            return AIC_EPERM;
        }
    }
    msg.cmd = AUDIO_CMD_PLAY;
    msg.handle = C.tone_handle;
    msg.aplay_type = APLAY_TYPE_TONE;
    msg.aplay_data = &sub;

    sub.tone_id = tone_type;
    sub.duation = duation;

    ret = audio_client_play(C.tone_handle, &msg);

    AUNLOCK

    return ret;
}

int audio_play_tone(enum tone_type tone_type, u32 duation)
{
#ifdef CONFIG_MCU_AUDIO
    return audio_play_tone_ex(tone_type, duation, 0);
#else
    return audio_play_tone_ex(tone_type, duation, AUDIO_OUTPUT_FLAG_COMPRESS);
#endif
}

int audio_stop_tone(void)
{
    int ret = 0;

    ALOCK

    if (C.tone_handle) {
        audio_stop(C.tone_handle);
        ret = audio_close(C.tone_handle);
        C.tone_handle = 0;
    }

    AUNLOCK

    return ret;
}

static ACLIENT g_voice_dl_handle = 0;
static ACLIENT g_voice_ul_handle = 0;
static BOOL g_voice_stream_on = 0;

void audio_set_voice_stream(BOOL is_on)
{
    g_voice_stream_on = is_on;
}

static int vs_set(int argc, char **argv)
{
    if (argc > 0)
        audio_set_voice_stream(*argv[0] != '0');

    CONS_OUT("voice stream %s\n", is_voice_stream_on() ? "on" : "off");

    return 0;
}
CONSOLE_EXPORT_CMD(vs_set, vs_set, vs_set test);

u32 is_voice_stream_on(void)
{
    return g_voice_stream_on;
}

int audio_start_voice(BOOL is_dl_on, BOOL is_ul_on)
{
    int ret = 0;
    u32 rate, brate, srate = 8000, flag = 0;
    enum astream_format fmt  = ASTREAM_FORMAT_AMR_NB;

    ALOCK

    rate = audio_get_voice_rate();
    brate = rate & 0xFFFF;

    if (is_voice_stream_on()) {
        fmt = ASTREAM_FORMAT_PCM;
        srate = 16000;
    } else {
#ifdef CONFIG_VOICE_SRATE_DYNAMIC
        if ((rate >> 16) == 16000)
            fmt = ASTREAM_FORMAT_AMR_WB;
        /* sample rate keep 16000 always */
        srate = 16000;
#else
        fmt = ASTREAM_FORMAT_AMR_WB;
        srate = 16000;
#endif
    }

    AIDXI(0x172c32, "dd", "start voice, srate=%d, brate=%d\n", srate, brate);

    if (is_ul_on && !g_voice_ul_handle) {
#ifdef CONFIG_MCU_AUDIO
        if (is_voice_stream_on())
            flag = AUDIO_INPUT_FLAG_EXTERNAL;
#else
        flag = AUDIO_INPUT_FLAG_COMPRESS;
#endif
        g_voice_ul_handle = audio_open(ASTREAM_TYPE_VOICE_UL, APLAY_TYPE_STREAM,
                                       flag, srate, brate,
                                       fmt,
                                       ASTREAM_CHN_MONO,
                                       ASTREAM_DIR_RECORD,
                                       ASTREAM_SOURCE_DEFAULT);
        if (!g_voice_ul_handle) {
            AIDXE(0x1090ca, "", "start voice ul failed, create handle failed\n");
            ret = AIC_EIO;

            goto err_proc;
        }

        ret = audio_play(g_voice_ul_handle);
        if (ret) {
            audio_close(g_voice_ul_handle);
            g_voice_ul_handle = 0;

            AIDXE(0x1cefe5, "d", "voice ul play failed, ret=%d\n", ret);
            goto err_proc;
        }
    }

    if (is_dl_on && !g_voice_dl_handle) {
#ifdef CONFIG_MCU_AUDIO
        if (is_voice_stream_on())
            flag = AUDIO_OUTPUT_FLAG_EXTERNAL;
#else
        flag = AUDIO_OUTPUT_FLAG_COMPRESS;
#endif
        g_voice_dl_handle = audio_open(ASTREAM_TYPE_VOICE, APLAY_TYPE_STREAM,
                                       flag, srate, brate,
                                       fmt,
                                       ASTREAM_CHN_MONO,
                                       ASTREAM_DIR_PLAY,
                                       ASTREAM_SOURCE_DEFAULT);
        if (!g_voice_dl_handle) {
            AIDXE(0x143b28, "", "start voice dl failed, create handle failed\n");
            ret = AIC_EIO;
            goto err_proc;
        }

        ret = audio_play(g_voice_dl_handle);
        if (ret) {
            audio_close(g_voice_dl_handle);
            g_voice_dl_handle = 0;

            AIDXE(0x15651c, "d", "voice dl play failed, ret=%d\n", ret);

            goto err_proc;
        }
    }

    voice_route_notify(TRUE);

#ifdef CONFIG_VOICE_STREAM
    if (is_voice_stream_on())
        voice_stream_start();
#endif

err_proc:
    AUNLOCK

    return ret;
}

int audio_stop_voice(void)
{
    int ret;

    ALOCK

    voice_route_notify(FALSE);

#ifdef CONFIG_VOICE_STREAM
    if (is_voice_stream_on())
        voice_stream_stop();
#endif

    if (g_voice_dl_handle) {
        ret = audio_stop(g_voice_dl_handle);
        if (ret)
            AIDXE(0x0aec18, "d", "voice dl stop failed, ret=%d\n", ret);

        ret = audio_close(g_voice_dl_handle);
        if (ret)
            AIDXE(0x0712e6, "d", "voice dl close failed, ret=%d\n", ret);

        g_voice_dl_handle = 0;
    }
    if (g_voice_ul_handle) {
        ret = audio_stop(g_voice_ul_handle);
        if (ret)
            AIDXE(0x0306a8, "d", "voice ul stop failed, ret=%d\n", ret);

        ret = audio_close(g_voice_ul_handle);
        if (ret)
            AIDXE(0x1a1f4a, "d", "voice ul close failed, ret=%d\n", ret);
        g_voice_ul_handle = 0;
    }

    AUNLOCK

    return 0;
}

int audio_stop(ACLIENT handle)
{
    return audio_client_stop(handle);
}

int audio_close(ACLIENT handle)
{
    return audio_client_close(handle);
}

int audio_set_param(ACLIENT handle)
{
    return 0;
}

int audio_pause(ACLIENT handle)
{
    return audio_client_pause(handle);
}

int audio_resume(ACLIENT handle)
{
    return audio_client_resume(handle);
}

int audio_seek(ACLIENT handle, u32 timestamp)
{
    return audio_client_seek(handle, timestamp);
}

int audio_write(ACLIENT handle, char *buf, u32 len, u32 wait_time)
{
    char *dest_buf;
    u32 buf_len = len, copy = 0, copied_total = 0;
    struct asink_buf *abuf;

    abuf = audio_client_get_buf(handle);
    if (!abuf) {
        AIDXE(0x1e4f7e, "", "audio write failed, no abuf!\n");
        return AIC_EPERM;
    }

    while (len > 0) {
        buf_len = len;
        dest_buf = abuf_write_get(abuf, &buf_len, wait_time);
        if (!dest_buf) {
            AIDXE(0x1d27ab, "", "audio write failed, can't get buf!\n");
            return copied_total;
        }

        copy = len > buf_len ? buf_len : len;
        memcpy(dest_buf, buf, copy);
        len -= copy;
        copied_total += copy;
        buf += copy;
        abuf_write_release(abuf, copy);
    }

    AEXIT;
    AIDXD(0x048e11, "sd", "%s, copied_total=%d\n", __func__, copied_total);

    return copied_total;
}

int audio_read(ACLIENT handle, char *buf, u32 len, u32 wait_time)
{
    char *data_buf;
    u32 buf_len = len;
    struct asink_buf *abuf;

    abuf = audio_client_get_buf(handle);
    if (!abuf) {
        AIDXE(0x1ae96e, "", "audio read failed, no abuf!\n");
        return AIC_EPERM;
    }

    buf_len = len;
    data_buf = abuf_read_get(abuf, &buf_len, wait_time);
    if (!data_buf) {
        AIDXE(0x15efff, "", "audio read failed, can't get buf!\n");
        return AIC_EPIPE;
    }

    memcpy(buf, data_buf, buf_len);
    abuf_read_release(abuf, buf_len);

    AIDXD(0x048e11, "sd", "%s, copied_total=%d\n", __func__, buf_len);

    return buf_len;
}

int audio_drain(ACLIENT handle)
{
    struct asink_buf *abuf;

    abuf = audio_client_get_buf(handle);
    abuf_set_drain(abuf);

    return 0;
}

int audio_force_out_device(u32 device)
{
    struct audio_msg_set_device msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_FORCE_OUT_DEVICE;
    msg.device = device;

    ret = audio_bind(&msg, &rsp, -1);

    return (ret | rsp.result);
}

int audio_force_in_device(u32 device)
{
    struct audio_msg_set_device msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_FORCE_IN_DEVICE;
    msg.device = device;

    ret = audio_bind(&msg, &rsp, -1);

    return (ret | rsp.result);
}

int audio_get_device(u32 *in_device, u32 *out_device)
{
    struct audio_msg_get_device msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_GET_DEVICE;

    ret = audio_bind(&msg, &rsp, -1);

    if (!ret) {
        if (in_device)
            *in_device = (((u32)rsp.result) & 0xffff) | 0x80000000u;
        if (out_device)
            *out_device = (((u32)rsp.result) >> 16);
    }

    return 0;
}

int audio_get_force_device(u32 *in_device, u32 *out_device)
{
    struct audio_msg_get_device msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_GET_FORCE_DEVICE;

    ret = audio_bind(&msg, &rsp, -1);

    if (!ret) {
        if (in_device)
            *in_device = (((u32)rsp.result) & 0xffff) | 0x80000000u;
        if (out_device)
            *out_device = (((u32)rsp.result) >> 16);
    }

    return ret;
}

int audio_set_volume(ACLIENT handle, u32 vol)
{
    return 0;
}

u32 audio_get_volume(ACLIENT handle)
{
    return 0;
}

int audio_set_master_volume(u32 vol)
{
    struct audio_msg_set_master_volume msg;
    struct audio_rsp_base rsp;
    int ret;

    if (vol > AUDIO_MVOL_LEVEL_MAX)
        return AIC_EPERM;

    msg.cmd = AUDIO_CMD_SET_MASTER_VOLUME;
    msg.vol = vol;

    ret = audio_bind(&msg, &rsp, -1);

    return (ret | rsp.result);
}

u32 audio_get_master_volume(void)
{
    struct audio_msg_get_master_volume msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_GET_MASTER_VOLUME;

    ret = audio_bind(&msg, &rsp, -1);

    if (!ret)
        return rsp.result;

    return (u32)(-1);
}

int audio_mute(ACLIENT handle)
{
    return 0;
}

int audio_unmute(ACLIENT handle)
{
    return 0;
}

int audio_mute_all(void)
{
    return 0;
}

int audio_unmute_all(void)
{
    return 0;
}

int audio_mute_ul(u32 is_mute)
{
    struct audio_msg_mute_ul msg;
    struct audio_rsp_base rsp;
    int ret;

    msg.cmd = AUDIO_CMD_MUTE_UL;
    msg.is_mute = is_mute;

    ret = audio_bind(&msg, &rsp, -1);

    return (ret | rsp.result);
}

int audio_set_voice_info(ACLIENT handle)
{
    return 0;
}

/* return: msecond */
u32 audio_get_timestamp(ACLIENT handle)
{
    return audio_client_get_timestamp(handle);
}

#if CONFIG_LOOP_TEST_VOIP
void aloop_test_thread_loop(void *data)
{
#define CNT     52
#define FSZ     64

    struct audio_system_ctx *pctx = (struct audio_system_ctx *)data;
    enum astream_format fmt  = ASTREAM_FORMAT_AMR_WB;
    u32  srate = 16000, rd = 0, wr = 0;
    ACLIENT ul_handle = 0, dl_handle = 0;
    int ret;
    u8 buf[(FSZ + 1) * CNT] = {0}, *pbuf;

    ul_handle = audio_open(ASTREAM_TYPE_VOIP_UL, APLAY_TYPE_STREAM,
                           AUDIO_INPUT_FLAG_COMPRESS, srate, 0,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_RECORD,
                           ASTREAM_SOURCE_DEFAULT);
    if (!ul_handle) {
        AIDXE(0x107a35, "", "aloop ul open failed\n");

        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(ul_handle, 0, 0);
    if (ret) {
        audio_close(ul_handle);
        ul_handle = 0;
        AIDXE(0x175b4d, "d", "aloop ul play failed, ret=%d\n", ret);

        goto err_proc;
    }

    dl_handle = audio_open(ASTREAM_TYPE_VOIP, APLAY_TYPE_STREAM,
                           AUDIO_OUTPUT_FLAG_COMPRESS, srate, 0,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_PLAY,
                           ASTREAM_SOURCE_DEFAULT);
    if (!dl_handle) {
        AIDXE(0x04e0cd, "", "aloop dl open failed\n");
        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(dl_handle, 0, 0);
    if (ret) {
        audio_close(dl_handle);
        dl_handle = 0;

        AIDXE(0x16e4ac, "d", "aloop dl play failed, ret=%d\n", ret);

        goto err_proc;
    }

    /* read 50 frames first */
    while (rd < 50) {
        pbuf = &buf[rd * FSZ];
        ret = audio_read(ul_handle, pbuf, FSZ, 40);
        if (ret < 0)
            continue;
        *pbuf = ret;
        rd++;
    }

    /* then read and write continuersely */
    while (pctx->lp_state == 1) {
        pbuf = &buf[rd * FSZ];
        ret = audio_read(ul_handle, pbuf, FSZ, 40);
        if (ret < 0) {
            AIDXE(0x187d59, "d", "read error, ret=%d\n", ret);

            continue;
        }

        rd = (rd + 1) % CNT;
        pbuf = &buf[wr * FSZ];
        wr = (wr + 1) % CNT;
        /* totol 64, the first is frame len */
        ret = audio_write(dl_handle, pbuf + 1, FSZ, 21);
        if (ret < 0) {
            AIDXE(0x0a5692, "d", "write error, ret=%d\n", ret);
        }
    }

    audio_stop(ul_handle);
    audio_close(ul_handle);

    audio_stop(dl_handle);
    audio_close(dl_handle);

    AIDXE(0x013cb7, "", "aloop thread exit!\n");

    pctx->lp_state = 0;
err_proc:
    return;
}
#else
void aloop_test_thread_loop(void *data)
{
#define BUF_SIZE    1024 * 10

    struct audio_system_ctx *pctx = (struct audio_system_ctx *)data;
    enum astream_format fmt  = ASTREAM_FORMAT_AMR_WB;
    u32  srate = 16000, rd = 0;
    ACLIENT ul_handle = 0, dl_handle = 0;
    int ret;
    u8 buf[BUF_SIZE], *pbuf;

    ul_handle = audio_open(ASTREAM_TYPE_RECORD, APLAY_TYPE_STREAM,
                           AUDIO_INPUT_FLAG_COMPRESS, srate, 7,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_RECORD,
                           ASTREAM_SOURCE_DEFAULT);
    if (!ul_handle) {
        AIDXE(0x107a35, "", "aloop ul open failed\n");

        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(ul_handle, 0, 0);
    if (ret) {
        audio_close(ul_handle);
        ul_handle = 0;
        AIDXE(0x175b4d, "d", "aloop ul play failed, ret=%d\n", ret);

        goto err_proc;
    }
    rd = strlen("#!AMR-WB\n");
    memcpy(buf, "#!AMR-WB\n", rd);
    /* read 50 frames first */
    while (rd < BUF_SIZE - 64) {
        pbuf = buf + rd;
        ret = audio_read(ul_handle, pbuf, 64, 40);
        if (ret < 0)
            continue;

        rd += ret;
    }

    audio_stop(ul_handle);
    audio_close(ul_handle);

    dl_handle = audio_open(ASTREAM_TYPE_MUSIC, APLAY_TYPE_BUF,
                           AUDIO_OUTPUT_FLAG_COMPRESS, srate, 7,
                           fmt, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_PLAY,
                           ASTREAM_SOURCE_DEFAULT);
    if (!dl_handle) {
        AIDXE(0x04e0cd, "", "aloop dl open failed\n");
        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_buf(dl_handle, buf, rd, 0, 0);
    if (ret) {
        audio_close(dl_handle);
        dl_handle = 0;

        AIDXE(0x16e4ac, "d", "aloop dl play failed, ret=%d\n", ret);

        goto err_proc;
    }

    aos_sleep(4000);

    AIDXE(0x013cb7, "", "aloop thread exit!\n");

    pctx->lp_state = 0;

err_proc:
    return;
}
#endif

int audio_start_loopback(void)
{
    struct audio_system_ctx *pctx = &g_audio_system_ctx;

    ALOCK

    if (pctx->lp_state != 0) {
        AIDXE(0x14fdf9, "", "loopback is active or stopping!\n");
        AUNLOCK

        return AIC_EPERM;
    }
    pctx->lp_state = 1;

    if (pctx->lp_th)
        aos_delete_thread(pctx->lp_th);
    pctx->lp_th = aos_create_thread("loopback test",
                                    aloop_test_thread_loop,
                                    pctx, 1024 * 12, 0, NULL, 10,
                                    AOS_AUTO_START);

    AUNLOCK

    return 0;
}
CONSOLE_EXPORT_CMD(lp_start, audio_start_loopback, audio start loopback);

int audio_stop_loopback(void)
{
    struct audio_system_ctx *pctx = &g_audio_system_ctx;

    ALOCK

    if (pctx->lp_state == 1)
        pctx->lp_state = 2;

    while(pctx->lp_state)
        aos_sleep(40);

    if (pctx->lp_th) {
        aos_delete_thread(pctx->lp_th);
        pctx->lp_th = 0;
    }
    AUNLOCK

    AIDXE(0x196d86, "", "loopback exited!\n");

    return 0;
}
CONSOLE_EXPORT_CMD(lp_stop, audio_stop_loopback, audio stop loopback);

static int audio_system_init(void)
{
    g_audio_system_ctx.mutex = aos_create_mutex("audio system mutex");

    return 0;
}
AIC_NORMAL_APP_P1_INIT_EXPORT(audio_system_init);

