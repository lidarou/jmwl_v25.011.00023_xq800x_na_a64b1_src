/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines test case of assert Interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0c7ddc
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <tx_api.h>
#include <aic_system_assert.h>

TX_THREAD assert_test_thread;

typedef void (*test_func_type)(void);

void assert_test_thread_entry(ULONG thread_input)
{
#if 1
    /* Normal assert test */
    AIC_ASSERT(0);
#endif

#if 0
    /* Data abort test */
    volatile u32 *temp_ptr = (volatile u32 *)(0x61000000);
    *temp_ptr |= 0x1;
#endif

#if 0
    /* Prefetch abort test */
    test_func_type test_func = (test_func_type)0x61000000;
    test_func();
#endif

#if 0
    /* Undefine ins test */
    test_func_type test_func = (test_func_type)(0x80001740);
    test_func();
#endif
}

int assert_test_init(void)
{
    CHAR *pointer = TX_NULL;

    aicx_info(0x16d1d6, "", "assert test init\n");

    /* Allocate the stack for assert test thread */
    pointer = aos_malloc(1024);
    if (!pointer) {
        aicx_error(0x1cdcbf, "", "assert test init failed: no memory!\n");
        return AIC_ENOMEM;
    }

    /* Create the assert test thread.  */
    tx_thread_create(&assert_test_thread, "assert test thread",
            assert_test_thread_entry, 0, pointer, 1024, 20, 20,
            TX_NO_TIME_SLICE, TX_AUTO_START);

    return 0;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(assert_test_init);

