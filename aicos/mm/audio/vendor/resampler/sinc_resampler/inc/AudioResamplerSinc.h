/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_AUDIO_RESAMPLER_SINC_H
#define ANDROID_AUDIO_RESAMPLER_SINC_H

#include <stdint.h>
#include <sys/types.h>
#include "aic_resampler_api.h"
#include "aicxtek_audio_api.h"

#define CC_LIKELY
#define CC_UNLIKELY



extern "C" void memset(void*s, int ch, size_t n);
extern "C" void *memcpy(void *dst, void *src, size_t n);
extern "C" void *memmove(void *dst, const void *src, size_t size);

typedef const int32_t * (*readCoefficientsFn)(bool upDownSample);
typedef int32_t (*readResampleFirNumCoeffFn)();
typedef int32_t (*readResampleFirLerpIntBitsFn)();


typedef void* (resampler_malloc)(void* mem_state, unsigned int size8);
typedef void   (resampler_free)(void* mem_state, void* mem_ptr);


typedef struct {
    resampler_malloc* pmalloc;
    resampler_free* pfree;
} memory_func_t;

typedef enum {
    AUDIO_FORMAT_PCM_16_BIT = 0x1,

}audio_format_t;

typedef struct {
    int8_t div_ratio;
    int8_t div_exact;
}samplediv_info_t;

static inline uint32_t u4_28_from_float(float f)
{
    static const float scale = (float)(1 << 28);
    static const float limpos = ((float)0xffffffffUL) / scale;

    if (f <= 0.) {
        return 0;
    }
    else if (f >= limpos) {
        return 0xffffffff;
    }
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return f * scale + 0.5;
}

/**
 * Convert a single-precision floating point value to a U4.12 integer value.
 * Rounds to nearest, ties away from 0.
 *
 * Values outside the range [0, 16.0) are properly clamped to [0, 65535]
 * including -Inf and +Inf. NaN values are considered undefined, and behavior may change
 * depending on hardware and future implementation of this function.
 */
 // ----------------------------------------------------------------------------
static inline uint16_t u4_12_from_float(float f)
{
    static const float scale = (float)(1 << 12);
    static const float limpos = 0xffff / scale;

    if (f <= 0.) {
        return 0;
    }
    else if (f >= limpos) {
        return 0xffff;
    }
    /* integer conversion is through truncation (though int to float is not).
     * ensure that we round to nearest, ties away from 0.
     */
    return f * scale + 0.5;
}


class AudioBufferProvider
{
public:
    class Buffer {
    public:
        size_t frameCount;
        const int16_t* i16;
        const void* raw;
    };
    int getNextBuffer(Buffer* buffer);
    void releaseBuffer(Buffer* buffer);
};

class AudioResamplerSinc {
public:
    enum src_quality {
        DEFAULT_QUALITY = 0,
        LOW_QUALITY = 1,
        MED_QUALITY = 2,
        HIGH_QUALITY = 3,
        VERY_HIGH_QUALITY = 4,
        DYN_LOW_QUALITY = 5,
        DYN_MED_QUALITY = 6,
        DYN_HIGH_QUALITY = 7,
    };
    static const constexpr float UNITY_GAIN_FLOAT = 1.0f;

    static AudioResamplerSinc* create(memory_func_t *memory_func, audio_format_t format, int inChannelCount,
        int32_t sampleRate, int32_t inSampleCount, src_quality quality = DEFAULT_QUALITY);

    AudioResamplerSinc(int inChannelCount, int32_t sampleRate, int32_t inSampleCount,
            src_quality quality = HIGH_QUALITY);

    ~AudioResamplerSinc();

    void AudioResamplerSinc_init(memory_func_t *memory_func, int inChannelCount,
        int32_t sampleRate, int32_t inSampleCount, src_quality quality);

    void AudioResamplerSinc_deinit();

    size_t resample(const int16_t *inbuf, int32_t in_sample, /*int32_t**/int16_t *out, size_t outFrameCount,
            AudioBufferProvider* provider);

    void setSampleRate(int32_t inSampleRate);
    void setVolume(float left, float right);
    int32_t get_max_outputsize();
    void* memalign(uint32_t align, uint32_t size);
    void check_inout_ratio();
public:
    resampler_malloc* mMalloc;
    resampler_free* mFree;
    samplediv_info_t mdiv_info;

protected:

    // number of bits for phase fraction - 30 bits allows nearly 2x downsampling
    static const int kNumPhaseBits = 30;

    // phase mask for fraction
    static const uint32_t kPhaseMask = (1LU << kNumPhaseBits) - 1;

    // multiplier to calculate fixed point phase increment
    static const double kPhaseMultiplier;

    inline size_t getInFrameCountRequired(size_t outFrameCount) {
        return (static_cast<uint64_t>(outFrameCount) * mInSampleRate
            + (mSampleRate - 1)) / mSampleRate;
    }

    inline float clampFloatVol(float volume) {
        if (volume > UNITY_GAIN_FLOAT) {
            return volume;// UNITY_GAIN_FLOAT;
        }
        else if (volume >= 0.) {
            return volume;
        }
        return 0.;  // NaN or negative volume maps to 0.
    }
private:
    void init();


    template<int CHANNELS>
    size_t resample(const int16_t *inbuf, int32_t in_sample,/*int32_t**/int16_t* out, size_t outFrameCount,
            AudioBufferProvider* provider);

    template<int CHANNELS>
    inline void filterCoefficient(
            int16_t* out, uint32_t phase, const int16_t* samples, uint32_t vRL);

    template<int CHANNELS>
    inline void interpolate(
            int32_t& l, int32_t& r,
            const int32_t* coefs, size_t offset,
            int32_t lerp, const int16_t* samples);

    template<int CHANNELS>
    inline void read(int16_t*& impulse, uint32_t& phaseFraction,
            const int16_t* in, size_t inputIndex);

    int16_t *mState;
    int16_t *mImpulse;
    int16_t *mRingFull;
    int32_t mVolumeSIMD[2];

    const int32_t * mFirCoefs;
    static const uint32_t mFirCoefsDown[];
    static const uint32_t mFirCoefsUp[];

    // ----------------------------------------------------------------------------
    static const int32_t RESAMPLE_FIR_NUM_COEF       = 8;
    static const int32_t RESAMPLE_FIR_LERP_INT_BITS  = 7;

    struct Constants {
        int coefsBits;
        int cShift;
        uint32_t cMask;
        int pShift;
        uint32_t pMask;
        // number of zero-crossing on each side
        unsigned int halfNumCoefs;
    };

    static Constants highQualityConstants;
    static Constants veryHighQualityConstants;
    const Constants *mConstants;    // points to appropriate set of coefficient parameters

    /*aicxtek move from AudioResampler*/
    int32_t mChannelCount;
    int32_t mSampleRate;
    int32_t mMaxInSampleCount;
    int32_t mInSampleRate;
    AudioBufferProvider::Buffer mBuffer;
#if 0
    int16_t* mTempOut;
#endif
    union {
        int16_t mVolume[2];
        uint32_t mVolumeRL;
    };
    int16_t mTargetVolume[2];
    size_t mInputIndex;
    int32_t mPhaseIncrement;
    uint32_t mPhaseFraction;

    src_quality mQuality;

    static void init_routine();
};



#endif
