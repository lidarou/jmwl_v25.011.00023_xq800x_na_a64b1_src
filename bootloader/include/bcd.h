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

#ifndef __BCD_H__
#define __BCD_H__

typedef enum {
    AIC_BC_TYPE_NO_DCD = 0,
    AIC_BC_TYPE_PS2,
    AIC_BC_TYPE_SDP,
    AIC_BC_TYPE_CDP,
    AIC_BC_TYPE_DCP,
    AIC_BC_TYPE_MAX
} AIC_BC_TYPE;

int bcd_detect(void);

#endif /* __BCD_H__ */
