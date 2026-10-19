/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio policy header file, struct definition and APIs
 *  about audio policy are declared here.
 *
 *****************************************************************************/

#ifndef __AUDIO_POLICY_H__
#define __AUDIO_POLICY_H__

#include "aic_type.h"
#include "audio_system.h"

#define APOLICY_DEFAULT_PCM_FORMAT  PCM_FORMAT_S16_LE
#define APOLICY_HWCODEC_DL_PERIOD_SIZE   2048
#define APOLICY_HWCODEC_UL_PERIOD_SIZE   960

struct apolicy_stream_config {
    u32 period_size;
    u32 period_cnt;
    u32 start_threshold;
    u32 stop_threshold;
};

struct astream_property {
    u32 type        : 5;
    u32 is_dynamic  : 1;
    u32 has_stream  : 1;
    u32 priority    : 5;
};

const struct apolicy_stream_config *apolicy_get_config(
                enum astream_type type,
                enum astream_format format,
                enum astream_direct dir,
                u32 flag);
const struct apolicy_stream_config *apolicy_get_compr_config(
                enum astream_type type,
                enum astream_format format,
                enum astream_direct dir,
                u32 flag);

struct astream_property *apolicy_get_stream_property(enum astream_type type);
int apolicy_get_stream_chn(enum astream_type type,
                           enum astream_format format, int flag);
void apolicy_add_stream_handle(u32 handle);
void apolicy_delete_stream_handle(u32 handle);
void apolicy_device_update(void);
void apolicy_param_update(void);
void apolicy_get_current_device(u32 *out_device, u32 *in_device);
void apolicy_set_force_out_device(u32 out_device);
void apolicy_set_force_in_device(u32 in_device);
int apolicy_conflict_check(enum astream_type type,
                           enum astream_format format, int flag);

#endif /* __AUDIO_POLICY_H__ */

