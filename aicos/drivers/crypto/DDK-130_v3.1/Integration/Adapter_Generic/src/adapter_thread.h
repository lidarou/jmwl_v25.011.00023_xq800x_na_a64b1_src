/*
 * adapter_thread.h
 *
 *  This file implements the firing of new thread
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

#ifndef INCLUDE_GUARD_ADAPTER_THREAD_H
#define INCLUDE_GUARD_ADAPTER_THREAD_H

#include "basic_defs.h"                 /* bool */

/*----------------------------------------------------------------------------
 * Adapter_ThreadStart
 *
 * Start a new thread
 *
 * pThreadFunc (input):
 *      Pointer to the function that should be started as a thread.
 *
 * pThreadData (input):
 *      Data arguments passed to the thread.
 *
 * ThreadName (input):
 *      Name of the thread to start.
 *
 * Return value:
 *     Pointer to the thread reference or
 *     NULL when thread could not be started.
 */
void *
Adapter_ThreadStart(void * pThreadFunc,
                    void * pThreadData,
                    const char ThreadName[]);

/*----------------------------------------------------------------------------
 * Adapter_ThreadStop
 *
 * Stops the specified thread or all threads
 *
 * pThreadRef (input):
 *      Pointer to a thread reference, returned by Adapter_ThreadStart() or
 *      NULL if all started threads needs to stopped.
 *
 * Return value:
 *     none
 */
void
Adapter_ThreadStop(void * pThreadRef);

/*----------------------------------------------------------------------------
 * Adapter_ThreadSetCancelState
 */
void
Adapter_ThreadSetCancelState(void);

/*----------------------------------------------------------------------------
 * Adapter_ThreadShouldStop
 *
 * Requery for the thread, which indicates if it should stop.
 *
 * Return value:
 *     boolean: true yes, false no
 */
bool
Adapter_ThreadShouldStop(void);

/*----------------------------------------------------------------------------
 * Adapter_ThreadYield
 *
 * Causes the calling thread to relinquish the CPU.
 */
void
Adapter_ThreadYield(void);


/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrier
 *
 * This function inserts a hardware memory barrier that prevents any memory
 * access from being moved and executed on the other side of the barrier. It
 * guarantees that any memory access initiated before the memory barrier will
 * be complete before passing the barrier, and all subsequent memory accesses
 * will be executed after the barrier.
 */
void
Adapter_ThreadMemoryBarrier(bool Enter);

/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrierRead
 *
 * This function inserts a hardware memory barrier that prevents any memory
 * read access from being moved and executed on the other side of the barrier.
 * It guarantees that any memory read access initiated before the memory
 * barrier will be complete before passing the barrier, and all subsequent
 * memory read accesses will be executed after the barrier.
 */
void
Adapter_ThreadMemoryBarrierRead(void);

/*----------------------------------------------------------------------------
 * Adapter_ThreadMemoryBarrierWrite
 *
 * This function inserts a hardware memory barrier that prevents any memory
 * write access from being moved and executed on the other side of the barrier.
 * It guarantees that any memory write access initiated before the memory
 * barrier will be complete before passing the barrier, and all subsequent
 * memory write accesses will be executed after the barrier.
 */
void
Adapter_ThreadMemoryBarrierWrite(void);


#endif /* INCLUDE_GUARD_ADAPTER_THREAD_H */

/* end of file adapter_thread.h */
