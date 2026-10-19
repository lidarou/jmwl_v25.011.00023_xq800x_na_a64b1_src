/* umdevxsproxy_shmem.h
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

#ifndef INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_H
#define INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_H

/*----------------------------------------------------------------------------
 * Types used by SHMem API
 */

// Static DMA bank with fixed address type for UMDevXS
#define UMDEVXSPROXY_SHMEM_BANK_STATIC_FIXED_ADDR          99

typedef struct
{
    void * p;
} UMDevXSProxy_SHMem_Handle_t;


typedef struct
{
    void * p;
} UMDevXSProxy_SHMem_BufPtr_t;

typedef struct
{
    void * p;
} UMDevXSProxy_SHMem_DevAddr_t;


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Alloc
 *
 * This routine allocates a block of memory that can be shared with another
 * host. The memory is allocated in kernel space. A handle is returned that
 * can be passed to the SMBUF_MAP service to map the memory in the user's
 * address space.
 * Also, a device (aka physical or bus) address is returned that can be used
 * by DMA devices.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Alloc(
        const unsigned int Size,
        const unsigned int Bank,
        const unsigned int Alignment,
        UMDevXSProxy_SHMem_Handle_t * const Handle_p,
        UMDevXSProxy_SHMem_BufPtr_t * const BufPtr_p,
        UMDevXSProxy_SHMem_DevAddr_t * const DevAddr_p,
        unsigned int * const ActualSize_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Register
 *
 * This routine registers a block of memory in order to obtain a handle
 * to be used in subsequent Pre/PostDMA calls.
 * The passed-in handle must refer to an already allocated/attached block
 * that encompasses the registered block.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Register(
        const unsigned int Size,
        const UMDevXSProxy_SHMem_BufPtr_t BufPtr,
        const UMDevXSProxy_SHMem_Handle_t Handle,
        UMDevXSProxy_SHMem_Handle_t * const RetHandle_p);


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
 * UMDevXSProxy_SHMem_Attach
 *
 * This function registers a block of memory, typically allocated by another
 * host to exchange data through shared memory.
 * A handle is returned that can be passed to the SHMEM_MAP service to map
 * the memory into the user's address space.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Attach(
        const UMDevXSProxy_SHMem_DevAddr_t DevAddr,
        const unsigned int Size,
        const unsigned int Bank,
        UMDevXSProxy_SHMem_Handle_t * const Handle_p,
        UMDevXSProxy_SHMem_BufPtr_t * const BufPtr_p,
        unsigned int * const Size_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_SHMem_Detach
 *
 * This function detaches from a block of shared memory.
 * The memory block is removed from the caller's memory map.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to ...
 */
int
UMDevXSProxy_SHMem_Detach(
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


#endif /* INCLUDE_GUARD_UMDEVXSPROXY_SHMEM_H */

/* umdevxsproxy_shmem.h */
