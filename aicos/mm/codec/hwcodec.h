/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is codec header file, codec related struct and
 *  exported APIs are declared here.
 *
 *****************************************************************************/

#ifndef __HWCODEC_H__
#define __HWCODEC_H__

#include "codec_mng.h"

struct hwcodec_init_param {
    u32 brate;
    u32 ul_fmt;
    u32 dl_fmt;
    u32 srate;
    u32 chn;
    u32 period_cnt;
    u32 ul_stream_id;
    u32 dl_stream_id;
    u32 client_obj;
    codec_get fget;
    void *residue_data;
    u32 residue_size;
};

void* hwcodec_init(struct hwcodec_init_param *param);
int hwcodec_deinit(void *obj);
int hwcodec_start(void *obj);
int hwcodec_stop(void *obj);
int hwcodec_get_data(void* obj, short *pbuf, u32 count, u32 *real_count);
int hwcodec_seek(void *obj , u32 percent);
#endif
