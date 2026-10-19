/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines interrupt process handler.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d3d22
#define LOG_MODULE    AMOD_DRV

#ifdef CONFIG_DBG_CPPC_SAMP
#include <aic_type.h>
#include <sys_mem_config.h>
#include <timer_inc.h>
#include <reg_base.h>

/* use the end of ap rw region. */
#define PC_SAMP_REGION_SIZE     0x40000

#define PC_SAMP_REGION_END      (AP_REGION_BASE + AP_REGION_SIZE)
#define PC_SAMP_REGION_BASE     (PC_SAMP_REGION_END - PC_SAMP_REGION_SIZE)
#define TMR_INT_CLR             0x10

const u32 pc_samp_base = PC_SAMP_REGION_BASE;
const u32 pc_samp_end = PC_SAMP_REGION_END;
const u32 tmr_clr_reg = (REG_BASE_BLK_CP_TMR + TMR_INT_CLR);

void cpu_pc_samp(u32 act, u32 us)
{
    aicx_info(0x0e4175, "dd", "cpu_pc_samp: act=%d. us=%d.\n", act, us);

    if (act == 0)
        us = 0;

    aic_tmr_samp_cfg(us);
}
#endif

