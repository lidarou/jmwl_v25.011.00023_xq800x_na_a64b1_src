/* adapter_vex_emmc.c
 *
 * Implementation of the VEX API.
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

#include "c_adapter_vex.h"          // configuration

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_internal.h"   // API implementation
#include "adapter_bufmanager.h"     // BufManager_*()
#include "eip130_token_emmc.h"      // Eip130Token_Command_eMMC*()


/*----------------------------------------------------------------------------
 * vex_eMMC
 */
#ifdef VEX_ENABLE_EMMC
VexStatus_t
vex_eMMC(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    uint64_t InputDataAddr = 0;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;
    bool fWriteRequest = false;
    bool fVerify = false;

    // Format command token
    ZEROINIT(CommandToken);
    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_EMMC_RDREQ:
        Eip130Token_Command_eMMC_ReadRequest(
            &CommandToken,
            CommandToken_p->Service.eMMCRdRequest.AssetId);
        break;

    case VEXTOKEN_SUBCODE_EMMC_RDVER:
        Eip130Token_Command_eMMC_ReadVerify(&CommandToken);
        fVerify = true;
        break;

    case VEXTOKEN_SUBCODE_EMMC_RDWRCNTREQ:
        Eip130Token_Command_eMMC_ReadWriteCounterRequest(
            &CommandToken,
            CommandToken_p->Service.eMMCRdRequest.AssetId);
        break;

    case VEXTOKEN_SUBCODE_EMMC_RDWRCNTVER:
        Eip130Token_Command_eMMC_ReadWriteCounterVerify(&CommandToken);
        fVerify = true;
        break;

    case VEXTOKEN_SUBCODE_EMMC_WRREQ:
        Eip130Token_Command_eMMC_WriteRequest(&CommandToken);
        fWriteRequest = true;
        fVerify = true;
        break;

    case VEXTOKEN_SUBCODE_EMMC_WRVER:
        Eip130Token_Command_eMMC_WriteVerify(&CommandToken);
        fVerify = true;
        break;

    default:
        return VEX_UNSUPPORTED;
    }
    if (fVerify)
    {
        InputDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                       BUFMANAGER_BUFFERTYPE_IN,
                                       CommandToken_p->Service.eMMCRdVerifyWrReqVer.Data_p,
                                       CommandToken_p->Service.eMMCRdVerifyWrReqVer.DataSize,
                                       NULL);
        if (InputDataAddr == 0)
        {
            return VEX_NO_MEMORY;
        }

        Eip130Token_Command_eMMC_SetInputDataAndMACInfo(
            &CommandToken,
            InputDataAddr,
            CommandToken_p->Service.eMMCRdVerifyWrReqVer.DataSize);
        Eip130Token_Command_eMMC_SetStateAssetID(
            &CommandToken,
            CommandToken_p->Service.eMMCRdVerifyWrReqVer.StateAssetId);
        if (!fWriteRequest)
        {
            Eip130Token_Command_eMMC_CopyMAC(
                &CommandToken,
                CommandToken_p->Service.eMMCRdVerifyWrReqVer.Mac,
                CommandToken_p->Service.eMMCRdVerifyWrReqVer.MacSize);
        }
    }
    Eip130Token_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID(), false);

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if (ResultToken_p->Result >= 0)
        {
            // Success
            if (fVerify)
            {
                if (fWriteRequest)
                {
                    Eip130Token_Result_eMMC_CopyMAC(
                            &ResultToken,
                            32,
                            ResultToken_p->Service.eMMCRdVerifyWrReqVer.Mac);
                }
            }
            else
            {
                if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_EMMC_RDREQ)
                {
                    Eip130Token_Result_eMMC_ReadStateID(
                        &ResultToken,
                        &ResultToken_p->Service.eMMCRdRequest.AssetId);
                }
                else
                {
                    // Simply copy it from the command token
                    ResultToken_p->Service.eMMCRdRequest.AssetId =
                        CommandToken_p->Service.eMMCRdRequest.AssetId;
                }

                // Copy the Nonce from result token
                Eip130Token_Result_eMMC_ReadCopyNonce(
                    &ResultToken,
                    ResultToken_p->Service.eMMCRdRequest.Nonce);
            }
        }
    }

    // Release used buffers, if needed
    if (InputDataAddr != 0)
    {
        (void)BufManager_Unmap(InputDataAddr, false, false);
    }

    return funcres;
}
#endif

/* end of file adapter_vex_emmc.c */
