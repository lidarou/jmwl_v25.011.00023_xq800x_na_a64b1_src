/* adapter_event.h
 *
 * Data types and Interfaces
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


#ifndef ADAPTER_EVENT_H
#define ADAPTER_EVENT_H


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * Abstract event / condition handler.
 */
typedef struct Adapter_Event * Adapter_Event_t;


/*!
 * Function for condition evaluation. The function will take an
 * argument as supplied to Adapter_Event_Wait(). The function shall
 * return:
 *   0 to continue waiting
 * < 0 for an irrecoverable error (returned to caller)
 * > 0 when finished waiting
 */
typedef int (*Adapter_Event_Eval_t)(void *);


/*!
 * Allocate a new event.
 * @return newly allocated non-reported event, or NULL
 */
Adapter_Event_t
Adapter_Event_Alloc(void);


/*!
 * Free an event.
 * @param event event allocated by Adapter_Event_Alloc()
 */
void
Adapter_Event_Free(Adapter_Event_t event);


/*!
 * Wait for the event to happen. The event being signaled is
 * either a waitq (kernel) or something else (but not a condition.)
 * @param event event
 * @param checkf function for checking condition, must return 0 to
 * continue waiting or > 0 when condition is reached
 * @param context context passed to checkf
 * @param timeout timeout in milliseconds
 * @return return value from checkf() or 0 for a timeout
 */
int
Adapter_Event_Wait(Adapter_Event_t event,
                   Adapter_Event_Eval_t checkf,
                   void * context,
                   int timeout);


/*!
 * Signal the "event" flag.  All waiters are woken up and will re-evaluate
 * conditions.  The return value depends
 * @param event event to wait
 */
void
Adapter_Event_Signal(Adapter_Event_t event);


#ifdef __cplusplus
}
#endif


#endif
