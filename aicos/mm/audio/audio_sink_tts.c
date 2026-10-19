/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is TTS sink client file, it provides methods to fill data
 *  when play TTS.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cba60
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_init.h>
#include <alog_api.h>
#include <aic_errno.h>

#include "audio_system.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_stream.h"
#include "audio_policy.h"
#include "audio_sink_record.h"
#include "yt_tts_interface_871.h"

#define TTS_DBG             ALOGD

#define YT_TTS_BUF_MAX      (512 * 2)

struct asink_tts {
    char *buf;
    char *buf_engine;
    u32 cur_pos;
    u32 cur_len;
    BOOL is_done;
};

void yt_log(char* strMessage, unsigned int *pTimeSnapshot)
{
    *pTimeSnapshot = 0;
    AIDXE(0x180758, "s", "[YT LIB LOG]: %s\n", strMessage);
}


static u32 asink_tts_open(enum astream_format format, void *msg)
{
    int ret;

    struct asink_tts *asink;

    TTS_DBG("tts open entry!\n");
    yt_tts_set_cb_print_info_871(yt_log);

    ret = yt_tts_start_activation_871();
    if (ret < 0) {
        AIDXE(0x1bb525, "d", "yt start active ret=%d\n", ret);
        //return 0;
    }
    ret = yt_tts_is_active_871();
    if (ret < 0) {
        AIDXD(0x1c169c, "d", "yt active ret=%d\n", ret);
        //return 0;
    }

    asink = audio_malloc(sizeof(*asink));
    if (!asink) {
        AIDXE(0x0a22c0, "", "tts open, no memory1!\n");
        return 0;
    }
    memset(asink, 0, sizeof(*asink));
    asink->buf = (char *)audio_malloc(YT_TTS_BUF_MAX);
    if (!asink->buf) {
        audio_free(asink);
        AIDXE(0x0ae3dc, "", "tts open, no memory2!\n");
        return 0;
    }
    asink->buf_engine = (char *)audio_malloc(YT_TTS_MEM_SIZE_IN_BYTE);
    if (!asink->buf_engine) {
        audio_free(asink->buf);
        audio_free(asink);
        AIDXE(0x010d0a, "", "tts open, no memory3!\n");
        return 0;
    }

    ret = yt_tts_initialize_871(YT_LANG_ID_MANDARIN_871, asink->buf_engine,
                                YT_TTS_MEM_SIZE_IN_BYTE);

    yt_tts_set_rate_871(102);
    yt_tts_set_pitch_871(105);
    yt_tts_set_volume_871(32768);

    return (u32)asink;
}

static int asink_tts_close(u32 obj)
{
    struct asink_tts *asink = (struct asink_tts *)obj;

    yt_tts_free_resource_871();

    if (!asink)
        return 0;

    if (asink->buf)
        audio_free(asink->buf);
    if (asink->buf_engine)
        audio_free(asink->buf_engine);

    audio_free(asink);

    AIDXI(0x04219c, "", "asink tts closed\n");

    return 0;
}

static int asink_tts_start(u32 obj, void *aplay_data)
{
    struct audio_play_sub_tts *sub = (struct audio_play_sub_tts *)aplay_data;

    if (!sub) {
        AIDXE(0x183e50, "", "asink tts start error, null param\n");

        return AIC_EPERM;
    }

    TTS_DBG("tts start entry, text=%s, len=%d!\n", sub->buf, sub->buf_len);

    return yt_tts_input_text_mbcs_871(sub->buf, sub->buf_len,0,0);
}

static int asink_tts_stop(u32 obj)
{
    return 0;
}

static int asink_tts_callback(u32 obj, int event, char *buffer,
                                u32 *get_size)
{
    struct asink_tts *asink = (struct asink_tts *)obj;
    u32 copy_len = 0, gen_count;
    int ret;

    TTS_DBG("asink tts callback, get_size=%d\n", *get_size);
    /* copy left data first */
    if (asink->cur_pos < asink->cur_len) {
        /* if left is enough, copy and return */
        if ( *get_size <= asink->cur_len - asink->cur_pos) {
            copy_len = *get_size;
            memcpy(buffer, asink->buf + asink->cur_pos, copy_len);
            asink->cur_pos += copy_len;
            /* get_size need not change */
            TTS_DBG("left enough, cur_pos=%d, cur_len=%d\n",
                    asink->cur_pos, asink->cur_len);
            return 0;
        } else {
            /* copy left, and if not done, continue get from engine */
            copy_len = asink->cur_len - asink->cur_pos;
            memcpy(buffer, asink->buf + asink->cur_pos, copy_len);
            asink->cur_pos = 0;
            asink->cur_len = 0;

            TTS_DBG("left not enough, cur_pos=%d, cur_len=%d,copy_len=%d\n",
                      asink->cur_pos, asink->cur_len, copy_len);
            if (asink->is_done) {
                *get_size = copy_len;
                return ASTREAM_DATA_EOS;
            }
        }
    }
    /* get from engine */
    while (!asink->is_done) {
        ret = yt_tts_get_speech_frame_871((short *)asink->buf, &gen_count);
        if (ret == 0)
            asink->is_done = TRUE;
        gen_count *= 2;
        TTS_DBG("engine ret=%d, gen_count=%d\n", ret, gen_count);
        /* gen not enough, continue */
        if (gen_count + copy_len < *get_size) {
            memcpy(buffer + copy_len, asink->buf, gen_count);
            copy_len += gen_count;
            TTS_DBG("gen not enough, continue, copy_len=%d\n", copy_len);
        } else {
            /* gen enough, will return */
            memcpy(buffer + copy_len, asink->buf, *get_size - copy_len);

            if (asink->is_done && asink->cur_pos == asink->cur_len) {
                asink->cur_pos = 0;
                asink->cur_len = 0;
                TTS_DBG("gen enough, and just done\n");
                return ASTREAM_DATA_EOS;
            } else {
                asink->cur_pos = *get_size - copy_len;
                asink->cur_len = gen_count;
                TTS_DBG("gen enough, cur_pos=%d, cur_len=%d\n",
                      asink->cur_pos, asink->cur_len);
                return 0;
            }
        }
    }

    *get_size = copy_len;
    TTS_DBG("finished\n");

    return ASTREAM_DATA_EOS;
}

struct audio_client asink_client_tts = {
    .type = ASTREAM_TYPE_TTS,
    .aplay_type = APLAY_TYPE_TTS,
    .open = asink_tts_open,
    .start = asink_tts_start,
    .stop = asink_tts_stop,
    .close = asink_tts_close,
    .cb = asink_tts_callback
};

static int asink_tts_init(void)
{
    audio_client_register(&asink_client_tts);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(asink_tts_init);

