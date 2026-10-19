/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AOS memory Interface.
 */

#ifndef __AOS_MEM_H__
#define __AOS_MEM_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/

/* aos reserve flag */
#define AOS_BLK_RESERVED_VALUE          0xCF
#define AOS_BLK_FREEED_MARK             0xBB
#define AOS_BYTE_RESERVED_VALUE         0xFC

/* max link times */
#define AOS_MAX_LINK_BUFF_TIMES         250

/* OSA page size */
#define AOS_MEM_POOL_PAGE_SIZE          1024

/* OSA pool serach cnt */
#define AOS_MEM_POOL_SERACH_CNT         2

/* mem align */
#define AOS_MEM_ALIGN(size, agsize) ((((size) + (agsize - 1)) / agsize) * agsize)

#define AOS_BLOCK_GET                   0
#define AOS_BLOCK_LINK                  1
#define AOS_BLOCK_FREE                  2

/* mem mutex */
#ifdef CONFIG_RTOS_THREADX
#define aos_pool_lock_define            TX_INTERRUPT_SAVE_AREA
#define aos_pool_lock_get(pid)          TX_DISABLE
#define aos_pool_lock_put(pid)          TX_RESTORE
#define aos_dynamic_lock_define         TX_INTERRUPT_SAVE_AREA
#define aos_dynamic_lock_get()          TX_DISABLE
#define aos_dynamic_lock_put()          TX_RESTORE
#else
#define aos_pool_lock_define            AOS_INTERRUPT_DECLARE
#define aos_pool_lock_get(pid)          AOS_INTERRUPT_DISABLE
#define aos_pool_lock_put(pid)          AOS_INTERRUPT_RESTORE
#define aos_dynamic_lock_define         AOS_INTERRUPT_DECLARE
#define aos_dynamic_lock_get()          AOS_INTERRUPT_DISABLE
#define aos_dynamic_lock_put()          AOS_INTERRUPT_RESTORE
#endif

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/

typedef u8               aos_pool_id;

/* pool description */
typedef struct aos_mem_desc_tbl_st
{
    u32                  block_size;
    u32                  block_nums;
} aos_mem_desc_tbl;

/* block info */
typedef struct aos_block_st
{
    u8                   reserved;
    u8                   pool_id;
    u8                   cl_id;
    u8                   used_cnt;
    struct aos_block_st  *next_blk_ptr;
    filename_t           getfile;          /* file name to allocate memory */
    filename_t           linkfreefile;     /* file name to free memory
                                               if used_cnt is 0;
                                               file name to link memory
                                               if used_cnt > 0 */
    u16                  getline;           /* file line number to
                                               allocate memory */
    u16                  linkfreeline;      /* file line to free memory
                                               if used_cnt is 0;
                                               file line to link memory
                                               if used_cnt > 0 */
    u32                  real_size;         /* real size to allocate */
    u32                  systime;           /* sys time (ms) to
                                               allocate/free/link memory */
    u8                   *usr_data;         /* user data area */
} aos_block;

/* cluster info */
typedef struct aos_cl_info_st
{
    u32                  blk_nums;          /* block number in cluster */
    u32                  blk_size;          /* block size */

    aos_block*           blk_cont_head_ptr; /* block control header ptr */
    void*                blk_user_head_ptr; /* block user header ptr */

    u32                  free_num;          /* free block number */
    aos_block*           free_head_blk_ptr; /* free block list header ptr */
    aos_block*           free_tail_blk_ptr; /* free block list tail ptr */
} aos_cl_info;

/* pool info */
typedef struct aos_pool_info_st
{
    u32                  cl_nums;           /* cluster number */
    u32                  max_size;          /* max block size in cluster */
    u8*                  size_map;          /* size map */
    aos_cl_info          *cl_info;          /* cluster info address */
} aos_pool_info;

/* pool description */
typedef struct aos_pool_st
{
    aos_mem_desc_tbl     *pool_desc;
    aos_pool_info        pool_info;
    aos_pool_id          pool_id;           /* pool id, start from 0 */
    u32                  align_size;        /* align size of block address */
    u32                  reserve_head;      /* block header reserve size */
} aos_pool;

/* block peak statistic */
typedef struct aos_block_peak_st
{
    u32                  exp_peak_num;
    u32                  exp_used_num;
    u32                  act_peak_num;
    u32                  act_used_num;
} aos_block_peak;

typedef aos_block_peak   aos_block_peak_ar[];

/* dynamic block info */
typedef struct aos_dynamic_block_st
{
    u8                   reserved;          /* reserved */
    //u8                   pool_id;           /* no use */
    //u8                   cl_id;             /* no use */
    u8                   used_cnt;
    u16                  getline;           /* line number to allocate memory */

    struct aos_dynamic_block_st
                         *next_blk_ptr;     /* point to next block */
    struct aos_dynamic_block_st
                         *pre_blk_ptr;      /* point to pre block */

    filename_t           getfile;          /* file name to allocate memory */
    filename_t           linkfreefile;         /* file name to free memory
                                               if used_cnt is 0;
                                               file name to link memory
                                               if used_cnt > 0 */

    u32                  real_size;         /* real size to allocate */
    u32                  systime;           /* system time (ms) to
                                               allocate/free memory */
    u16                  linkfreeline;      /* file line to free memory
                                               if used_cnt is 0;
                                               file line to link memory
                                               if used_cnt > 0 */
    u8                   offset;             /* offset for dma memory align */
    u8                   flag;           /* flag to indicate align, always be 0xff */
} aos_dynamic_block;

/* dynamic mem pool info */
typedef struct aos_dynamic_mem_pool_st
{
    u32                  total_mem_size;
    u32                  max_mem_size;
    u32                  total_mem_num;
    aos_dynamic_block    *block_head;
} aos_dynamic_mem_pool;

#include <aos_config.h>
extern aos_pool aos_pool_tbl[AOS_MAX_POOLS];

typedef void* tlsf_pool_t;
u32 aos_init_tlsf_mem(tlsf_pool_t *pool, addr_t heap_base, size_t heap_size);
void *tlsf_mem_alloc(tlsf_pool_t pool, u32 size);
u32 tlsf_mem_free(tlsf_pool_t pool, void *ptr);
u32 tlsf_mem_info_get(tlsf_pool_t pool, aos_heap_size *info);
u32 tlsf_mem_status_get(tlsf_pool_t pool, u32 *total, u32 *used);
u32 aos_deinit_tlsf_mem(tlsf_pool_t pool);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AOS_MEM_H__ */

