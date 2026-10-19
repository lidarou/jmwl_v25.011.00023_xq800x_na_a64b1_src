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

#ifndef __AOS_CONFIG_H__
#define __AOS_CONFIG_H__

#include <aic_osa.h>

/* ---thread--- */
/* specified threads number */
#define AOS_STANDARD_THREADS            128
/* dynamic threads number */
#define AOS_DYNAMIC_THREADS             125
/* max threads number */
#define AOS_MAX_THREADS                 (AOS_STANDARD_THREADS + AOS_DYNAMIC_THREADS)
/* max thread priority */
#define AOS_MAX_PRIORITY                TX_MAX_PRIORITIES

/* ---tracex--- */
#ifdef TX_ENABLE_EVENT_TRACE
#define AOS_CONFIG_TRACEX
#define AOS_TRACE_BUFFER_SIZE           0x6000    /* 24KB buffer for tracex */
#endif

 /* ---stack check--- */
#ifdef TX_ENABLE_STACK_CHECKING
#define AOS_ENABLE_STACK_CHECKING
#endif

/* max value of AOS pools */
#define AOS_MAX_POOLS                   1
/* pool id */
#define AOS_COMMON_POOL_ID              0
#define AOS_POOL_DEFAULT                AOS_COMMON_POOL_ID

typedef struct aos_thread_config_st
{
    aos_thread_id       thread_id;
    u8                  thread_pri;
    u16                 stack_size;
    u16                 intq_num;    /* internal queue */
    u16                 extq_num;    /* external queue */
    char                *thread_name;
    struct{
        aos_func_init   task_init;
        aos_func_main   task_main;
#ifdef CONFIG_TASK_RESET_AND_DEL
        aos_func_reset  task_reset;
        aos_func_delete task_delete;
#endif
    }task_func;
} aos_thread_config;

#endif /* __AOS_CONFIG_H__ */

