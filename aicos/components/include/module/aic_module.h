/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define all mudule in aic software.
 */
#ifndef __AIC_MODULE_H__
#define __AIC_MODULE_H__
#include <aic_type.h>

#define MODULE_SUPPORT_GET_NAME
#define MODULE_SAVE_SPACE_AND_FAST

/**
 * +++++++++++++++++++++++++++++++++++++++++++
 *                WARNING
 * Add a module here, must add the module
 * string in aic_module.c.
 * +++++++++++++++++++++++++++++++++++++++++++
 */
enum {
    /* AIPC mod use 0, can't be change. */
    AMOD_AIPC = 0,

    /* componets module */
    AMOD_AOS,
    AMOD_SYS,
    AMOD_INIT,
    AMOD_CONS,
    AMOD_ALOG,
    AMOD_DIAG,
    AMOD_SUBSYS,
    AMOD_NVRAM,
    AMOD_MISC,
    AMOD_FS,
    AMOD_MCX,
    AMOD_LVGL,
    AMOD_WIFI,
    AMOD_BLT,
    AMOD_PRODINFO,

    /* mm module */
    AMOD_AUDIO,
    AMOD_CAMERA,

    /* chip dirver module */
    AMOD_DRV,
    AMOD_AMBOX,
    AMOD_BUSMON,
    AMOD_USB,
    AMOD_FLASH,
    AMOD_SOUND,
    AMOD_I2C,
    AMOD_TIMER,
    AMOD_DMA,
    AMOD_GPD,
    AMOD_GPIO,
    AMOD_SoCMPU,
    AMOD_PCDUMP,
    AMOD_PERFMON,
    AMOD_PMIC_I2C,
    AMOD_SLP,
    AMOD_EIPU,
    AMOD_ZJTAG,
    AMOD_KPC,
    AMOD_TSC,
    AMOD_DBG,
    AMOD_SDMMC,
    AMOD_RTC,
    AMOD_CLK_CALI,
    AMOD_SSP,
    AMOD_CMUX,
    AMOD_CAN,

    /* modem module */
    AMOD_MODEM,
    AMOD_AT,
    AMOD_IMS,
    AMOD_SIM,
    AMOD_TEL,
    AMOD_PB,
    AMOD_IPCC,
    AMOD_ATM,
    AMOD_PDCP,
    AMOD_RLC,
    AMOD_MAC,
    AMOD_HC,

    /* net module */
    AMOD_NET,

    /* secure boot*/
    AMOD_SECBOOT,

    /* app module */
    AMOD_APP,
    AMOD_FOTA,
    AMOD_GNSS,

    /* phy module */
    AMOD_PHY,

    /* EL2 module */
    AMOD_EL2,

    /* new add module */
    AMOD_VENDOR,
    AMOD_I2S,
    AMOD_PMIC,
    AMOD_GPAPH,
    AMOD_AS,
    AMOD_ERRC,
    AMOD_L2,
    AMOD_MMA,
    AMOD_NAS,
    AMOD_UAI,

    AMOD_ACCDD,
    AMOD_RFDD,
    AMOD_D2D,
    AMOD_L1C,

    AMOD_CNT,
};

#define MAX_AMOD_CNT  256
#if (AMOD_CNT >= MAX_AMOD_CNT)
#error The max moudlue num is 254
#endif

char *aic_get_moudule_name(u8 module);
u8 aic_get_moudule(char *name);
u8 aic_get_moudule_def_loglv(u8 module);

#endif /* __AIC_MODULE_H__ */

