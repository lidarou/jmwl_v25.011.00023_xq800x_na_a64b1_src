/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file declare API of psram module.
*/

#ifndef __PSRAM_H__
#define __PSRAM_H__

void psram_exit_sleep_mode(void);
void psram_psramc_recovery(u8 tfp);
void psram_sw_dfs(u8 tfp);
void psram_mtb_code_preload(void);

#endif /* __PSRAM_H__ */
