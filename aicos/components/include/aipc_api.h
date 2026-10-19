/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aipc module-related data and api.
 *    AIPC is a protocol for passing messages and data between two systems
 *    over shared memory. It mainly includes two parts: the message
 *    protocol(AMSG) and the data protocol(ABLK) now. New protocols can be
 *    extended in the future.
 *
 *    If a system that needs to communicate with multiple system, it needs to
 *    create multiple instances of AIPC. Because the source system is the system
 *    executing the code itself, it can be hidden, so we use the remote system
 *    represents the AIPC instance(local system<--->remote system).
 */
#ifndef __AIPC_API_H__
#define __AIPC_API_H__

#include <aic_type.h>
#include <aic_sys.h>

#if defined (CONFIG_SUBSYS_AP)
#define AIC_DST_SYS    AIC_SYS_CP
#define AIC_SRC_SYS    AIC_SYS_AP

#elif defined (CONFIG_SUBSYS_CP)
#define AIC_DST_SYS    AIC_SYS_AP
#define AIC_SRC_SYS    AIC_SYS_CP
#else
#define AIC_SRC_SYS    AIC_SYS_ADSP
#endif

#define AIPC_MSG_ASSERT 0xFEFE

/* for modem temp, will remove it. */
#define AIPC_SYS_AP AIC_SYS_AP
#define AIPC_SYS_CP AIC_SYS_CP

#if defined(WIN32) || defined(CONFIG_SUBSYS_AP)
#define AIPC_ASSERT(n)  AIC_ASSERT(n)
#else
#define AIPC_ASSERT(n) do { \
        if (!(n)) { \
            sys_notify_ap_assert(); \
        } \
    } while (0)
#endif

/**
 * proto type: Now contains AMSG protocol and ABLK protocol two parts,
 * New protocols can be extended in the future.
 */
enum {
    AIPC_PROT_AMSG = 0,
    AIPC_PROT_ABLK,
    AIPC_PROT_NR
};

/**
 * share memory type:
 * @ASMEM_NORMAL_RAM: PSRAM or DDR, use for storage data block of brr,
 *                    will be configured to cacheable.
 * @ASMEM_INTERNAL_RAM: system cache or aon iram, use for storage control block
                        of brr, will be configured to uncacheable.
 *
 */
enum {
    ASMEM_NORMAL_RAM = 0,
    ASMEM_INTERNAL_RAM
};

#define NOT_A_ASMEM_ADDR    0xff

/**
 * AMSG struct define.
 * A local module can send a message to any module of the remote system,
 * but the premise condition is that the destination module must register
 * a receive function in remote system to receive the messages.
 *
 * All the module id defined in aic_module.h.
 * All the bearer defined in this file.
 */
typedef struct AMSG {
    /* the protocol type */
    u8    ty;
    /*
     * in AMSG protocol it's module id,
     * in ABLK protocol it's bearer id.
     */
    u8    id;
    /* parameter 1 */
    u16   p1;
    /* parameter 2 */
    u32   p2;
} AMSG_T;

/**
 * AMSG handler function define, It is the receive function mentioned above.
 * This function will be executed in the Interrupt upper half context, requiring
 * it to do only extremely simple things, can't call blocking apis and do things
 * that take time.
 */
typedef int (*AMSG_HANDLER_T)(AMSG_T *amsg, void *p);

/**
 * ABLK link state define and listen.
 * Call ablk_listen_a_link to listen a link.
 */
typedef enum link_state_tag {
    ALINK_STATE_UNLINKED = 0,
    ALINK_STATE_LINKED,
    ALINK_STATE_RECV_BLKS,
} link_state_t;

typedef int (*ALINK_LISTEN_PFN)(u8 linkid, link_state_t state);

/**
 * ABLK struct define. - The ABLK protocol transfers the address
 * and size of the data between the two systems, not transfers the data.
 *
 * @addr: the data address on share memory.
 * @size: When a module takes a block out of the pool, it is the size of the
 *        block, When the receiver gets the block, it is the size of the valid
 *        data.
 */
typedef struct ABLK {
    void        *addr;
    size_t      size;
} ABLK_T;

/**
 * LINK_INFO defines.
 * @fifo_depth: fifo depth
 * @fifo_size: the size of the fifo that has been used.
 */
typedef struct LINK_INFO {
    u16 fifo_depth;
    u16 fifo_size;
} LINK_INFO_T;


/**
 * BRR_MEM_INFO defines.
 * @free_cnt: the free blk cnt.
 * @total: the total blk cnt.
 */
typedef struct BRR_MEM_INFO {
    u16 free_cnt;
    u16 total;
} BRR_MEM_INFO_T;

/* ablk bearer handle type */
typedef void* ABRR_HANDLE;

/**
 * AIPC bearer defines.
 * The AIPC divides several bearers based on services,
 * Each bearer contains one or more links, the bearer
 * use different links transport different type data.
 */
enum {
    AIPC_BRR_COMM = 0,
    AIPC_BRR_AT,
    AIPC_BRR_IP,
    AIPC_BRR_CNT
};

#define AIPC_BRR_DEBUG  AIPC_BRR_COMM
#define AIPC_BRR_DIAG   AIPC_BRR_COMM

/**
 * The link id define for commom bearer.
 */
enum {
    COMM_LINK0 = 0,
    COMM_LINK1,
    COMM_LINK2,
    COMM_LINK3,
    COMM_LINK4,
    COMM_LINK5,
    COMM_LINK_CNT
};

#define ALTRACE_LINK    COMM_LINK0
#define ALMSG_LINK      COMM_LINK1
#define ALDATA_LINK     COMM_LINK2
#define STATUS_LINK     COMM_LINK3
#define ADIAG_LINK      COMM_LINK4
#define RMSG_LINK       COMM_LINK5

/**
 * The link id define for at bearer.
 */
enum {
    AT_LINK0 = 0,
    AT_LINK1,
    AT_LINK2,
    AT_LINK3,
    AT_LINK4,
    AT_LINK5,
    AT_LINK_CNT
};

#define CONNECT_BYPSS_AT_LINK   AT_LINK5

/**
 * The link id define for tcp/ip data bearer.
 */
enum {
    IP_LINK0 = 0,
    IP_LINK1,
    IP_LINK2,
    IP_LINK3,
    IP_LINK4,
    IP_LINK5,
    IP_LINK6,
    IP_LINK7,
    IP_LINK8,
    IP_LINK9,
    IP_LINK10,
    IP_LINK11,
    IP_LINK12,
    IP_LINK13,
    IP_LINK_CNT
};
#define IP_DATA_LINK0   IP_LINK0
#define IP_ACK_LINK0    IP_LINK1
#define IP_DATA_LINK1   IP_LINK2
#define IP_ACK_LINK1    IP_LINK3
#define IP_DATA_LINK2   IP_LINK4
#define IP_ACK_LINK2    IP_LINK5
#define IP_DATA_LINK3   IP_LINK6
#define IP_ACK_LINK3    IP_LINK7
#define IP_DATA_LINK4   IP_LINK8
#define IP_ACK_LINK4    IP_LINK9
#define IP_DATA_LINK5   IP_LINK10
#define IP_ACK_LINK5    IP_LINK11
#define IP_DATA_LINK6   IP_LINK12
#define IP_ACK_LINK6    IP_LINK13

/* wait forever option define */
#define AIPC_WAIT_FOREVER   (0xFFFFFFFF)
#define AIPC_NO_WAIT        (0)

/* one brr only support 32 link now for support ablk select function */
#define MAX_LINK_CNT    32

/******************************************************************************
 *                      AIPC API
 ******************************************************************************/
/**
 * aipc_alloc_smem - This api is used to alloc a share memory from
 *                         specified aipc.
 *
 * @sys: the remote system that communicates with the local system.
 * @type: the share memory type. (please see the share memory type define).
 * @size: the size of the requeset share memory.
 *
 * @return: NULL or memory addrress.
 */
addr_t aipc_alloc_smem(u8 sys, u8 type, size_t size);

/**
 * aipc_free_smem - This api is used to free share memory..
 *
 * @sys: the remote system that communicates with the local system.
 * @type: the share memory type. (please see the share memory type define).
 * @addr: the addr of the share memory.
 */
void aipc_free_smem(u8 sys, u8 type, addr_t addr);

/**
 * aipc_get_smem_type - This api is used to get a share memory type.
 *
 * @sys: the remote system that communicates with the local system.
 * @addr: the addr of the share memory.
 *
 * @return: if is not a share memory addres, return NOT_A_ASMEM_ADDR
 */
u8 aipc_get_smem_type(u8 sys, void *addr);
#define aipc_is_a_smem_addr(sys, addr) \
    return (NOT_A_ASMEM_ADDR != aipc_get_smem_type(sys, addr))

/*
 * aipc_data_alloc - alloc a noraml share memory for data block.
 */
#define aipc_data_alloc(sys, size) \
    aipc_alloc_smem((sys), ASMEM_NORMAL_RAM, (size))

/*
 * aipc_data_free - free a noraml share memory for data block.
 */
#define aipc_data_free(sys, addr) \
    aipc_free_smem((sys), ASMEM_NORMAL_RAM, (addr))

/*
 * aipc_ctrl_alloc - alloc a IRAM share memory  for control block.
 */
#define aipc_ctrl_alloc(sys, size) \
    aipc_alloc_smem((sys), ASMEM_INTERNAL_RAM, (size))

/*
 * aipc_ctrl_free - free a IRAM share memory  for control block.
 */
#define aipc_ctrl_free(sys, addr) \
    aipc_free_smem((sys), ASMEM_INTERNAL_RAM, (addr))


/******************************************************************************
 *                      AMSG API
 ******************************************************************************/
#ifdef CONFIG_SYS_HIGH_PERF
/**
 * amsg_fill_msg - This api is used to fill a message context.
 *
 * @msg: the message pointer.
 * @mod: the module number of receiver.
 * @p1: the parameter 1, size is 2 byte.
 * @p2: the parameter 2, size is 4 byte.
 */
static inline void amsg_fill_msg(AMSG_T *msg, u8 mod, u16 p1, u32 p2)
{
    msg->ty = AIPC_PROT_AMSG;
    msg->id = mod;
    msg->p1 = p1;
    msg->p2 = p2;
}
#else
/**
 * amsg_fill_msg - This api is used to fill a message context.
 *
 * @msg: the message pointer.
 * @mod: the module number of receiver.
 * @p1: the parameter 1, size is 2 byte.
 * @p2: the parameter 2, size is 4 byte.
 */
void amsg_fill_msg(AMSG_T *msg, u8 mod, u16 p1, u32 p2);
#endif

/**
 * amsg_send_msg - This api is used to send a message to specified system.
 *
 * @sys: the remote system that communicates with the local system.
 * @msg: the message pointer.
 * @timeout: the timeout time, the unit is milliseconds.
 * @return: AIC_SUCCESS or aic errno.
 */
int amsg_send_msg(u8 sys, AMSG_T *msg, u32 timeout);

/**
 * amsg_send_msg_nobackup - This api is similar to amsg_send_msg,
 * but if the aipc is not ready until timeout, will return AIC_ENODEV,
 *  dont't backup it to later send.
 *
 * @sys: the remote system that communicates with the local system.
 * @msg: the message pointer.
 * @timeout: the timeout time, the unit is milliseconds.

 * @return: AIC_SUCCESS or aic errno.
 */
int amsg_send_msg_nobackup(u8 sys, AMSG_T *msg, u32 timeout);

/**
 * amsg_raw_send_msg - This api is used to send a send a msg in irq timer or
                        assert context, It can't cause schadule.
 *
 * @sys: the remote system that communicates with the local system.
 * @msg: the message pointer.
 * @return: AIC_SUCCESS or aic errno.
 */
int amsg_raw_send_msg(u8 sys, AMSG_T *msg);

/**
 * amsg_register_mod_handler -  register a message handler for specified mod.
 *
 * @sys: the remote system that communicates with the local system.
 * @mod: the module number of receiver.
 * @handler: the message handler function. Details see the description of
 *           the struct AMSG_HANDLER_T.
 * @p: the data pointer of callback handler function
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int amsg_register_mod_handler(u8 sys, u8 mod, AMSG_HANDLER_T handler, void *p);

/**
 * amsg_unregister_mod_handler -  uregister a message handler for specified mod.
 *
 * @sys: the remote system that communicates with the local system.
 * @mod: the module number of receiver.
 *
 * @return: none.
 */
void amsg_unregister_mod_handler(u8 sys, u8 mod);

/******************************************************************************
 *                      ABLK API
 * This set of apis are potentially blocking apis that
 * must be used in the task context.
 *
 * Typical usage:
 *  sys1: ablk_get_block ==> ablk_send_block
 *  sys2: aipc_recv_block ==> aipc_finish_block
 ******************************************************************************/
/**
 * ablk_request_brr - Request the bearer handle by the specified bearer.
 * It only add a reference count of the bearer handle, don't create it.
 *
 * @sys: the remote system that communicates with the local system.
 * @brr_id: the bearer id.
 *
 * @return: NULL or the bearer handle.
 */
ABRR_HANDLE ablk_request_brr(u8 sys, u8 brr_id);

/**
 * ablk_release_brr - release a bearer by the specified bearer handle.
 * It only decrease a reference count of the bearer handle, don't delete it.
 *
 * @handle: the bearer handle.
 */
void ablk_release_brr(ABRR_HANDLE handle);

/**
 * ablk_set_loopback - switch ablk work mode: normal mode or loopback mode.
 * usually used for switch log output path betweem
 * local system and remote system.
 *
 * @handle: the bearer handle.
 * @loopback: TRUE - loopback mode, FALSE - normal mode.
 */
void ablk_set_loopback(ABRR_HANDLE handle, bool loopback);

/**
 * ablk_get_link_info - Get the link info size of the specified link id.
 *
 * @handle: the bearer handle.
 * @rx: TURE - is rx ablk, FALSE - is tx ablk.
 *
 * @return: Link cnt of the brr.
 */
int ablk_get_link_cnt(ABRR_HANDLE handle, bool rx);

/**
 * ablk_get_link_info - Get the link info size of the specified link id.
 *
 * @handle: the bearer handle.
 * @link_id: link id.
 * @rx: TURE - is rx ablk, FALSE - is tx ablk.
 * @info: output parameter, used to save the link info.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_get_link_info(ABRR_HANDLE handle, u8 link_id,
                       bool rx, LINK_INFO_T *info);

/**
 * ablk_has_full_link - if the brr has link full.
 *
 * @handle: the bearer handle.
 *
 * @return: TRUE or FALSE.
 */
bool ablk_has_full_link(ABRR_HANDLE handle);

/**
 * ablk_no_free_blk - if the brr no free blk.
 *
 * @handle: the bearer handle.
 *
 * @return: TRUE or FALSE.
 */
bool ablk_no_free_blk(ABRR_HANDLE handle);

/**
* ablk_get_brr_mem_info - Get the the mem used info for the request size.
*
* @handle: the bearer handle.
* @req_size:the request size.
*
* @return: AIC_SUCCESS or aic errno.
*/
int ablk_req_mem_info(ABRR_HANDLE handle, u16 req_size, BRR_MEM_INFO_T *info);

/**
 * ablk_get_max_pool_size - Get the max pool size of the specified bearer.
 *
 * @brr: the bearer handle.
 *
 * @return: the max pool size of the specified bearer.
 */
size_t ablk_get_max_pool_size(ABRR_HANDLE handle);

/**
 * ablk_get_min_pool_size - Get the min pool size of the specified bearer.
 *
 * @brr: the bearer handle.
 *
 * @return: the min pool size of the specified bearer.
 */
size_t ablk_get_min_pool_size(ABRR_HANDLE handle);

#ifdef CONFIG_SYS_HIGH_PERF
/**
* ablk_get_recv_block_count - Get the block count in the specified rx link fifo.
*
* @handle: the bearer handle.
* @link_id: link id.
*
* @return: the recevie block counts or aic errno.
*/
static inline int ablk_get_recv_block_count(ABRR_HANDLE handle, u8 link_id)
{
    LINK_INFO_T info;
    int ret;

    ret = ablk_get_link_info(handle, link_id, TRUE, &info);
    if (ret)
        return ret;

    return (info.fifo_size);
}

/**
* ablk_get_send_left_size - Get the left size in the specified tx link fifo.
*
* @handle: the bearer handle.
* @link_id: link id.
*
* @return: the the left size in the tx fifo or aic errno.
*/
static inline int ablk_get_send_left_size(ABRR_HANDLE handle, u8 link_id)
{
    LINK_INFO_T info;
    int ret;

    ret = ablk_get_link_info(handle, link_id, FALSE, &info);
    if (ret)
        return ret;

    return (info.fifo_depth - info.fifo_size);
}
#else
/**
* ablk_get_recv_block_count - Get the block count in the specified rx link fifo.
*
* @handle: the bearer handle.
* @link_id: link id.
*
* @return: the recevie block counts or aic errno.
*/
int ablk_get_recv_block_count(ABRR_HANDLE handle, u8 link_id);

/**
* ablk_get_send_left_size - Get the left size in the specified tx link fifo.
*
* @handle: the bearer handle.
* @link_id: link id.
*
* @return: the the left size in the tx fifo or aic errno.
*/
int ablk_get_send_left_size(ABRR_HANDLE handle, u8 link_id);
#endif

/**
 * ablk_create_link - Create a link for the specified link id.
 *
 * @handle: the bearer handle.
 * @link_id: link id.
 * @rx_depth: the depth of link rx fifo.
 * @tx_depth: the depth of link tx fifo.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_create_link(ABRR_HANDLE handle, u8 link_id,
                     u16 rx_depth, u16 tx_depth);

/**
 * ablk_destroy_link - Destroy a for the specified link id.
 *
 * @handle: the bearer handle.
 * @link_id: link id.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_destroy_link(ABRR_HANDLE handle, u8 link_id);

/**
 * ablk_listen_dynamic_ablk - listen a dynamic ablk.
 *
 * @handle: the bearer handle.
 * @pfn: the callback function to recieve the new state of a link.
 */
void ablk_listen_a_blk(ABRR_HANDLE handle, ALINK_LISTEN_PFN pfn);

/**
 * ablk_get_a_link_state - get the state of a link.
 *
 * @handle: the bearer handle.
 * @link_id: link id.
 *
 * @b_rx: TRUE - the blk get by ablk_recv_block api, is rx blk.
 *        FALSE - the blk get by ablk_get_block api, is tx blk.

 * @return: the state of the specified link.
 */
link_state_t ablk_get_a_link_state(ABRR_HANDLE handle, u8 link_id, bool b_rx);

/**
 * ablk_select - select some links of the specified bearer.
 * Only used to receive blocks now.
 *
 * @handle: the bearer handle.
 * @link_sets: the sets of link ids that you want to select.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: linksets, a collection of links with data.
 */
u32 ablk_select(ABRR_HANDLE handle, ulong link_sets, u32 timeout);

/**
 * ablk_get_blocks_ex - Get the specified number blocks from specified bearer.
 *
 * @handle: the bearer handle.
 * @size: the size of the request block.
 * @blks: the arry of the blocks.
 * @fix_count: FALSE, when the tx fifo is empty or get *cnt_ptr blks will return.
 *             TRUE,  will wait until get *cnt_ptr blks or timeout.
 *                    if timeout will get none blks.
 * @cnt_ptr: As input param, it save the max want get blks count.
 *           As output param, it save the real get blks count.
             if cnt_ptr is none, means only get one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_get_blocks_ex(ABRR_HANDLE handle, size_t size, ABLK_T *blk,
                       int *cnt_ptr, bool fix_count, u32 timeout);

/**
 * ablk_get_fix_blocks - Get the specified number blocks from specified bearer.
 *
 * @handle: the bearer handle.
 * @size: the size of the request block.
 * @blks: the arry of the blocks.
 * @cnt_ptr: As input param, it save the max want get blks count.
 *           As output param, it save the real get blks count.
             if cnt_ptr is none, means only get one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 * will wait until get *cnt_ptr blks or timeout. if timeout will get none blks.
 */
#define ablk_get_fix_blocks(handle, size, blk, cnt_ptr, timeout) \
        ablk_get_blocks_ex(handle, size, blk, cnt_ptr, TRUE, timeout)


/**
 * ablk_get_blocks - Get some blocks from specified bearer.
 *
 * @handle: the bearer handle.
 * @size: the size of the request block.
 * @blks: the arry of the blocks.
 * @cnt_ptr: As input param, it save the max want get blks count.
 *           As output param, it save the real get blks count.
             if cnt_ptr is none, means only get one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
#define ablk_get_blocks(handle, size, blk, cnt_ptr, timeout) \
        ablk_get_blocks_ex(handle, size, blk, cnt_ptr, FALSE, timeout)

/**
 * ablk_send_blocks - Send the specified number blocks by
 *                    specified link of the bearer.
 *
 * @handle: the bearer handle.
 * @link_id: the link id in the the bearer.
 * @blks: the arry of the blocks.
 * @count: the count of want send blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 *          Only send all blks return succ, otherwise will send none.
 */
int ablk_send_blocks(ABRR_HANDLE handle, u8 link_id, ABLK_T *blks,
                     int count, u32 timeout);

/**
 * ablk_recv_blocks_ex - Receive some blocks  from specified link.
 *
 * @handle: the bearer handle.
 * @link: the link id in the the bearer.
 * @blks: the arry of the blocks.
 * @fix_count: FALSE, the recv fifo is empty or recv *cnt_ptr blks will return.
 *             TRUE,  will wait until recv *cnt_ptr blks or timeout.
 *                    if timeout will recv none blks.
 * @cnt_ptr: As input param, it save the max blks count.
 *           As output param, it save the real recv blks count.
             if cnt_ptr is none, means only recv one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_recv_blocks_ex(ABRR_HANDLE handle, u8 link_id, ABLK_T *blks,
                        int *cnt_ptr, bool fix_count, u32 timeout);

/**
 * ablk_recv_fix_blocks - Receive the fix number blocks from specified link.
 *
 * @handle: the bearer handle.
 * @link: the link id in the the bearer.
 * @blks: the arry of the blocks.
 * @cnt_ptr: As input param, it save the max blks count.
 *           As output param, it save the real recv blks count.
             if cnt_ptr is none, means only recv one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 * only recv *cnt_ptr blks will return AIC_SUCCESS.
 */
#define ablk_recv_fix_blocks(handle, link, blks, cnt_ptr, timeout) \
        ablk_recv_blocks_ex(handle, link, blks, cnt_ptr, TRUE, timeout)

/**
 * ablk_recv_blocks - Receive some blocks from specified link.
 *
 * @handle: the bearer handle.
 * @link: the link id in the the bearer.
 * @blks: the arry of the blocks.
 * @cnt_ptr: As input param, it save the max blks count.
 *           As output param, it save the real recv blks count.
             if cnt_ptr is none, means only recv one blks.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 * As log as blk is recevied, will return succ, and the cnt_ptr
 * will save the actual number of received blks.
 */
#define ablk_recv_blocks(handle, link, blks, cnt_ptr, timeout) \
        ablk_recv_blocks_ex(handle, link, blks, cnt_ptr, FALSE, timeout)

/**
 * ablk_release_block - Release the specified number blocks and put them back
 * to block pool of specified bearer to finish the transfer of the block.
 * it only can be used after receive a block from remote system.
 *
 * @handle: the bearer handle.
 * @blks: the arry of the blocks.
 * @count: the count of the blocks.
 */
void ablk_release_blocks(ABRR_HANDLE handle, ABLK_T *blks, int count);

/**
 * ablk_recycle_blocks - recycle the specified number blocks and  put them back
 * to block pool.in local system. If you don't want to send the block to,
 *  the other side, please use it to recycle the block.
 *
 * @handle: the bearer handle.
 * @blks: the arry of the blocks.
 * @count: the count of the blocks.
 */
void ablk_recycle_blocks(ABRR_HANDLE handle, ABLK_T *blks, int count);

/**
 * ablk_get_block - Get a block memory from specified bearer.
 *
 * @handle: the bearer handle.
 * @size: the size of the request block.
 * @blk: output parameter, used to use to get the block.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_get_block(ABRR_HANDLE handle, size_t size, ABLK_T *blk, u32 timeout);

/**
 * ablk_send_block - Send a block by specified link of the bearer.
 *
 * @handle: the bearer handle.
 * @link_id: the link id in the the bearer.
 * @blk: the block pointer to be sent.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
#define ablk_send_block(handle, link, blk, timeout) \
        ablk_send_blocks(handle, link, blk, 1, timeout)

/**
 * ablk_recv_block - Receive a block from specified link.
 *
 * @handle: the bearer handle.
 * @link: the link id in the the bearer.
 * @blk: the block pointer.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
#define ablk_recv_block(handle, link, blk, timeout) \
        ablk_recv_blocks_ex(handle, link, blk, NULL, FALSE, timeout)

/**
 * ablk_release_block - Release a block memory and put it back to block
 * pool of specified bearer to finish the transfer of the block.
 * it only can be used after receive a block from remote system.
 *
 * @handle: the bearer handle.
 * @blk: the block pointer use to receive block.
 */
#define ablk_release_block(handle, blk) ablk_release_blocks(handle, blk, 1)

/**
 * ablk_recycle_block - Release a block memory and put it back to block pool.
 * in local system. If you don't want to send the block to the other side,
 * please use it to recycle the block.
 *
 * @handle: the bearer handle.
 * @blk: the block pointer.
 */
#define ablk_recycle_block(handle, blk) ablk_recycle_blocks(handle, blk, 1)

/**
 * ablk_release_block_ex - Release a block memory and put it back to block pool.
 * in local system. If you don't want to send the block to the other side,
 * please use it to recycle the block.
 *
 * @handle: the bearer handle.
 * @blk: the block pointer.
 * @line: line number, the line number is not saved again.
 *
 * this function is reserve for romcode call(can't be delete).
 */
void ablk_release_block_ex(ABRR_HANDLE handle, ABLK_T *blk, u16 line);

/**
 * ablk_recycle_block_ex - Release a block memory and put it back to block pool.
 * in local system. If you don't want to send the block to the other side,
 * please use it to recycle the block.
 *
 * @handle: the bearer handle.
 * @blk: the block pointer.
 * @line: line number, the line number is not saved again.
 *
 * this function is reserve for romcode call(can't be delete).
 */
void ablk_recycle_block_ex(ABRR_HANDLE handle, ABLK_T *blk, u16 line);

/**
 * ablk_rsv_blocks - Reserve some blocks in the link, will not change
 * all the blks state.
 *
 * @handle: the bearer handle.
 * @link: the link id in the the bearer.
 * @count: the count of the reserve blks.
 *
 * all the reserve blocks in the link must be get by call the function
 * ablk_get_rsv_blk_blocks, otherwise may get the wrong blks.
 */
void ablk_rsv_blocks(ABRR_HANDLE handle, u8 link_id, u8 count);
#define ablk_rsv_block(handle, link_id) \
        ablk_rsv_blocks(handle, link_id, 1)

/**
 * ablk_get_rsv_blocks - Get the reserve blocks in the link.
 *
 * @handle: the bearer handle.
 * @count: the count of the blks array.
 *
 * @return: the count of the get blks;
 */
u8 ablk_get_rsv_blocks(ABRR_HANDLE handle, u8 link_id, ABLK_T *blks, u8 count);
#define ablk_get_rsv_block(handle, link_id, blk) \
        ablk_get_rsv_blocks(handle, link_id, blk, 1)

/**
 * ablk_is_brr_blk - if the blk is the blk of the specified brr.
 *
 * @handle: the bearer handle.
 * @b_rx: TRUE - the blk get by ablk_recv_block api, is rx blk.
 *        FALSE - the blk get by ablk_get_block api, is tx blk.
 *
 * @return: TRUE or FALSE;
 */
bool ablk_is_brr_blk(ABRR_HANDLE handle, void *addr, bool b_rx);

/******************************************************************************
 *                     The ABUF API
 * The below ABUF api is a layer of encapsulation on top of ABLK. it reduced
 * the number of API calls for passing data, thus make the business layer
 * easier to use, but it add two copies duringdata transfer.
 * It is not recommended for scenarios with a large amount of data.
 * Typical usage:
 * sys1: abuf_write
 * sys2: abuf_read
 *******************************************************************************/
/**
 * abuf_request_brr - Request the bearer handle by the specified bearer.
 * It only add a reference count of the bearer handle, don't create it.
 *
 * @sys: type u8, the remote system that communicates with the local system.
 * @brr_id: type u8, the bearer id.
 *
 * @return: NULL or the bearer handle.
 */
#define abuf_request_brr(sys, brr_id)   ablk_request_brr(sys, brr_id);

/**
 * abuf_release_brr - release a bearer by the specified bearer handle.
 * It only decrease a reference count of the bearer handle, don't delete it.
 *
 * @handle: the bearer handle.
 */
#define abuf_release_brr(handle)   ablk_release_brr(handle);

/**
 * abuf_select - select some links of the specified bearer.
 * Only used to receive buffer data now.
 *
 * @handle: the bearer handle.
 * @link_sets: the sets of link ids that you want to select.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: linksets, a collection of links with data.
 */
#define abuf_select(handle, link_sets, timeout) \
    ablk_select(handle, link_sets, timeout)

/**
 * abuf_write -Send some data to remote system by specified bearer and link.
 *
 * @sys: the remote system that communicates with the local system.
 * @handle: the bearer handle.
 * @link: the link number in the the block bearer.
 * @buf: the pointer of the data which will send by specified link.
 * @size: the size of the data which will send by specified link.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: actual write size or aic errno.
 */
int abuf_write(ABRR_HANDLE handle, u8 link_id,
               u8 *buf, size_t size, u32 timeout);

/**
 * abuf_read -Read some data from specified bearer and link.
 *
 * @sys: the remote system that communicates with the local system.
 * @handle: the bearer handle.
 * @link: the link number in the the block bearer.
 * @buf: the pointer of the data which will send by specified link.
 * @size: the size of the data which will read by specified link.
 *        It must be big enough to read a the buffer once write, so
 *        it must >= the size that pass to abuf_write.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: actual read size or aic errno.
 */
int abuf_read(ABRR_HANDLE handle, u8 link_id,
              u8 *buf, size_t size, u32 timeout);

#ifdef CONFIG_ABLK_ENABLE_ANY_API
/******************************************************************************
 * xxx_any api - can be call in any context.
 ******************************************************************************/
int ablk_get_block_any(ABRR_HANDLE handle, size_t size, ABLK_T *blk, bool lock);
int ablk_recycle_block_any(ABRR_HANDLE handle, ABLK_T *blk, bool lock);
int ablk_send_block_any(ABRR_HANDLE handle, u8 link_id, ABLK_T *blk, bool lock);
int ablk_recv_block_any(ABRR_HANDLE handle, u8 link_id, ABLK_T *blk, bool lock);
int ablk_release_block_any(ABRR_HANDLE handle, ABLK_T *blk, bool lock);
#else
#define ablk_get_block_any(hd, sz, blk, lock)   ablk_get_block(hd, sz, blk, 0)
#define ablk_recycle_block_any(hd,blk, lock)    ablk_recycle_block(hd, blk)
#define ablk_send_block_any(hd, lk, blk, lock)  ablk_send_block(hd, lk, blk, 0)
#define ablk_recv_block_any(hd, lk, blk, lock)  ablk_recv_block(hd, lk, blk, 0)
#define ablk_release_block_any(hd, blk, lock)   ablk_release_block(hd, blk)
#endif

/******************************************************************************
 * aipc debug api
 ******************************************************************************/
void ablk_list_all_used_blk(ABRR_HANDLE handle, bool rx);

#endif /* __AIPC_API_H__ */

