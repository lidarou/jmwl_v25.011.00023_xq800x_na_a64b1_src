/* adapter_vex_asym_asset_keywrap.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the asymmetric crypto services for key wrap and unwrap.
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
 * vex_Asym_AssetKeyWrap
 */
VexStatus_t
vex_Asym_AssetKeyWrap(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    bool fCopy = false;
    uint64_t InDataBlobAddr = 0;
    uint64_t OutDataBlobAddr = 0;
    uint16_t InBlobSize = 0;
    uint16_t OutBlobSize = 0;
    uint16_t TokenID;
    uint8_t ModulusWords;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if (CommandToken_p->Service.PkAssetWrap.AdditionalInputSize > 208)
    {
        return VEX_BAD_ARGUMENT;
    }

    // Get key blob data address for token can be input or output
    TokenID = vex_DeviceGetTokenID();
    if ((CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING) ||
        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH) ||
        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP) ||
        (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_SM2ENC_WRAP))
    {
        OutDataBlobAddr = BufManager_Map(
                              CommandToken_p->fFromUserSpace,
                              BUFMANAGER_BUFFERTYPE_OUT,
                              CommandToken_p->Service.PkAssetWrap.Data_p,
                              CommandToken_p->Service.PkAssetWrap.DataSize,
                              (void *)&TokenID);
        if (OutDataBlobAddr == 0)
        {
            goto error_func_exit;
        }

        if (CommandToken_p->Service.PkAssetWrap.Method == VEXTOKEN_PKASSET_SM2ENC_WRAP)
        {
            /* SM2ENC output data size checking does not support TokenId size inclusion */
            OutBlobSize = (uint16_t)CommandToken_p->Service.PkAssetWrap.DataSize;
        }
        else
        {
            OutBlobSize = (uint16_t)BufManager_GetSize(OutDataBlobAddr);
        }
    }
    else
    {
        InBlobSize = (uint16_t)CommandToken_p->Service.PkAssetWrap.DataSize;
        InDataBlobAddr = BufManager_Map(
                             CommandToken_p->fFromUserSpace,
                             BUFMANAGER_BUFFERTYPE_IN,
                             CommandToken_p->Service.PkAssetWrap.Data_p,
                             InBlobSize,
                             NULL);
        if (InDataBlobAddr == 0)
        {
            goto error_func_exit;
        }
    }

    ModulusWords = (uint8_t)((CommandToken_p->Service.PkAssetWrap.ModulusSizeInBits + 31) / 32);

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Asset_Command(
        &CommandToken,
        (uint8_t)CommandToken_p->Service.PkAssetWrap.Method,
        ModulusWords, 0,
        0,
        CommandToken_p->Service.PkAssetWrap.KeyAssetId,
        CommandToken_p->Service.PkAssetWrap.DomainAssetId,
        CommandToken_p->Service.PkAssetWrap.AssetId,
        InDataBlobAddr, InBlobSize,
        OutDataBlobAddr, OutBlobSize);
    Eip130Token_Command_Pk_Asset_SetAdditionalData(
        &CommandToken,
        CommandToken_p->Service.PkAssetWrap.AdditionalInput,
        (uint8_t)CommandToken_p->Service.PkAssetWrap.AdditionalInputSize);
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
            // Copy output data if needed
            fCopy = true;
        }
    }

error_func_exit:
    // Release used buffers, if needed
    if (InDataBlobAddr != 0)
    {
        (void)BufManager_Unmap(InDataBlobAddr, false, false);
    }
    if (OutDataBlobAddr != 0)
    {
        int rc = BufManager_Unmap(OutDataBlobAddr, true, fCopy);
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


/* end of file adapter_vex_asym_asset_keywrap.c */
