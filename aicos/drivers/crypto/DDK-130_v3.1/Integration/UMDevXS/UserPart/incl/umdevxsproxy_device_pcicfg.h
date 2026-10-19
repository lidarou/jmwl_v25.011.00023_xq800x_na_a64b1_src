/* umdevxsproxy_device_pcicfg.h
 *
 * UMDevXS Proxy interface for reading and writing the PCI Configuration Space.
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

#ifndef INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_PCICFG_H
#define INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_PCICFG_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_PciCfg_Read32
 *
 * This function can be used to read a 32-bit integer at the specified
 * byte offset from the PCI Configuration Space
 *
 * ByteOffset
 *     Byte offset to read at.
 *
 * Int32_p (output)
 *     Pointer to memory location where the 32-bit integer will be stored.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to execute the request
 */
int
UMDevXSProxy_Device_PciCfg_Read32(
        const unsigned int ByteOffset,
        unsigned int * const Int32_p);


/*----------------------------------------------------------------------------
 * UMDevXSProxy_Device_PciCfg_Write32
 *
 * This function can be used to write a 32-bit integer at the specified
 * byte offset from the PCI Configuration Space
 *
 * ByteOffset
 *     Byte offset to write at.
 *
 * Int32
 *     32-bit integer value to write.
 *
 * Return Value
 *     0  Success
 *    -1  Failed to execute the request
 */
int
UMDevXSProxy_Device_PciCfg_Write32(
        const unsigned int ByteOffset,
        const unsigned int Int32);


#endif /* INCLUDE_GUARD_UMDEVXSPROXY_DEVICE_PCICFG_H */

/* end of file umdevxsproxy_device_pcicfg.h */
