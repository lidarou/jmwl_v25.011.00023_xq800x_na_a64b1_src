/* adapter_vex_aunlock.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the Authenticated Unlock and Secure Debug services.
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
#include "adapter_bufmanager.h"     // BufManager_*()
#include "eip130_token_aunlock.h"   // Eip130Token_Command_RegisterRead/Write()
                                    // Eip130Token_Command_ZeroizeOutputMailbox()
#include "eip130_token_otp.h"       // Eip130Token_Command_OTP(Select)Zeroize()


/*----------------------------------------------------------------------------
 * vex_AUnlock
 */
VexStatus_t
vex_AUnlock(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;
    uint64_t SignAddr = 0;

    // Format command token
    ZEROINIT(CommandToken);
    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_AUNLOCKSTART:
        Eip130Token_Command_AUnlock_Start(
            &CommandToken,
            CommandToken_p->Service.AuthUnlock.AuthStateAssetId,
            CommandToken_p->Service.AuthUnlock.AuthKeyAssetId);
        break;

    case VEXTOKEN_SUBCODE_AUNLOCKVERIFY:
        if (CommandToken_p->Service.AuthUnlock.Sign_p == NULL)
        {
            return VEX_BAD_ARGUMENT;
        }

        // Get signature address for token
        SignAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_IN,
                                  CommandToken_p->Service.AuthUnlock.Sign_p,
                                  CommandToken_p->Service.AuthUnlock.SignSize,
                                  NULL);
        if (SignAddr == 0)
        {
            return VEX_NO_MEMORY;
        }

        Eip130Token_Command_AUnlock_Verify(
            &CommandToken,
            CommandToken_p->Service.AuthUnlock.AuthStateAssetId,
            CommandToken_p->Service.AuthUnlock.Nonce,
            SignAddr, CommandToken_p->Service.AuthUnlock.SignSize);
        break;

    case VEXTOKEN_SUBCODE_SETSECUREDEBUG:
        Eip130Token_Command_SetSecureDebug(
            &CommandToken,
            CommandToken_p->Service.SecureDebug.AuthStateAssetId,
            CommandToken_p->Service.SecureDebug.fSet);
        break;

    default:
        return VEX_UNSUPPORTED;
    }
    Eip130Token_Command_SetTokenID(&CommandToken,
                                   vex_DeviceGetTokenID(),
                                   false);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if ((ResultToken_p->Result >= 0) &&
            (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_AUNLOCKSTART))
        {
            // Success - Copy AssetId and Nonce from result token
            Eip130Token_Result_AUnlock_CopyNonce(&ResultToken,
                                                 ResultToken_p->Service.AuthUnlock.Nonce);
        }
    }

    if (SignAddr != 0)
    {
        (void)BufManager_Unmap(SignAddr, false, false);
    }

    return funcres;
}

/* end of file adapter_vex_aunlock.c */
