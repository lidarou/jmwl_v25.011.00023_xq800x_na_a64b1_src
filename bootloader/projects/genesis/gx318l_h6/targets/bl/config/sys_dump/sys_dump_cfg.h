/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sys dump configuration.
 */

#ifndef __SYS_DUMP_CFG_H__
#define __SYS_DUMP_CFG_H__

#include <aonram_mem_config.h>
#include <flash_mem_config.h>
#include <psram_mem_config.h>

static const struct sys_dump_cfg_info s_sys_dump_cfg[] = {
    {
        .type = DUMP_MEM,
        .address = AONRAM_REGION_BASE,
        .length = AONRAM_REGION_SIZE,
    },
    {
        .type = DUMP_MEM,
        .address = PSRAM_REGION_BASE,
        .length = PSRAM_REGION_SIZE,
    },
    {
        .type = DUMP_MEM,
        .address = FLASH0_BASE,
        .length = FLASH0_TOTAL_SIZE,
    },
};

static const struct sys_dump_cfg_info *sys_dump_get_cfg(void)
{
    return s_sys_dump_cfg;
}

static int sys_dump_get_cfg_num(void)
{
    return sizeof(s_sys_dump_cfg)/sizeof(struct sys_dump_cfg_info);
}

#endif /* __SYS_DUMP_CFG_H__ */
