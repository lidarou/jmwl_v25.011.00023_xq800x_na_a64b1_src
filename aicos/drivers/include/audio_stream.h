/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio HAL, in which defined audio stream interface and
 *    related struct, MACRO, enum, etc. These API is used by application
 *    to control audio stream.
 */

#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#define STREAM_CHN_PCMP0    0
#define STREAM_CHN_PCMP1    1
#define STREAM_CHN_PCMP2    2
#define STREAM_CHN_PCMC0    3
#define STREAM_CHN_PCMC1    4
#define STREAM_CHN_PCMC2    5
#define STREAM_CHN_COMPRP0  6
#define STREAM_CHN_COMPRP1  7
#define STREAM_CHN_COMPRP2  8
#define STREAM_CHN_COMPRC0  9
#define STREAM_CHN_COMPRC1  10
#define STREAM_CHN_COMPRC2  11
#define STREAM_CHN_MAX      12
#define STREAM_CHN_PCM_MAX  STREAM_CHN_PCMC2

#define STREAM_CHN_DIR_P        STREAM_CHN_PCMP0
#define STREAM_CHN_DIR_C        STREAM_CHN_PCMC0
#define STREAM_CHN_VOICE_DL     STREAM_CHN_COMPRP2
#define STREAM_CHN_VOICE_UL     STREAM_CHN_COMPRC2
#define STREAM_CHN_OFFLD_PCM    STREAM_CHN_COMPRP0
#define STREAM_CHN_OFFLD_COMPR  STREAM_CHN_COMPRP1
#define STREAM_CHN_OFFLD_REC    STREAM_CHN_COMPRC0
#define STREAM_CHN_HWCODEC_DL   1
#define STREAM_CHN_HWCODEC_UL   2
#define STREAM_CHN_VOIP_DL      4
#define STREAM_CHN_VOIP_UL      5
#define STREAM_CHN_VOICE_DL_COMPR_PCM 12
#define STREAM_CHN_VOICE_UL_COMPR_PCM 13
#define STREAM_CHN_VOICE_REC    STREAM_CHN_COMPRC1
#ifdef CONFIG_MCU_AUDIO
#define STREAM_CHN_VOICE_DL_PCM STREAM_CHN_PCMP0
#define STREAM_CHN_VOICE_UL_PCM STREAM_CHN_PCMC0
#endif
#define STREAM_CHN_EXT          14
#define STREAM_CHN_VOICE_MIX    15
#define STREAM_CHN_EXTV_P       16
#define STREAM_CHN_EXTV_C       17

/* don't change, corresponding with driver definition */
#define PCM_IN      1
#define PCM_OUT     2
#define COMPR_IN    4
#define COMPR_OUT   8
#define NO_IRQ      16
#define MM_VOIP     32

/* don't change, corresponding with SND_AUDIOCODEC_XXX in driver */
#define AUDIO_FMT_PCM                   (0x00000001)
#define AUDIO_FMT_MP3                   (0x00000002)
#define AUDIO_FMT_AMR                   (0x00000003)
#define AUDIO_FMT_AMRWB                 (0x00000004)
#define AUDIO_FMT_AMRWBPLUS             (0x00000005)
#define AUDIO_FMT_AAC                   (0x00000006)
#define AUDIO_FMT_WMA                   (0x00000007)
#define AUDIO_FMT_REAL                  (0x00000008)
#define AUDIO_FMT_VORBIS                (0x00000009)
#define AUDIO_FMT_FLAC                  (0x0000000A)
#define AUDIO_FMT_IEC61937              (0x0000000B)
#define AUDIO_FMT_G723_1                (0x0000000C)
#define AUDIO_FMT_G729                  (0x0000000D)
#define AUDIO_FMT_BESPOKE               (0x0000000E)

/* Bit formats */
enum pcm_format {
    PCM_FORMAT_INVALID = -1,
    PCM_FORMAT_S16_LE = 0,  /* 16-bit signed */
    PCM_FORMAT_S32_LE,      /* 32-bit signed */
    PCM_FORMAT_S8,          /* 8-bit signed */
    PCM_FORMAT_S24_LE,      /* 24-bits in 4-bytes */
    PCM_FORMAT_S24_3LE,     /* 24-bits in 3-bytes */

    PCM_FORMAT_MAX,
};

struct stream_config {
    u32 flag;
    u16 period_cnt;
    u16 period_size;
    u16 start_threshold;    /* pcm only */
    u16 stop_threshold;     /* pcm only */
    u32 sample_rate;
    u32 bit_rate;
    u16 chn;                /* pcm only */
    u16 fmt;
};

struct audtimespec {
    u32 hour;
    u32 sec;
};

int audio_stream_open(u32 stream_chn, struct stream_config *config, int flag);
int audio_stream_close(int stream);
int audio_stream_start(int stream);
int audio_stream_stop(int stream);
int audio_stream_pause(int stream);
int audio_stream_resume(int stream);
int audio_stream_write(int stream, char *buff, u32 size);
int audio_stream_read(int stream, char *buff, u32 size);
int audio_stream_drain(int stream);
u32 audio_stream_get_timestamp(int stream);

#endif /* __AUDIO_STREAM_H__ */

