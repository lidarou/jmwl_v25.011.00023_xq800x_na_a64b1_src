/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** Export the audio nv operation interface.
 **---------------------------------------------------------------------------*/
#include "aic_type.h"

#ifndef __AUDIO_NV_DRV_H__
#define __AUDIO_NV_DRV_H__

u32 audio_nv_get_offset(u16 item_id);
char *audio_nv_get_base(void);
int audio_nv_readi(u32 nvid, u32 index, void *buf, u32 nvid_size);
int audio_nv_writei(u32 nvid, u32 index, void *buf, u32 nvid_size);

#endif /* __AUDIO_NV_DRV_H__ */

