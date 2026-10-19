/* adapter_vex_asym_asset_signverify.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the asymmetric crypto services for Sign/Verify.
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

#include "c_adapter_vex.h"              // configuration

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_intern_asym.h"   // API implementation
#include "adapter_bufmanager.h"        // BufManager_*()
#include "eip130_token_pk.h"           // Eip130Token_Command_Pk_Asset*()
#include "eip130_token_asset.h"        // Eip130Token_Result_AssetCreate()
                                       // for StateAssetId


/*----------------------------------------------------------------------------
 * vex_Asym_AssetSign
 */
VexStatus_t
vex_Asym_AssetSign(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    bool fCopy = false;
    uint64_t HashDataAddr = 0;
    uint64_t SignAddr = 0;
    uint16_t TokenID;
    uint8_t ModulusWords;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if (((CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize == 0) &&
         (CommandToken_p->Service.PkAssetSignVerify.HashData_p == NULL)) ||
        ((CommandToken_p->Service.PkAssetSignVerify.Sign_p == NULL) &&
         (CommandToken_p->Service.PkAssetSignVerify.Method != VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL) &&
         (CommandToken_p->Service.PkAssetSignVerify.Method != VEXTOKEN_PKASSET_EDDSA_SIGN_UPDATE)))
    {
        return VEX_BAD_ARGUMENT;
    }

    if (CommandToken_p->Service.PkAssetSignVerify.HashData_p != NULL)
    {
        // Get hash data address for token
        HashDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                      BUFMANAGER_BUFFERTYPE_IN,
                                      CommandToken_p->Service.PkAssetSignVerify.HashData_p,
                                      CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
                                      NULL);
        if (HashDataAddr == 0)
        {
            goto error_func_exit;
        }
    }

    // Get signature output address for token
    TokenID = vex_DeviceGetTokenID();
    if (CommandToken_p->Service.PkAssetSignVerify.Sign_p != NULL)
    {
        SignAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_OUT,
                                  CommandToken_p->Service.PkAssetSignVerify.Sign_p,
                                  CommandToken_p->Service.PkAssetSignVerify.SignSize,
                                  (void *)&TokenID);
        if (SignAddr == 0)
        {
            goto error_func_exit;
        }
    }

    ModulusWords = (uint8_t)((CommandToken_p->Service.PkAssetSignVerify.ModulusSizeInBits+31)/32);

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetSignVerify.Method,
        ModulusWords, ModulusWords,
        (uint8_t)CommandToken_p->Service.PkAssetSignVerify.SaltSize,
        CommandToken_p->Service.PkAssetSignVerify.KeyAssetId,
        CommandToken_p->Service.PkAssetSignVerify.DomainAssetId,
        CommandToken_p->Service.PkAssetSignVerify.DigestAssetId,
        HashDataAddr, (uint16_t)CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
        SignAddr, (uint16_t)CommandToken_p->Service.PkAssetSignVerify.SignSize);
#ifdef EIP130_ENABLE_EXPLICITDIGEST
    if(CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize != 0)
    {
        Eip130Token_Command_Pk_Asset_SetExplicitDigest(
            &CommandToken,
            CommandToken_p->Service.PkAssetSignVerify.ExplicitDigest,
            CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize);
    }
    else
#endif
    {
        Eip130Token_Command_Pk_Asset_SetAdditionalLength(
            &CommandToken,
            CommandToken_p->Service.PkAssetSignVerify.TotalMessageSize);
    }
    Vex_Command_SetTokenID(&CommandToken, TokenID);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if (ResultToken_p->Result >= 0)
        {
            // Copy output data
            fCopy = true;

            if (CommandToken_p->Service.PkAssetSignVerify.Method == VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL)
            {
                // Get StateAssetId
                Eip130Token_Result_AssetCreate(
                    &ResultToken,
                    &ResultToken_p->Service.PkAssetSignVerify.StateAssetId);
            }
        }
    }

error_func_exit:
    // Release used buffers, if needed
    if (HashDataAddr != 0)
    {
        (void)BufManager_Unmap(HashDataAddr, false, false);
    }
    if (SignAddr != 0)
    {
        int rc = BufManager_Unmap(SignAddr, true, fCopy);
        if (rc != 0)
        {
            if (rc == -3)
            {
                funcres = VEX_DATA_TIMEOUT;
            }
            else
            {
                funcres = VEX_INTERNAL_ERROR;
            }
        }
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * vex_Asym_AssetVerify
 */
VexStatus_t
vex_Asym_AssetVerify(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint64_t HashDataAddr = 0;
    uint64_t SignAddr = 0;
    uint8_t ModulusWords;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if (((CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize == 0) &&
         (CommandToken_p->Service.PkAssetSignVerify.HashData_p == NULL)) ||
        ((CommandToken_p->Service.PkAssetSignVerify.Sign_p == NULL) &&
         (CommandToken_p->Service.PkAssetSignVerify.Method != VEXTOKEN_PKASSET_EDDSA_VERIFY_FINAL)))
    {
        return VEX_BAD_ARGUMENT;
    }

    if (CommandToken_p->Service.PkAssetSignVerify.HashData_p != NULL)
    {
        // Get hash data address for token
        HashDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                      BUFMANAGER_BUFFERTYPE_IN,
                                      CommandToken_p->Service.PkAssetSignVerify.HashData_p,
                                      CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
                                      NULL);
        if (HashDataAddr == 0)
        {
            goto error_func_exit;
        }
    }

    // Get signature output address for token
    if (CommandToken_p->Service.PkAssetSignVerify.Sign_p != NULL)
    {
        SignAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_IN,
                                  CommandToken_p->Service.PkAssetSignVerify.Sign_p,
                                  CommandToken_p->Service.PkAssetSignVerify.SignSize,
                                  NULL);
        if (SignAddr == 0)
        {
            goto error_func_exit;
        }
    }

    ModulusWords = (uint8_t)((CommandToken_p->Service.PkAssetSignVerify.ModulusSizeInBits+31)/32);

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetSignVerify.Method,
        ModulusWords, ModulusWords,
        (uint8_t)CommandToken_p->Service.PkAssetSignVerify.SaltSize,
        CommandToken_p->Service.PkAssetSignVerify.KeyAssetId,
        CommandToken_p->Service.PkAssetSignVerify.DomainAssetId,
        CommandToken_p->Service.PkAssetSignVerify.DigestAssetId,
        HashDataAddr, (uint16_t)CommandToken_p->Service.PkAssetSignVerify.HashDataSize,
        SignAddr, (uint16_t)CommandToken_p->Service.PkAssetSignVerify.SignSize);
#ifdef EIP130_ENABLE_EXPLICITDIGEST
    if(CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize != 0)
    {
        Eip130Token_Command_Pk_Asset_SetExplicitDigest(
            &CommandToken,
            CommandToken_p->Service.PkAssetSignVerify.ExplicitDigest,
            CommandToken_p->Service.PkAssetSignVerify.ExplicitDigestSize);
    }
    else
#endif
    {
        Eip130Token_Command_Pk_Asset_SetAdditionalLength(
            &CommandToken,
            CommandToken_p->Service.PkAssetSignVerify.TotalMessageSize);
    }
    Eip130Token_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID(), false);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if ((ResultToken_p->Result >= 0) &&
            (CommandToken_p->Service.PkAssetSignVerify.Method == VEXTOKEN_PKASSET_EDDSA_VERIFY_INITIAL))
        {
            // Get StateAssetId
            Eip130Token_Result_AssetCreate(
                &ResultToken,
                &ResultToken_p->Service.PkAssetSignVerify.StateAssetId);
        }
    }

error_func_exit:
    // Release used buffers, if needed
    if (HashDataAddr != 0)
    {
        (void)BufManager_Unmap(HashDataAddr, false, false);
    }
    if (SignAddr != 0)
    {
        (void)BufManager_Unmap(SignAddr, false, false);
    }

    return funcres;
}


/* end of file adapter_vex_asym_asset_signverify.c */
