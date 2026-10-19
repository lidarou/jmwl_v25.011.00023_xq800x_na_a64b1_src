/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the range of misc id
 */

#ifndef __AIC_MD_ID_H__
#define __AIC_MD_ID_H__

#define MD_ID_MIN                   0

/* MD CONFIG BASE ID: 0 */
#define MD_ID_CONFIG_BASE           (MD_ID_MIN)
#define MD_ID_CONFIG_SIZE           (5)
#define MD_ID_CONFIG_MAX            (MD_ID_CONFIG_BASE + MD_ID_CONFIG_SIZE - 1)

/* PLATFORM BASE ID: 5 */
#define MD_ID_PLATFORM_BASE         (MD_ID_CONFIG_MAX + 1)
#define MD_ID_PLATFORM_SIZE         (95)
#define MD_ID_PLATFORM_MAX          (MD_ID_PLATFORM_BASE + MD_ID_PLATFORM_SIZE - 1)

/* AUDIO BASE ID: 100 */
#define MD_ID_AUDIO_BASE            (MD_ID_PLATFORM_MAX + 1)
#define MD_ID_AUDIO_SIZE            (100)
#define MD_ID_AUDIO_MAX             (MD_ID_AUDIO_BASE + MD_ID_AUDIO_SIZE - 1)

typedef enum {
    MD_ID_START = MD_ID_MIN,                                        /* 0 */

    MD_ID_CONFIG_START = MD_ID_CONFIG_BASE,                         /* 0 */
    MD_ID_VERSION,                                                  /* 1 */
    MD_ID_CONFIG_END = MD_ID_CONFIG_MAX,                            /* 4 */

    MD_ID_PLATFORM_START = MD_ID_PLATFORM_BASE,                     /* 5 */
    MD_ID_RESERVED = MD_ID_PLATFORM_START,                          /* 5 */
    MD_ID_MAC,                                                      /* 6 */
    MD_ID_MANUFACTURER_INFO,                                        /* 7 */
    MD_ID_MODEL_INFO,                                               /* 8 */
    MD_ID_FIXED_APN_INFO,                                           /* 9 */
    MD_ID_ADDITIONAL_APN_INFO,                                      /* 10 */
    MD_ID_DCXOCALI_INFO,                                            /* 11 */
    MD_ID_SN0,                                                      /* 12 */
    MD_ID_SN1,                                                      /* 13 */
    MD_ID_DEBUG,                                                    /* 14 */
    MD_ID_NET_CFG,                                                  /* 15 */
    MD_ID_RTC,                                                      /* 16 */
    MD_ID_CM_AT_LBS,                                                /* 17 */
    MD_ID_EXT_MODEM,                                                /* 18 */
    MD_ID_MOBILE_DM,                                                /* 19 */
    MD_ID_CERT_KEY0,                                                /* 20 */
    MD_ID_CERT_KEY1,                                                /* 21 */
    MD_ID_CERT_KEY2,                                                /* 22 */
    MD_ID_CERT_KEY3,                                                /* 23 */
    MD_ID_CERT_KEY4,                                                /* 24 */
    MD_ID_CERT_KEY5,                                                /* 25 */
    MD_ID_CERT_KEY6,                                                /* 26 */
    MD_ID_CERT_KEY7,                                                /* 27 */
    MD_ID_CERT_KEY8,                                                /* 28 */
    MD_ID_D2D_L3_CONFIG,                                            /* 29 */
    MD_ID_D2D_ID_INFO,                                              /* 30 */

    MD_ID_D2D = 40,                                                 /* 40 */
    MD_ID_DEV_INFO = 41,                                            /* 41 */

    MD_ID_PLATFORM_END = MD_ID_PLATFORM_MAX,                        /* 99 */

    MD_ID_AUDIO_START = MD_ID_AUDIO_BASE,                           /* 100 */
    MD_ID_AUDIO_END = MD_ID_AUDIO_MAX,                              /* 199 */

    MD_ID_END,
}AIC_MD_ID_E;

#define MD_ID_MAX                   MD_ID_END

#endif /* __AIC_MD_ID_H__ */

