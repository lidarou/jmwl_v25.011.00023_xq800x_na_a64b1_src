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
#define AIC_THIS_FILE    0x09a747
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>
#include "los_interrupt.h"
#ifndef WIN32
#include <irq.h>
#endif

u32 aos_disable_irq(void)
{
    return LOS_IntLock();
}

void aos_restore_irq(u32 save)
{
    LOS_IntRestore(save);
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

#if 0
void aos_request_irq(int irq, int (*handler)(void *p), void *p)
{
    (void)LOS_HwiCreate(irq, 0, 0, (HWI_PROC_FUNC)handler, p);
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
    LOS_HwiDelete(irq, 0);
}
#endif
