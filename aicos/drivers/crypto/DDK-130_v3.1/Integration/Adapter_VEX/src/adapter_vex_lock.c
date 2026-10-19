/* adapter_vex_lock.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the locking specific functions.
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

#include "adapter_vex_internal.h"   // API implementation


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_adapter_vex.h"          // configuration

#include "adapter_lock.h"           // Adapter_Lock_t, Adapter_Lock*()
#include "adapter_sleep.h"          // Adapter_SleepMS()


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * Local variables
 */

// Lock and critical section for vex_Init/Uninit()
static Adapter_Lock_t gl_VexInitLock;
static Adapter_Lock_CS_t gl_VexInitCS;


/*----------------------------------------------------------------------------
 * vex_LockInit
 */
int
vex_LockInit(void)
{
    // Initialize lock mechanism
    gl_VexInitLock = Adapter_Lock_Alloc();

    if (gl_VexInitLock == Adapter_Lock_NULL)
        return -1;

    Adapter_Lock_CS_Set(&gl_VexInitCS, gl_VexInitLock);

    return 0;
}


/*----------------------------------------------------------------------------
 * vex_LockExit
 */
void
vex_LockExit(void)
{
    Adapter_Lock_Free(gl_VexInitLock);

    Adapter_Lock_CS_Set(&gl_VexInitCS, Adapter_Lock_NULL);
}


/*----------------------------------------------------------------------------
 * vex_LockAcquire
 */
int
vex_LockAcquire(void)
{
    while (!Adapter_Lock_CS_Enter(&gl_VexInitCS))
    {
        Adapter_SleepMS(0);
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * vex_LockRelease
 */
void
vex_LockRelease(void)
{
    Adapter_Lock_CS_Leave(&gl_VexInitCS);
}


/* end of file adapter_vex_lock.c */

