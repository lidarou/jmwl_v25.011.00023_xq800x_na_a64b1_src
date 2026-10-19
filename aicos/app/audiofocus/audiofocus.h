/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare the API of audiofocus service.
 */

#ifndef __AUDIOFOCUS_H__
#define __AUDIOFOCUS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "aic_type.h"
// TODO
#include "aic_osa_ex.h"
#include "app_id.h"

enum af_event {
    AF_EVENT_FOCUS_GAIN,
    AF_EVENT_FOCUS_LOSS,
    AF_EVENT_FOCUS_LOSS_TRANSIENT
};

enum af_audio_context {
    AF_CTX_IDLE,
    /* Only define call and others for testing*/
    AF_CTX_CALL,
    AF_CTX_OTHERS,
    /*
        AF_CTX_RING,
        AF_CTX_MUSIC,
        AF_CTX_ALARM,
        AF_CTX_NOTIFICATION,
        AF_CTX_KEY,
    */
    AF_CTX_MAX
};

/* Never call audio focus api in callback functions*/
typedef void (*af_notify_t)(enum af_event, void *appdata);

u32 audio_focus_request(enum app_id id, enum af_audio_context ctx,
                        af_notify_t notify, void *appdata);
u32 audio_focus_release(enum app_id id);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __AUDIOFOCUS_H__ */