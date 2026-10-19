/* adapter_val_random.c
 *
 * Implementation of the VAL API.
 *
 * This file implements the random data related services.
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

#include "api_val_random.h"             // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_RandomData
 */
#ifndef VAL_REMOVE_RANDOM_DATA
ValStatus_t
val_RandomData(
        const ValSize_t DataSize,
        ValOctetsOut_t * const  Data_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((Data_p == NULL) ||
        (DataSize == 0) || (DataSize > 65535))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_RANDOMNUMBER;
    t_cmd.Service.Random.Data_p = (uint8_t *)Data_p;
    t_cmd.Service.Random.Size = (uint16_t)DataSize;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_RANDOM_DATA */


/*----------------------------------------------------------------------------
 * val_TrngRawData
 */
#ifndef VAL_REMOVE_TRNG_RAW_DATA
ValStatus_t
val_TrngRawData(
        const ValSize_t DataSize,
        ValOctetsOut_t * const  Data_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((Data_p == NULL) ||
        (DataSize == 0) || (DataSize > 65280) || ((DataSize % 256) != 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGRAWDATA;
    t_cmd.Service.Random.Data_p = (uint8_t *)Data_p;
    t_cmd.Service.Random.Size = (uint16_t)DataSize;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_TRNG_RAW_DATA */


/*----------------------------------------------------------------------------
 * val_RandomReseed
 */
#ifndef VAL_REMOVE_RANDOM_RESEED
ValStatus_t
val_RandomReseed(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 0;
    t_cmd.Service.TrngConfig.fReseed = 1;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 0;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_RANDOM_RESEED */


/*----------------------------------------------------------------------------
 * val_TrngConfig
 */
#ifndef VAL_REMOVE_TRNG_CONFIG
#ifdef EIP130_ENABLE_TRNG_CUTOFF
ValStatus_t
val_TrngConfigCutoff(uint8_t  AutoSeed,
                     uint16_t SampleCycles,
                     uint8_t  SampleDiv,
                     uint8_t  NoiseBlocks,
                     uint8_t  Scale,
                     uint8_t  RepCntCutoff,
                     uint8_t  AdaptProp64Cutoff,
                     uint16_t AdaptProp512Cutoff,
                     bool     fReseed)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 1;
    t_cmd.Service.TrngConfig.fReseed = fReseed ? 1 : 0;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 0;
    t_cmd.Service.TrngConfig.AutoSeed = AutoSeed;
    t_cmd.Service.TrngConfig.SampleCycles = SampleCycles;
    t_cmd.Service.TrngConfig.SampleDiv = SampleDiv;
    t_cmd.Service.TrngConfig.NoiseBlocks = NoiseBlocks;
    t_cmd.Service.TrngConfig.Scale = Scale;
    t_cmd.Service.TrngConfig.RepCntCutoff = RepCntCutoff;
    t_cmd.Service.TrngConfig.AdaptProp64Cutoff = AdaptProp64Cutoff;
    t_cmd.Service.TrngConfig.AdaptProp512Cutoff = AdaptProp512Cutoff;
    t_cmd.Service.TrngConfig.FroBlockKey = 0;
    t_cmd.Service.TrngConfig.fAria = 0;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#else /* !EIP130_ENABLE_TRNG_CUTOFF */
ValStatus_t
val_TrngConfig(uint8_t  AutoSeed,
               uint16_t SampleCycles,
               uint8_t  SampleDiv,
               uint8_t  NoiseBlocks,
               uint8_t  Scale,
               bool     fReseed)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 1;
    t_cmd.Service.TrngConfig.fReseed = fReseed ? 1 : 0;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 0;
    t_cmd.Service.TrngConfig.AutoSeed = AutoSeed;
    t_cmd.Service.TrngConfig.SampleCycles = SampleCycles;
    t_cmd.Service.TrngConfig.SampleDiv = SampleDiv;
    t_cmd.Service.TrngConfig.NoiseBlocks = NoiseBlocks;
    t_cmd.Service.TrngConfig.Scale = Scale;
    t_cmd.Service.TrngConfig.RepCntCutoff = 0;
    t_cmd.Service.TrngConfig.AdaptProp64Cutoff = 0;
    t_cmd.Service.TrngConfig.AdaptProp512Cutoff = 0;
    t_cmd.Service.TrngConfig.FroBlockKey = 0;
    t_cmd.Service.TrngConfig.fAria = 0;

    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#endif /* EIP130_ENABLE_TRNG_CUTOFF */
#endif /* !VAL_REMOVE_TRNG_CONFIG */

#ifndef VAL_REMOVE_TRNG_CONFIG_ARIA
#ifdef EIP130_ENABLE_TRNG_CUTOFF
ValStatus_t
val_TrngConfigAriaDRBGCutoff(uint8_t  AutoSeed,
                             uint16_t SampleCycles,
                             uint8_t  SampleDiv,
                             uint8_t  NoiseBlocks,
                             uint8_t  Scale,
                             uint8_t  RepCntCutoff,
                             uint8_t  AdaptProp64Cutoff,
                             uint16_t AdaptProp512Cutoff,
                             bool     fReseed)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 1;
    t_cmd.Service.TrngConfig.fReseed = fReseed ? 1 : 0;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 0;
    t_cmd.Service.TrngConfig.AutoSeed = AutoSeed;
    t_cmd.Service.TrngConfig.SampleCycles = SampleCycles;
    t_cmd.Service.TrngConfig.SampleDiv = SampleDiv;
    t_cmd.Service.TrngConfig.NoiseBlocks = NoiseBlocks;
    t_cmd.Service.TrngConfig.Scale = Scale;
    t_cmd.Service.TrngConfig.RepCntCutoff = RepCntCutoff;
    t_cmd.Service.TrngConfig.AdaptProp64Cutoff = AdaptProp64Cutoff;
    t_cmd.Service.TrngConfig.AdaptProp512Cutoff = AdaptProp512Cutoff;
    t_cmd.Service.TrngConfig.FroBlockKey = 0;
    t_cmd.Service.TrngConfig.fAria = 1;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#else /* !EIP130_ENABLE_TRNG_CUTOFF */
ValStatus_t
val_TrngConfigAriaDRBG(uint8_t  AutoSeed,
                       uint16_t SampleCycles,
                       uint8_t  SampleDiv,
                       uint8_t  NoiseBlocks,
                       uint8_t  Scale,
                       bool     fReseed)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 1;
    t_cmd.Service.TrngConfig.fReseed = fReseed ? 1 : 0;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 0;
    t_cmd.Service.TrngConfig.AutoSeed = AutoSeed;
    t_cmd.Service.TrngConfig.SampleCycles = SampleCycles;
    t_cmd.Service.TrngConfig.SampleDiv = SampleDiv;
    t_cmd.Service.TrngConfig.NoiseBlocks = NoiseBlocks;
    t_cmd.Service.TrngConfig.Scale = Scale;
    t_cmd.Service.TrngConfig.RepCntCutoff = 0;
    t_cmd.Service.TrngConfig.AdaptProp64Cutoff = 0;
    t_cmd.Service.TrngConfig.AdaptProp512Cutoff = 0;
    t_cmd.Service.TrngConfig.FroBlockKey = 0;
    t_cmd.Service.TrngConfig.fAria = 1;
    t_res.Result = VEXTOKEN_RESULT_NO_ERROR;

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if ((funcres == VAL_SUCCESS) &&
        (t_res.Result != VEXTOKEN_RESULT_NO_ERROR))
    {
        // Error
        LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);

        if (t_res.Result == VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER)
        {
            funcres = VAL_NOT_INITIALIZED;
        }
        else
        {
            funcres = (ValStatus_t)t_res.Result;
        }
    }

    return funcres;
}
#endif /* EIP130_ENABLE_TRNG_CUTOFF */
#endif /* !VAL_REMOVE_TRNG_CONFIG_ARIA */

/*----------------------------------------------------------------------------
 * val_TrngDrbgPostProcessing
 */
#ifndef VAL_REMOVE_VERIFY_DRBG_POSTPROCCESSING
ValStatus_t
val_TrngDrbgPostProcessing(
        const ValTrngDrbgPPTest_t Test,
        const uint16_t Pattern,
        const ValSize_t DataSize,
        ValOctetsIn_t * const InputData_p,
        const ValSize_t InputDataSize,
        ValOctetsOut_t * const OutputData_p,
        const ValSize_t OutputDataSize)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_VERIFYDRBG;
    switch (Test)
    {
    case VAL_TRNG_DRBGPP_CF_12B_REPEAT_PATTERN:
#ifdef VAL_STRICT_ARGS
        if ((DataSize == 0) || (DataSize > 255) ||
            (OutputData_p == NULL))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
        t_cmd.Service.DrbgPP.Pattern = Pattern;
        t_cmd.Service.DrbgPP.Size = (uint16_t)DataSize;
        t_cmd.Service.DrbgPP.InputData_p = 0;
        t_cmd.Service.DrbgPP.InputDataSize = 0;
        break;

    case VAL_TRNG_DRBGPP_CF_EXTERNAL_NOISE:
#ifdef VAL_STRICT_ARGS
        if ((DataSize == 0) || (DataSize > 255) ||
            (InputData_p == NULL) ||
            (OutputData_p == NULL))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
        t_cmd.Service.DrbgPP.Pattern = 0;
        t_cmd.Service.DrbgPP.Size = (uint16_t)DataSize;
        t_cmd.Service.DrbgPP.InputData_p = InputData_p;
        t_cmd.Service.DrbgPP.InputDataSize = (uint32_t)InputDataSize;
        break;

    case VAL_TRNG_DRBGPP_KNOWN_ANSWER:
#ifdef VAL_STRICT_ARGS
        if (((DataSize & MASK_4_BITS) == 0) ||
            (InputData_p == NULL) ||
            (OutputData_p == NULL))
        {
            return VAL_BAD_ARGUMENT;
        }
#endif
        t_cmd.Service.DrbgPP.Pattern = 0;
        t_cmd.Service.DrbgPP.Size = (uint16_t)DataSize;
        t_cmd.Service.DrbgPP.InputData_p = InputData_p;
        t_cmd.Service.DrbgPP.InputDataSize = (uint32_t)InputDataSize;
        break;

    default:
        return VAL_BAD_ARGUMENT;
    }
    t_cmd.Service.DrbgPP.Test = (uint8_t)Test;
    t_cmd.Service.DrbgPP.OutputData_p = OutputData_p;
    t_cmd.Service.DrbgPP.OutputDataSize = (uint32_t)OutputDataSize;
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
#endif /* !VAL_REMOVE_VERIFY_DRBG_POSTPROCCESSING */


/*----------------------------------------------------------------------------
 * val_TrngNrbgSelfTest
 */
#ifndef VAL_REMOVE_VERIFY_NRBG_SELFTEST
ValStatus_t
val_TrngNrbgSelfTest(
        ValOctetsIn_t * const InputData_p,
        const ValSize_t Size,
        uint8_t * const RepCntCutoff_p,
        uint8_t * const RepCntCount_p,
        uint8_t * const RepCntData_p,
        uint8_t * const AdaptProp64Cutoff_p,
        uint8_t * const AdaptProp64Count_p,
        uint8_t * const AdaptProp64Data_p,
        uint16_t * const AdaptProp512Cutoff_p,
        uint16_t * const AdaptProp512Count_p,
        uint8_t * const AdaptProp512Data_p,
        uint8_t * const AdaptProp64Fail_p,
        uint8_t * const AdaptProp512Fail_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    if ((InputData_p == NULL) ||
        (RepCntCutoff_p == NULL) ||
        (RepCntCount_p == NULL) ||
        (RepCntData_p == NULL) ||
        (AdaptProp64Cutoff_p == NULL) ||
        (AdaptProp64Count_p == NULL) ||
        (AdaptProp64Data_p == NULL) ||
        (AdaptProp64Fail_p == NULL) ||
        (AdaptProp512Cutoff_p == NULL) ||
        (AdaptProp512Count_p == NULL) ||
        (AdaptProp512Data_p == NULL) ||
        (AdaptProp512Fail_p == NULL) ||
        ((*RepCntCutoff_p & ~(MASK_6_BITS)) != 0) ||
        ((*AdaptProp64Cutoff_p & ~(MASK_6_BITS)) != 0) ||
        ((*AdaptProp512Cutoff_p & ~(MASK_12_BITS)) != 0) ||
        ((*RepCntCount_p & ~(MASK_6_BITS)) != 0) ||
        ((*AdaptProp64Count_p & ~(MASK_6_BITS)) != 0) ||
        ((*AdaptProp512Count_p & ~(MASK_12_BITS)) != 0) ||
        (Size == 0) ||
        ((Size & ~(MASK_16_BITS)) != 0))
    {
        return VAL_BAD_ARGUMENT;
    }
#endif

    // Format service request
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_VERIFYNRBG;

    t_cmd.Service.NrbgVerify.RepCntCutoff = *RepCntCutoff_p;
    t_cmd.Service.NrbgVerify.RepCntCount = *RepCntCount_p;
    t_cmd.Service.NrbgVerify.RepCntData = *RepCntData_p;

    t_cmd.Service.NrbgVerify.AdaptProp64Cutoff = *AdaptProp64Cutoff_p;
    t_cmd.Service.NrbgVerify.AdaptProp64Count = *AdaptProp64Count_p;
    t_cmd.Service.NrbgVerify.AdaptProp64Data = *AdaptProp64Data_p;

    t_cmd.Service.NrbgVerify.AdaptProp512Cutoff = *AdaptProp512Cutoff_p;
    t_cmd.Service.NrbgVerify.AdaptProp512Count = *AdaptProp512Count_p;
    t_cmd.Service.NrbgVerify.AdaptProp512Data = *AdaptProp512Data_p;

    t_cmd.Service.NrbgVerify.InputData_p = InputData_p;
    t_cmd.Service.NrbgVerify.Size = (uint16_t)Size;
    ZEROINIT(t_res);

    // Exchange service request with the next driver level
    funcres = val_ExchangeToken(&t_cmd, &t_res);
    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            *RepCntCutoff_p = t_res.Service.NrbgVerify.RepCntCutoff;
            *RepCntCount_p = t_res.Service.NrbgVerify.RepCntCount;
            *RepCntData_p = t_res.Service.NrbgVerify.RepCntData;

            *AdaptProp64Cutoff_p = t_res.Service.NrbgVerify.AdaptProp64Cutoff;
            *AdaptProp64Count_p = t_res.Service.NrbgVerify.AdaptProp64Count;
            *AdaptProp64Data_p = t_res.Service.NrbgVerify.AdaptProp64Data;
            *AdaptProp64Fail_p = t_res.Service.NrbgVerify.AdaptProp64Fail;

            *AdaptProp512Cutoff_p = t_res.Service.NrbgVerify.AdaptProp512Cutoff;
            *AdaptProp512Count_p = t_res.Service.NrbgVerify.AdaptProp512Count;
            *AdaptProp512Data_p = t_res.Service.NrbgVerify.AdaptProp512Data;
            *AdaptProp512Fail_p = t_res.Service.NrbgVerify.AdaptProp512Fail;
        }
        else
        {
            // Error
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }

    return funcres;
}
#endif /* !VAL_REMOVE_VERIFY_NRBG_SELFTEST */

#ifndef VAL_REMOVE_DRBG_FORCESEED
ValStatus_t
val_DRBGForceSeed(uint8_t        Autoseed,
                  const uint8_t *Seed_p)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;
    ValStatus_t funcres;
#ifdef VAL_STRICT_ARGS
    if (Seed_p == NULL)
    {
        return VAL_BAD_ARGUMENT;
    }
#endif
    t_cmd.OpCode = VEXTOKEN_OPCODE_TRNG;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_TRNGCONFIG;
    t_cmd.Service.TrngConfig.fLoadStart = 0;
    t_cmd.Service.TrngConfig.fReseed = 0;
    t_cmd.Service.TrngConfig.fDRBGForceSeed = 1;
    t_cmd.Service.TrngConfig.AutoSeed = Autoseed;
    memcpy(t_cmd.Service.TrngConfig.Seed, Seed_p, sizeof(t_cmd.Service.TrngConfig.Seed));

    ZEROINIT(t_res);
    funcres = val_ExchangeToken(&t_cmd, &t_res);

    if (funcres == VAL_SUCCESS)
    {
        if (t_res.Result == VEXTOKEN_RESULT_NO_ERROR)
        {
            LOG_INFO("DRBG force seeded\n");
        }
        else
        {
            // Error
            funcres = (ValStatus_t)t_res.Result;
            LOG_WARN("Abort - %s()=%d\n", __func__, t_res.Result);
        }
    }
    return funcres;
}
#endif

/* end of file adapter_val_random.c */
