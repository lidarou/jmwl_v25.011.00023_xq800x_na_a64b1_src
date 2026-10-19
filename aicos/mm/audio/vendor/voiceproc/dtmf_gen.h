/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This is the header file for audio sink and record common, some structs
 *  and APIs are used both audio sink and record.
 *
 *****************************************************************************/

#ifndef _DTMF_GEN_H
#define _DTMF_GEN_H
#include "aicxtek_audio_api.h"

enum {
    AIC_DTMF_CMD_CODE_RESERVED,
    AIC_DTMF_CMD_PROPERTY,
};

enum {
    AIC_DTMF_RC_SUCCEED       = 0,
    AIC_DTMF_RC_GENERIC_ERR   = -100,
    AIC_DTMF_RC_CTRL_CODE_ERR = -101,
    AIC_DTMF_RC_SAMPLRATE_ERR = -102,
    AIC_DTMF_RC_KEY_ERR       = -103,
    AIC_DTMF_RC_VOLUME_ERR    = -104,
};

typedef struct {
    int sampleRate; //16000(WB) or 8000(NB)
    int key;        // one of char in "123A456B768C*0#D", for example 'A'
    int volume_level;// 0-100. 0: silence.  100: max volume
}dtmf_cmd_property_t;

void * aic_dtmf_create(memory_api_t* p_memory, int* init_param, unsigned int param_size32);
void   aic_dtmf_delete(void* handle);
int    aic_dtmf_process(void* handle, char* p_out, unsigned int in_size8);
int    aic_dtmf_control(void* handle, int param_code, int* p_param, unsigned int param_size32);

#endif
