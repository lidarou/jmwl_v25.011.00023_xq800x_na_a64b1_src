/*****************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the audio nv base driver interface.
 **---------------------------------------------------------------------------*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09f626
#define LOG_MODULE    AMOD_AUDIO

#include "aic_errno.h"
#include "aic_init.h"
#include "audio_nv.h"
#include "audio_nv_drv.h"
#include "audio_util.h"

#include "NV_AUDIO_CONFIG.h"
#include "NV_AUDIO_HW.h"

#ifndef AUDIO_NV_CONFIG_NUM
#define AUDIO_NV_CONFIG_NUM     4
#endif

#ifndef AUDIO_NV_DSP_NUM
#define AUDIO_NV_DSP_NUM        5
#endif

#ifndef AUDIO_NV_VOICE_PROC_NUM
#define AUDIO_NV_VOICE_PROC_NUM 5
#endif

#define AUDIO_NV_ITEM_SIZE_MAX 4096

#ifdef CONFIG_AUDIO_NV_TEST
static const char *nv_name_config[] = {
    "A", "B", "C", "D", "E", "F", "G", "H"
};

static const char *nv_name_dsp[] = {
    "O", "P", "Q", "R", "S", "F", "G", "H"
};

static const char *nv_name_voice_proc[] = {
    "A", "B", "C", "D", "E", "F", "G", "H"
};
#else
static const char *nv_name_config[] = {
    "audio config"
};

static const char *nv_name_dsp[] = {
    "handset",
    "headset",
    "handfree",
    "bt",
    "bt-nrec"
};

static const char *nv_name_voice_proc[] = {
    "handset",
    "headset",
    "handfree",
    "bt",
    "bt-nrec"
};
#endif

static struct audio_nv_config *g_nv_config;
static struct audio_nv_dsp *g_nv_dsp;
static struct audio_nv_voice_proc *g_nv_voice_proc;

u32 audio_nv_get_num(u32 nvid)
{
    switch(nvid) {
    case AUDIO_NVID_CONFIG:
        return AUDIO_NV_CONFIG_NUM;
    case AUDIO_NVID_DSP:
        return AUDIO_NV_DSP_NUM;
    case AUDIO_NVID_VOICE_PROC:
        return AUDIO_NV_VOICE_PROC_NUM;
    }

    return 0;
}

static const char *get_config_nv_name(char *name)
{
    int i, len;

    for(i = 0; i < sizeof(nv_name_config)/sizeof(char *); i++) {
        len = strlen(nv_name_config[i]);
        if (!memcmp(name, nv_name_config[i], len))
            return nv_name_config[i];
    }

    return 0;
}

static const char *get_dsp_nv_name(char *name)
{
    int i;

    for(i = 0; i < sizeof(nv_name_dsp)/sizeof(char *); i++) {
        if (!strncmp(name, nv_name_dsp[i], strlen(nv_name_dsp[i])))
            return nv_name_dsp[i];
    }

    return 0;
}

static const char *get_voice_proc_nv_name(char *name)
{
    int i;

    for(i = 0; i < sizeof(nv_name_voice_proc)/sizeof(char *); i++) {
        if (!strncmp(name, nv_name_voice_proc[i],
                     strlen(nv_name_voice_proc[i])))
            return nv_name_voice_proc[i];
    }

    return 0;
}

static u32 audio_nv_get_size(int nvid)
{
    switch (nvid) {
    case AUDIO_NVID_CONFIG:
        return sizeof(NV_AUDIO_CONFIG_T);
    case AUDIO_NVID_DSP:
        return sizeof(NV_AUDIO_HW_T);
    case AUDIO_NVID_VOICE_PROC:
        /* TODO */
    default:
        return 0;
    }
}

static int audio_nv_init(void)
{
    struct audio_nv_config *p_nv_config;
    struct audio_nv_dsp *p_nv_dsp;
    struct audio_nv_voice_proc *p_nv_voice_proc;
    char *p_nv_buf;
    int i, ret;

    g_nv_config = (struct audio_nv_config *)
                  audio_malloc(sizeof(struct audio_nv_config) *
                               AUDIO_NV_CONFIG_NUM);
    if (!g_nv_config) {
        aicx_error(0x164a5f, "", "nv config no memory!\n");
        return AIC_ENOMEM;
    }
    g_nv_dsp = (struct audio_nv_dsp *)
               audio_malloc(sizeof(struct audio_nv_dsp) * AUDIO_NV_DSP_NUM);
    if (!g_nv_dsp) {
        aicx_error(0x1e5ad5, "", "nv dsp no memory!\n");
        goto err_dsp;
    };

    g_nv_voice_proc = (struct audio_nv_voice_proc *)
                      audio_malloc(sizeof(struct audio_nv_voice_proc) *
                                   AUDIO_NV_VOICE_PROC_NUM);
    if (!g_nv_voice_proc) {
        aicx_error(0x1503a2, "", "nv voice proc no momory!\n");
        goto err_voice_proc;
    }

    p_nv_buf = (char *)audio_malloc(AUDIO_NV_ITEM_SIZE_MAX);
    if (!p_nv_buf) {
         aicx_error(0x1bf1b1, "", "p_nv_buf, no momory!\n");
        goto err_voice_proc;
    }

    for (i = 0; i < AUDIO_NV_CONFIG_NUM; i++) {
        ret = audio_nv_readi(AUDIO_NVID_CONFIG, i, (void *)p_nv_buf,
                             audio_nv_get_size(AUDIO_NVID_CONFIG));
        if (ret) {
            aicx_error(0x060fcd, "", "nv config read err!\n");
            goto err;
        }
        p_nv_config = &g_nv_config[i];
        p_nv_config->name =
            (char *)get_config_nv_name(&(((NV_AUDIO_CONFIG_T *)p_nv_buf)->name));
        p_nv_config->index = i;
    }

    for (i = 0; i < AUDIO_NV_DSP_NUM; i++) {
        ret = audio_nv_readi(AUDIO_NVID_DSP, i, (void *)p_nv_buf,
                             audio_nv_get_size(AUDIO_NVID_DSP));
        if (ret) {
            aicx_error(0x17d788, "", "nv dsp read err!\n");
            goto err;
        }
        p_nv_dsp = &g_nv_dsp[i];
        p_nv_dsp->name =
            (char *)get_dsp_nv_name(&(((NV_AUDIO_HW_T *)p_nv_buf)->name));
        p_nv_dsp->index = i;
    }

    for (i = 0; i < AUDIO_NV_VOICE_PROC_NUM; i++) {
        ret = audio_nv_readi(AUDIO_NVID_VOICE_PROC, i, (void *)p_nv_buf,
                             audio_nv_get_size(AUDIO_NVID_DSP));
        if (ret) {
            aicx_error(0x1ae74b, "", "nv voice proc read err!\n");
            goto err;
        }
        p_nv_voice_proc = &g_nv_voice_proc[i];
        p_nv_voice_proc->name =
            (char *)get_voice_proc_nv_name(&(((NV_AUDIO_HW_T *)p_nv_buf)->name));
        p_nv_voice_proc->index = i;
    }

    return 0;
err:
    audio_free(p_nv_buf);
    audio_free(g_nv_voice_proc);
    g_nv_voice_proc = 0;
err_voice_proc:
    audio_free(g_nv_dsp);

    g_nv_dsp = 0;
err_dsp:
    audio_free(g_nv_config);
    g_nv_config = 0;

    return AIC_ENOMEM;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(audio_nv_init);

static int get_index_offset_by_name(u32 nvid, char *name)
{
    struct audio_nv_base *nv;
    int i, num, offset_unit;

    if (!name) {
        aicx_error(0x019d86, "", "get nv index, name is null\n");
        return AIC_EINVAL;
    }
    switch (nvid) {
    case AUDIO_NVID_CONFIG:
        nv = (struct audio_nv_base *)g_nv_config;
        num = AUDIO_NV_CONFIG_NUM;
        offset_unit = sizeof(struct audio_nv_config);
        break;
    case AUDIO_NVID_DSP:
        nv = (struct audio_nv_base *)g_nv_dsp;
        num = AUDIO_NV_DSP_NUM;
        offset_unit = sizeof(struct audio_nv_dsp);
        break;
    case AUDIO_NVID_VOICE_PROC:
        nv = (struct audio_nv_base *)g_nv_voice_proc;
        num = AUDIO_NV_VOICE_PROC_NUM;
        offset_unit = sizeof(struct audio_nv_voice_proc);
    default:
        return AIC_EINVAL;
    }

    for (i = 0; i < num; i++) {
        if (!strcmp(name, nv->name)) {
            return nv->index;
        }
        nv = (struct audio_nv_base *)((char *)nv + offset_unit);
    }

    return AIC_ENOENT;
}

int audio_nv_read(u32 nvid, char *name, void *buf)
{
    int index;

    if (!buf) {
        aicx_error(0x0436c8, "", "audio nv read, buf null\n");
        return AIC_EINVAL;
    }
    index = get_index_offset_by_name(nvid, name);
    if (index < 0) {
        aicx_error(0x174fc5, "", "audio nv read, get index error\n");
        return 0;
    }

    return audio_nv_readi(nvid, index, buf, audio_nv_get_size(nvid));
}

int audio_nv_write(u32 nvid, char *name, void *buf)
{
    int index;

    if (!buf) {
        aicx_error(0x10560f, "", "audio nv write, buf null\n");
        return AIC_EINVAL;
    }
    index = get_index_offset_by_name(nvid, name);
    if (index < 0) {
        aicx_error(0x16fa56, "", "audio nv write, get index error\n");
        return 0;
    }

    return audio_nv_writei(nvid, index, buf, audio_nv_get_size(nvid));
}

