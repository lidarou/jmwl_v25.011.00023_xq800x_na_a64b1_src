/* adapter_val_encryptvector.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the symmetric crypto encrypted vector for PKI services.
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

#ifndef VAL_REMOVE_ENCRYPTED_VECTOR

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_symkeywrap.h"         // the API to implement
#include "api_val_sym.h"                // VAL_SYM_ALGO_AES_BLOCK_SIZE
#include "api_val_asset.h"              // Asset Management related information
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_EncryptVectorForPKI
 */
ValStatus_t
val_EncryptVectorForPKI(
        const ValAssetId_t AssetId,
        const ValAssetNumber_t AssetNumber,
        ValOctetsOut_t * const EncVectorData_p,
        ValSize_t * const EncVectorDataSize_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((AssetNumber < 37) || (AssetNumber > 40) ||
        (AssetId == VAL_ASSETID_INVALID) ||
        (EncVectorDataSize_p == NULL) || (*EncVectorDataSize_p > 512) ||
        ((*EncVectorDataSize_p != 0) && (EncVectorData_p == NULL)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SYMWRAP;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_ENCRYPTVECTOR;
    t_cmd.Service.EncVector.AssetId = (uint32_t)AssetId;
    t_cmd.Service.EncVector.AssetNumber = AssetNumber;
    t_cmd.Service.EncVector.Data_p = (uint8_t *)EncVectorData_p;
    t_cmd.Service.EncVector.DataSize = (uint32_t)*EncVectorDataSize_p;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.EncVector.DataSize = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        funcres = (ValStatus_t)t_res.Result;
        if ((funcres == VAL_SUCCESS) ||
            (funcres == VAL_INVALID_LENGTH))
        {
            // Copy real/desired size
            *EncVectorDataSize_p = t_res.Service.KeyWrap.Size;
       }
        else
        {
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}


#endif /* !VAL_REMOVE_ENCRYPTED_VECTOR */


/* end of file adapter_val_encryptvector.c */
