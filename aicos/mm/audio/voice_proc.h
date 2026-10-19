/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is voice manager header file, message about voice between ap
 *  and cp are defined, APIs about sending and process message are declaired.
 *
 *****************************************************************************/

#ifndef __VOICE_PROC_H__
#define __VOICE_PROC_H__

#include "codec_mng.h"

typedef int (*vp_codec_get)(void *hvp, u8 *buf, u32 cnt, u32 *real_cnt,
                            u32 *fmt, u32 *mode);
typedef int (*vp_codec_send)(void *hvp, u8 *buf, u32 cnt);

void *voice_proc_open(void *priv, struct codec_info *codec_info,
                      vp_codec_get get_func, vp_codec_send send_func);
int voice_proc_get(void *hvp, void *buff, u32 *size);
int voice_proc_send(void *hvp, void *buff, u32 size);
int voice_proc_close(void *hvp);
int voice_proc_md_set(void *pmd);
int voice_proc_record_start(u32 fmt, u32 srate, u32 brate, u32 chn,
                            void *cb, void *priv);
int voice_proc_record_stop(void);
int voice_proc_record_pause(void);
int voice_proc_record_resume(void);

void voice_proc_mix_start(void);
void voice_proc_mix_stop(void);

#endif


