/* adapter_val_sym_mac.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the symmetric crypto MAC services.
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

#ifndef VAL_REMOVE_SYM_MAC

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_sym.h"                // the API to implement
#include "api_val_asset.h"              // Asset Management related information
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


#ifdef VAL_STRICT_ARGS
#define MAC_DATASIZE_CHECK(block_size) do { \
        if (DataSize & (block_size - 1)) {  \
            return VAL_BAD_ARGUMENT;        \
        }                                   \
    } while(0)
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
#define MAC_SHA3_DATASIZE_CHECK(block_size) do { \
            if ((DataSize % block_size) != 0) {  \
                return VAL_BAD_ARGUMENT;         \
            }                                    \
        } while(0)
#endif
#else
#define MAC_DATASIZE_CHECK(block_size)
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
#define MAC_SHA3_DATASIZE_CHECK(block_size)
#endif
#endif


/*----------------------------------------------------------------------------
 * val_SymMacInitVerify
 */
ValStatus_t
val_SymMacInitVerify(
        ValSymContextPtr_t const Context_p)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_MAC_HMAC_SHA1:
    case VAL_SYM_ALGO_MAC_HMAC_SHA224:
    case VAL_SYM_ALGO_MAC_HMAC_SHA256:
#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_MAC_HMAC_SHA384:
    case VAL_SYM_ALGO_MAC_HMAC_SHA512:
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_224:
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_256:
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_384:
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_512:
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_MAC_HMAC_SM3:
#endif
    case VAL_SYM_ALGO_MAC_AES_CMAC:
    case VAL_SYM_ALGO_MAC_AES_CBC_MAC:
#ifndef VAL_REMOVE_SYM_ALGO_ARIA
    case VAL_SYM_ALGO_MAC_ARIA_CMAC:
    case VAL_SYM_ALGO_MAC_ARIA_CBC_MAC:
#endif
#ifndef VAL_REMOVE_SYM_ALGO_POLY1305
    case VAL_SYM_ALGO_MAC_POLY1305:
#endif
        if (SymContext_p->fInitDone)
        {
            return VAL_OPERATION_FAILED;
        }
        SymContext_p->Service.Mac.fVerify = true;
        break;

    default:
        return VAL_INVALID_ALGORITHM;
    }
    return VAL_SUCCESS;
}


/*----------------------------------------------------------------------------
 * val_SymMacUpdate
 */
ValStatus_t
val_SymMacUpdate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;
    ValSize_t MacNBytes = 0;
    ValPolicyMask_t AssetPolicy;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0) ||
        (Data_p == NULL) ||
        (DataSize == 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_SYM_TEMP;
    if (SymContext_p->Service.Mac.fVerify)
    {
        AssetPolicy |= EIP130_ASSET_POLICY_SCDIRDECVRFY;
    }
    else
    {
        AssetPolicy |= EIP130_ASSET_POLICY_SCDIRENCGEN;
    }
#else
    AssetPolicy = VAL_POLICY_TEMP_MAC;
#endif

    ZEROINIT(t_cmd);

    // Check and set MAC algorithm
    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_MAC_HMAC_SHA1:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA1;
        MacNBytes = (160 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA1;
#else
        AssetPolicy |= VAL_POLICY_SHA1;
#endif
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA224:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA224;
        MacNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA224;
#else
        AssetPolicy |= VAL_POLICY_SHA224;
#endif
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA256:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA256;
        MacNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA256;
#else
        AssetPolicy |= VAL_POLICY_SHA256;
#endif
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_MAC_HMAC_SHA384:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA384;
        MacNBytes = (512 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA384;
#else
        AssetPolicy |= VAL_POLICY_SHA384;
#endif
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA512:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA512;
        MacNBytes = (512 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA512;
#else
        AssetPolicy |= VAL_POLICY_SHA512;
#endif
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_224:
        MAC_SHA3_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_224;
        MacNBytes = (1600 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA3_224;
#else
        AssetPolicy |= EIP130_ASSET_POLICY_SHA3_224;
#endif
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_256:
        MAC_SHA3_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_256;
        MacNBytes = (1600 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA3_256;
#else
        AssetPolicy |= EIP130_ASSET_POLICY_SHA3_256;
#endif
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_384:
        MAC_SHA3_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_384;
        MacNBytes = (1600 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA3_384;
#else
        AssetPolicy |= EIP130_ASSET_POLICY_SHA3_384;
#endif
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_512:
        MAC_SHA3_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_512;
        MacNBytes = (1600 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSHA3_512;
#else
        AssetPolicy |= EIP130_ASSET_POLICY_SHA3_512;
#endif
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_MAC_HMAC_SM3:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_HASH_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SM3;
        MacNBytes = (256 / 8);
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHSM3;
#else
        AssetPolicy |= VAL_POLICY_SM3;
#endif
        break;
#endif

    case VAL_SYM_ALGO_MAC_AES_CMAC:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_AES_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_AES_CMAC;
        MacNBytes = VAL_SYM_ALGO_AES_IV_SIZE;
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACCIPHER | EIP130_ASSET_POLICY_SCACAES | EIP130_ASSET_POLICY_SCMCMCMAC;
#else
        AssetPolicy |= VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_CMAC;
#endif
        break;

    case VAL_SYM_ALGO_MAC_AES_CBC_MAC:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_AES_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_AES_CBC_MAC;
        MacNBytes = VAL_SYM_ALGO_AES_IV_SIZE;
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACCIPHER | EIP130_ASSET_POLICY_SCACAES | EIP130_ASSET_POLICY_SCMCMCBCMAC;
#else
        AssetPolicy |= VAL_POLICY_ALGO_CIPHER_AES;
#endif
        break;

#ifndef VAL_REMOVE_SYM_ALGO_ARIA
    case VAL_SYM_ALGO_MAC_ARIA_CMAC:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_ARIA_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_ARIA_CMAC;
        MacNBytes = VAL_SYM_ALGO_ARIA_IV_SIZE;
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACCIPHER | EIP130_ASSET_POLICY_SCACARIA | EIP130_ASSET_POLICY_SCMCMCMAC;
#else
        AssetPolicy |= VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_CMAC;
#endif
        break;

    case VAL_SYM_ALGO_MAC_ARIA_CBC_MAC:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_ARIA_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_ARIA_CBC_MAC;
        MacNBytes = VAL_SYM_ALGO_ARIA_IV_SIZE;
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACCIPHER | EIP130_ASSET_POLICY_SCACARIA | EIP130_ASSET_POLICY_SCMCMCBCMAC;
#else
        AssetPolicy |= VAL_POLICY_ALGO_CIPHER_ARIA;
#endif
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_POLY1305
    case VAL_SYM_ALGO_MAC_POLY1305:
        MAC_DATASIZE_CHECK(VAL_SYM_ALGO_POLY1305_BLOCK_SIZE);
        t_cmd.Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_POLY1305;
        MacNBytes = 24;
#ifdef EIP130_FW_ASSETPOLICY_V2
        AssetPolicy |= EIP130_ASSET_POLICY_SCUIMACHASH | EIP130_ASSET_POLICY_SCAHPOLY1305;
#else
        AssetPolicy |= VAL_POLICY_POLY1305;
#endif
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }

    // Set service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_MAC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_MACUPDATE;

    // Set MAC mode
    if (!SymContext_p->fInitDone)
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_INIT2CONT;

        if (!SymContext_p->fUseTokenTemp &&
            (SymContext_p->Service.Mac.TempAssetId == VAL_ASSETID_INVALID))
        {
            // Allocate Asset for intermediate MAC value
            if (!val_IsAccessSecure())
            {
                AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
            }

            funcres = val_AssetAlloc(AssetPolicy, MacNBytes,
                                     false, false,
                                     VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                                     &SymContext_p->Service.Mac.TempAssetId);
            if (funcres != VAL_SUCCESS)
            {
                LOG_WARN("%s: Abort - AssetAlloc()=%d\n", __func__, funcres);
                return funcres;
            }

            if (SymContext_p->Algorithm == VAL_SYM_ALGO_MAC_AES_CBC_MAC)
            {
                // Initialize the intermediate MAC Asset data
                uint8_t * AssetData_p = Adapter_Alloc((unsigned int)MacNBytes);
                if (AssetData_p == NULL)
                {
                    funcres = VAL_NO_MEMORY;
                }
                else
                {
                    memcpy(AssetData_p, SymContext_p->Service.Mac.Mac, MacNBytes);
                    funcres = val_AssetLoadPlaintext(SymContext_p->Service.Mac.TempAssetId,
                                                     AssetData_p, MacNBytes);
                    Adapter_Free(AssetData_p);
                }
                if (funcres != VAL_SUCCESS)
                {
                    LOG_WARN("%s: Abort - AssetLoadPlaintext()=%d\n",
                             __func__, funcres);

                    // Free Asset again
                    (void)val_AssetFree(SymContext_p->Service.Mac.TempAssetId);
                    SymContext_p->Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
                    return funcres;
                }
            }
        }
    }
    else
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_CONT2CONT;
    }

    // Set temporary Asset or intermediate MAC, Key, Data and TotalMessageLength=0
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
    if (SymContext_p->fUseTokenTemp)
    {
        if (SymContext_p->Service.Mac.KeyAssetId != VAL_ASSETID_INVALID)
        {
            // Token intermediate MAC and Key from Asset Store is not allowed
            return VAL_INVALID_ASSET;
        }

        t_cmd.Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
        if (SymContext_p->fInitDone)
        {
            memcpy(t_cmd.Service.Mac.TempMac, SymContext_p->Service.Mac.Mac,
                   sizeof(t_cmd.Service.Mac.TempMac));
        }
    }
    else
    {
        t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
    }
#else
    t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
#endif
    t_cmd.Service.Mac.KeyAssetId = SymContext_p->Service.Mac.KeyAssetId;
    t_cmd.Service.Mac.KeySize = SymContext_p->Service.Mac.KeySize;
    if (t_cmd.Service.Mac.KeyAssetId == VAL_ASSETID_INVALID)
    {
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
        if (SymContext_p->Service.Mac.KeySize > sizeof(t_cmd.Service.Mac.Key))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.Mac.Key, SymContext_p->Service.Mac.Key,
               SymContext_p->Service.Mac.KeySize);
#else
        return VAL_INVALID_ASSET;
#endif
    }
    t_cmd.Service.Mac.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.Mac.DataSize = (uint32_t)DataSize;
    t_cmd.Service.Mac.TotalMessageLength = 0;

    // Initialize result part
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
    if (SymContext_p->fUseTokenTemp)
    {
        ZEROINIT(t_res.Service.Mac.Mac);
    }
#endif

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            SymContext_p->Service.Mac.TotalMessageLength += (uint64_t)DataSize;

#ifdef VAL_REMOVE_SYM_ALGO_SHA3
            if (SymContext_p->fUseTokenTemp)
            {
                // Save the intermediate MAC for the next operation
                memcpy(SymContext_p->Service.Mac.Mac, t_res.Service.Mac.Mac,
                       sizeof(SymContext_p->Service.Mac.Mac));
            }
#endif

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


static ValStatus_t
valLocal_SymMacGenVerAlgCheck(
        ValSymContext_t * SymContext_p,
        VexToken_Command_t * t_cmd_p,
        const ValSize_t DataSize,
        uint32_t * MacNBytes_p)
{
    switch (SymContext_p->Algorithm)
    {
    case VAL_SYM_ALGO_MAC_HMAC_SHA1:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA1;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (160 / 8);
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA224:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA224;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (224 / 8);
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA256:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA256;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (256 / 8);
        break;

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
    case VAL_SYM_ALGO_MAC_HMAC_SHA384:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA384;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (384 / 8);
        break;

    case VAL_SYM_ALGO_MAC_HMAC_SHA512:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA512;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (512 / 8);
        break;
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_224:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_224;
        t_cmd_p->Service.Mac.TotalMessageLength =
            (DataSize % VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE);
        if (t_cmd_p->Service.Mac.TotalMessageLength == 0)
        {
            t_cmd_p->Service.Mac.TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE;
        }
        *MacNBytes_p = (224 / 8);
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_256:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_256;
        t_cmd_p->Service.Mac.TotalMessageLength =
            (DataSize % VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE);
        if (t_cmd_p->Service.Mac.TotalMessageLength == 0)
        {
            t_cmd_p->Service.Mac.TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE;
        }
        *MacNBytes_p = (256 / 8);
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_384:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_384;
        t_cmd_p->Service.Mac.TotalMessageLength =
            (DataSize % VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE);
        if (t_cmd_p->Service.Mac.TotalMessageLength == 0)
        {
            t_cmd_p->Service.Mac.TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE;
        }
        *MacNBytes_p = (384 / 8);
        break;
    case VAL_SYM_ALGO_MAC_HMAC_SHA3_512:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SHA3_512;
        t_cmd_p->Service.Mac.TotalMessageLength =
            (DataSize % VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE);
        if (t_cmd_p->Service.Mac.TotalMessageLength == 0)
        {
            t_cmd_p->Service.Mac.TotalMessageLength = VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE;
        }
        *MacNBytes_p = (512 / 8);
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM3
    case VAL_SYM_ALGO_MAC_HMAC_SM3:
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_HMAC_SM3;
        t_cmd_p->Service.Mac.TotalMessageLength =
            SymContext_p->Service.Mac.TotalMessageLength + DataSize;
        *MacNBytes_p = (256 / 8);
        break;
#endif

    case VAL_SYM_ALGO_MAC_AES_CMAC:
        // TotalMessageLength and last block padding are handled in VEX part
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_AES_CMAC;
        *MacNBytes_p = VAL_SYM_ALGO_AES_BLOCK_SIZE;
        break;

    case VAL_SYM_ALGO_MAC_AES_CBC_MAC:
        // TotalMessageLength and last block padding are handled in VEX part
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_AES_CBC_MAC;
        *MacNBytes_p = VAL_SYM_ALGO_AES_BLOCK_SIZE;
        break;

#ifndef VAL_REMOVE_SYM_ALGO_ARIA
    case VAL_SYM_ALGO_MAC_ARIA_CMAC:
        // TotalMessageLength and last block padding are handled in VEX part
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_ARIA_CMAC;
        *MacNBytes_p = VAL_SYM_ALGO_ARIA_BLOCK_SIZE;
        break;

    case VAL_SYM_ALGO_MAC_ARIA_CBC_MAC:
        // TotalMessageLength and last block padding are handled in VEX part
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_ARIA_CBC_MAC;
        *MacNBytes_p = VAL_SYM_ALGO_ARIA_BLOCK_SIZE;
        break;
#endif

#ifndef VAL_REMOVE_SYM_ALGO_POLY1305
    case VAL_SYM_ALGO_MAC_POLY1305:
        // TotalMessageLength is not used
        t_cmd_p->Service.Mac.Algorithm = VEXTOKEN_ALGO_MAC_POLY1305;
        *MacNBytes_p = 16;
        break;
#endif

    default:
        return VAL_INVALID_ALGORITHM;
    }

    return VAL_SUCCESS;
}
/*----------------------------------------------------------------------------
 * val_SymMacGenerate
 */
ValStatus_t
val_SymMacGenerate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsOut_t * const Mac_p,
        ValSize_t * const MacSize_p)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;
    uint32_t MacNBytes = 0;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0) ||
        (Mac_p == NULL) ||
        (MacSize_p == NULL) ||
        ((Data_p == NULL) && (DataSize != 0)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    ZEROINIT(t_cmd);

    // Check and set MAC algorithm and TotalMessageLength
    funcres = valLocal_SymMacGenVerAlgCheck(SymContext_p, &t_cmd, DataSize, &MacNBytes);
    if (funcres != VAL_SUCCESS)
    {
        return funcres;
    }

    if (MacNBytes > *MacSize_p)
    {
        // Final digest does not fit in buffer
        *MacSize_p = MacNBytes;
        return VAL_BUFFER_TOO_SMALL;
    }

    // Set service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_MAC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_MACGENERATE;

    // Set MAC mode and temporary Asset or intermediate MAC
    if (SymContext_p->fInitDone)
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_CONT2FINAL;
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
        if (SymContext_p->fUseTokenTemp)
        {
            t_cmd.Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
            memcpy(t_cmd.Service.Mac.TempMac, SymContext_p->Service.Mac.Mac,
                   sizeof(t_cmd.Service.Mac.TempMac));
        }
        else
        {
            t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
        }
#else
        t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
#endif
    }
    else
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_INIT2FINAL;
        t_cmd.Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
    }

    // Set other relevant information
    t_cmd.Service.Mac.KeyAssetId = SymContext_p->Service.Mac.KeyAssetId;
    t_cmd.Service.Mac.KeySize = SymContext_p->Service.Mac.KeySize;
    if (t_cmd.Service.Mac.KeyAssetId == VAL_ASSETID_INVALID)
    {
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
        if (SymContext_p->Service.Mac.KeySize > sizeof(t_cmd.Service.Mac.Key))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.Mac.Key, SymContext_p->Service.Mac.Key,
               SymContext_p->Service.Mac.KeySize);
#else
        return VAL_INVALID_ASSET;
#endif
    }
    t_cmd.Service.Mac.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.Mac.DataSize = (uint32_t)DataSize;

    // Initialize result part
    ZEROINIT(t_res);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Copy generated MAC value
            memcpy(Mac_p, t_res.Service.Mac.Mac, MacNBytes);
            *MacSize_p = MacNBytes;

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


/*----------------------------------------------------------------------------
 * val_SymMacVerify
 */
ValStatus_t
val_SymMacVerify(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValAssetId_Optional_t const MacAssetId,
        ValOctetsIn_Optional_t * const Mac_p,
        const ValSize_t MacSize)
{
    ValSymContext_t * SymContext_p = (ValSymContext_t *)Context_p;
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;
    uint32_t MacNBytes = 0;

#ifdef VAL_STRICT_ARGS
    if ((SymContext_p == NULL) ||
        (SymContext_p->MagicBegin != VALMARKER_SYMCONTEXT) ||
        ((SymContext_p->MagicBegin ^ SymContext_p->MagicEnd) != 0) ||
        ((MacAssetId == VAL_ASSETID_INVALID) && (Mac_p == NULL)) ||
        ((Data_p == NULL) && (DataSize != 0)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_MAC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_MACVERIFY;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Set MAC algorithm and TotalMessageLength
    funcres = valLocal_SymMacGenVerAlgCheck(SymContext_p, &t_cmd, DataSize, &MacNBytes);
    if (funcres != VAL_SUCCESS)
    {
        IDENTIFIER_NOT_USED(MacNBytes);
        return funcres;
    }

    // Set MAC mode and temporary Asset or intermediate MAC
    if (SymContext_p->fInitDone)
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_CONT2FINAL;
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
        if (SymContext_p->fUseTokenTemp)
        {
            t_cmd.Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
            memcpy(t_cmd.Service.Mac.TempMac, SymContext_p->Service.Mac.Mac,
                   sizeof(t_cmd.Service.Mac.TempMac));
        }
        else
        {
            t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
        }
#else
        t_cmd.Service.Mac.TempAssetId = SymContext_p->Service.Mac.TempAssetId;
#endif
    }
    else
    {
        t_cmd.Service.Mac.Mode = VEXTOKEN_MODE_HASH_MAC_INIT2FINAL;
        t_cmd.Service.Mac.TempAssetId = VAL_ASSETID_INVALID;
    }

    // Set Key, Data and final MAC (Asset)
    t_cmd.Service.Mac.KeyAssetId = SymContext_p->Service.Mac.KeyAssetId;
    t_cmd.Service.Mac.KeySize = SymContext_p->Service.Mac.KeySize;
    if (t_cmd.Service.Mac.KeyAssetId == VAL_ASSETID_INVALID)
    {
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
        if (SymContext_p->Service.Mac.KeySize > sizeof(t_cmd.Service.Mac.Key))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.Mac.Key, SymContext_p->Service.Mac.Key,
               SymContext_p->Service.Mac.KeySize);
#else
        return VAL_INVALID_ASSET;
#endif
    }
    t_cmd.Service.Mac.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.Mac.DataSize = (uint32_t)DataSize;
    t_cmd.Service.Mac.MacAssetId = MacAssetId;
    if (MacAssetId == VAL_ASSETID_INVALID)
    {
        if (MacSize > sizeof(t_cmd.Service.Mac.Mac))
        {
            return VAL_INVALID_LENGTH;
        }
        memcpy(t_cmd.Service.Mac.Mac, Mac_p, MacSize);
        t_cmd.Service.Mac.MacSize = (uint32_t)MacSize;
    }

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        funcres = (ValStatus_t)t_res.Result;
        if ((t_res.Result == VEXTOKEN_RESULT_NO_ERROR) ||
            (t_res.Result == VEXTOKEN_RESULT_SEQ_VERIFY_ERROR))
        {
            // Release symmetric crypto context
            (void)val_SymRelease(Context_p);
        }
        else
        {
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}


#endif /* !VAL_REMOVE_SYM_MAC */

/* end of file adapter_val_sym_mac.c */
