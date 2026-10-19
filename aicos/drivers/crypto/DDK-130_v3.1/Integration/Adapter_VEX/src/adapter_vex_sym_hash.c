/* adapter_vex_sym_hash.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the symmetric crypto hash services.
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
#include "eip130_token_hash.h"      // Eip130Token_Command_Hash*()


/*----------------------------------------------------------------------------
 * vex_SymHash
 */
VexStatus_t
vex_SymHash(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    uint64_t DataAddr = 0;
    bool fInitWithDefault;
    bool fFinalize;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    switch (CommandToken_p->Service.Hash.Mode)
    {
    default:
    case VEXTOKEN_MODE_HASH_MAC_INIT2FINAL:
        fInitWithDefault = true;
        fFinalize = true;
        break;

    case VEXTOKEN_MODE_HASH_MAC_CONT2FINAL:
        fInitWithDefault = false;
        fFinalize = true;
        break;

    case VEXTOKEN_MODE_HASH_MAC_INIT2CONT:
        fInitWithDefault = true;
        fFinalize = false;
        break;

    case VEXTOKEN_MODE_HASH_MAC_CONT2CONT:
        fInitWithDefault = false;
        fFinalize = false;
        break;
    }

    if (CommandToken_p->Service.Hash.DataSize != 0)
    {
        // Convert input buffer and get address for token
        DataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_IN,
                                  CommandToken_p->Service.Hash.Data_p,
                                  CommandToken_p->Service.Hash.DataSize,
                                  NULL);
        if (DataAddr == 0)
        {
            return VEX_NO_MEMORY;
        }
    }
    else if (!fInitWithDefault || !fFinalize)
    {
        return VEX_INVALID_LENGTH;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Hash(&CommandToken,
                             (uint8_t)CommandToken_p->Service.Hash.Algorithm,
                             fInitWithDefault,
                             fFinalize,
                             DataAddr,
                             CommandToken_p->Service.Hash.DataSize);
    if (CommandToken_p->Service.Hash.TempAssetId)
    {
        Eip130Token_Command_Hash_SetStateASID(
            &CommandToken,
            CommandToken_p->Service.Hash.TempAssetId);
    }
    else
    {
        Eip130Token_Command_Hash_CopyState(
            &CommandToken,
            CommandToken_p->Service.Hash.Digest,
            sizeof(CommandToken_p->Service.Hash.Digest));
    }
    Eip130Token_Command_Hash_SetTotalMessageLength(
        &CommandToken,
        CommandToken_p->Service.Hash.TotalMessageLength);
    Eip130Token_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID(), false);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if ((ResultToken_p->Result >= 0) &&
            (fFinalize || (CommandToken_p->Service.Hash.TempAssetId == 0)))
        {
            // Copy digest from result
            Eip130Token_Result_Hash_CopyState(
                &ResultToken,
                sizeof(ResultToken_p->Service.Hash.Digest),
                ResultToken_p->Service.Hash.Digest);
        }
    }

    // Release used buffer, if needed
    (void)BufManager_Unmap(DataAddr, false, false);

    return funcres;
}


/* end of file adapter_vex_sym_hash.c */
