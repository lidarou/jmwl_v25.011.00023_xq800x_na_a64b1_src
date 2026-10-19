/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the qspi export structure and interface.
 */

#ifndef __QSPI_INC_H__
#define __QSPI_INC_H__

#include <aic_type.h>

struct flash_pin {
    u32 reg_adr;
    u32 reg_val;
};

struct qspi_cfg {
    u32 cs_num;
    u32 dual_xip_mode;
    u32 flash1_size;
};

void qspi_init(struct qspi_cfg *cfg);
void qspi_sleep_suspend(void);
void qspi_sleep_resume(u8 tfp);
void qspi_sw_dfs(u8 tfp);
void qspi_lock(void);
void qspi_unlock(void);
void qspi_clk_switch(u8 tfp);

#endif /* __QSPI_INC_H__ */

