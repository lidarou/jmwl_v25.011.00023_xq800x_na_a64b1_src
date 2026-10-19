/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is hte ABLOCK header file.
 */
#ifndef __AIPC_ABLK_H__
#define __AIPC_ABLK_H__

#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_osa.h>
#include <aipc_inner.h>

/* not local mem or not loopback, need flush cache, */
#define ABLK_NEED_FLUSH(type) (!((type) & (ABLK_LOCAL_MEM | ABLK_LOOPBACK)))

/**
 * the bearer state define:
 * default state is idle.
 */
#define ABLK_STATE_IDLE         0x00
#define ABLK_RECV_HEAD_ADDR     BIT_0
#define ABLK_STATE_TX_READY     BIT_1
#define ABLK_STATE_RX_READY     BIT_2
#define ABLK_STATE_TX_SUSPEND   BIT_3
#define ABLK_STATE_RX_SUSPEND   BIT_4

#define ABLK_STATE_ALL_TX \
    (ABLK_STATE_TX_READY | ABLK_RECV_HEAD_ADDR | ABLK_STATE_TX_SUSPEND)
#define ABLK_STATE_ALL_RX \
    (ABLK_STATE_RX_READY | ABLK_STATE_RX_SUSPEND | ABLK_RECV_HEAD_ADDR)

/**
 * the name of ablk format is rx/tx-%x-%02x, sys, brr id.
 * the name of link format is rx/tx-%x-%02x-%02x, sys, brr id, link id.
 */
#define ABLK_NAME_LEN       7
#define ABLK_LINK_NAME_LEN  10

/**
 * the ablk state define.
 * The Bit 31~Bit28 of the blk size use to save the blk state.
 * 0 is idle, can't change to other value.
 */
#define BLK_STATE_IDLE_INIT         0
#define BLK_STATE_IDLE_RESET        1
#define BLK_STATE_IDLE_EMPTY        2
#define BLK_STATE_IDLE_IRE_MAX      3

#define BLK_STATE_IDLE_RELEASE      4
#define BLK_STATE_IDLE_RECYCLE      5
#define BLK_STATE_IDLE_MAX          6

#define BLK_STATE_GET               10
#define BLK_STATE_SEND              11
#define BLK_STATE_RECV              12

#define ABLK_GET_STATE(x)        ((u32)(x) >> 28)
#define ABLK_GET_SIZE(x)         ((u32)(x) & 0x0FFFFFFF)
#define ABLK_SET_STATE(x, state) \
                       (x) = ((u32)(x) & 0x0FFFFFFF) | ((u32)(state) << 28)
#define ABLK_SET_SIZE(x, size) \
                      (x) = ((u32)(x) & 0xF0000000) | (u32)(size)
#define ABLK_SET_SIZE_STATE(x, size, state) \
                           (x) = ((u32)(size & 0x0FFFFFFF) | ((u32)(state) << 28))

#define ABLK_SET_IDLE_INIT(x)    ABLK_SET_STATE(x, BLK_STATE_IDLE_INIT)
#define ABLK_SET_IDLE_RESET(x)   ABLK_SET_STATE(x, BLK_STATE_IDLE_RESET)
#define ABLK_SET_IDLE_EMPTY(x)   ABLK_SET_STATE(x, BLK_STATE_IDLE_EMPTY)
#define ABLK_SET_IDLE_RELEASE(x) ABLK_SET_STATE(x, BLK_STATE_IDLE_RELEASE)
#define ABLK_SET_IDLE_RECYCLE(x) ABLK_SET_STATE(x, BLK_STATE_IDLE_RECYCLE)

#define ABLK_IS_IRE_IDLE(x)     (ABLK_GET_STATE(x) < BLK_STATE_IDLE_IRE_MAX)
#define ABLK_IS_IDLE(x)         (ABLK_GET_STATE(x) < BLK_STATE_IDLE_MAX)
#define ABLK_IS_EMPTY(x)        (ABLK_GET_STATE(x) == BLK_STATE_IDLE_EMPTY)
#define ABLK_IS_GET(x)          (ABLK_GET_STATE(x) == BLK_STATE_GET)
#define ABLK_IS_SEND(x)         (ABLK_GET_STATE(x) == BLK_STATE_SEND)
#define ABLK_IS_RECV(x)         (ABLK_GET_STATE(x) == BLK_STATE_RECV)
/* 
 * The free state is idle and no rsv.
 * Now is the high 8 bit is 0. define like this can improve perf.
 */
#define ABLK_IS_FREE(x)     (!(x & 0x00FFFFFF))

/**
 *              ablk share memory layout description:
 ****************************************************************
 *                     ABLK blocks area:
 ----------------------------------------------------------------
 * block0, ......, ......, blockm, ......, ......, ......, ......
 * ......, ......, ......, blockx, ......, ......, ......, ......
 * ......, ......, ......, blocky, ......, ......, ......, ......
 * ......, ......, ......, blockz, ......, ......, ......, blockn
 *---------------------------------------------------------------
 ****************************************************************
 *                     ABLK header area:
 *---------------------------------------------------------------
 *  | brrid |  wait flag  |  reserve  |  pools_cnt  |
 *---------------------------------------------------------------
 *  | links_cnt | blks_cnt | blks_base | links_base  |
 *---------------------------------------------------------------
 * pool cnt  arr: arr[0], arr[1], arr[2], ......, ......, arr[n]
 *---------------------------------------------------------------
 * free cnt  arr: arr[0], arr[1], arr[2], ......, ......, arr[n]
 *---------------------------------------------------------------
 * pool size  arr: arr[0], arr[1], arr[2], ......, ......, arr[n]
 *---------------------------------------------------------------
 * blks size  arr: arr[0], arr[1], arr[2], ......, ......, arr[n]
 *---------------------------------------------------------------
 ****************************************************************
 *                     ABLK link area:
 *---------------------------------------------------------------
 * link0_ofs | link1_ofs  | ....... | linkm_ofs  | linkn_ofs |
 *---------------------------------------------------------------
 * fifo_depth| fifo_depth | ....... | fifo_depth | fifo_depth |
 *---------------------------------------------------------------
 * link 0 :| rdpptr | wtptr |              fifo               |
 *---------------------------------------------------------------
 * link 1 :| rdpptr | wtptr |              fifo               |
 *---------------------------------------------------------------
 * .............................................................
 *---------------------------------------------------------------
 * link m :| rdpptr | wtptr |              fifo               |
 *---------------------------------------------------------------
 * link n :| rdpptr | wtptr |              fifo               |
 *---------------------------------------------------------------
 ****************************************************************
 */

/**
 * ABLK_HEAD - the description of ablk header.
 * It's a share memory struct, Used __packed prefix to make sure
 * that it have exactly the same structure on different systems, so
 * the structure of this head cannot be easily modified,
 * Ensure that the address of each member is an integer
 * multiple of its type size.
 *
 * @brr_id: the bearer id.
 * @reserve:not used now, reserve it.
 * @pools_cnt: the pool cnt of the ablk
 * @links_cnt: the link cnt of the ablk
 * @blks_cnt: the blks cnt of the ablk
 * @blks_base: the base address of the blks arear
 * @blks_size: the size of the blks arear
 * @links_base: the base address of the link arear
 *
 * the data just contains the fixed head, the variable part is
 * next to the fixed head. Its contains below parts:
 * size_t *pool_size_arr;
 * size_t *blks_size_arr;
 */
#ifdef WIN32
#pragma pack(push, 1)
typedef struct ABLK_HEAD {
        u8      brr_id;
        u8      pools_cnt;
        u8      links_cnt;
        u8      reserve;
        u16     blks_cnt;
        u16     link_size;
        u32     blks_size;
        u32     blks_base;
        u32     links_base;
    } ABLK_HEAD_T;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) ABLK_HEAD {
    u8      brr_id;
    u8      pools_cnt;
    u8      links_cnt;
    u8      reserve;
    u16     blks_cnt;
    u16     link_size;
    u32     blks_size;
    u32     blks_base;
    u32     links_base;
} ABLK_HEAD_T;
#endif /* WIN32 */

/**
 * ABLK_LINK - the description of ablk link header.
 * It's a share memory struct, Used __packed prefix to make sure
 * that it have exactly the same structure on different systems, so
 * the structure of this head cannot be easily modified,
 * Ensure that the address of each member is an integer
 * multiple of its type size.
 *
 * @rdptr: the read pointer of the link ring fifo.
 * @rwtptr: the writer pointer of the link ring fifo.
 *
 * the data just contains the fixed head, the variable part is
 * next to the fixed head. Its contains below parts:
 * @link 0 block fifo: the block fifo of link 0.
 * @link n block fifo: the block fifo of link n.
*/
#ifdef WIN32
#pragma pack(push, 1)
typedef struct ABLK_LINK_HEAD {
    u32 volatile rdptr;
    u32 volatile wtptr;
} ABLK_LINK_HEAD_T;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) ABLK_LINK_HEAD {
    u32 volatile rdptr;
    u32 volatile wtptr;
} ABLK_LINK_HEAD_T;
#endif /* WIN32 */

/**
 * ABLK_LINK - the description of ablk links.
 *
 * @head: the header pointer of the link.
 * @fifo: the fifo of the link.
 * @fifo_depth: the depth of the fifo.
 * @sem: the semaphore of the link.
 * @name: the name of the link.
 * @rsv_cnt: the rsv the block count(update the read ptr but not proc)
*/
typedef struct ABLK_LINK {
    ABLK_LINK_HEAD_T *head;
#if ABLK_LINK_HEAD_DEBUG
    ABLK_LINK_HEAD_T *debug_head;
#endif
    u16     *fifo;
    u16     fifo_depth;
    aos_semaphore_ptr   sem;
    char    name[ABLK_LINK_NAME_LEN + 1];
    u8      rsv_cnt;
} ABLK_LINK_T;

/**
 * The declaration of ABLK_BRR_T, See Its definition for details.
 */
typedef struct ABLK_BRR ABLK_BRR_T;

/**
 * ABLK_SGL - the description of a single ablk.
 *
 * @wait_free: wait free blk.
 * @pools_cnt: the pool cnt of the ablk.
 * @links_cnt: the link cnt of the ablk.
 * @links_max: the max link cnt of the ablk.
 * @blks_cnt: the block cnt of the ablk.
 * @blks_base: the base addres of the blocks area.
 * @blks_size: the size of the blocks area.
 * @head_base: the base addres of the head area.
 * @head_size: the size of the head area.
 * @links_base: the base addres of the link area.
 * @link_size: the size of the link area.
 * @pool_addr_arr: stored the end address of one pool.
 *                 it also is the first block adrress of the next pool.
 * @pool_size_arr: stored each pool size of the brr.
 * @free_used_arr: record the free and used state.
 * @blks_size_arr: stored the size of valid data in a block on share memory.
 * @pool_cnt_arr: stored each pool cnt of the brr.
 * @pool_alloc_arr: stored the last alloc blocks index in one pool.
 * @pool_number_arr: stored the number of first block in the pool
 * @mutex: the mutex of the ablk.
 * @sem: the samphore of the ablk.
 * @links: save all the links manager pointer.
*/
typedef struct ABLK_SGL {
    u8 volatile wait_free;
    u8      pools_cnt;
    u8      links_cnt;
    u8      links_max;
    u16     blks_cnt;

    u16     head_size;
    u16     link_size;
    size_t  blks_size;

    u32     blks_base;
    u32     head_base;
    u32     links_base;

    u32     *pool_addr_arr;
    size_t  *pool_size_arr;
    size_t  *blks_size_arr;

    u32 volatile   *free_used_arr;
    u16     *pool_cnt_arr;
    u16     *pool_alloc_arr;
    u16     *pool_number_arr;

    aos_mutex_ptr       mutex;
    aos_event_ptr       event;
    char                name[ABLK_NAME_LEN + 1];
    ABLK_LINK_T         **links;
} ABLK_SGL_T;

/**
 * ABLK_BRR - the bearer data struct of ablk.
 *
 * @sys: remote system.
 * @brr_id: the bearer id of the ablk.
 * @@type: ABLK type:loopback, multitask.
 * @alloc_range: The alloc range for memory allocation.
 * @ref_cnt: the reference cnt.
 * @waiting:  can't get free block, wait other module free some blocks.
 * @state: the state of the brr.
 * @max_links: save The maximum number of links that this BRR can create.
 * @mem_base: the memory base address of tx single ablk;
 * @mem_size: the memory size of tx single ablk;
 * @tx_ablk: the tx single ablk;
 * @rx_ablk: the rx single ablk;
 * @sema_dlink: dynamic link semphore.
 * @sema_dsuspend: dynamic suspend semphore.
 * @mutex: brr mutex.
 * @event: brr select event.
 * @thread: the pointer of ablk thread;
*/
struct ABLK_BRR {
    u8          sys;
    u8          brr_id;
    u8          type;
    u8          alloc_range;

    u8          ref_cnt;
    u8 volatile state;
    u8          reservd1;
    u8          reservd2;

    AIPC_T      *aipc;
    u32         data_base;
    size_t      data_size;
    u32         ctrl_base;
    size_t      ctrl_size;

    ABLK_SGL_T  *rx_ablk;
    ABLK_SGL_T  *tx_ablk;

    ALINK_LISTEN_PFN    link_pfn;
#if IS_CONF_ENABLED(CONFIG_ABLK_SYNC_CREATE_LINK)
    aos_semaphore_ptr   sema_dlink;
#endif
#if IS_CONF_ENABLED(CONFIG_ABLK_DYNAMIC_SUSPEND_LINK)
    aos_semaphore_ptr   sema_dsuspend;
#endif
    aos_mutex_ptr       mutex;
    aos_event_ptr       event;
};

/**
 * ablk_create - create a ablk instance according to
 * the specified configuration.
 *
 * @cfg: the specified configuration of the ablk.
 *
 * @return: the bearer instance or aic null.
 */
ABLK_BRR_T *ablk_create_brr(const ABLK_CFG_T *cfg);

/**
 * ablk_destroy - destroy a block instance.
 *
 * @brr: the pointer of the bearer instance.
 *
 * @return: AIC_SUCCESS or aic errno.
 */
int ablk_destroy_brr(ABLK_BRR_T *brr);

/**
 * ablk_reset_brr - reset a brr.
 *
 * @brr: the pointer of the ablk instance.
 */
void ablk_reset_brr(ABLK_BRR_T *brr);

#endif /* __AIPC_ABLK_H__ */

