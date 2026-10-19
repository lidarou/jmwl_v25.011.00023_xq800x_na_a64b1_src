/* adapter_val_asset.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the Asset Store services.
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

#include "api_val_asset.h"              // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_AssetAlloc
 *
 * Allocate an Asset and set its policy. Its content is setup later.
 */
#ifndef VAL_REMOVE_ASSET_ALLOC
ValStatus_t
val_AssetAlloc(
        const ValPolicyMask_t AssetPolicy,
        const ValSize_t AssetSize,
        const bool fAllHosts,
        const bool fRemoveSecure,
        const ValAssetLifetimeUse_t LifetimeUse,
        const bool fLifetimeNoLoad,
        const bool fLifetimeRelative,
        const uint32_t Lifetime,
        ValAssetId_t * const AssetId_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AssetPolicy == 0) ||
        (AssetSize > VAL_ASSET_SIZE_MAX) ||
        (AssetId_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    *AssetId_p = VAL_ASSETID_INVALID;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETCREATE;
    t_cmd.Service.AssetCreate.Policy = (uint64_t)AssetPolicy;
    t_cmd.Service.AssetCreate.Size = (uint32_t)AssetSize;
    t_cmd.Service.AssetCreate.fAllHosts = (uint8_t)fAllHosts;
    t_cmd.Service.AssetCreate.fRemoveSecure = (uint8_t)fRemoveSecure;
    t_cmd.Service.AssetCreate.LifetimeUse = (uint8_t)LifetimeUse;
    t_cmd.Service.AssetCreate.fLifetimeRelative = (uint8_t)fLifetimeRelative;
    t_cmd.Service.AssetCreate.fLifetimeNoLoad = (uint8_t)fLifetimeNoLoad;
    t_cmd.Service.AssetCreate.Lifetime = Lifetime;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.AssetCreate.AssetId = VAL_ASSETID_INVALID;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *AssetId_p = (ValAssetId_t)t_res.Service.AssetCreate.AssetId;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASSET_ALLOC */


/*----------------------------------------------------------------------------
 * val_AssetFree
 *
 * Free the Asset referenced by AssetId.
 */
#ifndef VAL_REMOVE_ASSET_FREE
ValStatus_t
val_AssetFree(
        const ValAssetId_t AssetId)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if (AssetId == VAL_ASSETID_INVALID)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETDELETE;
    t_cmd.Service.AssetDelete.AssetId = (uint32_t)AssetId;
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
#endif /* !VAL_REMOVE_ASSET_FREE */


/*----------------------------------------------------------------------------
 * val_AssetSearch
 *
 * Get the id for the asset identified by the given StaticAssetNumber.
 */
#ifndef VAL_REMOVE_ASSET_SEARCH
ValStatus_t
val_AssetSearch(
        const ValAssetNumber_t StaticAssetNumber,
        ValAssetId_t * const AssetId_p,
        ValSize_t * const AssetSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AssetId_p == NULL) ||
        (StaticAssetNumber > (VAL_ASSET_NUMBER_MAX + VAL_ASSET_NUMBER_CONSTANTS)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    *AssetId_p = VAL_ASSETID_INVALID;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETSEARCH;
    t_cmd.Service.AssetSearch.Number = StaticAssetNumber;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.AssetSearch.AssetId = VAL_ASSETID_INVALID;
    t_res.Service.AssetSearch.Size = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *AssetId_p = (ValAssetId_t)t_res.Service.AssetSearch.AssetId;
            if (AssetSize_p != NULL)
            {
                *AssetSize_p = (ValSize_t)t_res.Service.AssetSearch.Size;
            }
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASSET_SEARCH */


/*----------------------------------------------------------------------------
 * val_AssetGetRootKey
 *
 * Request the asset id associated with the root key.
 */
#ifndef VAL_REMOVE_GET_ROOT_KEY
ValAssetId_t
val_AssetGetRootKey(void)
{
    ValStatus_t funcres;
    ValAssetId_t RootKeyAssetId = VAL_ASSETID_INVALID;

#ifdef EIP130_FW_ASSETPOLICY_V2
    funcres = val_AssetSearch(VAL_ASSETNUMBER_HUK, &RootKeyAssetId, NULL);
#else
    funcres = val_AssetSearch(VAL_ROOT_KEY_INDEX, &RootKeyAssetId, NULL);
#endif
    if (funcres != VAL_SUCCESS)
    {
        LOG_WARN("Abort - %s()=%d\n", __func__, funcres);
    }

    return RootKeyAssetId;
}
#endif /* !VAL_REMOVE_GET_ROOT_KEY */


/*----------------------------------------------------------------------------
 * val_AssetLoadImport
 *
 * Setup the content of the asset referenced by TargetAssetId from the
 * given key blob. Use the given KEK and AD to unwrap the key blob.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_IMPORT
ValStatus_t
val_AssetLoadImport(
        const ValAssetId_t TargetAssetId,
        const ValAssetId_t KekAssetId,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsIn_t * const KeyBlob_p,
        const ValSize_t KeyBlobSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (KekAssetId == VAL_ASSETID_INVALID) ||
        (AssociatedData_p == NULL) ||
        (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
        (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
        (KeyBlob_p == NULL) ||
        (KeyBlobSize == 0) ||
        (KeyBlobSize > VAL_KEYBLOB_SIZE(VAL_ASSET_SIZE_MAX)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadImport.Method = VEXTOKEN_ASSETLOAD_IMPORT;
    t_cmd.Service.AssetLoadImport.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadImport.KekAssetId = (uint32_t)KekAssetId;
    if (AssociatedDataSize > sizeof(t_cmd.Service.AssetLoadImport.AssociatedData))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.AssetLoadImport.AssociatedData, AssociatedData_p, AssociatedDataSize);
    t_cmd.Service.AssetLoadImport.AssociatedDataSize = (uint32_t)AssociatedDataSize;
    t_cmd.Service.AssetLoadImport.KeyBlob_p = (const uint8_t *)KeyBlob_p;
    t_cmd.Service.AssetLoadImport.KeyBlobSize = (uint32_t)KeyBlobSize;
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
#endif /* !VAL_REMOVE_ASSET_LOAD_IMPORT */


/*----------------------------------------------------------------------------
 * val_AssetLoadDerive
 *
 * Setup the content of the asset referenced by TargetAssetId by deriving it
 * from the given KDK and label info.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_DERIVE
ValStatus_t
val_AssetLoadDerive(
        const ValAssetId_t TargetAssetId,
        const ValAssetId_t KdkAssetId,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        const bool fCounter,
        const bool fRFC5869,
        ValOctetsIn_Optional_t * const Salt_p,
        const ValSize_t SaltSize,
        ValOctetsIn_Optional_t * const IV_p,
        const ValSize_t IVSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (KdkAssetId == VAL_ASSETID_INVALID) ||
        (AssociatedData_p == NULL) ||
        (AssociatedDataSize < VAL_KDF_LABEL_MIN_SIZE) ||
        (AssociatedDataSize > VAL_KDF_LABEL_MAX_SIZE) ||
        (fCounter && fRFC5869))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadDerive.Method = VEXTOKEN_ASSETLOAD_DERIVE;
    t_cmd.Service.AssetLoadDerive.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadDerive.KdkAssetId = (uint32_t)KdkAssetId;
    if (AssociatedDataSize > sizeof(t_cmd.Service.AssetLoadDerive.AssociatedData))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.AssetLoadDerive.AssociatedData, AssociatedData_p, AssociatedDataSize);
    t_cmd.Service.AssetLoadDerive.AssociatedDataSize = (uint32_t)AssociatedDataSize;
    t_cmd.Service.AssetLoadDerive.Salt_p = (const uint8_t *)Salt_p;
    t_cmd.Service.AssetLoadDerive.SaltSize = (uint32_t)SaltSize;
    t_cmd.Service.AssetLoadDerive.IV_p = (const uint8_t *)IV_p;
    t_cmd.Service.AssetLoadDerive.IVSize = (uint32_t)IVSize;
    t_cmd.Service.AssetLoadDerive.fCounter = (bool)fCounter;
    t_cmd.Service.AssetLoadDerive.fRFC5869 = (bool)fRFC5869;
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
#endif /* !VAL_REMOVE_ASSET_LOAD_DERIVE */


/*----------------------------------------------------------------------------
 * val_AssetLoadPlaintext
 *
 * Setup the content of the asset referenced by TargetAssetId from the
 * given plain data.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_PLAINTEXT
ValStatus_t
val_AssetLoadPlaintext(
        const ValAssetId_t TargetAssetId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (DataSize > VAL_ASSET_SIZE_MAX))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadPlaintext.Method = VEXTOKEN_ASSETLOAD_PLAINTEXT;
    t_cmd.Service.AssetLoadPlaintext.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadPlaintext.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.AssetLoadPlaintext.DataSize = (uint32_t)DataSize;
    t_cmd.Service.AssetLoadPlaintext.KekAssetId = VAL_ASSETID_INVALID;
    t_cmd.Service.AssetLoadPlaintext.AssociatedDataSize = 0;
    t_cmd.Service.AssetLoadPlaintext.KeyBlob_p = NULL;
    t_cmd.Service.AssetLoadPlaintext.KeyBlobSize = 0;
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
#endif /* !VAL_REMOVE_ASSET_LOAD_PLAINTEXT */


/*----------------------------------------------------------------------------
 * val_AssetLoadPlaintextExport
 *
 * Setup the content of the asset referenced by TargetAssetId from the
 * given plain data and export the resulting asset as a key blob.
 * Use the given KEK and AD to create the key blob.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_PLAINTEXT_EXPORT
ValStatus_t
val_AssetLoadPlaintextExport(
        const ValAssetId_t TargetAssetId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        const ValAssetId_t KekAssetId,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (DataSize > VAL_ASSET_SIZE_MAX) ||
        (KekAssetId == VAL_ASSETID_INVALID) ||
        (AssociatedData_p == NULL) ||
        (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
        (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
        (KeyBlob_p == NULL) ||
        (KeyBlobSize_p == NULL) ||
        (*KeyBlobSize_p < VAL_KEYBLOB_SIZE(DataSize)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadPlaintext.Method = VEXTOKEN_ASSETLOAD_PLAINTEXT;
    t_cmd.Service.AssetLoadPlaintext.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadPlaintext.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.AssetLoadPlaintext.DataSize = (uint32_t)DataSize;
    t_cmd.Service.AssetLoadPlaintext.KekAssetId = (uint32_t)KekAssetId;
    if (AssociatedDataSize > sizeof(t_cmd.Service.AssetLoadPlaintext.AssociatedData))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.AssetLoadPlaintext.AssociatedData, AssociatedData_p, AssociatedDataSize);
    t_cmd.Service.AssetLoadPlaintext.AssociatedDataSize = (uint32_t)AssociatedDataSize;
    t_cmd.Service.AssetLoadPlaintext.KeyBlob_p = (uint8_t *)KeyBlob_p;
    t_cmd.Service.AssetLoadPlaintext.KeyBlobSize = (uint32_t)*KeyBlobSize_p;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.AssetLoad.KeyBlobSize = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *KeyBlobSize_p = t_res.Service.AssetLoad.KeyBlobSize;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASSET_LOAD_PLAINTEXT_EXPORT */


/*----------------------------------------------------------------------------
 * val_AssetLoadRandom
 *
 * Setup the content of the asset referenced by TargetAssetId with random
 * data obtained from the RNG.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_RANDOM
ValStatus_t
val_AssetLoadRandom(
        const ValAssetId_t TargetAssetId)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if (TargetAssetId == VAL_ASSETID_INVALID)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadRandom.Method = VEXTOKEN_ASSETLOAD_RANDOM;
    t_cmd.Service.AssetLoadRandom.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadRandom.KekAssetId = VAL_ASSETID_INVALID;
    t_cmd.Service.AssetLoadRandom.AssociatedDataSize = 0;
    t_cmd.Service.AssetLoadRandom.KeyBlob_p = NULL;
    t_cmd.Service.AssetLoadRandom.KeyBlobSize = 0;
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
#endif /* !VAL_REMOVE_ASSET_LOAD_RANDOM */


/*----------------------------------------------------------------------------
 * val_AssetLoadRandomExport
 *
 * Setup the content of the asset referenced by TargetAssetId with random
 * data obtained from the RNG and export the resulting asset as a key blob.
 * Use the given KEK and AD to create the key blob.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_RANDOM_EXPORT
ValStatus_t
val_AssetLoadRandomExport(
        const ValAssetId_t TargetAssetId,
        const ValAssetId_t KekAssetId,
        ValOctetsIn_t * const AssociatedData_p,
        ValSize_t AssociatedDataSize,
        ValOctetsOut_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (KekAssetId == VAL_ASSETID_INVALID) ||
        (AssociatedData_p == NULL) ||
        (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
        (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
        (KeyBlob_p == NULL) ||
        (KeyBlobSize_p == NULL) ||
        (*KeyBlobSize_p < VAL_KEYBLOB_SIZE(0)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadRandom.Method = VEXTOKEN_ASSETLOAD_RANDOM;
    t_cmd.Service.AssetLoadRandom.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadRandom.KekAssetId = (uint32_t)KekAssetId;
    if (AssociatedDataSize > sizeof(t_cmd.Service.AssetLoadRandom.AssociatedData))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.AssetLoadRandom.AssociatedData, AssociatedData_p, AssociatedDataSize);
    t_cmd.Service.AssetLoadRandom.AssociatedDataSize = (uint32_t)AssociatedDataSize;
    t_cmd.Service.AssetLoadRandom.KeyBlob_p = (uint8_t *)KeyBlob_p;
    t_cmd.Service.AssetLoadRandom.KeyBlobSize = (uint32_t)*KeyBlobSize_p;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.AssetLoad.KeyBlobSize = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *KeyBlobSize_p = t_res.Service.AssetLoad.KeyBlobSize;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_ASSET_LOAD_RANDOM_EXPORT */


/*----------------------------------------------------------------------------
 * val_AssetLoadSymUnwrap
 *
 * Setup the content of the asset referenced by TargetAssetId from the
 * given AES/SM4 WRAP key blob. Use the given KEK to unwrap the key blob.
 */
#ifndef VAL_REMOVE_ASSET_LOAD_SYMWRAP
ValStatus_t
val_AssetLoadSymUnwrap(
        const ValAssetId_t TargetAssetId,
        const ValAssetId_t KekAssetId,
        ValOctetsIn_t * const KeyBlob_p,
        const ValSize_t KeyBlobSize,
        uint8_t Algorithm)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        (KekAssetId == VAL_ASSETID_INVALID) ||
        (KeyBlob_p == NULL) ||
        ((KeyBlobSize - (64/8)) > VAL_ASSET_SIZE_MAX))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETLOAD;
    t_cmd.Service.AssetLoadSymUnwrap.Method = VEXTOKEN_ASSETLOAD_SYMUNWRAP;
    t_cmd.Service.AssetLoadSymUnwrap.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetLoadSymUnwrap.KekAssetId = (uint32_t)KekAssetId;
    t_cmd.Service.AssetLoadSymUnwrap.KeyBlob_p = (const uint8_t *)KeyBlob_p;
    t_cmd.Service.AssetLoadSymUnwrap.KeyBlobSize = (uint32_t)KeyBlobSize;

    if (Algorithm == VAL_SYM_ALGO_CIPHER_AES)
    {
        t_cmd.Service.AssetLoadSymUnwrap.Algorithm = VEXTOKEN_ALGO_CIPHER_AES;
    }
    else if (Algorithm == VAL_SYM_ALGO_CIPHER_SM4)
    {
        t_cmd.Service.AssetLoadSymUnwrap.Algorithm = VEXTOKEN_ALGO_CIPHER_SM4;
    }
    else
    {
        return VAL_BAD_ARGUMENT;
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
#endif /* !VAL_REMOVE_ASSET_LOAD_SYMWRAP */


/*----------------------------------------------------------------------------
 * val_AssetExportViaCPIF
 *
 * This operation transfers data from a specified Asset to an address within
 * one or more of the Coprocessor that can be connected to the EIP-130 CPIF.
 */
#ifndef VAL_REMOVE_ASSET_EXPORT_VIA_CPIF
ValStatus_t
val_AssetExportViaCPIF(
    const ValAssetId_t TargetAssetId,
    const uint16_t Address,
    const uint16_t CPxSelectionBits)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((TargetAssetId == VAL_ASSETID_INVALID) ||
        ((CPxSelectionBits & ~(BIT_10-1)) != 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_CPIF_EXPORT;

    t_cmd.Service.AssetExportCPIF.AssetId = (uint32_t)TargetAssetId;
    t_cmd.Service.AssetExportCPIF.Address = Address;
    t_cmd.Service.AssetExportCPIF.CPxSelectionBits = CPxSelectionBits;
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
#endif /* !VAL_REMOVE_ASSET_EXPORT_VIA_CPIF */


/*----------------------------------------------------------------------------
 * val_AssetStoreReset
 *
 * This operation resets/clears the dynamic Asset Store.
 */
#ifndef VAL_REMOVE_ASSET_STORE_RESET
ValStatus_t
val_AssetStoreReset(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ASSETSTORERESET;
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
#endif /* !VAL_REMOVE_ASSET_STORE_RESET */

/* end of file adapter_val_asset.c */
