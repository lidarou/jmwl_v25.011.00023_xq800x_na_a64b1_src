/* adapter_interrupts.c
 *
 * Linux kernel-space implementation EIP201 (AIC) functionality of the
 * Adapter_Interrupt API.
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
#include "eip201.h"
#include "device_mgmt.h"
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
// Logging API, should we include c_adapter....h
#ifndef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX LOG_SEVERITY_CRIT
#endif
#include "log.h"                    // LOG_*

enum
{
    k_num_irqs = 16
};

static Device_Handle_t s_aic = NULL;
static Adapter_InterruptHandler_t s_irqs[k_num_irqs];
static unsigned s_irqs_enabled = 0;
static unsigned s_irq_count = 0;


/*!
 * Main host interrupt from AIC connected to this host.
 */
// static void irqf(void)          /* irqhandler_t */
irqreturn_t
irqf(const int irqno,
     void * context)
{
    unsigned i = 0, mask = 1;   /* C90 compatible pre-declaration */
    EIP201_SourceBitmap_t irqs = EIP201_SourceStatus_ReadAllEnabled(s_aic);

    EIP201_Acknowledge(s_aic, irqs); /* clear edge detect */
    ++s_irq_count;
    for (i = 0, mask = 1; (i < k_num_irqs) && irqs; ++i, mask <<= 1)
    {
        if (irqs & mask)
        {
            Adapter_InterruptHandler_t f = s_irqs[i];
            if (f)
            {
                /* Either call the interrupt directly, or signal a
                 * work thread to invoke the function.  This function
                 * is already the bottom end handler, -- run it
                 * directly.
                 */
                f((int)i, 0);   /* or WORKQUEUE_.... */
            }
            else
            {
                /* warn, we don't disable however. */
            }
        }
    }
    return irqs ? IRQ_HANDLED : IRQ_NONE;
}


#undef CHECKEMALL
#ifdef CHECKEMALL
static unsigned s_irqs_taken[256/32];
static unsigned s_registered[256/32];

irqreturn_t
irqfr(const int irqno,
      void * context)
{
    // return this one...
    s_irqs_taken[irqno / 32] |= 1u << (irqno % 32);
    (void)context;
    return IRQ_NONE;
}

static unsigned s_token;

static void
RequestAllIrqs(unsigned flags,
               unsigned irq)
{
    memset(s_irqs_taken, 0, sizeof(s_irqs_taken));
    for (; irq < 256; ++irq)
    {
        if (request_irq(irq, irqfr, flags, "EIP130_test", &s_token) == 0)
        {
            s_registered[irq / 32] |= (1u << (irq % 32));
        }
    }
}


static void
CheckAllIrqs(void)
{
    unsigned irq;

    for (irq = 0; irq < 256; ++irq)
    {
        if (s_irqs_taken[irq / 32] & (1u << (irq % 32)))
        {
            LOG_CRIT("irq#%u\n", irq);
        }
    }
}


static void
FreeAllIrqs(void)
{
    unsigned irq;

    for (irq = 0; irq < 256; ++irq)
    {
        if (s_registered[irq / 32] & (1u << (irq % 32)))
            free_irq(irq, &s_token);
    }
    memset(s_irqs_taken, 0, sizeof(s_irqs_taken));
    memset(s_registered, 0, sizeof(s_registered));
}


#else
static void
RequestAllIrqs(unsigned flags,
               unsigned start)
{
    (void)flags; (void)start;
}
static void CheckAllIrqs(void) { }
static void FreeAllIrqs(void) { }
#endif

/* This is again uglyness, -- just use something "ok"
 */
#undef EIP201_SOURCE_ALL
#define EIP201_SOURCE_ALL 0xff


int
Adapter_Interrupts_Init(int irq)
{
    int rval = -1;

    s_irq_count = 0;
    s_aic = Device_Find("EIP130_AIC");
    if ((s_aic != NULL) && (irq >= 0))
    {
        /* Configure them all for an edge detect.  We should
         * probably only have the interrupts we need.  We could
         * also use EIP201_Config_Change(), but there's no adapter
         * function(!)
         */
        EIP201_SourceSettings_t settings = {
            .Source = EIP201_SOURCE_ALL,         /* or, only ours */
            .Config = EIP201_CONFIG_RISING_EDGE, /* active high etc. */
            .fEnable = false // later
        };
        rval = EIP201_Initialize(s_aic, &settings, 1);
        LOG_CRIT("EIP201_Initialize() = %d\n", rval);
        if (rval == 0)
        {
            // mode can only be either HIGH or RISING, 0 to leave as is
            unsigned flags = IRQF_TRIGGER_HIGH | IRQF_SHARED;
            rval = request_irq(irq, irqf, flags, "EIP130_AIC", s_aic);
            RequestAllIrqs(flags, irq);
            if (rval != 0)
            {
                LOG_CRIT("%s: request_irq(%d) failed %d\n",
                         __func__, irq, rval);
            }
            // irq is enabled
        }
    }

    // Transform return value into either irq, or a negative error value
    if (rval != 0)
    {
        rval = -abs(rval);
    }
    else
    {
        rval = irq;
    }

    LOG_CRIT("%s(%d) [%p] = %d\n", __func__, irq, s_aic, rval);

    return rval;
}


int
Adapter_Interrupts_UnInit(const int irq)
{
    LOG_CRIT("%s(irq#%d) [%p], irqs = %d\n", __func__, irq, s_aic, s_irq_count);
    if (irq >= 0)
    {
        EIP201_SourceMask_DisableSource(s_aic, EIP201_SOURCE_ALL);
        free_irq((unsigned)irq, s_aic);
    }
    FreeAllIrqs();
    return irq;
}


int
Adapter_Interrupt_SetHandler(const int irq,
                             Adapter_InterruptHandler_t f)
{
    int rval = -1;
    if ((irq >= 0) && (irq < k_num_irqs))
    {
        s_irqs[irq] = f;
        rval = irq;
    }
    return rval;
}


int
Adapter_Interrupt_Context_Enable(const int irq,
                                 const unsigned flags)
{
    (void)flags;
    s_irqs_enabled &= (1u << irq); /* run the context in here */
    return 0;
}


int
Adapter_Interrupt_Clear(const int irq,
                        const unsigned int flags)
{
    (void)flags;
    return EIP201_Acknowledge(s_aic, 1u << irq);
}


int
Adapter_Interrupt_ClearAndEnable(const int irq,
                                 const unsigned int flags)
{
    EIP201_SourceBitmap_t mask = 1u << irq, r;
    EIP201_Acknowledge(s_aic, mask);
    r = EIP201_SourceMask_EnableSource(s_aic, mask);
    (void)flags;
    return r;
}


int
Adapter_Interrupt_Disable(const int nIRQ,
                          const unsigned int Flags)
{
    EIP201_SourceBitmap_t mask = 1u << nIRQ;
    (void)Flags;
    return EIP201_SourceMask_DisableSource(s_aic, mask);
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
    s_irqs_enabled &= ~(1u << nIRQ);
    return 0;
}


int
Adapter_Interrupts_Resume(void)
{
    return 0;
}


int
Adapter_Interrupt_Ext_RawStatus(void)
{
    EIP201_SourceBitmap_t sources = 0;
    EIP201_Status_t status = EIP201_SourceStatus_ReadAllRawCheck(s_aic, &sources);
    if (status)
    {
        return -abs((int)status);
    }
    return (int)sources;
}


int
Adapter_Interrupt_Ext_Status(void)
{
    EIP201_SourceBitmap_t sources = 0;
    EIP201_Status_t status = EIP201_SourceStatus_ReadAllEnabledCheck(s_aic, &sources);
    CheckAllIrqs();
    if (status)
    {
        return -abs((int)status);
    }
    return (int)sources;
}


/* end of file adapter_interrupts.c */
