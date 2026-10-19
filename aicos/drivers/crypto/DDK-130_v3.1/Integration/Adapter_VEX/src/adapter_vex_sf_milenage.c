/* adapter_vex_sf_milenage.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the Special Functions services for Milenage.
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

#ifdef VEX_ENABLE_SF_MILENAGE

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_intern_sf.h"   // API implementation
#include "adapter_bufmanager.h"      // BufManager_*()
#include "eip130_token_sf_milenage.h" // Eip130Token_Command_SF_Milenage*()
#include "eip130_token_result.h"     // EIP130TOKEN_RESULT_VERIFY_ERROR


/*----------------------------------------------------------------------------
 * vex_SF_Milenage
 */
VexStatus_t
vex_SF_Milenage(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    bool fCopy = false;
    uint64_t DataBlobAddr = 0;
    uint16_t TokenID;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Format command token
    ZEROINIT(CommandToken);
    TokenID = vex_DeviceGetTokenID();

    switch (CommandToken_p->Service.SF_Milenage.Operation)
    {
    case VEXTOKEN_MILENAGE_OP_SQNADMINCREATE:
        if (CommandToken_p->Service.SF_Milenage.AssetId == 0)
        {
            // SQN Administration Create
            Eip130Token_Command_SF_MilenageSqnAdminCreate(&CommandToken,
                                                          CommandToken_p->Service.SF_Milenage.AssetNumber,
                                                          CommandToken_p->Service.SF_Milenage.fAMF_SBtest);
        }
        else
        {
            // SQN Administration Reset
            Eip130Token_Command_SF_MilenageSqnAdminReset(&CommandToken,
                                                         CommandToken_p->Service.SF_Milenage.AssetId);
        }
        break;

    case VEXTOKEN_MILENAGE_OP_AUTNVERIFICATION:
        if (CommandToken_p->Service.SF_Milenage.AssetId != 0)
        {
            Eip130Token_Command_SF_MilenageAutnVerificationSqn(&CommandToken,
                                                               CommandToken_p->Service.SF_Milenage.AssetId,
                                                               CommandToken_p->Service.SF_Milenage.RAND,
                                                               CommandToken_p->Service.SF_Milenage.AUTN);
        }
        else
        {
            Eip130Token_Command_SF_MilenageAutnVerification(&CommandToken,
                                                            CommandToken_p->Service.SF_Milenage.AssetNumber,
                                                            CommandToken_p->Service.SF_Milenage.RAND,
                                                            CommandToken_p->Service.SF_Milenage.AUTN);
        }
        break;

    case VEXTOKEN_MILENAGE_OP_AUTSGENERATION:
        // Note AMF is directly behind SQN
        Eip130Token_Command_SF_MilenageAutsGeneration(&CommandToken,
                                                      CommandToken_p->Service.SF_Milenage.AssetNumber,
                                                      CommandToken_p->Service.SF_Milenage.RAND,
                                                      CommandToken_p->Service.SF_Milenage.SQN);
        break;

    case VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT:
        DataBlobAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                      BUFMANAGER_BUFFERTYPE_OUT,
                                      CommandToken_p->Service.SF_MilenageExport.DataBlob_p,
                                      CommandToken_p->Service.SF_MilenageExport.DataBlobSize,
                                      (void *)&TokenID);
        if (DataBlobAddr == 0)
        {
            goto error_func_exit;
        }
        Eip130Token_Command_SF_MilenageSqnAdminExport(&CommandToken,
                                                      CommandToken_p->Service.SF_MilenageExport.AssetId,
                                                      CommandToken_p->Service.SF_MilenageExport.KekAssetId,
                                                      DataBlobAddr,
                                                      (uint32_t)BufManager_GetSize(DataBlobAddr),
                                                      CommandToken_p->Service.SF_MilenageExport.AssociatedData,
                                                      CommandToken_p->Service.SF_MilenageExport.AssociatedDataSize);
        break;

    case VEXTOKEN_MILENAGE_OP_CONFORMANCECHECK:
        // Note AMF is directly behind SQN
        Eip130Token_Command_SF_MilenageConformance(&CommandToken,
                                                   CommandToken_p->Service.SF_Milenage.RAND,
                                                   CommandToken_p->Service.SF_Milenage.SQN,
                                                   CommandToken_p->Service.SF_Milenage.K,
                                                   CommandToken_p->Service.SF_Milenage.OP);
        break;

    default:
        return VEX_UNSUPPORTED;
    }

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
            switch (CommandToken_p->Service.SF_Milenage.Operation)
            {
            default:
            case VEXTOKEN_MILENAGE_OP_SQNADMINCREATE:
                if (CommandToken_p->Service.SF_Milenage.AssetId == 0)
                {
                    Eip130Token_Result_SF_MilenageSqnAdminCreate(&ResultToken,
                                                                 &ResultToken_p->Service.SF_Milenage.AssetId);

                }
                break;

            case VEXTOKEN_MILENAGE_OP_AUTNVERIFICATION:
                ResultToken_p->Service.SF_Milenage.EMMCause = 0;
                Eip130Token_Result_SF_MilenageAutnVerification(&ResultToken,
                                                               ResultToken_p->Service.SF_Milenage.RES,
                                                               ResultToken_p->Service.SF_Milenage.CK,
                                                               ResultToken_p->Service.SF_Milenage.IK);
                if (CommandToken_p->Service.SF_Milenage.AssetId == 0)
                {
                    Eip130Token_Result_SF_MilenageAutnVerificationSQNAMF(&ResultToken,
                                                                         ResultToken_p->Service.SF_Milenage.SQN);
                }
                break;

            case VEXTOKEN_MILENAGE_OP_AUTSGENERATION:
                Eip130Token_Result_SF_MilenageAuts(&ResultToken,
                                                   ResultToken_p->Service.SF_Milenage.AUTS);
                break;

            case VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT:
                Eip130Token_Result_SF_MilenageSqnAdminExport(&ResultToken,
                                                             &ResultToken_p->Service.SF_MilenageExport.DataBlobSize);
                // Copy output data
                fCopy = true;
                break;

            case VEXTOKEN_MILENAGE_OP_CONFORMANCECHECK:
                Eip130Token_Result_SF_MilenageConformance(&ResultToken,
                                                          ResultToken_p->Service.SF_Milenage.RES,
                                                          ResultToken_p->Service.SF_Milenage.CK,
                                                          ResultToken_p->Service.SF_Milenage.IK,
                                                          ResultToken_p->Service.SF_Milenage.MACA,
                                                          ResultToken_p->Service.SF_Milenage.MACS,
                                                          ResultToken_p->Service.SF_Milenage.AK,
                                                          ResultToken_p->Service.SF_Milenage.AKstar,
                                                          ResultToken_p->Service.SF_Milenage.OPc);
                break;
            }
        }
        else
        {
            switch (CommandToken_p->Service.SF_Milenage.Operation)
            {
            case VEXTOKEN_MILENAGE_OP_AUTNVERIFICATION:
                if (ResultToken_p->Result == EIP130TOKEN_RESULT_VERIFY_ERROR)
                {
                    Eip130Token_Result_SF_MilenageAutnVerificationEMMCause(&ResultToken,
                                                                           &ResultToken_p->Service.SF_Milenage.EMMCause);
                    if (ResultToken_p->Service.SF_Milenage.EMMCause == 21)
                    {
                        Eip130Token_Result_SF_MilenageAuts(&ResultToken,
                                                           ResultToken_p->Service.SF_Milenage.AUTS);
                    }
                    else
                    {
                        memset(ResultToken_p->Service.SF_Milenage.AUTS, 0, sizeof(ResultToken_p->Service.SF_Milenage.AUTS));
                    }
                }
                break;

            default:
            case VEXTOKEN_MILENAGE_OP_SQNADMINCREATE:
            case VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT:
            case VEXTOKEN_MILENAGE_OP_CONFORMANCECHECK:
            case VEXTOKEN_MILENAGE_OP_AUTSGENERATION:
                break;
            }
        }
    }

error_func_exit:
    // Release used buffers, if needed
    if (DataBlobAddr != 0)
    {
        int rc = BufManager_Unmap(DataBlobAddr, true, fCopy);
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

#endif /* VEX_ENABLE_SF_MILENAGE */

/* end of file adapter_vex_sf_milenage.c */
