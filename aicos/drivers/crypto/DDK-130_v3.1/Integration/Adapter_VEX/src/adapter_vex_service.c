/* adapter_vex_service.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the service services.
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

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_internal.h"   // API implementation
#include "eip130_token_service.h"   // Eip130Token_Command_RegisterRead/Write()
                                    // Eip130Token_Command_ZeroizeOutputMailbox()
#include "eip130_token_otp.h"       // Eip130Token_Command_OTP(Select)Zeroize()
#include "adapter_bufmanager.h"     // bufmanager for firmwarecheck


/*----------------------------------------------------------------------------
 * vex_Service
 */
VexStatus_t
vex_Service(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;
#ifdef VEX_ENABLE_FIRMWARE_LOAD
    uint64_t SrcDataAddr = 0;
#endif

    // Format command token
    ZEROINIT(CommandToken);
    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_REGISTERREAD:
        Eip130Token_Command_RegisterRead(
            &CommandToken,
            (uint16_t)CommandToken_p->Service.Register.Address);
        break;

    case VEXTOKEN_SUBCODE_REGISTERWRITE:
        Eip130Token_Command_RegisterWrite(
            &CommandToken,
            (uint16_t)CommandToken_p->Service.Register.Address,
            CommandToken_p->Service.Register.Value);
        break;

    case VEXTOKEN_SUBCODE_CLOCKSWITCH:
        Eip130Token_Command_ClockSwitch(
            &CommandToken,
            CommandToken_p->Service.ClockSwitch.On,
            CommandToken_p->Service.ClockSwitch.Off);
        break;

    case VEXTOKEN_SUBCODE_ZEROOUTMAILBOX:
        Eip130Token_Command_ZeroizeOutputMailbox(&CommandToken);
        break;

    case VEXTOKEN_SUBCODE_SELECTOTPZERO:
        Eip130Token_Command_OTPSelectZeroize(&CommandToken);
        break;

    case VEXTOKEN_SUBCODE_ZEROIZEOTP:
        Eip130Token_Command_OTPZeroize(&CommandToken);
        break;

#ifdef VEX_ENABLE_FIRMWARE_LOAD
    case VEXTOKEN_SUBCODE_FIRMWARECHECK:
        // Convert input buffer and get address for token
        SrcDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                    BUFMANAGER_BUFFERTYPE_IN,
                                    CommandToken_p->Service.FirmwareCheck.FirmwareImage_p,
                                    CommandToken_p->Service.FirmwareCheck.FirmwareImageSize,
                                    NULL);
        if (SrcDataAddr == 0)
        {
            return VEX_INTERNAL_ERROR;
        }
        Eip130Token_Command_FirmwareCheck(
            &CommandToken,
            SrcDataAddr,
            CommandToken_p->Service.FirmwareCheck.FirmwareImageSize);
        break;

    case VEXTOKEN_SUBCODE_UPDATEROLLBACKID:
        Eip130Token_Command_UpdateRollbackID(
            &CommandToken,
            CommandToken_p->Service.UpdateRollbackID.RollbackID);
        break;
#endif

    default:
        return VEX_UNSUPPORTED;
    }
    Eip130Token_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID(), false);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if ((ResultToken_p->Result >= 0) &&
            (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_REGISTERREAD))
        {
            // Success
            ResultToken_p->Service.Register.Value = ResultToken.W[1];
        }
    }

#ifdef VEX_ENABLE_FIRMWARE_LOAD
    if (SrcDataAddr != 0)
    {
        (void)BufManager_Unmap(SrcDataAddr, false, false);
    }
#endif

    return funcres;
}

/* end of file adapter_vex_service.c */
