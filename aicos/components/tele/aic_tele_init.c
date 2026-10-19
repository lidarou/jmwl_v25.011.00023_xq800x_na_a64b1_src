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
#define AIC_THIS_FILE    0x10cf8f
#define LOG_MODULE    AMOD_SYS

#include "aic_init.h"
#include "aic_tele.h"

static s32 aic_comp_tele_init(void)
{
    int32_t ret = 0;

    ret = aic_tele_init();

    return (s32)ret;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(aic_comp_tele_init);
