/* adapter_val_securetimer.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the Secure Timer services.
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

#include "api_val_securetimer.h"        // the API to implement
#include "api_val_asset.h"              // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_SecureTimerStart
 */
#ifndef VAL_REMOVE_SECURETIMER
ValStatus_t
val_SecureTimerStart(
        const bool fSecond,
        ValAssetId_t * const AssetId_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if (AssetId_p == NULL)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SECURETIMER;
    t_cmd.Service.SecureTimer.Operation = VEXTOKEN_SECURETIMER_START;
    t_cmd.Service.SecureTimer.AssetId = (uint32_t)*AssetId_p;
    t_cmd.Service.SecureTimer.fSecond = fSecond;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.SecureTimer.AssetId = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *AssetId_p = (ValAssetId_t)t_res.Service.SecureTimer.AssetId;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_SECURETIMER */


/*----------------------------------------------------------------------------
 * val_SecureTimerStop
 */
#ifndef VAL_REMOVE_SECURETIMER
ValStatus_t
val_SecureTimerStop(
        const ValAssetId_t AssetId,
        uint32_t * const ElapsedTime_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AssetId == VAL_ASSETID_INVALID) ||
        (ElapsedTime_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SECURETIMER;
    t_cmd.Service.SecureTimer.Operation = VEXTOKEN_SECURETIMER_STOP;
    t_cmd.Service.SecureTimer.fSecond = false; // not used
    t_cmd.Service.SecureTimer.AssetId = (uint32_t)AssetId;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.SecureTimer.ElapsedTime = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if(t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *ElapsedTime_p = (uint32_t)t_res.Service.SecureTimer.ElapsedTime;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_SECURETIMER */


/*----------------------------------------------------------------------------
 * val_SecureTimerRead
 */
#ifndef VAL_REMOVE_SECURETIMER
ValStatus_t
val_SecureTimerRead(
        const ValAssetId_t AssetId,
        uint32_t * const ElapsedTime_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AssetId == VAL_ASSETID_INVALID) ||
        (ElapsedTime_p == NULL))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_ASSETMANAGEMENT;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SECURETIMER;
    t_cmd.Service.SecureTimer.Operation = VEXTOKEN_SECURETIMER_READ;
    t_cmd.Service.SecureTimer.fSecond = false; // not used
    t_cmd.Service.SecureTimer.AssetId = (uint32_t)AssetId;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.SecureTimer.ElapsedTime = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if(t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *ElapsedTime_p = (uint32_t)t_res.Service.SecureTimer.ElapsedTime;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_SECURETIMER */


/* end of file adapter_val_securetimer.c */
