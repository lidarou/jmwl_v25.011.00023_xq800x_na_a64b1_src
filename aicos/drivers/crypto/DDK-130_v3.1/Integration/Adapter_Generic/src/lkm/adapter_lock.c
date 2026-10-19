/* adapter_lock.c
 *
 * Adapter concurrency (locking) management
 * Linux kernel-space implementation
 *
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Adapter locking API
#include "adapter_lock.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // IDENTIFIER_NOT_USED

// Adapter Lock Internal API
#include "adapter_lock_internal.h"

// Adapter Memory Allocation API
#include "adapter_alloc.h"

// Logging API
#undef LOG_SEVERITY_MAX
#define LOG_SEVERITY_MAX    LOG_SEVERITY_WARN
#include "log.h"

// Linux Kernel API
#include <linux/spinlock.h>     // spinlock_*


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Adapter_Lock_Alloc
 */
Adapter_Lock_t
Adapter_Lock_Alloc(void)
{
    spinlock_t * Lock_p;
    size_t LockSize = sizeof(spinlock_t);

    if (LockSize == 0)
    {
        LockSize = 4;
    }

    Lock_p = Adapter_Alloc(LockSize);
    if (Lock_p == NULL)
    {
        Lock_p = Adapter_Lock_NULL;
    }
    else
    {
        Log_FormattedMessage("%s: Lock = spinlock\n", __func__);

        spin_lock_init(Lock_p);
    }

    return Lock_p;
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Free
 */
void
Adapter_Lock_Free(Adapter_Lock_t Lock)
{
    if (Lock != Adapter_Lock_NULL)
    {
        Adapter_Free((void *)Lock);
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Acquire
 */
void
Adapter_Lock_Acquire(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    if (Lock != Adapter_Lock_NULL)
    {
        spin_lock_irqsave((spinlock_t *)Lock, *Flags);
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_Release
 */
void
Adapter_Lock_Release(
        Adapter_Lock_t Lock,
        unsigned long * Flags)
{
    if (Lock != Adapter_Lock_NULL)
    {
        spin_unlock_irqrestore((spinlock_t *)Lock, *Flags);
    }
}


/* end of file adapter_lock.c */
