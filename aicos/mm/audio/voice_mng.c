/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines APIs about voice process between ap and cp.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a4d00
#define LOG_MODULE    AMOD_AUDIO

#include <aic_errno.h>
#include <aipc_api.h>
#ifdef CONFIG_VOICE_STREAM
#include <cmux.h>
#include "cmux_cfg.h"
#endif
#include <aic_cmd_api.h>
#include <slp_inc.h>

#include "audio_system.h"
#include "audio_util.h"
#include "voice_mng.h"
#include "audio_service.h"
#include "audio_sink.h"
#include "audio_record.h"
#include "audio_stream.h"

int audio_send_cp_msg(u16 cmd, u32 param)
{
    AMSG_T msg;

    AIDXW(0x110a62, "dx", "send cp cmd = %d, param = 0x%x\n", cmd, param);

    msg.ty = AIPC_PROT_AMSG;
    msg.id = AMOD_AUDIO;
    msg.p1 = cmd;
    msg.p2 = param;

    return amsg_send_msg(AIC_SYS_CP, (AMSG_T *)&msg, 3000);
}

int audio_proc_cp_msg(AMSG_T *msg, void *p)
{
    struct audio_service_info *info = (struct audio_service_info *)p;

    if (!msg)
        return AIC_EPIPE;

    AIDXW(0x150474, "dd", "cp cmd = %d, value=%d\n", msg->p1, msg->p2);

    switch(msg->p1) {
    case VOICE_MSG_VOICE_SRATE:
        info->voice_srate = msg->p2;
        break;
    default:
        break;
    }

    return 0;
}

int voice_route_notify(bool is_on)
{
    return audio_send_cp_msg(VOICE_MSG_ROUTE_READY, is_on);
}

#ifdef CONFIG_VOICE_STREAM

#define VOICE_STREAM_PROC_THD_NAME  "voice stream proc"
#define VOICE_STREAM_PROC_STACK     2048
#define VOICE_STREAM_PROC_PRI       5

#define VOICE_STS_IDLE              0
#define VOICE_STS_START             1
#define VOICE_STS_STOPPING          2
#define VOICE_STS_STOPPED           3

struct voice_stream {
    u32 ul_handle;  /* record from dsp, actually it's voice downlink data */
    u32 dl_handle;  /* send to dsp, actually it's voice uplink data */
};

char voice_tmp_buf[640];
char voice_tmp_buf_r[640];

static u32 voice_stream_state;
static aos_thread_id g_voice_stream_thd;
static void *voice_cmux_handle;
static u8 is_voice_loop;

void voice_stream_proc_loop(void *data)
{
    struct voice_stream *stream = (struct voice_stream *)data;
    char *buf = voice_tmp_buf;
    u32 ul_handle, dl_handle;
    int len, rd_len, wr_len = 0;
    void *cmux_handle = voice_cmux_handle;

    ul_handle = stream->ul_handle;
    dl_handle = stream->dl_handle;
    len = 640;
    AIDXI(0x15dd3a, "xx", "voice stream thread, ul=0x%x, dl=0x%x\n", ul_handle, dl_handle);

    aic_get_wakelock();
    if (is_voice_loop) {
        memset(buf, 0, len);
        audio_stream_write(ul_handle, buf, len);
        while(voice_stream_state == VOICE_STS_START) {
            rd_len = audio_stream_read(dl_handle, buf, len);
            if (rd_len > 0) {
                wr_len = audio_stream_write(ul_handle, buf, rd_len);
                audio_data(buf, wr_len, 0);
            } else {
                memset(buf, 0, len);
                wr_len = audio_stream_write(ul_handle, buf, len);
            }
            AIDXD(0x104f5a, "dd", "voice stream, rd_len=%d, wr_len=%d\n", rd_len, wr_len);
        }
    } else {
        while(voice_stream_state == VOICE_STS_START) {
            /* read from remote and send to dsp */
            rd_len = aic_cmux_read(cmux_handle, buf, len, 21);
            if (rd_len <= 0) {
                memset(buf, 0, len);
                AIDXI(0x0a2382, "d", "voice cmux read fail, %d\n", rd_len);
            }
            wr_len = audio_stream_write(ul_handle, buf, len);
            if (wr_len <= 0) {
                AIDXI(0x0f3771, "d", "voice write fail, %d\n", wr_len);
            } else {
                audio_data(buf, wr_len, 0);
            }
            wr_len = aic_cmux_write(cmux_handle, voice_tmp_buf_r, len, 11);
            if (wr_len <= 0) {
                AIDXI(0x1b5d2b, "d", "voice cmux write fail, %d\n", wr_len);
            } else {
                audio_data(voice_tmp_buf_r, wr_len, 1);
            }
            rd_len = audio_stream_read(dl_handle, voice_tmp_buf_r, len);
            if (rd_len <= 0) {
                memset(voice_tmp_buf_r, 0, len);
                AIDXI(0x05c06c, "d", "voice read fail, %d\n", rd_len);
            }
        }
    }
    aic_put_wakelock();

    voice_stream_state = VOICE_STS_STOPPED;
    AIDXI(0x15f808, "", "voice stream loop exit\n");
}

int voice_stream_start(void)
{
    struct voice_stream stream;
    int ul, dl;

    if (voice_stream_state != VOICE_STS_IDLE && voice_stream_state != VOICE_STS_STOPPED) {
        AIDXE(0x158a71, "", "voice stream already started\n");
        return FALSE;
    }
    ul = audio_get_voice_ul_dev();
    dl = audio_get_voice_dl_dev();
    if (!ul || !dl) {
        AIDXE(0x00841d, "", "voice stream handle is 0\n");
        return AIC_EPIPE;
    }

    if (g_voice_stream_thd && g_voice_stream_thd != AOS_INVALID_THREAD_ID) {
        aos_delete_thread(g_voice_stream_thd);
        g_voice_stream_thd = 0;
    }
    if (voice_cmux_handle) {
        AIDXE(0x1b86ec, "", "voice cmux not close!\n");
        return AIC_EPIPE;
    }
    voice_cmux_handle = aic_cmux_open(AIC_CMUX_VOICE_COM);
    if (!voice_cmux_handle) {
        AIDXE(0x0cdb29, "", "voice cmux open failed!\n");
        return AIC_EPIPE;
    }
    stream.ul_handle = ul;
    stream.dl_handle = dl;
    voice_stream_state = VOICE_STS_START;
    g_voice_stream_thd = aos_create_thread(VOICE_STREAM_PROC_THD_NAME,
                                           voice_stream_proc_loop, &stream,
                                           VOICE_STREAM_PROC_STACK, 0, NULL,
                                           VOICE_STREAM_PROC_PRI,
                                           AOS_AUTO_START);

    return (g_voice_stream_thd == AOS_INVALID_THREAD_ID);
}

int voice_stream_stop(void)
{
    if (voice_stream_state != VOICE_STS_START) {
        AIDXI(0x069d32, "", "voice stream not start!\n");
        return AIC_EPERM;
    }
    voice_stream_state = VOICE_STS_STOPPING;

    while (voice_stream_state != VOICE_STS_STOPPED)
        aos_sleep(10);

    if (voice_cmux_handle) {
        aic_cmux_close(voice_cmux_handle);
        voice_cmux_handle = 0;
    }
    AIDXI(0x0cf628, "", "voice stream exit!\n");

    return 0;
}

static int voice_loop(int argc, char **argv)
{
    if (argc == 1) {
        is_voice_loop = (argv[0][0] == '0' ? 0 : 1);
        CONS_OUT("voice loop %s\n", is_voice_loop ? "on" : "off");
    }

    return 0;
}
CONSOLE_EXPORT_CMD(voice_loop, voice_loop, voice stream loop set);

#endif
