/* adapter_val_asym_curve25519.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto Elliptic Curve 25519
 * Diffie-Hellman related services.
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

#ifndef VAL_REMOVE_ASYM_CURVE25519

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_asym.h"               // the API to implement
#include "api_val_asset.h"              // Asset Management related information
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken(), valInternal_*()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_AsymCurve25519AllocPublicKeyAsset
 */
ValStatus_t
val_AsymCurve25519AllocPrivateKeyAsset(
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if (AssetId_p == NULL)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_KEYEXCHANGE | EIP130_ASSET_POLICY_ACA_CURVE25519 | EIP130_ASSET_POLICY_PRIVATEDATA;
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
    return val_AssetAlloc(AssetPolicy, VAL_ASYM_DATA_SIZE_VWB(255),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}


/*----------------------------------------------------------------------------
 * val_AsymCurve25519AllocPublicKeyAsset
 */
ValStatus_t
val_AsymCurve25519AllocPublicKeyAsset(
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if (AssetId_p == NULL)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_KEYEXCHANGE | EIP130_ASSET_POLICY_ACA_CURVE25519;
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
    return val_AssetAlloc(AssetPolicy, VAL_ASYM_DATA_SIZE_VWB(255),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}

/*----------------------------------------------------------------------------
 * val_AsymCurve25519LoadKeyAssetPlaintext
 */
ValStatus_t
val_AsymCurve25519LoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const KeyValue_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    return val_AsymEccLoadPrivateKeyAssetPlaintext(KeyValue_p, 255,
                                                   TargetAssetId,
                                                   KekAssetId,
                                                   AssociatedData_p, AssociatedDataSize,
                                                   KeyBlob_p, KeyBlobSize_p);
}

/*----------------------------------------------------------------------------
 * val_AsymCurve25519GenKeyPair
 */
ValStatus_t
val_AsymCurve25519GenKeyPair(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValAsymBigInt_t * const PubKeyValue_p,
        ValOctetsOut_Optional_t * const PrivKeyBlob_p,
        ValSize_t * const PrivKeyBlobSize_p)
{
    ValStatus_t funcres;
    uint8_t * Data_p = NULL;
    ValSize_t PrvKeyDataSize = 0;
    ValSize_t PubKeyDataSize = 0;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

#ifdef VAL_STRICT_ARGS
    if ((PubKey_p == NULL) ||
        (PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->ModulusSizeBits != 255) ||
        (PrivKey_p == NULL) ||
        (PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey_p->ModulusSizeBits != PubKey_p->ModulusSizeBits))
    {
        return VAL_BAD_ARGUMENT;
    }

    if ((KekAssetId != VAL_ASSETID_INVALID) &&
        ((AssociatedData_p == NULL) ||
         (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
         (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
         (PrivKeyBlobSize_p == NULL)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine the keyblob size for the private key
    PrvKeyDataSize = VAL_KEYBLOB_SIZE(VAL_ASYM_DATA_SIZE_VWB(PrivKey_p->ModulusSizeBits));
    if ((PrivKeyBlobSize_p != NULL) &&
        ((PrivKeyBlob_p == NULL) || (*PrivKeyBlobSize_p < PrvKeyDataSize)))
    {
        *PrivKeyBlobSize_p = PrvKeyDataSize;
        return VAL_BUFFER_TOO_SMALL;
    }

    if (PubKeyValue_p != NULL)
    {
        // Validate application format public key data size
#ifdef VAL_STRICT_ARGS
        PubKeyDataSize = VAL_ASYM_DATA_SIZE_B2B(PubKey_p->ModulusSizeBits);
        if ((PubKeyValue_p->Data_p == NULL) ||
            (PubKeyValue_p->ByteDataSize < PubKeyDataSize))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif

        // Determine the HW size for the public key
        PubKeyDataSize = VAL_ASYM_DATA_SIZE_VWB(PubKey_p->ModulusSizeBits);

        // Allocate buffer for the public key data
        Data_p = Adapter_Alloc((unsigned int)PubKeyDataSize);
        if (Data_p == NULL)
        {
            return VAL_NO_MEMORY;
        }
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetGenKey.Method = VEXTOKEN_PKASSET_CURVE25519_GEN_KEYPAIR;
    t_cmd.Service.PkAssetGenKey.ModulusSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenKey.DivisorSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenKey.PubKeyAssetId = PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenKey.PrivKeyAssetId = PrivKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenKey.DomainAssetId = PubKey_p->DomainAssetId;
    t_cmd.Service.PkAssetGenKey.PubKey_p = (uint8_t *)Data_p;
    t_cmd.Service.PkAssetGenKey.PubKeySize = (uint32_t)PubKeyDataSize;
    t_cmd.Service.PkAssetGenKey.KekAssetId = (uint32_t)KekAssetId;
    if ((KekAssetId != VAL_ASSETID_INVALID) &&
        (PrivKeyBlob_p != NULL) && (PrivKeyBlobSize_p != NULL))
    {
        if (AssociatedDataSize > sizeof(t_cmd.Service.PkAssetGenKey.AssociatedData))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.PkAssetGenKey.AssociatedData, AssociatedData_p, AssociatedDataSize);
        t_cmd.Service.PkAssetGenKey.AssociatedDataSize = (uint32_t)AssociatedDataSize;
        t_cmd.Service.PkAssetGenKey.KeyBlob_p = (uint8_t *)PrivKeyBlob_p;
        t_cmd.Service.PkAssetGenKey.KeyBlobSize = (uint32_t)*PrivKeyBlobSize_p;
    }
    else
    {
        t_cmd.Service.PkAssetGenKey.AssociatedDataSize = 0;
        t_cmd.Service.PkAssetGenKey.KeyBlob_p = NULL;
        t_cmd.Service.PkAssetGenKey.KeyBlobSize = 0;
    }
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Convert the public key from HW to application format
            if (PubKeyValue_p != NULL)
            {
                valInternal_AsymBigIntFromHW(Data_p,
                                             PubKey_p->ModulusSizeBits,
                                             PubKeyValue_p);
            }

            // Set the actual keyblob size
            if (PrivKeyBlobSize_p != NULL)
            {
                *PrivKeyBlobSize_p = PrvKeyDataSize;
            }
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    // Clean-up
    if (Data_p != NULL)
    {
        Adapter_Free(Data_p);
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * val_AsymCurve25519GenPublicKey
 */
ValStatus_t
val_AsymCurve25519GenPublicKey(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymBigInt_t * const PubKeyValue_p)
{
    ValStatus_t funcres;
    uint8_t * Data_p = NULL;
    ValSize_t DataSize = 0;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

#ifdef VAL_STRICT_ARGS
    if ((PubKey_p == NULL) ||
        (PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (PubKey_p->ModulusSizeBits != 255) ||
        (PrivKey_p == NULL) ||
        (PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey_p->ModulusSizeBits != PubKey_p->ModulusSizeBits))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (PubKeyValue_p != NULL)
    {
#ifdef VAL_STRICT_ARGS
        // Validate application format public key data size
        DataSize = VAL_ASYM_DATA_SIZE_B2B(PubKey_p->ModulusSizeBits);
        if ((PubKeyValue_p->Data_p == NULL) ||
            (PubKeyValue_p->ByteDataSize < DataSize))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif

        // Determine the HW size for the public key
        DataSize = VAL_ASYM_DATA_SIZE_VWB(PubKey_p->ModulusSizeBits);

        // Allocate buffer for the public key data
        Data_p = Adapter_Alloc((unsigned int)DataSize);
        if (Data_p == NULL)
        {
            return VAL_NO_MEMORY;
        }
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetGenKey.Method = VEXTOKEN_PKASSET_CURVE25519_GEN_PUBKEY;
    t_cmd.Service.PkAssetGenKey.ModulusSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenKey.DivisorSizeInBits = (uint32_t)PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenKey.PubKeyAssetId = PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenKey.PrivKeyAssetId = PrivKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenKey.DomainAssetId = PubKey_p->DomainAssetId;
    t_cmd.Service.PkAssetGenKey.PubKey_p = (uint8_t *)Data_p;
    t_cmd.Service.PkAssetGenKey.PubKeySize = (uint32_t)DataSize;
    t_cmd.Service.PkAssetGenKey.KekAssetId = VAL_ASSETID_INVALID;
    t_cmd.Service.PkAssetGenKey.AssociatedDataSize = 0;
    t_cmd.Service.PkAssetGenKey.KeyBlob_p = NULL;
    t_cmd.Service.PkAssetGenKey.KeyBlobSize = 0;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Convert the public key from HW to application format
            if (PubKeyValue_p != NULL)
            {
                valInternal_AsymBigIntFromHW(Data_p,
                                             PubKey_p->ModulusSizeBits,
                                             PubKeyValue_p);
            }
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    // Clean-up
    if (Data_p != NULL)
    {
        Adapter_Free(Data_p);
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * val_AsymCurve25519GenSharedSecret
 */
ValStatus_t
val_AsymCurve25519GenSharedSecret(
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
        (PubKey_p->ModulusSizeBits != 255) ||
        (PrivKey_p == NULL) ||
        (PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (PrivKey_p->DomainAssetId != PubKey_p->DomainAssetId) ||
        (PrivKey_p->ModulusSizeBits !=  PubKey_p->ModulusSizeBits) ||
        (!fSaveSharedSecret && (OtherInfo_p == NULL) && (OtherInfoSize != 0)) ||
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
    t_cmd.Service.PkAssetGenSharedSecret.Method = VEXTOKEN_PKASSET_CURVE25519_GEN_SHARED_SECRET;
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


#endif /* !VAL_REMOVE_ASYM_CURVE25519 */

/* end of file adapter_val_asym_curve25519.c */
