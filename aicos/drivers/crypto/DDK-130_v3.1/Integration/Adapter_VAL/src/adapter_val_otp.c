/* adapter_val_otp.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the OTP Data Write services.
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
 * val_OTPDataWrite
 */
#ifndef VAL_REMOVE_OTPDATAWRITE
ValStatus_t
val_OTPDataWrite(
        const ValAssetNumber_t StaticAssetNumber,
#ifdef EIP130_FW_ASSETPOLICY_V2
        ValPolicyMask_t AssetPolicy,
#else
        const ValAssetNumber_t AssetPolicyNumber,
#endif
        const bool fAddCRC,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsIn_t * const KeyBlob_p,
        const ValSize_t KeyBlobSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StaticAssetNumber > VAL_ASSET_NUMBER_MAX) ||
#ifdef EIP130_FW_ASSETPOLICY_V2
        (StaticAssetNumber == 0) ||
        (AssetPolicy == 0) ||
#else
        (AssetPolicyNumber == 0) ||
        (AssetPolicyNumber > VAL_POLICY_NUMBER_MAX) ||
#endif
        (AssociatedData_p == NULL) ||
        (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
        (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE) ||
        (KeyBlob_p == NULL) ||
        (KeyBlobSize == 0) ||
        (KeyBlobSize > VAL_OTP_KEYBLOB_SIZE(3072/8)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_OTPDATAWRITE;
    t_cmd.Service.OTPDataWrite.AssetNumber = (uint16_t)StaticAssetNumber;
#ifdef EIP130_FW_ASSETPOLICY_V2
    t_cmd.Service.OTPDataWrite.Policy = AssetPolicy;
#else
    t_cmd.Service.OTPDataWrite.PolicyNumber = (uint16_t)AssetPolicyNumber;
#endif
    t_cmd.Service.OTPDataWrite.fAddCRC = fAddCRC;
    if (AssociatedDataSize > sizeof(t_cmd.Service.OTPDataWrite.AssociatedData))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.OTPDataWrite.AssociatedData, AssociatedData_p, AssociatedDataSize);
    t_cmd.Service.OTPDataWrite.AssociatedDataSize = (uint32_t)AssociatedDataSize;
    t_cmd.Service.OTPDataWrite.KeyBlob_p = (const uint8_t *)KeyBlob_p;
    t_cmd.Service.OTPDataWrite.KeyBlobSize = (uint32_t)KeyBlobSize;
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
#endif /* !VAL_REMOVE_OTPDATAWRITE */


/*----------------------------------------------------------------------------
 * val_ProvisionRandomRootKey
 */
#ifndef VAL_REMOVE_PROVISIONRANDOMROOTKEY
ValStatus_t
val_ProvisionRandomRootKey(
        const uint32_t Identity,
        const ValAssetNumber_t StaticAssetNumber,
        const bool f128bit,
        const bool fAddCRC,
        const uint8_t  AutoSeed,
        const uint16_t SampleCycles,
        const uint8_t  SampleDiv,
        const uint8_t  NoiseBlocks,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((Identity == 0) ||
#ifdef EIP130_FW_ASSETPOLICY_V2
        (StaticAssetNumber != VAL_ASSETNUMBER_HUK))
#else
        (StaticAssetNumber > 30))
#endif
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PROVISIONRANDOMHUK;
    t_cmd.Service.ProvisionRandomHUK.Identity = Identity;
    t_cmd.Service.ProvisionRandomHUK.AssetNumber = (uint16_t)StaticAssetNumber;
    t_cmd.Service.ProvisionRandomHUK.f128bit = f128bit;
    t_cmd.Service.ProvisionRandomHUK.fAddCRC = fAddCRC;
    t_cmd.Service.ProvisionRandomHUK.AutoSeed = AutoSeed;
    t_cmd.Service.ProvisionRandomHUK.SampleCycles = SampleCycles;
    t_cmd.Service.ProvisionRandomHUK.SampleDiv = SampleDiv;
    t_cmd.Service.ProvisionRandomHUK.NoiseBlocks = NoiseBlocks;
    if ((KeyBlobSize_p != NULL) && (*KeyBlobSize_p > 0))
    {
#ifdef VAL_STRICT_ARGS
        if (f128bit)
        {
            if (*KeyBlobSize_p < VAL_OTP_KEYBLOB_SIZE(128/8))
            {
                return VAL_INVALID_LENGTH;
            }
        }
        else if (*KeyBlobSize_p < VAL_OTP_KEYBLOB_SIZE(256/8))
        {
            return VAL_INVALID_LENGTH;
        }

        if ((AssociatedData_p == NULL) ||
            (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
            (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif

        t_cmd.Service.ProvisionRandomHUK.KeyBlob_p = (const uint8_t *)KeyBlob_p;
        t_cmd.Service.ProvisionRandomHUK.KeyBlobSize = (uint32_t)*KeyBlobSize_p;
        t_cmd.Service.ProvisionRandomHUK.AssociatedDataSize = (uint8_t)AssociatedDataSize;
        if (AssociatedDataSize > sizeof(t_cmd.Service.ProvisionRandomHUK.AssociatedData))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.ProvisionRandomHUK.AssociatedData, AssociatedData_p, AssociatedDataSize);
    }
    else
    {
        t_cmd.Service.ProvisionRandomHUK.KeyBlob_p = NULL;
        t_cmd.Service.ProvisionRandomHUK.KeyBlobSize = 0;
        t_cmd.Service.ProvisionRandomHUK.AssociatedDataSize = 0;
    }
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.ProvisionRandomHUK.KeyBlobSize = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            if ((t_cmd.Service.ProvisionRandomHUK.KeyBlobSize != 0) &&
                (KeyBlobSize_p != NULL))
            {
                *KeyBlobSize_p = t_res.Service.ProvisionRandomHUK.KeyBlobSize;
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


/*----------------------------------------------------------------------------
 * val_ProvisionRandomRootKeyARIA
 */
#ifndef VAL_REMOVE_PROVISIONRANDOMROOTKEYARIA
ValStatus_t
val_ProvisionRandomRootKeyARIA(const uint32_t Identity,
                               const ValAssetNumber_t StaticAssetNumber,
                               const bool f128bit,
                               const bool fAddCRC,
                               const uint8_t  AutoSeed,
                               const uint16_t SampleCycles,
                               const uint8_t  SampleDiv,
                               const uint8_t  NoiseBlocks,
                               ValOctetsIn_t * const AssociatedData_p,
                               const ValSize_t AssociatedDataSize,
                               ValOctetsOut_t * const KeyBlob_p,
                               ValSize_t * const KeyBlobSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((Identity == 0) ||
#ifdef EIP130_FW_ASSETPOLICY_V2
        (StaticAssetNumber != VAL_ASSETNUMBER_HUK))
#else
        (StaticAssetNumber > 30))
#endif
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_PROVISIONRANDOMHUK;
    t_cmd.Service.ProvisionRandomHUK.Identity = Identity;
    t_cmd.Service.ProvisionRandomHUK.AssetNumber = (uint16_t)StaticAssetNumber;
    t_cmd.Service.ProvisionRandomHUK.f128bit = f128bit;
    t_cmd.Service.ProvisionRandomHUK.fAddCRC = fAddCRC;
    t_cmd.Service.ProvisionRandomHUK.AutoSeed = AutoSeed;
    t_cmd.Service.ProvisionRandomHUK.SampleCycles = SampleCycles;
    t_cmd.Service.ProvisionRandomHUK.SampleDiv = SampleDiv;
    t_cmd.Service.ProvisionRandomHUK.NoiseBlocks = NoiseBlocks;
    t_cmd.Service.ProvisionRandomHUK.fAria = true;

    if ((KeyBlobSize_p != NULL) && (*KeyBlobSize_p > 0))
    {
#ifdef VAL_STRICT_ARGS
        if (f128bit)
        {
            if (*KeyBlobSize_p < VAL_OTP_KEYBLOB_SIZE(128/8))
            {
                return VAL_INVALID_LENGTH;
            }
        }
        else if (*KeyBlobSize_p < VAL_OTP_KEYBLOB_SIZE(256/8))
        {
            return VAL_INVALID_LENGTH;
        }

        if ((AssociatedData_p == NULL) ||
            (AssociatedDataSize < VAL_KEYBLOB_AAD_MIN_SIZE) ||
            (AssociatedDataSize > VAL_KEYBLOB_AAD_MAX_SIZE))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
        t_cmd.Service.ProvisionRandomHUK.KeyBlob_p = (const uint8_t *)KeyBlob_p;
        t_cmd.Service.ProvisionRandomHUK.KeyBlobSize = (uint32_t)*KeyBlobSize_p;
        t_cmd.Service.ProvisionRandomHUK.AssociatedDataSize = (uint8_t)AssociatedDataSize;
        if (AssociatedDataSize > sizeof(t_cmd.Service.ProvisionRandomHUK.AssociatedData))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.ProvisionRandomHUK.AssociatedData, AssociatedData_p, AssociatedDataSize);
    }
    else
    {
        t_cmd.Service.ProvisionRandomHUK.KeyBlob_p = NULL;
        t_cmd.Service.ProvisionRandomHUK.KeyBlobSize = 0;
        t_cmd.Service.ProvisionRandomHUK.AssociatedDataSize = 0;
    }
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.ProvisionRandomHUK.KeyBlobSize = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            if ((t_cmd.Service.ProvisionRandomHUK.KeyBlobSize != 0) &&
                (KeyBlobSize_p != NULL))
            {
                *KeyBlobSize_p = t_res.Service.ProvisionRandomHUK.KeyBlobSize;
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
#endif
#endif /* !VAL_REMOVE_PROVISIONRANDOMROOTKEY */


/* end of file adapter_val_otp.c */
