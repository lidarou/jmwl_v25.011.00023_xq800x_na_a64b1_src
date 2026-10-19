/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA win32 dummy functions.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10b02f
#define LOG_MODULE    AMOD_SYS

#include <stdio.h>
#include <aic_osa.h>
#include <tx_port.h>

#define MAX_ASSERT_INFO_LEN 256
static uint8_t s_assert_cnt;
char s_assert_info[MAX_ASSERT_INFO_LEN];

boolean aic_sys_is_assert(void)
{
    return (boolean)s_assert_cnt;
}

void aic_assert_set_queue_full(void)
{

}

void aic_sys_assert(char *fmt, ...)
{
    va_list args;
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    while(s_assert_cnt);
    s_assert_cnt++;

    va_start(args, fmt);
    vsnprintf(s_assert_info, MAX_ASSERT_INFO_LEN - 1, fmt, args);
    va_end(args);

    aicx_info(0x090928, "s", "%s\n", s_assert_info);
    alog_flush_all_log_in_assert();
    //aos_memory_dump();
    aos_pool_dump("../../../../out/win32/Debug/");
    while(s_assert_cnt);
    TX_RESTORE
}

void printf_report(aos_thread_id thread_id, aos_process_id proc_id,
                u8 *str, u16 length, boolean force_critical,
                char *filename, u32 line)
{
/* temp delete for WIN32 PSIT timing
    AOS_ASSERT(str && filename);

    alog_info("printf_report:\n"
                "src_thread_id: %u, src_process_id: %u, "
                "str: %s, length: %u, "
                "force_critical: %u, filename: %s, "
                "line: %u\n",
                thread_id,
                proc_id,
                str,
                length,
                force_critical,
                filename,
                line);
*/
}

void printf_message(void *message)
{
/* temp delete for WIN32 PSIT timing
    aos_fsm_msg *msg_ptr = message;

    AOS_ASSERT(msg_ptr);

    alog_info("printf_message:\n"
                "src_thread_id: %u, src_process_id: %u, "
                "dest_thread_id: %u, dest_process_id: %u, "
                "sap_id: %u, prim_id: %u, "
                "timestamp: %u\n",
                msg_ptr->src_thread_id,
                msg_ptr->src_process_id,
                msg_ptr->dest_thread_id,
                msg_ptr->dest_process_id,
                msg_ptr->sap_id,
                msg_ptr->prim_id,
                msg_ptr->timestamp);
*/
}

void printf_data(u8 *data, size_t size)
{
    aicx_info(0x0c5bb8, "XX", "printf_data !\ndata address is 0x%X, size is 0x%X \n",
                data, size);
}

void assert_passive_handle(void)
{

}
