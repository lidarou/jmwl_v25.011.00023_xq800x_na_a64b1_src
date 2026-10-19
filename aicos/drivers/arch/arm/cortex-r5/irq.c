/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines interrupt process handler.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02e0d3
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <timer_inc.h>
#include <irq.h>
#include <core_op.h>
#include <arch/arm/cortex-r5/pl192.h>

#ifdef CONFIG_RTOS_LITEOS_M
#include "los_hook.h"
#endif

//#define IRQ_DEBUG

#ifdef IRQ_DEBUG
typedef struct int_info_s {
    const char *irq_name;
    u32 irq_num;
    u32 last_time;
    u32 count;
    u32 time_cost; /* in us */
} int_info_t;

int_info_t g_dbgirq_record[INT_MAXIUM] = {
    { .irq_name = "IRQ_SLP",        .irq_num =  IRQ_SLP},
    { .irq_name = "IRQ_SLPTMR0",    .irq_num = IRQ_SLPTMR0},
    { .irq_name = "IRQ_SLPTMR1",    .irq_num = IRQ_SLPTMR1},
    { .irq_name = "IRQ_SLPTMR2",    .irq_num = IRQ_SLPTMR2},
    { .irq_name = "IRQ_SLPTMR3",    .irq_num = IRQ_SLPTMR3},
    { .irq_name = "IRQ_SLPTMR4",    .irq_num = IRQ_SLPTMR4},
    { .irq_name = "IRQ_SLPTMR5",    .irq_num = IRQ_SLPTMR5},
    { .irq_name = "IRQ_SLPTMR6",    .irq_num = IRQ_SLPTMR6},
    { .irq_name = "IRQ_SLPTMR7",    .irq_num = IRQ_SLPTMR7},
    { .irq_name = "IRQ_RFC",        .irq_num = IRQ_RFC},
    { .irq_name = "IRQ_SIMC1",      .irq_num = IRQ_SIMC1},
    { .irq_name = "IRQ_SIMC0",      .irq_num = IRQ_SIMC0},
    { .irq_name = "IRQ_TMR",        .irq_num = IRQ_TMR},
    { .irq_name = "IRQ_CIPHC",      .irq_num = IRQ_CIPHC},
    { .irq_name = "NULL",           .irq_num = 14},
    { .irq_name = "IRQ_DMA",        .irq_num = IRQ_DMA},
    { .irq_name = "IRQ_MB_OUT_DIR",  .irq_num = IRQ_MB_OUT_DIR},
    { .irq_name = "IRQ_MB_OUT",     .irq_num = IRQ_MB_OUT},
    { .irq_name = "IRQ_MB_IN",      .irq_num = IRQ_MB_IN},
    { .irq_name = "IRQ_TOP_UART0",  .irq_num = IRQ_TOP_UART0},
    { .irq_name = "IRQ_TOP_UART1",  .irq_num = IRQ_TOP_UART1},
    { .irq_name = "IRQ_AON_TMR0",   .irq_num = IRQ_AON_TMR0},
    { .irq_name = "IRQ_WDG",        .irq_num = IRQ_WDG},
    { .irq_name = "IRQ_CP_BM",      .irq_num = IRQ_CP_BM},
    { .irq_name = "NULL",           .irq_num = 24},
    { .irq_name = "IRQ_RFCSLP",     .irq_num = IRQ_RFCSLP},
    { .irq_name = "IRQ_I2C_PMIC",   .irq_num = IRQ_I2C_PMIC},
    { .irq_name = "IRQ_CP_EIPU",    .irq_num = IRQ_CP_EIPU},
    { .irq_name = "IRQ_ACC_SRC1",   .irq_num = IRQ_ACC_SRC1},
    { .irq_name = "IRQ_ACC_SRC0",   .irq_num = IRQ_ACC_SRC0},
    { .irq_name = "NULL",           .irq_num = 30},
    { .irq_name = "IRQ_COUNTER",    .irq_num = IRQ_COUNTER},
};
/* record irq_handler called times */
u32 g_dbg_irq_null = 0;

#endif
/* 0,select irq;1,select fiq */
#define INTSEL                  (0x0)
#define ALL_INT_MASK            (0xFFFFFFFF)
#define HW_STACK_SIZE           (600)
#define LW_STACK_SIZE           (500)
/* Assume every ints will use worker thread */
#define WORKER_QUEUE_SIZE       (INT_MAXIUM)
#define HWORKER_PRI             (0)
#define LWORKER_PRI             (1)

INT_CONFIG_T g_irq_config[INT_MAXIUM] = {0};
/* Task id of high priority worker task */
aos_thread_id g_hworker_tid;
/* Task id of lower priority worker task */
aos_thread_id g_lworker_tid;

/**
 * isr_wrapper() - ISR wrapper
 *
 * @no: IRQ number
 *
 * Execute ISR and call worker thread if necessary.
 *
 * Return: No return value
 *
 */
void isr_wrapper(IRQn_Type no)
{
    IRQn_Type int_num = no;
    UISR_FUNC uisr;
    ISR_RET_E ret;
    void *param;

#ifdef IRQ_DEBUG
    g_dbgirq_record[int_num].count++;
    g_dbgirq_record[int_num].last_time = aic_get_timestamp();
#endif
#ifdef CONFIG_RTOS_LITEOS_M
    extern u32 g_los_int_nesting;
    g_los_int_nesting++;
    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, int_num);
#endif
    aos_insert_isr_enter((u32)int_num);
    uisr = g_irq_config[int_num].p_uisr;
    param = g_irq_config[int_num].uparam;
    if (uisr && param)
        ret = uisr(param);
    else if (uisr && !param)
        ret = uisr(&int_num);
    else
        ret = NONE_WORKER;

    switch(ret) {
    case CALL_HWORKER:
        if(g_irq_config[int_num].flag++ == 0)
            aos_send_msg(g_hworker_tid, (aos_msg_ptr)int_num, AOS_NO_WAIT);
        break;
    case CALL_LWORKER:
        if(g_irq_config[int_num].flag++ == 0)
            aos_send_msg(g_lworker_tid, (aos_msg_ptr)int_num, AOS_NO_WAIT);
        break;
    default:
        break;
    }
    pl192_writevec();
    aos_insert_isr_exit((u32)int_num);
#ifdef CONFIG_RTOS_LITEOS_M
    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, int_num);
    g_los_int_nesting--;
#endif
#ifdef IRQ_DEBUG
    g_dbgirq_record[int_num].time_cost = aic_get_timestamp() - g_dbgirq_record[int_num].last_time;
#endif

    return;
}

/* isr wrappers */
extern void isr_func0(void);
extern void isr_func1(void);
extern void isr_func2(void);
extern void isr_func3(void);
extern void isr_func4(void);
extern void isr_func5(void);
extern void isr_func6(void);
extern void isr_func7(void);
extern void isr_func8(void);
extern void isr_func9(void);
extern void isr_func10(void);
extern void isr_func11(void);
extern void isr_func12(void);
extern void isr_func13(void);
extern void isr_func14(void);
extern void isr_func15(void);
extern void isr_func16(void);
extern void isr_func17(void);
extern void isr_func18(void);
extern void isr_func19(void);
extern void isr_func20(void);
extern void isr_func21(void);
extern void isr_func22(void);
extern void isr_func23(void);
extern void isr_func24(void);
extern void isr_func25(void);
extern void isr_func26(void);
extern void isr_func27(void);
extern void isr_func28(void);
extern void isr_func29(void);
extern void isr_func30(void);
extern void isr_func31(void);

u32 g_wrapper[INT_MAXIUM] = {
    (u32)isr_func0,
    (u32)isr_func1,
    (u32)isr_func2,
    (u32)isr_func3,
    (u32)isr_func4,
    (u32)isr_func5,
    (u32)isr_func6,
    (u32)isr_func7,
    (u32)isr_func8,
    (u32)isr_func9,
    (u32)isr_func10,
    (u32)isr_func11,
    (u32)isr_func12,
    (u32)isr_func13,
    (u32)isr_func14,
    (u32)isr_func15,
    (u32)isr_func16,
    (u32)isr_func17,
    (u32)isr_func18,
    (u32)isr_func19,
    (u32)isr_func20,
    (u32)isr_func21,
    (u32)isr_func22,
    (u32)isr_func23,
    (u32)isr_func24,
    (u32)isr_func25,
    (u32)isr_func26,
    (u32)isr_func27,
    (u32)isr_func28,
    (u32)isr_func29,
    (u32)isr_func30,
    (u32)isr_func31
};

/**
 * register_irq() - Register an interrupt.
 *
 * @int_num: Interrupt number to be registered
 * @uisr: Upper ISR
 * @uparam: Parameters for upper ISR
 * @bisr: Bottom ISR
 * @bparam: Parameters for bottom handler
 * @pri: Priority
 *
 * Register an interrupt and enable it. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_SUCCESS, if register success
 *         AIC_EINVAL, if upper handler is null or int number is too large
 *         AIC_EPERM, if get mutex failed
 *
 */
s32 register_irq(IRQn_Type int_num,
                 UISR_FUNC uisr,
                 void *uparam,
                 BISR_FUNC bisr,
                 void *bparam)
{
    s32 ret_val = AIC_SUCCESS;

    if (int_num >= INT_MAXIUM || !uisr)
        return AIC_EINVAL;

    g_irq_config[int_num].p_uisr= uisr;
    g_irq_config[int_num].uparam = uparam;
    g_irq_config[int_num].p_bisr= bisr;
    g_irq_config[int_num].bparam = bparam;
    /* The priority will be arranged globally if necessary */
    pl192_setpri(int_num, 0);
    pl192_seten(0x1 << int_num);

    return ret_val;
}

/**
 * unregister_irq() - Unregister an interrupts
 *
 * Disable interrupt and clear its handler.
 *
 *
 */
void unregister_irq(IRQn_Type int_num)
{
    AIC_INT_SAVE_AREA
    if (int_num < INT_MAXIUM) {
        AIC_INT_DISABLE
        pl192_clren(0x1 << int_num);
        pl192_setpri(int_num, 0xf);
        g_irq_config[int_num].p_uisr= AIC_NULL;
        g_irq_config[int_num].uparam = AIC_NULL;
        g_irq_config[int_num].p_bisr= AIC_NULL;
        g_irq_config[int_num].bparam = AIC_NULL;
        AIC_INT_RESTORE
    }
    return;
}

/**
 * register_frq() - Register a fiq interrupt.
 *
 * @int_num: Interrupt number to be registered
 * @uisr: Upper ISR
 * @uparam: Parameters for upper ISR
 *
 * Register a fiq interrupt and enable it. If new int registered which already exist,
 * The old one will be replaced.
 *
 * Return: AIC_SUCCESS, if register success
 *         AIC_EINVAL, if upper handler is null or int number is too large
 *         AIC_EPERM, if get mutex failed
 *
 */
s32 register_frq(IRQn_Type int_num,
                 UISR_FUNC uisr,
                 void *uparam)
{
    s32 ret_val = AIC_SUCCESS;

    if (int_num >= INT_MAXIUM || !uisr)
        return AIC_EINVAL;

    g_irq_config[int_num].p_uisr= uisr;
    g_irq_config[int_num].uparam = uparam;
    g_irq_config[int_num].p_bisr= NULL;
    g_irq_config[int_num].bparam = NULL;
    /* The priority will be arranged globally if necessary */
    pl192_setpri(int_num, 0);
    pl192_seten(0x1 << int_num);
    pl192_intsel(0x1 << int_num);

    return ret_val;
}

/**
 * unregister_frq() - Unregister a fiq interrupts
 *
 * Disable interrupt and clear its handler.
 *
 *
 */
void unregister_frq(IRQn_Type int_num)
{
    AIC_INT_SAVE_AREA
    if (int_num < INT_MAXIUM) {
        AIC_INT_DISABLE
        pl192_clren(0x1 << int_num);
        pl192_setpri(int_num, 0xf);
        g_irq_config[int_num].p_uisr= AIC_NULL;
        g_irq_config[int_num].uparam = AIC_NULL;
        g_irq_config[int_num].p_bisr= AIC_NULL;
        g_irq_config[int_num].bparam = AIC_NULL;
        AIC_INT_RESTORE
    }
    return;
}

#define CREATE_THRD_N_QUEUE(msg, thrd, entry, stack, pri) \
        aos_create_thread(#thrd, entry, 0, stack, WORKER_QUEUE_SIZE, #msg, pri, AOS_AUTO_START)

/**
 * hworker_entry() - High priority worker task entry
 *
 * Receiving message from irq handler, and do bottom processing.
 *
 */
void hworker_entry(void *param)
{
    IRQn_Type int_num;

    while(1) {
        AIC_INT_SAVE_AREA
        int_num = (IRQn_Type)aos_recv_msg(g_hworker_tid, AOS_WAIT_FOREVER);
        AIC_ASSERT(int_num >= 0 && int_num < INT_MAXIUM);
        AIC_INT_DISABLE
        g_irq_config[int_num].flag = 0;
        AIC_INT_RESTORE
        if (g_irq_config[int_num].p_bisr)
            g_irq_config[int_num].p_bisr(g_irq_config[int_num].bparam);
    }
}

/**
 * lworker_entry() - Lower priority worker task entry
 *
 * Receiving message from irq handler, and do bottom processing.
 *
 */
void lworker_entry(void *param)
{
    IRQn_Type int_num;

    while(1) {
        AIC_INT_SAVE_AREA
        int_num = (IRQn_Type)aos_recv_msg(g_lworker_tid, AOS_WAIT_FOREVER);
        AIC_ASSERT(int_num >= 0 && int_num < INT_MAXIUM);
        AIC_INT_DISABLE
        g_irq_config[int_num].flag = 0;
        AIC_INT_RESTORE
        if (g_irq_config[int_num].p_bisr)
            g_irq_config[int_num].p_bisr(g_irq_config[int_num].bparam);
    }
}

/**
 * irq_handler() - IRQ handler
 *
 * As interrupt vectors are provided by vic, this function should not be
 * used. Do some debug process.
 *
 */
void irq_handler(void)
{
#ifdef IRQ_DEBUG
    g_dbg_irq_null++;
#endif
}

/**
 * fiq_handler() - FIQ handler
 *
 * Get fiq status, and pass it to FIQ handler.
 * It is assumed all FIQS(if any) use one handler.
 * The handler do other process.
 *
 */
void fiq_handler(void)
{
    u32 sts;
    s32 i = 0;

    sts = pl192_get_fiqsts();
    while (i < INT_MAXIUM) {
        if ((sts & 0x1)) {
            AIC_ASSERT(g_irq_config[i].p_uisr);
            g_irq_config[i].p_uisr(g_irq_config[i].uparam);
        }
        sts >>= 0x1;
        i++;
    }
}

/**
 * irq_init() - Initialize interrupt
 *
 * Create 2 worker tasks to process bottom interrupt.Setup PL192 vectors
 * and priority,enable all interrupt.
 *
 */
s32 irq_init(void)
{
    u32 scr_val;
    /* Create worker task for bottom processing */
    g_hworker_tid = CREATE_THRD_N_QUEUE(hq, hw, hworker_entry, HW_STACK_SIZE, HWORKER_PRI);
    AIC_ASSERT(g_hworker_tid != AOS_INVALID_THREAD_ID);
    g_lworker_tid = CREATE_THRD_N_QUEUE(lq, lw, lworker_entry, LW_STACK_SIZE, LWORKER_PRI);
    AIC_ASSERT(g_lworker_tid != AOS_INVALID_THREAD_ID);
    /* set VE to 1, so that pl192 do handshake with cr5 automaticly */
    scr_val = get_sctlr();
    scr_val |= BIT_24;
    set_sctlr(scr_val);

    pl192_en(TRUE);
    /* Ensure no protection */
    pl192_prot(false);
    /* Ensure not in test mode */
    pl192_testmode_ctrl(false);
    /* Clear and disable all int */
    pl192_clren(ALL_INT_MASK);
    pl192_clear_sgi(ALL_INT_MASK);
    /* Set vectaddr and priority */
    for (s32 i = 0; i < INT_MAXIUM; i++) {
        pl192_setvec(i, (u32)g_wrapper[i]);
        pl192_setpri(i, 0xf);
    }
    /* Set fiq/irq select */
    pl192_intsel(INTSEL);
    /* Unmask all priority levels */
    pl192_swpri_mask(0xFFFF);
    return AIC_SUCCESS;
}

/**
 * irq_suspend() - Interrupt backup
 *
 * @addr: buffer address to save irq registers.
 *
 */
void irq_suspend(u32 *addr)
{
    pl192_suspend();
    return;
}

__attribute__ ((section(".idle_text")))
/**
 * irq_resume() - Interrupt restore
 *
 * @addr: buffer address to save irq registers.
 *
 */
void irq_resume(u32 *addr)
{
    pl192_resume();
    return;
}
