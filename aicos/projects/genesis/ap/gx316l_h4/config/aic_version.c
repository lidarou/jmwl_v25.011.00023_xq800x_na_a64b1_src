/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC version configurations.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19af7c
#define LOG_MODULE    AMOD_SYS

#include <aic_version.h>
#include <stdio.h>
#include <aic_cmdline.h>

#define AIC_RELEASE_NUM   "00009"
#define AIC_BUILD_NUMBER  "J444"

struct aic_version s_version = {
    .version = AIC_SW_VERSION,
    .branch = AIC_SW_BRANCH,
    .board = CMAKE_CONFIG_BOARD,
    .date    = __DATE__,
    .time    = __TIME__
};

void aic_get_version(char *buf, size_t size)
{
    snprintf_nano(buf, size, AIC_VERSION_FMT,
             s_version.version,
             AIC_RELEASE_NUM,
             s_version.board,
             aic_is_user_mode() ? "release" : "debug",
             AIC_BUILD_NUMBER,
             s_version.branch,
             s_version.date,
             s_version.time);
}

