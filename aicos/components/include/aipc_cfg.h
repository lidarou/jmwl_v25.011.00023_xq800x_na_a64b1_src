
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the header file of the aic_aipc_cfg.c.
 */
#ifndef __AIC_ABLK_CFG_H__
#define __AIC_ABLK_CFG_H__
#include <aic_type.h>

/**
 * AIPC_SIMULATOR_IN_ONE_SYS - AIPC simulator feature.
 * If open the feature can simulator AIPC in one system.
*/
#ifdef WIN32
#define AIPC_SIMULATOR_IN_ONE_SYS   1
#else
#define AIPC_SIMULATOR_IN_ONE_SYS   0
#endif

/**
 * ABLK_FUNCTION_TEST - ABLK function test feature.
 * If open the feature will enable the ablk function test.
*/
#define ABLK_FUNCTION_TEST  0

/**
 * RMSG_FUNCTION_TEST - RMSG function test feature.
 * If open the feature will enable the RMSG function test.
*/
#define RMSG_FUNCTION_TEST  0

#ifdef CONFIG_AIPC_BLIK_BIG_SIZE
typedef unsigned int        aipc_size_t;
#else
typedef unsigned short      aipc_size_t;
#endif

/* amem mini order define. */
#define AMEM_4K_SHIFT    (12)
#define AMEM_1K_SHIFT    (10)
#define AMEM_256B_SHIFT  (8)
#define AMEM_64B_SHIFT   (6)
#define AMEM_16B_SHIFT   (4)

/**
 * link type define:
 * only support normal now.
*/
/* NORMAL: rx and tx and not mutitask. */
#define LINK_NORMAL     (0)

/**
 * ablk type define.
 * ABLK_LOOPBACK: the tx ablk and rx ablk in the same sys.
 * ABLK_LOCAL_MEM: use the local memory
 *                 (loopback can use both local memory and share memory).
 * ABLK_MULTITASK: link support multitask;
 * ABLK_DYNAMIC_LINK: link support create.
 * ABLK_RX_ONLY: like log brr, AP only can receive blk, can't be loopback,
 *               and do not need to configure block pool and link info.
 */
#define ABLK_LOOPBACK       BIT_0
#define ABLK_LOCAL_MEM      BIT_1
#define ABLK_MULTITASK      BIT_2
#define ABLK_DYNAMIC_LINK   BIT_3
#define ABLK_RX_ONLY        BIT_4
#define ABLK_TX_ONLY        BIT_5

/* NORMAL: multitask and not loopback. */
#define ABLK_NORMAL     (ABLK_MULTITASK)
/* LNORMAL: multitask and loopback. */
#define ABLK_LNORMAL    (ABLK_MULTITASK | ABLK_LOOPBACK)
/* SNORMAL: single task and not loopback. */
#define ABLK_SNORMAL    (0)
/* SLNORMAL: single task and loopback. */
#define ABLK_SLNORMAL    (ABLK_LOOPBACK)

/**
 * The alloc range for memory allocation.
 * If a pool of a nearest size fails to allocate,
 * the range of larger pools will be found.
 */
#define ABLK_DEFAULT_ALLOC_RANGE    0x02
#define ABLK_MININUM_ALLOC_RANGE    0x00
#define ABLK_MAXNUM_ALLOC_RANGE     0xff

/**
 * AMEM_INFO_T - sharemem arear data type
 *
 * @smem_base: the base of the sharemem arear.
 * @smem_size: the size of the sharemem arear.
 * @min_alloc: the min alloc order of he sharemem arear.
*/
typedef struct AMEM_AREAR {
    u32 smem_base;
    u32 smem_size;
    int min_alloc;
} AMEM_INFO_T;

/**
 * AIPC_CFG - The aipc cfg data type.
 *
 * @sys: remote system.
 * @is_subsys: If it is a subsys of the aipc.
 * @task_pri: the priority of the aipc task.
 * @fifo_depth: the size of the link ring fifo.
 * @normal_mem: the data sharemem of the aipc.
 * @internal_mem: the ctrl sharemem of the aipc.
 * @aipc_name: the name of the aipc.
 */
typedef struct AIPC_CFG {
    u8      sys;
    u8      is_subsys;
    u8      task_pri;
    //u8      pad;

    AMEM_INFO_T   normal_mem;
    AMEM_INFO_T   internal_mem;

    char          *aipc_name;
} AIPC_CFG_T;

/**
 * ABLK_LINK_CFG - contains all ablk link cfg of one block.
 *
 * @link: link id
 * @type: link type, it have 3 status: normal, rx only, tx only.
 * @fifo_depth: the size of the link ring fifo.
*/
typedef struct ABLK_LINK_CFG {
    u8  link_id;
    u8  type;
    u16 fifo_depth;
} ABLK_LINK_CFG_T;

/**
 * ablk_cfg - contans all configs of a ablk.
 *
 * @sys: remote system.
 * @brr_id: the bearer id.
 * @pools_cnt: the pool count of an ablk.
 * @links_cnt: the link count of an ablk.
 * @type: ABLK type:loopback, multitask.
 * @alloc_range: maximum range for memory allocation.
 * @cnt_arr: store the blocks cnt of each pool.
 * @size_arr: store the blocks size(must be multiple of 4) of each pool.
 * @link_cfg: all the links config data of the ABLK.
*/
typedef struct ABLK_CFG {
    char *name;
    u8  sys;
    u8  brr_id;
    u8  pools_cnt;
    u8  links_cnt;
    u8  type;
    u8  alloc_range;
    const u16               *cnt_arr;
    const aipc_size_t       *size_arr;
    const ABLK_LINK_CFG_T   *link_cfg;
}  ABLK_CFG_T;

/**
 * RMT_MSG_CFG
 *
 * @sys: remote system.
 * @brr_id: the bearer id.
 * @link_id: the link id.
 *
*/
typedef struct RMSG_CFG {
    u8  sys;
    u8  brr_id;
    u8  link_id;
    u8  rsv;
} RMSG_CFG_T;

const RMSG_CFG_T *aipc_get_rmsg_cfg(u8 *cnt_ptr);

#endif /* __AIC_ABLK_CFG_H__ */

