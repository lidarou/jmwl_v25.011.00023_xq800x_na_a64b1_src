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

#define CLK_IIS_EN                  CLK_APLL_EN

#define CLK_IIS_PLL                 CLK_TOP_IIS0

#define CLK_IIS_PARENT              1

#define CLK_SET_IIS_PARAENT         clk_set_parent_comp

#define CLK_SET_IIS_RATE_COMP(id, clk)  clk_set_rate_comp(id, clk)

