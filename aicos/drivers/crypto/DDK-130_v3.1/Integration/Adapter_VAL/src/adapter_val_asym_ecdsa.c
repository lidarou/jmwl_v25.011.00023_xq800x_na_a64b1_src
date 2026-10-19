/* adapter_val_asym_ecdsa.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto ECDSA related services.
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
 * val_AsymEcdsaAllocPrivateKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_ECDSA_SIGN
ValStatus_t
val_AsymEcdsaAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSymAlgo_t HashAlgorithm,
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
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_SIGNVERIFY | EIP130_ASSET_POLICY_ACA_ECDSA | EIP130_ASSET_POLICY_PRIVATEDATA;
#else
    AssetPolicy = VAL_POLICY_PK_ECC_ECDSA_SIGN;
#endif
    switch (HashAlgorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 160)
        if (ModulusSizeBits < 160)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA1;
#else
        AssetPolicy |= VAL_POLICY_SHA1;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA224:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 224)
        if (ModulusSizeBits < 224)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA224;
#else
        AssetPolicy |= VAL_POLICY_SHA224;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA256:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 256)
        if (ModulusSizeBits < 256)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA256;
#else
        AssetPolicy |= VAL_POLICY_SHA256;
#endif
        break;
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 384)
        if (ModulusSizeBits < 384)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA384;
#else
        AssetPolicy |= VAL_POLICY_SHA384;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA512:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 512)
        if (ModulusSizeBits < 512)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA512;
#else
        AssetPolicy |= VAL_POLICY_SHA512;
#endif
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }
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
#endif /* !VAL_REMOVE_ASYM_ECDSA_SIGN */


/*----------------------------------------------------------------------------
 * val_AsymEcdsaAllocPublicKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_ECDSA_VERIFY
ValStatus_t
val_AsymEcdsaAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSymAlgo_t HashAlgorithm,
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
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_SIGNVERIFY | EIP130_ASSET_POLICY_ACA_ECDSA;
#else
    AssetPolicy = VAL_POLICY_PUBLIC_KEY | VAL_POLICY_PK_ECC_ECDSA_SIGN;
#endif
    switch (HashAlgorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
#if  defined(VAL_STRICT_ARGS) && (VAL_ASYM_ECP_MIN_BITS < 160)
        if (ModulusSizeBits < 160)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA1;
#else
        AssetPolicy |= VAL_POLICY_SHA1;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA224:
#ifdef VAL_STRICT_ARGS
        if (ModulusSizeBits < 224)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA224;
#else
        AssetPolicy |= VAL_POLICY_SHA224;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA256:
#ifdef VAL_STRICT_ARGS
        if (ModulusSizeBits < 256)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA256;
#else
        AssetPolicy |= VAL_POLICY_SHA256;
#endif
        break;
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
#ifdef VAL_STRICT_ARGS
        if (ModulusSizeBits < 384)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA384;
#else
        AssetPolicy |= VAL_POLICY_SHA384;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA512:
#ifdef VAL_STRICT_ARGS
        if (ModulusSizeBits < 512)
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA512;
#else
        AssetPolicy |= VAL_POLICY_SHA512;
#endif
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }
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
#endif /* !VAL_REMOVE_ASYM_ECDSA_VERIFY */


/*----------------------------------------------------------------------------
 * val_AsymEcdsaSign
 */
#ifndef VAL_REMOVE_ASYM_ECDSA_SIGN
ValStatus_t
val_AsymEcdsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest)
{
    ValStatus_t funcres = VAL_SUCCESS;
    ValOctetsIn_t * HashData_p = (ValOctetsIn_t *)HashMessage_p;
    ValSize_t HashDataSize = HashMessageSize;
    ValSymContext_t * SymContext_p = (ValSymContext_t *)HashContext_p;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (HashMessage_p == NULL) ||
        (HashMessageSize == 0) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->r.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        (Signature_p->s.Data_p == NULL) ||
        (Signature_p->s.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)))
    {
        return VAL_BAD_ARGUMENT;
    }

    if (!fExplicitDigest)
    {
        switch (KeyContext_p->HashAlgorithm)
        {
        case VAL_SYM_ALGO_HASH_SHA1:
#if (VAL_ASYM_ECP_MIN_BITS < 160)
            if (KeyContext_p->ModulusSizeBits < 160)
            {
                return VAL_BAD_ARGUMENT;
            }
#endif
            break;
        case VAL_SYM_ALGO_HASH_SHA224:
            if (KeyContext_p->ModulusSizeBits < 224)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
        case VAL_SYM_ALGO_HASH_SHA256:
            if (KeyContext_p->ModulusSizeBits < 256)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
        case VAL_SYM_ALGO_HASH_SHA384:
            if (KeyContext_p->ModulusSizeBits < 384)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
        case VAL_SYM_ALGO_HASH_SHA512:
            if (KeyContext_p->ModulusSizeBits < 512)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
#endif
        default:
            return VAL_BAD_ARGUMENT;
        }
    }
    else
    {
        // Check whether given explicit digest has a valid size
        switch (HashMessageSize)
        {
        case (160/8):
        case (224/8):
        case (256/8):
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
        case (384/8):
        case (512/8):
#endif
            break;
        default:
            return VAL_BAD_ARGUMENT;
        }
    }
#endif

    if (!fExplicitDigest && (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE))
    {
        // Perform a partial hash first, so that remaining size is less than
        // the allowed maximum for the ECDSA sign operation
        if (SymContext_p == NULL)
        {
            // Allocate hash context
            funcres = val_SymAlloc(KeyContext_p->HashAlgorithm,
                                   VAL_SYM_MODE_NONE, false,
                                   (ValSymContextPtr_t *)&SymContext_p);
            if (funcres != VAL_SUCCESS)
            {
                return funcres;
            }
        }

        while (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE)
        {
            // Hash a block of data
            ValSize_t Size;

            if (HashDataSize > VAL_SIZE_MAX_DMA)
            {
                Size = VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
            }
            else
            {
                Size = HashDataSize - (2 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
            }
            Size &= (ValSize_t)~(VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE -1);

            funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                        (ValOctetsIn_t *)HashData_p,
                                        Size);
            if (funcres != VAL_SUCCESS)
            {
                goto error_func_exit;
            }

            // Adjust references
            HashData_p   += Size;
            HashDataSize -= Size;
        }
    }

    {
        VexToken_Command_t t_cmd;
        VexToken_Result_t t_res;
        uint8_t * SigData_p;
        uint32_t SignatureSize = (uint32_t)(2 * VAL_ASYM_DATA_SIZE_VWB(KeyContext_p->ModulusSizeBits));
        ValAssetId_t TempAssetId = VAL_ASSETID_INVALID;

        // Allocate a temporary buffer for the signature
        SigData_p = Adapter_Alloc(SignatureSize);
        if (SigData_p == NULL)
        {
            funcres = VAL_NO_MEMORY;
            goto error_func_exit;
        }

        // Format service request
        t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
        t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
        t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_ECDSA_SIGN;
        t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)KeyContext_p->ModulusSizeBits;
        t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
        t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
        t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
        t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
        t_cmd.Service.PkAssetSignVerify.SignSize = SignatureSize;

        if (fExplicitDigest)
        {
            t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = (uint32_t)HashMessageSize;
            if (HashMessageSize > sizeof(t_cmd.Service.PkAssetSignVerify.ExplicitDigest))
            {
                return VAL_BAD_ARGUMENT;
            }
            memcpy(t_cmd.Service.PkAssetSignVerify.ExplicitDigest,
                   (const uint8_t *)HashData_p, HashMessageSize);

            t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
            t_cmd.Service.PkAssetSignVerify.HashData_p = NULL;
            t_cmd.Service.PkAssetSignVerify.HashDataSize = 0;
            t_cmd.Service.PkAssetSignVerify.TotalMessageSize = 0;
        }
        else
        {
            t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = 0;

            if (SymContext_p == NULL)
            {
                t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
                t_cmd.Service.PkAssetSignVerify.TotalMessageSize = HashDataSize;
            }
            else
            {
                if (SymContext_p->Service.Hash.TempAssetId != VAL_ASSETID_INVALID)
                {
                    t_cmd.Service.PkAssetSignVerify.DigestAssetId =
                        SymContext_p->Service.Hash.TempAssetId;
                }
                else
                {
                    funcres = valInternal_SymCreateTempAsset(SymContext_p, &TempAssetId);
                    if (funcres != VAL_SUCCESS)
                    {
                        goto error_func_exit;
                    }
                    t_cmd.Service.PkAssetSignVerify.DigestAssetId = TempAssetId;
                }
                t_cmd.Service.PkAssetSignVerify.TotalMessageSize =
                    SymContext_p->Service.Hash.TotalMessageLength + (uint64_t)HashDataSize;
            }
            t_cmd.Service.PkAssetSignVerify.HashData_p = (const uint8_t *)HashData_p;
            t_cmd.Service.PkAssetSignVerify.HashDataSize = (uint32_t)HashDataSize;
        }
        t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

        // Exchange service request with the next driver level
        funcres = val_ExchangeToken(&t_cmd, &t_res);
        if (funcres == VAL_SUCCESS)
        {
            // Check for errors
            if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
            {
                // Convert the signature from HW to application format
                valInternal_AsymDsaSignatureFromHW(SigData_p,
                                                   KeyContext_p->ModulusSizeBits,
                                                   Signature_p);
            }
            else
            {
                funcres = (ValStatus_t)t_res.Result;
                LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
            }
        }

        if (TempAssetId != VAL_ASSETID_INVALID)
        {
            (void)val_AssetFree(TempAssetId);
        }
        Adapter_Free(SigData_p);
    }

    // Clean-up
error_func_exit:
    if (((funcres == VAL_SUCCESS) || (HashContext_p == NULL)) &&
        (SymContext_p != NULL))
    {
        (void)val_SymRelease((ValSymContextPtr_t const)SymContext_p);
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASYM_ECDSA_SIGN */


/*----------------------------------------------------------------------------
 * val_AsymEcdsaVerify
 */
#ifndef VAL_REMOVE_ASYM_ECDSA_VERIFY
ValStatus_t
val_AsymEcdsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest)
{
    ValStatus_t funcres = VAL_SUCCESS;
    ValOctetsIn_t * HashData_p = (ValOctetsIn_t *)HashMessage_p;
    ValSize_t HashDataSize = HashMessageSize;
    ValSymContext_t * SymContext_p = (ValSymContext_t *)HashContext_p;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (HashMessage_p == NULL) ||
        (HashMessageSize == 0) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->r.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        (Signature_p->s.Data_p == NULL) ||
        (Signature_p->s.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)))
    {
        return VAL_BAD_ARGUMENT;
    }
    if (!fExplicitDigest)
    {
        switch (KeyContext_p->HashAlgorithm)
        {
        case VAL_SYM_ALGO_HASH_SHA1:
#if (VAL_ASYM_ECP_MIN_BITS < 160)
            if (KeyContext_p->ModulusSizeBits < 160)
            {
                return VAL_BAD_ARGUMENT;
            }
#endif
            break;
        case VAL_SYM_ALGO_HASH_SHA224:
            if (KeyContext_p->ModulusSizeBits < 224)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
        case VAL_SYM_ALGO_HASH_SHA256:
            if (KeyContext_p->ModulusSizeBits < 256)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
        case VAL_SYM_ALGO_HASH_SHA384:
            if (KeyContext_p->ModulusSizeBits < 384)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
        case VAL_SYM_ALGO_HASH_SHA512:
            if (KeyContext_p->ModulusSizeBits < 512)
            {
                return VAL_BAD_ARGUMENT;
            }
            break;
    #endif
        default:
            return VAL_BAD_ARGUMENT;
        }
    }
    else
    {
        // Check whether given explicit digest has a valid size
        switch (HashMessageSize)
        {
        case (160/8):
        case (224/8):
        case (256/8):
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
        case (384/8):
        case (512/8):
#endif
            break;
        default:
            return VAL_BAD_ARGUMENT;
        }
    }
#endif

    if (!fExplicitDigest && (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE))
    {
        // Perform a partial hash first, so that remaining size is less than
        // the allowed maximum for the ECDSA verify operation
        if (SymContext_p == NULL)
        {
            // Allocate hash context
            funcres = val_SymAlloc(KeyContext_p->HashAlgorithm,
                                   VAL_SYM_MODE_NONE, false,
                                   (ValSymContextPtr_t *)&SymContext_p);
            if (funcres != VAL_SUCCESS)
            {
                return funcres;
            }
        }

        while (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE)
        {
            // Hash a block of data
            ValSize_t Size;

            if (HashDataSize > VAL_SIZE_MAX_DMA)
            {
                Size = VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
            }
            else
            {
                Size = HashDataSize - (2 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
            }
            Size &= (ValSize_t)~(VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE -1);

            funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                        (ValOctetsIn_t *)HashData_p,
                                        Size);
            if (funcres != VAL_SUCCESS)
            {
                goto error_func_exit;
            }

            // Adjust references
            HashData_p   += Size;
            HashDataSize -= Size;
        }
    }

    {
        VexToken_Command_t t_cmd;
        VexToken_Result_t t_res;
        uint8_t * SigData_p;
        uint32_t SignatureSize = (uint32_t)(2 * VAL_ASYM_DATA_SIZE_VWB(KeyContext_p->ModulusSizeBits));
        ValAssetId_t TempAssetId = VAL_ASSETID_INVALID;

        // Allocate a temporary buffer for the signature
        SigData_p = Adapter_Alloc(SignatureSize);
        if (SigData_p == NULL)
        {
            funcres = VAL_NO_MEMORY;
            goto error_func_exit;
        }

        // Convert the signature to from application to HW format
        valInternal_AsymDsaSignatureToHW(Signature_p,
                                         KeyContext_p->ModulusSizeBits,
                                         SigData_p);

        // Format service request
        t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
        t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
        t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_ECDSA_VERIFY;
        t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)KeyContext_p->ModulusSizeBits;
        t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
        t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
        t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
        t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
        t_cmd.Service.PkAssetSignVerify.SignSize = SignatureSize;

        if (fExplicitDigest)
        {
            t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = (uint32_t)HashMessageSize;
            if (HashMessageSize > sizeof(t_cmd.Service.PkAssetSignVerify.ExplicitDigest))
            {
                return VAL_BAD_ARGUMENT;
            }
            memcpy(t_cmd.Service.PkAssetSignVerify.ExplicitDigest,
                   (const uint8_t *)HashData_p, HashMessageSize);

            t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
            t_cmd.Service.PkAssetSignVerify.HashData_p = NULL;
            t_cmd.Service.PkAssetSignVerify.HashDataSize = 0;
            t_cmd.Service.PkAssetSignVerify.TotalMessageSize = 0;
        }
        else
        {
            t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = 0;

            if (SymContext_p == NULL)
            {
                t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
                t_cmd.Service.PkAssetSignVerify.TotalMessageSize = HashDataSize;
            }
            else
            {
                if (SymContext_p->Service.Hash.TempAssetId != VAL_ASSETID_INVALID)
                {
                    t_cmd.Service.PkAssetSignVerify.DigestAssetId =
                        SymContext_p->Service.Hash.TempAssetId;
                }
                else
                {
                    funcres = valInternal_SymCreateTempAsset(SymContext_p, &TempAssetId);
                    if (funcres != VAL_SUCCESS)
                    {
                        goto error_func_exit;
                    }
                    t_cmd.Service.PkAssetSignVerify.DigestAssetId = TempAssetId;
                }
                t_cmd.Service.PkAssetSignVerify.TotalMessageSize =
                    SymContext_p->Service.Hash.TotalMessageLength + (uint64_t)HashDataSize;
            }
            t_cmd.Service.PkAssetSignVerify.HashData_p = (const uint8_t *)HashData_p;
            t_cmd.Service.PkAssetSignVerify.HashDataSize = (uint32_t)HashDataSize;
        }
        t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

        // Exchange service request with the next driver level
        funcres = val_ExchangeToken(&t_cmd, &t_res);
        if ((funcres == VAL_SUCCESS) &&
            (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
        {
            // Check for errors
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }

        if (TempAssetId != VAL_ASSETID_INVALID)
        {
            (void)val_AssetFree(TempAssetId);
        }
        Adapter_Free(SigData_p);
    }

    // Clean-up
error_func_exit:
    if (((funcres == VAL_SUCCESS) || (HashContext_p == NULL)) &&
        (SymContext_p != NULL))
    {
        (void)val_SymRelease((ValSymContextPtr_t const)SymContext_p);
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASYM_ECDSA_VERIFY */


/* end of file adapter_val_asym_ecdsa.c */
