/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the IIS definition related with chip.
 */

#define CLK_IIS_EN                  0

#define CLK_IIS_PLL                 CLK_TOP_IIS_CFG

#if (IIS_MCLK == 12288000)
#define CLK_IIS_PARENT              4
#elif (IIS_MCLK == 24576000)
#define CLK_IIS_PARENT              6
#else
#error "not support this freqency!"
#endif

#define CLK_SET_IIS_PARAENT         clk_set_parent_mux

#define CLK_SET_IIS_RATE_COMP(id, clk)

