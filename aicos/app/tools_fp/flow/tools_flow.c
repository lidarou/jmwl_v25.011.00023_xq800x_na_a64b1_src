

/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is defined apis of settings
 */

/*********************
 *      INCLUDES
 *********************/
#include "app_log.h"
#include "aic_srv_tele.h"
#include "app_msg.h"
#include "factoryreset.h"
#include "settings_fp_flow.h"
#include "sysoff.h"
#include "app_audio.h"
#include "audio_system.h"
#include "backlight.h"
#include "aic_md_api.h"
#include "aic_md_id.h"

/*********************
 *      DEFINES
 *********************/
#ifndef min
#define min(a,b)                ((a) < (b) ? (a) : (b))
#endif


/************************
 *  STATIC VARIABLES
 ************************/
