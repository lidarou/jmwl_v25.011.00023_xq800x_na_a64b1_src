

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x109662
#define LOG_MODULE    AMOD_AUDIO
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>

#include "audio_util.h"
#include "eas.h"

#define MIDI_OUT_BUF_LEN (512 * 1024)
static short pcm_buf[MIDI_OUT_BUF_LEN];
static const unsigned char midi_data[] = {
    #include "midi_file.h"
};

static EAS_BOOL eas_change(EAS_VOID_PTR pInstData, S_EXT_AUDIO_PRG_CHG *pPrgChg)
{
    CONS_OUT("eas_change: pInstData=%p\n", pInstData);
    CONS_OUT("\tbank=%d, channel=%d, program=%d\n",
             pPrgChg->bank, pPrgChg->channel, pPrgChg->program);

    return TRUE;
}

static EAS_BOOL eas_event_callback(EAS_VOID_PTR pInstData,
                                   S_EXT_AUDIO_EVENT *pEvent)
{
    CONS_OUT("event callback: pInstData=%p\n", pInstData);
    CONS_OUT("\tchannel=%d, note=%d, noteOn=%d, velocity=%d\n",
             pEvent->channel, pEvent->note, pEvent->noteOn, pEvent->velocity);

    return 0;
}

static int midi_test(int argc, char **argv)
{
    S_EAS_LIB_CONFIG *config;
    EAS_DATA_HANDLE eas_data;
    EAS_HANDLE eas_handle;
    u32 ret, gen_total = 0;
    EAS_I32 cur_pos, play_len, gen_size, state;
    short *pbuf;
    S_EAS_MEMFILE_DESCRIPTOR midi_desc;

    midi_desc.dataBuffer = (unsigned char *)midi_data;
    midi_desc.fileSize = sizeof(midi_data);

    config = (S_EAS_LIB_CONFIG *)EAS_Config();
    if (!config) {
        CONS_OUT("config is NULL\n");
        return 0;
    }
    CONS_OUT("libVersion=%d\n", config->libVersion);
    CONS_OUT("checkedVersion=%d\n", config->checkedVersion);
    CONS_OUT("maxVoices=%d\n", config->maxVoices);
    CONS_OUT("numChannels=%d\n", config->numChannels);
    CONS_OUT("sampleRate=%d\n", config->sampleRate);
    CONS_OUT("mixBufferSize=%d\n", config->mixBufferSize);
    CONS_OUT("filterEnabled=%d\n", config->filterEnabled);
    CONS_OUT("buildTimeStamp=%d\n", config->buildTimeStamp);
    CONS_OUT("buildGUID=%d\n", config->buildGUID);

    ret = (u32)EAS_Init(&eas_data);
    if (ret) {
        CONS_OUT("init failed, ret=%d\n", ret);
        return 0;
    }

    ret = EAS_OpenFile(eas_data, (EAS_FILE_LOCATOR *)&midi_desc, &eas_handle);
    if (ret) {
        CONS_OUT("open failed, ret=%d\n", ret);
        goto end_test;
    };
    ret = EAS_Prepare(eas_data, eas_handle);
    if (ret) {
        CONS_OUT("prepare failed, ret=%d\n", ret);
        goto end_test;
    }
    ret = EAS_RegExtAudioCallback(eas_data, eas_handle, NULL,
                                  eas_change, eas_event_callback);
    if (ret) {
        CONS_OUT("regiest callback failed, ret=%d\n", ret);
        goto end_test;
    }

    ret = EAS_SetPolyphony(eas_data, eas_handle, 16);
    if (ret) {
        CONS_OUT("SetPolyphony failed, ret=%d\n", ret);
        goto end_test;
    }

    ret = EAS_ParseMetaData(eas_data, eas_handle, &play_len);
    if (ret) {
        CONS_OUT("ParseMetaData failed, ret=%d\n", ret);
        goto end_test;
    }

    //ret = EAS_SetPlaybackRate(eas_data, eas_handle, 0);
    //ret = EAS_Locate(eas_data, eas_handle, 0, FALSE);
    pbuf = pcm_buf;
    while(1) {
        ret = EAS_GetLocation(eas_data, eas_handle, &cur_pos);
        if (ret) {
            CONS_OUT("GetLocation failed, ret=%d\n", ret);
            break;
        }
        ret = EAS_Render(eas_data, pbuf, config->mixBufferSize, &gen_size);
        if (ret) {
            CONS_OUT("rendor failed, ret=%d\n", ret);
            break;
        }
        pbuf += gen_size * config->numChannels;
        gen_total += gen_size;
        ret = EAS_State(eas_data, eas_handle, &state);
        if (ret) {
            CONS_OUT("get state failed, ret=%d\n", ret);
            break;
        }
        if ((gen_total * config->numChannels > MIDI_OUT_BUF_LEN) ||
            (state == EAS_STATE_STOPPED) || (state == EAS_STATE_ERROR) ||
            (state == EAS_STATE_EMPTY) ) {
            CONS_OUT("state=%d\n", state);
            break;
        }
    }

end_test:
    ret = EAS_Shutdown(eas_data);
    if (ret) {
        CONS_OUT("shutdown failed, ret=%d\n", ret);
        return 0;
    }

    CONS_OUT("over! gen_total=%d\n", gen_total);

    return 0;
}
CONSOLE_EXPORT_CMD(midi_test, midi_test, midi test);

