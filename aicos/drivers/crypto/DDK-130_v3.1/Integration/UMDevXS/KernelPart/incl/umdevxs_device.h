/* umdevxs_device.h
 *
 * Exported Device API of the UMDexXS/UMPCI driver.
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

#ifndef INCLUDE_GUARD_UMDEVXS_DEVICE_H
#define INCLUDE_GUARD_UMDEVXS_DEVICE_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define UMDEVXS_KERNEL_APPID ((void*)-1)

/*----------------------------------------------------------------------------
 * UMDevXS_Device_LockRange
 *
 * Lock the specified device resource, so no other application will be
 * allowed to lock it through this function. Locking is bypassed if
 * the current application is the privileged application.
 *
 * DeviceNr (input)
 *   Selection of a specific (sub)-device.
 * offset (input)
 *   Byte offset of the range to lock. Must be a multiple of PAGE_SIZE.
 * size (input)
 *   Size in bytes of the range to lock. Must be a multiple of PAGE_SIZE.
 * AppID (input)
 *   ID of the application that requests the lock. Use UMDEVXS_KERNEL_APPID
 *   when calling this function from the kernel.
 *
 * Return 0 on success, -1 on failure.
 */
int
UMDevXS_Device_LockRange(
        unsigned int DeviceNr,
        unsigned int offset,
        unsigned int size,
        void * AppID);


/*----------------------------------------------------------------------------
 * UMDevXS_Device_SetPrivileged
 *
 * Mark the indicated application as the privileged application. Only one
 * application is allowed to claim this status.
 *
 * AppID (input)
 *   ID of the application that requests the lock. Use UMDEVXS_KERNEL_APPID
 *   when calling this function from the kernel.
 *
 * Return 0 on success, -1 on failure
 */
int
UMDevXS_Device_SetPrivileged(
        void *AppID);

/*----------------------------------------------------------------------------
 * UMDevXS_Device_Unlock
 *
 * Release all device ranges locked by the indicated application.  If
 * the indicated application was the privileged application, release
 * this status as well.
 *
 * AppID (input)
 *    ID of the application that releases the resources.
 *    Use UMDEVXS_KERNEL_APPID when calling this function from the kernel.
*/
void
UMDevXS_Device_Unlock(
        void *AppID);


#endif /* INCLUDE_GUARD_UMDEVXS_DEVICE_H */

/* umdevxs_device.h */
