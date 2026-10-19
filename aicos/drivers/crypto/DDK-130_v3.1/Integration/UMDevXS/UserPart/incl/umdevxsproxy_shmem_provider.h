/* umdevxsproxy_shmem_provider.h
 *
 * This user-mode library handles the communication with the
 * Linux User Mode Device Access driver. Using the API it is possible to
 * access memory mapped devices and to enable access to foreign-allocated
 * shared memory buffers.
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

#ifndef INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_PROVIDER_H
#define INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_PROVIDER_H

#include "umdevxsproxy_shmem_types.h"


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Alloc
 *
 * This routine allocates a block of memory that can be shared with another
 * host. The memory is allocated in kernel space and mapped into the user's
 * address space. A handle and a pointer to the start of the memory block are
 * returned. The GBI returned can be sent to the other host.
 * The size of the block allocated is rounded up to the required alignment.
 * The aligned size is returned.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Alloc(
        const unsigned int Size,
        UMDevXSProxy_SHMem_Handle_t * const Handle_p,
        UMDevXSProxy_SHMem_BufPtr_t * const BufPtr_p,
        UMDevXSProxy_SHMem_GBI_t * const GBI_p,
        unsigned int * const ActualSize_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Free
 *
 * This function frees a memory block previously allocated using
 * UMDevXSProxy_SHMem_Alloc. The memory block is removed from the caller's
 * memory map and then freed. The caller must make sure the buffer is not
 * accessed anymore (also by the remote host!) before calling this function.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Free(
        const UMDevXSProxy_SHMem_Handle_t Handle);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Commit
 *
 * This function ensures the [subset of the] buffer is committed from a cache
 * (if any) to system memory, to ensure the other host can read it.
 */
void
UMDevXSProxy_SHMem_Commit(
        const UMDevXSProxy_SHMem_Handle_t Handle,
        const unsigned int SubsetStart,
        const unsigned int SubsetLength);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Refresh
 *
 * This function ensures the [subset of the] buffer is refreshed from system
 * memory into the cache (if any), to ensure we are not looking at old data
 * that has been replaced by the other host.
 */
void
UMDevXSProxy_SHMem_Refresh(
        const UMDevXSProxy_SHMem_Handle_t Handle,
        const unsigned int SubsetStart,
        const unsigned int SubsetLength);


#endif /* INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_PROVIDER_H */

/* end of file umdevxsproxy_shmem_provider.h */
