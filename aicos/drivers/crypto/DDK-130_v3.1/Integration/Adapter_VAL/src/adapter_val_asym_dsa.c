/* adapter_val_asym_dsa.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto DSA related services.
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

#if !defined(VAL_REMOVE_ASYM_DSA_SIGN) || !defined(VAL_REMOVE_ASYM_DSA_VERIFY)
static ValPolicyMask_t
getDSAPolicy(ValSymAlgo_t HashAlgorithm, const bool fCrossDomain,
          const bool fExportAllowed, const bool fPrivate)
{
    ValPolicyMask_t AssetPolicy;

    // Determine Asset policy
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_SIGNVERIFY | EIP130_ASSET_POLICY_ACA_DSA;
#else
    AssetPolicy = VAL_POLICY_PK_DSA_SIGN;
#endif
    switch (HashAlgorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA1;
#else
        AssetPolicy |= VAL_POLICY_SHA1;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA224:
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA224;
#else
        AssetPolicy |= VAL_POLICY_SHA224;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA256:
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA256;
#else
        AssetPolicy |= VAL_POLICY_SHA256;
#endif
        break;
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA384;
#else
        AssetPolicy |= VAL_POLICY_SHA384;
#endif
        break;
    case VAL_SYM_ALGO_HASH_SHA512:
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACH_SHA512;
#else
        AssetPolicy |= VAL_POLICY_SHA512;
#endif
        break;
#endif
    default:
        return 0;
    }
    if (fCrossDomain)
    {
        AssetPolicy |= VAL_POLICY_CROSS_DOMAIN;
    }
    if (fExportAllowed)
    {
        AssetPolicy |= VAL_POLICY_EXPORT;
    }
#ifdef EIP130_FW_ASSETPOLICY_V2
    if(fPrivate)
    {
        AssetPolicy |= EIP130_ASSET_POLICY_PRIVATEDATA;
    }
#else
    if(!fPrivate)
    {
        AssetPolicy |= EIP130_ASSET_POLICY_PUBLIC_KEY;
    }
#endif
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }
    return AssetPolicy;
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymDsaAllocPrivateKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_DSA_SIGN
ValStatus_t
val_AsymDsaAllocPrivateKeyAsset(
        const ValSize_t DivisorSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (DivisorSizeBits < VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS) ||
        (DivisorSizeBits > VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    AssetPolicy = getDSAPolicy(HashAlgorithm, fCrossDomain, fExportAllowed, true);
    if (AssetPolicy == 0)
    {
        return VAL_INVALID_ALGORITHM;
    }

    // Determine the Asset size and allocate the Asset
    return val_AssetAlloc(AssetPolicy,
                          VAL_ASYM_DATA_SIZE_VWB(DivisorSizeBits),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif /* !VAL_REMOVE_ASYM_DSA_SIGN */


/*----------------------------------------------------------------------------
 * val_AsymDsaAllocPublicKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_DSA_VERIFY
ValStatus_t
val_AsymDsaAllocPublicKeyAsset(
        const ValSize_t PrimeSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (PrimeSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
        (PrimeSizeBits > VAL_ASYM_DH_DSA_MAX_BITS))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    AssetPolicy = getDSAPolicy(HashAlgorithm, fCrossDomain, fExportAllowed, false);
    if (AssetPolicy == 0)
    {
        return VAL_INVALID_ALGORITHM;
    }

    // Determine the Asset size and allocate the Asset
    return val_AssetAlloc(AssetPolicy,
                          VAL_ASYM_DATA_SIZE_VWB(PrimeSizeBits),
                          false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif /* !VAL_REMOVE_ASYM_DSA_VERIFY */


/*----------------------------------------------------------------------------
 * val_AsymDsaSign
 */
#ifndef VAL_REMOVE_ASYM_DSA_SIGN
ValStatus_t
val_AsymDsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p)
{
    ValStatus_t funcres = VAL_SUCCESS;
    ValOctetsIn_t * HashData_p = (ValOctetsIn_t *)HashMessage_p;
    ValSize_t HashDataSize = HashMessageSize;
    ValSymContext_t * SymContext_p = (ValSymContext_t *)HashContext_p;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->ModulusSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_DH_DSA_MAX_BITS) ||
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

    switch (KeyContext_p->HashAlgorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
    case VAL_SYM_ALGO_HASH_SHA224:
    case VAL_SYM_ALGO_HASH_SHA256:
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
    case VAL_SYM_ALGO_HASH_SHA512:
#endif
        break;
    default:
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE)
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
        t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_DSA_SIGN;
        t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)KeyContext_p->ModulusSizeBits;
        t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
        t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
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
        t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
        t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
        t_cmd.Service.PkAssetSignVerify.SignSize = SignatureSize;
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
#endif /* !VAL_REMOVE_ASYM_DSA_SIGN */


/*----------------------------------------------------------------------------
 * val_AsymDsaVerify
 */
#ifndef VAL_REMOVE_ASYM_DSA_VERIFY
ValStatus_t
val_AsymDsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p)
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
        (KeyContext_p->ModulusSizeBits < VAL_ASYM_DH_DSA_MIN_BITS) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_DH_DSA_MAX_BITS) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->r.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        (Signature_p->s.Data_p == NULL) ||
        (Signature_p->s.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)))
    {
        return VAL_BAD_ARGUMENT;
    }

    switch (KeyContext_p->HashAlgorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
    case VAL_SYM_ALGO_HASH_SHA224:
    case VAL_SYM_ALGO_HASH_SHA256:
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
    case VAL_SYM_ALGO_HASH_SHA512:
#endif
        break;
    default:
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (HashDataSize > VAL_ASYM_CMD_MAX_HASH_SIZE)
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
        t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_DSA_VERIFY;
        t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)KeyContext_p->ModulusSizeBits;
        t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
        t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
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
        t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
        t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
        t_cmd.Service.PkAssetSignVerify.SignSize = SignatureSize;
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
#endif /* !VAL_REMOVE_ASYM_DSA_VERIFY */


/* end of file adapter_val_asym_dsa.c */
