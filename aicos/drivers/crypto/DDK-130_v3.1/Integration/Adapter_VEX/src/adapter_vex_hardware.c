/* adapter_vex_hardware.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the hardware registers related services.
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

#include "c_adapter_vex.h"          // configuration

#ifdef VEX_ENABLE_HW_FUNCTIONS

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "adapter_vex_internal.h"   // API implementation
#define EIP130_REMOVE_READMAILBOXSTATUS     // Remove unused function warnings
#define EIP130_REMOVE_WRITEMAILBOXCONTROL
#define EIP130_REMOVE_MAILBOXRAWSTATUS
#define EIP130_REMOVE_MAILBOXRESET
#define EIP130_REMOVE_MAILBOXLINKRESET
#define EIP130_REMOVE_MAILBOXLINKID
#define EIP130_REMOVE_MAILBOXOUTID
#define EIP130_REMOVE_MAILBOXACCESSCONTROL
#define EIP130_REMOVE_MAILBOXACCESSVERIFY
#define EIP130_REMOVE_MODULEFIRMWAREWRITTEN
#define EIP130_REMOVE_FIRMWAREDOWNLOAD
#include "eip130_level0.h"


/*----------------------------------------------------------------------------
 * vex_Hardware
 */
VexStatus_t
vex_Hardware(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    Device_Handle_t Device;
    uint32_t Value;

    Device = vex_DeviceGetHandle();
    if (Device == NULL)
    {
        return VEX_NOT_CONNECTED;
    }

    // Format result token
    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_MODULESTATUS:
        Value = EIP130_RegisterReadModuleStatus(Device);
        ResultToken_p->Service.ModuleStatus.FIPSmode = (uint8_t)(MASK_1_BIT & (Value >> 0));
        ResultToken_p->Service.ModuleStatus.NonFIPSmode = (uint8_t)(MASK_1_BIT & (Value >> 1));
        ResultToken_p->Service.ModuleStatus.CRC24Busy = (uint8_t)(MASK_1_BIT & (Value >> 8));
        ResultToken_p->Service.ModuleStatus.CRC24Ok = (uint8_t)(MASK_1_BIT & (Value >> 9));
        ResultToken_p->Service.ModuleStatus.CRC24Error = (uint8_t)(MASK_1_BIT & (Value >> 10));
        ResultToken_p->Service.ModuleStatus.FwImageWritten = (uint8_t)(MASK_1_BIT & (Value >> 20));
        ResultToken_p->Service.ModuleStatus.FwImageCheckDone = (uint8_t)(MASK_1_BIT & (Value >> 22));
        ResultToken_p->Service.ModuleStatus.FwImageAccepted = (uint8_t)(MASK_1_BIT & (Value >> 23));
        ResultToken_p->Service.ModuleStatus.FatalError = (uint8_t)(MASK_1_BIT & (Value >> 31));
        break;

    case VEXTOKEN_SUBCODE_EIP_OPTIONS:
        Value = EIP130_RegisterReadOptions(Device);
        ResultToken_p->Service.EipOptions.nMailboxes = (uint8_t)(MASK_4_BITS & (Value >> 0));
        ResultToken_p->Service.EipOptions.MailboxSize = (uint16_t)(0x80 << (MASK_2_BITS & (Value >> 4)));
        ResultToken_p->Service.EipOptions.HostId = (uint8_t)(MASK_8_BITS & (Value >> 8));
        ResultToken_p->Service.EipOptions.MasterId = (uint8_t)(MASK_3_BITS & (Value >> 16));
        ResultToken_p->Service.EipOptions.ProtectionAvailable = (uint8_t)(MASK_1_BIT & (Value >> 19));
        ResultToken_p->Service.EipOptions.MyHostId = (uint8_t)(MASK_3_BITS & (Value >> 20));
        ResultToken_p->Service.EipOptions.Protection = (uint8_t)(MASK_1_BIT & (Value >> 23));
        ResultToken_p->Service.EipOptions.SecureHostId = (uint8_t)(MASK_8_BITS & (Value >> 24));

        Value = EIP130_RegisterReadOptions2(Device);
        ResultToken_p->Service.EipOptions.StandardEngines = (uint16_t)(MASK_16_BITS & (Value >> 0));
        ResultToken_p->Service.EipOptions.CustomEngines = (uint16_t)(MASK_16_BITS & (Value >> 16));
        break;

    case VEXTOKEN_SUBCODE_EIP_VERSION:
        Value = EIP130_RegisterReadVersion(Device);
        ResultToken_p->Service.EipVersion.EipNumber = (uint8_t)(MASK_8_BITS & (Value >> 0));
        ResultToken_p->Service.EipVersion.PatchLevel = (uint8_t)(MASK_4_BITS & (Value >> 16));
        ResultToken_p->Service.EipVersion.MinorVersion = (uint8_t)(MASK_4_BITS & (Value >> 20));
        ResultToken_p->Service.EipVersion.MajorVersion = (uint8_t)(MASK_4_BITS & (Value >> 24));
        break;

    default:
        return VEX_UNSUPPORTED;
    }

    ResultToken_p->Result = 0;
    return VEX_SUCCESS;
}

#endif /* VEX_ENABLE_HW_FUNCTIONS */

/* end of file adapter_vex_hardware.c */
