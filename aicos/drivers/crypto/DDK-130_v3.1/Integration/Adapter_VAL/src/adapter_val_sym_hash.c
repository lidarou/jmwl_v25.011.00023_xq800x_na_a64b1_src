/* adapter_val_sym_hash.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the symmetric crypto hash services.
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

#ifndef VAL_REMOVE_SYM_HASH

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_sym.h"                // the API to implement
#include "api_val_buffers.h"            // VAL_SIZE_MAX_DMA
#include "api_val_asset.h"              // Asset Management related information
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_SymHashUpdate
 */
ValStatus_t
val_SymHashUpdate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0) ||
        (Data_p == NULL) ||
        (DataSize == 0))
    {
        return VAL_BAD_ARGUMENT;
    }

    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
    case VAL_SYM_ALGO_HASH_SHA224:
    case VAL_SYM_ALGO_HASH_SHA256:
#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_HASH_SM3:
#endif
        if (((DataSize & (VAL_SYM_ALGO_HASH_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
    case VAL_SYM_ALGO_HASH_SHA512:
        if (((DataSize & (VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_HASH_SHA3_224:
        if (((DataSize & (VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
    case VAL_SYM_ALGO_HASH_SHA3_256:
        if (((DataSize & (VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
    case VAL_SYM_ALGO_HASH_SHA3_384:
        if (((DataSize & (VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
    case VAL_SYM_ALGO_HASH_SHA3_512:
        if (((DataSize & (VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE - 1)) != 0) ||
            (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE - 1))))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
#endif

    default:
        break;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_HASH;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_NOT_USED;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Set hash algorithm
    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA1;
        break;

    case VAL_SYM_ALGO_HASH_SHA224:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA224;
        break;

    case VAL_SYM_ALGO_HASH_SHA256:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA256;
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA384;
        break;

    case VAL_SYM_ALGO_HASH_SHA512:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA512;
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_HASH_SHA3_224:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_224;
        break;
    case VAL_SYM_ALGO_HASH_SHA3_256:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_256;
        break;
    case VAL_SYM_ALGO_HASH_SHA3_384:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_384;
        break;
    case VAL_SYM_ALGO_HASH_SHA3_512:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_512;
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_HASH_SM3:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SM3;
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }

    // Set hash mode
    if (!SymContext_p->fInitDone)
    {
        t_cmd.Service.Hash.Mode = VEXTOKEN_MODE_HASH_MAC_INIT2CONT;

        if (!SymContext_p->fUseTokenTemp &&
            (SymContext_p->Service.Hash.TempAssetId == VAL_ASSETID_INVALID))
        {
            ValSize_t DigestNBytes = 0;
#ifdef EIP130_FW_ASSETPOLICY_V2
            ValPolicyMask_t AssetPolicy = EIP130_ASSET_POLICY_SYM_TEMP | EIP130_ASSET_POLICY_SCUIHASH | EIP130_ASSET_POLICY_SCDIRENCGEN;
#else
            ValPolicyMask_t AssetPolicy = VAL_POLICY_TEMP_MAC;
#endif

            switch (SymContext_p->Algorithm)
            {
            case VAL_SYM_ALGO_HASH_SHA1:
                DigestNBytes = (160 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA1;
#else
                AssetPolicy |= VAL_POLICY_SHA1;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA224:
                DigestNBytes = (256 / 8);
                // Note: intermediate digest is always 256 bits, final is 224 bits
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA224;
#else
                AssetPolicy |= VAL_POLICY_SHA224;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA256:
                DigestNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA256;
#else
                AssetPolicy |= VAL_POLICY_SHA256;
#endif
                break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
            case VAL_SYM_ALGO_HASH_SHA384:
                DigestNBytes = (512 / 8);
                // Note: intermediate digest is always 512 bits, final is 384 bits
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA384;
#else
                AssetPolicy |= VAL_POLICY_SHA384;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA512:
                DigestNBytes = (512 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA512;
#else
                AssetPolicy |= VAL_POLICY_SHA512;
#endif
                break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA3
            case VAL_SYM_ALGO_HASH_SHA3_224:
                DigestNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA3_224;
#else
                AssetPolicy |= EIP130_ASSET_POLICY_SHA3_224;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA3_256:
                DigestNBytes =(256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA3_256;
#else
                AssetPolicy |= EIP130_ASSET_POLICY_SHA3_256;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA3_384:
                DigestNBytes = (512 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA3_384;
#else
                AssetPolicy |= EIP130_ASSET_POLICY_SHA3_384;
#endif
                break;

            case VAL_SYM_ALGO_HASH_SHA3_512:
                DigestNBytes = (512 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSHA3_512;
#else
                AssetPolicy |= EIP130_ASSET_POLICY_SHA3_512;
#endif
                break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM3
            case VAL_SYM_ALGO_HASH_SM3:
                DigestNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
                AssetPolicy |= EIP130_ASSET_POLICY_SCAHSM3;
#else
                AssetPolicy |= VAL_POLICY_SM3;
#endif
                break;
#endif

            default:
                return VAL_INVALID_ALGORITHM;
            }

            // Allocate Asset for intermediate digest
            if (!val_IsAccessSecure())
            {
                AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
            }
            funcres = val_AssetAlloc(AssetPolicy, DigestNBytes,
                                     false, false,
                                     VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                                     &SymContext_p->Service.Hash.TempAssetId);
            if (funcres != VAL_SUCCESS)
            {
                LOG_WARN("%s: Abort - AssetAlloc()=%d\n", __func__, funcres);
                return funcres;
            }
        }
    }
    else
    {
        t_cmd.Service.Hash.Mode = VEXTOKEN_MODE_HASH_MAC_CONT2CONT;
    }

    // Set temporary Asset or intermediate Digest, Data and TotalMessageLength=0
    if (SymContext_p->fUseTokenTemp)
    {
        t_cmd.Service.Hash.TempAssetId = VAL_ASSETID_INVALID;
        if (SymContext_p->fInitDone)
        {
            memcpy(t_cmd.Service.Hash.Digest, SymContext_p->Service.Hash.Digest,
                   sizeof(t_cmd.Service.Hash.Digest));
        }
    }
    else
    {
        t_cmd.Service.Hash.TempAssetId = SymContext_p->Service.Hash.TempAssetId;
    }
    t_cmd.Service.Hash.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.Hash.DataSize = (uint32_t)DataSize;
    t_cmd.Service.Hash.TotalMessageLength = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            SymContext_p->Service.Hash.TotalMessageLength += (uint64_t)DataSize;

            if (SymContext_p->fUseTokenTemp)
            {
                // Save the intermediate Digest for the next operation
                memcpy(SymContext_p->Service.Hash.Digest, t_res.Service.Hash.Digest,
                       sizeof(SymContext_p->Service.Hash.Digest));
            }

            // Mark initialization done
            SymContext_p->fInitDone = true;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * val_SymHashFinal
 */
ValStatus_t
val_SymHashFinal(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsOut_t * const Digest_p,
        ValSize_t * const DigestSize_p)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;
    uint32_t DigestNBytes = 0;
    uint64_t TotalMessageLength;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0) ||
        (Digest_p == NULL) ||
        (DigestSize_p == NULL) ||
        ((Data_p == NULL) && (DataSize != 0)))
    {
        return VAL_BAD_ARGUMENT;
    }

    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
    case VAL_SYM_ALGO_HASH_SHA224:
    case VAL_SYM_ALGO_HASH_SHA256:
#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_HASH_SM3:
#endif
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_BLOCK_SIZE-1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
    case VAL_SYM_ALGO_HASH_SHA512:
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE-1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_HASH_SHA3_224:
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE-1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_256:
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE-1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_384:
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE-1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_512:
        if (DataSize > (VAL_SIZE_MAX_DMA - (VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE -1)))
        {
            return VAL_BAD_ARGUMENT;
        }
        break;
#endif
    default:
        break;
    }
#endif

    ZEROINIT(t_cmd);

    // Check and set hash algorithm
    TotalMessageLength = SymContext_p->Service.Hash.TotalMessageLength + (uint64_t)DataSize;
    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_HASH_SHA1:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA1;
        DigestNBytes = (160 / 8);
        break;

    case VAL_SYM_ALGO_HASH_SHA224:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA224;
        DigestNBytes = (224 / 8);
        break;

    case VAL_SYM_ALGO_HASH_SHA256:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA256;
        DigestNBytes = (256 / 8);
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_HASH_SHA384:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA384;
        DigestNBytes = (384 / 8);
        break;

    case VAL_SYM_ALGO_HASH_SHA512:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA512;
        DigestNBytes = (512 / 8);
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_HASH_SHA3_224:
        DigestNBytes = (224 / 8);
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_224;
        TotalMessageLength = (DataSize % VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE);
        if (TotalMessageLength == 0)
        {
            TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_256:
        DigestNBytes =(256 / 8);
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_256;
        TotalMessageLength = (DataSize % VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE);
        if (TotalMessageLength == 0)
        {
            TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_384:
        DigestNBytes = (384 / 8);
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_384;
        TotalMessageLength = (DataSize % VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE);
        if (TotalMessageLength == 0)
        {
            TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE;
        }
        break;

    case VAL_SYM_ALGO_HASH_SHA3_512:
        DigestNBytes = (512 / 8);
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SHA3_512;
        TotalMessageLength = (DataSize % VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE);
        if (TotalMessageLength == 0)
        {
            TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE;
        }
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_HASH_SM3:
        t_cmd.Service.Hash.Algorithm = VEXTOKEN_ALGO_HASH_SM3;
        DigestNBytes = (256 / 8);
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }

    if (DigestNBytes > *DigestSize_p)
    {
        // Final digest does not fit in buffer
        *DigestSize_p = DigestNBytes;
        return VAL_BUFFER_TOO_SMALL;
    }

    // Set service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_HASH;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_NOT_USED;

    // Set hash mode and temporary Asset or intermediate Digest
    if (SymContext_p->fInitDone)
    {
        t_cmd.Service.Hash.Mode = VEXTOKEN_MODE_HASH_MAC_CONT2FINAL;
        if (SymContext_p->fUseTokenTemp)
        {
            t_cmd.Service.Hash.TempAssetId = VAL_ASSETID_INVALID;
            memcpy(t_cmd.Service.Hash.Digest, SymContext_p->Service.Hash.Digest,
                   sizeof(t_cmd.Service.Hash.Digest));
        }
        else
        {
            t_cmd.Service.Hash.TempAssetId = SymContext_p->Service.Hash.TempAssetId;
        }
    }
    else
    {
        t_cmd.Service.Hash.Mode = VEXTOKEN_MODE_HASH_MAC_INIT2FINAL;
        t_cmd.Service.Hash.TempAssetId = VAL_ASSETID_INVALID;
    }

    // Set Data and TotalMessageLength=0
    t_cmd.Service.Hash.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.Hash.DataSize = (uint32_t)DataSize;
    t_cmd.Service.Hash.TotalMessageLength = TotalMessageLength;

    // Initialize result part
    ZEROINIT(t_res);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Copy final digest
            memcpy(Digest_p, t_res.Service.Hash.Digest, DigestNBytes);
            *DigestSize_p = DigestNBytes;

            // Release symmetric crypto context
            (void)val_SymRelease(Context_p);
       }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}


#endif /* !VAL_REMOVE_SYM_HASH */

/* end of file adapter_val_sym_hash.c */
