/* adapter_interrupts.c
 *
 * Linux user-space implementation EIP201 (AIC) functionality of the
 * Adapter_Interrupt API.  Spawns a thread using the UMDevXSProxy
 * interrupt reads, which call into the single interrupt handler
 * routine gl_InterruptHandler from within that thread.
 *
 * The interrupt handler thread can be slept ("disabled".)  All
 * communication happens through the flags member of
 * gl_InterruptContext.  It is only ever written by external thread:
 * there is no protection, and only read from the interrupt thread
 * (this works from a memory visibility point as
 * Adapter_Event_Signal() will take and release the mutex.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : DDK-130_bsd                                              */
/*   Version       : 3.1                                                      */
/*   Configuration : DDK_EIP130_BSD                                           */
/*                                                                            */
/*   Date          : 2022-Feb-14                                              */
/*                                                                            */
/* Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright          */
/* notice, this list of conditions and the following disclaimer.              */
/*                                                                            */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/* notice, this list of conditions and the following disclaimer in the        */
/* documentation and/or other materials provided with the distribution.       */
/*                                                                            */
/* 3. Neither the name of the copyright holder nor the names of its           */
/* contributors may be used to endorse or promote products derived from       */
/* this software without specific prior written permission.                   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR      */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT           */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,      */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.       */
/* -------------------------------------------------------------------------- */

#include "adapter_interrupts.h"
#include "adapter_interrupts_ext.h"
#include <stdlib.h>
#include <string.h>
#include "eip201.h"
#include "device_mgmt.h"
#include "umdevxsproxy_interrupt.h"     /* UMDevXSProxy_Interrupt_WaitWithTimeout */
#include "adapter_event.h"              /* adapter_Event_Wait/Signal */
#include "adapter_thread.h"             /* Adapter_ThreadStart/Stop */
#include "adapter_sleep.h"              /* Adapter_SleepMS */

#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_WARN
#endif
#include "log.h"                        /* LOG_* */

enum
{
    k_irq_alive = 1 << 0,
    k_irq_enabled = 1 << 1,
};

static Device_Handle_t gl_Aic = NULL;
static Adapter_InterruptHandler_t gl_InterruptHandler = NULL;

struct InterruptContext
{
    int InterruptId;
    int flags;                          /* atomic */
    unsigned int irqCount;
    Adapter_Event_t event;
};
static struct InterruptContext gl_InterruptContext = { 0 };
static void * gl_pInterruptThread = NULL;

/* Having it as a variable will allow adjustment (only via debugger
 * currently.)  The value balances between unnecessary wakeups, and
 * evaluation of the finishing condition (joins can take up to this
 * time.)
 */
static unsigned int gl_PollPeriod = 1000;


/* There is a problem with EIP201_SOURCE_ALL being set as -1, which
 * will cause assertion failures in the calls!
 */
#undef EIP201_SOURCE_ALL
#define EIP201_SOURCE_ALL 0xff


/*!
 * Wait for an event on the interrupt context; either (or both)
 * - enabled
 * - killed (not alive), this is inverted from the flags
 * The function is used as an argument to Adapter_Events_Wait()
 * @return flags to act on, 0 if no action (continue sleep)
 */
static int
WaitReady(void * pvArg)
{
    struct InterruptContext * args = pvArg;
    Adapter_ThreadShouldStop();
    return ((args->flags) & (k_irq_enabled | k_irq_alive)) ^ k_irq_alive;
}


/* The thread is started in running state (default options.)  The
 * argument contains flag for the execution of this thread.  The
 * thread will run for as long as flags has the \a k_irq_alive bit
 * set.  State changes shall be accompanied by a setting of the event
 * to re-read the conditions.
 * @param pvArg arguments of type InterruptContext
 */
static void
Interrupts_ThreadFunc(void * pvArg)
{
    struct InterruptContext * args = pvArg;
    EIP201_SourceBitmap_t irqs;

    /* The thread (adapter) will go into the following states:
     *  - disabled: wait for the argument event to be woken up
     *  - enabled: poll for timeout on the interrupt: a disable won't
     *    have an effect until done
     *
     * We shall be able to handle spurious wakeups, the test for the
     * flags should always be done with the mutex held (the setting
     * should factually be as well.)
     */
    Adapter_ThreadSetCancelState();

    while ((args->flags & (k_irq_alive)) != 0)
    {
        (void)Adapter_Event_Wait(args->event, WaitReady, args, -1);
        if (((args->flags & (k_irq_alive)) != 0) &&
            ((args->flags & (k_irq_enabled)) != 0) &&
            (UMDevXSProxy_Interrupt_WaitWithTimeout(gl_PollPeriod,
                                                    args->InterruptId) >= 0))
        {
            /* The interrupt is signaled, but not disabled by the caller,
             * -- that must be done here already, and should traverse all
             * interrupts with the handler as well. New edges will result
             * in another interrupt taken.
             */
            irqs = EIP201_SourceStatus_ReadAllEnabled(gl_Aic);
            if (irqs != 0)
            {
                EIP201_Acknowledge(gl_Aic, irqs); /* clear edge detect */
                args->irqCount++;

                if (gl_InterruptHandler != NULL)
                {
                    (*gl_InterruptHandler)(args->InterruptId, 0u);
                }
            }
        }
    }
}


int
Adapter_Interrupts_Init(int irq)
{
    int rval = -1;

    memset(&gl_InterruptContext, 0, sizeof(gl_InterruptContext));

    gl_Aic = Device_Find("EIP130_AIC");
    if (gl_Aic != NULL)
    {
        gl_InterruptContext.event = Adapter_Event_Alloc();
        if (gl_InterruptContext.event)
        {
            /* Configure them all for an edge detect.  We should probably
             * only have the interrupts we need.  We could also use
             * EIP201_Config_Change(), but there's no adapter function(!)
             */
            EIP201_SourceSettings_t settings = {
                .Source = EIP201_SOURCE_ALL,
                .Config = EIP201_CONFIG_RISING_EDGE,
                .fEnable = false /* enable source only when active */
            };
            if (EIP201_Initialize(gl_Aic, &settings, 1) == 0)
            {
                struct InterruptContext *const p = &gl_InterruptContext;

                p->flags = k_irq_enabled | k_irq_alive; /* Set alive and trigger */
                gl_pInterruptThread = Adapter_ThreadStart(Interrupts_ThreadFunc, p, "Interrupts");
                if (gl_pInterruptThread != NULL)
                {
                    rval = 0;
                }
                else
                {
                    rval = -4;
                }
            }
            else
            {
                rval = -3;
            }
        }
        else
        {
            rval = -2;
        }
    }

    if (rval != 0)
    {
        if (gl_InterruptContext.event != NULL)
        {
            Adapter_Event_Free(gl_InterruptContext.event);
            gl_InterruptContext.event = NULL;
        }
    }
    else if (irq > 0)
    {
        /* Return irq value */
        rval = irq;
    }

    LOG_WARN("%s(%d) [%p] = %d\n", __func__, irq, gl_Aic, rval);

    return rval;
}


int
Adapter_Interrupts_UnInit(const int irq)
{
    LOG_WARN("%s(irq#%d) [%p] = %u\n",
             __func__, irq, gl_Aic, gl_InterruptContext.irqCount);

    if (gl_Aic != NULL)
    {
        EIP201_SourceMask_DisableSource(gl_Aic, EIP201_SOURCE_ALL);

        if (gl_pInterruptThread != NULL)
        {
            Adapter_ThreadStop(gl_pInterruptThread);
            gl_InterruptContext.flags &= ~(k_irq_alive); /* Clear alive */
            Adapter_Event_Signal(gl_InterruptContext.event);
            gl_pInterruptThread = NULL;

            /* Wait to ensure that the thread has finished */
            Adapter_SleepMS(gl_PollPeriod+50);
        }
        Adapter_Event_Free(gl_InterruptContext.event);
        gl_InterruptContext.event = NULL;
        gl_Aic = NULL;
    }

    return irq;
}


int
Adapter_Interrupt_SetHandler(const int irq,
                             Adapter_InterruptHandler_t f)
{
    (void)irq;
    gl_InterruptHandler = f;            /* atomic */
    return 0;
}


int
Adapter_Interrupt_Context_Enable(const int irq,
                                 const unsigned flags)
{
    (void)irq;
    (void)flags;
    Adapter_Event_Signal(gl_InterruptContext.event);
    return 0;
}


int
Adapter_Interrupt_Clear(const int irq,
                        const unsigned int flags)
{
    (void)flags;
    if (gl_Aic)
    {
        return EIP201_Acknowledge(gl_Aic, 1u << irq);
    }
    return -1;
}


int
Adapter_Interrupt_ClearAndEnable(const int nIRQ,
                                 const unsigned int Flags)
{
    EIP201_SourceBitmap_t mask = 1u << nIRQ;
    int rc;

    EIP201_Acknowledge(gl_Aic, mask);
    rc = EIP201_SourceMask_EnableSource(gl_Aic, mask);
    if (rc >= 0)
    {
        gl_InterruptContext.flags |= k_irq_enabled; /* is it? */
        Adapter_Event_Signal(gl_InterruptContext.event);
    }
    else
    {
        (void)Flags;
    }
    return rc;
}


int
Adapter_Interrupt_Disable(const int nIRQ,
                          const unsigned int Flags)
{
    EIP201_SourceBitmap_t mask = 1u << nIRQ;
    int rc;

    rc = EIP201_SourceMask_DisableSource(gl_Aic, mask);
    if (rc >= 0)
    {
        gl_InterruptContext.flags &= ~(k_irq_enabled);
        Adapter_Event_Signal(gl_InterruptContext.event);
    }
    else
    {
        (void)Flags;
    }
    return rc;
}


/*
 * This function disables execution context (allows it to run) that services
 * the nIRQ interrupt.
 *
 * Note: this function is different from the Adapter_Interrupt_Disable()
 *       function which only disables the nIRQ interrupt event at an interrupt
 *       controller. The interrupt can be dispatched to some execution context
 *       (for example, a thread) that can be also enabled or disabled.
 */
int
Adapter_Interrupt_Context_Disable(const int nIRQ,
                                  const unsigned int Flags)
{
    (void)nIRQ;
    (void)Flags;
    /* stop the dispatch thread and remove everything matching from wq */
    Adapter_Event_Signal(gl_InterruptContext.event);
    return 0;
}


int
Adapter_Interrupts_Resume(void)
{
    /* We should be able to wake up the interrupt thread with the same
       flag.  Keep the current system unchanged, -- which means that
       we'd really better use an event flag with a condition.
    */
    gl_InterruptContext.flags |= k_irq_enabled;
    Adapter_Event_Signal(gl_InterruptContext.event);
    return 0;
}


int
Adapter_Interrupt_Ext_RawStatus(void)
{
    EIP201_SourceBitmap_t sources = 0;
    EIP201_Status_t status = EIP201_SourceStatus_ReadAllRawCheck(gl_Aic, &sources);
    if (status)
    {
        return -abs((int)status);
    }
    return (int)sources;
}


int
Adapter_Interrupt_Ext_Status(void)
{
    return 0;
    EIP201_SourceBitmap_t sources = 0;
    EIP201_Status_t status = EIP201_SourceStatus_ReadAllEnabledCheck(gl_Aic, &sources);
    if (status)
    {
        return -abs((int)status);
    }
    return (int)sources;
}


/* end of file adapter_interrupts.c */
