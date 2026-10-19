/* adapter_val_service.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the service services.
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

#include "api_val_service.h"            // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_ServiceRegisterRead
 */
#ifndef VAL_REMOVE_SERVICE_REGISTERREAD
ValStatus_t
val_ServiceRegisterRead(
        uint32_t         Address,
        uint32_t * const Value_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    if (Value_p == NULL)
    {
        return VAL_BAD_ARGUMENT;
    }

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_REGISTERREAD;
    t_cmd.Service.Register.Address = Address;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.Register.Value = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *Value_p = t_res.Service.Register.Value;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_SERVICE_REGISTERREAD */


/*----------------------------------------------------------------------------
 * val_ServiceRegisterWrite
 */
#ifndef VAL_REMOVE_SERVICE_REGISTERWRITE
ValStatus_t
val_ServiceRegisterWrite(
        uint32_t Address,
        uint32_t Value)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_REGISTERWRITE;
    t_cmd.Service.Register.Address = Address;
    t_cmd.Service.Register.Value = Value;
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
#endif /* !VAL_REMOVE_SERVICE_REGISTERWRITE */


/*----------------------------------------------------------------------------
 * val_ServiceZeroizeMailbox
 */
#ifndef VAL_REMOVE_SERVICE_ZEROIZEMAILBOX
ValStatus_t
val_ServiceZeroizeMailbox(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ZEROOUTMAILBOX;
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
#endif /* !VAL_REMOVE_SERVICE_ZEROIZEMAILBOX */


/*----------------------------------------------------------------------------
 * val_ServiceSelectOTPZeroize
 */
#ifndef VAL_REMOVE_SERVICE_ZEROIZEOTP
ValStatus_t
val_ServiceSelectOTPZeroize(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SELECTOTPZERO;
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
#endif /* !VAL_REMOVE_SERVICE_ZEROIZEOTP */


/*----------------------------------------------------------------------------
 * val_ServiceZeroizeOTP
 */
#ifndef VAL_REMOVE_SERVICE_ZEROIZEOTP
ValStatus_t
val_ServiceZeroizeOTP(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ZEROIZEOTP;
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
#endif /* !VAL_REMOVE_SERVICE_ZEROIZEOTP */


/*----------------------------------------------------------------------------
 * val_ServiceClockSwitch
 */
#ifndef VAL_REMOVE_CLOCKSWITCH
ValStatus_t
val_ServiceClockSwitch(
        uint16_t ClocksForcedOn,
        uint16_t ClocksForcedOff)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_CLOCKSWITCH;
    t_cmd.Service.ClockSwitch.On = ClocksForcedOn;
    t_cmd.Service.ClockSwitch.Off = ClocksForcedOff;
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
#endif /* !VAL_REMOVE_CLOCKSWITCH */


/*----------------------------------------------------------------------------
 * val_ServiceFirmwareCheck
 */
#ifndef VAL_REMOVE_FIRMWARECHECK
ValStatus_t
val_ServiceFirmwareCheck(
        const uint8_t * const FWImage_p,
        const uint32_t FWImageSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((FWImage_p == NULL) || (FWImageSize == 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_FIRMWARECHECK;
    t_cmd.Service.FirmwareCheck.FirmwareImage_p = FWImage_p;
    t_cmd.Service.FirmwareCheck.FirmwareImageSize = FWImageSize;
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
#endif /* VAL_REMOVE_FIRMWARECHECK */


/*----------------------------------------------------------------------------
 * val_ServiceUpdateRollbackID
 */
#ifndef VAL_REMOVE_UPDATEROLLBACKID
ValStatus_t
val_ServiceUpdateRollbackID(
        uint16_t RollbackID)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if (RollbackID > 32)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    t_cmd.OpCode = VEXTOKEN_OPCODE_SERVICE;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_UPDATEROLLBACKID;
    t_cmd.Service.UpdateRollbackID.RollbackID = RollbackID;
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
#endif /* VAL_REMOVE_UPDATEROLLBACKID */

/* end of file adapter_val_service.c */
