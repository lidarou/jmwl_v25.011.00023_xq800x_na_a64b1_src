/* adapter_lock_internal.c
 *
 * Adapter concurrency (locking) management
 * Generic implementation
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
#include "basic_defs.h"         // bool

// Adapter Lock Internal API
#include "adapter_lock_internal.h"

#define TEST_SIZEOF(type, size) \
    extern int must_bigger[1 - 2*((int)(sizeof(type) > size))]

// Check the size of the data structures
TEST_SIZEOF(Adapter_Lock_CS_Internal_t, sizeof(Adapter_Lock_CS_t));


/*----------------------------------------------------------------------------
 * Adapter_Lock_NULL
 *
 */
const Adapter_Lock_t Adapter_Lock_NULL = NULL;


/*----------------------------------------------------------------------------
 * Adapter_Lock_CS_Set
 */
void
Adapter_Lock_CS_Set(
        Adapter_Lock_CS_t * const CS_p,
        Adapter_Lock_t Lock)
{
    Adapter_Lock_CS_Internal_t * CS_Internal_p = (Adapter_Lock_CS_Internal_t*)CS_p;

    if (CS_Internal_p != NULL)
    {
        // Lock cannot be set while the critical section is entered
        if (CS_Internal_p->fLocked == false)
        {
            CS_Internal_p->Lock_p = Lock;
        }
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_CS_Get
 */
Adapter_Lock_t
Adapter_Lock_CS_Get(
        Adapter_Lock_CS_t * const CS_p)
{
    Adapter_Lock_CS_Internal_t * CS_Internal_p = (Adapter_Lock_CS_Internal_t*)CS_p;

    if (CS_Internal_p != NULL)
    {
        return CS_Internal_p->Lock_p;
    }
    return Adapter_Lock_NULL;
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_CS_Enter
 */
bool
Adapter_Lock_CS_Enter(
        Adapter_Lock_CS_t * const CS_p)
{
    Adapter_Lock_CS_Internal_t * CS_Internal_p = (Adapter_Lock_CS_Internal_t*)CS_p;
    bool rc = false;
    unsigned long Flags;

    if (CS_Internal_p != NULL)
    {
        // Enter critical section
        Adapter_Lock_Acquire(CS_Internal_p->Lock_p, &Flags);

        // Check if critical section is already entered
        if (CS_Internal_p->fLocked == false)
        {
            CS_Internal_p->fLocked = true;
            rc = true;
        }

        // Leave critical section
        Adapter_Lock_Release(CS_Internal_p->Lock_p, &Flags);
    }

    return rc;
}


/*----------------------------------------------------------------------------
 * Adapter_Lock_CS_Leave
 */
void
Adapter_Lock_CS_Leave(
        Adapter_Lock_CS_t * const CS_p)
{
    Adapter_Lock_CS_Internal_t * CS_Internal_p = (Adapter_Lock_CS_Internal_t*)CS_p;

    if (CS_Internal_p != NULL)
    {
        CS_Internal_p->fLocked = false;
    }
}


/* end of file adapter_lock_internal.c */
