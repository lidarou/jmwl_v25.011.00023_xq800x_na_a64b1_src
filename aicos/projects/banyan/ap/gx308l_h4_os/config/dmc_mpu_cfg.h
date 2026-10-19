/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __DMC_MPU_BOARD_CFG_H__
#define __DMC_MPU_BOARD_CFG_H__

#include <psram_mem_config.h>

#define DMC_MPU_AP_USRID            0xFFDF0000
#define DMC_MPU_CP_USRID            0xFFCF0020
#define DMC_MPU_MDM_USRID           0xFFCF0030

/* dynamics config channel, configured by at or console cmd */
#define DMC_MPU_DYNC_CHN            7

struct gx_dmc_mpu_def_mnt {
    char *name;
    u32 usrid;
    u32 start;
    u32 end;
};

static struct gx_dmc_mpu_def_mnt gx_dmc_mpu_mnt[] = {
    {"AP overlay CP",       DMC_MPU_AP_USRID,   CP_REGION_BASE,     CP_REGION_BASE + CP_REGION_SIZE - 1},
    {"CP overlay AP",       DMC_MPU_CP_USRID,   AP_REGION_BASE,     AP_REGION_BASE + AP_REGION_SIZE - 1},
    {"Modem overlay AP",    DMC_MPU_MDM_USRID,  0x80173000,         0x801757FF},
};

#endif /* __DMC_MPU_BOARD_CFG_H__ */

