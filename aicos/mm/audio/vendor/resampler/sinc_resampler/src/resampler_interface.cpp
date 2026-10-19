
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x130469
#define LOG_MODULE    AMOD_AUDIO
#include "aic_resampler_api.h"
#include "AudioResamplerSinc.h"
#include "sincrsm_adpter.h"




extern "C" {

    void* resampler_init(memory_api_t* p_memory, AICInit_t* init_param)
    {
        AICInit_t* p_param = (AICInit_t*)init_param;
        AudioResamplerSinc* pInstance;
        AudioResamplerSinc::src_quality quality = AudioResamplerSinc::HIGH_QUALITY;
        memory_func_t memory_func;

        if (p_memory) {
            RSM_AIC_ASSERT((p_memory->pf_heap_malloc != NULL) &&
                (p_memory->pf_heap_free != NULL));
            memory_func.pmalloc = (resampler_malloc*)p_memory->pf_heap_malloc;
            memory_func.pfree = (resampler_free*)p_memory->pf_heap_free;
        }
        else {
            RSM_AIC_ASSERT(0);
        }
        pInstance = AudioResamplerSinc::create(&memory_func, AUDIO_FORMAT_PCM_16_BIT, p_param->ch_i, p_param->fs_o, p_param->in_samplenum, quality);

        pInstance->setSampleRate(p_param->fs_i);
        pInstance->check_inout_ratio();
        resampler_log(LOG_LEVEL_INFO, "resampler init fsin:%d, fs_o:%d, ch_i:%d, ch_o:%d", p_param->fs_i, p_param->fs_o, p_param->ch_i, p_param->ch_o);
        return pInstance;
    }

    void  resampler_deinit(void* handle)
    {
        AudioResamplerSinc* pInstance = (AudioResamplerSinc*)handle;

        if (handle) {
            ((AudioResamplerSinc*)pInstance)->AudioResamplerSinc_deinit();
            if (pInstance->mFree)
                pInstance->mFree(NULL, pInstance);
        }
    }

    int   resampler_process(void* handle, const short* in_buf, unsigned int in_size8, short* out_buf, unsigned int* outsample_num)
    {
        AudioResamplerSinc* pResampler;
        unsigned int in_sample;
        pResampler = (AudioResamplerSinc*)handle;
        AudioBufferProvider provider;

        in_sample = in_size8;
        *outsample_num = pResampler->resample(in_buf, in_sample, (int16_t*)out_buf, size_t(*outsample_num), &provider);
        return 0;
    }

    int   resampler_control(void* handle, int param_code, void* p_param, void *out)
    {
        AudioResamplerSinc* pResampler = (AudioResamplerSinc*)handle;

        switch (param_code) {
        case AIC_RESAMPLER_GET_MAXOUTSIZE:
            *((int32_t*)out) = pResampler->get_max_outputsize();
            resampler_log(LOG_LEVEL_INFO, "yzl add resampler get output size:%d", *((int32_t*)out));

            break;
        }
        return 0;
    }

}
