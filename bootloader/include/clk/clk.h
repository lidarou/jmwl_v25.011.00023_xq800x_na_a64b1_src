/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is export uart phy interface.
 */

#ifndef __CLK_INIT_H__
#define __CLK_INIT_H__

#include <pmic_clk_cfg.h>

#ifdef CONFIG_CLK
void clk_init(void);
#else
static inline void clk_init(void)
{
}
#endif

enum CLK_32K_SRC_SEL {
    TCXO_EXTERNAL_32K = 0,
    TCXO_32K,
    TCXO_32KLESS,
    TCXO_RC32K,
    DCXO_EXTERNAL_32K,
    DCXO_32K,
    DCXO_32KLESS,
    DCXO_RC32K
};

#ifdef CONFIG_PMIC_CLK
int pmic_clk_init(enum CLK_32K_SRC_SEL clk_32k_src_sel);
#else
static int pmic_clk_init(enum CLK_32K_SRC_SEL clk_32k_src_sel)
{
}
#endif

#endif /*__CLK_INIT_H__*/

