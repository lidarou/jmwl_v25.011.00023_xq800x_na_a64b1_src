/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is audio HAL, it provide some interfaces to control audio
 *    route and set parameter.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x188d42
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "audio_control.h"

#ifdef CONFIG_SOUND_SUPPORT
#include "aic_init.h"
#include "snd_ctl.h"

#define CTL_NAME_LEN_MAX 44

struct audio_ctl {
    struct audio_ctl_platform *ctl_platform;
    struct snd_ctl_elem_info info;
    char **ename;
};

struct audio_ctl_platform {
    struct audio_ctl *ctl;
    unsigned int count;
};


static struct audio_ctl_platform *g_audio_ctl;

void audio_ctl_close(void)
{
    if (g_audio_ctl) {
        snd_free(g_audio_ctl->ctl);
        snd_free(g_audio_ctl);
        g_audio_ctl = 0;
    }
}

int audio_clt_init(void)
{
    struct audio_ctl_platform *ctl_platform = NULL;
    struct audio_ctl *ctl;
    int i, ret;

    ctl_platform = snd_malloc(sizeof(*ctl_platform));
    if (!ctl_platform)
        return AIC_ENOMEM;

    ctl_platform->count = snd_ctl_get_cnt();
    if (!ctl_platform->count) {
        aicx_info(0x0656cb, "", "actl count is 0!\n");
        return 0;
    }
    ctl_platform->ctl = snd_malloc(sizeof(struct audio_ctl) * ctl_platform->count);
    if (!ctl_platform->ctl) {
        snd_free(ctl_platform);
        return AIC_ENOMEM;
    }

    memset(ctl_platform->ctl, 0,
           sizeof(struct audio_ctl) * ctl_platform->count);
    for (i = 1; i <= ctl_platform->count; i++) {
        ctl = &ctl_platform->ctl[i - 1];
        ret = snd_ctl_get_info_numid(i, &ctl->info);
        if (ret < 0) {
            aicx_error(0x0c99a1, "", "actl init failed!\n");
            goto failed;
        }
        ctl->ctl_platform = ctl_platform;
    }
    g_audio_ctl = ctl_platform;

    return 0;
failed:
    snd_free(ctl_platform->ctl);
    snd_free(ctl_platform);

    return ret;
}
AIC_NORMAL_COMP_L1_INIT_EXPORT(audio_clt_init);

unsigned int audio_ctl_get_num(void)
{
    if (g_audio_ctl)
        return g_audio_ctl->count;
    return 0;
}

enum audio_ctl_type audio_ctl_get_type(struct audio_ctl *ctl)
{
    if (!ctl)
        return AUDIO_CTL_TYPE_UNKNOWN;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:    return AUDIO_CTL_TYPE_BOOL;
    case SNDRV_CTL_ELEM_TYPE_INTEGER:    return AUDIO_CTL_TYPE_INT;
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED: return AUDIO_CTL_TYPE_ENUM;
    case SNDRV_CTL_ELEM_TYPE_BYTES:      return AUDIO_CTL_TYPE_BYTE;
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:  return AUDIO_CTL_TYPE_INT64;
    default:                             return AUDIO_CTL_TYPE_UNKNOWN;
    };
}

const char *audio_ctl_get_type_string(struct audio_ctl *ctl)
{
    if (!ctl)
        return "";

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:    return "BOOL";
    case SNDRV_CTL_ELEM_TYPE_INTEGER:    return "INT";
    case SNDRV_CTL_ELEM_TYPE_ENUMERATED: return "ENUM";
    case SNDRV_CTL_ELEM_TYPE_BYTES:      return "BYTE";
    case SNDRV_CTL_ELEM_TYPE_IEC958:     return "IEC958";
    case SNDRV_CTL_ELEM_TYPE_INTEGER64:  return "INT64";
    default:                             return "Unknown";
    };
}

#if 0
static int percent_to_int(struct snd_ctl_elem_info *ei, int percent)
{
    int range;

    if (percent > 100)
        percent = 100;
    else if (percent < 0)
        percent = 0;

    range = (ei->value.integer.max - ei->value.integer.min);

    return ei->value.integer.min + (range * percent) / 100;
}

static int int_to_percent(struct snd_ctl_elem_info *ei, int value)
{
    int range = (ei->value.integer.max - ei->value.integer.min);

    if (range == 0)
        return 0;

    return ((value - ei->value.integer.min) / range) * 100;
}
#endif

/**
 * Note:
 *      This function is just get enum type control's info,
 *      other type is no necessary.
 */
static bool audio_ctl_get_elem_info(struct audio_ctl* ctl)
{
    struct snd_ctl_elem_info *tmp;
    char **enames;
    unsigned int i;

    if (ctl->info.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED || ctl->ename)
        return TRUE;

    tmp = &ctl->info;
    enames = snd_zalloc(ctl->info.value.enumerated.items * sizeof(char*));
    if (!enames)
        return FALSE;

    for (i = 0; i < ctl->info.value.enumerated.items; i++) {
        //tmp->id.numid = ctl->info.id.numid;
        tmp->value.enumerated.item = i;
        if (snd_ctl_get_info_numid(tmp->id.numid, tmp) < 0)
            goto fail;
        enames[i] = tmp->value.enumerated.name;
        if (!enames[i])
            goto fail;
    }
    ctl->ename = enames;
    return TRUE;

fail:
    snd_free(enames);
    return FALSE;
}

struct audio_ctl *audio_clt_get(unsigned int id)
{
    struct audio_ctl *ctl;

    if (!g_audio_ctl || (id >= g_audio_ctl->count))
        return NULL;

    ctl = g_audio_ctl->ctl + id;

    if (!audio_ctl_get_elem_info(ctl))
        return NULL;

    return ctl;
}

const char *audio_ctl_get_name(struct audio_ctl *ctl)
{
    if (!ctl)
        return NULL;

    return (const char *)ctl->info.id.name;
}

struct audio_ctl *audio_ctl_get_by_name(const char *name)
{
    unsigned int n;

    if (!g_audio_ctl)
        return NULL;
    for (n = 0; n < g_audio_ctl->count; n++)
        if (!strcmp(name, (char*) g_audio_ctl->ctl[n].info.id.name))
            return audio_clt_get(n);

    return NULL;
}

int audio_ctl_get_value(struct audio_ctl *ctl, unsigned int id)
{
    struct snd_ctl_elem_value ev;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return AIC_EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;
    ret = snd_ctl_read(&ev);
    if (ret < 0)
        return ret;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
        return !!ev.value.integer.value[id];

    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        return ev.value.integer.value[id];

    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        return ev.value.enumerated.item[id];

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        return ev.value.bytes.data[id];

    default:
        return AIC_EINVAL;
    }
}

unsigned int audio_ctl_get_num_values(struct audio_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.count;
}

const char *audio_ctl_get_enum_string(struct audio_ctl *ctl,
                                      unsigned int enum_id)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED) ||
        (enum_id >= ctl->info.value.enumerated.items))
        return NULL;

    /* TODO */
    //return 0;
    return (const char *)ctl->ename[enum_id];
}

int audio_ctl_get_array(struct audio_ctl *ctl, void *array, size_t count)
{
    struct snd_ctl_elem_value ev;
    int ret = 0;
    size_t size;
    void *source;
    size_t total_count;

    if ((!ctl) || !count || !array)
        return AIC_EINVAL;

    total_count = ctl->info.count;

    if (count > total_count)
        return AIC_EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        ret = snd_ctl_read(&ev);
        if (ret < 0)
            return ret;
        size = sizeof(ev.value.integer.value[0]);
        source = ev.value.integer.value;
        break;

    case SNDRV_CTL_ELEM_TYPE_BYTES:
            ret = snd_ctl_read(&ev);
            if (ret < 0)
                return ret;
            size = sizeof(ev.value.bytes.data[0]);
            source = ev.value.bytes.data;
            break;

    default:
        return AIC_EINVAL;
    }

    memcpy(array, source, size * count);

    return 0;
}

int audio_ctl_set_value(struct audio_ctl *ctl, unsigned int id, int value)
{
    struct snd_ctl_elem_value ev;
    int ret;

    if (!ctl || (id >= ctl->info.count))
        return AIC_EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;
    ret = snd_ctl_read(&ev);
    if (ret < 0)
        return ret;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
        ev.value.integer.value[id] = !!value;
        break;

    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        ev.value.integer.value[id] = value;
        break;

    case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
        ev.value.enumerated.item[id] = value;
        break;

    case SNDRV_CTL_ELEM_TYPE_BYTES:
        ev.value.bytes.data[id] = value;
        break;

    default:
        return AIC_EINVAL;
    }

    ret = snd_ctl_write(&ev);
    if (ret < 0)
        return ret;

    return 0;
}

int audio_ctl_set_array(struct audio_ctl *ctl, const void *array, size_t count)
{
    struct snd_ctl_elem_value ev;
    size_t size;
    void *dest;
    size_t total_count;

    if ((!ctl) || !count || !array)
        return AIC_EINVAL;

    total_count = ctl->info.count;
    if (count > total_count)
        return AIC_EINVAL;

    memset(&ev, 0, sizeof(ev));
    ev.id.numid = ctl->info.id.numid;

    switch (ctl->info.type) {
    case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
    case SNDRV_CTL_ELEM_TYPE_INTEGER:
        size = sizeof(ev.value.integer.value[0]);
        dest = ev.value.integer.value;
        break;
    case SNDRV_CTL_ELEM_TYPE_BYTES:
        size = sizeof(ev.value.bytes.data[0]);
        dest = ev.value.bytes.data;
        break;
    /*
    case SNDRV_CTL_ELEM_TYPE_IEC958:
        size = sizeof(ev.value.iec958);
        dest = &ev.value.iec958;
        break;
    */
    default:
        return AIC_EINVAL;
    }

    memcpy(dest, array, size * count);

    return snd_ctl_write(&ev);
}

int audio_ctl_get_range_min(struct audio_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return AIC_EINVAL;

    return ctl->info.value.integer.min;
}

int audio_ctl_get_range_max(struct audio_ctl *ctl)
{
    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_INTEGER))
        return AIC_EINVAL;

    return ctl->info.value.integer.max;
}

unsigned int audio_ctl_get_num_enums(struct audio_ctl *ctl)
{
    if (!ctl)
        return 0;

    return ctl->info.value.enumerated.items;
}


int audio_ctl_set_enum_by_string(struct audio_ctl *ctl, const char *string)
{
    struct snd_ctl_elem_value ev;
    unsigned int i, num_enums;
    int ret;

    if (!ctl || (ctl->info.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED))
        return AIC_EINVAL;

    num_enums = ctl->info.value.enumerated.items;
    for (i = 0; i < num_enums; i++) {
        if (!strcmp(string, ctl->ename[i])) {
            memset(&ev, 0, sizeof(ev));
            ev.value.enumerated.item[0] = i;
            ev.id.numid = ctl->info.id.numid;
            strncpy((char *)ev.id.name, string, CTL_NAME_LEN_MAX - 1);
            ret = snd_ctl_write(&ev);
            if (ret < 0)
                return ret;
            return 0;
        }
    }

    return AIC_EINVAL;
}

#else


unsigned int audio_ctl_get_num(void)
{
    return 0;
}

const char *audio_ctl_get_name(struct audio_ctl *ctl)
{
    return 0;
}

const char *audio_ctl_get_type_string(struct audio_ctl *ctl)
{
    return 0;
}

enum audio_ctl_type audio_ctl_get_type(struct audio_ctl *ctl)
{
    return 0;
}

struct audio_ctl *audio_clt_get(unsigned int id)
{
    return 0;
}

struct audio_ctl *audio_ctl_get_by_name(const char *name)
{
    return 0;
}

int audio_ctl_get_value(struct audio_ctl *ctl, unsigned int id)
{
    return 0;
}

int audio_ctl_set_value(struct audio_ctl *ctl, unsigned int id, int value)
{
    return 0;
}

int audio_ctl_set_array(struct audio_ctl *ctl, const void *array, size_t count)
{
    return 0;
}

int audio_ctl_get_array(struct audio_ctl *ctl, void *array, size_t count)
{
    return 0;
}

int audio_ctl_get_range_min(struct audio_ctl *ctl)
{
    return 0;
}

int audio_ctl_get_range_max(struct audio_ctl *ctl)
{
    return 0;
}

unsigned int audio_ctl_get_num_enums(struct audio_ctl *ctl)
{
    return 0;
}

int audio_ctl_set_enum_by_string(struct audio_ctl *ctl, const char *string)
{
    return 0;
}

unsigned int audio_ctl_get_num_values(struct audio_ctl *ctl)
{
    return 0;
}

const char *audio_ctl_get_enum_string(struct audio_ctl *ctl,
                                      unsigned int enum_id)
{
    return 0;
}

#endif
