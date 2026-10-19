/* c_umdevxsproxy.h
 *
 * Configuration options for UMDevXS Proxy Library.
 *
 * This file includes cs_umdevxsproxy.h (from the product-level) and then
 * provides defaults for missing configuration switches.
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

#ifndef INCLUDE_GUARD_C_UMDEVXSPROXY_H
#define INCLUDE_GUARD_C_UMDEVXSPROXY_H

// get the product-level configuration
#include "cs_umdevxsproxy.h"

#ifndef UMDEVXSPROXY_LOG_PREFIX
#define UMDEVXSPROXY_LOG_PREFIX "UMDevXSProxy: "
#endif

#ifndef UMDEVXSPROXY_NODE_NAME
#define UMDEVXSPROXY_NODE_NAME "/dev/umdevxs_c"
#endif

// #define UMDEVXSPROXY_REMOVE_SMBUF
// #define UMDEVXSPROXY_REMOVE_SMBUF_ATTACH
// #define UMDEVXSPROXY_REMOVE_SMBUF_COMMIT


// Disable the UMDevXSProxy_Device_Find function.
//#define UMDEVXSPROXY_REMOVE_DEVICE_FIND

// Disable the UMDevXSProxy_Device_Enum function.
//#define UMDEVXSPROXY_REMOVE_DEVICE_ENUM

// Disable the UMDevXSProxy_Device_Unmap function.
//#define UMDEVXSPROXY_REMOVE_DEVICE_UNMAP


#endif /* INCLUDE_GUARD_C_UMDEVXSPROXY_H */

/* end of file c_umdevxsproxy.h */
