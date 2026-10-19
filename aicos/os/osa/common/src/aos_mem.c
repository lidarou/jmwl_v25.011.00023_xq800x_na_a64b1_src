/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA memory function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x045b45
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <aos_config.h>
#include <aos_mem.h>
#include <aic_assert.h>
#include <aic_math.h>
#include <mpu.h>

#ifdef CONFIG_CUSTOMER_MEM_LAYOUT
#include <sys_mem_api.h>
#endif
#ifdef WIN32
#include <drv_file.h>
#else
#include <cache.h>
#endif

#define aos_mem_get_tickcount()   aos_get_aon_raw_tick()

//#define AOS_DEBUG
#ifdef AOS_DEBUG
#define AOS_INTERNAL_TRACE  alog_info
#define AOS_INTERNAL_AIDXP  aicx_info

#define AOS_INTERNAL_FREE_NUM_CHECK() do {\
    AOS_ASSERT_INFO(cl_info[cl_index].free_num, \
                   "current pool(pool_id:%u, cl_index:%u) is used up!", \
                   pool_id, cl_index); \
    } while (0)
#else
#define AOS_INTERNAL_TRACE(fmt, ...)
#define AOS_INTERNAL_AIDXP(file, fmt, pf, ...)
#define AOS_INTERNAL_FREE_NUM_CHECK()
#endif

#ifdef CONFIG_MEM_CHECK_API
#include <aic_list.h>
typedef struct mem_check_item {
    struct list_head    node;
    void                *p;
    u32                 used_cnt;
} mem_check_item_t;

struct list_head g_check_list;
u32 g_check_cnt;
u32 g_check_peak;
#endif

#ifdef CONFIG_MEM_MODE_TLSF
tlsf_pool_t    g_aos_system_pool;
#else
TX_BYTE_POOL    g_aos_system_pool;
#endif
aos_block_peak_ar*  g_aos_block_peak[AOS_MAX_POOLS];
static boolean b_aos_check_error = AOS_TRUE;
static boolean b_aos_peak_info = AOS_TRUE;
static boolean b_aos_pool_init = AOS_FALSE;

/* dynamic memory */
aos_dynamic_mem_pool    aos_dynamic_pool;

//extern aos_pool aos_pool_tbl[AOS_MAX_POOLS];

static u32 block_peak_init()
{
    u32 pool_id;

    for (pool_id = 0; pool_id < AOS_MAX_POOLS; pool_id++) {
        g_aos_block_peak[pool_id] = (aos_block_peak_ar *)aos_malloc(
                                        aos_pool_tbl[pool_id].pool_info.cl_nums
                                        * sizeof(aos_block_peak));
        if (g_aos_block_peak[pool_id] == AOS_NULL) {
            AIC_ASSERT(AIC_FALSE);
        }

        memset(g_aos_block_peak[pool_id], 0,
                                       (aos_pool_tbl[pool_id].pool_info.cl_nums
                                       * sizeof(aos_block_peak)));
    }

    return AOS_SUCCESS;
}

static u16 peak_locate_buff_blk(u8 pool_id, u32 blk_size)
{
    u16 index;
    u16 cl_nums;

    index = aos_pool_tbl[pool_id].pool_info.size_map[blk_size
                                       / AOS_MEM_POOL_PAGE_SIZE];
    cl_nums = aos_pool_tbl[pool_id].pool_info.cl_nums;

    for (;index < cl_nums; index++) {
        if (aos_pool_tbl[pool_id].pool_info.cl_info[index].blk_size>= blk_size)
            break;
    }

    return index;
}

static u32 block_peak_statistics(u8 op_type, u8 exp_pool_id, u32 exp_blk_size,
                                 u8 act_pool_id, u16 act_clus_id)
{
    u16 expect_clus_id;
    u16 actual_clus_id;

    expect_clus_id = peak_locate_buff_blk(exp_pool_id,exp_blk_size);
    actual_clus_id = act_clus_id;

    switch(op_type) {
    case AOS_BLOCK_GET:
        (*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_used_num++;
        if ((*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_used_num
                > (*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_peak_num)
             (*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_peak_num =
                  (*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_used_num;

        (*g_aos_block_peak[act_pool_id])[actual_clus_id].act_used_num++;
        if ((*g_aos_block_peak[act_pool_id])[actual_clus_id].act_used_num
                   > (*g_aos_block_peak[act_pool_id])[act_clus_id].act_peak_num)
             (*g_aos_block_peak[act_pool_id])[actual_clus_id].act_peak_num =
                     (*g_aos_block_peak[act_pool_id])[act_clus_id].act_used_num;
        break;
    case AOS_BLOCK_FREE:
        (*g_aos_block_peak[exp_pool_id])[expect_clus_id].exp_used_num--;
        (*g_aos_block_peak[act_pool_id])[actual_clus_id].act_used_num--;
        break;
    default:
        AIC_ASSERT(AIC_FALSE);
        break;
    }

    return AOS_SUCCESS;
}

static u32 block_peak_exit()
{
    u32 pool_id;

    for (pool_id = 0; pool_id < AOS_MAX_POOLS; pool_id++) {
        aos_free((u8 *)g_aos_block_peak[pool_id]);
        g_aos_block_peak[pool_id] = AOS_NULL;
    }

    return AOS_SUCCESS;
}

static void init_pool_config(aos_pool_id pool_id)
{
    u32 align_size;
    aos_mem_desc_tbl *tbl_ptr = aos_pool_tbl[pool_id].pool_desc;
    aos_pool_info *pool_ptr = &aos_pool_tbl[pool_id].pool_info;

    /* user area start and end address should be MemAlignSize align
     * |-HeadSize--|__________user_____________|-HeadSize--|________user_____|*/

    /* calculate max block and cluster number */
    while (tbl_ptr->block_size != 0) {
        align_size = aos_pool_tbl[pool_id].align_size;

        /* block size aligned */
        tbl_ptr->block_size = (AOS_MEM_ALIGN(tbl_ptr->block_size, align_size));

        tbl_ptr->block_size += aos_pool_tbl[pool_id].reserve_head;

        pool_ptr->cl_nums++;

        /* cluster's block size should incremental in each block pool */
        AIC_ASSERT(tbl_ptr->block_size >= pool_ptr->max_size);

        /* record max size */
        pool_ptr->max_size = tbl_ptr->block_size;

        tbl_ptr++;
    }

    /* each block pool must have one cluster at least */
    AIC_ASSERT(pool_ptr->cl_nums != 0);
}

static u32 init_cl(aos_pool_id pool_id)
{
    u32 cl_index;
    u32 align_size, malloc_size, head_size;

    void                *temp_ptr;

    aos_pool_info       *pool_ptr = &aos_pool_tbl[pool_id].pool_info;
    aos_mem_desc_tbl    *tbl_ptr = aos_pool_tbl[pool_id].pool_desc;

    /* Note: relationship betwwen head_size, align_size and reserve_head
            *
            *   |__________|--sizeof(aos_block*)--|~~~reserve_Head~~~  |
            *                                                         /\
            *                                                          |
            *                                                       user_data
            *   |                                                              |
            *   |<-------------------head_size------------------------>|
            *   |                                                              |
            *   |<----------------align_size * n---------------------->|
        */

    /* initialize cluster info */
    for (cl_index = 0; cl_index < pool_ptr->cl_nums; cl_index++) {
        pool_ptr->cl_info[cl_index].blk_nums = tbl_ptr->block_nums;
        pool_ptr->cl_info[cl_index].blk_size = tbl_ptr->block_size;
        pool_ptr->cl_info[cl_index].free_num = tbl_ptr->block_nums;

        /* align head_size */
        align_size = aos_pool_tbl[pool_id].align_size;
        head_size = aos_pool_tbl[pool_id].reserve_head + sizeof(aos_block*);
        head_size = AOS_MEM_ALIGN(head_size, align_size);

        /* allocate memory for block header, and reserve 1 aos_block */
        malloc_size = sizeof(aos_block) * (tbl_ptr->block_nums + 1);
        temp_ptr = aos_malloc(malloc_size);
        if (temp_ptr == AOS_NULL)
            return AOS_NO_MEMORY;
        pool_ptr->cl_info[cl_index].blk_cont_head_ptr = (aos_block*)temp_ptr;
        pool_ptr->cl_info[cl_index].free_head_blk_ptr = (aos_block*)temp_ptr;
        pool_ptr->cl_info[cl_index].free_tail_blk_ptr =
                                   pool_ptr->cl_info[cl_index].free_head_blk_ptr
                                   + (tbl_ptr->block_nums - 1);

        /* allocate memory for block */
        malloc_size = (tbl_ptr->block_size + head_size
                     - aos_pool_tbl[pool_id].reserve_head) * tbl_ptr->block_nums
                     + align_size + head_size;

        temp_ptr = aos_malloc(malloc_size);
        if (temp_ptr == AOS_NULL)
            return AOS_NO_MEMORY;
        pool_ptr->cl_info[cl_index].blk_user_head_ptr = temp_ptr;

        tbl_ptr++;
    }

    return AOS_SUCCESS;
}

static void init_block(aos_pool_id pool_id)
{
    u32 cl_index, blk_index, align_size, head_size;
    /* block control header pointer */
    aos_block *pstBlock;
    /* block control header pointer in user block, and it point to real block
    control header */
    aos_block **ppstBlock;
    aos_pool_info *pool_ptr = &aos_pool_tbl[pool_id].pool_info;

    for (cl_index = 0; cl_index < pool_ptr->cl_nums; cl_index++) {
        align_size = aos_pool_tbl[pool_id].align_size;
        head_size = aos_pool_tbl[pool_id].reserve_head + sizeof(aos_block*);
        head_size = AOS_MEM_ALIGN(head_size, align_size);

        /* get first user block address */
        ppstBlock = (aos_block**)pool_ptr->cl_info[cl_index].blk_user_head_ptr;

        /* align block user area start address */
        ppstBlock = (aos_block**)AOS_MEM_ALIGN(((u32)ppstBlock) + head_size,
                                               align_size);
        ppstBlock = (aos_block**)(((u32)ppstBlock)
                    - aos_pool_tbl[pool_id].reserve_head - sizeof(aos_block*));

        /* get first block control header address */
        pstBlock = pool_ptr->cl_info[cl_index].blk_cont_head_ptr;

        /* map pstBlock and ppstBlock */
        for (blk_index = 0; blk_index < pool_ptr->cl_info[cl_index].blk_nums;
             blk_index++) {
            *ppstBlock = pstBlock;

            /* initialize real block control header */
            pstBlock->reserved = AOS_BLK_RESERVED_VALUE;
            pstBlock->pool_id = pool_id;
            pstBlock->cl_id = (u8)cl_index;
            pstBlock->used_cnt = 0;
            pstBlock->usr_data = (u8*)&ppstBlock[1];
            pstBlock->next_blk_ptr = pstBlock + 1;
            pstBlock->getfile = 0;
            pstBlock->linkfreefile = 0;
            pstBlock->getline = 0;
            pstBlock->linkfreeline = 0;
            pstBlock->real_size = pool_ptr->cl_info[cl_index].blk_size;
            pstBlock->systime = 0;

            /* update header pointer in user block list */
            ppstBlock = (aos_block**)(pstBlock->usr_data
                        + pool_ptr->cl_info[cl_index].blk_size + head_size);
            ppstBlock = (aos_block**)(((u32)ppstBlock)
                        - aos_pool_tbl[pool_id].reserve_head
                        - sizeof(aos_block*));

            /* update header pointer in control header list */
            pstBlock++;
        }
        pstBlock--;
        pstBlock->next_blk_ptr = AOS_NULL;

        /* initialize the last reserved control header */
        pstBlock++;
        *ppstBlock = pstBlock;
        pstBlock->reserved = AOS_BLK_RESERVED_VALUE;
        pstBlock->pool_id = pool_id;
        pstBlock->cl_id = (u8)cl_index;
        pstBlock->used_cnt = 0;
        pstBlock->usr_data = (u8*)&ppstBlock[1];
        pstBlock->next_blk_ptr = (aos_block*)AOS_NULL;
        pstBlock->getfile = 0;
        pstBlock->linkfreefile = 0;
        pstBlock->getline = 0;
        pstBlock->linkfreeline = 0;
        pstBlock->real_size = 0;
        pstBlock->systime = 0;
    }
}

static void init_block_map(aos_pool_id pool_id)
{
    u32 map_index,map_index_end;
    u8 current_cl_index;
    u32 need_size;
    aos_pool_info *pool_ptr = &aos_pool_tbl[pool_id].pool_info;
    aos_mem_desc_tbl *tbl_ptr = aos_pool_tbl[pool_id].pool_desc;

    current_cl_index = 0;
    map_index_end = pool_ptr->max_size / AOS_MEM_POOL_PAGE_SIZE;

    for (map_index = 0; map_index <= map_index_end; map_index++) {
        need_size = map_index * AOS_MEM_POOL_PAGE_SIZE;

        while (current_cl_index < pool_ptr->cl_nums) {
            if (tbl_ptr[current_cl_index].block_size >= need_size) {
                pool_ptr->size_map[map_index] = current_cl_index;
                break;
            }
            current_cl_index++;
        }
    }
}

static boolean block_check(aos_block *blk_ptr)
{
    aos_block **ppBlock;

    /* check the block header */
    ppBlock = (aos_block**)(blk_ptr->usr_data - sizeof(aos_block*));
    return (*ppBlock == blk_ptr);
}

static u32 init_block_pool(aos_pool_id pool_id)
{
    u32 ret;
    aos_pool_info *pool_ptr = &aos_pool_tbl[pool_id].pool_info;

    init_pool_config(pool_id);

    /* allocate cluster info */
    pool_ptr->cl_info = (aos_cl_info *)aos_malloc(sizeof(aos_cl_info)
                        * pool_ptr->cl_nums);
    AIC_ASSERT(pool_ptr->cl_info);

    /* allocate size_map */
    pool_ptr->size_map = (u8*)aos_malloc(
                            (pool_ptr->max_size / AOS_MEM_POOL_PAGE_SIZE) + 1);
    AIC_ASSERT(pool_ptr->size_map);

    /* initialize cluster info */
    ret = init_cl(pool_id);
    AIC_ASSERT(ret == AOS_SUCCESS);

    /* initialize block control info */
    init_block(pool_id);
    /* initialize block map table */
    init_block_map(pool_id);

    return AOS_SUCCESS;
}

/**
* aos_init_mem() - Create low-level byte pool.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means fail.
*/
u32 aos_init_mem(void)
{
    u32 ret;
    RG_DATA_T rg_heap;
    static boolean b_initial = AOS_FALSE;

    if (b_initial)
        return AOS_SUCCESS;

#ifdef CONFIG_CUSTOMER_MEM_LAYOUT
    sys_get_sysmem_region(MEM_HEAP_RG, &rg_heap);
#else
    rg_heap.base = (addr_t)AOS_MEM_BASE_ADDR;
    rg_heap.size = (size_t)AOS_MEM_TOTAL_SIZE;
#endif

#ifdef CONFIG_MEM_MODE_TLSF
    ret = aos_init_tlsf_mem(&g_aos_system_pool, rg_heap.base, rg_heap.size);
#else
#ifdef CONFIG_RTOS_THREADX
    ret = tx_byte_pool_create(&g_aos_system_pool, (CHAR*)"SYSTEM",
                        (VOID*)(rg_heap.base), (ULONG)rg_heap.size);
#endif
    if (ret != TX_SUCCESS)
        return AOS_ERROR;
#endif

    b_initial = AOS_TRUE;
    return AOS_SUCCESS;
}

/**
* aos_delete_mem() - Delete low-level byte pool.
*
* Return: void.
*/
static void aos_delete_mem(void)
{
#ifdef CONFIG_MEM_MODE_TLSF
    aos_deinit_tlsf_mem(g_aos_system_pool);
#else
#ifdef CONFIG_RTOS_THREADX
    tx_byte_pool_delete(&g_aos_system_pool);
#endif
#endif
}

/**
* aos_init_block_pools() - Initialize aos block pools.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means fail.
*/
u32 aos_init_block_pools(void)
{
    u32 ret;
    aos_pool_id id;

    for (id = 0; id < AOS_MAX_POOLS; id++) {
        aos_pool_tbl[id].pool_info.cl_nums = 0;
        aos_pool_tbl[id].pool_info.max_size = 0;
        aos_pool_tbl[id].pool_info.size_map = (u8*)AOS_NULL;
        aos_pool_tbl[id].pool_info.cl_info = (aos_cl_info*)AOS_NULL;
    }

    /* initialize block pool */
    for (id = 0; id < AOS_MAX_POOLS; id++) {
        AIC_ASSERT(aos_pool_tbl[id].pool_id == id);
        ret = init_block_pool(id);
        if (ret != AOS_SUCCESS)
            return ret;
    }

    ret = block_peak_init();
    if (ret != AOS_SUCCESS)
        return ret;

    b_aos_pool_init = AOS_TRUE;

    return AOS_SUCCESS;
}

/**
* aos_init_pools() - Initialize aos memory pool.
*
* Return: AOS_SUCCESS means success, AOS_ERROR means fail.
*/
u32 aos_init_pools(void)
{
    u32 ret = AOS_SUCCESS;

#ifdef CONFIG_MEM_CHECK_API
    list_head_init(&g_check_list);
#endif

    b_aos_check_error = AOS_TRUE;
    b_aos_peak_info = AOS_TRUE;

    /* initialize dynamic memory header */
    aos_dynamic_pool.block_head = AOS_NULL;
    aos_dynamic_pool.total_mem_size = 0;
    aos_dynamic_pool.max_mem_size = 0;
    aos_dynamic_pool.total_mem_num = 0;

    /* initialize low level byte pool */
    aos_init_mem();

    /* initialize block pool */
#ifdef CONFIG_MEM_BLOCK_POOL
    ret = aos_init_block_pools();
    if (ret != AOS_SUCCESS) {
        return ret;
    }
#endif

    return ret;
}

/**
* aos_pool_check() - Check specified memory overflow or not.
* @dest_ptr: Specified memory ptr to check
* @length: Specified memory length to check.
*
* Return: AOS_SUCCESS means success, others means fail.
*/
aos_status aos_pool_check(u8 *dest_ptr, u32 length)
{
    u8 pool_id,cl_id;
    u32 dest_position;
    aos_cl_info *cl_ptr;
    aos_block *block_ptr;
    u8 *cl_head_ptr, *cl_tail_ptr;

    for(pool_id = 0; pool_id < AOS_MAX_POOLS; pool_id++) {
        cl_ptr = aos_pool_tbl[pool_id].pool_info.cl_info;
        AOS_ASSERT(cl_ptr);

        for(cl_id = 0;
            cl_id < aos_pool_tbl[pool_id].pool_info.cl_nums;
            cl_id++, cl_ptr++) {
            cl_head_ptr = (u8 *)cl_ptr->blk_cont_head_ptr->usr_data;
            cl_tail_ptr =
            (u8 *)(cl_ptr->blk_cont_head_ptr[cl_ptr->blk_nums-1].usr_data +
            cl_ptr->blk_size);

            if((dest_ptr >= cl_head_ptr) && (dest_ptr <= cl_tail_ptr)) {

                dest_position = (((u32)dest_ptr) - ((u32)cl_head_ptr)) %
                                ((u32)(cl_ptr->blk_cont_head_ptr[1].usr_data -
                                cl_head_ptr));

                block_ptr = *(aos_block**)(((u32)dest_ptr) -
                                           dest_position - sizeof(aos_block *));

                if(block_ptr->used_cnt == 0) {
                    AOS_ASSERT(AOS_FALSE);
                    return AOS_ERROR;
                }

                if(dest_position + length > block_ptr->real_size) {
                    AOS_ASSERT(AOS_FALSE);
                    return AOS_ERROR;
                }
                return AOS_SUCCESS;
            }
        }
    }
    return AOS_SUCCESS;    /* not pool address, return SUCCESS */
}

/**
* aos_is_heap_ptr() - Check specified memory located in heap or not.
* @dest_ptr: Specified memory ptr to check
*
* Return: AOS_SUCCESS means success, others means fail.
*/
boolean aos_is_heap_ptr(u8 *dest_ptr)
{
    RG_DATA_T rg_heap;

#ifdef CONFIG_CUSTOMER_MEM_LAYOUT
    sys_get_sysmem_region(MEM_HEAP_RG, &rg_heap);
#else
    rg_heap.base = (addr_t)AOS_MEM_BASE_ADDR;
    rg_heap.size = (size_t)AOS_MEM_TOTAL_SIZE;
#endif

    if ((u32)dest_ptr > (u32)rg_heap.base &&
        (u32)dest_ptr < (u32)rg_heap.base + rg_heap.size)
        return AOS_TRUE;
    else
        return AOS_FALSE;
}

u32 aos_get_heap_size(void)
{
    RG_DATA_T rg_heap;

#ifdef CONFIG_CUSTOMER_MEM_LAYOUT
    sys_get_sysmem_region(MEM_HEAP_RG, &rg_heap);
#else
    rg_heap.size = (size_t)AOS_MEM_TOTAL_SIZE;
#endif

    return rg_heap.size;
}

u32 aos_get_heap_size_ex(aos_heap_size *heap_size)
{
    if (heap_size == NULL) {
        return AOS_ERROR;
    }

#ifdef CONFIG_MEM_MODE_TLSF
    return tlsf_mem_info_get(g_aos_system_pool, heap_size);
#else
    return AOS_ERROR;
#endif
}

u32 aos_get_heap_usage(u32 *total_size, u32 *used_size)
{
#ifdef CONFIG_MEM_MODE_TLSF
    return tlsf_mem_status_get(g_aos_system_pool, total_size, used_size);
#else
    return AOS_ERROR;
#endif
}

/**
* aos_malloc_internal_debug() - Allocate memory from byte pool directly.
* @size: memory size to alloc.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
static void *aos_malloc_internal_debug(u32 size, filename_t file_name, u32 line)
{
#if defined(CONFIG_RTOS_THREADX) && !defined(CONFIG_MEM_MODE_TLSF)
    UINT ret;
#endif

    u32 malloc_size;
    aos_dynamic_block *return_mem;
    aos_dynamic_lock_define

    AIC_ASSERT(size);

    AOS_INTERNAL_AIDXP(0x02f8ab, "xdd", "aos malloc byte debug, (0x%x, %d, %d)", file_name, line, size);

    /* calculate memory size to allocate, include 1 byte reserved as tail */
    malloc_size = sizeof(aos_dynamic_block) + size + 1;

#ifdef CONFIG_MEM_MODE_TLSF
    return_mem = tlsf_mem_alloc(g_aos_system_pool, malloc_size);
    if (!return_mem)
        return AOS_NULL;
#else
#ifdef CONFIG_RTOS_THREADX
    ret = tx_byte_allocate(&g_aos_system_pool, (void **)&return_mem,
                           malloc_size, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
        return AOS_NULL;
#endif
#endif

    /* initialize memory header */
    return_mem->reserved = AOS_BYTE_RESERVED_VALUE;
    return_mem->used_cnt = 1;
    return_mem->getfile = file_name;
    return_mem->linkfreefile = 0;
    return_mem->getline = line;
    return_mem->linkfreeline = 0;
    return_mem->real_size = size;
    return_mem->systime = aos_mem_get_tickcount();
    return_mem->offset = 0;
    return_mem->flag = 0xff;

    /* initialize memory tail */
    *(((u8*)return_mem) + sizeof(aos_dynamic_block) + size)
                        = AOS_BYTE_RESERVED_VALUE;

    /* enter critical section */
    aos_dynamic_lock_get();

    /* add into allocated memory list */
    if (aos_dynamic_pool.block_head == AOS_NULL) {
        aos_dynamic_pool.block_head = return_mem;
        return_mem->next_blk_ptr = AOS_NULL;
        return_mem->pre_blk_ptr = AOS_NULL;
    } else {
        aos_dynamic_pool.block_head->pre_blk_ptr = return_mem;
        return_mem->next_blk_ptr = aos_dynamic_pool.block_head;
        return_mem->pre_blk_ptr = AOS_NULL;
        aos_dynamic_pool.block_head = return_mem;
    }

    /* update allocated memory statistic */
    aos_dynamic_pool.total_mem_num++;
    aos_dynamic_pool.total_mem_size += size;
    if (aos_dynamic_pool.max_mem_size < aos_dynamic_pool.total_mem_size)
        aos_dynamic_pool.max_mem_size = aos_dynamic_pool.total_mem_size;

    /* exit critical section */
    aos_dynamic_lock_put();

    /* skip memory header, return actual user memory address */
    return (void*)&return_mem[1];
}

/**
* aos_malloc_debug() - Allocate memory from block pool or byte pool.
* @size: memory size to alloc.
* @zero: if init the memory to 0.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_malloc_debug(u32 size, bool zero, filename_t file_name, u32 line)
{
    void * mem_ptr = AOS_NULL;

#ifdef CONFIG_MEM_BLOCK_POOL
    /* 1. try to malloc from block pool first */
    mem_ptr = aos_get_mem_debug(AOS_COMMON_POOL_ID, size, AOS_FALSE,
                                file_name, line);
#endif
    /* 2. try to malloc from byte pool */
    if(!mem_ptr)
        mem_ptr = aos_malloc_internal_debug(size, file_name, line);

    AOS_ASSERT(mem_ptr);

    /* zero is true, memset to 0. */
    if (zero)
        memset(mem_ptr, 0, size);

    return mem_ptr;
}

/**
* aos_malloc_prefer_bytepool() - Allocate memory from byte pool preferentially.
* It is more fit for alloc in init [hase] and never release.
* @size: memory size to alloc.
* @zero: if init the memory to 0.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
* @b_strict: must get available memory, or it will assert.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_malloc_prefer_bytepool(u32 size, bool zero, filename_t file_name, u32 line, boolean b_strict)
{
    void * mem_ptr = AOS_NULL;

    /* 1. try to malloc from byte pool */
    mem_ptr = aos_malloc_internal_debug(size, file_name, line);

#ifdef CONFIG_MEM_BLOCK_POOL
    /* 2. try to malloc from block pool first */
    if(!mem_ptr)
        mem_ptr = aos_get_mem_debug(AOS_COMMON_POOL_ID, size, AOS_FALSE,
                                    file_name, line);
#endif

    if (!mem_ptr) {
        if (b_strict)
            AOS_ASSERT(0);
        else
            return AOS_NULL;
    }

    /* zero is true, memset to 0. */
    if (zero)
        memset(mem_ptr, 0, size);

    return mem_ptr;
}

static void *do_aos_dma_malloc_debug(u32 alloc_size, u32 align, filename_t file_name, u32 line)
{
    void *mem_ptr, *dma_mem_ptr;
    u8 *temp, offset;

#ifdef CONFIG_MEM_MODE_TLSF
    mem_ptr = aos_malloc_internal_debug(alloc_size, file_name, line);
#else
    mem_ptr = aos_malloc_debug(alloc_size, AOS_FALSE, file_name, line);
#endif
    if (!mem_ptr)
        return AOS_NULL;

    /* get the cacheline-aligned address */
    dma_mem_ptr = (void *)ALIGN((u32)mem_ptr, align);
    /* save offset from mem_ptr to act_mem_ptr */
    offset = (u32)dma_mem_ptr - (u32)mem_ptr;
    if (offset < 2) {    /* reseve space to save offset */
        dma_mem_ptr = (void *)((u32)dma_mem_ptr + align);
        offset = align;
    }
    /* save align offset */
    temp = (u8 *)dma_mem_ptr - 1;
    *temp = 0xff;
    /* save align flag */
    temp = (u8 *)dma_mem_ptr - 2;
    *temp = offset;

    AOS_INTERNAL_AIDXP(0x0eb969, "XXx", "[DMA Malloc] cacheline size: 0x%X, actual malloc address: 0x%X, total malloc size: 0x%x\n",
                        cacheline_size, mem_ptr, alloc_size);
    AOS_INTERNAL_AIDXP(0x1a4d15, "XXx", "[DMA Malloc] offset: 0x%X, return dma address: 0x%X, return size: 0x%x\n",
                        offset, dma_mem_ptr, alloc_size);
    /* return the cacheline-aligned address */
    return dma_mem_ptr;
}

void *aos_dma64_malloc_debug(u32 size, filename_t file_name, u32 line)
{
    u32 alloc_size, cacheline_size;

#ifdef WIN32
    cacheline_size = 32;
#else
    cacheline_size = get_dcache_linesize();
#endif
    /* first, round up size to cacheline_size */
    alloc_size = ALIGN(size, cacheline_size);
    /* to makesure allocated memory and actual size are both cacheline-aligned,
       we allocate (size + 64 + 2) totally, 2 for save offset/flag */
    alloc_size = alloc_size + 64 + 2;

    return do_aos_dma_malloc_debug(alloc_size, 64, file_name, line);
}

/**
* aos_dma_malloc_debug() - Allocate memory from block pool or byte pool, and
*                          makesure the memory is cacheline-aligned
* @size: memory size to alloc.
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Valid memory ptr means success, AOS_NULL means fail.
*/
void *aos_dma_malloc_debug(u32 size, filename_t file_name, u32 line)
{
    u32 alloc_size, cacheline_size;

#ifdef WIN32
    cacheline_size = 32;
#else
    cacheline_size = get_dcache_linesize();
#endif
    /* first, round up size to cacheline_size */
    alloc_size = ALIGN(size, cacheline_size);
    /* to makesure allocated memory and actual size are both cacheline-aligned,
       we allocate (size + cacheline_size + 2) totally, 2 for save offset/flag */
    alloc_size = alloc_size + cacheline_size + 2;

    return do_aos_dma_malloc_debug(alloc_size, cacheline_size, file_name, line);
}

/**
* aos_get_dynamic_real_ptr() - Get real pointer for aos_dynamic_block.
*
* @mem_ptr: memory to free
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
static u8 * aos_get_dynamic_real_ptr(u8 *mem_ptr)
{
    u8 *real_ptr, offset;

    /* get the offset from mem_ptr to dma_mem_ptr */
    offset = *(mem_ptr - 2);
    /* get the actual memory allocated from heap */
    real_ptr = mem_ptr - offset;

    return real_ptr;
}

/**
* aos_free_internal_debug() - Free mem allocated from byte pool directly.
*                             only called by aos_free_debug
* @mem_ptr: memory to free
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
static u32 aos_free_internal_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    u32 ret;
    aos_dynamic_block *blk_head;
    aos_dynamic_lock_define

    blk_head = (aos_dynamic_block*)(mem_ptr - sizeof(aos_dynamic_block));

    /* enter critical section, and delete from list */
    aos_dynamic_lock_get();

    /* check blk has not be free and check reserve flag. */
    AOS_ASSERT(blk_head->used_cnt >= 1 &&
               blk_head->reserved == AOS_BYTE_RESERVED_VALUE);

    blk_head->used_cnt--;
    AOS_INTERNAL_AIDXP(0x18d33d, "xdd", "aos free byte debug, (0x%x, %d, %d)", file_name, line, blk_head->used_cnt);

    /* record the latest file info. */
    blk_head->linkfreefile = file_name;
    blk_head->linkfreeline = (u16)line;

    /* do actually free */
    if (blk_head->used_cnt == 0) {
        if (aos_dynamic_pool.block_head == blk_head) {
            aos_dynamic_pool.block_head = blk_head->next_blk_ptr;
            if (aos_dynamic_pool.block_head != AOS_NULL)
                aos_dynamic_pool.block_head->pre_blk_ptr = AOS_NULL;
        }
        else {
            AOS_ASSERT(blk_head->pre_blk_ptr);
            blk_head->pre_blk_ptr->next_blk_ptr = blk_head->next_blk_ptr;
            if (blk_head->next_blk_ptr != AOS_NULL)
                blk_head->next_blk_ptr->pre_blk_ptr = blk_head->pre_blk_ptr;
        }

        /* after free, zero used_cnt and set freed mark. */
        blk_head->used_cnt = 0;
        blk_head->reserved = AOS_BLK_FREEED_MARK;

        /* update statistic */
        aos_dynamic_pool.total_mem_num--;
        aos_dynamic_pool.total_mem_size -= blk_head->real_size;
        /* free blk memory */
#ifdef CONFIG_MEM_MODE_TLSF
        ret = tlsf_mem_free(g_aos_system_pool, (void *)blk_head);
        AOS_ASSERT(ret == AOS_SUCCESS);
#else
#ifdef CONFIG_RTOS_THREADX
        ret = tx_byte_release((u8 *)blk_head);
        AOS_ASSERT(ret == AOS_SUCCESS);
#endif
#endif
    }

    /* exit critical section */
    aos_dynamic_lock_put();

    return AOS_SUCCESS;
}

/**
* aos_free_debug() - Free mem allocated from block pool or byte pool.
* @mem_ptr: memory to free
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_free_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    u32 ret = AOS_SUCCESS;

    AOS_ASSERT(mem_ptr);

#ifdef CONFIG_MEM_BLOCK_POOL
    /* 1. try to free memory allocated from block pool */
    {
        aos_block *blk_head;
        blk_head = *(aos_block**)(mem_ptr - sizeof(aos_block*));
        if (aos_is_heap_ptr((u8 *)blk_head) &&
            (blk_head->reserved == AOS_BLK_RESERVED_VALUE) &&
            blk_head->usr_data == mem_ptr) {
                ret = aos_free_mem_debug(mem_ptr, file_name, line);
            return ret;
        }
    }
#endif
    /* 2. try to free memory allocated from byte pool */
    {
        aos_dynamic_block *byte_head;
        u8 *real_ptr;
        real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
        byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));
        if (aos_is_heap_ptr((u8 *)byte_head) &&
             (*(real_ptr + byte_head->real_size) == AOS_BYTE_RESERVED_VALUE)){

            ret = aos_free_internal_debug(real_ptr, file_name, line);
        }
        else
            AOS_ASSERT(AOS_FALSE);
    }

    return ret;
}

/**
* aos_mem_get_size() - Get allocated memory size.
* @mem_ptr: memory pointer
*
* Return: 0 means error, else the memory size.
*/
u32 aos_mem_get_size(u8 *mem_ptr)
{
    u32 ret = 0;

    AOS_ASSERT(mem_ptr);

#ifdef CONFIG_MEM_BLOCK_POOL
    /* 1. try to free memory allocated from block pool */
    {
        aos_block *blk_head;
        blk_head = *(aos_block**)(mem_ptr - sizeof(aos_block*));
        if (aos_is_heap_ptr((u8 *)blk_head) &&
            (blk_head->reserved == AOS_BLK_RESERVED_VALUE) &&
            blk_head->usr_data == mem_ptr) {
                ret = blk_head->real_size;
            return ret;
        }
    }
#endif
    /* 2. try to free memory allocated from byte pool */
    {
        aos_dynamic_block *byte_head;
        u8 *real_ptr;
        real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
        byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));
        if (aos_is_heap_ptr((u8 *)byte_head) &&
             (*(real_ptr + byte_head->real_size) == AOS_BYTE_RESERVED_VALUE)){

            ret = byte_head->real_size - (mem_ptr - real_ptr);
        }
        else
            AOS_ASSERT(AOS_FALSE);
    }

    return ret;
}

/**
* aos_dma_free_debug() - Free dma mem allocated from block pool or byte pool.
* @mem_ptr: memory to free
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_ERROR means error, AOS_SUCCESS means success.
*/
u32 aos_dma_free_debug(u8 *dma_mem_ptr, filename_t file_name, u32 line)
{
    u8 *mem_ptr;

    mem_ptr = aos_get_dynamic_real_ptr(dma_mem_ptr);
    /* free memory */
    AOS_INTERNAL_AIDXP(0x116a67, "Xxx", "[DMA Free] dma address to free: 0x%X, actual address to free: 0x%x, offset: 0x%x\n",
                        dma_mem_ptr, mem_ptr, offset);
    aos_free_debug(mem_ptr, file_name, line);

    return AOS_SUCCESS;
}

#ifdef WIN32
#define DUMP_BUF_LENGTH 300
u8 dump_buf[DUMP_BUF_LENGTH];
#endif
/**
 * aos_pool_dump() - Dump aos pool information to file.
 */
void aos_pool_dump(char *path)
{
#ifdef WIN32
    char file_name[256];
    char *f_name;
    FILE *fp;
    u32 pool_id, cl_id, block_id, length, index = 0x0;
    aos_block *blk_ptr;
    aos_cl_info *cl_info;
    aos_block_peak block_peak;
    aos_dynamic_block *header;

    if (file_access(path, NORMAL_MODE)) {
        //file_mkdir(path);
        /* failed, use current work path. */
        path = "./";
    }

/* 1.collect block pool allocation info */
    f_name="block_pool_info.csv";
    snprintf(file_name, sizeof(file_name), "%s%s", path, f_name);

    fp = fopen(file_name, "wb");
    AOS_ASSERT(fp);

    aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
    length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                      "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n",
                      "pool_id","cl_id","blk_id","used_cnt","blk_addr",
                      "blk_size","real_size","getfile","getline",
                      "linkfile","linkline","systime");
    AOS_ASSERT(length < DUMP_BUF_LENGTH);
    fwrite(dump_buf, length, 1, fp);

    for (pool_id = 0; pool_id < AOS_MAX_POOLS; pool_id++) {
        cl_info = aos_pool_tbl[pool_id].pool_info.cl_info;
        AOS_ASSERT(cl_info);

        for (cl_id = 0;
             cl_id < aos_pool_tbl[pool_id].pool_info.cl_nums;
             cl_id++) {

            blk_ptr = cl_info[cl_id].blk_cont_head_ptr;

            for (block_id = 0;
                 block_id < cl_info[cl_id].blk_nums;
                 block_id++) {
                    aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
                    length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                                     "%d,%d,%d,%d,0x%x,%d,%d,%s,%d,%s,%d,%d\r\n",
                                     blk_ptr->pool_id,
                                     blk_ptr->cl_id,
                                     block_id,
                                     blk_ptr->used_cnt,
                                     blk_ptr->usr_data,
                                     cl_info[cl_id].blk_size,
                                     blk_ptr->real_size,
                                     blk_ptr->getfile,
                                     blk_ptr->getline,
                                     blk_ptr->linkfreefile,
                                     blk_ptr->linkfreeline,
                                     blk_ptr->systime);
                    AOS_ASSERT(length < DUMP_BUF_LENGTH);
                    fwrite(dump_buf, length, 1, fp);
                    blk_ptr++;
            }
        }
    }
    fclose(fp);

/* 2.collect block pool peak info */
    f_name="block_pool_peak_info.csv";
    snprintf(file_name, sizeof(file_name), "%s%s", path, f_name);
    fp = fopen(file_name, "wb");
    AOS_ASSERT(fp);

    aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
    length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                      "%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n",
                      "pool_id","cl_id","blk_nums","blk_size","free_nums",
                      "exp_peak_num","exp_used_num","act_peak_num","act_used_num");
    AOS_ASSERT(length < DUMP_BUF_LENGTH);
    fwrite(dump_buf, length, 1, fp);

    for (pool_id = 0; pool_id < AOS_MAX_POOLS; pool_id++) {
        cl_info = aos_pool_tbl[pool_id].pool_info.cl_info;
        AOS_ASSERT(cl_info);

        for (cl_id = 0;
             cl_id < aos_pool_tbl[pool_id].pool_info.cl_nums;
             cl_id++) {
             block_peak = (*g_aos_block_peak[pool_id])[cl_id];
             aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
             length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                              "%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                              pool_id,
                              cl_id,
                              cl_info[cl_id].blk_nums,
                              cl_info[cl_id].blk_size,
                              cl_info[cl_id].free_num,
                              block_peak.exp_peak_num,
                              block_peak.exp_used_num,
                              block_peak.act_peak_num,
                              block_peak.act_used_num);
            AOS_ASSERT(length < DUMP_BUF_LENGTH);
            fwrite(dump_buf, length, 1, fp);
        }
    }
    fclose(fp);

/* 3.collect byte pool allocation info */
    f_name="byte_pool_info.csv";
    snprintf(file_name, sizeof(file_name), "%s%s", path, f_name);
    fp = fopen(file_name, "wb");
    AOS_ASSERT(fp);

    aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
    length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                      "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\r\n",
                      "total_mem_size","max_mem_size","total_mem_num",
                      "index","used_cnt","getfile","getline","freefile",
                      "freeline","real_size","systime");
    AOS_ASSERT(length < DUMP_BUF_LENGTH);
    fwrite(dump_buf, length, 1, fp);

    for (header = aos_dynamic_pool.block_head;
         header!=AOS_NULL;
         header=header->next_blk_ptr) {
         aos_memset(dump_buf, 0, DUMP_BUF_LENGTH);
         length = snprintf(dump_buf, DUMP_BUF_LENGTH,
                           "%d,%d,%d,%d,%d,%s,%d,%s,%d,%d,%d\r\n",
                           aos_dynamic_pool.total_mem_size,
                           aos_dynamic_pool.max_mem_size,
                           aos_dynamic_pool.total_mem_num,
                           index,
                           header->used_cnt,
                           header->getfile,
                           header->getline,
                           header->linkfreefile,
                           header->linkfreeline,
                           header->real_size,
                           header->systime);
         AOS_ASSERT(length < DUMP_BUF_LENGTH);
         fwrite(dump_buf, length, 1, fp);
         index++;
    }
    fclose(fp);

    aicx_info(0x130caf, "s", "aos_pool_dump finished, path: %s!\n", path);

#endif

#ifdef THREADX_SWITCH

#endif
}

/**
* aos_get_mem_debug() - Get mem block from specified block pool.
* @pool_id: id of the pool to allocate from.
* @size: memory size to allocate.
* @b_strict: must get available block, or it will assert
* @file_name: name of the file to allocate memory.
* @line: line number of the file to allocate memory.
*
* Return: Allocated memory ptr.
*/
void *aos_get_mem_debug(u32 pool_id, u32 size, bool b_strict,
                        filename_t file_name, u32 line)
{
    aos_pool_info*  pool_ptr;
    aos_cl_info*    cl_info;
    aos_block*      blk_ptr;
    u32             cl_nums;
    u8              cl_index;
    u8              serach_cnt;

    aos_pool_lock_define

    if(!b_aos_pool_init) {
        AOS_ASSERT(!b_strict);
        return (void*)AOS_NULL;
    }

    AIC_ASSERT(pool_id < AOS_MAX_POOLS);
    AIC_ASSERT(size);

    /* get pool information */
    pool_ptr = &aos_pool_tbl[pool_id].pool_info;
    cl_info = pool_ptr->cl_info;
    cl_nums = pool_ptr->cl_nums;

    if (size > pool_ptr->max_size) {
        AOS_ASSERT(!b_strict);
        return (void*)AOS_NULL;
    }

    blk_ptr = (aos_block*)AOS_NULL;
    /* calculate initial index */
    cl_index = pool_ptr->size_map[size / AOS_MEM_POOL_PAGE_SIZE];

    /* enter critical section */
    aos_pool_lock_get(pool_id);

    for (serach_cnt = 0; cl_index < cl_nums; cl_index++) {
        if (cl_info[cl_index].blk_size >= size)
        {
            if (serach_cnt++ == AOS_MEM_POOL_SERACH_CNT)
                break;

            AOS_INTERNAL_FREE_NUM_CHECK();

            if (cl_info[cl_index].free_num > 0) {
                /* suitable blk is found */
                blk_ptr = cl_info[cl_index].free_head_blk_ptr;
                cl_info[cl_index].free_num--;

                /* remove this blk from free blk list */
                cl_info[cl_index].free_head_blk_ptr =
                              cl_info[cl_index].free_head_blk_ptr->next_blk_ptr;
                if (cl_info[cl_index].free_head_blk_ptr == AOS_NULL)
                    cl_info[cl_index].free_tail_blk_ptr = AOS_NULL;

                if (b_aos_peak_info == AOS_TRUE)
                    block_peak_statistics(AOS_BLOCK_GET, pool_id, size,
                                          pool_id, cl_index);

                break;
            }
        }
    }

    if (!b_strict && !blk_ptr) {
        aos_pool_lock_put(pool_id);
        return (void*)AOS_NULL;
    }

    AOS_ASSERT(blk_ptr);

    if (b_aos_check_error == AOS_TRUE)
        AOS_ASSERT(block_check(blk_ptr));

    blk_ptr->used_cnt = 1;
    /* record allocate information */
    blk_ptr->getfile = file_name;
    blk_ptr->getline = (u16)line;
    blk_ptr->linkfreefile = 0;
    blk_ptr->linkfreeline = 0;
    blk_ptr->real_size = size;
    blk_ptr->systime = (u32)aos_mem_get_tickcount();

    aos_pool_lock_put(pool_id);

    return (void*)blk_ptr->usr_data;

}

/**
* aos_free_mem_debug() - Free mem block.
* @ptr: memory ptr to free.
* @file_name: name of the file to free memory.
* @line: line number of the file to free memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_free_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    aos_block* blk_ptr;
    aos_cl_info *cl_info_ptr;
    aos_pool_lock_define

    aos_pool_lock_get(blk_ptr->pool_id);

    AIC_ASSERT(mem_ptr);
    blk_ptr = *(aos_block**)(mem_ptr - sizeof(aos_block*));
    AIC_ASSERT(blk_ptr->used_cnt);

    if (b_aos_check_error == AOS_TRUE)
        AOS_ASSERT(block_check(blk_ptr));

    cl_info_ptr =
              &aos_pool_tbl[blk_ptr->pool_id].pool_info.cl_info[blk_ptr->cl_id];

    blk_ptr->used_cnt--;

    /* record the latest file info. */
    blk_ptr->linkfreefile = file_name;
    blk_ptr->linkfreeline = (u16)line;

    /* save free info */
    if (blk_ptr->used_cnt == 0) {
        cl_info_ptr->free_num++;

        /* put node into free list tail */
        if (cl_info_ptr->free_tail_blk_ptr == AOS_NULL) {
            cl_info_ptr->free_tail_blk_ptr = blk_ptr;
            cl_info_ptr->free_head_blk_ptr = blk_ptr;
        }
        else {
            cl_info_ptr->free_tail_blk_ptr->next_blk_ptr = blk_ptr;
            cl_info_ptr->free_tail_blk_ptr = blk_ptr;
        }

        blk_ptr->next_blk_ptr = AOS_NULL;

        if (b_aos_peak_info == AOS_TRUE)
            block_peak_statistics(AOS_BLOCK_FREE, blk_ptr->pool_id,
                                  blk_ptr->real_size, blk_ptr->pool_id,
                                  blk_ptr->cl_id);
    }

    aos_pool_lock_put(blk_ptr->pool_id);

    return AOS_SUCCESS;
}

/**
* aos_link_blk_mem_debug() - Link allocated mem block.
* @ptr: memory ptr to link.
* @file_name: name of the file to link memory.
* @line: line number of the file to link memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_link_blk_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    aos_block   *blk_ptr;
    aos_pool_lock_define

    aos_pool_lock_get(blk_ptr->pool_id);

    AIC_ASSERT(mem_ptr);
    blk_ptr = *(aos_block**)(mem_ptr - sizeof(aos_block*));

    /* check block error */
    if (b_aos_check_error == AOS_TRUE)
        AOS_ASSERT(block_check(blk_ptr));

    AIC_ASSERT(blk_ptr->used_cnt);

    AIC_ASSERT(blk_ptr->used_cnt < AOS_MAX_LINK_BUFF_TIMES);
    blk_ptr->used_cnt++;

    /* save link info */
    blk_ptr->linkfreefile = file_name;
    blk_ptr->linkfreeline = (u16)line;

    aos_pool_lock_put(blk_ptr->pool_id);

    return AOS_SUCCESS;
}

/**
* aos_link_byte_mem_debug() - Link allocated mem in byte poll.
* @ptr: memory ptr to link.
* @file_name: name of the file to link memory.
* @line: line number of the file to link memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_link_byte_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    aos_dynamic_block *blk_head;
    aos_dynamic_lock_define

    blk_head = (aos_dynamic_block*)(mem_ptr - sizeof(aos_dynamic_block));

    /* enter critical section, and delete from list */
    aos_dynamic_lock_get();

    /* check blk has not be free and check reserve flag. */
    AOS_ASSERT(blk_head->used_cnt &&
               blk_head->reserved == AOS_BYTE_RESERVED_VALUE);

    AIC_ASSERT(blk_head->used_cnt < AOS_MAX_LINK_BUFF_TIMES);
    blk_head->used_cnt++;

    /* save link info */
    blk_head->linkfreefile = file_name;
    blk_head->linkfreeline = (u16)line;

    /* exit critical section */
    aos_dynamic_lock_put();


    return AOS_SUCCESS;
}

/**
* aos_link_mem_debug() - Link allocated memory.
* @ptr: memory ptr to link.
* @file_name: name of the file to link memory.
* @line: line number of the file to link memory.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_link_mem_debug(u8 *mem_ptr, filename_t file_name, u32 line)
{
    u32 ret = AOS_SUCCESS;

    AOS_ASSERT(mem_ptr);

#ifdef CONFIG_MEM_BLOCK_POOL
    /* 1. try to free memory allocated from block pool */
    {
        aos_block *blk_head;
        blk_head = *(aos_block**)(mem_ptr - sizeof(aos_block*));
        if (aos_is_heap_ptr((u8 *)blk_head) &&
            (blk_head->reserved == AOS_BLK_RESERVED_VALUE) &&
            blk_head->usr_data == mem_ptr) {
                ret = aos_link_blk_mem_debug(mem_ptr, file_name, line);
            return ret;
        }
    }
#endif
#ifdef CONFIG_MEM_MODE_TLSF
    /* 2. try to free memory allocated from byte pool */
    {
        aos_dynamic_block *byte_head;
        u8 *real_ptr;
        real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
        byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));
        if (aos_is_heap_ptr((u8 *)byte_head) &&
             (*(real_ptr + byte_head->real_size) == AOS_BYTE_RESERVED_VALUE)){

            ret = aos_link_byte_mem_debug(real_ptr, file_name, line);
        }
        else
            AOS_ASSERT(AOS_FALSE);
    }
#endif
    return ret;
}

/**
* aos_delete_all_mem() - Free all allocated mem block.
*
* Return: AOS_SUCCESS means success, AOC_ERROR means error.
*/
u32 aos_delete_all_mem(void)
{
    aos_pool_id id;
    u32 cl_index;

    for (id = 0; id < AOS_MAX_POOLS; id++) {
        AOS_ASSERT(aos_pool_tbl[id].pool_info.cl_info);

        for (cl_index = 0; cl_index < aos_pool_tbl[id].pool_info.cl_nums;
            cl_index++) {
            /* free block control header and user header */
            aos_free((u8 *)(aos_pool_tbl[id].pool_info.cl_info[cl_index].blk_cont_head_ptr));
            aos_free((u8 *)(aos_pool_tbl[id].pool_info.cl_info[cl_index].blk_user_head_ptr));
        }

        /* free size map */
        aos_free((u8 *)(aos_pool_tbl[id].pool_info.size_map));

        /* free cluster info */
        aos_free((u8 *)(aos_pool_tbl[id].pool_info.cl_info));

        /* free pool info */
        aos_pool_tbl[id].pool_info.cl_nums = 0;
        aos_pool_tbl[id].pool_info.max_size = 0;
        aos_pool_tbl[id].pool_info.size_map = (u8 *)AOS_NULL;
        aos_pool_tbl[id].pool_info.cl_info = (aos_cl_info *)AOS_NULL;
    }

    block_peak_exit();

    /* note: if dynamic memory also allocated from the same low-level byte pool
       with OSA Block pool */
    aos_delete_mem();

    return AOS_SUCCESS;
}

#ifdef CONFIG_MEM_CHECK_API
static void *aos_malloc_internal_debug_check(u32 size, filename_t file_name, u32 line, void*check)
{
#if defined(CONFIG_RTOS_THREADX) && !defined(CONFIG_MEM_MODE_TLSF)
    UINT ret;
#endif

    u32 malloc_size;
    aos_dynamic_block *return_mem;
    aos_dynamic_lock_define

    AIC_ASSERT(size);

    //alog_info("aos malloc byte debug, (0x%x, %d, %d)", file_name, line, size);

    /* calculate memory size to allocate, include 1 byte reserved as tail */
    malloc_size = sizeof(aos_dynamic_block) + size + 1;

#ifdef CONFIG_MEM_MODE_TLSF
    return_mem = tlsf_mem_alloc(g_aos_system_pool, malloc_size);
    if (!return_mem)
        return AOS_NULL;
#else
#ifdef CONFIG_RTOS_THREADX
    ret = tx_byte_allocate(&g_aos_system_pool, (void **)&return_mem,
                           malloc_size, TX_NO_WAIT);
    if (ret != TX_SUCCESS)
        return AOS_NULL;
#endif
#endif

    /* initialize memory header */
    return_mem->reserved = AOS_BYTE_RESERVED_VALUE;
    return_mem->used_cnt = 1;
    return_mem->getfile = file_name;
    return_mem->linkfreefile = 0;
    return_mem->getline = line;
    return_mem->linkfreeline = 0;
    return_mem->real_size = size;
    return_mem->systime = (u32)check;
    return_mem->offset = 0;
    return_mem->flag = 0xff;

    /* initialize memory tail */
    *(((u8*)return_mem) + sizeof(aos_dynamic_block) + size)
                        = AOS_BYTE_RESERVED_VALUE;

    /* enter critical section */
    aos_dynamic_lock_get();

    /* add into allocated memory list */
    if (aos_dynamic_pool.block_head == AOS_NULL) {
        aos_dynamic_pool.block_head = return_mem;
        return_mem->next_blk_ptr = AOS_NULL;
        return_mem->pre_blk_ptr = AOS_NULL;
    } else {
        aos_dynamic_pool.block_head->pre_blk_ptr = return_mem;
        return_mem->next_blk_ptr = aos_dynamic_pool.block_head;
        return_mem->pre_blk_ptr = AOS_NULL;
        aos_dynamic_pool.block_head = return_mem;
    }

    /* update allocated memory statistic */
    aos_dynamic_pool.total_mem_num++;
    aos_dynamic_pool.total_mem_size += size;
    if (aos_dynamic_pool.max_mem_size < aos_dynamic_pool.total_mem_size)
        aos_dynamic_pool.max_mem_size = aos_dynamic_pool.total_mem_size;

    /* exit critical section */
    aos_dynamic_lock_put();

    /* skip memory header, return actual user memory address */
    return (void*)&return_mem[1];
}

static void *do_aos_dma_malloc_debug_check(u32 alloc_size, u32 align, filename_t file_name, u32 line, void *check)
{
    void *mem_ptr, *dma_mem_ptr;
    u8 *temp, offset;

    mem_ptr = aos_malloc_internal_debug_check(alloc_size, file_name, line, check);

    if (!mem_ptr)
        return AOS_NULL;

    /* get the cacheline-aligned address */
    dma_mem_ptr = (void *)ALIGN((u32)mem_ptr, align);
    /* save offset from mem_ptr to act_mem_ptr */
    offset = (u32)dma_mem_ptr - (u32)mem_ptr;
    if (offset < 2) {    /* reseve space to save offset */
        dma_mem_ptr = (void *)((u32)dma_mem_ptr + align);
        offset = align;
    }
    /* save align offset */
    temp = (u8 *)dma_mem_ptr - 1;
    *temp = 0xff;
    /* save align flag */
    temp = (u8 *)dma_mem_ptr - 2;
    *temp = offset;

    /* return the cacheline-aligned address */
    return dma_mem_ptr;
}

static void aos_check_mem_ptr(void *mem_ptr)
{
    aos_dynamic_block *byte_head;
    u8 *real_ptr;

    real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
    byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));

    AOS_ASSERT(byte_head->real_size > 0);
    AOS_ASSERT(aos_is_heap_ptr((u8 *)byte_head) &&
         (*(real_ptr + byte_head->real_size) == AOS_BYTE_RESERVED_VALUE));

    AOS_ASSERT(byte_head->reserved == AOS_BYTE_RESERVED_VALUE);
    AOS_ASSERT(byte_head->used_cnt > 0);
    AOS_ASSERT(byte_head->getline > 0);
    AOS_ASSERT(byte_head->getfile > 0);
    AOS_ASSERT(byte_head->flag == 0xff);
    AOS_ASSERT(byte_head->next_blk_ptr == NULL || aos_is_heap_ptr((u8 *)byte_head->next_blk_ptr));
    AOS_ASSERT(byte_head->pre_blk_ptr == NULL || aos_is_heap_ptr((u8 *)byte_head->pre_blk_ptr));
}

#ifdef CONFIG_CHECK_ALL_NODE
void aos_check_all_mem(void)
{
    struct list_head *head = &g_check_list;
    mem_check_item_t *item;
    int cnt = 0;

    aos_dynamic_lock_define

    aos_dynamic_lock_get();

    /* find the channel handler to process it. */
    list_for_each_entry(item, head, node, mem_check_item_t) {
        cnt++;
        aos_check_mem_ptr(item->p);
    }

    //alog_info("aos_check_all_mem, cnt=%d", cnt);
    aos_dynamic_lock_put();
}
#endif

void aos_check_part_mem(struct list_head *head, int cnt)
{
    mem_check_item_t *item;
    struct list_head *cur;
    int i = 0;

    aos_dynamic_lock_define

    aos_dynamic_lock_get();

    list_for_each(cur, head) {
        i++;
        if (i > cnt/2)
          break;
        if (cur == &g_check_list)
          continue;

        item = list_entry(cur, mem_check_item_t, node);
        aos_check_mem_ptr(item->p);
    }

    i= 0;
    list_for_each_prev(cur, head) {
        i++;
        if (i > cnt/2)
          break;
        if (cur == &g_check_list)
          continue;

        item = list_entry(cur, mem_check_item_t, node);
        aos_check_mem_ptr(item->p);
    }

    aos_dynamic_lock_put();
}

void *aos_check_malloc(u32 size, filename_t file_name, u32 line)
{
    void * mem_ptr = AOS_NULL;
    mem_check_item_t *item;
    aos_dynamic_lock_define

    item = (mem_check_item_t*)aos_get_mem_debug(AOS_COMMON_POOL_ID,
                                                  sizeof(*item),
                                                  AOS_TRUE,
                                                  file_name, line);
    AIC_ASSERT(item);

    mem_ptr = aos_malloc_internal_debug_check(size, file_name, line, item);
    AOS_ASSERT(mem_ptr);

    aos_dynamic_lock_get();
    item->used_cnt = 1;
    item->p = mem_ptr;
    list_add(&item->node, &g_check_list);
    g_check_cnt++;
    if (g_check_cnt > g_check_peak)
       g_check_peak = g_check_cnt;
    aos_dynamic_lock_put();

#ifdef CONFIG_CHECK_ALL_NODE
    aos_check_all_mem();
#else
    aos_check_part_mem(&item->node, CHECK_NODE_NUM);
#endif


    return mem_ptr;
}

void *aos_check_dma_malloc_debug(u32 size, filename_t file_name, u32 line)
{
    u32 alloc_size, cacheline_size;
    void * mem_ptr = AOS_NULL;
    mem_check_item_t *item;
    aos_dynamic_lock_define

    item = (mem_check_item_t*)aos_get_mem_debug(AOS_COMMON_POOL_ID,
                                                  sizeof(*item),
                                                  AOS_TRUE,
                                                  file_name, line);
    AIC_ASSERT(item);

#ifdef WIN32
    cacheline_size = 32;
#else
    cacheline_size = get_dcache_linesize();
#endif
    /* first, round up size to cacheline_size */
    alloc_size = ALIGN(size, cacheline_size);
    /* to makesure allocated memory and actual size are both cacheline-aligned,
       we allocate (size + cacheline_size + 2) totally, 2 for save offset/flag */
    alloc_size = alloc_size + cacheline_size + 2;

    mem_ptr = do_aos_dma_malloc_debug_check(alloc_size, cacheline_size, file_name, line, item);

    aos_dynamic_lock_get();
    item->used_cnt = 1;
    item->p = mem_ptr;
    list_add(&item->node, &g_check_list);
    g_check_cnt++;
    if (g_check_cnt > g_check_peak)
       g_check_peak = g_check_cnt;

    aos_dynamic_lock_put();

#ifdef CONFIG_CHECK_ALL_NODE
    aos_check_all_mem();
#else
    aos_check_part_mem(&item->node, CHECK_NODE_NUM);
#endif

    return mem_ptr;
}

u32 aos_check_link_mem(u8 *mem_ptr, filename_t file_name, u32 line)
{
    mem_check_item_t *item;
    u32 ret = 0;
    aos_dynamic_block *byte_head;
    u8 *real_ptr;

    aos_dynamic_lock_define

    aos_dynamic_lock_get();

    real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
    byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));
    item = (mem_check_item_t *)byte_head->systime;

#ifdef CONFIG_CHECK_ALL_NODE
    aos_check_all_mem();
#else
    aos_check_part_mem(&item->node, CHECK_NODE_NUM);
#endif

    item->used_cnt++;
    if (aos_is_heap_ptr((u8 *)byte_head) &&
       (*(real_ptr + byte_head->real_size) == AOS_BYTE_RESERVED_VALUE)){
    ret = aos_link_byte_mem_debug(real_ptr, file_name, line);
    }
    else
      AOS_ASSERT(AOS_FALSE);

    aos_dynamic_lock_put();

    return ret;
}

void aos_check_free(u8 *mem_ptr, filename_t file_name, u32 line)
{
    mem_check_item_t *item;
    aos_dynamic_block *byte_head;
    u8 *real_ptr;

    aos_dynamic_lock_define

    aos_dynamic_lock_get();
    real_ptr = aos_get_dynamic_real_ptr(mem_ptr);
    byte_head = (aos_dynamic_block*)(real_ptr - sizeof(aos_dynamic_block));
    item = (mem_check_item_t *)byte_head->systime;

#ifdef CONFIG_CHECK_ALL_NODE
    aos_check_all_mem();
#else
    aos_check_part_mem(&item->node, CHECK_NODE_NUM);
#endif

    AIC_ASSERT(item->used_cnt);
    item->used_cnt--;
    if (!item->used_cnt) {
        list_del(&item->node);
        aos_free_mem_debug((u8 *)item, file_name, line);
        g_check_cnt--;
    }
    aos_dynamic_lock_put();

    aos_free_debug(mem_ptr, file_name, line);
}
#endif /* CONFIG_MEM_CHECK_API */

