/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the wrapper of audio arec.
 *
 */

#ifndef __AUDIO_NREC_H__
#define __AUDIO_NREC_H__

#include "voice_proc_api.h"

void *audio_ec_create(u32 srate, u32 chn, u32 wid);
int audio_ec_proc(void *haec, char *in_buf, char *ref_buf, u32 in_size,
                  char *out_buf, u32 *out_size);
void audio_ec_delete(void *haec);
int audio_ec_control(void *haec, u32 ctl_cmd, void *param);

void *audio_nr_create(u32 srate, u32 chn, u32 wid);
int audio_nr_proc(void *hanr, char *in_buf, u32 in_size,
                  char *out_buf, u32 *out_size);
void audio_nr_delete(void *hanr);
int audio_nr_control(void *haec, u32 ctl_cmd, void *param);

#endif /* __AUDIO_NREC_H__ */

