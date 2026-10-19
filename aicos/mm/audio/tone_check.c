
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02fdc7
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_osa.h>
#include <alog_api.h>

#ifdef CONFIG_AUDIO_SUPPORT
#include <audio_system.h>
#include "audio_service.h"
/*
 * 测试方式：
 * 1， 喇叭播放一个包含目标频率1477Hz且不包含背景参考7000Hz的信号（频率分辨率+-10%）。
 * 2， 麦克风以16KHz，单身道，16bit录音。
 *
 * 检测方式：
 * 1， 级联4个相同的带通滤波器检查目标频率1477。
 * 2， 级联4个相同的带通滤波器检查频率7000的信号做为背景参考。
 * 3， 目标频率平均幅度大于满幅的5%且大于背景参考10倍以上，认为测试通过，否则认为测试失败。
 */

#define INLINE
#define FRAME_COUNT 320
#undef  ABS
#define ABS(val) ((val) > 0.f ? (val):-(val))

#define ONE_PRESET_FULL_SCALE  327.67f
#define TOTAL_COUNT 100
#define PASS_RATIO  80

static float g_coeff[4][5] =
{
    //BPF 1477
    {
        0.21730715036392211914f,
        0.00000000000000000000f,
       -0.21730715036392211914f,
       -1.32667732238769531250f,
        0.58608156442642211914f,
    },
    //BPF 7000
    {
        0.16184800863265991211f,
        0.00000000000000000000f,
       -0.16184800863265991211f,
        1.56294357776641845703f,
        0.69171798229217529297f,
    },
};

typedef struct
{
    float     coeff[4]; //b1, b2, a1, a2;
    float     b0;
    int32_t   reserved;
    float     fifo[4];
}biquad_iir_t;


INLINE void* memset32(void* dst, int32_t val, size_t n32) {
    size_t i;
    int32_t* dst32 = (int32_t*)dst;
    for (i = 0; i < n32; i++) dst32[i] = val;
    return dst;
}

INLINE void biquadIirReset(biquad_iir_t* p_iir)
{
    memset32(p_iir->fifo, 0, 4);
}

INLINE void biquadIirSetCoeff(biquad_iir_t* p_iir, float* coeff)
{
    p_iir->b0 = coeff[0];
    p_iir->coeff[0] = coeff[1];
    p_iir->coeff[1] = coeff[2];
    p_iir->coeff[2] = -coeff[3];
    p_iir->coeff[3] = -coeff[4];
    memset32(p_iir->fifo, 0, 4);
}

void biquadIirApply(biquad_iir_t* p_iir, float* in_buf, float* out_buf, int32_t samples)
{
    float* fifo = p_iir->fifo;
    float y0, x0;
    float* coeff = p_iir->coeff;
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;

    b0 = p_iir->b0;
    b1 = coeff[0];
    b2 = coeff[1];
    a1 = coeff[2];
    a2 = coeff[3];

    x1 = fifo[0];
    x2 = fifo[1];
    y1 = fifo[2];
    y2 = fifo[3];

    for (int32_t i = 0; i < samples; i++) {
        x0 = in_buf[i];
        y0 = b0 * x0;
        y0 += b1 * x1;
        y0 += b2 * x2;
        y0 += a1 * y1;
        y0 += a2 * y2;
        out_buf[i] = y0;

        y2 = y1;
        y1 = y0;
        x2 = x1;
        x1 = x0;
    }

    fifo[0] = x1;
    fifo[1] = x2;
    fifo[2] = y1;
    fifo[3] = y2;
}

INLINE float AbsSum(float* p_buf, int samples) {
    float sum = 0.f;

    for (int i = 0; i < samples; i++) {
        sum += ABS(p_buf[i]);
    }
    return sum;
}

/*
 * p_sample: the buffer to store float mono data with 16KHz sample rate.
 * p_tmp;    temp buffer with the same size of "p_samples"
 * samples : sample count in float of "p_samples".
 */
static int toneCheck(float* p_sample, float *p_tmp, int samples,
                     u32 *bd1, u32 *bd2)
{
    biquad_iir_t band1[4];
    biquad_iir_t band2[4];
    float band1_sum;
    float band2_sum;

    for (int i = 0; i < 4; i++) {
        biquadIirSetCoeff(&(band1[i]), g_coeff[0]);
        biquadIirSetCoeff(&(band2[i]), g_coeff[1]);
    }

    biquadIirApply(&(band1[0]), p_sample, p_tmp, samples);
    for (int i = 1; i < 4; i++) {
        biquadIirApply(&(band1[i]), p_tmp, p_tmp, samples);
    }
    band1_sum = AbsSum(p_tmp, samples);

    biquadIirApply(&(band2[0]), p_sample, p_tmp, samples);
    for (int i = 1; i < 4; i++) {
        biquadIirApply(&(band2[i]), p_tmp, p_tmp, samples);
    }
    band2_sum = AbsSum(p_tmp, samples);

#if 0
    //5% of full scale
    if (band1_sum < ONE_PRESET_FULL_SCALE * 1 * samples) {
        return 0;
    }
    if (band1_sum < band2_sum * 20.f) {
        return 0;
    }
#endif

    *bd1 = (u32)band1_sum;
    *bd2 = (u32)band2_sum;

    return 1;
}

extern int audio_client_play(ACLIENT aclient, void *msg);
static ACLIENT tone_hd;

int audio_play_tone_fast(enum tone_type tone_type, u32 duation)
{
    struct audio_msg_play msg;
    struct audio_play_sub_tone sub;
    int ret;

    tone_hd = audio_open(ASTREAM_TYPE_TONE, APLAY_TYPE_TONE,
                    AUDIO_INPUT_FLAG_FAST, 16000, 0,
                    ASTREAM_FORMAT_PCM, ASTREAM_CHN_MONO,
                    ASTREAM_DIR_PLAY, ASTREAM_SOURCE_DEFAULT);
    if (tone_hd == 0) {
        AIDXE(0x1bcbaa, "", "play tone failed, create handle failed\n");
        return -1;
    }

    msg.cmd = AUDIO_CMD_PLAY;
    msg.handle = tone_hd;
    msg.aplay_type = APLAY_TYPE_TONE;
    msg.aplay_data = &sub;

    sub.tone_id = tone_type;
    sub.duation = duation;

    ret = audio_client_play(tone_hd, &msg);

    return ret;
}

static void audio_stop_tone_fast(void)
{
    if (tone_hd) {
        audio_stop(tone_hd);
        audio_close(tone_hd);
        tone_hd = 0;
    }
}

u32 tone_check(u32 in_dev, u32 out_dev, u32 freq, u32 *avg1, u32 *avg2)
{
    int cnt = 0, i, ret;
    ACLIENT ac;
    static s16 *i16_buf = 0;
    static float *in_buf = 0, *tmp_buf = 0;
    u32 band1 = 0, band2 = 0, band1_total = 0, band2_total = 0;

    i16_buf = (s16 *)aos_malloc(sizeof(u16) * FRAME_COUNT);
    if (!i16_buf) {
        aicx_info(0x12bce2, "", "tone check, no mem!\n");
        goto err;
    }
    in_buf = (float *)aos_malloc(sizeof(float) * FRAME_COUNT * 2);
    if (!in_buf) {
        aicx_info(0x12bce2, "", "tone check, no mem!\n");
        goto err;
    }
    tmp_buf = in_buf + FRAME_COUNT;

    audio_play_tone_fast(TONE_DTMF_6, 3000);
    ac = audio_open_input(APLAY_TYPE_STREAM, ASTREAM_FORMAT_PCM,
                          16000, ASTREAM_CHN_MONO, 0, 0);
    if (!ac)
        goto err;
    audio_set_volume(ac, 10);
    /* TODO */
    /* audio_force_in_device(in_dev); */
    /* audio_force_out_device(out_dev); */
    audio_play_stream(ac, 0, 0);
    while (cnt++ < TOTAL_COUNT) {
        ret = audio_read(ac, (char *)i16_buf, FRAME_COUNT * sizeof(u16), 100);
        if (ret <= 0)
            continue;
        for (i = 0; i < FRAME_COUNT; i++)
            in_buf[i] = i16_buf[i];
        audio_data((u8 *)i16_buf, FRAME_COUNT * sizeof(u16), 0);
        audio_data((u8 *)in_buf, FRAME_COUNT * sizeof(float), 1);
        toneCheck(in_buf, tmp_buf, FRAME_COUNT, &band1, &band2);
        aicx_info(0x097867, "ddd", "%3d: %d, %d\n",cnt, band1, band2);
        band1_total += band1;
        band2_total += band2;
    }
    audio_stop_tone_fast();
    audio_stop(ac);
    audio_close(ac);

    *avg1 = band1_total / TOTAL_COUNT;
    *avg2 = band2_total / TOTAL_COUNT;

err:
    if (i16_buf)
        aos_free(i16_buf);
    if (in_buf)
        aos_free(in_buf);

    return 0;
}
#else
u32 tone_check(u32 in_dev, u32 out_dev, u32 freq)
{
    aicx_info(0x0a35ba, "", "Audio is not supportted!!!\n");
    return 0;
}
#endif
