/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert Interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1300f8
#define LOG_MODULE    AMOD_SYS

#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_io_dev.h>
#include <aic_boot_mode.h>
#include <aic_osa.h>
#include <aic_mcx.h>
#include <aipc_api.h>
#include <adbg_cfg_api.h>
#include <alog_api.h>
#include <adbg_cfg_api.h>
#include <aonram_mem_config.h>
#include <assert_cmd.h>
#include <wdt.h>
#include <sys_mem_config.h>
#include <sys_mem_api.h>
#include <cache.h>
#include <core_op.h>
#include <coresight.h>
#include <stdio.h>
#include <sysoff.h>
#include <user_tracex_api.h>
#include <sprintf_nano.h>

#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#endif

#ifdef CONFIG_SUBSYS_AP
#include <pcdump_inc.h>
#endif

#include "exception_parse.h"
#include "system_assert.h"

#if defined(CONFIG_SUBSYS_CP)
#include "assert_cmd.h"
#include "assert_dump.h"
#endif

#include "assert_adpter.h"

#define ASSERT_INT_SAVE     AIC_INT_SAVE_AREA
#define ASSERT_INT_DISABLE  AIC_INT_DISABLE
#define ASSERT_INT_RESTORE  AIC_INT_RESTORE

extern void __save_all_regs(void);

#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
struct assert_ar_common_reg cur_mode_reg = {0};
struct assert_ar_bank_reg svc_mode_reg = {0};
struct assert_ar_bank_reg und_mode_reg = {0};
struct assert_ar_bank_reg abt_mode_reg = {0};
struct assert_ar_bank_reg irq_mode_reg = {0};
struct assert_ar_fiq_bank_reg fiq_mode_reg = {0};
struct assert_ar_fault_status_reg dabt_reg = {0};
struct assert_ar_fault_status_reg pabt_reg = {0};
u32 g_save_lr_reg;

#elif defined(ASSERT_ARM_ARCH_M)
struct assert_m_common_reg cur_mode_reg = {0};
struct assert_m_exception_frame  exc_frame_reg = {0};
struct assert_m_fault_status_reg fault_reg = {0};
#else
#error "!!!CPU type not support!!!\n"
#endif

#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
#define IS_AP_NO_ENTER_SYSDUMP() \
   (adbg_get_log_dev() == AIC_IODEV_CMUX || adbg_get_log_dev() == AIC_IODEV_UART)
#else
#define IS_AP_NO_ENTER_SYSDUMP() FALSE
#endif

struct assert_info s_assert_info = {0};

aos_heap_size heap_info;

#ifdef CONFIG_SUBSYS_AP
struct aic_pcdump_data g_cp_reg_data[2] = {0};
#endif

static enum assert_mode assert_get_assert_mode(void)
{
    return DEBUG_ASSERT;
}

static inline void assert_save_pls_regs(u32 *sp_reg, u32 *lr_reg, u32 *pc_reg)
{
    __ASM volatile ("mov %0, pc" : "=r" (*pc_reg) );
    __ASM volatile ("mov %0, sp" : "=r" (*sp_reg) );
    __ASM volatile ("mov %0, lr" : "=r" (*lr_reg) );
}

static void assert_save_cpu_regs_info(struct assert_common_info *info)
{
#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    struct assert_ar_regs *reg_ptr = &(info->regs);

    memcpy(&(reg_ptr->cur_mode_reg), &cur_mode_reg,
                sizeof(struct assert_ar_common_reg));
    memcpy(&(reg_ptr->svc_mode_reg), &svc_mode_reg,
                sizeof(struct assert_ar_bank_reg));
    memcpy(&(reg_ptr->und_mode_reg), &und_mode_reg,
                sizeof(struct assert_ar_bank_reg));
    memcpy(&(reg_ptr->abt_mode_reg), &abt_mode_reg,
                sizeof(struct assert_ar_bank_reg));
    memcpy(&(reg_ptr->fiq_mode_reg), &fiq_mode_reg,
                sizeof(struct assert_ar_fiq_bank_reg));
    memcpy(&(reg_ptr->irq_mode_reg), &irq_mode_reg,
                sizeof(struct assert_ar_bank_reg));
    memcpy(&(reg_ptr->dabt_reg), &dabt_reg,
                sizeof(struct assert_ar_fault_status_reg));
    memcpy(&(reg_ptr->pabt_reg), &pabt_reg,
                sizeof(struct assert_ar_fault_status_reg));
#elif defined(ASSERT_ARM_ARCH_M)
    struct assert_m_regs *reg_ptr = &(info->regs);

    memcpy(&(reg_ptr->cur_mode_reg), &cur_mode_reg,
                sizeof(struct assert_m_common_reg));
    memcpy(&(reg_ptr->exc_frame), &exc_frame_reg,
                sizeof(struct assert_m_exception_frame));
    memcpy(&(reg_ptr->fault_reg), &fault_reg,
                sizeof(struct assert_m_fault_status_reg));

    if (aic_boot_mode_is_basic() &&
        sys_is_belong_extend_image((void*)exc_frame_reg.pc)) {
        ASSERT_LOG("base call extend: pc=0x%x, lr=0x%x.\n",
                    exc_frame_reg.pc & 0xFFFFFFFE, exc_frame_reg.lr);
    }

#else
#error "!!!CPU type not support!!!\n"
#endif
}

static int assert_notify_other_cpu(const char *info_string)
{
    int ret = 0;
#if defined(CONFIG_SUBSYS_AP)
    size_t aon_size, unused_size;
    extern void     *_tx_initialize_unused_memory;
    void *heap_mem;
#endif

#ifdef CONFIG_AIC_CTRL_SUPPORT
    if (aic_boot_mode_is_normal()) {
        aic_ctrl_set_abnormal_pin(TRUE);
        aic_ctrl_str_notify(info_string);
    }
#endif

#if defined(CONFIG_SUBSYS_AP)
    /* not PASSIVE_ASSERT, Notify cp to assert. If cp responses timeout, ap while(1). */
    if (s_assert_info.info[0].reason != PASSIVE_ASSERT)
        ret = ap_notify_other_sys_assert(AIC_SYS_CP);

    /*
     * For ap assert, will enter sysdump, than the romcode and bootloader
     * Will recover the start area of the aon iram.
     * So move the share memory to the psram for debug.
     */
    aon_size = PHY_BUF_REGION_BASE - AONRAM_REGION_BASE;
    heap_mem = aos_malloc_relaxed(aon_size);

    if (heap_mem) {
        _tx_initialize_unused_memory = heap_mem;
    } else {
        /* SDK version can't use the rw unused memory. */
        #ifdef CONFIG_SDK_ENABLE
        unused_size = 0;
        #else
        unused_size = ((AP_REGION_BASE + AP_REGION_SIZE) - (u32)_tx_initialize_unused_memory);
        #endif
        /* unused mem is not big enough, use the share memory to save it. */
        if (unused_size < aon_size)
            _tx_initialize_unused_memory = (void *)(SHM_CP2AP_BASE + SHM_AP2CP_SIZE - aon_size);
    }

    memcpy(_tx_initialize_unused_memory,(void *)AONRAM_REGION_BASE, aon_size);

#elif defined(CONFIG_SUBSYS_CP)
    MCX_CP_NOTIFY_ASSERT_TO_AP();
    cp_assert_notify_ap(info_string);
#endif

    return ret;
}

void system_assert(const char *info_string)
{
    void *thread_top;
    u8 stack_enough = 2;
    u32 assert_num;
    u32 cur_sp, cur_lr, cur_pc;
    struct assert_common_info *info;
    ASSERT_INT_SAVE;

    ASSERT_INT_DISABLE;

#if defined(CONFIG_DBG_CPPC_SAMP) && defined(CONFIG_CPU_CORTEX_R5)
    aic_tmr_samp_cfg(0);
#endif

    s_assert_info.assert_status = TRUE;
    s_assert_info.mode = assert_get_assert_mode();

#if defined(CONFIG_SUBSYS_AP)
    /* basic mode, don't dump, just reset to basic. */
    if (aic_boot_mode_is_basic())
        sysoff_reset_basic();
#endif

    /* CONFIG_AIC_CTR_DBG_ON_CMUX, user mode not reset sys. */
#ifndef CONFIG_AIC_CTR_DBG_ON_CMUX
    /* user mod and dump is not opend and reset is.opend */
    if (aic_is_user_mode() &&
        !adbg_is_dump(AIC_THIS_SYS) &&
        adbg_is_auto_reset(AIC_THIS_SYS)) {
#if defined(CONFIG_SUBSYS_AP)
        /* just reboot sys. */
        sysoff_reset();
#else
        /* PASSIVE_ASSERT, just deadloop. */
        if (s_assert_info.info[0].reason!= PASSIVE_ASSERT) {
            /* notfy ap. */
            MCX_CP_NOTIFY_ASSERT_TO_AP();
            cp_assert_notify_ap(info_string);
        }
        while (1);
#endif
    }
#endif

    /* if assert cnt greater than MAX_COUNT, enter while(1) */
    assert_num = s_assert_info.assert_cnt++;
    if (assert_num >= ASSERT_MAX_COUNT)
        while (1);

#if defined(CONFIG_SUBSYS_AP)
#ifdef CONFIG_DRIVER_CORESIGHT_MTB
    aic_cs_trace_stop();
#endif
#endif

    USER_TRACE_IN_LINE_INSERT(TX_TRACE_ASSERT_EVENT, 0xff, 0x01);

#if defined (CONFIG_DRIVER_WDT)
    aic_watchdog_stop();
#endif

    /* save current sp, lr, pc register value */
    assert_save_pls_regs(&cur_sp, &cur_lr, &cur_pc);

    /*
     * if the running thread remain space is not beg enough,
     * can't call some function that need big stack space
     */
    thread_top = aos_get_running_thread_stack_top();
    if (thread_top) {
        if ((cur_sp - 0x200) < (u32)thread_top)
            stack_enough--;
        if ((cur_sp - 0x100) < (u32)thread_top)
            stack_enough--;
    }

#ifndef CONFIG_SUBSYS_AP
    if (stack_enough)
        ASSERT_LOG("%s", info_string);
#endif


    info = &(s_assert_info.info[assert_num]);

    /* step 1 */
    info->assert_step = 1;

    /* Save CPU info */
#if defined(ASSERT_ARM_ARCH_R) || defined(ASSERT_ARM_ARCH_A)
    if (AR_EXCEPTION_ASSERT != info->assert_type) {
        u32 cur_cpsr;
#ifdef CONFIG_RTOS_THREADX

        __save_all_regs();
#endif
        cur_mode_reg.r13 = cur_sp;
        cur_mode_reg.r14 = cur_lr;
        cur_mode_reg.pc = cur_pc;

        cur_cpsr = get_cpsr() & 0x1F;

        if (cur_cpsr == IRQ_MODE) {
            info->assert_type = AR_IRQ_ASSERT;
            irq_mode_reg.r14 = cur_lr;
        } else if (cur_cpsr == SVC_MODE) {
            info->assert_type = AR_NORMAL_ASSERT;
            svc_mode_reg.r14 = cur_lr;
        } else if (cur_cpsr == FIQ_MODE)
            info->assert_type = AR_FIQ_ASSERT;
    }
#elif defined(ASSERT_ARM_ARCH_M)
    if (M_EXCEPTION_ASSERT != info->assert_type) {
        __save_all_regs();

        cur_mode_reg.r13 = cur_sp;
        cur_mode_reg.r14 = cur_lr;
        cur_mode_reg.pc = cur_pc;

        if ((__get_IPSR() & 0xFF) == 0)
            info->assert_type = M_NORMAL_ASSERT;
        else
            info->assert_type = M_IRQ_ASSERT;
    }
#else
#error "!!!CPU type not support!!!\n"
#endif

    /* Save all cpu register value and assert info */
    assert_save_cpu_regs_info(info);
    info->info_string = info_string;

    /* step 2 */
    info->assert_step = 2;

#if defined(CONFIG_SUBSYS_CP)
    /* If passive assert happend, stop here */
    if (s_assert_info.info[0].reason == PASSIVE_ASSERT) {
        /* flush dcache */
        dcache_invcc_all();

        /* send ack to ap */
        cp_assert_notify_ap(CP_ASSERT_ACK);
        alog_flush_all_log_in_assert();

        /* save heap info */
        if (assert_num == 0 && stack_enough == 2) {
            aos_get_heap_size_ex(&heap_info);
        }

        info->assert_step = 3;
        /* flush dcache */
        dcache_invcc_all();
        while(1);
    } else {
        if (s_assert_info.q_ful_assert &&
            (cp_assert_is_brr_blocked(AIPC_BRR_AT) ||
             cp_assert_is_brr_blocked(AIPC_BRR_IP))) {
            AMSG_T msg;

            amsg_fill_msg(&msg, AMOD_SYS, AIPC_MSG_ASSERT, 0);
            (void)amsg_raw_send_msg(AIC_SYS_AP, &msg);
        }
    }
#endif

    /* step 3 */
    info->assert_step = 3;

    /* Notify other CPU */
    if (!s_assert_info.assert_notified) {
        /* int ret = */assert_notify_other_cpu(info_string);
        s_assert_info.assert_notified = TRUE;

#if 0
        /*
         * The below code is used to debug some assert that the cp
         * can't respomd the assert.
         * Now remove it, if need reopen it again.
         *is not M_EXCEPTION_ASSERT, dead loop. */
        if (M_EXCEPTION_ASSERT != info->assert_type) {
            alog_flush_all_log_in_assert();
            while (ret);
        }
#endif
    }

    /* save heap info */
    if (assert_num == 0 && stack_enough == 2) {
        aos_get_heap_size_ex(&heap_info);
    }

    /* step 4 */
    info->assert_step = 4;

#if defined(CONFIG_SUBSYS_AP)
    u8 dev_type, dev_com;
    u32 dev_baud;
    char info_addr[sizeof(u32) * 2 + 1] = {0};
    snprintf_nano(info_addr, sizeof(info_addr), "%x", (u32)info_string);
    aic_cmdline_add_key(cl_key_asrt_info, info_addr);

    if (adbg_is_ldump_on()) {
        dev_type = adbg_get_log_dev();
        dev_com = adbg_get_log_dev_com();
        //dev_baud = adbg_get_log_baudrate();
    } else {
        dev_type = adbg_get_diag_dev();
        dev_com = adbg_get_diag_dev_com();
        //dev_baud = adbg_get_diag_baudrate();
    }

    dev_baud = adbg_get_diag_baudrate();
    aic_cmdline_add_key_int(cl_key_dev_type, dev_type);
    if (dev_type == AIC_IODEV_UART) {
        aic_cmdline_add_key_int(cl_key_dev_com, dev_com);
        aic_cmdline_add_key_int(cl_key_dev_baud, dev_baud);
    }

    /* save cp regs to global val. */
    ap_save_cp_reg_info();
#endif

    USER_TRACE_IN_LINE_INSERT(TX_TRACE_ASSERT_EVENT, 0xff, 0x02);

    /* flush all logs before assert. */
    alog_flush_all_log_in_assert();

    /* step 5 */
    info->assert_step = 5;

    /* flush dcache */
    dcache_invcc_all();

#if defined(CONFIG_SUBSYS_AP)
    if (!IS_AP_NO_ENTER_SYSDUMP())
        sysoff_set_dumpmode(SYSOFF_MODE_APCRASH);

/* CONFIG_AIC_CTR_DBG_ON_CMUX, will not enter to sysdump, just dump it by cmux. */
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
    if (IS_AP_NO_ENTER_SYSDUMP()) {
        assert_cmd_process(&s_assert_info.info[0]);
        while (1);
    }
#endif

    /* just enter to sysdump, regardless assert num. */
    if (adbg_is_ap_dump())
        sysoff_sysdump();
    else if (adbg_is_ap_reset())
        sysoff_reset();

    USER_TRACE_IN_LINE_INSERT(TX_TRACE_ASSERT_EVENT, 0xff, 0x03);

    while(1);
#else
    if (s_assert_info.mode != DEBUG_ASSERT) {
        /* If cpu runs in here, this means MCU needs to be reset. */
        cp_assert_notify_ap(CP_RESET_ACK);
        while(1);
    }

    USER_TRACE_IN_LINE_INSERT(TX_TRACE_ASSERT_EVENT, 0xff, 0x03);

    /* Interactive command handle, Prioritize reporting the fisrt assert info. */
    assert_cmd_process(&s_assert_info.info[0]);
#endif
}

void assert_passive_handle(void)
{
    u32 i = s_assert_info.assert_cnt;

    s_assert_info.info[i].reason = PASSIVE_ASSERT;
    system_assert("Passive assert!");
}

boolean aic_sys_is_assert(void)
{
    return s_assert_info.assert_status;
}

void aic_assert_set_queue_full(void)
{
    s_assert_info.q_ful_assert = TRUE;
}

#ifdef CONFIG_SUBSYS_CP
void aic_sys_assert_init(void)
{
    assert_cmd_init();
    assert_dump_region_init();
}
#else

#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
#include <assert_dump_api.h>

void assert_dump_region_init(void)
{
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    assert_dump_region_register_mem(AIC_THIS_SYS, FALSE,
                                    AONRAM_REGION_BASE, AONRAM_REGION_SIZE);
    assert_dump_region_register_mem(AIC_SYS_CP, TRUE,
                                    CP_REGION_BASE, CP_REGION_SIZE);
    assert_dump_region_register_mem(AIC_SYS_AP, FALSE,
                                    AP_REGION_BASE, AP_REGION_SIZE);
    assert_dump_region_register_mem(AIC_SYS_AP, FALSE,
                                    SHMEM_REGION_BASE, SHMEM_REGION_SIZE);

#ifdef CONFIG_SUPPORT_ADSP
    assert_dump_region_register_mem(AIC_SYS_ADSP, TRUE,
                                    ADSP_REGION_BASE, ADSP_REGION_SIZE);
#endif

    assert_dump_region_register_mem(AIC_THIS_SYS, FALSE,
                                    FLASH0_BASE, FLASH0_TOTAL_SIZE);
#endif
}
#endif

void aic_sys_assert_init(void)
{
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
    assert_cmd_init();
    assert_dump_region_init();
#endif
}
#endif

#ifdef CONFIG_SUBSYS_AP
void ap_save_cp_reg_info(void)
{
#ifdef CONFIG_DRIVER_PCDUMP
    /* dump cp regs twice */
    gx_pcdump_get_regs(&g_cp_reg_data[0]);
    gx_pcdump_get_regs(&g_cp_reg_data[1]);
#endif
}
#endif

