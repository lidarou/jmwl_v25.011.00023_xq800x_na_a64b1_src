/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA configurations.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x048730
#define LOG_MODULE    AMOD_SYS

#include <aic_osa.h>
#include <aos_mem.h>
#include <aos_thread.h>
#include <aos_config.h>
#include <aic_assert.h>

#ifdef CONFIG_AIC_OSA_TEST
/* demo thread1 */
#define AOS_DEMO_THREAD1_ID          16
#define AOS_DEMO_THREAD1_PRI         16
#define AOS_DEMO_STACK1_SIZE         (1*1024)
#define AOS_DEMO_INTQ1_NUM           8
#define AOS_DEMO_EXTQ1_NUM           8
#define AOS_DEMO_THREAD1_NAME        "demo_thread1"
aos_status demo_thread1_init(void);
aos_status demo_thread1_main(aos_fsm_msg *fsm_msg_ptr);
aos_status demo_thread1_reset(void);
aos_status demo_thread1_delete(void);

/* demo thread1 */
#define AOS_DEMO_THREAD2_ID          17
#define AOS_DEMO_THREAD2_PRI         17
#define AOS_DEMO_STACK2_SIZE         (2*1024)
#define AOS_DEMO_INTQ2_NUM           16
#define AOS_DEMO_EXTQ2_NUM           16
#define AOS_DEMO_THREAD2_NAME        "demo_thread2"
aos_status demo_thread2_init(void);
aos_status demo_thread2_main(aos_fsm_msg *fsm_msg_ptr);
aos_status demo_thread2_reset(void);
aos_status demo_thread2_delete(void);
#endif

#ifdef CONFIG_AIC_THREAD_TBL
/* standard thread config table */
static const aos_thread_config g_thread_config_tbl[] =
{
    #ifdef CONFIG_AIC_OSA_TEST
    {   AOS_DEMO_THREAD1_ID,
        AOS_DEMO_THREAD1_PRI,
        AOS_DEMO_STACK1_SIZE,
        AOS_DEMO_INTQ1_NUM,
        AOS_DEMO_EXTQ1_NUM,
        AOS_DEMO_THREAD1_NAME,
        {demo_thread1_init,demo_thread1_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,demo_thread1_reset,demo_thread1_delete
        #endif
        }
    },

    {   AOS_DEMO_THREAD2_ID, 
        AOS_DEMO_THREAD2_PRI,
        AOS_DEMO_STACK2_SIZE,
        AOS_DEMO_INTQ2_NUM,
        AOS_DEMO_EXTQ2_NUM,
        AOS_DEMO_THREAD2_NAME,
        {demo_thread2_init,demo_thread2_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,demo_thread2_reset,demo_thread2_delete
        #endif
        }
    },
    #endif

    {   AOS_FALSE,
        AOS_STANDARD_THREADS,
        0,
        0,
        0,
        0,
        "",
        {AOS_NULL, AOS_NULL
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,AOS_NULL, AOS_NULL
        #endif
        }
    }
};
#endif

/* pool description table, max cluster 255 */
/* the total size must be less than HEAP_REGION_SIZE in sys_mem_config.h */
/* pool 0 for common */
#ifndef CONFIG_MEM_MODE_TLSF
/* pool 0 for common */
aos_mem_desc_tbl g_aos_common_mem[] =
{
    {8,          50},
    {16,         100},
    {32,         130},
    {64,         250},
    {128,        50},
    {256,        50},
    {512,        15},
    {1024,       27},
    {2048,       26},
    {4096,       12},
    {0,          0}
};
#else
aos_mem_desc_tbl g_aos_common_mem[] =
{
    {0,          0}
};
#endif

/* aos pool table */
aos_pool aos_pool_tbl[AOS_MAX_POOLS] =
{
    {&g_aos_common_mem[0],      {0, 0, AOS_NULL, AOS_NULL},
    AOS_COMMON_POOL_ID,         4,         0},
};

/**
* aos_init_standard_thread() - create aos standard threads, which are described
* in g_thread_config_tbl in aos_config.c .
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_init_standard_thread(void)
{
#ifdef CONFIG_AIC_THREAD_TBL
    u32 ret = AOS_ERROR;
    const aos_thread_config *thread_config_ptr = g_thread_config_tbl;

    while (thread_config_ptr->thread_id < AOS_STANDARD_THREADS) {
        ret = aos_create_standard_thread(thread_config_ptr->thread_id,
                                         thread_config_ptr->thread_name,
                                         aos_standard_thread_entry,
                                         (void *)&thread_config_ptr->thread_id,
                                         thread_config_ptr->stack_size,
                                         thread_config_ptr->intq_num,
                                         NULL,
                                         thread_config_ptr->extq_num,
                                         NULL,
                                         thread_config_ptr->thread_pri,
                                         AOS_AUTO_START);
        if (ret) {
            AIC_ASSERT(ret);
            return AOS_ERROR;
        }

        thread_config_ptr++;
    }
#endif
    return AOS_SUCCESS;
}

u32 get_thread_config_tbl_idx(aos_thread_id thread_id)
{
#ifdef CONFIG_AIC_THREAD_TBL
    u32 index;

    for (index = 0;
         index < sizeof(g_thread_config_tbl)/sizeof(aos_thread_config);
         index++) {
        if (g_thread_config_tbl[index].thread_id == thread_id)
            return index;
    }
#endif

    return AOS_ERROR;
}

