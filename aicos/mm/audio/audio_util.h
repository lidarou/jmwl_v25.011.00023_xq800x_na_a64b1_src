/******************************************************************************/
/*                                                                            */
/*         Copyright 2021 by aic Micro Corporation. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define common MACRO and API prototype for audio module.
 */

#ifndef __AUDIO_UTIL_H__
#define __AUDIO_UTIL_H__

#include <aic_osa.h>
#include <alog_api.h>
#include <audio/audio_config.h>

#define ALOGV(...)
#define ALOGD       alog_debug
#define ALOGI       alog_info
#define ALOGW       alog_warn
#define ALOGE       alog_error

#define AIDXD       aicx_debug
#define AIDXI       aicx_info
#define AIDXW       aicx_warn
#define AIDXE       aicx_error

#define AASSERT(x)

#define audio_malloc aos_malloc
#define audio_free   aos_free

#ifndef mutex_lock
#define mutex_lock(x)   aos_get_mutex(x, AOS_WAIT_FOREVER)
#endif

#ifndef mutex_unlock
#define mutex_unlock    aos_put_mutex
#endif

#define AENTRY  ALOGD("%s entry, line=%d\n", __func__, __LINE__);
#define ADEBUG  ALOGD("%s line=%d\n", __func__, __LINE__);
#define AEXIT   ALOGD("%s exit, line=%d\n", __func__, __LINE__);

#define audio_tx_queue_create   tx_queue_create
#define audio_tx_queue_receive  tx_queue_receive
#define audio_tx_queue_send     tx_queue_send

#endif /* __AUDIO_UTIL_H__ */

