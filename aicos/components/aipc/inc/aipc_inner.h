
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aipc private data and api.
 */

#ifndef __AIPC_INNER_H__
#define __AIPC_INNER_H__
#include <aipc_amem.h>
#include <aipc_adapter.h>
#include <aipc_api.h>
#include <aipc_cfg.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <aic_list.h>
#include <stdio.h>

 /*
  * CONFIG_ABLK_SYNC_CREATE_LINK
  *  if open it, when the client create or destroy a link will wait the
  *  host finish create or destroy the link.
  */
#define CONFIG_ABLK_SYNC_CREATE_LINK   0

 /*
  * CONFIG_ABLK_DYNAMIC_SUSPEND_LINK
  *  if open it, can support dynamic suspend a link
  *  host finish create or destroy the link.
  */
#define CONFIG_ABLK_DYNAMIC_SUSPEND_LINK   0

 /*
  * CONFIG_ABLK_DYNAMIC_SET_LOOPBACKL
  *  if open it, can support dynamic set a brr to loopback.
  *  host finish create or destroy the link.
  */
#define CONFIG_ABLK_DYNAMIC_SET_LOOPBACKL   0

 /*
  * CONFIG_AIP_PRINT_SHMEM
  *  if open it, will print the share memory used info.
  *  host finish create or destroy the link.
  */
#define CONFIG_AIP_PRINT_SHMEM              0

 /*
  * ABLK_DEBUG_CHECK_PARAM - open it will check some parameters for aipc API.
  * In release version should close it.
  */
#define ABLK_DEBUG_CHECK_PARAM  0

 /*
  * ABLK_LINK_HEAD_DEBUG - open it will debug the ablk link head.
  * In release version should close it.
  */
#define ABLK_LINK_HEAD_DEBUG    0

/**
 * aipc state defines.
 * AIPC_STATE_IDLE: the default init state of the aipc.
 * AIPC_STATE_READY:After the aipc handshake, chane the state to ready.
 * In idle state, the aipc can only process handshake message, In ready state
 * the aipc can process normal message.
*/
#define AIPC_STATE_IDLE     (0x0)
#define AIPC_STATE_READY    (0x1)

/**
 * the handshake message define.
 */
enum {
    AIPC_OPEN_REQ = 0x1234,
    AIPC_OPEN_ACK,
    AIPC_CLOSE_REQ = 0x3456,
    AIPC_CLOSE_ACK,
};

/* share memory read and write. */
#define AMEM_READB(addr)    *(volatile u8)(addr)
#define AMEM_READW(addr)    *(volatile u16)(addr)
#define AMEM_READL(addr)    *(volatile u32)(addr)
#define AMEM_WRITB(addr, v) *(volatile u8)(addr) = (v)
#define AMEM_WRITW(addr, v) *(volatile u16)(addr) = (v)
#define AMEM_WRITL(addr, v) *(volatile u32)(addr) = (v)

/* 256 is the max value of u8, max save 32 msg in the cache. */
#define AIPC_CACHE_CNT      (256)
#define AIPC_CACHE_FULL     (AIPC_CACHE_CNT - sizeof(AMSG_T))
#define INC_MSG(pos)        (pos) += sizeof(AMSG_T);
#define DEC_MSG(pos)        (pos) -= sizeof(AMSG_T);

#define AIPC_RECV_BAK_CNT   20
#define AIPC_SEND_BAK_CNT   32

#define AIPC_UNUSED_P    (0xfefe)

typedef struct AIPC {
    /* remote system */
    u8    sys;

    /* local sysyem is sub system */
    u8    is_subsys;

    /* aipc state */
    u8 volatile state;
    u8          pad;

    u8          recv_bak_cnt;
    u8          send_bak_cnt;

    /* msg cache read and write point */
    u8 volatile rdptr;
    u8 volatile wtptr;

    /* msg cache buffer */
    AMSG_T cache[AIPC_CACHE_CNT / sizeof(AMSG_T)];
    AMSG_T *recv_bak;
    AMSG_T *send_bak;

    /* normal memory instance. */
    AMEM_T  *amem_normal;
    /* internal memory instance. */
    AMEM_T  *amem_internal;

    char                *name;
    aos_thread_id       thread;
    aos_semaphore_ptr   sem_recv;
    aos_event_ptr       event_ready;

    /* the AMSG handler list header */
    struct list_head    amsg_handler;

    /* the ABLK handler list header */
    struct list_head    ablk_handler;
} AIPC_T;

typedef struct AMH_ITEM {
    u8      id;
    void    *data;
    AMSG_HANDLER_T      handler;
    struct list_head    next;
} AMH_ITEM_T;

AIPC_T *aipc_create(const AIPC_CFG_T *cfg);
int aipc_destroy(AIPC_T *aipc);
AIPC_T *aipc_get_ipc(u8 sys);
void aipc_free_bak_msgs(AIPC_T *aipc);

#if IS_CONF_ENABLED(AIPC_SIMULATOR_IN_ONE_SYS)
AIPC_T *aipc_simulator_get_ipc(u8 sys);
ABRR_HANDLE ablk_simulator_request_brr(u8 sys, u8 brr_id);
#endif

int amsg_send(AIPC_T *aipc, AMSG_T *msg);

/**
 * amsg_register_brr_handler - Register a message handler of the specified
 *                             bearer. Who register the bearer indicates who
 *                             owns the bearer. Each bearer can be owned by
 *                             only an owner.
 *
 * @sys: the remote system that communicates with the local system.
 * @type: the protocol type.
 * @brr_id: bearer id, representing the receiver.
 * @handler: the message handler function. Details see the description of
 *           the struct AMSG_HANDLER_T.
 * @p: the data pointer of callback handler function
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int amsg_register_brr_handler(u8 sys, u8 type, u8 brr_id,
                              AMSG_HANDLER_T handler, void *p);

/**
 * amsg_unregister_brr_handler - Unregister the message handler of the specified
 *                               bearer.
 *
 * @sys: the remote system that communicates with the local system.
 * @type: the protocol type.
 * @brr_id: bearer id, representing the receiver.
 */
void amsg_unregister_brr_handler(u8 sys, u8 type, u8 brr_id);

/**
 * amsg_set_msg - This api is used to set a message context.
 *
 * @msg: the message pointer.
 * @ty: the proctol type.
 * @id: the bearer id.
 * @p1: the parameter 1.
 * @p2: the parameter 2.
 */
#ifdef CONFIG_SYS_HIGH_PERF
static inline void amsg_set_msg(AMSG_T *msg, u8 ty, u8 id, u16 p1, u32 p2)
{
    msg->ty = ty;
    msg->id = id;
    msg->p1 = p1;
    msg->p2 = p2;
}
#else
void amsg_set_msg(AMSG_T *msg, u8 ty, u8 id, u16 p1, u32 p2);
#endif

/* aipc_register_handler -This api is used to register a message handler.
 *
 * @aipc: the aipc instance.
 * @type: the protocol type.
 * @id: bearer id or module id, representing the receiver.
 * @handler: the message handler function. Details see the description of
 *           the struct AMSG_HANDLER_T.
 * @p: the data pointer of callback handler function
 *
 * @return: AIC_SUCCESS open ch succeeded; other values open ch failed.
 */
int aipc_register_handler(AIPC_T *aipc, u8 type, u8 id,
                          AMSG_HANDLER_T handler, void *p);

/**
* aipc_unregister_handler - unregister the message handler.
*
* @aipc: the aipc instance.
* @type: the protocol type.
* @id: bearer id or module id, representing the receiver.
*/
int aipc_unregister_handler(AIPC_T *aipc, u8 type, u8 id);

#endif /* __AIPC_INNER_H__ */

