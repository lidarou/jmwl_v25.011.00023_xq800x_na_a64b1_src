/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the mailbox api.
 */

#ifndef __AMBOX_API_H__
#define __AMBOX_API_H__
#include <aic_type.h>

/**
 * All pair mailboxes of the aic chip define.
 * A pail boxes contains 2 parts:the send box and the receive box.
 * Usually system use a pair of boxes to communicate with other system.
 * use It's send box send to other system's receive box.
 * use it's receive box receives messages that sent by other system.
 */
typedef enum {
    AMBOX_PAIR_0 = 0,
    AMBOX_PAIR_1,
    AMBOX_PAIR_2,
    AMBOX_PAIR_CNT
} AMBOX_ID_T;

/**
 * All system core define.
 * Default, the system core value is as same as box id of this system.
 * May some chip assign the box id to system is differrent from default.
 * So The real map please see the feature AMBOX_CORE2BOX that defined in
 * mbtx_cfg.h.
 */
typedef enum {
    AMBOX_AP_ID = 0,
    AMBOX_CP_ID,
#ifdef CONFIG_AIC_CHIP_GENESIS
    AMBOX_ADSP_ID,
#endif
    AMBOX_MAX_CORE_ID
} AMBOX_CORE_T;

/*
 * Assign boxes to each system according the design of aisc.
 * Here just defined for win32, for chip please see the define
 * in mbox_cfg.h.
 */
#if defined(WIN32)
#define MBOX_ID_CFG_CNT     AMBOX_PAIR_CNT
#define AMBOX_THIS_BOX      AMBOX_AP_ID
#define AMBOX_CORE2BOX(id)  (id)
#define AMBOX_RECV_IRQ      0
#define AMBOX_SEND_IRQ      0
#endif

/* ambox message data struct */
typedef struct ambox_msg {
    u32 p1;
    u32 p2;
} ambox_msg_t;

typedef void (*AMBOX_HANDLER_T)(ambox_msg_t *msg, void *p);

/**
 * ambox_send_message - This api is used to send a message to remote system.
 *
 * @ambox_core: the remote system mbox core id.
 * @ambox_msg_t: the ambox message pointer.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ambox_send_message(AMBOX_CORE_T ambox_core, ambox_msg_t* msg);

/**
 * ambox_raw_send_msg - This api is only used to send a msg in irq timer or
                        assert context, It can't cause schadule.
 *
 * @ambox_core: the remote system mbox core id.
 * @ambox_msg_t: the ambox message pointer.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ambox_raw_send_msg(AMBOX_CORE_T ambox_core, ambox_msg_t* msg);

/**
 * ambox_register_handler - Register a message handler to process the message
 * received from the specified box.
 *
 * @ambox_core: the remote system mbox core id.
 * @ambox_msg_t: the ambox message pointer pointer.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ambox_register_handler(AMBOX_CORE_T ambox_core, AMBOX_HANDLER_T handler, void *p);

/**
 * ambox_ubregister_handler - Unregisterthe the message handler.
 *
 * @ambox_id: the mailbox id of the source system who send the message.
 */
void ambox_unregister_handler(AMBOX_CORE_T ambox_core);

/**
 * ambox_get_recv_cnt - Get the recv mail cnt of the specified box.
 *
 * @ambox_core: the remote system mbox core id.
 */
int ambox_get_recv_cnt(AMBOX_CORE_T ambox_core);

/**
 * ambox_empty_recvbox_inassert - Empty the current sys recvbox in assert.
 */
void ambox_empty_recvbox_inassert(void);

/**
 * ambox_empty_a_recvbox - Empty a specified recvbox.
 *
 * @ambox_core: the remote system mbox core id.
 */
void ambox_empty_a_recvbox(AMBOX_CORE_T ambox_core);

#endif /* __AMBOX_API_H__ */

