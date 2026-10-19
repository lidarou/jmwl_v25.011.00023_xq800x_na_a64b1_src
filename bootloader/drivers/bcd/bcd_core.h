/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines bcd driver api.
 *    The bcd api will be called by aicos user functions.
 */

#ifndef __BCD_CORE_H__
#define __BCD_CORE_H__

#define AIC_BC_DETECT_DELAY 100 //ms

int aic_bcd_result(void);
int aic_chg_int_rise(void);

#endif /* __BCD_CORE_H__ */
