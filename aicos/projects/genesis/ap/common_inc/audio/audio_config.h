/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines audio module configurations.
 */

#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#define AUDIO_ROUTE_CFG_SPK     \
    { \
        .name = "SPKL Ctl", \
        .val_on = 1, \
        .val_off = 0,\
    },

#define AUDIO_ROUTE_CFG_HP      \
    { \
        .name = "HP Input Mux", \
        .val_on = 1, \
        .val_off = 0, \
    },

#define AUDIO_ROUTE_CFG_EP      \
    { \
        .name = "EP Input Mux", \
        .val_on = 1, \
        .val_off = 0, \
    },

#define AUDIO_ROUTE_CFG_MIC0    \
    { \
        .name = "MIC0 Input Mux", \
        .val_on = 1, \
        .val_off = 0, \
    },

#define AUDIO_ROUTE_CFG_MIC1      \
    { \
        .name = "MIC1 Input Mux", \
        .val_on = 1, \
        .val_off = 0, \
    },

#define AUDIO_ROUTE_CFG_MIC_HP  \
    { \
        .name = "HP MIC Input Mux", \
        .val_on = 1, \
        .val_off = 0, \
    },

/* audio volume level, 100 is the top, total steps 0 - AUDIO_MVOL_LEVEL_MAX */
#define AUDIO_VOLUME_LEVEL_TBL   \
    {0, 10, 20, 26, 33, 39, 46, 53, 60, 66, 72, 78, 84, 90, 95, 100}

#define SUPPORT_AUDIO_DEVICE_OUT            AUDIO_DEVICE_OUT_SPEAKER
#define SUPPORT_AUDIO_DEVICE_IN             AUDIO_DEVICE_IN_MIC

#endif /* __AUDIO_CONFIG_H__ */

