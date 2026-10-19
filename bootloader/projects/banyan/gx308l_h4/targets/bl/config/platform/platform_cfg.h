/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PROJECT_PLATFORM_CFG_H__
#define __PROJECT_PLATFORM_CFG_H__

#include "inttypes.h"
#include "io.h"
#include "platform/platform.h"

#define BOOT_PARAMETERS_ADDR    0x20000000
#define STARG2_BOOT_ADDR        0x20014000

struct platform_core platform = {
    .bp = (struct boot_parameters *)BOOT_PARAMETERS_ADDR,
    .next_img = (uint8_t *)STARG2_BOOT_ADDR,
    .name = "bootloader",
};

#endif /* __PROJECT_PLATFORM_CFG_H__ */
