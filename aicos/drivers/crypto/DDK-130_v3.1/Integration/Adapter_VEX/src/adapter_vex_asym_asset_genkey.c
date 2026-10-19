/* adapter_vex_asym_asset_genkey.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the asymmetric crypto services for key generation.
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


/*----------------------------------------------------------------------------
 * vex_Asym_AssetGenKeyPair
 */
VexStatus_t
vex_Asym_AssetGenKeyPair(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    bool fCopy = false;
    uint64_t KeyBlobAddr = 0;
    size_t KeyBlobSize = 0;
    uint64_t PubKeyAddr = 0;
    size_t PubKeySize = 0;
    uint16_t TokenID;
    uint16_t BufManagerTokenID;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    TokenID = vex_DeviceGetTokenID();
#ifdef VEX_DISABLE_TOKENID_WR_KEYPAIR
    BufManagerTokenID = 0;
#else
    BufManagerTokenID = TokenID;
#endif

    if (CommandToken_p->Service.PkAssetGenKey.KekAssetId != 0)
    {
        if ((CommandToken_p->Service.PkAssetGenKey.KeyBlob_p == NULL) ||
            (CommandToken_p->Service.PkAssetGenKey.AssociatedDataSize >= (256 - 4)))
        {
            return VEX_BAD_ARGUMENT;
        }

        // Get Key output address for token
        KeyBlobAddr = BufManager_Map(
                          CommandToken_p->fFromUserSpace,
                          BUFMANAGER_BUFFERTYPE_OUT,
                          CommandToken_p->Service.PkAssetGenKey.KeyBlob_p,
                          CommandToken_p->Service.PkAssetGenKey.KeyBlobSize,
                          (void *)&BufManagerTokenID);
        if (KeyBlobAddr == 0)
        {
            goto error_func_exit;
        }
#ifdef VEX_DISABLE_TOKENID_WR_KEYPAIR
        KeyBlobSize = CommandToken_p->Service.PkAssetGenKey.KeyBlobSize;

        BufManager_Zeroize(KeyBlobAddr);
#else
        KeyBlobSize = BufManager_GetSize(KeyBlobAddr);
#endif
    }

    if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
    {
        // Get public key output address for token
        PubKeyAddr = BufManager_Map(
                            CommandToken_p->fFromUserSpace,
                            BUFMANAGER_BUFFERTYPE_OUT,
                            CommandToken_p->Service.PkAssetGenKey.PubKey_p,
                            CommandToken_p->Service.PkAssetGenKey.PubKeySize,
                            (void *)&BufManagerTokenID);
        if (PubKeyAddr == 0)
        {
            goto error_func_exit;
        }
#ifdef VEX_DISABLE_TOKENID_WR_KEYPAIR
        PubKeySize = CommandToken_p->Service.PkAssetGenKey.PubKeySize;

        BufManager_Zeroize(PubKeyAddr);
#else
#ifdef VEX_KEYPAIR_PUBKEYSIZE_NOTOKENID
        PubKeySize = CommandToken_p->Service.PkAssetGenKey.PubKeySize;
#else
        PubKeySize = BufManager_GetSize(PubKeyAddr);
#endif
#endif
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetGenKey.Method,
        (uint8_t)((CommandToken_p->Service.PkAssetGenKey.ModulusSizeInBits+31)/32),
        (uint8_t)((CommandToken_p->Service.PkAssetGenKey.DivisorSizeInBits+31)/32),
        0,
        CommandToken_p->Service.PkAssetGenKey.PrivKeyAssetId,
        CommandToken_p->Service.PkAssetGenKey.DomainAssetId,
        CommandToken_p->Service.PkAssetGenKey.PubKeyAssetId,
        KeyBlobAddr, (uint16_t)KeyBlobSize,
        PubKeyAddr, (uint16_t)PubKeySize);
    if (KeyBlobAddr != 0)
    {
        Eip130Token_Command_Pk_Asset_SetAdditionalAssetId(
            &CommandToken,
            CommandToken_p->Service.PkAssetGenKey.KekAssetId);
        Eip130Token_Command_Pk_Asset_SetAdditionalData(
            &CommandToken,
            CommandToken_p->Service.PkAssetGenKey.AssociatedData,
            (uint8_t)CommandToken_p->Service.PkAssetGenKey.AssociatedDataSize);
        Eip130Token_Command_Pk_Asset_AddlenCorrection(&CommandToken, sizeof(uint32_t));
    }

#ifdef VEX_DISABLE_TOKENID_WR_KEYPAIR
    /* Skip WrTokenID indication set */
    Eip130Token_Command_SetTokenID(&CommandToken, TokenID, false);
#else
    if ((KeyBlobAddr == 0) && (PubKeyAddr == 0))
    {
        Eip130Token_Command_SetTokenID(&CommandToken, TokenID, false);
    }
    else
    {
        Vex_Command_SetTokenID(&CommandToken, TokenID);
    }
#endif

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
        }
    }

error_func_exit:
    // Release used buffers, if needed
    if (KeyBlobAddr != 0)
    {
        int rc = BufManager_Unmap(KeyBlobAddr, true, fCopy);
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
    if (PubKeyAddr != 0)
    {
        int rc = BufManager_Unmap(PubKeyAddr, true, fCopy);
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
 * vex_Asym_AssetGenKeyPublic
 */
VexStatus_t
vex_Asym_AssetGenKeyPublic(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    bool fCopy = false;
    uint64_t PubKeyAddr = 0;
    size_t PubKeySize = 0;
    uint16_t TokenID;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    TokenID = vex_DeviceGetTokenID();

    if (CommandToken_p->Service.PkAssetGenKey.PubKey_p != NULL)
    {
        // Get public key output address for token
        PubKeyAddr = BufManager_Map(
                            CommandToken_p->fFromUserSpace,
                            BUFMANAGER_BUFFERTYPE_OUT,
                            CommandToken_p->Service.PkAssetGenKey.PubKey_p,
                            CommandToken_p->Service.PkAssetGenKey.PubKeySize,
                            (void *)&TokenID);
        if (PubKeyAddr == 0)
        {
            return VEX_NO_MEMORY;
        }
        PubKeySize = CommandToken_p->Service.PkAssetGenKey.PubKeySize;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetGenKey.Method,
        (uint8_t)((CommandToken_p->Service.PkAssetGenKey.ModulusSizeInBits+31)/32),
        (uint8_t)((CommandToken_p->Service.PkAssetGenKey.DivisorSizeInBits+31)/32),
        0,
        CommandToken_p->Service.PkAssetGenKey.PrivKeyAssetId,
        CommandToken_p->Service.PkAssetGenKey.DomainAssetId,
        CommandToken_p->Service.PkAssetGenKey.PubKeyAssetId,
        0, 0,
        PubKeyAddr, (uint16_t)PubKeySize);
    if (PubKeyAddr == 0)
    {
        Eip130Token_Command_SetTokenID(&CommandToken, TokenID, false);
    }
    else
    {
        Vex_Command_SetTokenID(&CommandToken, TokenID);
    }

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
        }
    }

    // Release used buffer, if needed
    if (PubKeyAddr != 0)
    {
        int rc = BufManager_Unmap(PubKeyAddr, true, fCopy);
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


/* end of file adapter_vex_asym_asset_genkey.c */
