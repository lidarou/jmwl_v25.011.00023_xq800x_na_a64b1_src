/* adapter_val_aunlock.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the Authenticated Unlock and Secure Debug services.
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

#include "api_val_aunlock.h"            // the API to implement
#include "api_val_asset.h"              // the API to implement
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_AuthenticatedUnlockStart
 */
#ifndef VAL_REMOVE_AUTH_UNLOCK
ValStatus_t
val_AuthenticatedUnlockStart(
        ValAssetNumber_t AuthKeyNumber,
        ValAssetId_t * AuthStateASId_p,
        ValOctetsOut_t * Nonce_p,
        ValSize_t * NonceLength_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValAssetId_t KeyAssetId;
#ifdef EIP130_FW_ASSETPOLICY_V2
    ValPolicyMask_t AssetPolicy = EIP130_ASSET_POLICY_ASYM_TEMP | EIP130_ASSET_POLICY_ACUI_SDGPIO;
#else
    ValPolicyMask_t AssetPolicy = VAL_POLICY_TEMP_AUTH_STATE;
#endif
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AuthStateASId_p == NULL) ||
        (Nonce_p == NULL) ||
        (NonceLength_p == NULL) ||
        (*NonceLength_p < sizeof(t_res.Service.AuthUnlock.Nonce)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Get the AssetID of the authenticated unlock key
    funcres = val_AssetSearch(AuthKeyNumber, &KeyAssetId, NULL);
    if (funcres != VAL_SUCCESS)
    {
        goto error_func_exit;
    }

    // Allocate an Asset for the authenticated unlock state
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }
    funcres = val_AssetAlloc(AssetPolicy, 20,
                             false, false,
                             VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                             AuthStateASId_p);
    if (funcres != VAL_SUCCESS)
    {
        goto error_func_exit;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_AUTH_UNLOCK;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_AUNLOCKSTART;
    t_cmd.Service.AuthUnlock.AuthKeyAssetId = KeyAssetId;
    t_cmd.Service.AuthUnlock.AuthStateAssetId = *AuthStateASId_p;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    ZEROINIT(t_res.Service.AuthUnlock.Nonce);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            memcpy(Nonce_p, t_res.Service.AuthUnlock.Nonce, sizeof(t_res.Service.AuthUnlock.Nonce));
            *NonceLength_p = sizeof(t_res.Service.AuthUnlock.Nonce);
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;

            (void)val_AssetFree(*AuthStateASId_p);
            *AuthStateASId_p = VAL_ASSETID_INVALID;
        }
    }

error_func_exit:
    return funcres;
}
#endif /* !VAL_REMOVE_AUTH_UNLOCK */


/*----------------------------------------------------------------------------
 * val_AuthenticatedUnlockVerify
 */
#ifndef VAL_REMOVE_AUTH_UNLOCK
#ifdef EIP130_HW_V2
ValStatus_t
val_AuthenticatedUnlockVerify(
        const ValAssetId_t AuthStateASId,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * const Nonce_p,
        const ValSize_t NonceLength)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AuthStateASId == VAL_ASSETID_INVALID) ||
        (Signature_p == NULL) ||
        (Signature_p->Data_p == NULL) ||
        (Signature_p->ByteDataSize < VAL_ASYM_DATA_SIZE_B2B(1024)) ||
        (Signature_p->ByteDataSize > VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_RSA_MAX_BITS)) ||
        (Nonce_p == NULL) ||
        (NonceLength != sizeof(t_cmd.Service.AuthUnlock.Nonce)))
    {
        return VAL_BAD_ARGUMENT;
    }
#else
    IDENTIFIER_NOT_USED(NonceLength);
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_AUTH_UNLOCK;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_AUNLOCKVERIFY;
    t_cmd.Service.AuthUnlock.AuthStateAssetId = (uint32_t)AuthStateASId;
    t_cmd.Service.AuthUnlock.Sign_p = Signature_p->Data_p;
    t_cmd.Service.AuthUnlock.SignSize = Signature_p->ByteDataSize;
    memcpy(t_cmd.Service.AuthUnlock.Nonce, Nonce_p, sizeof(t_cmd.Service.AuthUnlock.Nonce));
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Copy possible service error
        funcres = (ValStatus_t)t_res.Result;
    }

    return funcres;
}
#else
ValStatus_t
val_AuthenticatedUnlockVerify(
        const ValAssetId_t AuthStateASId,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * Nonce_p,
        const ValSize_t NonceLength)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;
    uint8_t signbuffer[2*VAL_ASYM_DATA_SIZE_B2WB(521)];

#ifdef VAL_STRICT_ARGS
    if ((AuthStateASId == VAL_ASSETID_INVALID) ||
        (Signature_p == NULL) ||
        (Signature_p->r.Data_p == NULL) ||
        (Signature_p->s.Data_p == NULL) ||
        ((Signature_p->r.ByteDataSize != VAL_ASYM_DATA_SIZE_B2WB(256)) &&
         (Signature_p->r.ByteDataSize != VAL_ASYM_DATA_SIZE_B2WB(521))) ||
        ((Signature_p->s.ByteDataSize != VAL_ASYM_DATA_SIZE_B2WB(256)) &&
         (Signature_p->s.ByteDataSize != VAL_ASYM_DATA_SIZE_B2WB(521))) ||
        (Nonce_p == NULL) ||
        (NonceLength != sizeof(t_cmd.Service.AuthUnlock.Nonce)))
    {
        return VAL_BAD_ARGUMENT;
    }
#else
    IDENTIFIER_NOT_USED(NonceLength);
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_AUTH_UNLOCK;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_AUNLOCKVERIFY;
    t_cmd.Service.AuthUnlock.AuthStateAssetId = (uint32_t)AuthStateASId;
    memcpy(signbuffer, Signature_p->r.Data_p, Signature_p->r.ByteDataSize);
    memcpy(signbuffer+Signature_p->r.ByteDataSize, Signature_p->s.Data_p, Signature_p->s.ByteDataSize);
    t_cmd.Service.AuthUnlock.SignSize = 2 * Signature_p->r.ByteDataSize;
    t_cmd.Service.AuthUnlock.Sign_p = signbuffer;
    memcpy(t_cmd.Service.AuthUnlock.Nonce, Nonce_p, sizeof(t_cmd.Service.AuthUnlock.Nonce));
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Copy possible service error
        funcres = (ValStatus_t)t_res.Result;
    }

    return funcres;
}
#endif /* !EIP130_HW_V2 */
#endif /* !VAL_REMOVE_AUTH_UNLOCK */


/*----------------------------------------------------------------------------
 * val_SecureDebug
 */
#ifndef VAL_REMOVE_AUTH_UNLOCK
#ifndef VAL_REMOVE_SECURE_DEBUG
ValStatus_t
val_SecureDebug(
        const ValAssetId_t AuthStateASId,
        const bool fSet)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if (AuthStateASId == VAL_ASSETID_INVALID)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_AUTH_UNLOCK;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SETSECUREDEBUG;
    t_cmd.Service.SecureDebug.AuthStateAssetId = (uint32_t)AuthStateASId;
    t_cmd.Service.SecureDebug.fSet = fSet;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Copy possible service error
        funcres = (ValStatus_t)t_res.Result;
    }

    return funcres;
}
#endif /* !VAL_REMOVE_SECURE_DEBUG */
#endif /* !VAL_REMOVE_AUTH_UNLOCK */


/* end of file adapter_val_aunlock.c */
