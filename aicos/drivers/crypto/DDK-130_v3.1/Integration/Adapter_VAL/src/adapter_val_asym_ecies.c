/* adapter_val_asym_ecies.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the ECIES related services.
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

#include "c_adapter_val.h"              // configuration

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_asym.h"               // the API to implement
#include "adapter_val_internal.h"       // Various defines and typedefs


/*----------------------------------------------------------------------------
 * val_AsymECIESWrap
 */
#ifndef VAL_REMOVE_ASYM_ECIES
ValStatus_t
val_AsymECIESWrap(
        ValAsymKey_t * const PubKeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAsymECCPoint_t * const EphemeralPubkey_p,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize)
{
    ValStatus_t funcres = VAL_SUCCESS;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    uint8_t * PubkeyData_p = NULL;
    ValSize_t PubKeySize = 0;

#ifdef VAL_STRICT_ARGS
    if ((PubKeyContext_p == NULL) ||
        (PubKeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (AssetId == VAL_ASSETID_INVALID) ||
        (EphemeralPubkey_p == NULL) ||
        (WrappedData_p == NULL) ||
        (WrappedDataSize == 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine the HW size for the public key
    PubKeySize = (2 * VAL_ASYM_DATA_SIZE_VWB(PubKeyContext_p->ModulusSizeBits));

    // Allocate buffer for the public key data
    PubkeyData_p = Adapter_Alloc((unsigned int)PubKeySize);
    if (PubkeyData_p == NULL)
    {
        return VAL_NO_MEMORY;
    }

    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkEcies.Method = VEXTOKEN_PKASSET_ECIESWRAP;

    t_cmd.Service.PkEcies.KeyAssetId = PubKeyContext_p->KeyAssetId;
    t_cmd.Service.PkEcies.DomainAssetId = PubKeyContext_p->DomainAssetId;
    t_cmd.Service.PkEcies.ModulusSizeInBits = (uint32_t)PubKeyContext_p->ModulusSizeBits;

    t_cmd.Service.PkEcies.OtherInfo_p = OtherInfo_p;
    t_cmd.Service.PkEcies.OtherInfoSize = (uint32_t)OtherInfoSize;
    t_cmd.Service.PkEcies.Data_p = WrappedData_p;
    t_cmd.Service.PkEcies.DataSize = (uint32_t)WrappedDataSize;
    t_cmd.Service.PkEcies.EphPubKey_p = (uint8_t *)PubkeyData_p;
    t_cmd.Service.PkEcies.EphPubKeySize = (uint32_t)PubKeySize;
    t_cmd.Service.PkEcies.AssetDataId = AssetId;
    t_res.Result = 0;

    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result == VEXTOKEN_RESULT_NO_ERROR))
    {
        // Convert the public key from HW to application format
        valInternal_AsymECPointFromHW(PubkeyData_p,
                                      PubKeyContext_p->ModulusSizeBits,
                                      EphemeralPubkey_p);
    }
    else
    {
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    // Clean-up
    if (PubkeyData_p != NULL)
    {
        Adapter_Free(PubkeyData_p);
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * val_AsymECIESUnwrap
 */
ValStatus_t
val_AsymECIESUnwrap(
        ValAsymKey_t * const PrivkeyContext_p,
        const ValAssetId_t AssetId,
        const ValAssetId_t EphemeralPubkeyAssetId,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize)
{
    ValStatus_t funcres = VAL_SUCCESS;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

#ifdef VAL_STRICT_ARGS
    if ((PrivkeyContext_p == NULL) ||
        (PrivkeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivkeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (AssetId == VAL_ASSETID_INVALID) ||
        (EphemeralPubkeyAssetId == VAL_ASSETID_INVALID) ||
        (WrappedData_p == NULL) ||
        (WrappedDataSize == 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkEcies.Method = VEXTOKEN_PKASSET_ECIESUNWRAP;

    t_cmd.Service.PkEcies.KeyAssetId = PrivkeyContext_p->KeyAssetId;
    t_cmd.Service.PkEcies.DomainAssetId = PrivkeyContext_p->DomainAssetId;
    t_cmd.Service.PkEcies.ModulusSizeInBits = (uint32_t)PrivkeyContext_p->ModulusSizeBits;

    t_cmd.Service.PkEcies.EphPubKeyAssetId = EphemeralPubkeyAssetId;
    t_cmd.Service.PkEcies.AssetDataId = AssetId;
    t_cmd.Service.PkEcies.OtherInfo_p = OtherInfo_p;
    t_cmd.Service.PkEcies.OtherInfoSize = (uint32_t)OtherInfoSize;
    t_cmd.Service.PkEcies.Data_p = WrappedData_p;
    t_cmd.Service.PkEcies.DataSize = (uint32_t)WrappedDataSize;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Check for errors
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    return funcres;
}
#endif /* VAL_REMOVE_ASYM_ECIES */

/* end of file adapter_val_asym_ecies.c */
