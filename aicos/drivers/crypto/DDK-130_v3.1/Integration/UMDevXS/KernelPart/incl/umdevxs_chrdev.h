/* umdevxs_chrdev.h
 *
 * Linux UMDevXS Driver Character Device Interfaces.
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

#ifndef INCLUDE_GUARD_UMDEVXS_CHRDEV_H
#define INCLUDE_GUARD_UMDEVXS_CHRDEV_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "umdevxs_cmd.h"        // UMDevXS_CmdRsp_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef int (* UMDevXS_HandleCmdFunction_t)(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p);


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_Init
 *
 * This routine hooks the character device.
 *
 * Return Value:
 *     0    Success
 *     <0   Error code return by a kernel call
 */
int
UMDevXS_ChrDev_Init(void);


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_UnInit
 *
 * This routine unhooks the character device.
 */
void
UMDevXS_ChrDev_UnInit(void);

/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_HandleCmdFunc_Set
 *
 * This routine installs the external handle function for commands.
 */
void
UMDevXS_ChrDev_HandleCmdFunc_Set
        (UMDevXS_HandleCmdFunction_t Func);


#endif /* INCLUDE_GUARD_UMDEVXS_CHRDEV_H */

/* end of file umdevxs_chrdev.h */
