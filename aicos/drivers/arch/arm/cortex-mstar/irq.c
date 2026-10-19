/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * IRQ processing file.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0bbc2a
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <irq.h>
#include <cache.h>
#include <core_star.h>
#include <core_op.h>
#include <timer_inc.h>

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
    {.irq_name = "IRQ_EIP130",        .irq_num = IRQ_EIP130},
    {.irq_name = "IRQ_EIP130_SEC",    .irq_num = IRQ_EIP130_SEC},
    {.irq_name = "IRQ_USB",           .irq_num = IRQ_USB},
    {.irq_name = "IRQ_USBWU",         .irq_num = IRQ_USBWU},
    {.irq_name = "IRQ_SDIOH",         .irq_num = IRQ_SDIOH},
    {.irq_name = "IRQ_SSP0",          .irq_num = IRQ_SSP0},
    {.irq_name = "IRQ_SSP1",          .irq_num = IRQ_SSP1},
    {.irq_name = "IRQ_CSI",           .irq_num = IRQ_CSI},
    {.irq_name = "IRQ_LCDC",          .irq_num = IRQ_LCDC},
    {.irq_name = "IRQ_I2C0",          .irq_num = IRQ_I2C0},
    {.irq_name = "IRQ_I2C1",          .irq_num = IRQ_I2C1},
    {.irq_name = "IRQ_UART0",         .irq_num = IRQ_UART0},
    {.irq_name = "IRQ_UART1",         .irq_num = IRQ_UART1},
    {.irq_name = "IRQ_UART2",         .irq_num = IRQ_UART2},
    {.irq_name = "IRQ_TMR",           .irq_num = IRQ_TMR},
    {.irq_name = "IRQ_DMA",           .irq_num = IRQ_DMA},
    {.irq_name = "IRQ_DMA_SEC",       .irq_num = IRQ_DMA_SEC},
    {.irq_name = "IRQ_PPPD",          .irq_num = IRQ_PPPD},
    {.irq_name = "IRQ_PPPE",          .irq_num = IRQ_PPPE},
    {.irq_name = "IRQ_MCU_BM",        .irq_num = IRQ_MCU_BM},
    {.irq_name = "IRQ_AON_TMR0",      .irq_num = IRQ_AON_TMR0},
    {.irq_name = "IRQ_WDG",           .irq_num = IRQ_WDG},
    {.irq_name = "IRQ_AON_UART",      .irq_num = IRQ_AON_UART},
    {.irq_name = "IRQ_AON_GPIO",      .irq_num = IRQ_AON_GPIO},
    {.irq_name = "NULL",              .irq_num = 24},
    {.irq_name = "IRQ_EIPU",          .irq_num = IRQ_EIPU},
    {.irq_name = "IRQ_KPD",           .irq_num = IRQ_KPD},
    {.irq_name = "IRQ_GPD",           .irq_num = IRQ_GPD},
    {.irq_name = "IRQ_EIPU_SEC",      .irq_num = IRQ_EIPU_SEC},
    {.irq_name = "IRQ_TMR_SEC",       .irq_num = IRQ_TMR_SEC},
    {.irq_name = "IRQ_WDG_SEC",       .irq_num = IRQ_WDG_SEC},
    {.irq_name = "IRQ_AON_GPIO_SEC",  .irq_num = IRQ_AON_GPIO_SEC},
    {.irq_name = "IRQ_I2C_PMIC",      .irq_num = IRQ_I2C_PMIC},
    {.irq_name = "IRQ_AON_I2C0",      .irq_num = IRQ_AON_I2C0},
    {.irq_name = "IRQ_AON_I2C1",      .irq_num = IRQ_AON_I2C1},
    {.irq_name = "IRQ_THM",           .irq_num = IRQ_THM},
    {.irq_name = "IRQ_TOP_UART0",     .irq_num = IRQ_TOP_UART0},
    {.irq_name = "IRQ_TOP_UART1",     .irq_num = IRQ_TOP_UART1},
    {.irq_name = "IRQ_TOP_I2S0",      .irq_num = IRQ_TOP_I2S0},
    {.irq_name = "IRQ_TOP_I2S1",      .irq_num = IRQ_TOP_I2S1},
    {.irq_name = "IRQ_EFUSE",         .irq_num = IRQ_EFUSE},
    {.irq_name = "IRQ_TOP_GPIO",      .irq_num = IRQ_TOP_GPIO},
    {.irq_name = "IRQ_MB_OUT_DIR",    .irq_num = IRQ_MB_OUT_DIR},
    {.irq_name = "IRQ_MB_OUT",        .irq_num = IRQ_MB_OUT},
    {.irq_name = "IRQ_MB_IN",         .irq_num = IRQ_MB_IN},
    {.irq_name = "IRQ_LZMA",          .irq_num = IRQ_LZMA},
    {.irq_name = "IRQ_CPU_BM",        .irq_num = IRQ_CPU_BM},
    {.irq_name = "NULL",              .irq_num = 47},
    {.irq_name = "NULL",              .irq_num = 48},
    {.irq_name = "IRQ_IPU_SEC",       .irq_num = IRQ_IPU_SEC},
    {.irq_name = "IRQ_AUD_RST_REQ",   .irq_num = IRQ_AUD_RST_REQ},
    {.irq_name = "IRQ_CP_RST_REQ",    .irq_num = IRQ_CP_RST_REQ},
    {.irq_name = "IRQ_QSPI",          .irq_num = IRQ_QSPI},
    {.irq_name = "IRQ_PMIC",          .irq_num = IRQ_PMIC},
    {.irq_name = "IRQ_PSRM",          .irq_num = IRQ_PSRM},
    {.irq_name = "IRQ_BM_CP",         .irq_num = IRQ_BM_CP},
    {.irq_name = "IRQ_BM_CPU",        .irq_num = IRQ_BM_CPU},
    {.irq_name = "IRQ_BM_ACC",        .irq_num = IRQ_BM_ACC},
    {.irq_name = "IRQ_MPU_VIO",       .irq_num = IRQ_MPU_VIO},
};

#endif

#define HW_STACK_SIZE      (800)
#define LW_STACK_SIZE      (2048)
/* Assume every ints will use worker thread */
#define WORKER_QUEUE_SIZE  (INT_MAXIUM)
#define HWORKER_PRI        (0)
#define LWORKER_PRI        (1)

INT_CONFIG_T g_irq_config[INT_MAXIUM] = {0};
/* Task id of high priority worker task */
aos_thread_id g_hworker_tid;
/* Task id of lower priority worker task */
aos_thread_id g_lworker_tid;

/**
 * register_irq() - Register a interrupt.
 *
 * @int_num: Interrupt number to be registered
 * @uisr: Upper ISR
 * @uparam: Parameters for upper ISR
 * @bisr: Bottom ISR
 * @bparam: Parameters for bottom handler
 * @pri: Priority
 *
 * Register a interrupt. If new int registered which already exist,
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

    if (int_num >= INT_MAXIUM || int_num < 0 || !uisr)
        return AIC_EINVAL;

    g_irq_config[int_num].p_uisr= uisr;
    g_irq_config[int_num].uparam = uparam;
    g_irq_config[int_num].p_bisr= bisr;
    g_irq_config[int_num].bparam = bparam;
    g_irq_config[int_num].flag = 0;
    NVIC_DisableIRQ(int_num);
    NVIC_ClearPendingIRQ(int_num);
    /* The priority will be arranged globally if necessary */
    NVIC_SetPriority(int_num, 0);
    NVIC_EnableIRQ(int_num);

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
        g_irq_config[int_num].p_uisr= AIC_NULL;
        g_irq_config[int_num].uparam = AIC_NULL;
        g_irq_config[int_num].p_bisr= AIC_NULL;
        g_irq_config[int_num].bparam = AIC_NULL;
        g_irq_config[int_num].flag = 0;
        NVIC_DisableIRQ(int_num);
        NVIC_ClearPendingIRQ(int_num);
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
 * irq_init() - Initialize interrupt
 *
 * Create 2 worker tasks to process bottom interrupt.
 * Disable All interrupt and clear pending signals.
 *
 */
s32 irq_init(void)
{
    u32 vtor_addr;

    /* Create task for bottom processing */
    g_hworker_tid = CREATE_THRD_N_QUEUE(hq, hw, hworker_entry, HW_STACK_SIZE, HWORKER_PRI);
    AIC_ASSERT(g_hworker_tid != AOS_INVALID_THREAD_ID);
    g_lworker_tid = CREATE_THRD_N_QUEUE(lq, lw, lworker_entry, LW_STACK_SIZE, LWORKER_PRI);
    AIC_ASSERT(g_lworker_tid != AOS_INVALID_THREAD_ID);
    /* Clear and disable all int */
    for (s32 i = 0; i < INT_MAXIUM; i++)
    {
        NVIC_DisableIRQ(i);
        NVIC_ClearPendingIRQ(i);
        NVIC_SetVector(i, (uint32_t)irq_handler);
    }
    vtor_addr = __SCB_GetVector_Addr();
    dcache_clean_by_range(vtor_addr, ((INT_MAXIUM + 16) * 4));

    return AIC_SUCCESS;
}

/**
 * irq_handler() - IRQ handler
 *
 * Execute ISR, and send msg to ISR task if necessary.
 *
 */
void irq_handler(void)
{
    IRQn_Type int_num;
    UISR_FUNC uisr;
    void *param;
    ISR_RET_E ret;

    /* Get exception number */
    int_num = __get_IPSR()&IPSR_ISR_Msk;
    AIC_ASSERT(int_num >=16 && int_num < INT_MAXIUM + 16);
    int_num -= 16;

#ifdef IRQ_DEBUG
        g_dbgirq_record[int_num].count++;
        g_dbgirq_record[int_num].last_time = aic_get_timestamp();
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

    aos_insert_isr_exit((u32)int_num);
#ifdef IRQ_DEBUG
        g_dbgirq_record[int_num].time_cost = aic_get_timestamp() - g_dbgirq_record[int_num].last_time;
#endif

    return;
}

__attribute__ ((section(".idle_data")))
u32 g_nvic_bkarea[16 + 16] = {0};

/**
 * irq_suspend() - Process suspend.
 *
 * Do suspend processing.
 *
 */
void irq_suspend(u32 *addr)
{
    u32 *st_addr;
    u32 i;

    if (addr != AIC_NULL)
        st_addr = addr;
    else
        st_addr = (u32 *)g_nvic_bkarea;

    for (i = 0; i < 16; i++)
        *st_addr++ = NVIC->ISER[i];
    for (i = 0;i < 16; i++)
        *st_addr++ = NVIC->ITNS[i];

    return;
}

__attribute__ ((section(".idle_text")))
/**
 * irq_resume() - Prcoess resume.
 *
 * Do resume processing.
 *
 */
void irq_resume(u32 *addr)
{
    u32 *st_addr;
    u32 i;

    if (addr != AIC_NULL)
        st_addr = addr;
    else
        st_addr = (u32 *)g_nvic_bkarea;

    for (i = 0; i < 16; i++)
        NVIC->ISER[i] = *st_addr++;
    for (i = 0;i < 16; i++)
        NVIC->ITNS[i] = *st_addr++;
    for (i = 0; i < INT_MAXIUM; i++) {
        u32 *vectors = (u32 *)(*(u32 *)0xE000ED08);
        vectors[i + 16] = (u32)irq_handler;
        __DSB();
    }

    return;
}
