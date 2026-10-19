/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA utl function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b0e27
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include <alog_api.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <aos_config.h>

#define AOS_MEMORY_DEBUG AOS_FALSE

#ifdef AOS_CONFIG_TRACEX
u32 trace_buffer[AOS_TRACE_BUFFER_SIZE/4];    /* 4byte align */
#endif

void aos_memcpy(void *dest_ptr, const void *src_ptr, u32 copy_count)
{
#if IS_CONF_ENABLED(AOS_MEMORY_DEBUG)
    if (dest_ptr == src_ptr)
        return;

    if (((u8 *)dest_ptr + copy_count > (u8 *)src_ptr) &&
        ((u8 *)src_ptr + copy_count > (u8 *)dest_ptr)) {
        AOS_ASSERT(AOS_FALSE);
        return;
    }

    if(copy_count && aos_is_heap_ptr((u8*)dest_ptr))
        aos_pool_check((u8*)dest_ptr, copy_count);
#endif

    memcpy(dest_ptr, src_ptr, copy_count);
}

void aos_memset(void *dest_ptr, u8 value, u32 count)
{
#if IS_CONF_ENABLED(AOS_MEMORY_DEBUG)
    if(count  && aos_is_heap_ptr((u8*)dest_ptr))
    {
        aos_pool_check((u8 *)dest_ptr, count);
    }
#endif

    memset(dest_ptr, (int)value, (size_t)count);
}

void aos_memmove(void *dest_ptr, const void *src_ptr, u32 copy_count)
{
#if IS_CONF_ENABLED(AOS_MEMORY_DEBUG)
    if(copy_count  && aos_is_heap_ptr((u8*)dest_ptr))
        aos_pool_check((u8*)dest_ptr, copy_count);
#endif

    memmove(dest_ptr, src_ptr, copy_count);
}

void aos_report(aos_thread_id thread_id, aos_process_id proc_id,
                u8 *str, u16 length, boolean force_critical,
                char *filename, u32 line)
{
    aos_fsm_msg *msg_ptr = AOS_NULL;

    msg_ptr = aos_get_fsm_msg((u16)(16 + length), 0);/* allocate msg ptr */
    FSM_SRC_THREAD_ID(msg_ptr) = thread_id;
    FSM_SRC_PROCESS_ID(msg_ptr) = proc_id;

    /*initialize parameters*/
    *((u8 *)(FSM_PARAM_PTR(msg_ptr))) = 0;
    *((u8 *)(FSM_PARAM_PTR(msg_ptr)) + 1) = 0x0e;/* set exception type */
    *((u8 *)(FSM_PARAM_PTR(msg_ptr)) + 2) = 0;
    *((u8 *)(FSM_PARAM_PTR(msg_ptr)) + 3) = 0;
    *((u8 *)(FSM_PARAM_PTR(msg_ptr)) + 4) = thread_id;
    *((u8 *)(FSM_PARAM_PTR(msg_ptr)) + 5) = proc_id;
    aos_set_u16((u8 *)FSM_PARAM_PTR(msg_ptr) + 6, 0);
    aos_set_u16((u8 *)FSM_PARAM_PTR(msg_ptr) + 8, length);
    aos_memcpy((u8 *)FSM_PARAM_PTR(msg_ptr) + 10, str, length);
    aos_set_u32((u8 *)FSM_PARAM_PTR(msg_ptr) + 10 + length, line);
    aos_set_u16((u8 *)FSM_PARAM_PTR(msg_ptr) + 14 + length, 0);/* filename */
    FSM_TIMESTAMP(msg_ptr) = aos_get_tickcount();

    AOS_MESSAGE(msg_ptr);/* This API won't free msg ptr! */
    aos_free_fsm_msg(msg_ptr);/* free msg ptr */
    AOS_ASSERT(!force_critical);

    return;
}

/* random APIs are added temporarily for compile and test.
 * It will be replaced by security module API later */
f64 g_aos_random_seed;
void aos_init_random_seed(void)
{
    u32 seed_temp;

    seed_temp = aos_get_tickcount();
    seed_temp  = aos_btol_u32(seed_temp);\
    seed_temp = seed_temp * 2 + 1;
    g_aos_random_seed = 1 / ((f64)seed_temp);
}

void aos_set_random_seed(u32 seed_temp)
{
    u32  soc_seed_temp;
    soc_seed_temp = aos_get_tickcount();
    if(soc_seed_temp)
        seed_temp = seed_temp * soc_seed_temp;
    seed_temp  = aos_btol_u32(seed_temp);
    seed_temp = seed_temp * 2 + 3;
    g_aos_random_seed = 1 / ((f64)seed_temp);
}

s32 aos_random_value(s32 simin, s32 simax)
{
    f64	random_seed;
    f64	random_val;

    random_seed = g_aos_random_seed;

    if(random_seed == 0)
    {
        random_seed = 0.31415926;
    }

    random_seed = random_seed * random_seed;

    while(random_seed < 1.0)
        random_seed *= 10;

    random_seed = random_seed - (f64)(s64)random_seed;

    if(g_aos_random_seed == random_seed)
        aos_init_random_seed();
    else
        g_aos_random_seed = random_seed;

    random_val = (f64)simax - (f64)simin + 1;
    random_val = random_val * random_seed;

    return simin + (s32)random_val;
}

s32 aos_random_value_ex(u32 seed, s32 simin, s32 simax)
{
    AOS_ASSERT(simax-simin+1 != 0);
    return (((s32)(seed % ((u32)(simax - simin + 1)))) + simin);
}

#ifdef AOS_ENABLE_STACK_CHECKING
static void stack_error_handler (TX_THREAD *thread_ptr)
{
    aicx_info(0x00d5db, "sxxx", "thread %s stack, start=0x%x, end=0x%x, highest=0x%x\n",
              thread_ptr->tx_thread_name,
              thread_ptr->tx_thread_stack_start,
              thread_ptr->tx_thread_stack_end,
              thread_ptr->tx_thread_stack_highest_ptr);
    AOS_ASSERT(0);
}

void aos_enable_stackcheck(void)
{
    tx_thread_stack_error_notify(stack_error_handler);
}
#endif

#ifdef AOS_CONFIG_TRACEX
void aos_enable_tracex(void)
{
/* set trace object registry entries to 128 by default,
 * so it can store 128 components infomations, like task/mutex name, etc
 */
    tx_trace_enable(trace_buffer, sizeof(trace_buffer), 128);
}

void aos_disable_tracex(void)
{
    tx_trace_disable();
}

void aos_insert_isr_enter(u32 isr_id)
{
    tx_trace_isr_enter_insert(isr_id);
}

void aos_insert_isr_exit(u32 isr_id)
{
    tx_trace_isr_exit_insert(isr_id);
}

void aos_dump_trace_buffer(void)
{
#ifdef WIN32
    HANDLE hFile;
    DWORD dwWrites;
    LPCTSTR name = L"..\\..\\..\\..\\out\\win32\\Debug\\tracex.mem";

    hFile = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, trace_buffer, sizeof(trace_buffer), &dwWrites, NULL);
    if (dwWrites != sizeof(trace_buffer))
        aicx_info(0x19a416, "", "aos_dump_trace_buffer error!\n");
    CloseHandle(hFile);
    aicx_info(0x02af88, "", "aos_dump_trace_buffer finished!\n");
#else
    /* TODO */
#endif
}
#else
void aos_insert_isr_enter(u32 isr_id){}
void aos_insert_isr_exit(u32 isr_id){}
#endif

void aos_memory_dump(void)
{
#ifdef WIN32
/* no need to dump memory for WIN32, because the memory format can not be
   analyzed by T32 Simulator. Suggest analyze in VS directly */
#if 0
    HANDLE hFile;
    DWORD dwWrites;
    LPCTSTR name = L"..\\..\\..\\..\\out\\win32\\Debug\\aicos.mem";
    DWORD base_addr, size;

    hFile = CreateFile(name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    base_addr = (DWORD)GetModuleHandle(NULL);
    size = (DWORD)AOS_MEM_BASE_ADDR + AOS_MEM_TOTAL_SIZE - base_addr;
    AOS_IDXPR(0x1cea0f, "XX", "aos_memory_dump start address: 0x%X, size: 0x%X \n",
              base_addr, size);

    if (!hFile) {
        AOS_IDXPR(0x117130, "", "dump create file error\n");
        return;
    }

    while (size) {
        WriteFile(hFile, (LPCVOID)base_addr, (size<0x100000)?size:0x100000,
              &dwWrites, NULL);
        if (dwWrites == 0 && dwWrites > size) {
            AOS_IDXPR(0x0a417d, "d", "dump write file error dwWrites=%d\n", dwWrites);
            break;
        }
        size = size - dwWrites;
    }
    CloseHandle(hFile);
#endif
#else
    /* TODO */
#endif
}
