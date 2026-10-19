/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is iflyTek TTS adaptor file.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c3884
#define LOG_MODULE    AMOD_VENDOR

#include <aic_osa.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <aos_mem.h>
#include <aic_init.h>
#include <alog_api.h>
#include <fs_api.h>
#include <audio_system.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_cmd_api.h>
#include <flash_mem_config.h>

#include "ivTTS.h"

#define IVTTS_IRF_IN_FILE       0

#if IVTTS_IRF_IN_FILE
#define IVTTS_IRF_ADDR    (void *)fs_open("/O/ivTTS.irf", FS_O_RDONLY);
#else
#define IVTTS_IRF_ADDR    (void *)FLASH1_IVTTS_IRF_BASE;
#endif

/* constant for TTS heap size */
#define IVTTS_HEAP_SIZE         (1024 * 60)

#define IVTTS_STACK_SIZE        (1024 * 4)
#define IVTTS_THREAD_PRI        4
#define IVTTS_QUEUE_CNT         10

#define EAUDIO_DONE             1

enum {
    IVTTS_ST_READY,
    IVTTS_ST_PLAYING,
};
enum {
    IVTTS_CMD_NONE = 0,
    IVTTS_CMD_PLAY = 1,
    IVTTS_CMD_STOP = 2,
};

struct ivtts_msg {
    u32 id;
    void *param;
    u32 size;
};

struct ivtts_ctx {
    ivHTTS hTTS;

    void *heap;
    int irf;
    BOOL inited;
    aos_thread_id thread;
    aos_queue_ptr queue;
    ACLIENT haudio;
    aos_event_ptr event;

    u32 state;
    u32 force_stop;
};

struct ivtts_ctx g_ivtts_ctx;

static int ivtts_audio_cb(u32 data, enum astream_cb_event event)
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;

    aos_set_event(ivtts->event, EAUDIO_DONE, AOS_OR);

    return 0;
}

/* Message */
ivTTSErrID DoMessage(struct ivtts_ctx *ivtts)
{
    if(ivtts->force_stop) {
        ivtts->force_stop = FALSE;
        return ivTTS_ERR_EXIT;
    }

    return ivTTS_ERR_OK;
}

#if IVTTS_IRF_IN_FILE
/* read resource callback */
ivBool ivCall ReadResCB(
        ivPointer        pParameter,        /* [in] user callback parameter */
        ivPointer        pBuffer,        /* [out] read resource buffer */
        ivResAddress    iPos,            /* [in] read start position */
        ivResSize        nSize )            /* [in] read size */
{
    int pFile = (int)pParameter;
    int err;


    fs_seek(pFile, iPos, FS_SEEK_BEGIN);

    err = fs_read(pFile, pBuffer, nSize);
    alog_debug("ReadResCB, irf handle=0x%x, err=%d, nsize=%d\n",
              pFile, err, nSize);
    if (err == (int)nSize)
        return ivTrue;
    else
        return ivFalse;
}
#else
/* read resource callback */
ivBool ivCall ReadResCB(
        ivPointer        pParameter,        /* [in] user callback parameter */
        ivPointer        pBuffer,        /* [out] read resource buffer */
        ivResAddress    iPos,            /* [in] read start position */
        ivResSize        nSize )            /* [in] read size */
{
    char  *pirf;

    pirf = (char *)pParameter + iPos;
    memcpy(pBuffer, pirf, nSize);

    return ivTrue;
}
#endif

/* output callback */
ivTTSErrID ivCall OutputCB(
        ivPointer        pParameter,         /* [in] user callback parameter */
        ivUInt16        nCode,               /* [in] output data code */
        ivCPointer        pcData,            /* [in] output data buffer */
        ivSize            nSize )            /* [in] output data size */
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;

    ivTTSErrID tErr = DoMessage(ivtts);

    if ( tErr != ivTTS_ERR_OK ) {
        aicx_error(0x0829b1, "", "ivtts output exit\n");
        return tErr;
    }
    if (!ivtts->haudio) {
        aicx_error(0x1a070b, "", "ivtts output no audio handle\n");
        return AIC_EPIPE;
    }
    audio_write(ivtts->haudio, (char *)pcData, nSize, 1000);

    if (ivtts->state == IVTTS_ST_READY) {
        int ret;
        ret = audio_play_stream(ivtts->haudio, ivtts_audio_cb, 0);
        if (ret) {
            aicx_error(0x054a6e, "d", "ivtts audio_play failed, ret=%d\n", ret);
            audio_close(ivtts->haudio);
            return ivTTS_ERR_EXIT;
        }
        ivtts->state = IVTTS_ST_PLAYING;
    }

    return 0;
}

/* parameter change callback */
ivTTSErrID ivCall ParamChangeCB(
        ivPointer       pParameter,        /* [in] user callback parameter */
        ivUInt32        nParamID,        /* [in] parameter id */
        ivUInt32        nParamValue )    /* [in] parameter value */
{
    alog_info("iv param change, id=%d\n", nParamID);

    return ivTTS_ERR_OK;
}

/* progress callback */
ivTTSErrID ivCall ProgressCB(
        ivPointer       pParameter,        /* [in] user callback parameter */
        ivUInt32        iProcPos,        /* [in] current processing position */
        ivUInt32        nProcLen )        /* [in] current processing length */
{
    alog_info("iv process cb, pos=%d, len=%d\n", iProcPos, nProcLen);

    return ivTTS_ERR_OK;
}

static BOOL ivtts_wait_audio_comp(void)
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;
    u32 actual_flags = 0;

    aos_get_event(ivtts->event, EAUDIO_DONE, AOS_OR_CLEAR, &actual_flags,
                  AUDIO_WAIT_FOREVER);
    if (actual_flags & EAUDIO_DONE)
        return TRUE;

    aicx_error(0x122d54, "x", "ivtts wait audio comp failed, flag=0x%x\n", actual_flags);
    return FALSE;
}

static void ivtts_thread_loop(void *data)
{
    struct ivtts_ctx *ivtts = data;
    struct ivtts_msg msg;
    int ret;

    ivtts->queue = aos_create_queue("ivtts queue",
                                   sizeof(struct ivtts_msg),
                                   IVTTS_QUEUE_CNT);
    if (!ivtts->queue) {
        aicx_error(0x0cb9a4, "", "ivtts queue create, no memeory\n");
        return;
    }

    while(1) {
        ret = aos_recv_from_queue(ivtts->queue, &msg, AUDIO_WAIT_FOREVER);
        if (ret)
            continue;

        switch (msg.id) {
        case IVTTS_CMD_PLAY:
            ivtts->haudio = audio_open(ASTREAM_TYPE_MUSIC, APLAY_TYPE_STREAM, 0,
                                       16000, 0, ASTREAM_FORMAT_PCM, 1,
                                       ASTREAM_DIR_PLAY, ASTREAM_SOURCE_DEFAULT);
            if (!ivtts->haudio) {
                aicx_error(0x09504d, "", "ivtts audio open failed\n");
                continue;
            }
#if 0
            ret = audio_play_stream(ivtts->haudio, ivtts_audio_cb, 0);
            if (ret) {
                aicx_error(0x054a6e, "d", "ivtts audio_play failed, ret=%d\n", ret);
                audio_close(ivtts->haudio);
                continue;
            }
#endif
            ivtts->state = IVTTS_ST_READY;
            ivTTS_SynthText(ivtts->hTTS, msg.param, msg.size);
            audio_drain(ivtts->haudio);
            ivtts_wait_audio_comp();
            audio_close(ivtts->haudio);
            aos_free(msg.param);
            break;
        case IVTTS_CMD_STOP:
            break;
        default:
            break;
        }
    }
}

static int ivtts_open(void)
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;

    if (!ivtts->inited) {
        aicx_error(0x0bb9fa, "", "iv tts open failed, not init\n");
        return AIC_EPERM;
    }

    ivtts->event = aos_create_event("ivtts");
    AIC_ASSERT(ivtts->event);
    ivtts->thread = aos_create_thread("ivtts",
                                      ivtts_thread_loop,
                                      (void *)ivtts,
                                      IVTTS_STACK_SIZE,
                                      0,
                                      NULL,
                                      IVTTS_THREAD_PRI,
                                      AOS_AUTO_START);

    if (ivtts->thread == AOS_INVALID_THREAD_ID) {
        aicx_error(0x09319f, "", "ivtts create thread failed\n");
        return AIC_EACCES;
    }

    return 0;
}

int ivtts_close(void)
{
    return 0;
}

static int ivtts_init(void)
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;
    ivHTTS hTTS;

    ivTResPackDescExt    tResPackDesc;
    ivTTSErrID        ivReturn;

    if (ivtts->inited) {
        aicx_error(0x1e0cc6, "", "iv tts inited already!");
        return 0;
    }

    ivtts->heap = aos_malloc(IVTTS_HEAP_SIZE);
    if (!ivtts->heap) {
        aicx_error(0x1e0cc6, "", "iv tts inited already!");
        return AIC_ENOMEM;
    }
    memset(ivtts->heap, 0, IVTTS_HEAP_SIZE);


    tResPackDesc.pCBParam = IVTTS_IRF_ADDR;

    aicx_info(0x1bff4f, "x", "irf handle=0x%x\n", tResPackDesc.pCBParam);
    tResPackDesc.pfnRead = ReadResCB;
    tResPackDesc.pfnMap = NULL;
    tResPackDesc.nSize = 0;

    tResPackDesc.pCacheBlockIndex = NULL;
    tResPackDesc.pCacheBuffer = NULL;
    tResPackDesc.nCacheBlockSize = 0;
    tResPackDesc.nCacheBlockCount = 0;
    tResPackDesc.nCacheBlockExt = 0;

    if( !tResPackDesc.pCBParam ) {
        aicx_error(0x1b2dab, "", "can not open resource\n");
        goto err_no_irf;
    }

    ivReturn = ivTTS_Create(&hTTS, (ivPointer)ivtts->heap, IVTTS_HEAP_SIZE,
        ivNull, (ivPResPackDescExt)&tResPackDesc, (ivSize)1, NULL);
    if (ivReturn) {
        aicx_error(0x02f849, "d", "ivTTS_Create failed, return=%d\n", ivReturn);
        goto err_create;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_OUTPUT_CALLBACK, (ivUInt32)OutputCB);
    if (ivReturn) {
        aicx_error(0x0cc92c, "x", "ivTTS_SetParam ivTTS_PARAM_OUTPUT_CALLBACK return 0x%x\n", ivReturn);
        return -1;
    }
#if 0
    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PARAMCH_CALLBACK, (ivUInt32)ParamChangeCB);
    printf("ivTTS_SetParam ivTTS_PARAM_PARAMCH_CALLBACK return 0x%x\n", ivReturn);
    if (ivReturn) {
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_PROGRESS_CALLBACK, (ivUInt32)ProgressCB);
    printf("ivTTS_SetParam ivTTS_PARAM_PROGRESS_CALLBACK return 0x%x\n", ivReturn);
    if (ivReturn) {
        return -1;
    }
#endif
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_ASCII);
    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_GBK);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_INPUT_CODEPAGE, ivTTS_CODEPAGE_UTF8);
    if (ivReturn) {
        aicx_error(0x01b67a, "x", "ivTTS_SetParam ivTTS_PARAM_INPUT_CODEPAGE return 0x%x\n", ivReturn);
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_AUTO);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_ENGLISH);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_CHINESE);
    if (ivReturn) {
        aicx_error(0x11c300, "x", "ivTTS_SetParam ivTTS_PARAM_LANGUAGE return 0x%x\n", ivReturn);
        return -1;
    }
#if 0
    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOLUME, ivTTS_VOLUME_NORMAL);
    printf("ivTTS_SetParam ivTTS_PARAM_VOLUME return 0x%x\n", ivReturn);
    if (ivReturn) {
        return -1;
    }

    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOICE_SPEED, ivTTS_SPEED_NORMAL);
    printf("ivTTS_SetParam ivTTS_PARAM_VOICE_SPEED return 0x%x\n", ivReturn);
    if (ivReturn) {
        return -1;
    }
#endif
    ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOYAN);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_YUFENG);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_NANNAN);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOMENG);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_XIAOJIN);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_TERRY);
    if (ivReturn) {
        aicx_error(0x049ba8, "x", "ivTTS_SetParam ivTTS_PARAM_ROLE return 0x%x\n", ivReturn);
        return -1;
    }

    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_READ_DIGIT, ivTTS_READDIGIT_AS_NUMBER);
    //alog_error("ivTTS_SetParam ivTTS_PARAM_READ_DIGIT return 0x%x\n", ivReturn);
    //if (ivReturn) {
    //    return -1;
    //}

    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_VOICE_SPEED, 250);
    //printf("ivTTS_SetParam ivTTS_PARAM_VOICE_SPEED return 0x%x\n", ivReturn);
    //if (ivReturn) {
    //    return -1;
    //}

    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_CHINESE);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_READ_WORD, ivTTS_READWORD_BY_ALPHA);
    //printf("ivTTS_SetParam return 0x%x\n", ivReturn);
    //if (ivReturn) {
    //    return -1;
    //}
    //char buf[] = { 0xE6,0xAC, 0xA2, 0xE8, 0xBF, 0x8E, 0xE4, 0xBD, 0xBF, 0xE7, 0x94, 0xA8, 0xE7, 0xA7, 0xBB, 0xE8, 0x8A,
    //    0xAF, 0xE6, 0x94, 0xB6, 0xE6, 0xAC, 0xBE, 0xE6, 0x92, 0xAD, 0xE6, 0x8A, 0xA5, 0xE8, 0xAE, 0xBE, 0xE5, 0xA4, 0x87 };
    //char buf[6] = { 0xa2, 0xac, 0xe6, 0x8e, 0xbf, 0xe8, };
    //ivReturn = ivTTS_SynthText(hTTS, buf, sizeof(buf));
    //ivReturn = ivTTS_SynthText(hTTS, ivText("[f0][s10]请挥卡！"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("门前大桥下，游过一群鸭，快来快来数一数，二四六七八。456，hello！"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("[i1]ni3hao3zhong1guo2"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("我梦想有一天，这个国家会站立起来，真正实现其信条的真谛：“我们认为这些真理是不言而喻的;人人生而平等。"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("good morning"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("54363."), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("32.46元"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("1024"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("12.38"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("2/5"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("[n1][y1][f0][p1]1[p1]2[p1]3[p1]4[p1]5[p1]6"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("[f1][y0][s5]0551-82579707"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("[n1][y0]86[p200]021[p200]88568888[p200]6603"), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("14:45"), -1);


    //char buf[10] = { 0 };
    //buf[0] = 0xe4;
    //buf[1] = 0xb8;
    //buf[2] = 0xad;
    //ivReturn = ivTTS_SynthText(hTTS, buf, 3);
    //printf("ivTTS_SynthText return 0x%x\n", ivReturn);


    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_ROLE, ivTTS_ROLE_TERRY);
    //ivReturn = ivTTS_SetParam(hTTS, ivTTS_PARAM_LANGUAGE, ivTTS_LANGUAGE_ENGLISH);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("I have a dream that one day this nation will rise up, live up to the true meaning of its creed: We hold these truths to be self-evident; that all men are created equal."), -1);
    //ivReturn = ivTTS_SynthText(hTTS, ivText("It goes without any question that career plays a key role in our life. In the very first place, career can give us an aim to live on."), -1);

    ivtts->hTTS = hTTS;
    ivtts->inited = TRUE;

    ivtts_open();

    aicx_info(0x12fd01, "", "ivtts inited\n");

    return 0;

err_create:
    if (g_ivtts_ctx.irf)
        fs_close(g_ivtts_ctx.irf);

err_no_irf:
    if (ivtts->heap)
        aos_free(ivtts->heap);

    return AIC_EPERM;
}
AIC_NORMAL_APP_L2_INIT_EXPORT(ivtts_init);

int ivtts_destroy(void)
{
    if (g_ivtts_ctx.hTTS)
        ivTTS_Destroy(g_ivtts_ctx.hTTS);

    if (g_ivtts_ctx.heap)
        free(g_ivtts_ctx.heap);

    if (g_ivtts_ctx.irf)
        fs_close(g_ivtts_ctx.irf);

    memset(&g_ivtts_ctx, 0, sizeof(g_ivtts_ctx));

    aicx_info(0x0c846c, "", "ivtts destroyed\n");

    return 0;
}

int ivtts_play(char *txt, u32 len)
{
    struct ivtts_ctx *ivtts = &g_ivtts_ctx;
    struct ivtts_msg msg;

    if (!ivtts->inited) {
        aicx_error(0x11e676, "", "iv tts play failed, not init\n");
        return AIC_EPERM;
    }

    if (!txt || !len) {
        aicx_error(0x193313, "", "no tts text need play\n");
        return AIC_EPIPE;
    }
    msg.param = aos_malloc(len + 4);
    if (!msg.param) {
        aicx_error(0x0d3587, "", "ivtts play failed, no memory\n");
        return AIC_ENOMEM;
    }
    strcpy(msg.param, txt);
    msg.size = len;
    msg.id = IVTTS_CMD_PLAY;
    ivtts->force_stop = FALSE;
    return aos_send_to_queue(ivtts->queue, &msg, 10000);
}

int ivtts_stop(void)
{
    g_ivtts_ctx.force_stop = TRUE;

    return 0;
}

static int ivtts(int argc, char **argv)
{
    int ret;

    if (argc < 1) {
        CONS_OUT("please input the text you want to play\n");
        return 0;
    }

    ret = ivtts_play(argv[0], strlen(argv[0]));

    CONS_OUT("ivtts play %s!\n", ret ? "failed" : "success");

    return 0;
}
CONSOLE_EXPORT_CMD(ivtts, ivtts, ivtts test);

static int ivtts_stop_test(int argc, char **argv)
{
    int ret;

    ret = ivtts_stop();

    CONS_OUT("ivtts stop %s!\n", ret ? "failed" : "success");

    return 0;
}
CONSOLE_EXPORT_CMD(ivtts_stop, ivtts_stop_test, ivtts_stop test);

