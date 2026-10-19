#ifndef AIC_RESAMPLER_API_H
#define AIC_RESAMPLER_API_H
#include <stdint.h>
#include "aicxtek_audio_api.h"


typedef void (resampler_log_f)(const char* fmt, ...);

typedef enum {
    AIC_RESAMPLER_GET_MAXOUTSIZE,
    AIC_RESAMPLER_CMD_MAX,
}aic_resampler_cmd;

typedef enum {
    AIC_RESAMPLER_SUCCESS,
    AIC_RESAMPLER_INV_PARAM,
    AIC_RESAMPLER_FAULT,
}aic_resampler_result_t;

typedef struct log_info
{
    resampler_log_f* log_func;
}aic_log_param_t;

typedef struct AICInit {
    int fs_i;
    int fs_o;
    int ch_i;
    int ch_o;
    uint32_t in_samplenum;
    aic_log_param_t log_param;
} AICInit_t;


#ifdef __cplusplus
extern "C" {
#endif

void *resampler_init(memory_api_t *memory_api, AICInit_t* p_aicinit);
/*cmd AIC_RESAMPLER_GET_MAXOUTSIZE --> output is uint32_t* */
int resampler_control(void* handle, int cmd, void *input, void *output);
int resampler_process(void *handle, const short* inbuf, unsigned int inszie, short* outbuf, unsigned int *outsize);
void resampler_deinit(void* handle);

#ifdef __cplusplus
}
#endif

#endif