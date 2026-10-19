/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio resample header file.
 *
 */

#ifndef __AUDIO_RESAMPLE__
#define __AUDIO_RESAMPLE__

void *audio_rsmpl_open(u32 in_sr, u32 out_sr, u32 chn, u32 frm_num);
u32 audio_rsmpl_get_out_num(void *handle);
int audio_rsmpl_run(void *handle, s16 *in, u32 in_num, s16 *out, u32 *out_num);
void audio_rsmpl_close(void *handle);

#endif /* __AUDIO_RESAMPLE__ */
