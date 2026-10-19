/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA interrupt function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13e064
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#ifndef WIN32
#include <irq.h>
#include <user_tracex_api.h>
#endif

#include <core_op.h>

#define RESTORE_INT 0x00
#define DISABLE_INT 0x01

u32 aos_disable_irq(void)
{
#ifdef WIN32
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE

    return tx_saved_posture;
#else
    unsigned int interrupt_save;

    interrupt_save = disable_interrupts();

#ifdef CONFIG_AOS_INT_DEBUG
    if (!aic_sys_is_assert())
        USER_TRACE_IN_LINE_INSERT(TX_TRACE_INTDBG_EVENT, DISABLE_INT, interrupt_save);
#endif

    return interrupt_save;
#endif
}

void aos_restore_irq(u32 save)
{
#ifdef WIN32
    TX_INTERRUPT_SAVE_AREA

    tx_saved_posture = save;
    TX_RESTORE
#else
#ifdef CONFIG_AOS_INT_DEBUG
    if (!aic_sys_is_assert())
        USER_TRACE_IN_LINE_INSERT(TX_TRACE_INTDBG_EVENT, RESTORE_INT, save);
#endif
    restore_interrupts(save);
#endif
}

#ifndef WIN32
void aos_request_irq(int irq, int (*handler)(void *p), void *p)
{
    (void)register_irq(irq, (UISR_FUNC)handler, p, NULL, NULL);
}

void aos_request_irq_with_half_bottom(int irq,
                                      int (*handler)(void *p),
                                      void (*half)(void *p),
                                      void *p)
{
    (void)register_irq(irq, (UISR_FUNC)handler, p, half, p);
}

void aos_release_irq(int irq)
{
    unregister_irq(irq);
}
#else
void aos_request_irq(int irq, int (*handler)(void *p), void *p)
{

}

void aos_request_irq_with_half_bottom(int irq,
                                      int (*handler)(void *p),
                                      void (*half)(void *p),
                                      void *p)
{

}

void aos_release_irq(int irq)
{
}
#endif

