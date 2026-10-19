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


#ifndef INCLUDES_VOICE_PROC_H_
#define INCLUDES_VOICE_PROC_H_
#include <stdint.h>
#include "aicxtek_audio_api.h"



typedef void* (voice_create_mutex)();
typedef void (voice_destroy_mutex)(void* mutex);
typedef int (voice_get_mutex)(void* mutex, int type);
typedef void (voice_put_mutex)(void* mutex);

enum {
    AIC_VOICE_CMD_CODE_RESERVED,
    AIC_VOICE_CMD_NV,
};
enum {
    AIC_PLAYBACK_CMD_CODE_RESERVED,
    AIC_PLAYBACK_CMD_NV,
};
enum {
    AIC_VOICE_RC_SUCCEED = 0,
    AIC_VOICE_RC_GENERIC_ERR = -100,
    AIC_VOICE_RC_CTRL_CODE_ERR = -101,
    AIC_VOICE_RC_CTRL_SIZE_ERR = -102,
};


enum {
    AIC_PLAYBACK_RC_SUCCEED = 0,
    AIC_PLAYBACK_RC_GENERIC_ERR = -100,
    AIC_PLAYBACK_RC_CTRL_CODE_ERR = -101,
    AIC_PLAYBACK_RC_CTRL_SIZE_ERR = -102,
};

enum {
    AIC_VOICE_PROC_WAIT_FOREVER,
    AIC_VOICE_PROC_NO_WAIT,
    AIC_VOICE_PROC_WAIT_MAX,
};



typedef struct {
    voice_create_mutex* mutex_create;
    voice_destroy_mutex* mutex_destroy;
    voice_get_mutex* mutex_get;
    voice_put_mutex* mutex_put;
}voice_proc_lock_fun_t;

typedef struct {
    int32_t chn;
    int32_t fs;
    void *p_extscratch;
    voice_proc_lock_fun_t mutex_func;
}voice_proc_param_t;



void* aic_voice_create(memory_api_t* p_memory_api, voice_proc_param_t* init_param, unsigned int param_size32);

void aic_voice_delete(void* phandle);

int aic_voice_process(void* phandle, char* mic_in, char* ref_in, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);

int aic_voice_control(void* phandle, int param_code, int* p_param, unsigned int param_size32);



void* aic_playback_create(memory_api_t* p_memory_api, voice_proc_param_t *init_param, unsigned int param_size32);

void aic_playback_delete(void* phandle);

int aic_playback_process(void* phandle, char* in, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);

int aic_playback_control(void* phandle, int param_code, int *p_param, unsigned int param_size32);

uint32_t aic_effects_get_scratch_size();
#endif /* INCLUDES_MODULES_EFFECTS_EFFECTS_WRAPPER_H_ */
