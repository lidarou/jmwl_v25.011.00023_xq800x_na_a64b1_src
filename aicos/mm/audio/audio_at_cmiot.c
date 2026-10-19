/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio AT command file, in which we defined audio play&record
 *  etc. AT command.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e9d4a
#define LOG_MODULE    AMOD_AUDIO

#include <aic_at_cmd_handle.h>
#include <aic_at_process_common.h>
#include <aic_cmd_api.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>

#ifdef CONFIG_FS
#include <fs_api.h>
#endif
#include "audio_at_cmiot.h"
#include "audio_system.h"
#include "audio_util.h"

#ifdef CONFIG_FS
#define CM_AUDIO_AT_RECORDER
#endif

#define DEFAULT_AUD_PATH "/C/audio"
#define PARTITION_PATH_PREFIX 2 /* skip /C */
#define MAX_FILE_NAME_LEN 64
#define FILE_EXT_LEN 4 /* like ".amr" / ".mp3" etc */
/* /C/audio/<filename>.<ext> */
#define MAX_FULL_FILE_NAME_LEN ((sizeof(DEFAULT_AUD_PATH) - 1) + 1 + MAX_FILE_NAME_LEN + FILE_EXT_LEN)
#define AUDIO_AT_DEFAULT_SRATE 16000
#define AUDIO_AT_AMR_NB_SRATE 8000
#define AUDIO_AT_DEFAULT_CHN ASTREAM_CHN_MONO

enum {
    AT_PLCFG = 0,
    AT_PLFILE,
#ifdef CM_AUDIO_AT_PLSTREAM
    AT_PLSTREAM,
#endif
    AT_PLSTOP,
    AT_PLPAUSE,
    AT_PLRESUME,
#ifdef CM_AUDIO_AT_RECORDER
    AT_RECCFG,
    AT_RECFILE,
#ifdef CM_AUDIO_AT_RECSTREAM
    AT_RECSTREAM,
#endif
    AT_RECSTOP,
    AT_RECPAUSE,
    AT_RECRESUME,
#endif
#ifdef CM_AUDIO_AT_TTS
    AT_TTSCFG,
    AT_TTSPLAY,
    AT_TTSSTOP,
#endif
    AT_LPBCFG,
    AT_LPBSTART,
    AT_LPBSTOP,
    AT_MAX
};

/* mode enumeration used in file playing */
enum {
    PLFILE_MODE_ONCE = 0,
    PLFILE_MODE_CYCLE = 1,
};

/* mode enumeration used in loopback playing */
enum {
    LPBCFG_MODE_SELF, /* self-loop*/
    LPBCFG_MODE_EXT,  /* external-loop */
};

/* device enumeration used in audio-in of loopback playing */
enum {
    LPBCFG_AUDIO_IN_MASTER_MIC = 0,
    LPBCFG_AUDIO_IN_HEADSET_MIC = 1,
};

/* device enumeration used in audio-out of loopback playing */
enum {
    LPBCFG_AUDIO_OUT_EARPIECE = 0,
    LPBCFG_AUDIO_OUT_HEADSET = 1,
    LPBCFG_AUDIO_OUT_SPEAKER = 2,
};

/* state enumeration of loopback playing */
enum {
    LPB_STATE_NOT_TESTING = 0,
    LPB_STATE_TESTING = 1,
};

/* state enumeration of file playing or recording */
enum {
    STS_IDLE = 0,
    STS_PLAY = 1,
    STS_FIN = 2,
    STS_BRK = 3,
    STS_PAU = 4,
};

const static char *rsp_ok = "OK";
/* const static char *rsp_err = "ERROR"; */
const static char *tag_vol = "volume";
const static char *tag_enc = "encoding";
const static char *tag_mode = "mode";

const static char *aud_tag_arr[] = {"+MAUDPLCFG: ",
                                    "+MAUDPLFILE: ",
#ifdef CM_AUDIO_AT_PLSTREAM
                                    NULL, /* +MAUDPLSTREAM */
#endif
                                    NULL, /* +MAUDPLSTOP */
                                    NULL, /* +MAUDPLPAUSE */
                                    NULL, /* +MAUDPLRESUME */
#ifdef CM_AUDIO_AT_RECORDER
                                    "+MAUDRECCFG: ",
                                    "+MAUDRECFILE: ",
#ifdef CM_AUDIO_AT_RECSTREAM
                                    NULL, /* +MAUDRECSTREAM */
#endif
                                    NULL, /* +MAUDRECSTOP */
                                    NULL, /* +MAUDRECPAUSE */
                                    NULL, /* +MAUDRECRESUME */
#endif
#ifdef CM_AUDIO_AT_TTS
                                    "+MTTSCFG: ",
                                    "+MTTSPLAY: ",
                                    NULL, /* +MTTSSTOP */
#endif                                    /* CM_AUDIO_AT_TTS */
                                    "+MAUDLPBCFG: ",
                                    "+MAUDLPBSTART: ",
                                    "+MAUDLPBSTOP: ",
                                    };

const static char *rsp_query_arr[] = {"+MAUDPLCFG: \"volume\",(0-15)",
                                      "+MAUDPLFILE: \"filename\",(0)",
#ifdef CM_AUDIO_AT_PLSTREAM
                                      NULL, /* +MAUDPLSTREAM */
#endif
                                      NULL, /* +MAUDPLSTOP */
                                      NULL, /* +MAUDPLPAUSE */
                                      NULL, /* +MAUDPLRESUME */
#ifdef CM_AUDIO_AT_RECORDER
                                      "+MAUDRECCFG: \"encoding\",(3-10)",
                                      "+MAUDRECFILE: \"filename\"",
#ifdef CM_AUDIO_AT_RECSTREAM
                                      NULL, /* +MAUDRECSTREAM */
#endif
                                      NULL, /* +MAUDRECSTOP */
                                      NULL, /* +MAUDRECPAUSE */
                                      NULL, /* +MAUDRECRESUME */
#endif
#ifdef CM_AUDIO_AT_TTS
                                      NULL, /* +MTTSCFG */
                                      NULL, /* +MTTSPLAY */
                                      NULL, /* +MTTSSTOP */
#endif                                      /* CM_AUDIO_AT_TTS */
                                      "+MAUDLPBCFG: \"mode\",(0-1)",
                                      "+MAUDLPBSTART: (0-1),(0-2)",
                                      NULL, /* +MAUDLPBSTOP */
                                      };

#ifdef CM_AUDIO_AT_TTS
const static char *ttscfg_rsp_query = "+MTTSCFG: \"encoding\"\r\n"
                                      "+MTTSCFG: \"rdn\""
                                      "+MTTSCFG: \"speed\""
                                      "+MTTSCFG: \"pitch\""
                                      "+MTTSCFG: \"volume\""
                                      "+MTTSCFG: \"voice\""
                                      "+MTTSCFG: \"channel\""
                                      "+MTTSCFG: \"oauth\",,";
#endif /* CM_AUDIO_AT_TTS */
struct stm_inf {
    u32 hd;
    u8 sts;
    char name[MAX_FULL_FILE_NAME_LEN + 10];
    // u8      time;
};

struct aud_at_ctx {
    struct stm_inf pl;
    struct stm_inf rec;
    u8 rec_enc;
    u8 lpb_mode;  /* loop-back mode */
    u8 lpb_state; /* LPB_STATE_XXX */
    u8 com_id;
};

struct aud_at_ctx g_at_ctx;

extern u32 tone_check(u32 in_dev, u32 out_dev, u32 freq, u32 *avg1, u32 *avg2);
extern int audio_client_close_async(ACLIENT aclient);

static enum astream_format get_fmt_by_name(char *name)
{
    char *ext;

    if (!name)
        return ASTREAM_FORMAT_INVALID;
    ext = strrchr(name, '.');
    if (!ext)
        return ASTREAM_FORMAT_INVALID;
    ext += 1;
    if (strncasecmp(ext, "mp3", sizeof("mp3")) == 0)
        return ASTREAM_FORMAT_MP3;
    if (strncasecmp(ext, "amr", sizeof("amr")) == 0)
        return ASTREAM_FORMAT_AMR_NB;
    if (strncasecmp(ext, "wav", sizeof("wav")) == 0)
        return ASTREAM_FORMAT_WAV;
    if (strncasecmp(ext, "pcm", sizeof("pcm")) == 0)
        return ASTREAM_FORMAT_PCM;

    return ASTREAM_FORMAT_INVALID;
}

#ifdef CM_AUDIO_AT_RECORDER
static char *gen_rec_file_name(char *buf, size_t buf_size, char *name, u32 fmt)
{
    char *ext;

    if (!name)
        return 0;

    switch (fmt) {
    case AUD_AT_REC_FMT_PCM:
        ext = "pcm";
        break;
    case AUD_AT_REC_FMT_WAV:
        ext = "wav";
        break;
    case AUD_AT_REC_FMT_MP3:
        ext = "mp3";
        break;
    default:
        if (fmt <= AUD_AT_REC_FMT_MAX)
            ext = "amr";
        else
            ext = "pcm";
    }
    snprintf(buf, buf_size, "%s/%s.%s", DEFAULT_AUD_PATH, name, ext);

    return buf;
}
#endif

static int get_rec_fmt_rate(u8 enc, enum astream_format *fmt, u32 *srate, u32 *brate)
{
    switch (enc) {
    case AUD_AT_REC_FMT_PCM:
        *fmt = ASTREAM_FORMAT_PCM;
        *srate = AUDIO_AT_DEFAULT_SRATE;
        *brate = 0;
        break;
    case AUD_AT_REC_FMT_WAV:
        *fmt = ASTREAM_FORMAT_WAV;
        *srate = AUDIO_AT_DEFAULT_SRATE;
        *brate = 0;
        break;
    case AUD_AT_REC_FMT_MP3:
        /* note: MP3 value here is only for reference, however it is not supported yet */
        *fmt = ASTREAM_FORMAT_MP3;
        *srate = AUDIO_AT_DEFAULT_SRATE;
        *brate = 0;
        break;
    default:
        if (enc <= AUD_AT_REC_FMT_MAX) {
            *fmt = ASTREAM_FORMAT_AMR_NB;
            *srate = AUDIO_AT_AMR_NB_SRATE;
            /* tricky here. AMR NB bitrate value is equal to (enc - 3) */
            /* only available for AMR NB (0~7), refer to frame type in 3GPP TS 26.101 */
            *brate = enc - 3;
        } else {
            *fmt = ASTREAM_FORMAT_PCM;
            *srate = AUDIO_AT_DEFAULT_SRATE;
            *brate = 0;
        }
        break;
    }

    return 0;
}

static u8 audio_is_valid_to_play(void)
{
    if (g_at_ctx.pl.hd || g_at_ctx.rec.hd || g_at_ctx.lpb_state == LPB_STATE_TESTING)
        return AIC_TRUE;

    return AIC_FALSE;
}

static int audio_at_callback(u32 data, enum astream_cb_event event)
{
    struct aud_at_ctx *ctx = &g_at_ctx;
    char str[120] = {0};

    AIDXI(0x1b7747, "d", "cb event=%d\n", event);
    switch (event) {
    case ASTREAM_CB_EVENT_FINISH:
        if (ctx->pl.hd == data) {
            ctx->pl.sts = STS_FIN;
            snprintf(str, sizeof(str), "%s%d", aud_tag_arr[AT_PLFILE], ctx->pl.sts);
            /* close handle */
            if (audio_client_close_async((ACLIENT)data) == 0) {
                ctx->pl.hd = 0;
                ctx->pl.sts = STS_IDLE;
            }
        } else if (ctx->rec.hd == data) {
            ctx->rec.sts = STS_BRK; /* record finish here means break */
            snprintf(str, sizeof(str), "%s%d", aud_tag_arr[AT_RECFILE], ctx->rec.sts);
            /* close handle */
            if (audio_client_close_async((ACLIENT)data) == 0) {
                ctx->rec.hd = 0;
                ctx->rec.name[0] = 0;
                ctx->rec.sts = STS_IDLE;
            }
        } else
            AIDXE(0x1013fd, "x", "error audio call back handle=%x\n");
        break;
    case ASTREAM_CB_EVENT_BREAK:
        if (ctx->pl.hd == data) {
            ctx->pl.sts = STS_BRK;
            snprintf(str, sizeof(str), "%s%d", aud_tag_arr[AT_PLFILE], ctx->pl.sts);
            /* close handle */
            if (audio_client_close_async((ACLIENT)data) == 0) {
                ctx->pl.hd = 0;
                ctx->pl.sts = STS_IDLE;
            }
        } else
            AIDXE(0x1013fd, "x", "error audio call back handle=%x\n");
    default:
        break;
    }

    /* URC report */
    if (str[0]) {
        aic_transmit_at_cmd_urc(ctx->com_id, str, strlen(str));
    }

    return 0;
}

static s32 rsp_cmd(AT_REQ_RESP *at_req, const char *str1, const char *str2)
{
    ulong rsp[2];
    u16 cnt;

    if (str1) {
        cnt = 1;
        rsp[0] = (ulong)str1;
        if (str2) {
            cnt++;
            rsp[1] = (ulong)str2;
        }
    } else
        return AIC_EPERM;

    return aic_fill_transmit_cmd_resp(at_req, rsp, cnt);
}

static u32 test_cmd(AT_REQ_RESP *at_req, u32 id)
{
    ulong rsp[2];
    u16 num;

    if (id > AT_MAX) {
        AIDXE(0x0cecb2, "d", "invalid audio at id = %d\n", id);
        return AIC_EPERM;
    }

    if (rsp_query_arr[id]) {
        rsp[0] = (ulong)rsp_query_arr[id];
        rsp[1] = (ulong)rsp_ok;
        num = 2;
    } else {
        rsp[0] = (ulong)rsp_ok;
        num = 1;
    }

    return aic_fill_transmit_cmd_resp(at_req, rsp, num);
}

static u32 maudplcfg_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLCFG);
}

static u32 maudplcfg_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    u32 vol;
    char str[30];

    vol = audio_get_master_volume();
    snprintf(str, 30, "%s\"%s\",%d", aud_tag_arr[AT_PLCFG], tag_vol, vol);
    rsp[0] = (ulong)str;
    rsp[1] = (ulong)rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 maudplcfg_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u32 vol;
    u16 count;
    int ret;
    s32 err_code = AT_ERROR_INVALID_PARAS;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDPLCFG=", &count);
    if (!paras_arr) {
        AIDXE(0x025860, "", "MAUDPLCFG split failed\n");
        goto ERROR_EXIT;
    }

    if (strcmp((char *)paras_arr[0], tag_vol) == 0) {
        if (count == 1) {
            return maudplcfg_query_cmd(at_req);
        } else if (count == 2) {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                goto ERROR_EXIT;
            vol = atoi((char *)paras_arr[1]);
            if (vol > 15) {
                AIDXE(0x141afd, "", "invalid vol\n");
                err_code = AUD_AT_ERR_VOL;
                goto ERROR_EXIT;
            }
            ret = audio_set_master_volume(vol);
            if (ret) {
                AIDXE(0x157eae, "d", "MAUDPLCFG set volume failed, ret=%d\n", ret);
                err_code = AUD_AT_ERR_VOL;
                goto ERROR_EXIT;
            }
        } else {
            goto ERROR_EXIT;
        }
    } else {
        AIDXE(0x195c1e, "", "error param\n");
        goto ERROR_EXIT;
    }
    aic_free_paras_mem(paras_arr, count);
    aic_reply_ok_resp(at_req);

    return 0;
ERROR_EXIT:
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudplfile_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLFILE);
}

static u32 maudplfile_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s%d", aud_tag_arr[AT_PLFILE], g_at_ctx.pl.sts);

    return rsp_cmd(at_req, str, rsp_ok);
}

static u32 maudplfile_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    char *name;
    u16 count;
    int ret;
    enum astream_format fmt;
    ACLIENT hd;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    int mode = PLFILE_MODE_ONCE;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDPLFILE=", &count);
    if (!paras_arr || count > 2) {
        AIDXE(0x009827, "", "MAUDPLFILE split failed\n");
        goto ERROR_EXIT;
    }

    name = (char *)paras_arr[0];
    if (!name[0] || strlen(name) > MAX_FULL_FILE_NAME_LEN - PARTITION_PATH_PREFIX) {
        AIDXE(0x08cc97, "", "[+MAUDPLFILE]: invalid file name");
        goto ERROR_EXIT;
    }
    fmt = get_fmt_by_name(name);
    if (fmt == ASTREAM_FORMAT_INVALID) {
        AIDXE(0x075132, "", "MAUDPLFILE don't support this format\n");
        err_code = AUD_AT_ERR_PLAY;
        goto ERROR_EXIT;
    }
    if (count == 2) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto ERROR_EXIT;
        mode = atoi((u8 *)paras_arr[1]);
        if (mode != PLFILE_MODE_ONCE && mode != PLFILE_MODE_CYCLE) {
            goto ERROR_EXIT;
        }
        if (mode == PLFILE_MODE_CYCLE) {
            AIDXE(0x0c75f2, "", "[+MAUDPLFILE]: Not support repeat mode");
            err_code = AUD_AT_ERR_PLAY;
            goto ERROR_EXIT;
        }
    }
    if (audio_is_valid_to_play()) {
        AIDXE(0x00fb94, "", "MAUDPLFILE stop first!\n");
        err_code = AUD_AT_ERR_PLAY;
        goto ERROR_EXIT;
    }
#if 1
    hd = audio_open(ASTREAM_TYPE_MUSIC, APLAY_TYPE_FILE, AUDIO_OUTPUT_FLAG_COMPRESS, AUDIO_AT_DEFAULT_SRATE, 0, fmt,
                    AUDIO_AT_DEFAULT_CHN, ASTREAM_DIR_PLAY, 0);
#else
    hd = audio_open_output(ASTREAM_TYPE_MUSIC, APLAY_TYPE_FILE, fmt, AUDIO_OUTPUT_FLAG_COMPRESS);
#endif
    if (!hd) {
        AIDXE(0x01bf17, "", "MAUDPLFILE open failed!\n");
        err_code = AUD_AT_ERR_PLAY;
        goto ERROR_EXIT;
    }

    g_at_ctx.com_id = at_req->serial_port_id;
    /* re-org file name */
    /* rules: if start with prefix /, treat the path as full path and needs to add /C to redirect to partition C */
    /* otherwise, treat it as file name only and redirect to /C/audio */
    if (name[0] == '/') {
        snprintf(g_at_ctx.pl.name, sizeof(g_at_ctx.pl.name), "/C%s", name);
    } else {
        snprintf(g_at_ctx.pl.name, sizeof(g_at_ctx.pl.name), "/C/audio/%s", name);
    }
    ret = audio_play_file(hd, g_at_ctx.pl.name, audio_at_callback, hd);
    if (ret) {
        audio_close(hd);
        AIDXE(0x10f793, "s", "audplfile play failed!. %s\n", g_at_ctx.pl.name);
        err_code = AUD_AT_ERR_PLAY;
        goto ERROR_EXIT;
    }
    aic_free_paras_mem(paras_arr, count);
    g_at_ctx.pl.hd = hd;
    g_at_ctx.pl.sts = STS_PLAY;
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

#ifdef CM_AUDIO_AT_PLSTREAM
static u32 maudplstream_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLSTREAM);
}

static u32 maudplstream_query_cmd(AT_REQ_RESP *at_req)
{
    return rsp_cmd(at_req, rsp_err, 0);
}

static u32 maudplstream_set_cmd(AT_REQ_RESP *at_req)
{
    return rsp_cmd(at_req, rsp_err, 0);
}
#endif

static u32 maudplstop_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLSTOP);
}

static u32 maudplstop_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    char str[120] = {0};

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDPLSTOP", &cnt);
    if (paras_arr) {
        AIDXE(0x01f654, "", "[+MAUDPLSTOP]: parameter parse error\n");
        goto ERROR_EXIT;
    }

    if (ctx->pl.hd) {
        audio_stop(ctx->pl.hd);
        audio_close(ctx->pl.hd);
        ctx->pl.hd = 0;
        ctx->pl.sts = STS_IDLE;
    } else {
        AIDXE(0x15a88a, "", "[+MAUDPLSTOP]: no available handle to stop.");
        err_code = AUD_AT_ERR_STOP;
        goto ERROR_EXIT;
    }

    aic_reply_ok_resp(at_req);
    snprintf(str, sizeof(str), "%s2", aud_tag_arr[AT_PLFILE]);
    aic_transmit_at_cmd_urc(ctx->com_id, str, strlen(str));

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudplpause_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLPAUSE);
}

static u32 maudplpause_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDPLPAUSE", &cnt);
    if (paras_arr) {
        AIDXE(0x16016c, "", "[+MAUDPLPAUSE]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (ctx->pl.hd && ctx->pl.sts == STS_PLAY) {
        ret = audio_pause(ctx->pl.hd);
        if (!ret) {
            ctx->pl.sts = STS_PAU;
        } else {
            AIDXE(0x1c1499, "", "[+MAUDPLPAUSE]: unable to pause\n");
            err_code = AUD_AT_ERR_PAUSE;
            goto ERROR_EXIT;
        }
    } else {
        err_code = AUD_AT_ERR_PAUSE;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudplresume_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_PLRESUME);
}

static u32 maudplresume_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    int ret;

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDPLRESUME", &cnt);
    if (paras_arr) {
        AIDXE(0x0e1513, "", "[+MAUDPLRESUME]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (ctx->pl.hd && ctx->pl.sts == STS_PAU) {
        ret = audio_resume(ctx->pl.hd);
        if (!ret) {
            ctx->pl.sts = STS_PLAY;
        } else {
            AIDXE(0x0f35f4, "", "[+MAUDPLRESUME]: unable to resume\n");
            err_code = AUD_AT_ERR_RESUME;
            goto ERROR_EXIT;
        }
    } else {
        err_code = AUD_AT_ERR_RESUME;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudreccfg_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECCFG);
}

static u32 maudreccfg_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s\"%s\",%d", aud_tag_arr[AT_RECCFG], tag_enc, g_at_ctx.rec_enc);

    return rsp_cmd(at_req, str, rsp_ok);
}

static u32 maudreccfg_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    int enc;
    u16 count;
    s32 err_code = AT_ERROR_INVALID_PARAS;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDRECCFG=", &count);
    if (!paras_arr || count > 2) {
        AIDXE(0x027a0c, "", "MAUDRECCFG split failed\n");
        goto ERROR_EXIT;
    }

    if (strcmp((char *)paras_arr[0], tag_enc) == 0) {
        if (count == 1) {
            return maudreccfg_query_cmd(at_req);
        } else {
            if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
                goto ERROR_EXIT;
            enc = atoi((char *)paras_arr[1]);
            if (enc < AUD_AT_REC_FMT_4P75 || enc > AUD_AT_REC_FMT_MAX) {
                AIDXE(0x15b63d, "", "MAUDRECCFG invalid enc\n");
                err_code = AUD_AT_ERR_REC_SET;
                goto ERROR_EXIT;
            }
            g_at_ctx.rec_enc = enc;
        }
    } else {
        AIDXE(0x16ace4, "", "MAUDRECCFG error param\n");
        goto ERROR_EXIT;
    }
    aic_free_paras_mem(paras_arr, count);
    aic_reply_ok_resp(at_req);

    return 0;
ERROR_EXIT:
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

#ifdef CM_AUDIO_AT_RECORDER
static u32 maudrecfile_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECFILE);
}

static u32 maudrecfile_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s%d", aud_tag_arr[AT_RECFILE], g_at_ctx.rec.sts);

    return rsp_cmd(at_req, str, rsp_ok);
}

static u32 maudrecfile_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u32 srate = 0, flag = AUDIO_INPUT_FLAG_COMPRESS;
    u32 brate = 0;
    u16 count;
    int ret;
    ACLIENT hd;
    enum astream_format format;
    FS_FILE_INFO info;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    char *filename = NULL;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDRECFILE=", &count);
    if (!paras_arr || count > 3) {
        AIDXE(0x103bb3, "", "MAUDRECFILE split failed\n");
        goto ERROR_EXIT;
    }
    /*check file validity */
    filename = (char *)paras_arr[0];
    /* Note: filename is allowed to be empty in CM SPEC */
    if (strlen(filename) > MAX_FILE_NAME_LEN || strstr(filename, "/") || strstr(filename, "\\") ||
        strstr(filename, ",")) {
        AIDXE(0x045254, "", "[+MAUDRECFILE]: invalid file name");
        goto ERROR_EXIT;
    }
    if (audio_is_valid_to_play()) {
        AIDXE(0x0c355a, "", "record already started!\n");
        err_code = AUD_AT_ERR_REC_START;
        goto ERROR_EXIT;
    }
    /* Create audio folder if not exist */
    if (fs_getinfo(DEFAULT_AUD_PATH, &info)) {
        if (fs_createdir(DEFAULT_AUD_PATH)) {
            AIDXE(0x0f16d9, "", "MAUDRECFILE create default path error\n");
            err_code = AUD_AT_ERR_REC_START;
            goto ERROR_EXIT;
        }
    }
    gen_rec_file_name(g_at_ctx.rec.name, sizeof(g_at_ctx.rec.name), filename, g_at_ctx.rec_enc);
    get_rec_fmt_rate(g_at_ctx.rec_enc, &format, &srate, &brate);
    if (g_at_ctx.rec_enc == AUD_AT_REC_FMT_PCM || g_at_ctx.rec_enc == AUD_AT_REC_FMT_WAV) {
        flag = AUDIO_INPUT_FLAG_NONE;
    }
    hd = audio_open(ASTREAM_TYPE_RECORD, APLAY_TYPE_FILE, flag, srate, brate, format, AUDIO_AT_DEFAULT_CHN,
                    ASTREAM_DIR_RECORD, ASTREAM_SOURCE_MIC);
    if (!hd) {
        AIDXE(0x02730e, "", "MAUDRECFILE open error\n");
        err_code = AUD_AT_ERR_REC_START;
        goto ERROR_EXIT;
    }

    g_at_ctx.com_id = at_req->serial_port_id;
    ret = audio_play_file(hd, g_at_ctx.rec.name, audio_at_callback, hd);
    if (ret) {
        audio_close(hd);
        AIDXE(0x1c13fe, "", "MAUDRECFILE play error\n");
        err_code = AUD_AT_ERR_REC_START;
        goto ERROR_EXIT;
    }
    g_at_ctx.rec.hd = hd;
    g_at_ctx.rec.sts = STS_PLAY;
    aic_free_paras_mem(paras_arr, count);
    aic_reply_ok_resp(at_req);

    return 0;
ERROR_EXIT:
    g_at_ctx.rec.name[0] = 0;
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

#ifdef CM_AUDIO_AT_RECSTREAM
static u32 maudrecstream_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECSTREAM);
}

static u32 maudrecstream_query_cmd(AT_REQ_RESP *at_req)
{
    return rsp_cmd(at_req, rsp_err, 0);
}

static u32 maudrecstream_set_cmd(AT_REQ_RESP *at_req)
{
    return rsp_cmd(at_req, rsp_err, 0);
}
#endif

static u32 maudrecstop_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECSTOP);
}

static u32 maudrecstop_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    char str[120] = {0};
    FS_FILE_INFO info = {0};

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDRECSTOP", &cnt);
    if (paras_arr) {
        AIDXE(0x08687e, "", "[+MAUDRECSTOP]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (ctx->rec.hd) {
        audio_stop(ctx->rec.hd);
        audio_close(ctx->rec.hd);
        if (fs_getinfo(ctx->rec.name, &info)) {
            snprintf(str, sizeof(str), "%s2", aud_tag_arr[AT_RECFILE]);
        } else {
            snprintf(str, sizeof(str), "%s2,%s,%d", aud_tag_arr[AT_RECFILE], ctx->rec.name + PARTITION_PATH_PREFIX,
                     (int)info.size);
        }
        ctx->rec.hd = 0;
        ctx->rec.sts = 0;
        ctx->rec.name[0] = 0;
        rsp_cmd(at_req, str, rsp_ok);
    } else {
        err_code = AUD_AT_ERR_REC_STOP;
        goto ERROR_EXIT;
    }

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudrecpause_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECPAUSE);
}

static u32 maudrecpause_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    int ret;

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDRECPAUSE", &cnt);
    if (paras_arr) {
        AIDXE(0x1214ae, "", "[+MAUDRECPAUSE]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (ctx->rec.hd && ctx->rec.sts == STS_PLAY) {
        ret = audio_pause(ctx->rec.hd);
        if (!ret) {
            ctx->rec.sts = STS_PAU;
        } else {
            AIDXE(0x0fb0f6, "d", "[+MAUDRECPAUSE]: failed to pause %d\n", ret);
            err_code = AUD_AT_ERR_PAUSE;
            goto ERROR_EXIT;
        }
    } else {
        AIDXE(0x1d9231, "", "[+MAUDRECPAUSE]: not playing\n");
        err_code = AUD_AT_ERR_PAUSE;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudrecresume_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_RECRESUME);
}

static u32 maudrecresume_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    struct aud_at_ctx *ctx = &g_at_ctx;
    int ret;

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDRECRESUME", &cnt);
    if (paras_arr) {
        AIDXE(0x01b1cf, "", "[+MAUDRECRESUME]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (ctx->rec.hd && ctx->rec.sts == STS_PAU) {
        ret = audio_resume(ctx->rec.hd);
        if (!ret) {
            ctx->rec.sts = STS_PLAY;
        } else {
            AIDXE(0x1dc8af, "d", "[+MAUDRECRESUME]: failed to resume %d\n", ret);
            err_code = AUD_AT_ERR_RESUME;
            goto ERROR_EXIT;
        }
    } else {
        AIDXE(0x03d693, "", "[+MAUDRECRESUME]: not paused\n");
        err_code = AUD_AT_ERR_RESUME;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}
#endif

#ifdef CM_AUDIO_AT_TTS
static u32 mttscfg_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)ttscfg_rsp_query;
    rsp[1] = (ulong)rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 mttscfg_query_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}

static u32 mttscfg_set_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}

static u32 mttsplay_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_TTSPLAY);
}

static u32 mttsplay_query_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}

static u32 mttsplay_set_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}

static u32 mttsstop_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_TTSSTOP);
}

static u32 mttsstop_query_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}

static u32 mttsstop_set_cmd(AT_REQ_RESP *at_req)
{
    return 0;
}
#endif /* CM_AUDIO_AT_TTS */

static u32 maudlpbcfg_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_LPBCFG);
}

static u32 maudlpbcfg_set_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    int mode;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDLPBCFG=", &cnt);
    if (!paras_arr || cnt != 2) {
        AIDXE(0x085d41, "", "[+MAUDLPBCFG]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (strcmp((u8 *)paras_arr[0], tag_mode) == 0) {
        if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
            goto ERROR_EXIT;
        mode = atoi((u8 *)paras_arr[1]);
        if (mode != LPBCFG_MODE_SELF && mode != LPBCFG_MODE_EXT) {
            AIDXE(0x17aa54, "", "[+MAUDLPBCFG]: invalid mode\n");
            goto ERROR_EXIT;
        }
    } else {
        goto ERROR_EXIT;
    }
    g_at_ctx.lpb_mode = (u8)mode;
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, AT_ERROR_INVALID_PARAS);

    return 0;
}

static u32 maudlpbstart_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_LPBSTART);
}

static u32 maudlpbstart_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char str[30];

    snprintf(str, 30, "%s%d", aud_tag_arr[AT_LPBSTART], (int)g_at_ctx.lpb_state);
    rsp[0] = (ulong)str;
    rsp[1] = (ulong)rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 maudlpbstart_set_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    int mic_ch, spk_ch;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    u32 indev = AUDIO_DEVICE_IN_MIC, outdev = AUDIO_DEVICE_OUT_EARPIECE, v1 = 0, v2 = 0;
    char str[30];
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDLPBSTART=", &cnt);
    if (!paras_arr || cnt != 2) {
        AIDXE(0x06580e, "", "[+MAUDLPBSTART]: parameter parse error\n");
        goto ERROR_EXIT;
    }
    if (!aic_is_valid_integer_para((u8 *)paras_arr[0], strlen((u8 *)paras_arr[0])))
        goto ERROR_EXIT;
    mic_ch = atoi((u8 *)paras_arr[0]);
    if (mic_ch != LPBCFG_AUDIO_IN_MASTER_MIC && mic_ch != LPBCFG_AUDIO_IN_HEADSET_MIC) {
        AIDXE(0x0adbae, "", "[+MAUDLPBSTART]: invalid mic channel\n");
        goto ERROR_EXIT;
    }
    if (!aic_is_valid_integer_para((u8 *)paras_arr[1], strlen((u8 *)paras_arr[1])))
        goto ERROR_EXIT;
    spk_ch = atoi((u8 *)paras_arr[1]);
    if (spk_ch != LPBCFG_AUDIO_OUT_EARPIECE && spk_ch != LPBCFG_AUDIO_OUT_HEADSET &&
        spk_ch != LPBCFG_AUDIO_OUT_SPEAKER) {
        AIDXE(0x05619b, "", "[+MAUDLPBSTART]: invalid speaker channel\n");
        goto ERROR_EXIT;
    }
    /* determine if loopback playing is available or not */
    if (audio_is_valid_to_play()) {
        /* if playing or recording, loopback operation is forbidden */
        err_code = AUD_AT_ERR_LP;
        goto ERROR_EXIT;
    }
    /* convert audio in device type */
    switch (mic_ch) {
    case LPBCFG_AUDIO_IN_MASTER_MIC:
        indev = AUDIO_DEVICE_IN_MIC;
        break;
    case LPBCFG_AUDIO_IN_HEADSET_MIC:
        indev = AUDIO_DEVICE_IN_HEADPHONE;
        break;
    default:
        break;
    }
    /* convert audio out device type */
    switch (spk_ch) {
    case LPBCFG_AUDIO_OUT_EARPIECE:
        outdev = AUDIO_DEVICE_OUT_EARPIECE;
        break;
    case LPBCFG_AUDIO_OUT_HEADSET:
        outdev = AUDIO_DEVICE_OUT_HEADSET;
        break;
    case LPBCFG_AUDIO_OUT_SPEAKER:
        outdev = AUDIO_DEVICE_OUT_SPEAKER;
        break;
    default:
        break;
    }
    if (g_at_ctx.lpb_mode == LPBCFG_MODE_SELF) {
        /* self loopback. */
        /* response OK first */
        aic_reply_ok_resp(at_req);
        /* start to check */
        g_at_ctx.lpb_state = LPB_STATE_TESTING;
        /* Note: tone_check is a sync API */
        tone_check(indev, outdev, 0, &v1, &v2);
        snprintf(str, 30, "+MAUDLPB: %d, %d", (int)v1, (int)v2);
        aic_transmit_at_cmd_urc(at_req->serial_port_id, str, strlen(str));
        g_at_ctx.lpb_state = LPB_STATE_NOT_TESTING;
    } else {
        /* external loopback */
        ret = audio_force_in_device(indev);
        if (ret) {
            AIDXE(0x0159e7, "", "[+MAUDLPBSTART]: failed to set audio in dev\n");
            err_code = AUD_AT_ERR_LP;
            goto ERROR_EXIT;
        }
        ret = audio_force_out_device(outdev);
        if (ret) {
            AIDXE(0x1772cf, "", "[+MAUDLPBSTART]: failed to set audio out dev\n");
            /* reset audio in dev */
            audio_force_in_device(0);
            err_code = AUD_AT_ERR_LP;
            goto ERROR_EXIT;
        }
        ret = audio_start_loopback();
        if (ret) {
            AIDXE(0x1e313f, "", "[+MAUDLPBSTART]: failed to start loopback\n");
            /* reset audio in dev */
            audio_force_in_device(0);
            /* reset audio out dev */
            audio_force_out_device(0);
            err_code = AUD_AT_ERR_LP;
            goto ERROR_EXIT;
        }
        /* state update */
        g_at_ctx.lpb_state = LPB_STATE_TESTING;
        aic_reply_ok_resp(at_req);
    }
    aic_free_paras_mem(paras_arr, cnt);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

static u32 maudlpbstop_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_LPBSTOP);
}

static u32 maudlpbstop_exe_cmd(AT_REQ_RESP *at_req)
{
    u16 cnt;
    ulong *paras_arr;
    s32 err_code = AT_ERROR_INVALID_PARAS;
    int ret;

    /* there should be no parameters at all */
    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MAUDLPBSTOP", &cnt);
    if (paras_arr) {
        AIDXE(0x09ebe1, "", "[+MAUDLPBSTOP]: parameter parse error\n");
        goto ERROR_EXIT;
    }

    /* determine if loopback test is ongoing or not */
    if (g_at_ctx.lpb_state == LPB_STATE_TESTING) {
        ret = audio_stop_loopback();
        if (ret) {
            err_code = AUD_AT_ERR_STOP;
            goto ERROR_EXIT;
        }
        g_at_ctx.lpb_state = LPB_STATE_NOT_TESTING;
        /* reset audio in dev */
        audio_force_in_device(0);
        /* reset audio out dev */
        audio_force_out_device(0);
    } else {
        /* not playing state */
        err_code = AUD_AT_ERR_STATE;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, cnt);
    aic_reply_error_resp(at_req, err_code);

    return 0;
}

const static AT_CMD_HANDLE audio_at_handles[] = {
    {"AT+MAUDPLCFG", maudplcfg_test_cmd, maudplcfg_query_cmd, maudplcfg_set_cmd, AIC_NULL},
    {"AT+MAUDPLFILE", maudplfile_test_cmd, maudplfile_query_cmd, maudplfile_set_cmd, AIC_NULL},
#ifdef CM_AUDIO_AT_PLSTREAM
    {"AT+MAUDPLSTREAM", maudplstream_test_cmd, maudplstream_query_cmd, maudplstream_set_cmd, AIC_NULL},
#endif
    {"AT+MAUDPLSTOP", maudplstop_test_cmd, AIC_NULL, AIC_NULL, maudplstop_exe_cmd},
    {"AT+MAUDPLPAUSE", maudplpause_test_cmd, AIC_NULL, AIC_NULL, maudplpause_exe_cmd},
    {"AT+MAUDPLRESUME", maudplresume_test_cmd, AIC_NULL, AIC_NULL, maudplresume_exe_cmd},
#ifdef CM_AUDIO_AT_RECORDER
    {"AT+MAUDRECCFG", maudreccfg_test_cmd, maudreccfg_query_cmd, maudreccfg_set_cmd, AIC_NULL},
    {"AT+MAUDRECFILE", maudrecfile_test_cmd, maudrecfile_query_cmd, maudrecfile_set_cmd, AIC_NULL},
#ifdef CM_AUDIO_AT_RECSTREAM
    {"AT+MAUDRECSTREAM", maudrecstream_test_cmd, maudrecstream_query_cmd, maudrecstream_set_cmd, AIC_NULL},
#endif
    {"AT+MAUDRECSTOP", maudrecstop_test_cmd, AIC_NULL, AIC_NULL, maudrecstop_exe_cmd},
    {"AT+MAUDRECPAUSE", maudrecpause_test_cmd, AIC_NULL, AIC_NULL, maudrecpause_exe_cmd},
    {"AT+MAUDRECRESUME", maudrecresume_test_cmd, AIC_NULL, AIC_NULL, maudrecresume_exe_cmd},
#endif
#ifdef CM_AUDIO_AT_TTS
    {"AT+MTTSCFG", mttscfg_test_cmd, mttscfg_query_cmd, mttscfg_set_cmd, AIC_NULL},
    {"AT+MTTSPLAY", mttsplay_test_cmd, mttsplay_query_cmd, mttsplay_set_cmd, AIC_NULL},
    {"AT+MTTSSTOP", mttsstop_test_cmd, mttsstop_query_cmd, mttsstop_set_cmd, AIC_NULL},
#endif /* CM_AUDIO_AT_TTS */
    {"AT+MAUDLPBCFG", maudlpbcfg_test_cmd, AIC_NULL, maudlpbcfg_set_cmd, AIC_NULL},
    {"AT+MAUDLPBSTART", maudlpbstart_test_cmd, maudlpbstart_query_cmd, maudlpbstart_set_cmd, AIC_NULL},
    {"AT+MAUDLPBSTOP", maudlpbstop_test_cmd, AIC_NULL, AIC_NULL, maudlpbstop_exe_cmd},
    {AIC_NULL},
};

static int audio_at_init(void)
{
    aos_memset(&g_at_ctx, 0, sizeof(g_at_ctx));
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)audio_at_handles);
    g_at_ctx.rec_enc = AUD_AT_REC_FMT_4P75; /* amr-nb */

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(audio_at_init);
