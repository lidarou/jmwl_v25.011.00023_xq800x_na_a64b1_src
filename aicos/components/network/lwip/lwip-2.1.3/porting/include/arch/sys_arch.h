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
#ifndef LWIP_HDR_TEST_SYS_ARCH_H
#define LWIP_HDR_TEST_SYS_ARCH_H

#include "lwip/errno.h"
#include "fcntl.h"
#include <aic_osa_ex.h>

typedef unsigned int sys_sem_t;
typedef unsigned int sys_mutex_t;
typedef unsigned int sys_mbox_t;

#define SYS_MBOX_NULL NULL

/* DWORD (thread id) is used for sys_thread_t but we won't include windows.h */
typedef unsigned int sys_thread_t;

sys_prot_t sys_arch_protect(void);
void sys_arch_unprotect(sys_prot_t val);

#define SYS_ARCH_DECL_PROTECT(lev) sys_prot_t lev
#define SYS_ARCH_PROTECT(lev) lev = sys_arch_protect()
#define SYS_ARCH_UNPROTECT(lev) sys_arch_unprotect(lev)

/* to implement doing something while blocking on an mbox or semaphore:
 * pass a function to test_sys_arch_wait_callback() that returns
 * '0' if waiting again and
 * '1' if now there should be something to do (used for asserting)
 */
typedef int (*test_sys_arch_waiting_fn)(sys_sem_t* wait_sem, sys_mbox_t* wait_mbox);
void test_sys_arch_wait_callback(test_sys_arch_waiting_fn waiting_fn);

/* current time */
//extern u32_t lwip_sys_now;

#define mem_clib_free aic_free
#define mem_clib_malloc aic_zalloc

#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(message) do {app_logi message;} while(0)
#include <stdio.h>
#include <stdlib.h>
#endif

#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x) do {app_loge("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); aic_assert(FALSE); /* fflush(NULL); abort(); */} while(0)
#include <stdio.h>
#include <stdlib.h>
#endif

sys_sem_t* sys_arch_netconn_sem_get(void);
void sys_arch_netconn_sem_alloc(void);
void sys_arch_netconn_sem_free(void);
#define LWIP_NETCONN_THREAD_SEM_GET()   sys_arch_netconn_sem_get()
#define LWIP_NETCONN_THREAD_SEM_ALLOC() sys_arch_netconn_sem_alloc()
#define LWIP_NETCONN_THREAD_SEM_FREE()  sys_arch_netconn_sem_free()

#endif /* LWIP_HDR_TEST_SYS_ARCH_H */

