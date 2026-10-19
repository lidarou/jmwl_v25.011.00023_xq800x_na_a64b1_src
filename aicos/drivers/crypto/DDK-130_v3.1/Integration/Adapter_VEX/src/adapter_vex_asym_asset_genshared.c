/* adapter_vex_asym_asset_genshared.c
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
 * vex_Asym_AssetGenSharedSecret
 */
VexStatus_t
vex_Asym_AssetGenSharedSecret(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint64_t LocalEphPubKey = 0;
    uint64_t OtherInfoAddr = 0;
    size_t OtherInfoSize = 0;
    size_t LocalEphPubKeySize = 0;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if ((CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfo_p != NULL) &&
        (CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfoSize != 0))
    {
        // Get OtherInfo input address for token
        OtherInfoSize = CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfoSize;
        OtherInfoAddr = BufManager_Map(
                            CommandToken_p->fFromUserSpace,
                            BUFMANAGER_BUFFERTYPE_IN,
                            CommandToken_p->Service.PkAssetGenSharedSecret.OtherInfo_p,
                            OtherInfoSize,
                            NULL);
        if (OtherInfoAddr == 0)
        {
            goto error_func_exit;
        }
    }

    if ((CommandToken_p->Service.PkAssetGenSharedSecret.Method == VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET) &&
        (CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKey != NULL) &&
        (CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKeySize != 0))
    {
        // Get local ephemeral public key input address for token
        LocalEphPubKeySize = CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKeySize;
        LocalEphPubKey = BufManager_Map(
                             CommandToken_p->fFromUserSpace,
                             BUFMANAGER_BUFFERTYPE_IN,
                             CommandToken_p->Service.PkAssetGenSharedSecret.LocalEphPubKey,
                             LocalEphPubKeySize,
                             NULL);
        if (LocalEphPubKey == 0)
        {
            goto error_func_exit;
        }
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetGenSharedSecret.Method,
        (uint8_t)((CommandToken_p->Service.PkAssetGenSharedSecret.ModulusSizeInBits+31)/32),
        (uint8_t)((CommandToken_p->Service.PkAssetGenSharedSecret.DivisorSizeInBits+31)/32),
        (uint8_t)CommandToken_p->Service.PkAssetGenSharedSecret.AssetIdListSize,
        CommandToken_p->Service.PkAssetGenSharedSecret.PrivKeyAssetId,
        CommandToken_p->Service.PkAssetGenSharedSecret.DomainAssetId,
        CommandToken_p->Service.PkAssetGenSharedSecret.PubKeyAssetId,
        OtherInfoAddr, (uint16_t)OtherInfoSize,
        LocalEphPubKey, (uint16_t)LocalEphPubKeySize);

    if ((CommandToken_p->Service.PkAssetGenSharedSecret.Method == VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_DKEYPAIR) ||
        (CommandToken_p->Service.PkAssetGenSharedSecret.Method == VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR) ||
        (CommandToken_p->Service.PkAssetGenSharedSecret.Method == VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET))
    {
        if ((CommandToken_p->Service.PkAssetGenSharedSecret.PubKey2AssetId == 0) ||
            (CommandToken_p->Service.PkAssetGenSharedSecret.PrivKey2AssetId == 0))
        {
            funcres = VEX_BAD_ARGUMENT;
            goto error_func_exit;
        }

        Eip130Token_Command_Pk_Asset_SetAdditionalAssetId(
            &CommandToken,
            CommandToken_p->Service.PkAssetGenSharedSecret.PrivKey2AssetId);
        Eip130Token_Command_Pk_Asset_SetAdditionalAssetId(
            &CommandToken,
            CommandToken_p->Service.PkAssetGenSharedSecret.PubKey2AssetId);
    }

    {
        uint32_t i;

        for (i = 0; i < CommandToken_p->Service.PkAssetGenSharedSecret.AssetIdListSize; i++)
        {
            Eip130Token_Command_Pk_Asset_SetAdditionalAssetId(
                &CommandToken,
                CommandToken_p->Service.PkAssetGenSharedSecret.AssetIdList[i]);
        }
    }
    if (CommandToken_p->Service.PkAssetGenSharedSecret.fSaveSharedSecret)
    {
        Eip130Token_Command_Pk_Asset_SaveSharedSecret(&CommandToken);
    }

    Eip130Token_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID(), false);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
    }

error_func_exit:
    // Release used buffers, if needed
    if (LocalEphPubKey != 0)
    {
        (void)BufManager_Unmap(LocalEphPubKey, false, false);
    }
    if (OtherInfoAddr != 0)
    {
        (void)BufManager_Unmap(OtherInfoAddr, false, false);
    }

    return funcres;
}


/* end of file adapter_vex_asym_asset_genshared.c */
