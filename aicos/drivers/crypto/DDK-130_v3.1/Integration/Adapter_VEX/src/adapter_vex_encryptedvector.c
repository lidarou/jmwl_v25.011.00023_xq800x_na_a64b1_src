/* adapter_vex_encryptedvector.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the encrypted vector for PKI services.
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

#include "c_adapter_vex.h"          // configuration

#ifdef VEX_ENABLE_ENCRYPTED_VECTOR

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_internal.h"   // API implementation
#include "adapter_bufmanager.h"     // BufManager_*()
#include "eip130_token_wrap.h"    // Eip130Token_Command_Crypto*()


/*----------------------------------------------------------------------------
 * vex_EncryptedVector
 */
VexStatus_t
vex_EncryptedVector(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint16_t TokenID;
    uint64_t DataAddr = 0;
    bool fCopy = false;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Get output address for token
    TokenID = vex_DeviceGetTokenID();
    DataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                              BUFMANAGER_BUFFERTYPE_OUT,
                              CommandToken_p->Service.EncVector.Data_p,
                              CommandToken_p->Service.EncVector.DataSize,
                              (void *)&TokenID);
    if (DataAddr == 0)
    {
        return VEX_INTERNAL_ERROR;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_EncryptedVectorPKI(&CommandToken,
                                           CommandToken_p->Service.EncVector.AssetId,
                                           CommandToken_p->Service.EncVector.AssetNumber,
                                           DataAddr,
                                           CommandToken_p->Service.EncVector.DataSize);
    Vex_Command_SetTokenID(&CommandToken, TokenID);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if (ResultToken_p->Result >= 0)
        {
            // Copy output data
            fCopy = true;

            Eip130Token_Result_EncryptedVectorPKI_GetDataLength(
                &ResultToken,
                &ResultToken_p->Service.EncVector.DataSize);
        }
        else if (ResultToken_p->Result == VEXTOKEN_RESULT_SEQ_INVALID_LENGTH)
        {
            Eip130Token_Result_EncryptedVectorPKI_GetDataLength(
                &ResultToken,
                &ResultToken_p->Service.EncVector.DataSize);
        }
    }

    // Release used buffer, if needed
    if (BufManager_Unmap(DataAddr, true, fCopy) != 0)
    {
        funcres = VEX_INTERNAL_ERROR;
    }

    return funcres;
}

#endif /* VEX_ENABLE_ENCRYPTED_VECTOR */

/* end of file adapter_vex_encryptedvector.c */
