/* adapter_alloc.h
 *
 * FreeRTOS implementation of the Adapter buffer allocation functionality.
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

#ifndef INCLUDE_GUARD_ADAPTER_ALLOC_H
#define INCLUDE_GUARD_ADAPTER_ALLOC_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// FreeRTOS interfaces
#include "FreeRTOS.h"
#include "portable.h"   /* pvPortMalloc, vPortFree */


/*----------------------------------------------------------------------------
  Adapter_Alloc
*/
static inline void *
Adapter_Alloc(unsigned int size)
{
    /* Many callers do not check if the requested size is non-zero and to be
     * sure that the function has a predictable behavior it is test here as
     * well because pvPortMalloc() may return a non-NULL value for size = 0. */
    if (size > 0)
    {
        return pvPortMalloc(size);
    }
    return NULL;    /* ... only extreme optimists do not check the result! */
}


/*----------------------------------------------------------------------------
  Adapter_Free
*/
static inline void
Adapter_Free(void *p)
{
    if (p != NULL)
    {
        vPortFree(p);   /* vPortFree() may take a NULL pointer but why bother */
        p = NULL;
    }
}


#endif /* INCLUDE_GUARD_ADAPTER_ALLOC_H */

/* end of file adapter_alloc.h */
