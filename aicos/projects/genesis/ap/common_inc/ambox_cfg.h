/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __AMBOX_CFG_H__
#define __AMBOX_CFG_H__

#include <aic_auto_reg.h>
#include <irq.h>

#define MBOX_ID_CFG_CNT         3
#define MBOX_ID_MASK            0xff

#define AMBOX_THIS_BOX          AMBOX_AP_ID
#define AMBOX_CORE2BOX(id)      (id)

#define AMBOX_REG_BASE          REG_BASE_TOP_MBOX
#define AMBOX_EB_REG            REG_TOP_GLB_MBOX_EB
#define AMBOX_EB_BIT            BIT_TOP_GLB_MBOX_EB
#define AMBOX_RECV_IRQ          IRQ_MB_OUT
#define AMBOX_SEND_IRQ          IRQ_MB_IN
#define AMBOX_DIR_IRQ           IRQ_MB_OUT_DIR

#endif /* __AMBOX_CFG_H__ */

