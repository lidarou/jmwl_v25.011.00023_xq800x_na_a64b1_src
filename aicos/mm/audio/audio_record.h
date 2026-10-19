/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio record module header file, it defined related data
 *  struct and API used for audio service.
 *
 *****************************************************************************/

#ifndef __AUDIO_RECORD_H__
#define __AUDIO_RECORD_H__

AHANDLE audio_record_open(ACLIENT aclient, enum astream_type type,
                        enum aplay_type aplay_type,
                        u32 flag, enum astream_format format, u32 sample_rate,
                        u32 bit_rate, enum astream_channel chn,
                        enum astream_direct dir);

int audio_record_set_param(AHANDLE handle, void *param);
int audio_record_start(AHANDLE handle, enum aplay_type aplay_type,
                       void *aplay_data);
int audio_record_stop(AHANDLE handle);
int audio_record_close(AHANDLE handle);
int audio_record_pause(AHANDLE handle);
int audio_record_resume(AHANDLE handle);
u32 audio_record_get_time(AHANDLE handle);

int audio_record_user_cb(AHANDLE handle, enum astream_cb_event event);
struct asink_buf *audio_record_get_buf(AHANDLE handle);
#ifdef CONFIG_VOICE_STREAM
int audio_get_voice_ul_dev(void);
#endif
#ifdef CONFIG_REC_EFFECT
int audio_record_param_update(void *param);
#endif

#endif /* __AUDIO_RECORD_H__ */

