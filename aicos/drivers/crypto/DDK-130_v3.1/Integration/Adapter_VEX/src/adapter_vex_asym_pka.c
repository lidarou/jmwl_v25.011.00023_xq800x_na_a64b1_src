/* adapter_vex_asym_pka.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the asymmetric crypto services for direct PKA/PKCP use.
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

#include "c_adapter_vex.h"              // configuration

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_intern_asym.h"   // API implementation
#include "adapter_bufmanager.h"        // BufManager_*()
#include "eip130_token_pk.h"           // Eip130Token_Command_Pk_Asset*()


/*----------------------------------------------------------------------------
 * vex_Asym_PkaNumSet
 */
VexStatus_t
vex_Asym_PkaNumSet(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Claim(&CommandToken,
                                 CommandToken_p->Service.PkOperation.Nwords,
                                 CommandToken_p->Service.PkOperation.Mwords,
                                 CommandToken_p->Service.PkOperation.Mmask);
    Vex_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID());

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
    }

    return funcres;
}


/*----------------------------------------------------------------------------
 * vex_Asym_PkaNumLoad
 */
VexStatus_t
vex_Asym_PkaNumLoad(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    uint64_t VectorAddr = 0;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Get Vector input address for token
    VectorAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                BUFMANAGER_BUFFERTYPE_IN,
                                CommandToken_p->Service.PkOperation.InData_p,
                                CommandToken_p->Service.PkOperation.InDataSize,
                                NULL);
    if (VectorAddr == 0)
    {
        return VEX_NO_MEMORY;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_NumLoad(&CommandToken,
                                   CommandToken_p->Service.PkOperation.Index,
                                   VectorAddr,
                                   CommandToken_p->Service.PkOperation.InDataSize);
    Vex_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID());

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
    }

    // Release used buffer
    (void)BufManager_Unmap(VectorAddr, false, false);

    return funcres;
}


/*----------------------------------------------------------------------------
 * vex_Asym_PkaOperation
 */
VexStatus_t
vex_Asym_PkaOperation(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint64_t InputAddr = 0;
    uint64_t OutputAddr = 0;
    size_t InputSize = 0;
    size_t OutputSize = 0;
    uint16_t TokenID;
    bool fCopy = false;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Get Vector input address for token
    if (CommandToken_p->Service.PkOperation.InData_p != NULL)
    {
        InputSize = CommandToken_p->Service.PkOperation.InDataSize;
        InputAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                   BUFMANAGER_BUFFERTYPE_IN,
                                   CommandToken_p->Service.PkOperation.InData_p,
                                   InputSize,
                                   NULL);
        if (InputAddr == 0)
        {
            goto error_func_exit;
        }
    }

    TokenID = vex_DeviceGetTokenID();

    if (CommandToken_p->Service.PkOperation.OutData_p != NULL)
    {
        OutputAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                    BUFMANAGER_BUFFERTYPE_OUT,
                                    CommandToken_p->Service.PkOperation.OutData_p,
                                    CommandToken_p->Service.PkOperation.OutDataSize,
                                    (void *)&TokenID);
        if (OutputAddr == 0)
        {
            goto error_func_exit;
        }
        OutputSize = BufManager_GetSize(OutputAddr);
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_Pk_Operation(&CommandToken,
                                     (uint8_t)CommandToken_p->Service.PkOperation.Operation,
                                     CommandToken_p->Service.PkOperation.PublicExponent,
                                     InputAddr, (uint32_t)InputSize,
                                     OutputAddr, (uint32_t)OutputSize);
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
        }
    }

error_func_exit:
    // Release used buffer
    if (InputAddr != 0)
    {
        (void)BufManager_Unmap(InputAddr, false, false);
    }

    if (OutputAddr != 0)
    {
        int rc = BufManager_Unmap(OutputAddr, true, fCopy);
        if (rc != 0)
        {
            if (rc == -3)
            {
                funcres = VEX_DATA_TIMEOUT;
            }
            else
            {
                funcres = VEX_INTERNAL_ERROR;
            }
        }
    }

    return funcres;
}



/* end of file adapter_vex_asym_pka.c */
