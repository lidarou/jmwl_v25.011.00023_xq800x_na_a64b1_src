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

struct qspi_cfg {
    u32 cs_num;
    u32 dual_xip_mode;
};

void qspi_init(struct qspi_cfg *cfg);

#endif /* __QSPI_INC_H__ */

