/* umdevxs_smbuf.h
 *
 * Exported SMBuf API of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_SMBUF_H
#define INCLUDE_GUARD_UMDEVXS_SMBUF_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Static DMA bank with fixed address type for UMDevXS
#define UMDEVXS_SHMEM_BANK_STATIC_FIXED_ADDR          99


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_SetAppID
 */
void
UMDevXS_SMBuf_SetAppID(
        int Handle,
        void * AppID);

/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Register
 *
 * This function must be used to register an "alien" buffer that was allocated
 * somewhere else. The caller guarantees that this buffer can be used for DMA.
 *
 * Size (input)
 *     Size of the buffer to be registered (in bytes).
 *
 * Buffer_p (input)
 *     Pointer to the buffer. This pointer must be valid to use on the host
 *     in the domain of the driver.
 *
 * Alternative_p (input)
 *     Some allocators return two addresses. This parameter can be used to
 *     pass this second address to the driver. The type is pointer to ensure
 *     it is always large enough to hold a system address, also in LP64
 *     architecture. Set to NULL if not used.
 *
 * Handle_p (output)
 *     Pointer to the memory location when the handle will be returned.
 *
 * Return Values
 *     DMABUF_STATUS_OK: Success, Handle_p was written.
 *     DMABUF_ERROR_BAD_ARGUMENT
 */
int
UMDevXS_SMBuf_Register(
        const int Size,
        void * Buffer_p,
        void * Alternative_p,
        int * const Handle_p);


/*----------------------------------------------------------------------------
 * UMDevXS_SMBuf_Release
 *
 * Free the DMA resource (unless not allocated locally) and the record used
 * to describe it.
 *
 */
int
UMDevXS_SMBuf_Release(
    int Handle);


#endif /* INCLUDE_GUARD_UMDEVXS_SMBUF_H */

/* end of file umdevxs_smbuf.h */
