/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file realize the system assert function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16f791
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_osa.h>
#include <aos_thread.h>
#include <aos_config.h>

u8 *test_byte_ptr = AOS_NULL;
u8 *test_blk_ptr = AOS_NULL;
aos_mutex_ptr test_mutex_ptr = AOS_NULL;
aos_timer_ptr test_timer_ptr = AOS_NULL;
#ifdef CONFIG_FSMMSG_FEATURE
aos_timer_ptr test_fsm_timer_ptr = AOS_NULL;
#endif
u32 timer_count = 0;

#define AOS_DEMO_THREAD1_ID          16
#define AOS_DEMO_THREAD2_ID          17

#define TEST_PARAM_DATA     "test param data:0x0123456789888777666555444333222\n."
#define TEST_PEER_DATA      "test peer data:0x98765432100011122233344455566677\n."

void test_expiration_func(void *input)
{
    aos_fsm_msg *msg_ptr;

    ++timer_count;

    aicx_info(0x09134d, "uu", "aos_test_timer: run times: %u, tickcount: %u !\n",
                timer_count, aos_get_tickcount());

    if (timer_count%2) {
        aicx_info(0x106263, "", "send msg to thread_1 !\n");
#ifdef CONFIG_FSMMSG_FEATURE
        msg_ptr = aos_get_fsm_msg(strlen(TEST_PARAM_DATA), strlen(TEST_PEER_DATA));
        AOS_ASSERT(msg_ptr);
        FSM_SRC_THREAD_ID(msg_ptr) = 0xE1;
        FSM_SRC_PROCESS_ID(msg_ptr) = 0xE2;
        FSM_DEST_THREAD_ID(msg_ptr) = AOS_DEMO_THREAD1_ID;
        FSM_DEST_PROCESS_ID(msg_ptr) = 0xE3;
        FSM_SAP_ID(msg_ptr) = 0x11;
        FSM_PRIMITIVE_ID(msg_ptr) = 0x22;
        FSM_PARAM_SIZE(msg_ptr) = strlen(TEST_PARAM_DATA);
        FSM_PEER_BUF_SIZE(msg_ptr) = strlen(TEST_PEER_DATA);
        memcpy(FSM_PARAM_PTR(msg_ptr), TEST_PARAM_DATA, strlen(TEST_PARAM_DATA));
        memcpy(FSM_PEER_BUF_PTR(msg_ptr), TEST_PEER_DATA, strlen(TEST_PEER_DATA));
        FSM_TIMESTAMP(msg_ptr) = aos_get_tickcount();
        aos_send_fsm_msg(msg_ptr);
#endif
    } else {
        if (aos_is_thread_created(AOS_DEMO_THREAD2_ID)) {
            aicx_info(0x150d0f, "", "send msg to thread_2 !\n");
#ifdef CONFIG_FSMMSG_FEATURE
            msg_ptr = aos_get_fsm_msg(sizeof(u32), 0);
            AOS_ASSERT(msg_ptr);
            FSM_SRC_THREAD_ID(msg_ptr) = 0xF1;
            FSM_SRC_PROCESS_ID(msg_ptr) = 0xF2;
            FSM_DEST_THREAD_ID(msg_ptr) = AOS_DEMO_THREAD2_ID;
            FSM_DEST_PROCESS_ID(msg_ptr) = 0xF3;
            aos_start_fsm_timer(test_fsm_timer_ptr, 3000, msg_ptr);
            aicx_info(0x025c94, "", "Start test fsm timer!\n");
#endif
        }
    }
}

void demo_thread3_entry(void *input)
{
    u32 i=0;

    aicx_info(0x1842e6, "", "demo_thread3_entry enter!\n");

    while (AOS_TRUE) {
        if (AOS_SUCCESS == aos_get_mutex(test_mutex_ptr, AOS_WAIT_FOREVER)) {
            aicx_info(0x0e0da0, "u", "demo_thread3: mutex get %u times!\n", ++i);

            /* get (512+2*cacheline_size) from block or byte pool */
            test_byte_ptr = aos_dma_malloc(512);
            if (!test_byte_ptr)
                AOS_ASSERT(AIC_FALSE);
            /* get 512 from block or byte pool */
            test_blk_ptr = aos_malloc(512);
            if (!test_blk_ptr)
                AOS_ASSERT(AIC_FALSE);
            aos_put_mutex(test_mutex_ptr);
            aicx_info(0x12e898, "", "demo_thread3: mutex put!\n");
            aos_sleep(6000);
        } else {
            aicx_error(0x076a30, "", "demo_thread3: get mutex fail!, exit!\n");
            return;
        }
    }
    return;
}

void demo_thread4_entry(void *input)
{
    u32 i=0;

    aicx_info(0x01c659, "", "demo_thread4_entry enter!\n");

    while (AOS_TRUE) {
        i++;
        if (i == 5) {
#ifdef AOS_CONFIG_TRACEX
            aos_dump_trace_buffer();
            aicx_info(0x04867c, "", "aos_dump_trace_buffer ###!\n");
#endif
            //aos_memory_dump();
            //alog_info("aos_memory_dump ###!\n");
            aos_pool_dump("../../../../out/win32/Debug/");
            aicx_info(0x142636, "", "aos_pool_dump ###!\n");
            aos_delete_thread(AOS_DEMO_THREAD2_ID);
#ifdef CONFIG_FSMMSG_FEATURE
            aos_stop_fsm_timer(test_fsm_timer_ptr);
#endif
            aicx_info(0x1b594c, "", "thread_2 deleted!!!\n");
        }
        if (AOS_SUCCESS == aos_get_mutex(test_mutex_ptr, AOS_WAIT_FOREVER)) {
            aicx_info(0x13e8d6, "u", "demo_thread4: mutex get %u times!\n", i);
            if (test_byte_ptr) {
                aos_dma_free(test_byte_ptr);
                test_byte_ptr = AOS_NULL;
            }
            if (test_blk_ptr) {
                aos_free(test_blk_ptr);
                test_blk_ptr = AOS_NULL;
            }
            aos_put_mutex(test_mutex_ptr);
            aicx_info(0x1d02cb, "", "demo_thread4: mutex put!\n");
            aos_sleep(3000);
        } else {
            aicx_info(0x0ef0db, "", "demo_thread4: get mutex fail!, exit!\n");
            return;
        }
    }
    return;
}

/* Define the test threads. */
aos_status demo_thread1_init(void)
{
    aos_thread_id demo_id = (aos_thread_id)AOS_DEMO_THREAD1_ID;

    aicx_info(0x0c8340, "", "demo_thread1_init enter!\n");

    test_mutex_ptr = aos_create_mutex("aos_test_mutex");
    if (!test_mutex_ptr) {
        aicx_error(0x1d72b9, "", "aos test mutex create fail!\n");
        return AOS_ERROR;
    } else
        aicx_info(0x00d8ee, "", "aostest mutex create success!\n");

    test_timer_ptr = aos_create_timer("aos_test_timer", test_expiration_func,
                     NULL, 2000, AOS_AUTO_ACTIVATE, AOS_TRUE);
    AOS_ASSERT(test_timer_ptr);
#ifdef CONFIG_FSMMSG_FEATURE
    test_fsm_timer_ptr = aos_create_fsm_timer("aos_test_fsm_timer", AOS_FALSE);
    AOS_ASSERT(test_fsm_timer_ptr);
#endif
    aos_create_thread("demo_thread3", demo_thread3_entry, AOS_NULL, 1024, 8,
                      "demo_thread3", 16, AOS_AUTO_START);
    aos_create_thread("demo_thread4", demo_thread4_entry, AOS_NULL, 1024, 16,
                      "demo_thread4", 17, AOS_AUTO_START);

    return AOS_SUCCESS;
}

aos_status demo_thread1_main(aos_fsm_msg *fsm_msg_ptr)
{
    aos_fsm_msg *msg_ptr = fsm_msg_ptr;
    aicx_info(0x053a4f, "XX", "demo_thread1_main: src thread id: 0x%X, dest thread id: 0x%X!\n",
                FSM_SRC_THREAD_ID(msg_ptr),
                FSM_DEST_THREAD_ID(msg_ptr),
                FSM_DEST_PROCESS_ID(msg_ptr));
#ifdef CONFIG_FSMMSG_FEATURE
    aos_free_fsm_msg(msg_ptr);
#endif
    return AOS_SUCCESS;
}

aos_status demo_thread2_init(void)
{
    aos_thread_id demo_id = (aos_thread_id)AOS_DEMO_THREAD2_ID;

    aicx_info(0x062b40, "", "demo_thread2_init enter!\n");
    return AOS_SUCCESS;
}

aos_status demo_thread2_main(aos_fsm_msg *fsm_msg_ptr)
{
    aos_fsm_msg *msg_ptr = fsm_msg_ptr;
    aicx_info(0x0f93d9, "XX", "demo_thread2_main: src thread id: 0x%X, dest thread id: 0x%X!\n",
                FSM_SRC_THREAD_ID(msg_ptr),
                FSM_DEST_THREAD_ID(msg_ptr));
#ifdef CONFIG_FSMMSG_FEATURE
    aos_stop_fsm_timer(test_fsm_timer_ptr);
#endif
    aicx_info(0x0d1c29, "", "demo_thread2_main: Stop test fsm timer!\n");
    return AOS_SUCCESS;
}

aos_status demo_thread1_reset(void)
{
    aicx_info(0x169459, "", "demo_thread1_reset enter!\n");
    return AOS_SUCCESS;
}

aos_status demo_thread1_delete(void)
{
    aicx_info(0x144976, "", "demo_thread1_delete enter!\n");
    return AOS_SUCCESS;
}

aos_status demo_thread2_reset(void)
{
    aicx_info(0x1bab06, "", "demo_thread2_reset enter!\n");
    return AOS_SUCCESS;
}

aos_status demo_thread2_delete(void)
{
    aicx_info(0x1d2fc2, "", "demo_thread2_delete enter!\n");
    return AOS_SUCCESS;
}

