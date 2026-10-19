/* adapter_val_asym_dhdsa.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto Diffie-Hellman and DSA related
 * services.
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
 * val_AsymDhDsaAllocDomainAsset
 */
#if !defined(VAL_REMOVE_ASYM_DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL)

ValStatus_t
val_AsymDhDsaAllocDomainAsset(
        const ValSize_t PrimeSizeBits,
        const ValSize_t DivisorSizeBits,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (PrimeSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
        (PrimeSizeBits > VAL_ASYM_DH_DSA_MAX_BITS) ||
        (DivisorSizeBits < VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS) ||
        (DivisorSizeBits > VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_KEYPARAMS;
#else
    AssetPolicy = VAL_POLICY_PUBLIC_KEY_PARAM;
#endif
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }

    // Determine the Asset size and allocate the Asset
    return val_AssetAlloc(AssetPolicy,
                          ((2 * VAL_ASYM_DATA_SIZE_VWB(PrimeSizeBits)) +
                           VAL_ASYM_DATA_SIZE_VWB(DivisorSizeBits)),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDsaLoadDomainAssetPlaintext
 */
#if !defined(VAL_REMOVE_ASYM_DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL)
ValStatus_t
val_AsymDhDsaLoadDomainAssetPlaintext(
        const ValAsymDHDSADomainParam_t * const Domain_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    ValStatus_t funcres = VAL_INTERNAL_ERROR;
    uint8_t * AssetData_p;
    uint8_t * ptr;
    ValSize_t AssetSize;
    uint32_t ByteLength;
    int RemainingLength;

#ifdef VAL_STRICT_ARGS
    if ((Domain_p == NULL) ||
        (TargetAssetId == VAL_ASSETID_INVALID) ||
        (Domain_p->PrimeSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
        (Domain_p->PrimeSizeBits > VAL_ASYM_DH_DSA_MAX_BITS) ||
        (Domain_p->DivisorSizeBits < VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS) ||
        (Domain_p->DivisorSizeBits > VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS) ||
        (Domain_p->Prime.Data_p == NULL) ||
        (Domain_p->Prime.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(Domain_p->PrimeSizeBits)) ||
        (Domain_p->Divisor.Data_p == NULL) ||
        (Domain_p->Divisor.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(Domain_p->DivisorSizeBits)) ||
        (Domain_p->Base.Data_p == NULL) ||
        (Domain_p->Base.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(Domain_p->PrimeSizeBits)))
    {
        return VAL_BAD_ARGUMENT;
    }

    if ((KekAssetId != VAL_ASSETID_INVALID) &&
        ((AssociatedData_p == NULL) ||
         (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
         (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
         (KeyBlobSize_p == NULL)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine the Asset size
    AssetSize = (2 * VAL_ASYM_DATA_SIZE_VWB(Domain_p->PrimeSizeBits)) +
                VAL_ASYM_DATA_SIZE_VWB(Domain_p->DivisorSizeBits);
    if ((KeyBlobSize_p != NULL) &&
        ((KeyBlob_p == NULL) || (*KeyBlobSize_p < VAL_KEYBLOB_SIZE(AssetSize))))
    {
        *KeyBlobSize_p = VAL_KEYBLOB_SIZE(AssetSize);
        return VAL_BUFFER_TOO_SMALL;
    }

    // Allocate buffer for the Asset data
    AssetData_p = Adapter_Alloc((unsigned int)AssetSize);
    if (AssetData_p == NULL)
    {
        return VAL_NO_MEMORY;
    }
    ptr = AssetData_p;

    // Create domain parameters representation for plaintext asset load
    ByteLength = (uint32_t)VAL_ASYM_DATA_SIZE_B2WB(Domain_p->PrimeSizeBits);
    RemainingLength = (int)(ByteLength - Domain_p->Prime.ByteDataSize);
    if (RemainingLength < 0)
    {
        goto error_func_exit;
    }
    *ptr++ = (uint8_t)(Domain_p->PrimeSizeBits);
    *ptr++ = (uint8_t)(Domain_p->PrimeSizeBits >> 8);
    *ptr++ = 0;
    *ptr++ = 3;
    valInternal_ReverseMemCpy(ptr,
                              Domain_p->Prime.Data_p,
                              Domain_p->Prime.ByteDataSize);
    ptr += Domain_p->Prime.ByteDataSize;
    if (RemainingLength != 0)
    {
        memset(ptr, 0, (size_t)RemainingLength);
        ptr += RemainingLength;
    }

    RemainingLength = (int)(ByteLength - Domain_p->Base.ByteDataSize);
    if (RemainingLength < 0)
    {
        goto error_func_exit;
    }
    *ptr++ = (uint8_t)(Domain_p->PrimeSizeBits);
    *ptr++ = (uint8_t)(Domain_p->PrimeSizeBits >> 8);
    *ptr++ = 1;
    *ptr++ = 3;
    valInternal_ReverseMemCpy(ptr,
                              Domain_p->Base.Data_p,
                              Domain_p->Base.ByteDataSize);
    ptr += Domain_p->Base.ByteDataSize;
    if (RemainingLength != 0)
    {
        memset(ptr, 0, (size_t)RemainingLength);
        ptr += RemainingLength;
    }

    ByteLength = (uint32_t)VAL_ASYM_DATA_SIZE_B2WB(Domain_p->DivisorSizeBits);
    RemainingLength = (int)(ByteLength - Domain_p->Divisor.ByteDataSize);
    if (RemainingLength < 0)
    {
        goto error_func_exit;
    }
    *ptr++ = (uint8_t)(Domain_p->DivisorSizeBits);
    *ptr++ = (uint8_t)(Domain_p->DivisorSizeBits >> 8);
    *ptr++ = 2;
    *ptr++ = 3;
    valInternal_ReverseMemCpy(ptr,
                              Domain_p->Divisor.Data_p,
                              Domain_p->Divisor.ByteDataSize);
    ptr += Domain_p->Divisor.ByteDataSize;
    if (RemainingLength != 0)
    {
        memset(ptr, 0, (size_t)RemainingLength);
    }

    // Initialize the asset
    if (KekAssetId == VAL_ASSETID_INVALID)
    {
        funcres = val_AssetLoadPlaintext(TargetAssetId,
                                         (ValOctetsIn_t * const)AssetData_p,
                                         AssetSize);
    }
    else
    {
        funcres = val_AssetLoadPlaintextExport(TargetAssetId,
                                               (ValOctetsIn_t * const)AssetData_p,
                                               AssetSize,
                                               KekAssetId,
                                               AssociatedData_p,
                                               AssociatedDataSize,
                                               KeyBlob_p,
                                               KeyBlobSize_p);
    }

    // Clean-up
error_func_exit:
    Adapter_Free(AssetData_p);

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDhDsaLoadKeyAssetPlaintext
 */
#if !defined(VAL_REMOVE_ASYM_DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL)
ValStatus_t
val_AsymDhDsaLoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const Data_p,
        const ValSize_t DataSizeBits,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    ValStatus_t funcres = VAL_INTERNAL_ERROR;
    uint8_t * AssetData_p;
    ValSize_t AssetSize;

#ifdef VAL_STRICT_ARGS
    if ((Data_p == NULL) ||
        (DataSizeBits > VAL_ASYM_DH_DSA_MAX_BITS) ||
        ((DataSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) &&
         ((DataSizeBits < VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS) ||
          (DataSizeBits > VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS))) ||
        (Data_p->Data_p == NULL) ||
        (Data_p->ByteDataSize == 0) ||
        (Data_p->ByteDataSize > VAL_ASYM_DATA_SIZE_B2B(DataSizeBits)) ||
        (TargetAssetId == VAL_ASSETID_INVALID))
    {
        return VAL_BAD_ARGUMENT;
    }

    if ((KekAssetId != VAL_ASSETID_INVALID) &&
        ((AssociatedData_p == NULL) ||
         (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
         (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
         (KeyBlobSize_p == NULL)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Determine the Asset size
    AssetSize = VAL_ASYM_DATA_SIZE_VWB(DataSizeBits);
    if ((KeyBlobSize_p != NULL) &&
        ((KeyBlob_p == NULL) || (*KeyBlobSize_p < VAL_KEYBLOB_SIZE(AssetSize))))
    {
        *KeyBlobSize_p = VAL_KEYBLOB_SIZE(AssetSize);
        return VAL_BUFFER_TOO_SMALL;
    }

    // Allocate buffer for the Asset data
    AssetData_p = Adapter_Alloc((unsigned int)AssetSize);
    if (AssetData_p == NULL)
    {
        return VAL_NO_MEMORY;
    }

    // Create HW DSA Private/public Key representation for plaintext asset load
    valInternal_AsymBigIntToHW(Data_p, DataSizeBits, 0, 2, AssetData_p);

    // Initialize the asset
    if (KekAssetId == VAL_ASSETID_INVALID)
    {
        funcres = val_AssetLoadPlaintext(TargetAssetId, AssetData_p, AssetSize);
    }
    else
    {
        funcres = val_AssetLoadPlaintextExport(TargetAssetId,
                                               AssetData_p, AssetSize,
                                               KekAssetId,
                                               AssociatedData_p, AssociatedDataSize,
                                               KeyBlob_p, KeyBlobSize_p);
    }

    // Clean-up
    Adapter_Free(AssetData_p);

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDhDsaKeyCheck
 */
#if !defined(VAL_REMOVE_ASYM_DH_KEY_CHECK) || \
    !defined(VAL_REMOVE_ASYM_DSA_KEY_CHECK)
ValStatus_t
val_AsymDhDsaKeyCheck(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p)
{
#ifdef VAL_STRICT_ARGS
    if (((PubKey_p == NULL) && (PrivKey_p == NULL)) ||
        ((PubKey_p != NULL) &&
         ((PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
          (PubKey_p->ModulusSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
          (PubKey_p->ModulusSizeBits > VAL_ASYM_DH_DSA_MAX_BITS))) ||
        ((PrivKey_p != NULL) &&
         ((PrivKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
          (PrivKey_p->ModulusSizeBits < VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS) ||
          (PrivKey_p->ModulusSizeBits > VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS))))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    return valInternal_AsymKeyCheck(PubKey_p, PrivKey_p, false);
}
#endif

/* end of file adapter_val_asym_dhdsa.c */
