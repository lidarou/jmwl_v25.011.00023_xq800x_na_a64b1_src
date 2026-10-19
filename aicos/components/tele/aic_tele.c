/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  tele init
 */

/*********************
 *      INCLUDES
 *********************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00e955
#define LOG_MODULE    AMOD_SYS

#include "tele_cfg.h"
#include "aic_srv_voice.h"
#include "aic_srv_tele.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int32_t aic_tele_init(void)
{
    int32_t ret = 0;

#ifdef CONFIG_TELE_SERVICE_ENABLE
    /* Tele srv init */
    ret = aic_srv_tele_init();
    aicx_info(0x0aca89, "sd", "[%s]aic_srv_tele_init return %d.", __func__, ret);
#endif

#ifdef CONFIG_BYPASS_MD_AT
    /* TODO: Modem driver device init */
#endif

#ifdef CONFIG_VOICE_SERVICE_ENABLE
    /* Voice srv init */
    ret = aic_srv_voice_init();
    aicx_info(0x0de8c6, "sd", "[%s]aic_srv_voice_init return %d.", __func__, ret);
#endif

    return ret;
}

