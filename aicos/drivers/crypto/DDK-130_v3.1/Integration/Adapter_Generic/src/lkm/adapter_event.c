/* adapter_event.c
 *
 * Linux kernel-space implementation of the Adapter buffer allocation
 * functionality.
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

#include "adapter_event.h"
#include "adapter_alloc.h"
#include <linux/sched.h>
#include <linux/wait.h>


struct Adapter_Event
{
    wait_queue_head_t q;
};


Adapter_Event_t
Adapter_Event_Alloc(void)
{
    struct Adapter_Event * ev = Adapter_Alloc(sizeof(struct Adapter_Event));
    if (ev != NULL)
    {
        init_waitqueue_head(&ev->q);
    }
    return ev;
}


void
Adapter_Event_Free(Adapter_Event_t ev)
{
    if (ev != NULL)
    {
        Adapter_Free(ev);
    }
}


int
Adapter_Event_Wait(Adapter_Event_t ev,
                   int (*checkf)(void *),
                   void * context, int timeout)
{
    int rc;

    if (ev != NULL)
    {
        rc = wait_event_interruptible_timeout(ev->q,
                                              checkf(context) != 0,
                                              msecs_to_jiffies(timeout));
    }
    else
    {
        rc = 0;
    }
    return rc;
}


/*!
 * Signal the "event" flag. All waiters are woken up and will re-evaluate
 * conditions.
 */
void
Adapter_Event_Signal(Adapter_Event_t ev)
{
    if (ev != NULL)
    {
        wake_up(&ev->q);
    }
}
