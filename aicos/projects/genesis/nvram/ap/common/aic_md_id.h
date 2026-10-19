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
    MD_ID_D2D,                                                      /* 29 */
    MD_ID_SETTING_FAMILY_ENABLE,                                    /* 30 */ //亲情号使能 ID
    MD_ID_SETTING_FAMILY_NUMBER,                                    /* 31 */ //亲情号号码 ID
    MD_ID_SETTING_SOS_ENABLE,                                       /* 32 */ //SOS使能 ID
    MD_ID_SETTING_SOS_NUMBER,                                       /* 33 */ //SOS号号码 ID
    MD_ID_SETTING_DISPLAY,                                          /* 34 */ //显示时常以及亮度 ID
    MD_ID_SETTING_PHONE,                                            /* 35 */ //手机设置 ID
    MD_ID_SETTING_NET,                                              /* 36 */ //网络设置 ID
    MD_ID_SETTING_SECURITY,                                         /* 37 */ //安全设置 ID
    MD_ID_SETTING_THEME,                                            /* 38 */ //情景模式 ID
    MD_ID_SETTING_DATE,                                             /* 39 */ //日期时间 ID
    MD_ID_SETTING_COMM,                                             /* 40 */ //通话设置 ID
    MD_ID_TOOLS_ALARM,                                              /* 41 */ //工具箱中的闹钟 ID
    MD_ID_TOOLS_TODO,                                               /* 42 */ //工具箱中的备忘录 ID
    MD_ID_TOOLS_TODO_CONTENT,                                       /* 43 */ //工具箱中的备忘录内容 ID
    MD_ID_PHONE_INITED,                                             /* 44 */ //手机开机NVRAM初始化标志 ID
    MD_ID_ALARM_DOING_ID,                                           /* 45 */ //保存手机中正在起作用的闹铃的编号，用于关机时响闹铃导致开机显示闹铃内容 ID
    MD_ID_TTS_PARAM,                                                /* 46 */ //语音王控制
    MD_ID_TTS_SERVER_INFO,                                          /* 47 */ //语音王服务
    MD_ID_PHONEBOOK_DISPLAY,                                        /* 48 */ //电话本显示存储

    MD_ID_PLATFORM_END = MD_ID_PLATFORM_MAX,                        /* 99 */

    MD_ID_AUDIO_START = MD_ID_AUDIO_BASE,                           /* 100 */
    MD_ID_AUDIO_END = MD_ID_AUDIO_MAX,                              /* 199 */

    MD_ID_END,
}AIC_MD_ID_E;

#define MD_ID_MAX                   MD_ID_END

#endif /* __AIC_MD_ID_H__ */

