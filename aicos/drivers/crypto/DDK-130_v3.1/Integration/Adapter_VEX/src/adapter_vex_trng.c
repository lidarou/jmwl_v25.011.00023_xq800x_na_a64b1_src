/* adapter_vex_trng.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the TRNG services.
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
#include "eip130_token_random.h"    // Eip130Token_Command_RandomNumber_Generate()
                                    // Eip130Token_Command_TRNG_Configure()
                                    // Eip130Token_Command_PRNG_ReseedNow()
                                    // Eip130Token_Command_DRBG_ForceSeed()


/*----------------------------------------------------------------------------
 * vexLocal_TrngRandom
 */
static VexStatus_t
vexLocal_TrngRandom(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p,
        uint16_t RawKey)
{
    VexStatus_t funcres;
    uint16_t TokenID;
    uint16_t Size;
    uint64_t DataAddr = 0;
    bool fCopy = false;
    int rc;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if ((RawKey != 0) && ((CommandToken_p->Service.Random.Size % 256) != 0))
    {
        return VEX_INVALID_LENGTH;
    }

    // Random output via DMA and setup DMA OutputAddress in token.
    TokenID = vex_DeviceGetTokenID();

    if (RawKey == 0)
    {
        Size = CommandToken_p->Service.Random.Size;

        DataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_OUT,
                                  CommandToken_p->Service.Random.Data_p,
                                  CommandToken_p->Service.Random.Size,
                                  (void *)&TokenID);
    }
    else
    {
        Size = (CommandToken_p->Service.Random.Size / 256);

        // Note: The TRNG Raw data dump does not respect the write
        //       TokenID after DMA data write
        DataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                  BUFMANAGER_BUFFERTYPE_OUT,
                                  CommandToken_p->Service.Random.Data_p,
                                  CommandToken_p->Service.Random.Size,
                                  NULL/*(void *)&TokenID*/);
    }
    if (DataAddr == 0)
    {
        return VEX_NO_MEMORY;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_RandomNumber_Generate(&CommandToken, Size, DataAddr);
    if (RawKey != 0)
    {
        Eip130Token_Command_RandomNumber_SetRawKey(&CommandToken, RawKey);
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
            // Copy output data
            fCopy = true;
        }
    }

    // Release used buffer, if needed
    rc = BufManager_Unmap(DataAddr, true, fCopy);
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

    return funcres;
}


/*----------------------------------------------------------------------------
 * vexLocal_TrngConfig
 */
static VexStatus_t
vexLocal_TrngConfig(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Format command token
    ZEROINIT(CommandToken);
#ifdef VEX_ENABLE_DRBG_FORCESEED
    if(CommandToken_p->Service.TrngConfig.fDRBGForceSeed)
    {
        Eip130Token_Command_DRBG_ForceSeed(&CommandToken,
                                            CommandToken_p->Service.TrngConfig.AutoSeed,
                                            CommandToken_p->Service.TrngConfig.Seed);
    }
    else
#endif
    if (CommandToken_p->Service.TrngConfig.fReseed &&
        !CommandToken_p->Service.TrngConfig.fLoadStart)
    {
        // Only reseed
        Eip130Token_Command_PRNG_ReseedNow(&CommandToken);
    }
    else
    {
        // Configure
#ifdef EIP130_ENABLE_TRNG_CUTOFF
        Eip130Token_Command_TRNG_Configure(
            &CommandToken,
            CommandToken_p->Service.TrngConfig.AutoSeed,
            CommandToken_p->Service.TrngConfig.SampleCycles,
            CommandToken_p->Service.TrngConfig.SampleDiv,
            CommandToken_p->Service.TrngConfig.Scale,
            CommandToken_p->Service.TrngConfig.NoiseBlocks,
            CommandToken_p->Service.TrngConfig.RepCntCutoff,
            CommandToken_p->Service.TrngConfig.AdaptProp64Cutoff,
            CommandToken_p->Service.TrngConfig.AdaptProp512Cutoff);
#else
        Eip130Token_Command_TRNG_Configure(
            &CommandToken,
            CommandToken_p->Service.TrngConfig.AutoSeed,
            CommandToken_p->Service.TrngConfig.SampleCycles,
            CommandToken_p->Service.TrngConfig.SampleDiv,
            CommandToken_p->Service.TrngConfig.Scale,
            CommandToken_p->Service.TrngConfig.NoiseBlocks);
#endif

        if (CommandToken_p->Service.TrngConfig.fReseed)
        {
            // RRD = Reseed post-processor
            CommandToken.W[2] |= BIT_1;
        }
#ifdef VEX_ENABLE_ARIA_DRBG
        if(CommandToken_p->Service.TrngConfig.fAria)
        {
            Eip130Token_Command_TRNG_Configure_AriaDRBG(&CommandToken);
        }
#endif
    }

    Eip130Token_Command_SetTokenID(&CommandToken,
                                   vex_DeviceGetTokenID(),
                                   false);

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
 * vexLocal_VerifyDrbg
 */
static VexStatus_t
vexLocal_VerifyDrbg(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_NO_MEMORY;
    uint16_t TokenID;
    uint64_t InputDataAddr = 0;
    uint64_t OutputDataAddr = 0;
    bool fCopy = false;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    if (CommandToken_p->Service.DrbgPP.InputData_p != NULL)
    {
        // Convert input buffer and get address for token
        InputDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                       BUFMANAGER_BUFFERTYPE_IN,
                                       CommandToken_p->Service.DrbgPP.InputData_p,
                                       CommandToken_p->Service.DrbgPP.InputDataSize,
                                       NULL);
        if (InputDataAddr == 0)
        {
            goto error_func_exit;
        }
    }

    // Get output address for token
    // Note: The TRNG Post-Processing Verification does not respect the write
    //       TokenID after DMA data write
    TokenID = vex_DeviceGetTokenID();
    OutputDataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                                    BUFMANAGER_BUFFERTYPE_OUT,
                                    CommandToken_p->Service.DrbgPP.OutputData_p,
                                    CommandToken_p->Service.DrbgPP.OutputDataSize,
                                    NULL/*(void *)&TokenID*/);
    if (OutputDataAddr == 0)
    {
        goto error_func_exit;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_TRNG_PP_Verification(&CommandToken,
                                             CommandToken_p->Service.DrbgPP.Test,
                                             CommandToken_p->Service.DrbgPP.Pattern,
                                             CommandToken_p->Service.DrbgPP.Size,
                                             InputDataAddr,
                                             OutputDataAddr);
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
    // Release used buffers, if needed
    if (InputDataAddr != 0)
    {
        (void)BufManager_Unmap(InputDataAddr, false, false);
    }
    if (OutputDataAddr != 0)
    {
        int rc = BufManager_Unmap(OutputDataAddr, true, fCopy);
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


/*----------------------------------------------------------------------------
 * vexLocal_VerifyNrbg
 */
static VexStatus_t
vexLocal_VerifyNrbg(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    uint64_t DataAddr = 0;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;

    // Convert input buffer and get address for token
    DataAddr = BufManager_Map(CommandToken_p->fFromUserSpace,
                              BUFMANAGER_BUFFERTYPE_IN,
                              CommandToken_p->Service.NrbgVerify.InputData_p,
                              CommandToken_p->Service.NrbgVerify.Size,
                              NULL);
    if (DataAddr == 0)
    {
        return VEX_NO_MEMORY;
    }

    // Format command token
    ZEROINIT(CommandToken);
    Eip130Token_Command_TRNG_HW_SelfTest(&CommandToken,
                                         DataAddr,
                                         CommandToken_p->Service.NrbgVerify.Size,
                                         CommandToken_p->Service.NrbgVerify.RepCntCutoff,
                                         CommandToken_p->Service.NrbgVerify.RepCntCount,
                                         CommandToken_p->Service.NrbgVerify.RepCntData,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp64Cutoff,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp64Count,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp64Data,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp512Cutoff,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp512Count,
                                         CommandToken_p->Service.NrbgVerify.AdaptProp512Data);
    Vex_Command_SetTokenID(&CommandToken, vex_DeviceGetTokenID());

    // Initialize result token
    ZEROINIT(ResultToken);

    // Exchange token with the EIP-13x HW
    funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
    if (funcres == VEX_SUCCESS)
    {
        ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
        if (ResultToken_p->Result >= 0)
        {
            Eip130Token_Result_TRNG_HW_SelfTest(&ResultToken,
                                                &ResultToken_p->Service.NrbgVerify.RepCntCutoff,
                                                &ResultToken_p->Service.NrbgVerify.RepCntCount,
                                                &ResultToken_p->Service.NrbgVerify.RepCntData,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp64Cutoff,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp64Count,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp64Data,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp64Fail,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp512Cutoff,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp512Count,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp512Data,
                                                &ResultToken_p->Service.NrbgVerify.AdaptProp512Fail);
        }
    }

    // Release used buffer, if needed
    (void)BufManager_Unmap(DataAddr, false, false);

    return funcres;
}


/*----------------------------------------------------------------------------
 * vex_Trng
 */
VexStatus_t
vex_Trng(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_UNSUPPORTED;

    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_RANDOMNUMBER:
        funcres = vexLocal_TrngRandom(CommandToken_p, ResultToken_p, 0);
        break;

    case VEXTOKEN_SUBCODE_TRNGRAWDATA:
        funcres = vexLocal_TrngRandom(CommandToken_p, ResultToken_p, 0x5244);
        break;

    case VEXTOKEN_SUBCODE_TRNGCONFIG:
        funcres = vexLocal_TrngConfig(CommandToken_p, ResultToken_p);
        break;

    case VEXTOKEN_SUBCODE_VERIFYDRBG:
        funcres = vexLocal_VerifyDrbg(CommandToken_p, ResultToken_p);
        break;

    case VEXTOKEN_SUBCODE_VERIFYNRBG:
        funcres = vexLocal_VerifyNrbg(CommandToken_p, ResultToken_p);
        break;

    default:
        break;
    }

    return funcres;
}

/* end of file adapter_vex_trng.c */
