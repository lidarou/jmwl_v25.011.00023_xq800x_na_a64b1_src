/* adapter_vex_asym_asset_ecies.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the ECIES VEX API
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
 * vex_Asym_AssetEcies
 */
#ifdef VEX_ENABLE_ASYM_ECIES
VexStatus_t
vex_Asym_AssetEcies(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint64_t OtherInfoAddr  = 0;
    uint64_t OutputDataAddr = 0;
    uint64_t WrapDataAddr = 0;
    size_t OtherInfoSize  = 0;
    size_t OutputDataSize = 0;
    size_t WrapDataSize = 0;
    uint16_t TokenID = 0;
    bool fCopy = false;

    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    /* Required data for both WRAP and UNWRAP operations */
    if ((CommandToken_p->Service.PkEcies.Data_p == NULL) ||
        (CommandToken_p->Service.PkEcies.DataSize == 0) ||
        (CommandToken_p->Service.PkEcies.AssetDataId == 0) ||
        (CommandToken_p->Service.PkEcies.KeyAssetId == 0) ||
        (CommandToken_p->Service.PkEcies.DomainAssetId == 0))
    {
        return VEX_BAD_ARGUMENT;
    }

    TokenID = vex_DeviceGetTokenID();

    /* Operation specific token preparations & checks */
    if (CommandToken_p->Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP)
    {
        if ((CommandToken_p->Service.PkEcies.EphPubKey_p == NULL) ||
            (CommandToken_p->Service.PkEcies.EphPubKeySize == 0))
        {
            return VEX_BAD_ARGUMENT;
        }

        /* Map output buffer for ephemeral pubkey & Wrapped data */
        OutputDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                        BUFMANAGER_BUFFERTYPE_OUT,
                                        CommandToken_p->Service.PkEcies.EphPubKey_p,
                                        CommandToken_p->Service.PkEcies.EphPubKeySize,
                                        (void *)&TokenID);
        if (OutputDataAddr == 0)
        {
            goto error_func_exit;
        }
        OutputDataSize = BufManager_GetSize(OutputDataAddr);

        WrapDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                      BUFMANAGER_BUFFERTYPE_OUT,
                                      CommandToken_p->Service.PkEcies.Data_p,
                                      CommandToken_p->Service.PkEcies.DataSize,
                                      (void *)&TokenID);
        if (WrapDataAddr == 0)
        {
            goto error_func_exit;
        }
        WrapDataSize = BufManager_GetSize(WrapDataAddr);
    }
    else
    {
        if (CommandToken_p->Service.PkEcies.EphPubKeyAssetId == 0)
        {
            return VEX_BAD_ARGUMENT;
        }

        /* Map input buffer for wrapped data */
        OutputDataSize = CommandToken_p->Service.PkEcies.DataSize;
        OutputDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                        BUFMANAGER_BUFFERTYPE_IN,
                                        CommandToken_p->Service.PkEcies.Data_p,
                                        OutputDataSize,
                                        NULL);
        if (OutputDataAddr == 0)
        {
            goto error_func_exit;
        }
    }

    if ((CommandToken_p->Service.PkEcies.OtherInfo_p != NULL) &&
        (CommandToken_p->Service.PkEcies.OtherInfoSize != 0))
    {
        OtherInfoSize = CommandToken_p->Service.PkEcies.OtherInfoSize;
        OtherInfoAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                       BUFMANAGER_BUFFERTYPE_IN,
                                       CommandToken_p->Service.PkEcies.OtherInfo_p,
                                       OtherInfoSize,
                                       NULL);
        if (OtherInfoAddr == 0)
        {
            goto error_func_exit;
        }
    }

    Eip130Token_Command_Pk_Asset_Command(&CommandToken,
                                         (uint8_t)CommandToken_p->Service.PkEcies.Method,
                                         (uint8_t)((CommandToken_p->Service.PkEcies.ModulusSizeInBits+31)/32),
                                         (uint8_t)((CommandToken_p->Service.PkEcies.ModulusSizeInBits+31)/32),
                                         0,
                                         CommandToken_p->Service.PkEcies.KeyAssetId,
                                         CommandToken_p->Service.PkEcies.DomainAssetId,
                                         CommandToken_p->Service.PkEcies.AssetDataId,
                                         OtherInfoAddr,
                                         (uint16_t)OtherInfoSize,
                                         OutputDataAddr,
                                         (uint16_t)OutputDataSize);

    if (CommandToken_p->Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP)
    {
        /* Set buffer where encrypted/wrapped data shall be written */
        Eip130Token_Command_Pk_Asset_SetAdditionalOutputData(&CommandToken,
                                                             WrapDataAddr,
                                                             (uint32_t)WrapDataSize);
    }
    else
    {
        Eip130Token_Command_Pk_Asset_SetAdditionalAssetId(&CommandToken,
                                                          CommandToken_p->Service.PkEcies.EphPubKeyAssetId);
    }

    Eip130Token_Command_SetTokenID(&CommandToken, TokenID, true);
    /* Initialize result token */
    ZEROINIT(ResultToken);

    /* Exchange token with the EIP-13x HW */
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if ((ResultToken_p->Result >= 0) &&
            (CommandToken_p->Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP))
        {
            fCopy = true;
        }
    }

error_func_exit:
    if (OtherInfoAddr != 0)
    {
        (void)BufManager_Unmap(OtherInfoAddr, false, false);
    }
    if (OutputDataAddr != 0)
    {
        if (CommandToken_p->Service.PkEcies.Method == VEXTOKEN_PKASSET_ECIESWRAP)
        {
            int rc = BufManager_Unmap(OutputDataAddr, true, fCopy);
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
        else
        {
            (void)BufManager_Unmap(OutputDataAddr, false, false);
        }
    }
    if(WrapDataAddr != 0)
    {
        int rc = BufManager_Unmap(WrapDataAddr, true, fCopy);
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
#endif /* VEX_ENABLE_ASYM_ECIES */

/* end of file adapter_vex_asym_asset_ecies.c */
