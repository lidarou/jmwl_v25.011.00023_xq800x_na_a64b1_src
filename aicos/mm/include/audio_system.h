/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio system header file, in which defined audio system
 *  MACROs, structs and APIs for both module internal and external.
 *
 *****************************************************************************/

#ifndef __AUDIO_SYSTEM_H__
#define __AUDIO_SYSTEM_H__

#define ACLIENT unsigned int

#define AUDIO_WAIT_FOREVER  AOS_WAIT_FOREVER
#define AUDIO_CMD_WAIT_TIME 3000

#ifdef CONFIG_VOLUME_NO_CODEC
#define AUDIO_MVOL_LEVEL_MAX 2560
#else
#define AUDIO_MVOL_LEVEL_MAX 15
#endif

enum astream_type {
    ASTREAM_TYPE_DEFAULT = 0,
    ASTREAM_TYPE_MUSIC = ASTREAM_TYPE_DEFAULT,
    ASTREAM_TYPE_RING,
    ASTREAM_TYPE_SYSTEM,
    ASTREAM_TYPE_TONE,
    ASTREAM_TYPE_DTMF, /* special tone */
    ASTREAM_TYPE_VOICE,
    ASTREAM_TYPE_VOIP,
    ASTREAM_TYPE_FM,
    ASTREAM_TYPE_TTS,
    ASTREAM_TYPE_ALARM,
    ASTREAM_TYPE_NOTIFICATION,
    ASTREAM_TYPE_OUTPUT_MAX,
    ASTREAM_TYPE_INPUT_BASE = ASTREAM_TYPE_OUTPUT_MAX,
    ASTREAM_TYPE_RECORD = ASTREAM_TYPE_INPUT_BASE,
    ASTREAM_TYPE_VOICE_UL,
    ASTREAM_TYPE_VOIP_UL,
    ASTREAM_TYPE_MAX,

    ASTREAM_TYPE_INPUT_MAX = ASTREAM_TYPE_MAX - ASTREAM_TYPE_INPUT_BASE,
};

enum aplay_type {
    APLAY_TYPE_NONE = 0,
    APLAY_TYPE_BUF,
    APLAY_TYPE_FILE,
    APLAY_TYPE_ID,     /* DSP read data directly, or chanege to client read */
    APLAY_TYPE_TONE,
    APLAY_TYPE_TTS,    /* similar with APLAY_SUB_BUF */
    APLAY_TYPE_STREAM, /* such like online play */
    APLAY_TYPE_MAX
};

enum astream_format {
    ASTREAM_FORMAT_INVALID = -1,
    ASTREAM_FORMAT_DEFAULT = 0,
    ASTREAM_FORMAT_PCM,
    ASTREAM_FORMAT_WAV,
    ASTREAM_FORMAT_OFFLD_BEGIN,
    ASTREAM_FORMAT_MP3 = ASTREAM_FORMAT_OFFLD_BEGIN,
    ASTREAM_FORMAT_AAC,
    ASTREAM_FORMAT_AMR_NB,
    ASTREAM_FORMAT_OFFLD_END,
    ASTREAM_FORMAT_AMR_WB = ASTREAM_FORMAT_OFFLD_END,
    ASTREAM_FORMAT_SBC,
    ASTREAM_FORMAT_MIDI,
    ASTREAM_FORMAT_VOICE_DTMF,
    ASTREAM_FORMAT_MAX
};

enum astream_source {
    ASTREAM_SOURCE_DEFAULT = 0,
    ASTREAM_SOURCE_MIC,
    ASTREAM_SOURCE_VOICE_UPLINK,
    ASTREAM_SOURCE_VOICE_DONWLINK,
    ASTREAM_SOURCE_FM,
    ASTREAM_SOURCE_BT_SCO,
    ASTREAM_SOURCE_MAX
};

enum astream_channel {
    ASTREAM_CHN_NONE    = 0,
    ASTREAM_CHN_MONO    = 1,
    ASTREAM_CHN_STEREO  = 2,
    ASTREAM_CHN_MAX
};

enum astream_direct {
    ASTREAM_DIR_PLAY    = 0,
    ASTREAM_DIR_RECORD  = 1,
};

enum audio_mode {
    AUDIO_MODE_NORMAL   = 0,
    AUDIO_MODE_RING,
    AUDIO_MODE_CALL,
    AUDIO_MODE_COMMUNICATE,
    AUDIO_MODE_MAX
};

enum audio_device {
    AUDIO_DEVICE_NONE               = 0x0u,
    AUDIO_DEVICE_BIT_IN             = 0x80000000u,
    AUDIO_DEVICE_BIT_DEFAULT        = 0x40000000u,
    AUDIO_DEVICE_OUT_EARPIECE       = 0x1u,
    AUDIO_DEVICE_OUT_SPEAKER        = 0x2u,
    AUDIO_DEVICE_OUT_HEADSET        = 0x4u,
    AUDIO_DEVICE_OUT_HEADPHONE      = 0x8u,
    AUDIO_DEVICE_OUT_BT_SCO         = 0x10u,
    AUDIO_DEVICE_OUT_BT_A2DP        = 0x20u,
    AUDIO_DEVICE_OUT_FM             = 0x40u,
    AUDIO_DEVICE_IN_MIC             = 0x80000001u,
    AUDIO_DEVICE_IN_HEADPHONE       = 0x80000002u,
    AUDIO_DEVICE_IN_BACK_MIC        = 0x80000004u,
    AUDIO_DEVICE_IN_BT_SCO          = 0x80000008u,
    AUDIO_DEVICE_IN_LINE            = 0x80000020u,
};

enum audio_output_flag {
    AUDIO_OUTPUT_FLAG_NONE          = 0x0,
    AUDIO_OUTPUT_FLAG_DIRECT        = 0x1,
    AUDIO_OUTPUT_FLAG_FAST          = 0x2,
    AUDIO_OUTPUT_FLAG_DEEP_BUFF     = 0x4,
    AUDIO_OUTPUT_FLAG_COMPRESS      = 0x8,
    AUDIO_OUTPUT_FLAG_INCALL_ULINK  = 0x10,
    AUDIO_OUTPUT_FLAG_INCALL_DLINK  = 0x20,
    AUDIO_OUTPUT_FLAG_SYNC          = 0x40,
    AUDIO_OUTPUT_FLAG_EXTERNAL      = 0x80,
};

enum audio_input_flag {
    AUDIO_INPUT_FLAG_NONE           = 0x0,
    AUDIO_INPUT_FLAG_DIRECT         = 0x1,
    AUDIO_INPUT_FLAG_FAST           = 0x2,
    AUDIO_INPUT_FLAG_COMPRESS       = 0x4,
    AUDIO_INPUT_FLAG_VOREC          = 0x8,
    AUDIO_INPUT_FLAG_EXTERNAL       = 0x10,
};

enum astream_cb_event {
    ASTREAM_CB_EVENT_NONE           = 0,
    ASTREAM_CB_EVENT_FINISH,
    ASTREAM_CB_EVENT_BREAK,
};

enum tone_type {
    /* DTMF tones  ITU-T Recommendation Q.23 */
    TONE_DTMF_0 = 0,
    TONE_DTMF_1,
    TONE_DTMF_2,
    TONE_DTMF_3,
    TONE_DTMF_4,
    TONE_DTMF_5,
    TONE_DTMF_6,
    TONE_DTMF_7,
    TONE_DTMF_8,
    TONE_DTMF_9,
    TONE_DTMF_S,
    TONE_DTMF_P,
    TONE_DTMF_A,
    TONE_DTMF_B,
    TONE_DTMF_C,
    TONE_DTMF_D,
    /* Call supervisory tones:  3GPP TS 22.001 (CEPT) */
    TONE_SUP_DIAL,
    FIRST_SUP_TONE = TONE_SUP_DIAL,
    TONE_SUP_BUSY,
    TONE_SUP_CONGESTION,
    TONE_SUP_RADIO_ACK,
    TONE_SUP_RADIO_NOTAVAIL,
    TONE_SUP_ERROR,
    TONE_SUP_CALL_WAITING,
    TONE_SUP_RINGTONE,
    LAST_SUP_TONE = TONE_SUP_RINGTONE,
    /* Proprietary tones:  3GPP TS 31.111 */
    TONE_PROP_BEEP,
    TONE_PROP_ACK,
    TONE_PROP_NACK,
    TONE_PROP_PROMPT,
    TONE_PROP_BEEP2,
    /* Additional call supervisory tones: specified by IS-95 only */
    TONE_SUP_INTERCEPT,
    TONE_SUP_INTERCEPT_ABBREV,
    TONE_SUP_CONGESTION_ABBREV,
    TONE_SUP_CONFIRM,
    TONE_SUP_PIP,

    /* CDMA Tones */
    TONE_CDMA_DIAL_TONE_LITE,
    TONE_CDMA_NETWORK_USA_RINGBACK,
    TONE_CDMA_INTERCEPT,
    TONE_CDMA_ABBR_INTERCEPT,
    TONE_CDMA_REORDER,
    TONE_CDMA_ABBR_REORDER,
    TONE_CDMA_NETWORK_BUSY,
    TONE_CDMA_CONFIRM,
    TONE_CDMA_ANSWER,
    TONE_CDMA_NETWORK_CALLWAITING,
    TONE_CDMA_PIP,

    /* ISDN */
    TONE_CDMA_CALL_SIGNAL_ISDN_NORMAL,
    TONE_CDMA_CALL_SIGNAL_ISDN_INTERGROUP,
    TONE_CDMA_CALL_SIGNAL_ISDN_SP_PRI,
    TONE_CDMA_CALL_SIGNAL_ISDN_PAT3,
    TONE_CDMA_CALL_SIGNAL_ISDN_PING_RING,
    TONE_CDMA_CALL_SIGNAL_ISDN_PAT5,
    TONE_CDMA_CALL_SIGNAL_ISDN_PAT6,
    TONE_CDMA_CALL_SIGNAL_ISDN_PAT7,
    /* ISDN end */

    /* IS54 */
    TONE_CDMA_HIGH_L,
    TONE_CDMA_MED_L,
    TONE_CDMA_LOW_L,
    TONE_CDMA_HIGH_SS,
    TONE_CDMA_MED_SS,
    TONE_CDMA_LOW_SS,
    TONE_CDMA_HIGH_SSL,
    TONE_CDMA_MED_SSL,
    TONE_CDMA_LOW_SSL,
    TONE_CDMA_HIGH_SS_2,
    TONE_CDMA_MED_SS_2,
    TONE_CDMA_LOW_SS_2,
    TONE_CDMA_HIGH_SLS,
    TONE_CDMA_MED_SLS,
    TONE_CDMA_LOW_SLS,
    TONE_CDMA_HIGH_S_X4,
    TONE_CDMA_MED_S_X4,
    TONE_CDMA_LOW_S_X4,
    TONE_CDMA_HIGH_PBX_L,
    TONE_CDMA_MED_PBX_L,
    TONE_CDMA_LOW_PBX_L,
    TONE_CDMA_HIGH_PBX_SS,
    TONE_CDMA_MED_PBX_SS,
    TONE_CDMA_LOW_PBX_SS,
    TONE_CDMA_HIGH_PBX_SSL,
    TONE_CDMA_MED_PBX_SSL,
    TONE_CDMA_LOW_PBX_SSL,
    TONE_CDMA_HIGH_PBX_SLS,
    TONE_CDMA_MED_PBX_SLS,
    TONE_CDMA_LOW_PBX_SLS,
    TONE_CDMA_HIGH_PBX_S_X4,
    TONE_CDMA_MED_PBX_S_X4,
    TONE_CDMA_LOW_PBX_S_X4,
    /* IS54 end */

    /* proprietary */
    TONE_CDMA_ALERT_NETWORK_LITE,
    TONE_CDMA_ALERT_AUTOREDIAL_LITE,
    TONE_CDMA_ONE_MIN_BEEP,
    TONE_CDMA_KEYPAD_VOLUME_KEY_LITE,
    TONE_CDMA_PRESSHOLDKEY_LITE,
    TONE_CDMA_ALERT_INCALL_LITE,
    TONE_CDMA_EMERGENCY_RINGBACK,
    TONE_CDMA_ALERT_CALL_GUARD,
    TONE_CDMA_SOFT_ERROR_LITE,
    TONE_CDMA_CALLDROP_LITE,
    /* proprietary end */

    TONE_CDMA_NETWORK_BUSY_ONE_SHOT,
    TONE_CDMA_ABBR_ALERT,
    TONE_CDMA_SIGNAL_OFF,
    /* CDMA end */

    NUM_TONES,
    NUM_SUP_TONES = LAST_SUP_TONE - FIRST_SUP_TONE + 1
};

enum tone_region {
    TONE_REGION_ANSI,
    TONE_REGION_JAPAN,
    TONE_REGION_GB,
    TONE_REGION_AUSTRALIA,
    TONE_REGION_SINGAPORE,
    TONE_REGION_HONGKONG,
    TONE_REGION_IRELAND,
    TONE_REGION_CEPT,
    TONE_REGION_MAX
};

typedef int (*astream_user_cb)(u32 data, enum astream_cb_event event);

ACLIENT audio_open(enum astream_type type, enum aplay_type aplay_type, u32 flag,
                   u32 sample_rate, u32 bit_rate, enum astream_format format,
                   enum astream_channel chn,
                   enum astream_direct dir, enum astream_source source);
ACLIENT audio_open_output(enum astream_type type, enum aplay_type aplay_type,
                          enum astream_format format, u32 flag);
ACLIENT audio_open_input(enum aplay_type aplay_type, enum astream_format format,
                         u32 sample_rate, enum astream_channel chn,
                         enum astream_source source, u32 flag);
ACLIENT audio_open_output_pcm(enum astream_type type, enum aplay_type aplay_type,
                              u32 flag, u32 sample_rate, enum astream_channel chn,
                              enum astream_direct dir);
int audio_play(ACLIENT handle);
int audio_stop(ACLIENT handle);
int audio_seek(ACLIENT handle, u32 percent);
int audio_close(ACLIENT handle);
int audio_play_buf(ACLIENT handle, char *buf, u32 buf_len,
                   void *user_cb, u32 cb_data);
int audio_play_file(ACLIENT handle, char *file_name,
                    void *user_cb, u32 cb_data);
int audio_play_tts(ACLIENT handle, char *buf, u32 buf_len,
                   void *user_cb, u32 cb_data);
int audio_play_stream(ACLIENT handle, void *user_cb, u32 cb_data);
int audio_play_tone_ex(enum tone_type tone_type, u32 duation, u32 flag);
int audio_play_tone(enum tone_type tone_type, u32 duation);
int audio_stop_tone(void);
int audio_start_voice(BOOL is_dl_on, BOOL is_ul_on);
int audio_stop_voice(void);
int audio_start_loopback(void);
int audio_stop_loopback(void);
int audio_set_param(ACLIENT handle);
int audio_pause(ACLIENT handle);
int audio_resume(ACLIENT handle);
int audio_write(ACLIENT handle, char *buf, u32 len, u32 wait_time);
int audio_read(ACLIENT handle, char *buf, u32 len, u32 wait_time);
int audio_drain(ACLIENT handle);
int audio_force_out_device(u32 device);
int audio_force_in_device(u32 device);
int audio_get_device(u32 *in_device, u32 *out_device);
int audio_get_force_device(u32 *in_device, u32 *out_device);
int audio_set_volume(ACLIENT handle, u32 vol);
u32 audio_get_volume(ACLIENT handle);
int audio_set_master_volume(u32 vol);
u32 audio_get_master_volume(void);
int audio_mute(ACLIENT handle);
int audio_unmute(ACLIENT handle);
int audio_mute_all(void);
int audio_unmute_all(void);
int audio_mute_ul(u32 is_mute);
int audio_set_voice_info(ACLIENT handle);
/* return: msecond */
u32 audio_get_timestamp(ACLIENT handle);
void audio_set_voice_stream(BOOL is_on);
int audio_set_tone_region(u32 region_id);

#endif /* __AUDIO_SYSTEM_H__ */

