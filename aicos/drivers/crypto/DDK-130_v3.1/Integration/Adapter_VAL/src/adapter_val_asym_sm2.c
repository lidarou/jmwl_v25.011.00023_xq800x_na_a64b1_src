/* adapter_val_asym_sm2.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the asymmetric crypto SM2DSA related services.
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
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken(), valInternal_*()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


#if !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET) || \
    !defined(VAL_REMOVE_ASYM_SM2ENC)
static inline ValStatus_t
val_AsymSm2AllocKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p,
        const bool fIsPublic,
        const bool fIsDsa,
        const bool fIsKex,
        const bool fIsWrap,
        const bool fAllowAny)
{
    ValPolicyMask_t AssetPolicy;
    ValStatus_t AssetSize = VAL_ASYM_DATA_SIZE_VWB(ModulusSizeBits);

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (!fIsDsa && !fIsKex && !fIsWrap))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    /* Determine Asset policy */
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_ASYM_BASE | EIP130_ASSET_POLICY_ACA_SM2 | EIP130_ASSET_POLICY_ACH_SM3;
#else
    AssetPolicy = VAL_POLICY_SM3;
    IDENTIFIER_NOT_USED(fAllowAny);
#endif
    if (fIsPublic)
    {
#ifndef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= VAL_POLICY_PUBLIC_KEY;
#endif
        AssetSize *= 2;  /* Double for public key */
    }
#ifdef EIP130_FW_ASSETPOLICY_V2
    else
    {
        AssetPolicy |= EIP130_ASSET_POLICY_PRIVATEDATA;
        if(fAllowAny)
        {
            AssetPolicy |= EIP130_ASSET_POLICY_STANY;
        }
    }
#endif
    if (fIsDsa)
    {
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACUI_SIGNVRFY;
#else
        AssetPolicy |= VAL_POLICY_PK_SM2_SIGN;
#endif
    }
    if (fIsKex)
    {
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACUI_KEYEXCH;
#else
        AssetPolicy |= VAL_POLICY_PK_SM2_KEY;
#endif
    }
    if (fIsWrap)
    {
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_ACUI_DECENC;
#else
        AssetPolicy |= VAL_POLICY_PK_SM2_WRAP;
#endif
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

    /* Determine the Asset size and allocate the Asset */
    return val_AssetAlloc(AssetPolicy, AssetSize, false, false,
                          VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                          AssetId_p);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymSm2dsaAllocPrivateKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_SM2DSA_SIGN
ValStatus_t
val_AsymSm2dsaAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    false, true, false, false, false);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymSm2dsaAllocPublicKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_SM2DSA_VERIFY
ValStatus_t
val_AsymSm2dsaAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    true, true, false, false, false);
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymSm2dsaCalculateZA
 */
#if !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET)
ValStatus_t
val_AsymSm2dsaCalculateZA(
        ValOctetsIn_t * const UserIdentity_p,
        const uint32_t UserIdentitySize,
        const ValAsymECDomainParam_t * const Domain_p,
        const ValAsymECCPoint_t * const PublicKey_p,
        ValOctetsOut_t * const ZA_p,
        ValSize_t * const ZA_Size_p)
{
    uint8_t * TempBuffer_p;
    uint32_t DataSize, i;
    ValStatus_t funcres = VAL_SUCCESS;
    ValSymContextPtr_t SymContext_p = NULL;

#ifdef VAL_STRICT_ARGS
    if (((UserIdentity_p == NULL) && (UserIdentitySize > 0)) ||
        (Domain_p == NULL) ||
        (PublicKey_p == NULL) ||
        (ZA_p == NULL) ||
        (ZA_Size_p == NULL) ||
        (*ZA_Size_p == 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    DataSize = 2 + UserIdentitySize +
               Domain_p->a.ByteDataSize +
               Domain_p->b.ByteDataSize +
               Domain_p->BasePoint.x.ByteDataSize +
               Domain_p->BasePoint.y.ByteDataSize +
               PublicKey_p->x.ByteDataSize +
               PublicKey_p->y.ByteDataSize;

    // Allocate temp buffer
    TempBuffer_p = Adapter_Alloc(DataSize);
    if (TempBuffer_p == NULL)
    {
        return VAL_NO_MEMORY;
    }

    // First two bytes: UserIdentity length in bits (MSB first)
    TempBuffer_p[0] = (uint8_t)((8 * UserIdentitySize) >> 8);
    TempBuffer_p[1] = (uint8_t)(8 * UserIdentitySize);
    i = 2;
    // UserIdentity
    memcpy(&TempBuffer_p[i], UserIdentity_p, UserIdentitySize);
    i += UserIdentitySize;
    // Curve constant a
    memcpy(&TempBuffer_p[i], Domain_p->a.Data_p, Domain_p->a.ByteDataSize);
    i += Domain_p->a.ByteDataSize;
    // Curve constant b
    memcpy(&TempBuffer_p[i], Domain_p->b.Data_p, Domain_p->b.ByteDataSize);
    i += Domain_p->b.ByteDataSize;
    // Curve base point: Gx
    memcpy(&TempBuffer_p[i], Domain_p->BasePoint.x.Data_p, Domain_p->BasePoint.x.ByteDataSize);
    i += Domain_p->BasePoint.x.ByteDataSize;
    // Curve base point: Gy
    memcpy(&TempBuffer_p[i], Domain_p->BasePoint.y.Data_p, Domain_p->BasePoint.y.ByteDataSize);
    i += Domain_p->BasePoint.y.ByteDataSize;
    // Public key: Qx
    memcpy(&TempBuffer_p[i], PublicKey_p->x.Data_p, PublicKey_p->x.ByteDataSize);
    i += PublicKey_p->x.ByteDataSize;
    // Public key: Qy
    memcpy(&TempBuffer_p[i], PublicKey_p->y.Data_p, PublicKey_p->y.ByteDataSize);
    i += PublicKey_p->y.ByteDataSize;
    if (i != DataSize)
    {
        funcres = VAL_INTERNAL_ERROR;
        goto error_func_exit;
    }

    // Allocate hash context
    funcres = val_SymAlloc(VAL_SYM_ALGO_HASH_SM3, VAL_SYM_MODE_NONE, false, &SymContext_p);
    if (funcres != VAL_SUCCESS)
    {
        goto error_func_exit;
    }

    // Calculate final value (ZA)
    funcres = val_SymHashFinal(SymContext_p, TempBuffer_p, DataSize, ZA_p, ZA_Size_p);
    if (funcres != VAL_SUCCESS)
    {
        (void)val_SymRelease(SymContext_p);
    }

error_func_exit:
    if (TempBuffer_p != NULL)
    {
        Adapter_Free(TempBuffer_p);
    }

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_AsymSm2dsaSign
 */
#ifndef VAL_REMOVE_ASYM_SM2DSA_SIGN
ValStatus_t
val_AsymSm2dsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const ZA_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValOctetsIn_t * HashData_p = (ValOctetsIn_t *)HashMessage_p;
    ValSymContext_t * SymContext_p = (ValSymContext_t *)HashContext_p;
    uint8_t * MessageData_p = NULL;
    uint8_t * SigData_p = NULL;
    ValSize_t HashDataSize = 0;
    ValSize_t ModulusSize;
    ValSize_t SignatureSize;
    ValStatus_t funcres = VAL_SUCCESS;
    ValAssetId_t TempAssetId = VAL_ASSETID_INVALID;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->ModulusSizeBits < 256) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (HashMessage_p == NULL) ||
        (HashMessageSize == 0) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->r.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        (Signature_p->s.Data_p == NULL) ||
        (Signature_p->s.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        ((ZA_p != NULL) && (HashContext_p != NULL) && !fExplicitDigest) ||
        ((ZA_p == NULL) && (HashContext_p == NULL) && (HashMessageSize != 32) && !fExplicitDigest))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif /* VAL_STRICT_ARGS */

    ModulusSize = KeyContext_p->ModulusSizeBits;

    if (!fExplicitDigest)
    {
        // Allocate a buffer to prevent possible memory alignment issues
        if (HashMessageSize > (VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE)))
        {
            HashDataSize = (VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE));
        }
        else if (HashMessageSize > VAL_SYM_ALGO_HASH_BLOCK_SIZE)
        {
            HashDataSize = HashMessageSize;
        }
        else
        {
            HashDataSize = VAL_SYM_ALGO_HASH_BLOCK_SIZE;
        }
        MessageData_p = Adapter_Alloc((unsigned int)HashDataSize);
        if (MessageData_p == NULL)
        {
            return VAL_NO_MEMORY;
        }

        HashDataSize = HashMessageSize;

        /* 1. If HashContext_p is NULL and ZA_p is also NULL:
            Hash 'e' is precalculated and final hash value is provided through
            HashMessage_p (HashMessageSize must be 32).
        2. If HashContext_p is NULL and ZA_p is NOT NULL:
            Nothing is precalculated, use ZA_p and HashMessage_p fields to
            calculate hash value first before 'sign'/'verify'.
        3. If HashContext_p is NOT NULL and ZA_p is NULL:
            Intermediate Hash value is provided through HashContext_p,
            but HashMessage_p contains last block of data for HashFinal
            operation. */
        if ((HashContext_p == NULL) && (ZA_p != NULL))
        {
#define SM2_ZA_SIZE 32
            // Copy ZA_p array, thus total message size gets 32B bigger
            memcpy(MessageData_p, ZA_p, SM2_ZA_SIZE);
            HashDataSize += SM2_ZA_SIZE;

            if (HashDataSize > VAL_SYM_ALGO_HASH_BLOCK_SIZE)
            {
                // Allocate hash context
                funcres = val_SymAlloc(VAL_SYM_ALGO_HASH_SM3,
                                       VAL_SYM_MODE_NONE, false,
                                       (ValSymContextPtr_t *)&SymContext_p);
                if (funcres != VAL_SUCCESS)
                {
                    goto error_func_exit;
                }

                // Copy first part of HashMessage into temp buffer
                memcpy(&MessageData_p[SM2_ZA_SIZE], HashData_p,
                       (VAL_SYM_ALGO_HASH_BLOCK_SIZE - SM2_ZA_SIZE));

                // Do HashUpdate operation for the first block
                funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                            MessageData_p, VAL_SYM_ALGO_HASH_BLOCK_SIZE);
                if (funcres != VAL_SUCCESS)
                {
                    goto error_func_exit;
                }

                HashData_p   += (VAL_SYM_ALGO_HASH_BLOCK_SIZE - SM2_ZA_SIZE);
                HashDataSize -= VAL_SYM_ALGO_HASH_BLOCK_SIZE;

                // Perform a partial hash operation as long as the remaining size
                // is bigger than allowed maximum for the SM2DSA sign operation
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

                    // Prevent possible memory alignment issue
                    memcpy(MessageData_p, HashData_p, Size);

                    funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                                MessageData_p, Size);
                    if (funcres != VAL_SUCCESS)
                    {
                        goto error_func_exit;
                    }

                    // Adjust references
                    HashData_p   += Size;
                    HashDataSize -= Size;
                }

                // Prevent possible memory alignment issue
                memcpy(MessageData_p, HashData_p, HashDataSize);
            }
            else
            {
                // Total HashDataSize is less than one Hash block,
                // just copy rest of the message in temp buffer which
                // can be used in SM2DSA operation
                memcpy(&MessageData_p[SM2_ZA_SIZE], HashData_p,
                       (HashDataSize - SM2_ZA_SIZE));
            }
#undef SM2_ZA_SIZE
        }
        else
        {
            // Prevent possible memory alignment issue
            memcpy(MessageData_p, HashData_p, HashDataSize);
        }
    }

    // Allocate a temporary buffer for the signature
    SignatureSize = (2 * VAL_ASYM_DATA_SIZE_VWB(ModulusSize));
    SigData_p = Adapter_Alloc((unsigned int)SignatureSize);
    if (SigData_p == NULL)
    {
        funcres = VAL_NO_MEMORY;
        goto error_func_exit;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_SM2DSA_SIGN;
    t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)ModulusSize;
    t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
    t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
    t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
    t_cmd.Service.PkAssetSignVerify.SignSize = (uint32_t)SignatureSize;
    t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
    if (fExplicitDigest)
    {
        if (HashMessageSize > sizeof(t_cmd.Service.PkAssetSignVerify.ExplicitDigest))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.PkAssetSignVerify.ExplicitDigest,
               (const uint8_t *)HashData_p, HashMessageSize);
        t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = (uint32_t)HashMessageSize;
        t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
        t_cmd.Service.PkAssetSignVerify.HashData_p = NULL;
        t_cmd.Service.PkAssetSignVerify.HashDataSize = 0;
        t_cmd.Service.PkAssetSignVerify.TotalMessageSize = 0;
    }
    else
    {
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
        t_cmd.Service.PkAssetSignVerify.HashData_p = MessageData_p;
        t_cmd.Service.PkAssetSignVerify.HashDataSize = (uint32_t)HashDataSize;
        t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = 0;
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
            valInternal_AsymDsaSignatureFromHW(SigData_p, ModulusSize, Signature_p);
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    // Clean-up
error_func_exit:
    if (TempAssetId != VAL_ASSETID_INVALID)
    {
        (void)val_AssetFree(TempAssetId);
    }

    if (((funcres == VAL_SUCCESS) || (HashContext_p == NULL)) &&
        (SymContext_p != NULL))
    {
        (void)val_SymRelease((ValSymContextPtr_t const)SymContext_p);
    }

    if (SigData_p != NULL)
    {
        Adapter_Free(SigData_p);
    }
    if (MessageData_p != NULL)
    {
        Adapter_Free(MessageData_p);
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASYM_SM2DSA_SIGN */


/*----------------------------------------------------------------------------
 * val_AsymSm2dsaVerify
 */
#ifndef VAL_REMOVE_ASYM_SM2DSA_VERIFY
ValStatus_t
val_AsymSm2dsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const ZA_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValSymContext_t * SymContext_p = (ValSymContext_t *)HashContext_p;
    ValOctetsIn_t * HashData_p = (ValOctetsIn_t *)HashMessage_p;
    uint8_t * MessageData_p = NULL;
    uint8_t * SigData_p = NULL;
    ValSize_t HashDataSize = 0;
    ValSize_t ModulusSize;
    ValSize_t SignatureSize;
    ValStatus_t funcres = VAL_SUCCESS;
    ValAssetId_t TempAssetId = VAL_ASSETID_INVALID;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (HashMessage_p == NULL) ||
        (HashMessageSize == 0) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->ModulusSizeBits < 256) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->r.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        (Signature_p->s.Data_p == NULL) ||
        (Signature_p->s.ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(KeyContext_p->ModulusSizeBits)) ||
        ((ZA_p != NULL) && (HashContext_p != NULL) && !fExplicitDigest) ||
        ((ZA_p == NULL) && (HashContext_p == NULL) && (HashMessageSize != 32) && !fExplicitDigest))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif /* VAL_STRICT_ARGS */

    ModulusSize = KeyContext_p->ModulusSizeBits;

    if (!fExplicitDigest)
    {
        // Allocate a buffer to prevent possible memory alignment issues
        if (HashMessageSize > (VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE)))
        {
            HashDataSize = (VAL_SIZE_MAX_DMA - (4 * VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE));
        }
        else if (HashMessageSize > VAL_SYM_ALGO_HASH_BLOCK_SIZE)
        {
            HashDataSize = HashMessageSize;
        }
        else
        {
            HashDataSize = VAL_SYM_ALGO_HASH_BLOCK_SIZE;
        }
        MessageData_p = Adapter_Alloc((unsigned int)HashDataSize);
        if (MessageData_p == NULL)
        {
            return VAL_NO_MEMORY;
        }

        HashDataSize = HashMessageSize;

        /* 1. If HashContext_p is NULL and ZA_p is also NULL:
            Hash 'e' is precalculated and final hash value is provided through
            HashMessage_p (HashMessageSize must be 32).
        2. If HashContext_p is NULL and ZA_p is NOT NULL:
            Nothing is precalculated, use ZA_p and HashMessage_p fields to
            calculate hash value first before 'sign'/'verify'.
        3. If HashContext_p is NOT NULL and ZA_p is NULL:
            Intermediate Hash value is provided through HashContext_p,
            but HashMessage_p contains last block of data for HashFinal
            operation. */
        if ((HashContext_p == NULL) && (ZA_p != NULL))
        {
#define SM2_ZA_SIZE 32
            // Copy ZA_p array, thus total message size gets 32B bigger
            memcpy(MessageData_p, ZA_p, SM2_ZA_SIZE);
            HashDataSize += SM2_ZA_SIZE;

            if (HashDataSize > VAL_SYM_ALGO_HASH_BLOCK_SIZE)
            {
                // Allocate hash context
                funcres = val_SymAlloc(VAL_SYM_ALGO_HASH_SM3,
                                       VAL_SYM_MODE_NONE, false,
                                       (ValSymContextPtr_t *)&SymContext_p);
                if (funcres != VAL_SUCCESS)
                {
                    goto error_func_exit;
                }

                // Copy first part of HashMessage into temp buffer
                memcpy(&MessageData_p[SM2_ZA_SIZE], HashData_p,
                       (VAL_SYM_ALGO_HASH_BLOCK_SIZE - SM2_ZA_SIZE));

                // Do HashUpdate operation for the first block
                funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                            MessageData_p, VAL_SYM_ALGO_HASH_BLOCK_SIZE);
                if (funcres != VAL_SUCCESS)
                {
                    goto error_func_exit;
                }

                HashData_p   += (VAL_SYM_ALGO_HASH_BLOCK_SIZE - SM2_ZA_SIZE);
                HashDataSize -= VAL_SYM_ALGO_HASH_BLOCK_SIZE;

                // Perform a partial hash operation as long as the remaining size
                // is bigger than allowed maximum for the SM2DSA sign operation
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

                    // Prevent possible memory alignment issue
                    memcpy(MessageData_p, HashData_p, Size);

                    funcres = val_SymHashUpdate((ValSymContextPtr_t const)SymContext_p,
                                                MessageData_p, Size);
                    if (funcres != VAL_SUCCESS)
                    {
                        goto error_func_exit;
                    }

                    // Adjust references
                    HashData_p   += Size;
                    HashDataSize -= Size;
                }

                // Prevent possible memory alignment issue
                memcpy(MessageData_p, HashData_p, HashDataSize);
            }
            else
            {
                // Total HashDataSize is less than one Hash block,
                // just copy rest of the message in temp buffer which
                // can be used in SM2DSA operation
                memcpy(&MessageData_p[SM2_ZA_SIZE], HashData_p,
                       (HashDataSize - SM2_ZA_SIZE));
            }
#undef SM2_ZA_SIZE
        }
        else
        {
            // Prevent possible memory alignment issue
            memcpy(MessageData_p, HashData_p, HashDataSize);
        }
    }

    // Allocate a temporary buffer for the signature
    SignatureSize = (2 * VAL_ASYM_DATA_SIZE_VWB(ModulusSize));
    SigData_p = Adapter_Alloc((unsigned int)SignatureSize);
    if (SigData_p == NULL)
    {
        funcres = VAL_NO_MEMORY;
        goto error_func_exit;
    }

    // Convert the signature to from application to HW format
    valInternal_AsymDsaSignatureToHW(Signature_p, ModulusSize, SigData_p);

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetSignVerify.Method = VEXTOKEN_PKASSET_SM2DSA_VERIFY;
    t_cmd.Service.PkAssetSignVerify.ModulusSizeInBits = (uint32_t)ModulusSize;
    t_cmd.Service.PkAssetSignVerify.KeyAssetId = KeyContext_p->KeyAssetId;
    t_cmd.Service.PkAssetSignVerify.DomainAssetId = KeyContext_p->DomainAssetId;
    t_cmd.Service.PkAssetSignVerify.SaltSize = 0;
    t_cmd.Service.PkAssetSignVerify.Sign_p = SigData_p;
    t_cmd.Service.PkAssetSignVerify.SignSize = (uint32_t)SignatureSize;
    if (fExplicitDigest)
    {
        if (HashMessageSize > sizeof(t_cmd.Service.PkAssetSignVerify.ExplicitDigest))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.PkAssetSignVerify.ExplicitDigest,
               (const uint8_t *)HashData_p, HashMessageSize);
        t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = (uint32_t)HashMessageSize;
        t_cmd.Service.PkAssetSignVerify.DigestAssetId = VAL_ASSETID_INVALID;
        t_cmd.Service.PkAssetSignVerify.HashData_p = NULL;
        t_cmd.Service.PkAssetSignVerify.HashDataSize = 0;
        t_cmd.Service.PkAssetSignVerify.TotalMessageSize = 0;
    }
    else
    {
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
        t_cmd.Service.PkAssetSignVerify.HashData_p = MessageData_p;
        t_cmd.Service.PkAssetSignVerify.HashDataSize = (uint32_t)HashDataSize;
        t_cmd.Service.PkAssetSignVerify.ExplicitDigestSize = 0;
    }

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) && (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Check for errors
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    // Clean-up
error_func_exit:
    if (TempAssetId != VAL_ASSETID_INVALID)
    {
        (void)val_AssetFree(TempAssetId);
    }

    if (((funcres == VAL_SUCCESS) || (HashContext_p == NULL)) &&
        (SymContext_p != NULL))
    {
        (void)val_SymRelease((ValSymContextPtr_t const)SymContext_p);
    }

    if (SigData_p != NULL)
    {
        Adapter_Free(SigData_p);
    }
    if (MessageData_p != NULL)
    {
        Adapter_Free(MessageData_p);
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASYM_SM2DSA_VERIFY */


#ifndef VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET
/*----------------------------------------------------------------------------
 * val_AsymSm2kexAllocPrivateKeyAsset
 */
ValStatus_t
val_AsymSm2kexAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    false, false, true, false,
                                    fAllowAny);
}


/*----------------------------------------------------------------------------
 * val_AsymSm2kexAllocPublicKeyAsset
 */
ValStatus_t
val_AsymSm2kexAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    true, false, true, false, false);
}


/*----------------------------------------------------------------------------
 * val_AsymSm2kexGenSharedSecret
 */
ValStatus_t
val_AsymSm2kexGenSharedSecret(
        ValAsymKey_t * const Remote_Ephem_PubKey_p,
        ValAsymKey_t * const Local_Ephem_PrivKey_p,
        ValAsymKey_t * const Remote_Static_PubKey_p,
        ValAsymKey_t * const Local_Static_PrivKey_p,
        ValAsymECCPoint_t * const Local_Ephem_PubKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets)
{
    ValStatus_t funcres;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    uint8_t * LocalEphemPubKey = NULL;
    ValSize_t LocalEphemPubKeySize;
    unsigned int i;

#ifdef VAL_STRICT_ARGS
    if ((Remote_Ephem_PubKey_p == NULL) ||
        (Remote_Ephem_PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (Remote_Ephem_PubKey_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (Remote_Ephem_PubKey_p->ModulusSizeBits < VAL_ASYM_ECP_MIN_BITS) ||
        (Remote_Ephem_PubKey_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (Local_Ephem_PrivKey_p == NULL) ||
        (Local_Ephem_PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (Local_Ephem_PrivKey_p->DomainAssetId != Remote_Ephem_PubKey_p->DomainAssetId) ||
        (Local_Ephem_PrivKey_p->ModulusSizeBits != Remote_Ephem_PubKey_p->ModulusSizeBits) ||
        (Remote_Static_PubKey_p == NULL) ||
        (Remote_Static_PubKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (Remote_Static_PubKey_p->DomainAssetId != Remote_Ephem_PubKey_p->DomainAssetId) ||
        (Remote_Static_PubKey_p->ModulusSizeBits != Remote_Ephem_PubKey_p->ModulusSizeBits) ||
        (Local_Static_PrivKey_p == NULL) ||
        (Local_Static_PrivKey_p->KeyAssetId == VAL_ASSETID_INVALID) ||
        (Local_Static_PrivKey_p->DomainAssetId != Remote_Ephem_PubKey_p->DomainAssetId) ||
        (Local_Static_PrivKey_p->ModulusSizeBits != Remote_Ephem_PubKey_p->ModulusSizeBits) ||
        (AssetIdList_p == NULL) || (NumberOfAssets == 0) ||
        (NumberOfAssets > VEXTOKEN_MAX_PK_NUM_ASSETS) ||
        (!fSaveSharedSecret && ((OtherInfo_p == NULL) || (OtherInfoSize == 0)))  ||
        (fSaveSharedSecret && (NumberOfAssets != 1)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Allocate buffer for the local ephemeral public key
    LocalEphemPubKeySize = (2 * VAL_ASYM_DATA_SIZE_VWB(Remote_Ephem_PubKey_p->ModulusSizeBits));
    LocalEphemPubKey = Adapter_Alloc((unsigned int)LocalEphemPubKeySize);
    if (LocalEphemPubKey == NULL)
    {
        return VAL_NO_MEMORY;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    t_cmd.Service.PkAssetGenSharedSecret.Method = VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET;
    t_cmd.Service.PkAssetGenSharedSecret.ModulusSizeInBits =
        (uint32_t)Remote_Ephem_PubKey_p->ModulusSizeBits;
    t_cmd.Service.PkAssetGenSharedSecret.DivisorSizeInBits =
        (uint32_t)Local_Ephem_PrivKey_p->ModulusSizeBits;

    t_cmd.Service.PkAssetGenSharedSecret.DomainAssetId =
        (uint32_t)Remote_Ephem_PubKey_p->DomainAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PubKeyAssetId =
        Remote_Ephem_PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKeyAssetId =
        Local_Ephem_PrivKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PubKey2AssetId =
        (uint32_t)Remote_Static_PubKey_p->KeyAssetId;
    t_cmd.Service.PkAssetGenSharedSecret.PrivKey2AssetId =
        (uint32_t)Local_Static_PrivKey_p->KeyAssetId;

    valInternal_AsymECPointToHW(Local_Ephem_PubKey_p,
                                Remote_Ephem_PubKey_p->ModulusSizeBits,
                                0, 2, LocalEphemPubKey);
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKey = LocalEphemPubKey;
    t_cmd.Service.PkAssetGenSharedSecret.LocalEphPubKeySize = (uint32_t)LocalEphemPubKeySize;

    t_cmd.Service.PkAssetGenSharedSecret.fSaveSharedSecret = fSaveSharedSecret;
    if (fSaveSharedSecret)
    {
        t_cmd.Service.PkAssetGenSharedSecret.OtherInfo_p = NULL;
        t_cmd.Service.PkAssetGenSharedSecret.OtherInfoSize = 0;
    }
    else
    {
        t_cmd.Service.PkAssetGenSharedSecret.OtherInfo_p = OtherInfo_p;
        t_cmd.Service.PkAssetGenSharedSecret.OtherInfoSize = (uint32_t)OtherInfoSize;
    }

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

    // Clean-up
    Adapter_Free(LocalEphemPubKey);

    return funcres;
}
#endif /* !VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET */


/*----------------------------------------------------------------------------
 * val_AsymSm2encAllocPrivateKeyAsset
 */
#ifndef VAL_REMOVE_ASYM_SM2ENC
ValStatus_t
val_AsymSm2encAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    false, false, false, true, fAllowAny);
}


/*----------------------------------------------------------------------------
 * val_AsymSm2encAllocPublicKeyAsset
 */
ValStatus_t
val_AsymSm2encAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p)
{
    return val_AsymSm2AllocKeyAsset(ModulusSizeBits,
                                    fCrossDomain,
                                    fExportAllowed,
                                    AssetId_p,
                                    true, false, false, true, false);
}


/*-----------------------------------------------------------------------------
 * valLocal_Sm2enc
 */
static ValStatus_t
valLocal_Sm2enc(
        ValAsymKey_t * const KeyContext_p,
        ValAssetId_t AssetId,
        uint8_t * const WrappedData_p,
        const ValSize_t WrappedDataSize,
        bool fWrap)
{
    ValStatus_t funcres;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

#ifdef VAL_STRICT_ARGS
    if ((KeyContext_p == NULL) ||
        (WrappedData_p == NULL) ||
        (WrappedDataSize == 0) ||
        (WrappedDataSize > VAL_ASYM_SM2ENC_MAX_BYTES) ||
        (KeyContext_p->ModulusSizeBits < 256) ||
        (KeyContext_p->ModulusSizeBits > VAL_ASYM_ECP_MAX_BITS) ||
        (AssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->DomainAssetId == VAL_ASSETID_INVALID) ||
        (KeyContext_p->KeyAssetId == VAL_ASSETID_INVALID))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    t_cmd.OpCode = VEXTOKEN_OPCODE_PUBLIC_KEY;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PK_WITHASSETS;
    if (fWrap)
    {
        t_cmd.Service.PkAssetWrap.Method = VEXTOKEN_PKASSET_SM2ENC_WRAP;
    }
    else
    {
        t_cmd.Service.PkAssetWrap.Method = VEXTOKEN_PKASSET_SM2ENC_UNWRAP;
    }
    t_cmd.Service.PkAssetWrap.AssetId = AssetId;
    t_cmd.Service.PkAssetWrap.DomainAssetId = KeyContext_p->DomainAssetId;
    t_cmd.Service.PkAssetWrap.KeyAssetId = KeyContext_p->KeyAssetId;
    t_cmd.Service.PkAssetWrap.Data_p = WrappedData_p;
    t_cmd.Service.PkAssetWrap.DataSize = (uint32_t)WrappedDataSize;
    t_cmd.Service.PkAssetWrap.ModulusSizeInBits = (uint32_t)KeyContext_p->ModulusSizeBits;
    t_cmd.Service.PkAssetWrap.AdditionalInputSize = 0;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s(%s)=%d\n", __func__, fWrap ? "Wrap" : "Unwrap",funcres);
    }

    return funcres;
}


/*-----------------------------------------------------------------------------
 * val_AsymSm2EncWrap
 */
ValStatus_t
val_AsymSm2EncWrap(
        ValAsymKey_t * const PubKeyContext_p,
        ValAssetId_t AssetId,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize)
{
    return valLocal_Sm2enc(PubKeyContext_p,
                           AssetId,
                           WrappedData_p,
                           WrappedDataSize,
                           true);
}


/*-----------------------------------------------------------------------------
 * val_AsymSm2EncUnwrap
 */
ValStatus_t
val_AsymSm2EncUnwrap(
        ValAsymKey_t * const PrvKeyContext_p,
        ValAssetId_t AssetId,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize)
{
    union
    {
        ValOctetsIn_t * c_p;
        uint8_t * n_p;
    } ConvWrappedData;
    ConvWrappedData.c_p = WrappedData_p;
    return valLocal_Sm2enc(PrvKeyContext_p,
                           AssetId,
                           ConvWrappedData.n_p,
                           WrappedDataSize,
                           false);
}
#endif /* VAL_REMOVE_ASYM_SM2ENC */

/* end of file adapter_val_asym_sm2.c */
