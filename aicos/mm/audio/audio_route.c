/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is about audio route control, all routes and corresponding
 *   configures are defined here, relation APIs can enable/disable audio
 *   controls provided by audio Hal.
 *
 *****************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c7347
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>

#include "audio_system.h"
#include "audio_route.h"
#include "audio_control.h"
#include "audio_util.h"

struct aroute_cfg {
    char *name;
    u32 val_on;
    u32 val_off;
};

const struct aroute_cfg aroute_cfg_spk[] = {
    AUDIO_ROUTE_CFG_SPK
};

const struct aroute_cfg aroute_cfg_hp[] = {
    AUDIO_ROUTE_CFG_HP
};

const struct aroute_cfg aroute_cfg_ep[] = {
    AUDIO_ROUTE_CFG_EP
};

const struct aroute_cfg aroute_cfg_mic0[] = {
    AUDIO_ROUTE_CFG_MIC0
};

const struct aroute_cfg aroute_cfg_mic1[] = {
    AUDIO_ROUTE_CFG_MIC1
};

const struct aroute_cfg aroute_cfg_mic_hp[] = {
    AUDIO_ROUTE_CFG_MIC_HP
};

static int get_route_cfg(enum audio_device device, struct aroute_cfg **cfg)
{
    int cnt = 0;

    switch (device) {
    case AUDIO_DEVICE_OUT_EARPIECE:
        *cfg = (struct aroute_cfg *)aroute_cfg_ep;
        cnt = sizeof(aroute_cfg_ep) / sizeof(struct aroute_cfg);
        break;
    case AUDIO_DEVICE_OUT_SPEAKER:
        *cfg = (struct aroute_cfg *)aroute_cfg_spk;
        cnt = sizeof(aroute_cfg_spk) / sizeof(struct aroute_cfg);
        break;
    case AUDIO_DEVICE_OUT_HEADSET:
    case AUDIO_DEVICE_OUT_HEADPHONE:
        *cfg = (struct aroute_cfg *)aroute_cfg_hp;
        cnt = sizeof(aroute_cfg_hp) / sizeof(struct aroute_cfg);
        break;
    case AUDIO_DEVICE_IN_MIC:
        *cfg = (struct aroute_cfg *)aroute_cfg_mic0;
        cnt = sizeof(aroute_cfg_mic0) / sizeof(struct aroute_cfg);
        break;
    case AUDIO_DEVICE_IN_HEADPHONE:
        *cfg = (struct aroute_cfg *)aroute_cfg_mic_hp;
        cnt = sizeof(aroute_cfg_mic_hp) / sizeof(struct aroute_cfg);
        break;
    case AUDIO_DEVICE_IN_BACK_MIC:
        *cfg = (struct aroute_cfg *)aroute_cfg_mic1;
        cnt = sizeof(aroute_cfg_mic1) / sizeof(struct aroute_cfg);
        break;
    /* TODO: */
    case AUDIO_DEVICE_OUT_BT_SCO:
    case AUDIO_DEVICE_OUT_BT_A2DP:
    case AUDIO_DEVICE_OUT_FM:
    case AUDIO_DEVICE_IN_BT_SCO:
    case AUDIO_DEVICE_IN_LINE:
    default:
        break;
    }

    return cnt;
}

void audio_route_set(enum audio_device device, int on)
{
    int cnt, i;
    struct aroute_cfg *cfg = NULL;
    struct audio_ctl *ctl;

    cnt = get_route_cfg(device, &cfg);
    AIDXD(0x056a97, "dd", "audio route set, device=%d, cnt=%d\n", device, cnt);

    for (i = 0; i < cnt; i++) {
        ctl = audio_ctl_get_by_name(cfg[i].name);
        if (ctl) {
            if (on)
                audio_ctl_set_value(ctl, 0, cfg[i].val_on);
            else
                audio_ctl_set_value(ctl, 0, cfg[i].val_off);
        } else
            AIDXE(0x17848f, "s", "no this audio control:%s\n", cfg[i].name);
    }
}

