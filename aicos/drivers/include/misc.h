/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Interface of misc.
 *
 */

#ifndef __MISC_H__
#define __MISC_H__

#include <aic_type.h>

void eda_dbgreg_wr(u32 val);

u32 fpga_bitfile_id(void);

u32 fpga_bitfile_freq(void);

#endif /* __MISC_H__ */
