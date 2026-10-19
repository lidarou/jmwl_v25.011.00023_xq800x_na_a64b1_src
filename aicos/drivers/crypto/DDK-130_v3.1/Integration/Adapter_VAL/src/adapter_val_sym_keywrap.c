/* adapter_val_sym_keywrap.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the symmetric crypto key wrap services.
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

#ifndef VAL_REMOVE_SYM_KEYWRAP

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_symkeywrap.h"         // the API to implement
#include "api_val_sym.h"                // VAL_SYM_ALGO_AES_BLOCK_SIZE
#include "api_val_asset.h"              // Asset Management related information
#include "api_val_system.h"             // val_IsAccessSecure()
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


#define VAL_KEYWRAP_IV_SIZE         (64/8)
#define VAL_KEYWRAP_DATA_SIZE(x)    (((x) + VAL_KEYWRAP_IV_SIZE - 1) & (unsigned int)~(VAL_KEYWRAP_IV_SIZE - 1))
#define VAL_KEYWRAP_ENCDATA_SIZE(x) (VAL_KEYWRAP_DATA_SIZE(x) + VAL_KEYWRAP_IV_SIZE)
#define VAL_KEYWRAP_DECDATA_SIZE(x) (VAL_KEYWRAP_DATA_SIZE(x) - VAL_KEYWRAP_IV_SIZE)


/*----------------------------------------------------------------------------
 * valLocal_SymKeyWrap
 */
static ValStatus_t
valLocal_SymKeyWrap(
        bool fWrap,
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        uint8_t Algorithm)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_SYMWRAP;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_SYMKEYWRAP;
    t_cmd.Service.KeyWrap.fWrap = (uint8_t)fWrap;
    t_cmd.Service.KeyWrap.KeySize = (uint32_t)KeySize;
    t_cmd.Service.KeyWrap.KeyAssetId = (uint32_t)KeyAssetId;
    if (KeyAssetId == VAL_ASSETID_INVALID)
    {
        if (KeySize > sizeof(t_cmd.Service.KeyWrap.Key))
        {
            return VAL_BAD_ARGUMENT;
        }
        memcpy(t_cmd.Service.KeyWrap.Key, Key_p, KeySize);
    }

    if (Algorithm == VAL_SYM_ALGO_CIPHER_AES)
    {
        t_cmd.Service.KeyWrap.Algorithm = VEXTOKEN_ALGO_CIPHER_AES;
    }
    else if (Algorithm == VAL_SYM_ALGO_CIPHER_SM4)
    {
        t_cmd.Service.KeyWrap.Algorithm = VEXTOKEN_ALGO_CIPHER_SM4;
    }
    else
    {
        return VAL_BAD_ARGUMENT;
    }

    t_cmd.Service.KeyWrap.SrcData_p = (const uint8_t *)SrcData_p;
    t_cmd.Service.KeyWrap.SrcDataSize = (uint32_t)SrcDataSize;
    t_cmd.Service.KeyWrap.DstData_p = (uint8_t *)DstData_p;
    t_cmd.Service.KeyWrap.DstDataSize = (uint32_t)*DstDataSize_p;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;
    t_res.Service.KeyWrap.Size = 0;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        // Check for errors
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            // Copy real size
            *DstDataSize_p = t_res.Service.KeyWrap.Size;
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
 * val_SymKeyWrap
 */
ValStatus_t
val_SymKeyWrap(
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        uint8_t Algorithm)
{
#ifdef VAL_STRICT_ARGS
    if (((KeySize != (128/8)) && (KeySize != (192/8)) && (KeySize != (256/8))) ||
        ((KeyAssetId == VAL_ASSETID_INVALID) && (Key_p == NULL)) ||
        (SrcData_p == NULL) ||
        (DstDataSize_p == NULL) ||
        (SrcDataSize == 0) || (SrcDataSize > 1024))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    if ((DstData_p == NULL) ||
        (*DstDataSize_p < VAL_KEYWRAP_ENCDATA_SIZE(SrcDataSize)))
    {
        *DstDataSize_p = VAL_KEYWRAP_ENCDATA_SIZE(SrcDataSize);
        return VAL_BUFFER_TOO_SMALL;
    }

    return valLocal_SymKeyWrap(true,
                               KeyAssetId, Key_p, KeySize,
                               SrcData_p, SrcDataSize,
                               DstData_p, DstDataSize_p,
                               Algorithm);
}


/*----------------------------------------------------------------------------
 * val_SymKeyUnwrap
 */
ValStatus_t
val_SymKeyUnwrap(
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        uint8_t Algorithm)
{
    ValSize_t OutputSize;

#ifdef VAL_STRICT_ARGS
    if (((KeySize != (128/8)) && (KeySize != (192/8)) && (KeySize != (256/8))) ||
        ((KeyAssetId == VAL_ASSETID_INVALID) && (Key_p == NULL)) ||
        (SrcData_p == NULL) ||
        (DstDataSize_p == NULL) ||
        (SrcDataSize < (VAL_KEYWRAP_IV_SIZE + VAL_SYM_ALGO_AES_BLOCK_SIZE)) ||
        (SrcDataSize != VAL_KEYWRAP_DATA_SIZE(SrcDataSize)))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    OutputSize = VAL_KEYWRAP_DECDATA_SIZE(SrcDataSize);
    if ((DstData_p == NULL) ||
        (OutputSize > *DstDataSize_p))
    {
        *DstDataSize_p = OutputSize;
        return VAL_BUFFER_TOO_SMALL;
    }
    *DstDataSize_p = OutputSize;

    return valLocal_SymKeyWrap(false,
                               KeyAssetId, Key_p, KeySize,
                               SrcData_p, SrcDataSize,
                               DstData_p, DstDataSize_p,
                               Algorithm);
}


#endif /* !VAL_REMOVE_SYM_KEYWRAP */


/* end of file adapter_val_sym_keywrap.c */
