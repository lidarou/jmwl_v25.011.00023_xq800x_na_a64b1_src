/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is entry of benchmarks.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x051cae
#define LOG_MODULE    AMOD_SYS

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <core_op.h>

#ifdef CONFIG_BCMARK_DHRYSTONE
extern void dhry_test(void);
#endif
aos_thread_id g_bcmark_thd_id;

void bcmark_main(void *param)
{
#ifdef CONFIG_BCMARK_DHRYSTONE
    dhry_test();
#endif
}
/* Create benchmark thread
 *
 * Note: As dhrystone get time by sys tick, and sys tick
 * can work only after interrupt enable(so that the sys tick handler an run).
 * So put dhrystone test after tx_application_define(which enable interrupt
 * at its end).
 *
 */

s32 create_benchmark_thread(void)
{
    g_bcmark_thd_id = aos_create_thread("bcmark_thd",
                                         bcmark_main,
                                         AOS_NULL,          /* entry param */
                                         1024,              /* stack size */
                                         0,                 /* ext queue size */
                                         AOS_NULL,          /* ext queue name */
                                         10,                /* priority */
                                         AOS_AUTO_START);   /* auto start */
    if(g_bcmark_thd_id != AOS_INVALID_THREAD_ID)
        return AIC_SUCCESS;
    else
        return AIC_EPERM;
}

#ifdef CONFIG_BENCHMARK
AIC_NORMAL_COMP_P1_INIT_EXPORT(create_benchmark_thread);
#endif

