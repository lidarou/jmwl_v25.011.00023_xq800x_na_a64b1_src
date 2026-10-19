/* adapter_event.h
 *
 * Linux user-space implementation of the Adapter interrupt dispatch
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
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>


enum
{
    k_alive = 1 << 0
};


/*
 * Would really want ot pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) and
 * set it.
 */
struct Adapter_Event
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    unsigned int flags; /* alive flag */
};


static int
EventInit(Adapter_Event_t ev)
{
    pthread_condattr_t attr;
    int rval;

    rval = pthread_condattr_init(&attr);
    if (rval == 0)
    {
        rval = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
        if (rval == 0)
        {
            rval = pthread_cond_init(&ev->cond, &attr);
            if (rval == 0)
            {
                rval = pthread_mutex_init(&ev->mutex, NULL);
                if (rval == 0)
                {
                    ev->flags = k_alive;
                }
                else
                {
                    pthread_cond_destroy(&ev->cond);
                }
            }
        }
        else
        {
            /* couldn't set to monotonic clock */
        }
        pthread_condattr_destroy(&attr);
    }
    rval = -abs(rval); /* return something < 0 on failure */
    return rval;
}


Adapter_Event_t
Adapter_Event_Alloc(void)
{
    struct Adapter_Event * ev = Adapter_Alloc(sizeof(struct Adapter_Event));
    if (ev != NULL)
    {
        if (EventInit(ev) < 0)
        {
            Adapter_Free(ev);
            ev = NULL;
        }
    }
    return ev;
}


void
Adapter_Event_Free(Adapter_Event_t ev)
{
    if (ev != NULL)
    {
        pthread_mutex_lock(&ev->mutex);
        ev->flags = 0;

        /* A condition variable can be destroyed immediately after all
           threads blocking on it are awakened. This sequence is from
           the example for pthread_cond_destroy() on OpenGroup.
        */
        pthread_cond_broadcast(&ev->cond);
        pthread_mutex_unlock(&ev->mutex);
        pthread_cond_destroy(&ev->cond);
        Adapter_Free(ev);
    }
}


int
Adapter_Event_Wait(Adapter_Event_t ev,
                   int (*checkf)(void *),
                   void * context, int timeout)
{
    int r = 0;

    if (ev != NULL)
    {
        struct timespec ts;
        int rc = 0;

        if (timeout > 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &ts);
            long long ns = ts.tv_nsec + timeout * 1000000ll;
            lldiv_t q = lldiv(ns, 1000000000LL);
            ts.tv_sec += (time_t)q.quot;
            ts.tv_nsec = (long)q.rem;
        }

        pthread_mutex_lock(&ev->mutex);
        while (((r = checkf(context)) == 0) &&
               (rc == 0) &&
               (ev->flags & k_alive))
        {
            if (timeout > 0)
            {
                /* wait for a certain time */
                rc = pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts);
            }
            else
            {
                /* wait indefinitely */
                rc = pthread_cond_wait(&ev->cond, &ev->mutex);
            }
        }
        pthread_mutex_unlock(&ev->mutex);
    }
    return r;
}


/* Broadcast the event. All waiters are woken up; expected is
 * practically one, and in any case, the conditions will be
 * re-evaluated by the waiting tasks.
 */
void
Adapter_Event_Signal(Adapter_Event_t ev)
{
    if (ev != NULL)
    {
        pthread_mutex_lock(&ev->mutex);
        pthread_cond_broadcast(&ev->cond); /* hello everybody */
        pthread_mutex_unlock(&ev->mutex);
    }
}
