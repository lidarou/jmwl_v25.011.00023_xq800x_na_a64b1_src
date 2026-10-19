/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is codec manage file, all codecs should registe to codec
 *  manager at boot time.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1117d7
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "codec_mng.h"

struct codec_mng {
    struct list_head list;
};

struct codec_mng g_codec_mng;

struct codec_engine *codec_get_by_type(enum codec_type type, int dec_enc)
{
    struct codec_engine *engine;

    list_for_each_entry(engine, &g_codec_mng.list, list, struct codec_engine) {
        if (engine->type == type && engine->dir == dec_enc)
            return engine;
    }

    return NULL;
}

void codec_register(struct codec_engine *engine)
{
    aicx_info(0x1d924d, "d", "codec type=%d registed\n", engine->type);
    list_add(&engine->list, &g_codec_mng.list);
}

int codec_init(void)
{
    list_head_init(&g_codec_mng.list);
    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(codec_init);

