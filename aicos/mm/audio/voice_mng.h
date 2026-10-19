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

#ifndef __VOICE_MNG_H__
#define __VOICE_MNG_H__

#include <aic_type.h>
#include <aipc_api.h>

/* 1 - 100: cp -> ap */
#define VOICE_MSG_VOICE_SRATE   1
#define VOICE_MSG_OPEN          2
#define VOICE_MSG_CLOSE         3
#define VOICE_MSG_TEST          4

/* >= 100: ap -> cp */
#define VOICE_MSG_ROUTE_READY   100

int audio_send_cp_msg(u16 cmd, u32 param);
int audio_proc_cp_msg(AMSG_T *msg, void *p);

int voice_route_notify(bool is_on);
int voice_stream_start(void);
int voice_stream_stop(void);

#endif

