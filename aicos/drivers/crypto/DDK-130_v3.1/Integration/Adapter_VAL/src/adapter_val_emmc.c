/* adapter_val_emmc.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the eMMC related services.
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

#include "api_val_emmc.h"               // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_eMMCReadRequest
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCReadRequest(
        const ValAssetId_t KeyAssetId,
        ValAssetId_t * const StateASId_p,
        ValOctetsOut_t * const Nonce_p,
        ValSize_t * const NonceLength_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((KeyAssetId == VAL_ASSETID_INVALID) ||
        (StateASId_p == NULL) ||
        (Nonce_p == NULL) ||
        (NonceLength_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (*NonceLength_p < 16)
    {
        *NonceLength_p = 16;
        return VAL_BUFFER_TOO_SMALL;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_RDREQ;
    t_cmd.Service.eMMCRdRequest.AssetId = KeyAssetId;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.eMMCRdRequest.AssetId = 0;
    ZEROINIT(t_res.Service.eMMCRdRequest.Nonce);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *StateASId_p = t_res.Service.eMMCRdRequest.AssetId;

            // Copy Nonce
            memcpy(Nonce_p, t_res.Service.eMMCRdRequest.Nonce, 16);
            *NonceLength_p = 16;
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


/*----------------------------------------------------------------------------
 * val_eMMCReadVerify
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCReadVerify(
        const ValAssetId_t StateASId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsIn_t * const MAC_p,
        const ValSize_t MACSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StateASId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (MAC_p == NULL) ||
        (MACSize != (256/8)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_RDVER;
    t_cmd.Service.eMMCRdVerifyWrReqVer.StateAssetId = StateASId;
    t_cmd.Service.eMMCRdVerifyWrReqVer.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.eMMCRdVerifyWrReqVer.DataSize = (uint32_t)DataSize;
    if (MACSize > sizeof(t_cmd.Service.eMMCRdVerifyWrReqVer.Mac))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.eMMCRdVerifyWrReqVer.Mac, MAC_p, MACSize);
    t_cmd.Service.eMMCRdVerifyWrReqVer.MacSize = (uint32_t)MACSize;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_eMMCReadWriteCounterRequest
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCReadWriteCounterRequest(
        const ValAssetId_t StateASId,
        ValOctetsOut_t * const Nonce_p,
        ValSize_t * const NonceLength_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StateASId == VAL_ASSETID_INVALID) ||
        (Nonce_p == NULL) ||
        (NonceLength_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (*NonceLength_p < 16)
    {
        *NonceLength_p = 16;
        return VAL_BUFFER_TOO_SMALL;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_RDWRCNTREQ;
    t_cmd.Service.eMMCRdRequest.AssetId = StateASId;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    ZEROINIT(t_res.Service.eMMCRdRequest.Nonce);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Copy Nonce
            memcpy(Nonce_p, t_res.Service.eMMCRdRequest.Nonce, 16);
            *NonceLength_p = 16;
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


/*----------------------------------------------------------------------------
 * val_eMMCReadWriteCounterVerify
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCReadWriteCounterVerify(
        const ValAssetId_t StateASId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsIn_t * const MAC_p,
        const ValSize_t MACSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StateASId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (MAC_p == NULL) ||
        (MACSize != (256/8)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_RDWRCNTVER;
    t_cmd.Service.eMMCRdVerifyWrReqVer.StateAssetId = StateASId;
    t_cmd.Service.eMMCRdVerifyWrReqVer.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.eMMCRdVerifyWrReqVer.DataSize = (uint32_t)DataSize;
    if (MACSize > sizeof(t_cmd.Service.eMMCRdVerifyWrReqVer.Mac))
    {
        return VAL_BAD_ARGUMENT;
    }
    memcpy(t_cmd.Service.eMMCRdVerifyWrReqVer.Mac, MAC_p, MACSize);
    t_cmd.Service.eMMCRdVerifyWrReqVer.MacSize = (uint32_t)MACSize;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    return funcres;
}
#endif


/*----------------------------------------------------------------------------
 * val_eMMCWriteRequest
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCWriteRequest(
        const ValAssetId_t StateASId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsOut_t * const MAC_p,
        ValSize_t * const MACSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StateASId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (MAC_p == NULL) ||
        (MACSize_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    if (*MACSize_p < (256/8))
    {
        *MACSize_p = (256/8);
        return VAL_BUFFER_TOO_SMALL;
    }

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_WRREQ;
    t_cmd.Service.eMMCRdVerifyWrReqVer.StateAssetId = StateASId;
    t_cmd.Service.eMMCRdVerifyWrReqVer.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.eMMCRdVerifyWrReqVer.DataSize = (uint32_t)DataSize;
    t_cmd.Service.eMMCRdVerifyWrReqVer.MacSize = (uint32_t)*MACSize_p;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    ZEROINIT(t_res.Service.eMMCRdVerifyWrReqVer.Mac);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Copy generated MAC value
            memcpy(MAC_p, t_res.Service.eMMCRdVerifyWrReqVer.Mac, (256/8));
            *MACSize_p = (256/8);
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


/*----------------------------------------------------------------------------
 * val_eMMCWriteVerify
 */
#ifndef VAL_REMOVE_EMMC
ValStatus_t
val_eMMCWriteVerify(
        const ValAssetId_t StateASId,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsIn_t * const MAC_p,
        const ValSize_t MACSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((StateASId == VAL_ASSETID_INVALID) ||
        (Data_p == NULL) ||
        (DataSize == 0) ||
        (MAC_p == NULL) ||
        (MACSize != (256/8)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_EMMC;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_EMMC_WRVER;
    t_cmd.Service.eMMCRdVerifyWrReqVer.StateAssetId = StateASId;
    t_cmd.Service.eMMCRdVerifyWrReqVer.Data_p = (const uint8_t *)Data_p;
    t_cmd.Service.eMMCRdVerifyWrReqVer.DataSize = (uint32_t)DataSize;
    memcpy(t_cmd.Service.eMMCRdVerifyWrReqVer.Mac, MAC_p, MACSize);
    t_cmd.Service.eMMCRdVerifyWrReqVer.MacSize = (uint32_t)MACSize;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        funcres = (ValStatus_t)t_res.Result;
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
    }

    return funcres;
}
#endif


/* end of file adapter_val_emmc.c */
