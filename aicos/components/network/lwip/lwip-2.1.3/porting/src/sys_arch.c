/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13e28b
#define LOG_MODULE    AMOD_NET

#include <lwip/opt.h>
#include <lwip/arch.h>
#if !NO_SYS
#include "arch/sys_arch.h"
#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>

#include <string.h>
#include "aic_osa.h"
#ifdef CONFIG_RTOS_LITEOS_M
#include "los_task.h"
#endif

//u32_t lwip_sys_now;
#ifdef CONFIG_RTOS_THREADX
extern volatile UINT _tx_thread_preempt_disable;
#endif

u32_t sys_jiffies(void)
{
  return (u32_t)aic_get_tick();
}

u32_t sys_now(void)
{
  return (u32_t)aic_get_tick();
}

void sys_init(void)
{
    u32_t seedlsb = (u32_t)aic_get_tick();

    srand(seedlsb);
}

#ifdef CONFIG_RTOS_THREADX
sys_prot_t sys_arch_protect(void)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    _tx_thread_preempt_disable++;
    AOS_INTERRUPT_RESTORE

    return 0;
}

void sys_arch_unprotect(sys_prot_t val)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    _tx_thread_preempt_disable--;
    AOS_INTERRUPT_RESTORE
}
#endif

#ifdef CONFIG_RTOS_LITEOS_M
sys_prot_t sys_arch_protect(void)
{
    LOS_TaskLock();

    return 0;
}

void sys_arch_unprotect(sys_prot_t val)
{
    LOS_TaskUnlock();
}
#endif

#if !NO_SYS

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    aos_semaphore_ptr semaphore_ptr;

    if (!sem) {
        LWIP_DEBUGF(SYS_DEBUG, ("sem is invalid when new\r\n"));
        return ERR_ARG;
    }

    semaphore_ptr = aos_create_semaphore("lwip_sem", count);
    *sem = (sys_sem_t)semaphore_ptr;
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    if (!sem) {
        LWIP_DEBUGF(SYS_DEBUG, ("sem is invalid when free\r\n"));
        return;
    }

    aos_delete_semaphore((aos_semaphore_ptr)(*sem));
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    if (!sem) {
        LWIP_DEBUGF(SYS_DEBUG, ("sem is invalid when set invalid\r\n"));
        return;
    }

    *sem = 0;
}

int sys_sem_valid(sys_sem_t *sem)
{
    if (!sem || !*sem /*&& *(u32 *)(*sem) == AOS_SEMAPHORE_ID*/) {
        LWIP_DEBUGF(SYS_DEBUG ,("mutex is invalid\r\n"));
        return FALSE;
    }

    return TRUE;
}

/* semaphores are 1-based because RAM is initialized as 0, which would be valid */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    u32_t ret = 0;

    if (!sem || !*sem) {
        LWIP_DEBUGF(SYS_DEBUG ,("sem is invalid when wait\r\n"));
        return ERR_ARG;
    }

    if (!timeout)
        timeout = AOS_WAIT_FOREVER;

    ret = aos_get_semaphore((aos_semaphore_ptr)(*sem), timeout);
    if (!ret)
        return ERR_OK;

    LWIP_DEBUGF(SYS_DEBUG, ("wait semap fail 0x%x\r\n", ret));
    return SYS_ARCH_TIMEOUT;
}

void sys_sem_signal(sys_sem_t *sem)
{
    if (!sem || !*sem) {
        LWIP_DEBUGF(SYS_DEBUG ,("sem is invalid when signal\r\n"));
        return;
    }

    aos_put_semaphore((aos_semaphore_ptr)(*sem));
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    aos_mutex_ptr mutex_ptr;

    if (!mutex) {
        LWIP_DEBUGF(SYS_DEBUG ,("mutex is invalid when new\r\n"));
        return ERR_ARG;
    }

    mutex_ptr = aos_create_mutex("lwip_mutex");
    if (!mutex_ptr)
        return ERR_ARG;

    *mutex = (sys_mutex_t)mutex_ptr;

    return ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    /* parameter check */
    if (!mutex || !*mutex) {
        LWIP_DEBUGF(SYS_DEBUG ,("mutex is invalid when new\r\n"));
        return;
    }

    aos_delete_mutex((aos_mutex_ptr)(*mutex));
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
    if (!mutex || !*mutex /*|| *(u32 *)(*mutex) != AOS_MUTEX_ID*/) {
        LWIP_DEBUGF(SYS_DEBUG , ("mutex is invalid when valid\r\n"));
        return FALSE;
    }

    return TRUE;
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    if (!mutex) {
        LWIP_DEBUGF(SYS_DEBUG , ("mutex is invalid when invalid\r\n"));
        return;
    }

    *mutex = 0;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    if (!mutex || !*mutex) {
        LWIP_DEBUGF(SYS_DEBUG , ("mutex is invalid when lock\r\n"));
        return;
    }

    /* we count up just to check the correct pairing of lock/unlock */
    aos_get_mutex((aos_mutex_ptr)(*mutex), AOS_WAIT_FOREVER);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    if (!mutex || !*mutex) {
        LWIP_DEBUGF(SYS_DEBUG, ("mutex is invalid when unlock\r\n"));
        return;
    }

    /* we count down just to check the correct pairing of lock/unlock */
    aos_put_mutex((aos_mutex_ptr)(*mutex));
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function,
                            void *arg, int stacksize, int prio)
{
    aos_thread_id thread_id;
    thread_id = aos_create_thread((char *)name, (aos_entry_func)function, arg,
                                  (u32_t)stacksize, 0, NULL, prio,
                                  AOS_AUTO_START);
    return (sys_thread_t)thread_id;
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    if (!mbox) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when new\r\n"));
        return ERR_ARG;
    }

    if (size == 0)
        size = 32;

    *mbox = (sys_mbox_t)aos_create_queue("lwip_mbox", sizeof(void *), size);
    if (!*mbox)
        return ERR_MEM;

    return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    /* parameter check */
    if (!mbox || !*mbox) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when free\r\n"));
        return;
    }

    aos_delete_queue((aos_queue_ptr)(*mbox));
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    if (!mbox || !*mbox /*|| *(u32 *)(*mbox) != AOS_QUEUE_ID*/) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when valid\r\n"));
        return FALSE;
    }

    return TRUE;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    if (!mbox) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when invalid\r\n"));
        return;
    }

    *mbox = 0;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{
    int ret;

    if (!q || !*q || !msg) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when post\r\n"));
        return;
    }

    //LWIP_DEBUGF(SYS_DEBUG, ("mbox send 0x%x to queue\r\n", msg));
    ret = aos_send_to_queue((aos_queue_ptr)(*q), msg, AOS_WAIT_FOREVER);
    if (ret)
        LWIP_DEBUGF(SYS_DEBUG, ("queue write fail 0x%x\r\n", ret));
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
    int ret;

    if (!q || !*q || !msg) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when trypost\r\n"));
        return ERR_MEM;
    }

    //LWIP_DEBUGF(SYS_DEBUG, ("mbox send 0x%x to queue\r\n", msg));
    ret = aos_send_to_queue((aos_queue_ptr)(*q), &msg, 0);
    if (ret) {
        LWIP_DEBUGF(SYS_DEBUG, ("queue try write fail 0x%x\r\n", ret));
        return ERR_MEM;
    }

    return ERR_OK;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
    return sys_mbox_trypost(q, msg);
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
    u32_t ret = 0;

    if (!q || !*q || !msg) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when fetch\r\n"));
        return ERR_ARG;
    }

    if (!timeout)
        timeout = AOS_WAIT_FOREVER;

    *msg = NULL;
    ret = aos_recv_from_queue((aos_queue_ptr)(*q), msg, timeout);
    if (ret) {
        LWIP_DEBUGF(SYS_DEBUG, ("queue fetch fail 0x%x\r\n", ret));
        return SYS_ARCH_TIMEOUT;
    }

    LWIP_DEBUGF(SYS_DEBUG, ("mbox recv 0x%x from queue\r\n", *msg));

    return ERR_OK;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
    u32_t ret = 0;

    if (!q || !*q || !msg) {
        LWIP_DEBUGF(SYS_DEBUG, ("mbox is invalid when fetch\r\n"));
        return ERR_ARG;
    }

    *msg = NULL;
    ret = aos_recv_from_queue((aos_queue_ptr)(*q), msg, 0);
    if (ret) {
        LWIP_DEBUGF(SYS_DEBUG, ("queue fetch fail 0x%x\r\n", ret));
        return SYS_ARCH_TIMEOUT;
    }

    LWIP_DEBUGF(SYS_DEBUG, ("mbox recv 0x%x from queue\r\n", *msg));

    return ERR_OK;
}

#if LWIP_NETCONN_SEM_PER_THREAD
/* Simple implementation of this: unit tests only support one thread */
static sys_sem_t global_netconn_sem;

sys_sem_t* sys_arch_netconn_sem_get(void)
{
    return &global_netconn_sem;
}

void sys_arch_netconn_sem_alloc(void)
{
    sys_sem_new(&global_netconn_sem, 0);
}

void sys_arch_netconn_sem_free(void)
{
    sys_sem_free(&global_netconn_sem);
}
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */
