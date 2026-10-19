/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#ifndef __AMBOX_CFG_H__
#define __AMBOX_CFG_H__

#include <aic_auto_reg.h>
#include <irq.h>

#define MBOX_SLEEP_CLOSE_MBOX
#define MBOX_RESUME_RESET_CP_RECV_MASK

#define MBOX_ID_CFG_CNT         2
#define MBOX_ID_MASK            0xff

#define AMBOX_THIS_BOX          AMBOX_AP_ID
#define AMBOX_CORE2BOX(id)      (id)

#define AMBOX_REG_BASE          REG_BASE_BLK_MEM_CTRL_MBOX
#define AMBOX_EB_REG            REG_CKM_EN_GLB_TOP_TOP_MAILBOX_EB
#define AMBOX_EB_BIT            BIT_CKM_EN_GLB_TOP_TOP_MAILBOX_EB
#define AMBOX_RECV_IRQ          IRQ_MB_OUT // IRQ_MBOX_OUTBOX_D_AP
#define AMBOX_SEND_IRQ          IRQ_MB_IN // IRQ_MBOX_INBOX_AP
#define AMBOX_DIR_IRQ           IRQ_MB_OUT_DIR // IRQ_MBOX_OUTBOX_D_AP,

#endif /* __AMBOX_CFG_H__ */

