/* umdevxsproxy_device.h
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

#ifndef INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_H
#define INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_H


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_Find
 *
 * This function must be used to locate a named device resource. The returned
 * DeviceID can be used to map the device into user space memory using
 * UMDevXSProxy_Device_Map.
 *
 * Name_p (input)
 *     Pointer to the name of the device resource.
 *     A list of supported device resources is implementation specific.
 *
 * DeviceID_p (output)
 *     Returns the DeviceID that can be used in other API functions.
 *
 * DeviceMemorySize_p (output)
 *     Returns the size of the device memory window. This is the maximum size
 *     of the region that can be mapped into user space.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to locate device with this name
 */
int
UMDevXSProxy_Device_Find(
        const char * Name_p,
        int * const DeviceID_p,
        unsigned int * const DeviceMemorySize_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_FindRange
 *
 * This function must be used to locate a device resource specified by
 * an address range. The returned DeviceID can be used to map the
 * device into user space memory using UMDevXSProxy_Device_Map.  This
 * function does not actually map the device and it does not guarantee
 * that a returned DeviceID is valid for mapping.
 *
 * bar (input)
 *     Specify the PCI BAR on which the device is mapped.
 *
 * StartOffset (input)
 *     The start offset within the PCI BAR.
 *
 * RangeSize (input)
 *     The size of the requested range in bytes.
 *
 * DeviceID_p (output)
 *     Returns the DeviceID that can be used in other API functions.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to locate device matching this range.
 */
int
UMDevXSProxy_Device_FindRange(
    const unsigned int bar,
    const unsigned int StartOffset,
    const unsigned int RangeSize,
    int * const DeviceID_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_Enum
 *
 * This function can be used to get a list of device names supported by the
 * implementation.
 *
 * DeviceNr
 *     Device number to enumerate. Start at 0 and stop upon error.
 *
 * NameSize
 *     Size of the memory region pointer to by Name_p.
 *
 * Name_p (output)
 *     Pointer to the memory range where the device name will be copied to.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to execute the request, or invalid DeviceNr
 */
int
UMDevXSProxy_Device_Enum(
        const unsigned int DeviceNr,
        const unsigned int NameSize,
        char * Name_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_Map
 *
 * DeviceID
 *    Device identifier as returned by UMDevXSProxy_Device_Find.
 *
 * DeviceMemorySize
 *     Size of the memory block to map in (device-dependent).
 *
 * Return Value
 *     NULL   Failed to map this memory region (might be non-existing).
 *     Other  Pointer application can use to access this region.
 */
void *
UMDevXSProxy_Device_Map(
        const int DeviceID,
        const unsigned int DeviceMemorySize);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_Unmap
 *
 * DeviceID
 *    Device identifier as returned by UMDevXSProxy_Device_Find.
 *
 * DeviceMemory_p
 *    Pointer as returned by UMDevXSProxy_Device_Map.
 *
 * DeviceMemorySize
 *    Size of the device memory block as used in the UMDevXSProxy_Device_Map
 *    call.
 *
 * Return Value
 *     0   Success
 *    -1   Error code
 */
int
UMDevXSProxy_Device_Unmap(
        const int DeviceID,
        void * DeviceMemory_p,
        const unsigned int DeviceMemorySize);


#endif /* INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_H */

/* end of file umdevxsproxy_device.h */
