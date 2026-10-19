/* adapter_val_asym_ecdh.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto Elliptic Curve Diffie-Hellman
 * related services.
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
#include "api_val_asset.h"              // Asset Management related information
#include "api_val_sym.h"                // ValSymContextPtr_Optional_t, ValSymAlgo_t
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken(), valInternal_*()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_AsymEcdhAllocPrivateKeyAsset
 */
#if !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL)
ValStatus_t
val_AsymEcdhAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_KEYEXCHANGE | EIP130_ASSET_POLICY_ACA_ECDH | EIP130_ASSET_POLICY_PRIVATEDATA;
    if (fAllowAny)
    {
        AssetPolicy |= EIP130_ASSET_POLICY_STANY;
    }
#else
    AssetPolicy = VAL_POLICY_PK_ECDH_KEY;
    IDENTIFIER_NOT_USED(fAllowAny);
#endif
    if (fCrossDomain)
    {
        AssetPolicy |= VAL_POLICY_CROSS_DOMAIN;
    }
    if (fExportAllowed)
    {
        AssetPolicy |= VAL_POLICY_EXPORT;
    }
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }

    // Determine the Asset size and allocate the Asset
    return val_AssetAlloc(AssetPolicy,
                          VAL_ASYM_DATA_SIZE_VWB(ModulusSizeBits),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDhAllocPublicKeyAsset
 */
#if !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL)
ValStatus_t
val_AsymEcdhAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_KEYEXCHANGE | EIP130_ASSET_POLICY_ACA_ECDH;
#else
    AssetPolicy = VAL_POLICY_PUBLIC_KEY | VAL_POLICY_PK_ECDH_KEY;
#endif
    if (fCrossDomain)
    {
        AssetPolicy |= VAL_POLICY_CROSS_DOMAIN;
    }
    if (fExportAllowed)
    {
        AssetPolicy |= VAL_POLICY_EXPORT;
    }
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }

    // Determine the Asset size and allocate the Asset
    return val_AssetAlloc(AssetPolicy,
                          (2 * VAL_ASYM_DATA_SIZE_VWB(ModulusSizeBits)),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymEcdhGenSharedSecretSingle
 */
#ifndef VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE
ValStatus_t
val_AsymEcdhGenSharedSecretSingle(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets)
{
    ValStatus_t funcres;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    unsigned int i;

#ifdef VAL_STRICT_ARGS
    if ((PubKey_p == NULL) ||
        (PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (PubKey_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (PrivKey_p == NULL) ||
        (PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey_p->ModulusSizeBits !=  PubKey_p->ModulusSizeBits) ||
        (!fSaveSharedSecret && (OtherInfo_p == NULL) && (OtherInfoSize != 0))  ||
        (AssetIdList_p == NULL) || (NumberOfAssets == 0) ||
        (NumberOfAssets > VEXTOKEN_MAX_PK_NUM_ASSETS) ||
        (fSaveSharedSecret && (NumberOfAssets != 1)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetGenSharedSecret.Method = VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_SKEYPAIR;
    t_cmd.Service.PkAssetGenSharedSecret.ModulusSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenSharedSecret.DivisorSizeInBits = (uint32_t)PrivKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenSharedSecret.PubKeyAssetId = (uint32_t)PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKeyAssetId = (uint32_t)PrivKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.DomainAssetId = (uint32_t)PubKey_p->DomainAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PubKey2AssetId = VAL_ASSETID_INVALID;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKey2AssetId = VAL_ASSETID_INVALID;
    t_cmd.Service.PkAssetGenSharedSecret.OtherInfo_p = OtherInfo_p;
    t_cmd.Service.PkAssetGenSharedSecret.OtherInfoSize = (uint32_t)OtherInfoSize;
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKey = NULL;
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKeySize = 0;
    t_cmd.Service.PkAssetGenSharedSecret.fSaveSharedSecret = fSaveSharedSecret;
    t_cmd.Service.PkAssetGenSharedSecret.AssetIdListSize = (uint32_t)NumberOfAssets;
    for (i = 0; i < NumberOfAssets; i++)
    {
        t_cmd.Service.PkAssetGenSharedSecret.AssetIdList[i] = (uint32_t)AssetIdList_p[i];
    }
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, funcres);
    }

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDhGenSharedSecretDual
 */
#ifndef VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL
ValStatus_t
val_AsymEcdhGenSharedSecretDual(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymKey_t * const PubKey2_p,
        ValAsymKey_t * const PrivKey2_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets)
{
    ValStatus_t funcres;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    unsigned int i;

#ifdef VAL_STRICT_ARGS
    if ((PubKey_p == NULL) ||
        (PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (PubKey_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (PrivKey_p == NULL) ||
        (PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey_p->ModulusSizeBits !=  PubKey_p->ModulusSizeBits) ||
        (PubKey2_p == NULL) ||
        (PubKey2_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKey2_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PubKey2_p->ModulusSizeBits !=  PubKey_p->ModulusSizeBits) ||
        (PrivKey2_p == NULL) ||
        (PrivKey2_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey2_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey2_p->ModulusSizeBits !=  PubKey_p->ModulusSizeBits) ||
        (!fSaveSharedSecret && (OtherInfo_p == NULL) && (OtherInfoSize != 0))  ||
        (AssetIdList_p == NULL) || (NumberOfAssets == 0) ||
        (NumberOfAssets > VEXTOKEN_MAX_PK_NUM_ASSETS) ||
        (fSaveSharedSecret && (NumberOfAssets != 1)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetGenSharedSecret.Method = VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR;
    t_cmd.Service.PkAssetGenSharedSecret.ModulusSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenSharedSecret.DivisorSizeInBits = (uint32_t)PrivKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenSharedSecret.PubKeyAssetId = (uint32_t)PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKeyAssetId = (uint32_t)PrivKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.DomainAssetId = (uint32_t)PubKey_p->DomainAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PubKey2AssetId = (uint32_t)PubKey2_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKey2AssetId = (uint32_t)PrivKey2_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.OtherInfo_p = OtherInfo_p;
    t_cmd.Service.PkAssetGenSharedSecret.OtherInfoSize = (uint32_t)OtherInfoSize;
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKey = NULL;
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKeySize = 0;
    t_cmd.Service.PkAssetGenSharedSecret.fSaveSharedSecret = fSaveSharedSecret;
    t_cmd.Service.PkAssetGenSharedSecret.AssetIdListSize = (uint32_t)NumberOfAssets;
    for (i = 0; i < NumberOfAssets; i++)
    {
        t_cmd.Service.PkAssetGenSharedSecret.AssetIdList[i] = (uint32_t)AssetIdList_p[i];
    }
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, funcres);
    }

    return funcres;
}
#endif


/* end of file adapter_val_asym_ecdh.c */
