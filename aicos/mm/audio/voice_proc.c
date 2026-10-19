/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is about voice process framework.
 *
 */

#define AIC_THIS_FILE    0x163272
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_errno.h>

#include "audio_system.h"
#include "audio_util.h"
#include "codec_mng.h"
#include "voice_proc.h"
#include "voice_codec.h"
#include "audio_nrec.h"
#include "audio_resample.h"
#include "audio_dump.h"

#define VOICE_FRM_LEN_MAX       640

#define SWAP(a, b) \
{   \
    void *p = a;    \
    a = b;  \
    b = p;  \
}

enum {
    VP_REC_IDLE = 0,
    VP_REC_START,
    VP_REC_PAUSE,
};

typedef int (*rec_cb)(void *priv, u8 *buf, u32 size);

struct vp_rec {
    u32 fmt;
    u32 srate;
    u32 brate;
    u32 chn;
    rec_cb cb;
    void *priv;
    u32 state;
};

struct vp_runtime {
    void *hcodec;
    void *hresample;
    void *haec;
    void *hanr;
    void *rsmpl_d;
    void *rsmpl_u;

    void *in;
    u32 in_len;
    void *out;
    u32 out_len;
    void *aec;
    u32 aec_len;

    vp_codec_get get_func;
    vp_codec_send send_func;

    void *client_priv;
    u32 fmt;
    u32 enc_mode;
    bool is_mixing;
    struct vp_rec rec;
};

struct vp_runtime g_vp_runtime;

extern u8 *snd_stream_voice_mix_get(void);
extern void snd_stream_voice_mix_release(void);

static void mix_data(short *p1, short *p2, u32 cnt)
{
    if (!p1 || !p2)
        return;

    while(cnt--) {
        *p1 = ((s32)(*p1) + (s32)(*p2)) >> 1;
        p1++;
        p2++;
    }
}

void *voice_proc_open(void *priv, struct codec_info *codec_info,
                      vp_codec_get get_func, vp_codec_send send_func)
{
    struct vp_runtime *vp_runtime = &g_vp_runtime;
    char *p;

    if (vp_runtime->hcodec) {
        AIDXI(0x0211d0, "", "voice codec already opened\n");
        return NULL;
    }
    memset(vp_runtime, 0, sizeof(*vp_runtime));

    p = audio_malloc(VOICE_FRM_LEN_MAX * 4);
    if (!p) {
        AIDXE(0x05faf4, "", "voice proc open failed, no frame buff\n");
        return 0;
    }
    vp_runtime->in = p;
    vp_runtime->out = p + (VOICE_FRM_LEN_MAX << 1);
    vp_runtime->aec = p + VOICE_FRM_LEN_MAX * 3;
    vp_runtime->in_len = VOICE_FRM_LEN_MAX;
    vp_runtime->out_len = VOICE_FRM_LEN_MAX;
    vp_runtime->aec_len = VOICE_FRM_LEN_MAX;
    vp_runtime->get_func = get_func;
    vp_runtime->send_func = send_func;
    vp_runtime->client_priv = priv;
    vp_runtime->fmt = codec_info->fmt;
    vp_runtime->enc_mode = codec_info->brate;
    vp_runtime->hcodec = voice_codec_open(codec_info);
    if (!vp_runtime->hcodec) {
        AIDXE(0x0b8230, "", "voice codec open failed\n");
        goto codec_err;
    }

    vp_runtime->haec = audio_ec_create(codec_info->srate, codec_info->chn, 0);
    if (!vp_runtime->haec) {
        AIDXE(0x04765d, "", "voice aec open failed\n");
        goto aec_err;
    }
    vp_runtime->hanr = audio_nr_create(codec_info->srate, codec_info->chn, 0);
    if (!vp_runtime->hanr) {
        AIDXE(0x087a8e, "", "voice anr open failed\n");
        /* do not return error */
    }

    vp_runtime->rsmpl_d = audio_rsmpl_open(8000, 16000, 1, VOICE_FRM_LEN_MAX >> 1);
    if (!vp_runtime->rsmpl_d) {
        AIDXE(0x127d4b, "", "voice rsmpl_d open failed\n");
    }
    vp_runtime->rsmpl_u = audio_rsmpl_open(16000, 8000, 1, VOICE_FRM_LEN_MAX >> 1);
    if (!vp_runtime->rsmpl_u) {
        AIDXE(0x127d4b, "", "voice rsmpl_d open failed\n");
    }

    return vp_runtime;
aec_err:
    voice_codec_close(vp_runtime->hcodec);
    vp_runtime->hcodec = 0;
codec_err:
    audio_free(vp_runtime->in);
    vp_runtime->in = 0;

    return 0;
}

int voice_proc_get(void *hvp, void *buff, u32 *size)
{
    struct vp_runtime *vp_runtime = hvp;
    u32 len, in_len = 0, fmt, mode, out_len = 0;
    void *pout, *pin;
    int ret;

    if (!vp_runtime || !vp_runtime->get_func) {
        return AIC_EPERM;
    }

    len = VOICE_FRM_LEN_MAX; /* max value */
    fmt = vp_runtime->fmt;
    mode = vp_runtime->enc_mode;
    pin = vp_runtime->in;
    pout = (char *)pin + VOICE_FRM_LEN_MAX;
    ret = vp_runtime->get_func(vp_runtime->client_priv, pin, len,
                               &in_len, &fmt, &mode);
    if (ret < 0) {
        AIDXI(0x165e0c, "", "voice proc get failed\n");
        return AIC_EPIPE;
    }

    if (fmt == ASTREAM_FORMAT_PCM)  {
        out_len = in_len;
    } else {
        if (fmt == ASTREAM_FORMAT_VOICE_DTMF) {
            u8 is_wb;
            char *p = (char *)pin;
            /**
             *  For DTMF, in len is useless, we use it as DTMF param;
             *  bit0-7:   NB or WB indication;
             *  bit8-15:  key;
             *  bit16-32: volume;
             */
            is_wb = (vp_runtime->fmt == ASTREAM_FORMAT_AMR_WB);
            mode |= (p[1] << 8) | (p[2] << 16) | is_wb;
            AIDXI(0x1c79fe, "dds", "DTMF gen, key=%d, vol=%d, %s\n", p[1], p[2],
                  is_wb ? "WB" : "NB");
            in_len = mode;
        }
        audio_dump(pin, in_len, 10);
        voice_codec_dec(vp_runtime->hcodec, fmt, pin,
                        in_len, pout, &out_len);
        SWAP(pin, pout);
    }
    if (fmt != ASTREAM_FORMAT_VOICE_DTMF && fmt != ASTREAM_FORMAT_PCM &&
            (fmt != vp_runtime->fmt || mode != vp_runtime->enc_mode)) {
        AIDXI(0x05028a, "dddd", "voice proc change fmt or mode, fmt %d->%d, mode %d->%d\n",
              vp_runtime->fmt, fmt, vp_runtime->enc_mode, mode);
        voice_codec_set_mode(vp_runtime->hcodec, fmt, mode);
        vp_runtime->enc_mode = mode;
        vp_runtime->fmt = fmt;
    }

    /* resample */
    if (fmt == ASTREAM_FORMAT_AMR_NB && vp_runtime->rsmpl_d) {
        audio_dump(pin, out_len, 11);
        ret = audio_rsmpl_run(vp_runtime->rsmpl_d, pin,
                              out_len >> 1, pout, &out_len);
        SWAP(pin, pout);
        out_len = out_len << 1;
        AIDXD(0x1992ec, "dd", "DL rsmpl: ret=%d, out_len=%d\n", ret, out_len);
    }

    audio_dump(pin, out_len, 4);
    /* noise reduce */
    if (vp_runtime->hanr) {
        audio_nr_proc(vp_runtime->hanr, pin, out_len, pout, &out_len);
        SWAP(pin, pout);
    }
    audio_dump(pin, out_len, 5);
    /* mix */
#ifdef CONFIG_SND_STREAM_VOICE_MIX
    if (vp_runtime->is_mixing) {
        void *pmix = snd_stream_voice_mix_get();
        if (pmix) {
            mix_data(pin, pmix, out_len >> 1);
            snd_stream_voice_mix_release();
        } else
            AIDXE(0x1cec5d, "", "voice mix glitch\n");
    }
#endif
    *size = out_len;
    memcpy(buff, pin, out_len);

    /* save for aec */
    memcpy(vp_runtime->aec, buff, out_len);
    audio_dump(vp_runtime->aec, out_len, 12);
    return VOICE_FRM_LEN_MAX;
}

int voice_proc_send(void *hvp, void *buff, u32 size)
{
    struct vp_runtime *vp_runtime = hvp;
    u32 out_len = VOICE_FRM_LEN_MAX;
    int ret;

    if (!vp_runtime || !vp_runtime->send_func) {
        return AIC_EPERM;
    }

    /* aec */
    audio_dump(buff, size, 6);
    audio_ec_proc(vp_runtime->haec, buff, vp_runtime->aec, size,
                  vp_runtime->out, &out_len);
    audio_dump(vp_runtime->out, out_len, 7);
    /* For voice record */
    if (vp_runtime->rec.state == VP_REC_START)
        mix_data((short *)vp_runtime->aec, (short *)vp_runtime->out, size >> 1);

    /* resample */
    if (vp_runtime->fmt == ASTREAM_FORMAT_AMR_NB && vp_runtime->rsmpl_u) {
       ret = audio_rsmpl_run(vp_runtime->rsmpl_u, vp_runtime->out, size >> 1,
                             vp_runtime->out, &out_len);
       out_len = out_len << 1;
       audio_dump(vp_runtime->out, out_len, 8);
       AIDXD(0x1d17b3, "ddd", "UL rsmpl: ret=%d, size=%d, out_len=%d\n", ret, size, out_len);
    }
    /* encode */
    if (vp_runtime->fmt != ASTREAM_FORMAT_PCM) {
        ret = voice_codec_enc(vp_runtime->hcodec, vp_runtime->fmt,
                              vp_runtime->out, out_len,
                              vp_runtime->out, &out_len);
        if (ret || out_len == 0) {
            AIDXE(0x0948d9, "dd", "voice encode error, ret=%d, out_len=%d\n", ret, out_len);
            return ret;
        }
    }
    audio_dump(vp_runtime->out, out_len, 9);
    ret = vp_runtime->send_func(vp_runtime->client_priv, vp_runtime->out,
                                out_len);
    if (ret <= 0) {
        AIDXI(0x101444, "", "voice proc send failed\n");
        return AIC_EPIPE;
    }

    if (vp_runtime->rec.state == VP_REC_START && vp_runtime->rec.cb)
        vp_runtime->rec.cb(vp_runtime->rec.priv, vp_runtime->aec, size);

    return VOICE_FRM_LEN_MAX;
}

int voice_proc_close(void *hvp)
{
    struct vp_runtime *vp_runtime = hvp;

    if (vp_runtime->hanr)
        audio_nr_delete(vp_runtime->hanr);

    if (vp_runtime->haec)
        audio_ec_delete(vp_runtime->haec);

    if (vp_runtime->hcodec)
        voice_codec_close(vp_runtime->hcodec);

    if (vp_runtime->rsmpl_d)
        audio_rsmpl_close(vp_runtime->rsmpl_d);

    if (vp_runtime->rsmpl_u)
        audio_rsmpl_close(vp_runtime->rsmpl_u);

    if (vp_runtime->in)
        audio_free(vp_runtime->in);
    /* clean all */
    memset(vp_runtime, 0, sizeof(*vp_runtime));

    return 0;
}

int voice_proc_md_set(void *pmd)
{
    struct vp_runtime *prtd = &g_vp_runtime;
    int ret , ret1;

    ret = ret1 = 0;

    if (prtd->haec)
        ret = audio_ec_control(prtd->haec, AIC_VOICE_CMD_NV, pmd);

    if (ret != AIC_VOICE_RC_SUCCEED) {
        AIDXW(0x02da8e, "d", "audio_ec_control error:%d", ret);
        ret1 = ret;
    }

    if (prtd->hanr)
        ret = audio_nr_control(prtd->hanr, AIC_PLAYBACK_CMD_NV, pmd);

    if (ret != AIC_PLAYBACK_RC_SUCCEED) {
        AIDXW(0x1902f7, "d", "audio_nr_control error:%d", ret);
        ret1 = ret;
    }

    return ret1;
}

int voice_proc_record_start(u32 fmt, u32 srate, u32 brate, u32 chn,
                            void *cb, void *priv)
{
    struct vp_runtime *prtd = &g_vp_runtime;

    if (prtd->hanr && prtd->haec && prtd->rec.state == VP_REC_IDLE) {
        prtd->rec.srate = srate;
        prtd->rec.brate = brate;
        prtd->rec.chn = chn;
        prtd->rec.cb = (rec_cb)cb;
        prtd->rec.priv = priv;
        prtd->rec.state = VP_REC_START;
        return 0;
    } else {
        AIDXE(0x04973d, "d", "voice proc record start failed, state=%d\n", prtd->rec.state);
        return AIC_EPERM;
    }
}

int voice_proc_record_stop(void)
{
    memset(&g_vp_runtime.rec, 0, sizeof(struct vp_rec));

    return 0;
}

int voice_proc_record_pause(void)
{
    g_vp_runtime.rec.state = VP_REC_PAUSE;

    return 0;
}

int voice_proc_record_resume(void)
{
    struct vp_runtime *prtd = &g_vp_runtime;

    if (prtd->hanr && prtd->haec && prtd->rec.state == VP_REC_PAUSE) {
        prtd->rec.state = VP_REC_START;
        return 0;
    } else {
        AIDXE(0x1399df, "d", "voice proc record resume failed, state=%d\n", prtd->rec.state);
        return AIC_EPERM;
    }
}

void voice_proc_mix_start(void)
{
    struct vp_runtime *prtd = &g_vp_runtime;
    prtd->is_mixing = TRUE;
}

void voice_proc_mix_stop(void)
{
    struct vp_runtime *prtd = &g_vp_runtime;
    prtd->is_mixing = FALSE;
}

