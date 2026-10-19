/* da_val_specials.c
 *
 * Demo Application for the VAL API Specials that cannot be tested via the
 * test program
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "da_val_specials.h"            // da_val_specials_main()


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_da_val_specials.h"          // configuration

#include "log.h"
#include "api_driver_init.h"
#include "api_val.h"

#include <stdlib.h>                     // malloc, free, strtoull


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define DA_PREFIX   "DA_VAL_SPEC: "

#ifdef DA_VAL_SPECIAL_EMMC
// eMMC Message types
typedef enum
{
    // Requests
    EMMC_MSGTYP_REQ_AUTH_KEY_PROG = 1,
    EMMC_MSGTYP_REQ_RD_WCNTR      = 2,
    EMMC_MSGTYP_REQ_AUTH_DATA_WR  = 3,
    EMMC_MSGTYP_REQ_AUTH_DATA_RD  = 4,
    EMMC_MSGTYP_REQ_RESULT_RD     = 5,

    // Responses
    EMMC_MSGTYP_RSP_AUTH_KEY_PROG = 0x100,
    EMMC_MSGTYP_RSP_RD_WCNTR      = 0x200,
    EMMC_MSGTYP_RSP_AUTH_DATA_WR  = 0x300,
    EMMC_MSGTYP_RSP_AUTH_DATA_RD  = 0x400,
} eMMC_MsgTypes;
#endif /* DA_VAL_SPECIAL_EMMC */

#ifdef DA_VAL_SPECIAL_HIBERNATION
typedef struct
{
    // Param 0: Decryption counter
    uint8_t DecCounterLen;
    uint8_t DecCounterZero1;
    uint8_t DecCounterPolicy[4];
    uint8_t DecCounterZero2[4];
    uint8_t DecCounterData[4];
    // Param 1: Encryption counter
    uint8_t EncCounterLen;
    uint8_t EncCounterZero1;
    uint8_t EncCounterPolicy[4];
    uint8_t EncCounterZero2[4];
    uint8_t EncCounterData[4];
    // Param 2: eMMC address
    uint8_t eMmcAddrLen;
    uint8_t eMmcAddrZero1;
    uint8_t eMmcAddrPolicy[4];
    uint8_t eMmcAddrZero2[4];
    uint8_t eMmcAddrData[2];
    // Param 3: Associated Data Tail
    uint8_t ADTailLen;
    uint8_t ADTailZero1;
    uint8_t ADTailPolicy[4];
    uint8_t ADTailZero2[4];
    uint8_t ADTailData[48];
    uint8_t EndMarker;
} eMMC_HibernationData;
#endif /* DA_VAL_SPECIAL_HIBERNATION */


/*----------------------------------------------------------------------------
 * Local variables
 */
// Hardware Unique Key (HUK)
static const uint8_t gl_ADLabel[] = "SomeAssociatedDataForProvisioningWithKeyBlob";
static const uint8_t gl_HUK_OTPKeyBlob[] =
{
#ifdef EIP130_FW_ASSETPOLICY_V2
    0x7E, 0x5D, 0x36, 0x4E, 0x25, 0x7B, 0x27, 0xE4, 0x0B, 0x41, 0x05, 0xE9, 0x5E, 0xC0, 0xDE, 0x99,
    0xA0, 0xEA, 0x96, 0x0B, 0x72, 0xEB, 0x95, 0x37, 0x60, 0x34, 0x44, 0xA9, 0xFE, 0x62, 0xDB, 0x49,
    0x89, 0x63, 0x38, 0x2E, 0x24, 0x78, 0xE7, 0x17, 0x81, 0xE8, 0x63, 0x66, 0xF5, 0xE0, 0x67, 0xD8
#else
    0x18, 0x6E, 0x2E, 0xA2, 0x32, 0x09, 0x5B, 0x4A, 0x17, 0xCD, 0xA0, 0xDA, 0x8C, 0xB8, 0x88, 0xED,
    0x2B, 0x33, 0x8A, 0x33, 0xE6, 0x35, 0xC9, 0x8B, 0x20, 0x24, 0x3B, 0x44, 0x5B, 0x39, 0x4B, 0xDD,
    0x98, 0x11, 0x37, 0xF0, 0x96, 0x20, 0xB3, 0x34, 0x6E, 0xC4, 0xDE, 0xCB, 0xC4, 0x34, 0x53, 0x63,
#endif /* EIP130_FW_ASSETPOLICY_V2 */
};

static uint16_t gl_SampleCycles = 3072;

#ifdef EIP130_FW_ASSETPOLICY_V2
static const uint8_t gl_monotoniccounter_keyblob[] = {
    0xac, 0xcc, 0x47, 0x4b, 0x01, 0xb4, 0x3a, 0x3d, 0x5c, 0x17, 0x54, 0x68,
    0xfd, 0x7f, 0x30, 0xa5, 0xd1, 0x97, 0x26, 0xf8, 0x99, 0xfc, 0x39, 0x2e,
    0x3c, 0xd6, 0x13, 0xeb, 0x33, 0x4a, 0x56, 0x07
};
#endif /* EIP130_FW_ASSETPOLICY_V2 */

#ifdef DA_VAL_SPECIAL_EMMC
// eMMC Authentication Key (AuthKey)
static const uint8_t gl_eMMCAuthKey[] =
{
    0xC8, 0x4D, 0x7A, 0x95, 0xE8, 0x81, 0x48, 0xC1, 0x9E, 0x40, 0xE8, 0xFB, 0xCF, 0xE6, 0x4F, 0xBA,
    0xE6, 0xAF, 0x78, 0x19, 0x6F, 0x9C, 0xE9, 0xF7, 0x7A, 0xDD, 0x42, 0xCE, 0x8C, 0x03, 0xB8, 0x66
};
static const uint8_t gl_eMMCAuthKey_OTPKeyBlob[] =
{
#ifdef EIP130_FW_ASSETPOLICY_V2
    0xe1, 0x66, 0x34, 0x36, 0xd6, 0x2f, 0xd7, 0x0a, 0xeb, 0xf8, 0xcb, 0x8c, 0xf9, 0xe5, 0xce, 0xe0,
    0xfe, 0x02, 0xb1, 0x2f, 0x61, 0x7c, 0x70, 0x99, 0x8b, 0xc4, 0xc3, 0x49, 0xdf, 0xc3, 0x69, 0x22,
    0xd9, 0x0f, 0x81, 0xbf, 0xc6, 0x4e, 0x48, 0xe3, 0xcf, 0xb5, 0x7e, 0x0b, 0x41, 0x85, 0x94, 0x55
#else
    0x64, 0x0E, 0xCA, 0xDD, 0xCD, 0x59, 0xD1, 0x30, 0x6C, 0x1A, 0x9F, 0x06, 0x16, 0x73, 0x04, 0x71,
    0x24, 0x48, 0x24, 0x1F, 0x90, 0xE9, 0x1C, 0x8C, 0x40, 0xCD, 0xBC, 0x67, 0xE8, 0x11, 0xF6, 0xC0,
    0x59, 0x45, 0x7B, 0x1E, 0xCA, 0xA3, 0xB3, 0x5B, 0x72, 0x84, 0xE8, 0x5A, 0x65, 0x02, 0xAD, 0x81
#endif /* EIP130_FW_ASSETPOLICY_V2 */
};
static const uint8_t gl_eMMCAuthKey_AssetNumber = 12;

// eMMC Policies
#ifdef DA_VAL_SPECIAL_HIBERNATION
//static const uint8_t gl_eMMC_PolicyZero[4]      = { 0, 0, 0, 0 };    // empty
static const uint8_t gl_eMMC_PolicyConstant[4]  = { 1, 2, 2, 0 };    // public, service, constant
static const uint8_t gl_eMMC_PolicyMonCntBI[4]  = { 1, 1, 0x09, 0 }; // Monotonic counter, binary inc, without max
//static const uint8_t gl_eMMC_PolicyMonCntBIM[4] = { 1, 1, 0x19, 0 }; // Monotonic counter, binary inc, with max
#endif

// eMMC addresses
static const uint8_t gl_eMMCAddr_SequenceChk = 0;
#ifdef DA_VAL_SPECIAL_HIBERNATION
static const uint8_t gl_eMMCAddr_Hibernation = 5;
#endif

// eMMC Memory Model
static uint32_t gl_eMMCModelWriteCounter = 0;
static uint8_t gl_eMMCModelNonce[16] = { 0 };
#define DA_EMMC_MODEL_MEMORY_ENTRIES  6
static uint8_t gl_eMMCModelMemory[DA_EMMC_MODEL_MEMORY_ENTRIES][256];

static uint8_t gl_EmptyMac[1] = { 0 };
#endif /* DA_VAL_SPECIAL_EMMC */

#ifdef DA_VAL_SPECIAL_HIBERNATION
static bool gl_HibernationDone = false;
#endif /* DA_VAL_SPECIAL_HIBERNATION */

#if defined(DA_VAL_SPECIAL_HIBERNATION) || defined(DA_VAL_SPECIAL_SF_MILENAGE)
static uint8_t gl_DataBlobBuffer[(128 / 8) + 1024];
static ValSize_t gl_DataBlobSize;
#endif  /*DA_VAL_SPECIAL_HIBERNATION || DA_VAL_SPECIAL_SF_MILENAGE */

#ifdef DA_VAL_SPECIAL_SF_MILENAGE
// Milenage Related information
#ifdef EIP130_FW_ASSETPOLICY_V2
static const uint8_t gl_Milenage_AssetNumber = 6;
#else
static const uint8_t gl_Milenage_AssetNumber = 14;
#endif /* EIP130_FW_ASSETPOLICY_V2 */

/* Test Set1 */
static const uint8_t gl_Milenage_TS1_OTPBlob[] = {
#ifdef EIP130_FW_ASSETPOLICY_V2
    /* Keyblob without nodomain policy
    * 0xfa, 0xb5, 0x95, 0x7f, 0x74, 0x4c, 0x71, 0xa8, 0x8b, 0xf2, 0xf7, 0x73, 0x9d, 0x3c, 0xde, 0x36,
    * 0x63, 0x81, 0xeb, 0x75, 0x08, 0xdb, 0xcb, 0xcf, 0x6e, 0x1f, 0xeb, 0xee, 0xee, 0xf5, 0x38, 0xb1,
    * 0x3f, 0x0d, 0x69, 0xc0, 0x24, 0xe4, 0x86, 0xc6, 0x5e, 0x3c, 0x55, 0xa0, 0x25, 0xac, 0x92, 0x45
    */
    0x5a, 0xf7, 0xc3, 0x4a, 0x93, 0x3c, 0x0a, 0xc1, 0xc1, 0x51, 0x17, 0xbe, 0xc4, 0x70, 0x83, 0xef,
    0xd7, 0xff, 0x76, 0xac, 0x58, 0x49, 0x72, 0x03, 0xb5, 0x81, 0x46, 0x0f, 0x08, 0xb4, 0xf4, 0x37,
    0x26, 0x59, 0x0b, 0xa4, 0x42, 0xb7, 0xc2, 0x3d, 0x88, 0xdb, 0xfd, 0x3b, 0x66, 0x2d, 0x21, 0x80
#else
    0xA0, 0xF1, 0xC6, 0x1D, 0x26, 0x86, 0x33, 0x7A, 0xE3, 0xF4, 0x6D, 0x6E, 0xBB, 0x4C, 0xEB, 0x7F,
    0xBD, 0x66, 0xDA, 0x81, 0x38, 0xA1, 0x46, 0x01, 0x97, 0x0C, 0xAC, 0xAC, 0x9C, 0x67, 0x90, 0x98,
    0x25, 0xC6, 0x98, 0x15, 0xD9, 0xDA, 0x4D, 0x09, 0x50, 0x3D, 0xE6, 0x7C, 0x08, 0x33, 0x39, 0xDE
#endif /* EIP130_FW_ASSETPOLICY_V2 */
};
static const uint8_t gl_Milenage_TS1_RAND[] = {
    0x23, 0x55, 0x3C, 0xBE, 0x96, 0x37, 0xA8, 0x9D, 0x21, 0x8A, 0xE6, 0x4D, 0xAE, 0x47, 0xBF, 0x35
};
static const uint8_t gl_Milenage_TS1_SQN[] = {
    0xFF, 0x9B, 0xB4, 0xD0, 0xB6, 0x07
};
static const uint8_t gl_Milenage_TS1_AMF[] = {
    0xB9, 0xB9
};
static const uint8_t gl_Milenage_TS1_f1[] = {
    0x4A, 0x9F, 0xFA, 0xC3, 0x54, 0xDF, 0xAF, 0xB3
};
static const uint8_t gl_Milenage_TS1_f1star[] = {
    0x01, 0xCF, 0xAF, 0x9E, 0xC4, 0xE8, 0x71, 0xE9
};
static const uint8_t gl_Milenage_TS1_f2[] = {
    0xA5, 0x42, 0x11, 0xD5, 0xE3, 0xBA, 0x50, 0xBF
};
static const uint8_t gl_Milenage_TS1_f3[] = {
    0xB4, 0x0B, 0xA9, 0xA3, 0xC5, 0x8B, 0x2A, 0x05, 0xBB, 0xF0, 0xD9, 0x87, 0xB2, 0x1B, 0xF8, 0xCB
};
static const uint8_t gl_Milenage_TS1_f4[] = {
    0xF7, 0x69, 0xBC, 0xD7, 0x51, 0x04, 0x46, 0x04, 0x12, 0x76, 0x72, 0x71, 0x1C, 0x6D, 0x34, 0x41
};
static const uint8_t gl_Milenage_TS1_f5[] = {
    0xAA, 0x68, 0x9C, 0x64, 0x83, 0x70
};
static const uint8_t gl_Milenage_TS1_f5star[] = {
    0x45, 0x1E, 0x8B, 0xEC, 0xA4, 0x3B
};
#endif /* DA_VAL_SPECIAL_SF_MILENAGE */

#ifdef DA_VAL_SPECIAL_FIRMWARE_LOAD
static const uint8_t gl_SFWBPKD_keyblob[] = {
    0xEA, 0xCA, 0x25, 0x3C, 0xC2, 0x2F, 0x44, 0x32, 0x0A, 0x3B, 0xA8, 0x70, 0xB1, 0xE7, 0x74, 0x80,
    0xAF, 0x77, 0x0F, 0x61, 0x81, 0x62, 0xA4, 0x69, 0x69, 0x5C, 0x23, 0xAB, 0x45, 0x01, 0x82, 0x3C,
    0xF2, 0x00, 0x13, 0x9B, 0x7D, 0x08, 0x84, 0x70, 0xB2, 0x03, 0x71, 0x33, 0x88, 0x72, 0x68, 0x07
};
static const uint8_t gl_SFWBCR_keyblob[] = {
    0x11, 0x4D, 0x09, 0xE5, 0x23, 0xF7, 0x4A, 0x8F, 0x2E, 0xA3, 0x03, 0x02, 0x90, 0x27, 0xFB, 0xF0,
    0x68, 0xD6, 0xCA, 0x2B, 0x74, 0xEF, 0xE3, 0x91, 0xBB, 0x1C, 0xFB, 0x1C, 0xD5, 0xEE, 0xA6, 0xCD,
    0x3B, 0x7D, 0x10, 0x03, 0x9A, 0xBB, 0xF7, 0xE4, 0x14, 0xF5, 0xDD, 0xC7, 0x75, 0xA5, 0xF7, 0xFE
};
static const uint8_t gl_SFWBVER_keyblob[] = {
    0x95, 0xc2, 0x45, 0x92, 0x44, 0x08, 0x9f, 0x95, 0x2e, 0x40, 0x8c, 0x59, 0x92, 0xe5, 0xd9, 0x05,
    0x09, 0xbb, 0x44, 0x1a
};
#endif /* DA_VAL_SPECIAL_FIRMWARE_LOAD */


/*----------------------------------------------------------------------------
 * DoReset
 *
 * This function performs a firmware based reset.
 */
static ValStatus_t
DoReset(void)
{
    ValStatus_t FuncRes;

    FuncRes = val_SystemReset();
    if (FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: Reset system.\n");
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SystemReset()=%d.\n", (int)FuncRes);
    }
    return FuncRes;
}


/*----------------------------------------------------------------------------
 * WriteOTP
 *
 * This function writes an item (KeyBlob) to OTP.
 */
static ValStatus_t
WriteOTP(
        const ValAssetNumber_t AssetNumber,
#ifdef EIP130_FW_ASSETPOLICY_V2
        ValPolicyMask_t AssetPolicy,
#else
        const ValAssetNumber_t AssetPolicy,
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        const bool fAddCRC,
        ValOctetsIn_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsIn_t * const KeyBlob_p,
        const ValSize_t KeyBlobSize)
{
    ValStatus_t FuncRes;

    FuncRes = val_OTPDataWrite(AssetNumber, AssetPolicy, fAddCRC,
                               AssociatedData_p, AssociatedDataSize,
                               KeyBlob_p, KeyBlobSize);
    if (FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: OTP data written.\n");
        LOG_CRIT(DA_PREFIX "Reset system to enable the information stored in OTP\n");
        FuncRes = DoReset();
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_OTPDataWrite()=%d.\n", (int)FuncRes);
    }
    return FuncRes;
}


/*----------------------------------------------------------------------------
 * WriteRandomHuk
 *
 * This function writes a randomly generated HUK and default COID to OTP.
 */
static ValStatus_t
WriteRandomHuk(
        const ValAssetNumber_t StaticAssetNumber,
        const bool f128bit,
        const bool fAddCRC,
        char * const SampleCyclesInfo,
        const bool fAria)
{
    ValStatus_t FuncRes;
    ValSize_t KeyBlobSize = 0;
    uint8_t OTPKeyBlobBuffer[(128+256)/8];
    uint8_t * KeyBlob_p = NULL;

    if (SampleCyclesInfo[0] == '=')
    {
        gl_SampleCycles = (uint16_t)atoi(&SampleCyclesInfo[1]);
        if (gl_SampleCycles == 0)
        {
            gl_SampleCycles = 2;
        }
    }

    if (!f128bit)
    {
        KeyBlob_p = OTPKeyBlobBuffer;
        KeyBlobSize = sizeof(OTPKeyBlobBuffer);
        memset(OTPKeyBlobBuffer, 0, KeyBlobSize);
    }

#ifdef DA_VAL_SPECIAL_ARIA_DRBG
    if (fAria)
    {
        FuncRes = val_ProvisionRandomRootKeyARIA(0x4F5A3647, StaticAssetNumber,
                                                 f128bit, fAddCRC,
                                                 0, gl_SampleCycles, 1, 1,
                                                 gl_ADLabel, sizeof(gl_ADLabel)-1,
                                                 KeyBlob_p, &KeyBlobSize);
    }
    else
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */
    {
        FuncRes = val_ProvisionRandomRootKey(0x4F5A3647, StaticAssetNumber,
                                             f128bit, fAddCRC,
                                             0, gl_SampleCycles, 1, 1,
                                             gl_ADLabel, sizeof(gl_ADLabel)-1,
                                             KeyBlob_p, &KeyBlobSize);
    }
    if (FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: Random HUK written.\n");

        if (KeyBlob_p != NULL)
        {
            Log_HexDump("OTPKeyBlob(HUK)", 0, KeyBlob_p, (unsigned int)KeyBlobSize);
        }

        LOG_CRIT(DA_PREFIX "Reset system to enable the information stored in OTP\n");
        FuncRes = DoReset();
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_ProvisionRandomRootKey%s()=%d.\n",
                 fAria ? "Aria" : "", (int)FuncRes);
    }
    return FuncRes;
}


/*----------------------------------------------------------------------------
 * ConfigureTrng
 *
 * This function configures and actives the TRNG and AES based DRBG.
 */
static ValStatus_t
ConfigureTrng(void)
{
    ValStatus_t FuncRes;

    /* IntegrationItem:
     * The TRNG configuration needs to be adapted to meet the customer
     * specific FROs timing/behavior. */
    LOG_CRIT(DA_PREFIX "Configure TRNG (AES-DRBG) (%u)\n", gl_SampleCycles);
#ifdef EIP130_ENABLE_TRNG_CUTOFF
    FuncRes = val_TrngConfigCutoff(0, gl_SampleCycles, 1, 8, 0, 31, 56, 325, 1);
#else
    FuncRes = val_TrngConfig(0, gl_SampleCycles, 1, 8, 0, 1);
#endif
    if(FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: Configure TRNG\n");
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_TrngConfig()=%d.\n", (int)FuncRes);
    }
    return FuncRes;
}


#ifdef DA_VAL_SPECIAL_ARIA_DRBG
/*----------------------------------------------------------------------------
 * ConfigureAriaDrbg
 *
 * This function configures and actives the TRNG and ARIA based DRBG.
 */
static ValStatus_t
ConfigureAriaDrbg(void)
{
    ValStatus_t FuncRes;

    /* IntegrationItem:
     * The TRNG configuration needs to be adapted to meet the customer
     * specific FROs timing/behavior. */
    LOG_CRIT(DA_PREFIX "Configure ARIA-DRBG (%u)\n", gl_SampleCycles);
#ifdef EIP130_ENABLE_TRNG_CUTOFF
    FuncRes = val_TrngConfigAriaDRBGCutoff(0, gl_SampleCycles, 1, 8, 0, 31, 56, 325, 1);
#else
    FuncRes = val_TrngConfigAriaDRBG(0, gl_SampleCycles, 1, 8, 0, 1);
#endif
    if(FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: Configure ARIA-DRBG\n");
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_TrngConfigAriaDRBG()=%d.\n", (int)FuncRes);
    }
    return FuncRes;
}
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */


#ifdef DA_VAL_SPECIAL_DRBG_FORCESEED
/*----------------------------------------------------------------------------
 * ConfigureDrbgForceSeed
 *
 * This function configures and actives an AES based DRBG. Note that the use
 * of the FROs is disabled, so random data generation is stopped after
 * generating 64K bytes of random data.
 */
static ValStatus_t
ConfigureDrbgForceSeed(void)
{
    ValStatus_t FuncRes;

    LOG_CRIT(DA_PREFIX "Configure DRBG force seed\n");
    FuncRes = val_DRBGForceSeed(0, gl_HUK_OTPKeyBlob);
    if(FuncRes == VAL_SUCCESS)
    {
        uint8_t RandNum[128];

        FuncRes = val_RandomData(128, RandNum);
        if(FuncRes == VAL_SUCCESS)
        {
            LOG_CRIT(DA_PREFIX "PASSED: Configure DRBG force seed\n");
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_RandomData()=%d.\n", (int)FuncRes);
        }
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_DRBGForceSeed()=%d.\n", (int)FuncRes);
    }
    return FuncRes;
}
#endif /* DA_VAL_SPECIAL_DRBG_FORCESEED */


/*----------------------------------------------------------------------------
 * GetTrngRawData
 *
 * This function read raw data from TRNG.
 */
static ValStatus_t
GetTrngRawData(uint16_t aBlocks)
{
    uint8_t * pBuffer;
    ValSize_t Size;
    ValStatus_t FuncRes;

    Size = (ValSize_t)(aBlocks * 256);
    pBuffer = malloc(Size);
    if (pBuffer == 0)
    {
        LOG_CRIT(DA_PREFIX "ERROR: Memory allocation (%d)\n", (int)Size);
        FuncRes = VAL_NO_MEMORY;
    }
    else
    {
        LOG_CRIT(DA_PREFIX "Read TRNG Raw Data\n");
        FuncRes = val_TrngRawData(Size, pBuffer);
        if(FuncRes == VAL_SUCCESS)
        {
            Log_HexDump(DA_PREFIX "PASSED: Raw Data:", 0, pBuffer, (unsigned int)Size);
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_TrngRawData()=%d\n", FuncRes);
        }
        free(pBuffer);
    }
    return FuncRes;
}


/*----------------------------------------------------------------------------
 * GetVersion
 *
 * This function configures and actives the TRNG.
 */
static ValStatus_t
GetVersion(void)
{
    ValOctetsOut_t DataOut_p[100];
    ValSize_t DataOutByteCount = 100;
    ValStatus_t FuncRes;

    LOG_CRIT(DA_PREFIX "Get version information as a sanity check\n");
    FuncRes = val_SystemGetVersion(DataOut_p, &DataOutByteCount);
    if(FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "PASSED: Version: %s\n", DataOut_p);
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SystemGetVersion()=%d\n", FuncRes);
    }
    return FuncRes;
}


#ifdef DA_VAL_SPECIAL_EMMC
/*----------------------------------------------------------------------------
 * eMMCModelCreatePacket
 *
 * This function creates an eMMC packet based on the eMMC Model.
 */
static int
eMMCModelCreatePacket(
        uint16_t aMsgType,
        uint8_t * aPktBuffer_p,
        uint32_t aPktBufferSize,
        uint16_t aResult,
        uint16_t aBlockCount,
        uint16_t aAddress,
        uint8_t * aData_p,
        uint32_t aDataSize)
{
    uint8_t * Packet_p;
    uint16_t Result = 0;
    uint16_t BlockCount = 0;
    uint16_t Address = 0;
    uint32_t WriteCounter = 0;
    uint8_t * Nonce_p = NULL;
    uint8_t * Data_p = NULL;
    ValSize_t DataSize = 0;
    uint8_t * Mac_p = NULL;
    ValSize_t MacSize = 0;

    if ((aPktBuffer_p == NULL) || (aPktBufferSize < 512))
    {
        LOG_CRIT(DA_PREFIX "FAILED: Packet buffer too small\n");
        return -1;
    }

    switch (aMsgType)
    {
    default:
    case EMMC_MSGTYP_REQ_AUTH_KEY_PROG:
    case EMMC_MSGTYP_RSP_AUTH_KEY_PROG:
        LOG_CRIT(DA_PREFIX "FAILED: Operation not defined at the moment\n");
        return -2;

    case EMMC_MSGTYP_REQ_RD_WCNTR:
        Nonce_p = gl_eMMCModelNonce;
        Mac_p = gl_EmptyMac;
        MacSize = sizeof(gl_EmptyMac);
        break;

    case EMMC_MSGTYP_RSP_RD_WCNTR:
        Result = aResult;
        WriteCounter = gl_eMMCModelWriteCounter;
        Nonce_p = gl_eMMCModelNonce;
        break;

    case EMMC_MSGTYP_REQ_AUTH_DATA_WR:
        BlockCount = aBlockCount;
        Address = aAddress;
        WriteCounter = gl_eMMCModelWriteCounter;
        Data_p = aData_p;
        DataSize = aDataSize;
        break;

    case EMMC_MSGTYP_RSP_AUTH_DATA_WR:
        Result = aResult;
        Address = aAddress;
        WriteCounter = gl_eMMCModelWriteCounter;
        break;

    case EMMC_MSGTYP_REQ_AUTH_DATA_RD:
        Address = aAddress;
        Nonce_p = gl_eMMCModelNonce;
        Mac_p = gl_EmptyMac;
        MacSize = sizeof(gl_EmptyMac);
        break;

    case EMMC_MSGTYP_RSP_AUTH_DATA_RD:
        Result = aResult;
        BlockCount = aBlockCount;
        Address = aAddress;
        Nonce_p = gl_eMMCModelNonce;
        Data_p = aData_p;
        DataSize = aDataSize;
        break;

    case EMMC_MSGTYP_REQ_RESULT_RD:
        Mac_p = gl_EmptyMac;
        MacSize = sizeof(gl_EmptyMac);
        break;
    }

    // Assembly packet
    Packet_p = aPktBuffer_p;

    // + Message type
    *Packet_p++ = (uint8_t)(aMsgType >> 0);
    *Packet_p++ = (uint8_t)(aMsgType >> 8);

    // + Result
    *Packet_p++ = (uint8_t)(Result >> 0);
    *Packet_p++ = (uint8_t)(Result >> 8);

    // + BlockCount
    *Packet_p++ = (uint8_t)(BlockCount >> 0);
    *Packet_p++ = (uint8_t)(BlockCount >> 8);

    // + Address
    *Packet_p++ = (uint8_t)(Address >> 0);
    *Packet_p++ = (uint8_t)(Address >> 8);

    // + WriteCounter
    *Packet_p++ = (uint8_t)(WriteCounter >> 0);
    *Packet_p++ = (uint8_t)(WriteCounter >> 8);
    *Packet_p++ = (uint8_t)(WriteCounter >> 16);
    *Packet_p++ = (uint8_t)(WriteCounter >> 24);

    // + Nonce
    if (Nonce_p == NULL)
    {
        memset(Packet_p, 0, 16);
    }
    else
    {
        memcpy(Packet_p, Nonce_p, 16);
    }
    Packet_p += 16;

    // + Data
    if (Data_p == NULL)
    {
        memset(Packet_p, 0, 256);
    }
    else
    {
        memcpy(Packet_p, Data_p, DataSize);
        if (DataSize != 256)
        {
            memset((Packet_p + DataSize), 0, (256 - DataSize));
        }
    }
    Packet_p += 256;

    // + MAC
    if (Mac_p == NULL)
    {
        ValSymContextPtr_t SymContext_p = NULL;
        ValStatus_t Status;

        Status = val_SymAlloc(VAL_SYM_ALGO_MAC_HMAC_SHA256, VAL_SYM_MODE_NONE,
                              false, &SymContext_p);
        if (Status != VAL_SUCCESS)
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_SymAlloc()=%d.\n", (int)Status);
            return -3;
        }

        Status = val_SymInitKey(SymContext_p, VAL_ASSETID_INVALID,
                                gl_eMMCAuthKey, sizeof(gl_eMMCAuthKey));
        if (Status != VAL_SUCCESS)
        {
            (void)val_SymRelease(SymContext_p);
            LOG_CRIT(DA_PREFIX "FAILED: val_SymInitKey()=%d.\n", (int)Status);
            return -3;
        }

        MacSize = (256 / 8);
        Status = val_SymMacGenerate(SymContext_p,
                                    aPktBuffer_p,(ValSize_t)(Packet_p - aPktBuffer_p),
                                    Packet_p, &MacSize);
        if (Status != VAL_SUCCESS)
        {
            (void)val_SymRelease(SymContext_p);
            LOG_CRIT(DA_PREFIX "FAILED: val_SymMacGenerate()=%d.\n", (int)Status);
            return -3;
        }
    }
    else
    {
        memcpy(Packet_p, Mac_p, MacSize);
    }
    if (MacSize != 32)
    {
        memset((Packet_p + MacSize), 0, (32 - MacSize));
    }
    Packet_p += 32;

    // Pad/Stuff remaing packet
    memset(Packet_p, 0, 196);

    return 0;
}


/*----------------------------------------------------------------------------
 * eMMCModelHandleRequest
 *
 * This function handles an eMMC request packet and generates an response
 * based on the eMMC Model.
 */
static int
eMMCModelHandleRequest(
        uint8_t * aPktBuffer_p,
        uint32_t aPktBufferSize,
        uint8_t * aResponseBuffer_p,
        uint32_t aResponseBufferSize,
        uint16_t aResult)
{
    int rc = 0;
    uint16_t MsgType = 0;
    uint16_t Result = 0;
    uint16_t Address;
    uint32_t WriteCounter = 0;
    uint8_t * Data_p = NULL;
    uint32_t DataSize = 0;

    if ((aPktBuffer_p == NULL) || (aPktBufferSize < 512) ||
        (aResponseBuffer_p == NULL) || (aResponseBufferSize < 512))
    {
        LOG_CRIT(DA_PREFIX "FAILED: Packet/Response buffer too small\n");
        return -11;
    }

    if (aResult != 0xFFFF)
    {
        Result = aResult;
    }

    MsgType = (uint16_t)((aPktBuffer_p[1] << 8) + aPktBuffer_p[0]);
    switch (MsgType)
    {
    default:
        LOG_CRIT(DA_PREFIX "FAILED: Invalid request (%d).\n", (int)MsgType);
        return -12;

    case EMMC_MSGTYP_REQ_RD_WCNTR:
        memcpy(gl_eMMCModelNonce, &aPktBuffer_p[12], 16);
        rc = eMMCModelCreatePacket(EMMC_MSGTYP_RSP_RD_WCNTR,
                                   aResponseBuffer_p, aResponseBufferSize,
                                   Result, 1, 0,
                                   NULL, 0);
        break;

    case EMMC_MSGTYP_REQ_AUTH_DATA_WR:
        Address = (uint16_t)((aPktBuffer_p[7] << 8) + (aPktBuffer_p[6] << 0));
        WriteCounter = (uint32_t)((aPktBuffer_p[11] << 24) +
                                  (aPktBuffer_p[10] << 16) +
                                  (aPktBuffer_p[9]  <<  8) +
                                  (aPktBuffer_p[8]  <<  0));
        if (gl_eMMCModelWriteCounter == 0xFFFFFFFF)
        {
            LOG_CRIT(DA_PREFIX "WARNING: eMMC Model - Write counter expired\n");
            if (aResult == 0xFFFF)
            {
                Result = 0x85;          // Write failure
            }
        }
        else if (WriteCounter == gl_eMMCModelWriteCounter)
        {
            if ((aResult == 0) || (aResult == 0xFFFF))
            {
                if (Address < DA_EMMC_MODEL_MEMORY_ENTRIES)
                {
                    memcpy(gl_eMMCModelMemory[Address], &aPktBuffer_p[28], 256);
                }
                else
                {
                    Result = 0x4;           // Address failure
                }
            }
            gl_eMMCModelWriteCounter += 1;
        }
        else
        {
            LOG_CRIT(DA_PREFIX "WARNING: eMMC Model - Write counter expired\n");
            if (aResult != 0x3)
            {
                LOG_CRIT(DA_PREFIX "FAILED: eMMC Model - Write counter mismatch (%d != %d)\n",
                         WriteCounter, gl_eMMCModelWriteCounter);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "WARNING: eMMC Model - Write counter does not match (%d != %d)\n",
                         WriteCounter, gl_eMMCModelWriteCounter);
            }
            if (aResult == 0xFFFF)
            {
                Result = 0x3;           // Write failure
            }
        }
        rc = eMMCModelCreatePacket(EMMC_MSGTYP_RSP_AUTH_DATA_WR,
                                   aResponseBuffer_p, aResponseBufferSize,
                                   Result, 0, Address,
                                   NULL, 0);
        break;

    case EMMC_MSGTYP_REQ_AUTH_DATA_RD:
        Address = (uint16_t)((aPktBuffer_p[7] << 8) + aPktBuffer_p[6]);
        if ((aResult == 0) || (aResult == 0xFFFF))
        {
            if (Address < DA_EMMC_MODEL_MEMORY_ENTRIES)
            {
                Data_p = gl_eMMCModelMemory[Address];
                DataSize = 256;
            }
        }
        rc = eMMCModelCreatePacket(EMMC_MSGTYP_RSP_AUTH_DATA_RD,
                                   aResponseBuffer_p, aResponseBufferSize,
                                   Result, 1, Address,
                                   Data_p, DataSize);
        break;
    }

    return rc;
}


/*----------------------------------------------------------------------------
 * IncrementCounter
 *
 * This function increments a counter.
 */
static void
IncrementCounter(
        uint8_t * CounterData_p,
        ValSize_t Size)
{
    unsigned int i;

    for (i = 0; i < Size; i++)
    {
        CounterData_p[i] = (uint8_t)(CounterData_p[i] + 1);
        if (CounterData_p[i] != 0)
        {
            break;
        }
    }
}


/*----------------------------------------------------------------------------
 * eMMCCheckSequence
 *
 * This function checks the eMMC sequence and uses the eMMC Model as reference.
 */
static ValStatus_t
eMMCCheckSequence(void)
{
    ValStatus_t FuncRes;
    ValAssetId_t AuthKeyAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t StateAssetId = VAL_ASSETID_INVALID;
    uint8_t PacketBuffer[512];
    uint8_t ResponseBuffer[512];
    uint8_t * Data_p;
    ValSize_t Size;
    int rc;

    LOG_CRIT(DA_PREFIX "eMMC functions/sequence check\n");

    // Make sure that the TRNG is configured
    FuncRes = ConfigureTrng();
    if(FuncRes != VAL_SUCCESS)
    {
        return FuncRes;
    }

    // Get Authentication Key Asset ID
    FuncRes = val_AssetSearch(gl_eMMCAuthKey_AssetNumber, &AuthKeyAssetId, NULL);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetSearch(Authentication Key)=%d\n", (int)FuncRes);
        LOG_CRIT(DA_PREFIX "        Use '-emmcauthkey' to write it to OTP.\n");
        return FuncRes;
    }

    // eMMC read - get State Asset ID and Nonce
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadRequest(AuthKeyAssetId, &StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read verify - Get/Initialize eMMC item data
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_RD,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_SequenceChk, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadVerify(StateAssetId,
                                 ResponseBuffer, (28 + 256),
                                 &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read write counter request
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadWriteCounterRequest(StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read write counter verify - Get/Initialize write counter value
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_RD_WCNTR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 0, 0, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadWriteCounterVerify(StateAssetId,
                                             ResponseBuffer, (28 + 256),
                                             &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC write request
    Data_p = gl_eMMCModelMemory[0];
    if (*Data_p)                    // Check parameter length
    {
        // Increment four byte counter
        IncrementCounter(&Data_p[10], 4);
    }
    else
    {
        // Set parameter length to 4 (four byte counter) and initialize it to 0
        memset(Data_p, 0, 256);
        *Data_p = 4;
    }
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_WR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_SequenceChk, Data_p, 256);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    Size = (256 / 8);
    FuncRes = val_eMMCWriteRequest(StateAssetId,
                                   PacketBuffer, (28 + 256),
                                   &PacketBuffer[(28 + 256)], &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC write verify
    FuncRes = val_eMMCWriteVerify(StateAssetId,
                                  ResponseBuffer, (28 + 256),
                                  &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // State Asset is not valid anymore, so delete it
    FuncRes = val_AssetFree(StateAssetId);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetFree(StateAssetId)=%d\n", (int)FuncRes);
        return FuncRes;
    }
    StateAssetId =  VAL_ASSETID_INVALID;

    // eMMC read - get State Asset ID and Nonce
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadRequest(AuthKeyAssetId, &StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadRequest(2)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read verify
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_RD,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 0, gl_eMMCAddr_SequenceChk, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadVerify(StateAssetId,
                                 ResponseBuffer, (28 + 256),
                                 &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadVerify(2)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // State Asset is not needed anymore, so delete it
    FuncRes = val_AssetFree(StateAssetId);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetFree(StateAssetId2)=%d\n", (int)FuncRes);
    }

    return FuncRes;
}
#endif /* DA_VAL_SPECIAL_EMMC */


#ifdef DA_VAL_SPECIAL_HIBERNATION
/*----------------------------------------------------------------------------
 * Hibernation
 *
 * This function performs the sequence to go to hibernation mode.
 */
static ValStatus_t
Hibernation(void)
{
    ValStatus_t FuncRes = VAL_SUCCESS;
    ValAssetId_t AuthKeyAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t StateAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t KekAssetId = VAL_ASSETID_INVALID;
    ValPolicyMask_t AssetPolicy;
    eMMC_HibernationData HibernationData;
    uint8_t PacketBuffer[512];
    uint8_t ResponseBuffer[512];
    ValSize_t Size;
    int rc;
    eMMC_HibernationData * HiberData_p =
           (eMMC_HibernationData *)gl_eMMCModelMemory[gl_eMMCAddr_Hibernation];

    LOG_CRIT(DA_PREFIX "Hibernation sequence check\n");

    if (gl_HibernationDone)
    {
        LOG_CRIT(DA_PREFIX "FAILED: Please, perform Resume from Hibernation first\n");
        return VAL_INVALID_STATE;
    }

    if (HiberData_p->DecCounterLen == 0)
    {
        // Initialize hibernation data
        HiberData_p->DecCounterLen = 4;
        memcpy(HiberData_p->DecCounterPolicy, gl_eMMC_PolicyMonCntBI, sizeof(HiberData_p->DecCounterPolicy));
        HiberData_p->EncCounterLen = 4;
        memcpy(HiberData_p->EncCounterPolicy, gl_eMMC_PolicyMonCntBI, sizeof(HiberData_p->EncCounterPolicy));
        HiberData_p->eMmcAddrLen = 2;
        memcpy(HiberData_p->eMmcAddrPolicy, gl_eMMC_PolicyConstant, sizeof(HiberData_p->eMmcAddrPolicy));
        HiberData_p->eMmcAddrData[0] = gl_eMMCAddr_Hibernation;
        HiberData_p->ADTailLen = 48;
        memcpy(HiberData_p->ADTailPolicy, gl_eMMC_PolicyConstant, sizeof(HiberData_p->ADTailPolicy));
        memset(HiberData_p->ADTailData, 'a', sizeof(HiberData_p->ADTailData)); // some data
        HiberData_p->EndMarker = 0xFF;
    }

    // Make sure that the TRNG is configured and active.
    FuncRes = ConfigureTrng();
    if(FuncRes != VAL_SUCCESS)
    {
        return FuncRes;
    }

    // Get Authentication Key Asset ID
    FuncRes = val_AssetSearch(gl_eMMCAuthKey_AssetNumber, &AuthKeyAssetId, NULL);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetSearch(Authentication Key)=%d\n", (int)FuncRes);
        LOG_CRIT(DA_PREFIX "        Use '-emmcauthkey' to write it to OTP.\n");
        return FuncRes;
    }

    // eMMC read - get State Asset ID and Nonce
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadRequest(AuthKeyAssetId, &StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read verify - Get/Initialize eMMC item data
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_RD,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_Hibernation, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadVerify(StateAssetId,
                                 ResponseBuffer, (28 + 256),
                                 &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Copy eMMC hibernation data
    memcpy(&HibernationData, &ResponseBuffer[28], sizeof(HibernationData));

    // eMMC Read write counter request
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadWriteCounterRequest(StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read write counter verify - Get/Initialize write counter value
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_RD_WCNTR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 0, 0, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadWriteCounterVerify(StateAssetId,
                                             ResponseBuffer, (28 + 256),
                                             &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Increment the encryption monotonic counter and write it
    IncrementCounter(HibernationData.EncCounterData, HibernationData.EncCounterLen);

    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_WR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_Hibernation,
                               (uint8_t *)&HibernationData, sizeof(HibernationData));
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    Size = (256 / 8);
    FuncRes = val_eMMCWriteRequest(StateAssetId,
                                   PacketBuffer, (28 + 256),
                                   &PacketBuffer[(28 + 256)], &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC write verify
    FuncRes = val_eMMCWriteVerify(StateAssetId,
                                  ResponseBuffer, (28 + 256),
                                  &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Create Trusted Key Encryption Key (KEK) Asset
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_SYM_WRAP | EIP130_ASSET_POLICY_SCAWAESSIV | EIP130_ASSET_POLICY_SCDIRENCDEC;
#else
    AssetPolicy = VAL_POLICY_ENCRYPT | VAL_POLICY_DECRYPT | VAL_POLICY_TRUSTED_WRAP;
#endif /* EIP130_FW_ASSETPOLICY_V2 */
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }
    FuncRes = val_AssetAlloc(AssetPolicy, (512 / 8),
                             false, false,
                             VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                             &KekAssetId);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetAlloc(KEK)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Derive KEK from the HUK
    memcpy(PacketBuffer,
           HibernationData.EncCounterData, HibernationData.EncCounterLen);
    Size = HibernationData.EncCounterLen;
    memcpy(&PacketBuffer[Size],
           HibernationData.eMmcAddrData, HibernationData.eMmcAddrLen);
    Size += HibernationData.eMmcAddrLen;
    memcpy(&PacketBuffer[Size],
           HibernationData.ADTailData, HibernationData.ADTailLen);
    Size += HibernationData.ADTailLen;
    FuncRes = val_AssetLoadDerive(KekAssetId, val_AssetGetRootKey(),
                                  PacketBuffer, Size,
                                  false, false, NULL, 0, NULL, 0);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetLoadDerive(KEK)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Goto hibernation mode
    gl_DataBlobSize = sizeof(gl_DataBlobBuffer);
    FuncRes = val_SystemHibernation(StateAssetId, KekAssetId,
                                    PacketBuffer, Size,
                                    gl_DataBlobBuffer, &gl_DataBlobSize);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SystemHibernation()=%d\n", (int)FuncRes);
    }
    else
    {
        gl_HibernationDone = true;
        LOG_CRIT(DA_PREFIX "PASSED: Hardware is in Hibernation mode\n");

        LOG_CRIT(DA_PREFIX "Reset system to ensure hardware is in an initial state\n");
        FuncRes = DoReset();
    }

    return FuncRes;
}


/*----------------------------------------------------------------------------
 * ResumeFromHibernation
 *
 * This function performs the sequence to resume from hibernation mode.
 */
static ValStatus_t
ResumeFromHibernation(void)
{
    ValStatus_t FuncRes = VAL_SUCCESS;
    ValAssetId_t AuthKeyAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t StateAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t KekAssetId = VAL_ASSETID_INVALID;
    ValPolicyMask_t AssetPolicy;
    eMMC_HibernationData HibernationData;
    uint8_t PacketBuffer[512];
    uint8_t ResponseBuffer[512];
    ValSize_t Size;
    int rc;

    LOG_CRIT(DA_PREFIX "Resume From Hibernation sequence check\n");

    if (!gl_HibernationDone)
    {
        LOG_CRIT(DA_PREFIX "FAILED: Please, perform Hibernation first\n");
        return VAL_INVALID_STATE;
    }

    // Make sure that the TRNG is configured and active.
    FuncRes = ConfigureTrng();
    if(FuncRes != VAL_SUCCESS)
    {
        return FuncRes;
    }

    // Get Authentication Key Asset ID
    FuncRes = val_AssetSearch(gl_eMMCAuthKey_AssetNumber, &AuthKeyAssetId, NULL);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetSearch(Authentication Key)=%d\n", (int)FuncRes);
        LOG_CRIT(DA_PREFIX "        Use '-emmcauthkey' to write it to OTP.\n");
        return FuncRes;
    }

    // eMMC read - get State Asset ID and Nonce
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadRequest(AuthKeyAssetId, &StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read verify - Get/Initialize eMMC item data
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_RD,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_Hibernation, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadVerify(StateAssetId,
                                 ResponseBuffer, (28 + 256),
                                 &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Copy eMMC hibernation data
    memcpy(&HibernationData, &ResponseBuffer[28], sizeof(HibernationData));

    // eMMC Read write counter request
    Size = sizeof(gl_eMMCModelNonce);
    FuncRes = val_eMMCReadWriteCounterRequest(StateAssetId, gl_eMMCModelNonce, &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC Read write counter verify - Get/Initialize write counter value
    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_RD_WCNTR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 0, 0, NULL, 0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    FuncRes = val_eMMCReadWriteCounterVerify(StateAssetId,
                                             ResponseBuffer, (28 + 256),
                                             &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCReadWriteCounterVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Increment the decryption monotonic counter (must be equal to encryption
    // monotonic counter) and write it
    IncrementCounter(HibernationData.DecCounterData, HibernationData.DecCounterLen);

    rc = eMMCModelCreatePacket(EMMC_MSGTYP_REQ_AUTH_DATA_WR,
                               PacketBuffer, sizeof(PacketBuffer),
                               0, 1, gl_eMMCAddr_Hibernation,
                               (uint8_t *)&HibernationData, sizeof(HibernationData));
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    rc = eMMCModelHandleRequest(PacketBuffer, sizeof(PacketBuffer),
                                ResponseBuffer, sizeof(ResponseBuffer),
                                0);
    if (rc != 0)
    {
        return VAL_OPERATION_FAILED;
    }

    Size = (256 / 8);
    FuncRes = val_eMMCWriteRequest(StateAssetId,
                                   PacketBuffer, (28 + 256),
                                   &PacketBuffer[(28 + 256)], &Size);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteRequest()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // eMMC write verify
    FuncRes = val_eMMCWriteVerify(StateAssetId,
                                  ResponseBuffer, (28 + 256),
                                  &ResponseBuffer[(28 + 256)], (256 / 8));
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_eMMCWriteVerify()=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Create Trusted Key Encryption Key (KEK) Asset
#ifdef EIP130_FW_ASSETPOLICY_V2
    AssetPolicy = EIP130_ASSET_POLICY_SYM_WRAP | EIP130_ASSET_POLICY_SCAWAESSIV | EIP130_ASSET_POLICY_SCDIRENCDEC;
#else
    AssetPolicy = VAL_POLICY_ENCRYPT | VAL_POLICY_DECRYPT | VAL_POLICY_TRUSTED_WRAP;
#endif /* EIP130_FW_ASSETPOLICY_V2 */
    if (!val_IsAccessSecure())
    {
        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
    }
    FuncRes = val_AssetAlloc(AssetPolicy, (512 / 8),
                             false, false,
                             VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                             &KekAssetId);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetAlloc(KEK)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Derive KEK from the HUK
    memcpy(PacketBuffer,
           HibernationData.DecCounterData, HibernationData.DecCounterLen);
    Size = HibernationData.DecCounterLen;
    memcpy(&PacketBuffer[Size],
           HibernationData.eMmcAddrData, HibernationData.eMmcAddrLen);
    Size += HibernationData.eMmcAddrLen;
    memcpy(&PacketBuffer[Size],
           HibernationData.ADTailData, HibernationData.ADTailLen);
    Size += HibernationData.ADTailLen;
    FuncRes = val_AssetLoadDerive(KekAssetId, val_AssetGetRootKey(),
                                  PacketBuffer, Size,
                                  false, false, NULL, 0, NULL, 0);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_AssetLoadDerive(KEK)=%d\n", (int)FuncRes);
        return FuncRes;
    }

    // Resume from hibernation mode
    FuncRes = val_SystemResumeFromHibernation(StateAssetId, KekAssetId,
                                              PacketBuffer, Size,
                                              gl_DataBlobBuffer, gl_DataBlobSize);
    if(FuncRes != VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SystemResumeFromHibernation()=%d\n", (int)FuncRes);
    }
    else
    {
        gl_HibernationDone = false;
        LOG_CRIT(DA_PREFIX "PASSED: Resume hardware from Hibernation mode\n");

        FuncRes = GetVersion();
    }

    return FuncRes;
}
#endif /* DA_VAL_SPECIAL_HIBERNATION */


#ifdef DA_VAL_SPECIAL_SF_MILENAGE
/*----------------------------------------------------------------------------
 * MilenageXor
 *
 * This function performs a XOR of two values and returns the result.
 */
static void
MilenageXor(
        uint8_t * aValueResult,
        const uint8_t * aValue1,
        const uint8_t * aValue2,
        const uint32_t Size)
{
    uint32_t i;

    for (i = 0; i < Size; i++)
    {
        aValueResult[i] = aValue1[i] ^ aValue2[i];
    }
}

/*----------------------------------------------------------------------------
 * MilenageAutnCheck
 *
 * This function performs the Milenage AUTN verification related operations.
 */
static ValStatus_t
MilenageAutnCheck(void)
{
    uint8_t AUTN[sizeof(gl_Milenage_TS1_SQN) + sizeof(gl_Milenage_TS1_AMF) + sizeof(gl_Milenage_TS1_f1)];
    uint8_t RES[sizeof(gl_Milenage_TS1_f2)];
    uint8_t CK[sizeof(gl_Milenage_TS1_f3)];
    uint8_t IK[sizeof(gl_Milenage_TS1_f4)];
    ValStatus_t FuncRes = VAL_SUCCESS;
    ValAssetId_t SqnAdminAssetId = VAL_ASSETID_INVALID;
    ValAssetId_t KekAssetId = VAL_ASSETID_INVALID;

    LOG_CRIT(DA_PREFIX "Milenage AUTN verification check\n");

    // Format AUTN
    MilenageXor(AUTN,
                gl_Milenage_TS1_SQN, gl_Milenage_TS1_f5,
                sizeof(gl_Milenage_TS1_SQN));
    memcpy(&AUTN[sizeof(gl_Milenage_TS1_SQN)],
           gl_Milenage_TS1_AMF, sizeof(gl_Milenage_TS1_AMF));
    memcpy(&AUTN[sizeof(gl_Milenage_TS1_SQN) + sizeof(gl_Milenage_TS1_AMF)],
           gl_Milenage_TS1_f1, sizeof(gl_Milenage_TS1_f1));

    {
        uint8_t SQN[sizeof(gl_Milenage_TS1_SQN)];
        uint8_t AMF[sizeof(gl_Milenage_TS1_AMF)];

        FuncRes = val_SFMilenageAutnVerification(gl_Milenage_AssetNumber,
                                                 gl_Milenage_TS1_RAND, AUTN,
                                                 RES, CK, IK, SQN, AMF);
        if(FuncRes == VAL_SUCCESS)
        {
            if ((memcmp(RES, gl_Milenage_TS1_f2, sizeof(gl_Milenage_TS1_f2)) == 0) &&
                (memcmp(CK, gl_Milenage_TS1_f3, sizeof(gl_Milenage_TS1_f3)) == 0) &&
                (memcmp(IK, gl_Milenage_TS1_f4, sizeof(gl_Milenage_TS1_f4)) == 0) &&
                (memcmp(SQN, gl_Milenage_TS1_SQN, sizeof(gl_Milenage_TS1_SQN)) == 0) &&
                (memcmp(AMF, gl_Milenage_TS1_AMF, sizeof(gl_Milenage_TS1_AMF)) == 0))
            {
                LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageAutnVerification()=OK\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutnVerification()=Bad Result\n");
                return VAL_DATA_OVERRUN_ERROR;
            }
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutsGeneration()=%d\n", (int)FuncRes);
        }
    }

    FuncRes = val_SFMilenageSqnAdminCreate(gl_Milenage_AssetNumber, true,
                                           &SqnAdminAssetId);
    if(FuncRes == VAL_SUCCESS)
    {
        uint8_t AUTS[sizeof(gl_Milenage_TS1_SQN) + sizeof(gl_Milenage_TS1_f1star)];
        uint32_t EMMCause = 0;

        LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageSqnAdminCreate()=OK\n");

        FuncRes = val_SFMilenageAutnVerificationSqn(SqnAdminAssetId,
                                                    gl_Milenage_TS1_RAND, AUTN,
                                                    &EMMCause, RES, CK, IK, AUTS);
        if(FuncRes == VAL_SUCCESS)
        {
            int result1 = memcmp(RES, gl_Milenage_TS1_f2, sizeof(gl_Milenage_TS1_f2));
            int result2 = memcmp(CK, gl_Milenage_TS1_f3, sizeof(gl_Milenage_TS1_f3));
            int result3 = memcmp(IK, gl_Milenage_TS1_f4, sizeof(gl_Milenage_TS1_f4));
            if ((result1 == 0) && (result2 == 0) && (result3 == 0))
            {
                LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageAutnVerificationSqn()=OK\n");

                FuncRes = val_SFMilenageAutnVerificationSqn(SqnAdminAssetId,
                                                            gl_Milenage_TS1_RAND, AUTN,
                                                            &EMMCause, RES, CK, IK, AUTS);
                if((FuncRes == VAL_VERIFY_ERROR) && (EMMCause == 21))
                {
                    ValPolicyMask_t AssetPolicy;

                    LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageAutnVerificationSqn(EMMCause)=OK\n");

                    // Create Trusted Key Encryption Key (KEK) Asset
#ifdef EIP130_FW_ASSETPOLICY_V2
                    AssetPolicy = EIP130_ASSET_POLICY_SYM_WRAP | EIP130_ASSET_POLICY_SCAWAESSIV | EIP130_ASSET_POLICY_SCDIRENCDEC;
#else
                    AssetPolicy = VAL_POLICY_ENCRYPT | VAL_POLICY_DECRYPT | VAL_POLICY_TRUSTED_WRAP;
#endif /* EIP130_FW_ASSETPOLICY_V2 */
                    if (!val_IsAccessSecure())
                    {
                        AssetPolicy |= VAL_POLICY_SOURCE_NON_SECURE;
                    }
                    FuncRes = val_AssetAlloc(AssetPolicy, (512 / 8),
                                             false, false,
                                             VAL_ASSET_LIFETIME_INFINITE, false, false, 0,
                                             &KekAssetId);
                    if(FuncRes == VAL_SUCCESS)
                    {
                        static const uint8_t gl_AssocData[] = "Just some associated data for key derivation/datablob generation";

                        FuncRes = val_AssetLoadDerive(KekAssetId, val_AssetGetRootKey(),
                                                      gl_AssocData, sizeof(gl_AssocData)-1,
                                                      false, false, NULL, 0, NULL, 0);
                        if(FuncRes == VAL_SUCCESS)
                        {
                            gl_DataBlobSize = 300;
                            FuncRes = val_SFMilenageSqnAdminExport(SqnAdminAssetId, KekAssetId,
                                                                   gl_AssocData, sizeof(gl_AssocData)-1,
                                                                   gl_DataBlobBuffer, &gl_DataBlobSize);
                            if(FuncRes == VAL_SUCCESS)
                            {
                                if (gl_DataBlobSize == ((128/8) + 200))
                                {
                                    LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageSqnAdminExport()=OK\n");
                                }
                                else
                                {
                                    LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageSqnAdminExport() wrong blob size (%d != %d)\n",
                                             (int)gl_DataBlobSize, ((128/8) + 200));
                                }
                                FuncRes = val_SFMilenageSqnAdminReset(SqnAdminAssetId);
                                if(FuncRes == VAL_SUCCESS)
                                {
                                    LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageSqnAdminReset()=OK\n");
                                }
                                else
                                {
                                    LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageSqnAdminReset()=%d\n", (int)FuncRes);
                                }
                            }
                            else
                            {
                                LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageSqnAdminExport()=%d\n", (int)FuncRes);
                            }
                        }
                        else
                        {
                            LOG_CRIT(DA_PREFIX "FAILED: val_AssetLoadRandom(KEK)=%d\n", (int)FuncRes);
                        }
                    }
                    else
                    {
                        LOG_CRIT(DA_PREFIX "FAILED: val_AssetAlloc(KEK)=%d\n", (int)FuncRes);
                    }
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutnVerificationSqn(EMMCause)=%d (%d)\n",
                             (int)FuncRes, (int)EMMCause);
                }
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutnVerificationSqn()=Bad Result (%d,%d,%d)\n",
                         result1, result2, result3);
                FuncRes = VAL_DATA_OVERRUN_ERROR;
            }
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutnVerificationSqn()=%d\n", (int)FuncRes);
        }
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageSqnAdminCreate()=%d\n", (int)FuncRes);
    }

    if (SqnAdminAssetId != VAL_ASSETID_INVALID)
    {
        (void)val_SFMilenageSqnAdminRelease(SqnAdminAssetId); // Note: = val_AssetFree
    }
    if (KekAssetId != VAL_ASSETID_INVALID)
    {
        (void)val_AssetFree(KekAssetId);
    }

    return FuncRes;
}


/*----------------------------------------------------------------------------
 * MilenageAutsCheck
 *
 * This function performs the Milenage AUTS Generation.
 */
static ValStatus_t
MilenageAutsCheck(void)
{
    ValStatus_t FuncRes = VAL_SUCCESS;
    uint8_t AUTS[sizeof(gl_Milenage_TS1_SQN) + sizeof(gl_Milenage_TS1_f1star)];
    uint8_t SQNmsAK[sizeof(gl_Milenage_TS1_SQN)];

    FuncRes = val_SFMilenageAutsGeneration(gl_Milenage_AssetNumber,
                                           gl_Milenage_TS1_RAND,
                                           gl_Milenage_TS1_SQN, gl_Milenage_TS1_AMF,
                                           AUTS);
    if(FuncRes == VAL_SUCCESS)
    {
        int result1;
        int result2;

        MilenageXor(SQNmsAK,
                    gl_Milenage_TS1_SQN, gl_Milenage_TS1_f5star,
                    sizeof(gl_Milenage_TS1_SQN));
        result1 = memcmp(AUTS, SQNmsAK, sizeof(gl_Milenage_TS1_SQN));
        result2 = memcmp(&AUTS[sizeof(gl_Milenage_TS1_SQN)],
                         gl_Milenage_TS1_f1star, sizeof(gl_Milenage_TS1_f1star));
        if ((result1 == 0) && (result2 == 0))
        {
            LOG_CRIT(DA_PREFIX "PASSED: val_SFMilenageAutsGeneration=OK\n");
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutsGeneration=Bad Result (%d,%d)\n",
                     result1, result2);
            FuncRes = VAL_DATA_OVERRUN_ERROR;
        }
    }
    else
    {
        LOG_CRIT(DA_PREFIX "FAILED: val_SFMilenageAutsGeneration()=%d\n", (int)FuncRes);
    }

    return FuncRes;
}
#endif /* DA_VAL_SPECIAL_SF_MILENAGE */

#ifdef DA_VAL_SPECIAL_FIRMWARE_LOAD
/*----------------------------------------------------------------------------
 * read_file
 *
 * This is a helper function to load the Firmware image into memory
 */
static int
read_file(const char *filename, uint8_t **buffer, size_t *filesize)
{
    size_t ReadSize = 0;
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        LOG_CRIT(DA_PREFIX "ERROR: Could not open image file %s\n", filename);
        return 0;
    }

    fseek(file, 0L, SEEK_END);
    *filesize = (size_t)ftell(file);
    fseek(file, 0L, SEEK_SET);

    if (*filesize == 0)
    {
        LOG_CRIT(DA_PREFIX "ERROR: size of %s is 0\n", filename);
        return 0;
    }

    /* allocate the input buffer */
    *buffer = malloc(*filesize);
    if (!*buffer)
    {
        LOG_CRIT(DA_PREFIX "ERROR: Memory allocation (input)\n");
        return 0;
    }

    /* read data to buffer */
    ReadSize = fread(*buffer, 1, *filesize, file);
    if (ferror(file))
    {
        LOG_CRIT(DA_PREFIX "ERROR: Could not read file %s\n", filename);
        return 0;
    }

    if (*filesize != ReadSize)
    {
        LOG_CRIT(DA_PREFIX "ERROR: Error while reading file %s\n", filename);
        return 0;
    }
    fclose(file);
    return 1;
}


/*----------------------------------------------------------------------------
 * FirmwareCheck
 *
 * This function checks a firmware image trough the firmware check command
 */
static ValStatus_t
FirmwareCheck(const char *filename)
{
    ValStatus_t FuncRes;
    uint8_t * FirmwareImageBuf_p = NULL;
    size_t ImageSize = 0;

    if (read_file(filename, &FirmwareImageBuf_p, &ImageSize))
    {
        FuncRes = val_ServiceFirmwareCheck(FirmwareImageBuf_p, (uint32_t)ImageSize);
    }
    else
    {
        FuncRes = VAL_INTERNAL_ERROR;
    }
    free(FirmwareImageBuf_p);

    return FuncRes;
}
#endif


/*----------------------------------------------------------------------------
 * da_val_specials_main
 */
int
da_val_specials_main(
        int argc,
        char **argv)
{
    int i;

    if (Driver130_Init() < 0)
    {
        LOG_CRIT(DA_PREFIX "FAILED: Driver130_Init()\n");
        return -1;
    }

#ifdef DA_VAL_SPECIAL_EMMC
    // Initialize eMMC model
    for (i = 0; i < DA_EMMC_MODEL_MEMORY_ENTRIES; i++)
    {
        memset(gl_eMMCModelMemory[i], 0, 256);
    }
#endif /* DA_VAL_SPECIAL_EMMC */

    // Process option arguments
    for (i = 1; argc > i; i++)
    {
        ValStatus_t FuncRes = VAL_SUCCESS;

        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-help"))
        {
            LOG_CRIT(DA_PREFIX "Example application for special functionality provided by the VAL API.\n\n");
            LOG_CRIT("Syntax     : %s [<option> ...]\n", argv[0]);
            LOG_CRIT("Description: Executes a Demo Application that can perform special VAL API\n");
            LOG_CRIT("             operations. Note that you can use the options to perform an\n");
            LOG_CRIT("             operation to set the hardware in a certain state and the provided\n");
            LOG_CRIT("             operations (options) are executed in the provided order.\n");
            LOG_CRIT("Options    : -help              Display this information.\n");
#ifdef DA_VAL_SPECIAL_ASSET_STORE_RESET
            LOG_CRIT("             -asreset           Reset/Clear dynamic Asset Store\n");
#endif /* DA_VAL_SPECIAL_ASSET_STORE_RESET */
#ifdef DA_VAL_SPECIAL_EMMC
            LOG_CRIT("             -emmcauthkey       Write eMMC authentication key to OTP. Note that the\n");
            LOG_CRIT("                                OTP must be initialized with HUK and COID, and the\n");
            LOG_CRIT("                                EIP-130 firmware is reset to re-initialize the Static\n");
            LOG_CRIT("                                Asset references.\n");
            LOG_CRIT("             -emmccheck         Check the eMMC read and write functions based\n");
            LOG_CRIT("                                on the eMMC authentication key.\n");
#endif /* DA_VAL_SPECIAL_EMMC */
#ifdef DA_VAL_SPECIAL_SF_MILENAGE
            LOG_CRIT("             -milenageotp       Write Milenage K and OPc to OTP. Note that the OTP\n");
            LOG_CRIT("                                must be initialized with HUK and COID, and the EIP-130\n");
            LOG_CRIT("                                firmware is reset to re-initialize the Static Asset\n");
            LOG_CRIT("                                references.\n");
            LOG_CRIT("             -milenageautn      Check the Milenage AUTN verification sequence based\n");
            LOG_CRIT("                                on the Milenage K and OPc.\n");
            LOG_CRIT("             -milenageauts      Check the Milenage AUTS generation sequence based\n");
            LOG_CRIT("                                on the Milenage K and OPc.\n");
#endif /* DA_VAL_SPECIAL_SF_MILENAGE */
#ifdef EIP130_FW_ASSETPOLICY_V2
            LOG_CRIT("             -counterotp        Initialize an OTP monotonic counter with AssetNumber 33.\n");
            LOG_CRIT("                                Note that the OTP must be initialized with HUK and COID,\n");
            LOG_CRIT("                                and the EIP-130 firmware is reset to re-initialize the\n");
            LOG_CRIT("                                Static Asset references.\n");
#endif /* EIP130_FW_ASSETPOLICY_V2 */
            LOG_CRIT("             -counterinc[=<Assetnumber>]\n");
            LOG_CRIT("                                Increment the specified monotonic counter.\n");
            LOG_CRIT("                                The default AssetNumber is 33.\n");
            LOG_CRIT("             -initotp           Initialize the OTP with a default COID and HUK.\n");
#ifdef DA_VAL_SPECIAL_FIRMWARE_LOAD
            LOG_CRIT("                                Initializes SFWBVER to zero.\n");
#endif
            LOG_CRIT("                                After successful initialization of the COID and HUK, the\n");
            LOG_CRIT("                                EIP-130 firmware is reset to re-initialize the Static\n");
            LOG_CRIT("                                Asset references.\n");
            LOG_CRIT("             -provrndhuk128[=<Samples>]\n");
            LOG_CRIT("                                Initialize the OTP with a random 128-bit HUK\n");
            LOG_CRIT("                                based on AES-DRBG without CRC and default COID.\n");
            LOG_CRIT("                                There is no OTP key blob generated. Optionally\n");
            LOG_CRIT("                                <Samples> specify the TRNG cycle samples for\n");
            LOG_CRIT("                                the TRNG Configure/Start (default=3072).\n");
            LOG_CRIT("             -provrndhuk256[=<Samples>]\n");
            LOG_CRIT("                                Initialize the OTP with a random 256-bit HUK\n");
            LOG_CRIT("                                based on AES-DRBG with CRC and default COID.\n");
            LOG_CRIT("                                There is no OTP key blob generated. Optionally\n");
            LOG_CRIT("                                <Samples> specify the TRNG cycle samples for\n");
            LOG_CRIT("                                the TRNG Configure/Start (default=3072).\n");
#ifdef DA_VAL_SPECIAL_ARIA_DRBG
            LOG_CRIT("             -ariaprovrndhuk128[=<Samples>]\n");
            LOG_CRIT("                                Initialize the OTP with a random 128-bit HUK\n");
            LOG_CRIT("                                based on ARIA-DRBG without CRC and default COID.\n");
            LOG_CRIT("                                There is no OTP key blob generated. Optionally\n");
            LOG_CRIT("                                <Samples> specify the TRNG cycle samples for\n");
            LOG_CRIT("                                the TRNG Configure/Start (default=3072).\n");
            LOG_CRIT("             -ariaprovrndhuk256[=<Samples>]\n");
            LOG_CRIT("                                Initialize the OTP with a random 256-bit HUK\n");
            LOG_CRIT("                                based on ARIA-DRBG with CRC and default COID.\n");
            LOG_CRIT("                                There is no OTP key blob generated. Optionally\n");
            LOG_CRIT("                                <Samples> specify the TRNG cycle samples for\n");
            LOG_CRIT("                                the TRNG Configure/Start (default=3072).\n");
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */
            LOG_CRIT("             -sleep             Set hardware in Sleep mode.\n");
            LOG_CRIT("             -stopsleep         Resume hardware from Sleep mode.\n");
#ifdef DA_VAL_SPECIAL_HIBERNATION
            LOG_CRIT("             -hibernation       Set hardware in Hibernation mode.\n");
            LOG_CRIT("             -stophibernation   Resume hardware from hibernation mode.\n");
            LOG_CRIT("                                Note that this option can only be used in the\n");
            LOG_CRIT("                                combination '-hibernation -stophibernation'.\n");
#endif /* DA_VAL_SPECIAL_HIBERNATION */
            LOG_CRIT("             -hwmodulestatus    Get module status from hardware.\n");
            LOG_CRIT("             -hwoptions         Get hardware options.\n");
            LOG_CRIT("             -hwversion         Get hardware version information.\n");
            LOG_CRIT("             -selftest          Run self-test.\n");
#ifdef DA_VAL_SPECIAL_ENTERFIPSMODE
            LOG_CRIT("             -fips              Run Enter-FIPS-Mode command.\n");
#endif /* DA_VAL_SPECIAL_ENTERFIPSMODE */
            LOG_CRIT("             -system            System information.\n");
            LOG_CRIT("             -reset             Reset hardware.\n");
            LOG_CRIT("             -regread=<Register>\n");
            LOG_CRIT("                                Read an EIP-130 internal register, where <Register> must be\n");
            LOG_CRIT("                                a hexadecimal representation of an address (e.g. 0xF878).\n");
            LOG_CRIT("             -regwrite=<Register>,<Value>\n");
            LOG_CRIT("                                Write the specified <Value> to an EIP-130 internal register,\n");
            LOG_CRIT("                                where <Register> and <Value> must be a hexadecimal representation\n");
            LOG_CRIT("                                of an address or value (e.g. 0xF878).\n");
            LOG_CRIT("             -trng[=<Samples>]  Configure/Start the TRNG and enables an AES-DRBG.\n");
            LOG_CRIT("                                Optionally <Samples> specify the TRNG cycle\n");
            LOG_CRIT("                                samples (default=3072).\n");
            LOG_CRIT("             -trngrawdata[=<Blocks>]\n");
            LOG_CRIT("                                Read raw data from TRNG/NRBG.\n");
            LOG_CRIT("                                Optionally <Blocks> specify the number of data\n");
            LOG_CRIT("                                blocks (default=40), which is <Blocks>*256 bytes.\n");
#ifdef DA_VAL_SPECIAL_ARIA_DRBG
            LOG_CRIT("             -ariadrbg[=<Samples>]\n");
            LOG_CRIT("                                Configure/Start the TRNG and enables an ARIA-DRBG.\n");
            LOG_CRIT("                                Optionally <Samples> specify the TRNG cycle\n");
            LOG_CRIT("                                samples (default=3072).\n");
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */
#ifdef DA_VAL_SPECIAL_DRBG_FORCESEED
            LOG_CRIT("             -drbgforceseed     Initialize the DRBG with a seed. Note that this\n");
            LOG_CRIT("                                command disables FROs use and reseeding.\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            LOG_CRIT("                                Used Seed=0xD867E0F56663E88117E778242E386389\n");
            LOG_CRIT("                                            49DB62FEA94434603795EB720B96EAA0\n");
            LOG_CRIT("                                            99DEC05EE905410BE4277B254E365D7E\n");
#else
            LOG_CRIT("                                Used Seed=0x635334C4CBDEC46E34B32096F0371198\n");
            LOG_CRIT("                                            DD4B395B443B24208BC935E6338A332B\n");
            LOG_CRIT("                                            ED88B88CDAA0CD174A5B0932A22E6E18\n");
#endif /* EIP130_FW_ASSETPOLICY_V2 */
#endif /* DA_VAL_SPECIAL_DRBG_FORCESEED */
#ifdef DA_VAL_SPECIAL_FIRMWARE_LOAD
            LOG_CRIT("             -sfwbotp           Write SFWBPKD, SFWBCR and SFWBVER to OTP. Note that\n");
            LOG_CRIT("                                the OTP must be initialized with HUK and COID, and the\n");
            LOG_CRIT("                                EIP-130 firmware is reset to re-initialize the Static\n");
            LOG_CRIT("                                Asset references.\n");
            LOG_CRIT("             -firmwarecheck=filename\n");
            LOG_CRIT("                                Run a firmware check on the specified firmware image file.\n");
            LOG_CRIT("             -updaterollbackid=RollbackId\n");
            LOG_CRIT("                                Update the RollbackId (SFWBVER).\n");
#endif
            LOG_CRIT("             -zerootp           Zeroize (destroy) OTP.\n");
            continue;
        }

        if (strcmp(argv[i], "-reset") == 0)
        {
            // Reset system to have known starting state
            LOG_CRIT(DA_PREFIX "Reset system\n");
            FuncRes = DoReset();
        }
        else if (strncmp(argv[i], "-regread=", 9) == 0)
        {
            char *EndPtr_p;
            uint32_t RegAddress = (uint32_t)strtoull(&argv[i][9], &EndPtr_p, 0);
            uint32_t RegValue = 0;

            if ((EndPtr_p == argv[i + 1]) || (*EndPtr_p != '\0') ||
                (RegAddress > 0x0fffc))
            {
                LOG_CRIT(DA_PREFIX "FAILED: Invalid register address\n");
                break;
            }

            FuncRes = val_ServiceRegisterRead(RegAddress, &RegValue);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: RegisterRead(0x%x)=0x%x\n", RegAddress, RegValue);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_ServiceRegisterRead()=%d\n", (int)FuncRes);
            }
        }
        else if (strncmp(argv[i], "-regwrite=", 10) == 0)
        {
            char *EndPtr_pR;
            char *EndPtr_pV;
            uint32_t RegAddress = (uint32_t)strtoull(&argv[i][10], &EndPtr_pR, 0);
            uint32_t RegValue = 0;

            if ((*EndPtr_pR != ',') || (RegAddress > 0x0fffc))
            {
                LOG_CRIT(DA_PREFIX "FAILED: Invalid register address\n");
                break;
            }
            EndPtr_pR++;
            RegValue = (uint32_t)strtoull(EndPtr_pR, &EndPtr_pV, 0);
            if ((EndPtr_pV == argv[i + 1]) || (*EndPtr_pV != '\0'))
            {
                LOG_CRIT(DA_PREFIX "FAILED: Invalid register value\n");
                break;
            }

            FuncRes = val_ServiceRegisterWrite(RegAddress, RegValue);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: RegisterWrite(0x%x)=0x%x\n", RegAddress, RegValue);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_ServiceRegisterWrite()=%d\n", (int)FuncRes);
            }
        }
        else if (strncmp(argv[i], "-trngrawdata", 12) == 0)
        {
            uint16_t blocks = 40;

            if (argv[i][12] == '=')
            {
                blocks = (uint16_t)atoi(&argv[i][13]);
            }
            FuncRes = GetTrngRawData(blocks);
        }
        else if (strncmp(argv[i], "-trng", 5) == 0)
        {
            if (argv[i][5] == '=')
            {
                gl_SampleCycles = (uint16_t)atoi(&argv[i][6]);
                if (gl_SampleCycles == 0)
                {
                    gl_SampleCycles = 2;
                }
            }
            FuncRes = ConfigureTrng();
        }
#ifdef DA_VAL_SPECIAL_ARIA_DRBG
        else if (strncmp(argv[i], "-ariadrbg", 9) == 0)
        {
            if (argv[i][9] == '=')
            {
                gl_SampleCycles = (uint16_t)atoi(&argv[i][10]);
                if (gl_SampleCycles == 0)
                {
                    gl_SampleCycles = 2;
                }
            }
            FuncRes = ConfigureAriaDrbg();
        }
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */
#ifdef DA_VAL_SPECIAL_DRBG_FORCESEED
        else if (strcmp(argv[i], "-drbgforceseed") == 0)
        {
            FuncRes = ConfigureDrbgForceSeed();
        }
#endif /* DA_VAL_SPECIAL_DRBG_FORCESEED */
        else if (strcmp(argv[i], "-initotp") == 0)
        {
            LOG_CRIT(DA_PREFIX "Program default HUK and Crypto Officer ID in OTP\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = WriteOTP(VAL_ASSETNUMBER_HUK,
                               EIP130_ASSET_POLICY_NONMODIFIABLE |
                               (VAL_ASSETNUMBER_HUK << 1) |
                               EIP130_ASSET_POLICY_NODOMAIN |
                               EIP130_ASSET_POLICY_PRIVATEDATA |
                               EIP130_ASSET_POLICY_GDHUK,
                               true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                               gl_HUK_OTPKeyBlob, sizeof(gl_HUK_OTPKeyBlob));
#else
            FuncRes = WriteOTP(0, 1, true,
                               gl_ADLabel, sizeof(gl_ADLabel) - 1,
                               gl_HUK_OTPKeyBlob, sizeof(gl_HUK_OTPKeyBlob));
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        }
#ifdef DA_VAL_SPECIAL_ASSET_STORE_RESET
        else if (strcmp(argv[i], "-asreset") == 0)
        {
            LOG_CRIT(DA_PREFIX "Reset/Clear dynamic Asset Store\n");
            FuncRes = val_AssetStoreReset();
            if(FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_AssetStoreReset()=%d\n", (int)FuncRes);
                break;
            }
        }
#endif /* DA_VAL_SPECIAL_ASSET_STORE_RESET */
        else if (strncmp(argv[i], "-provrndhuk128", 14) == 0)
        {
            LOG_CRIT(DA_PREFIX "Program random 128-bit HUK (no CRC&KeyBlob) and default Crypto Officer ID in OTP (AES-DRBG)\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = WriteRandomHuk(VAL_ASSETNUMBER_HUK, true, true, &argv[i][14], false);
#else
            FuncRes = WriteRandomHuk(0, true, true, &argv[i][14], false);
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        }
        else if (strncmp(argv[i], "-provrndhuk256", 14) == 0)
        {
            LOG_CRIT(DA_PREFIX "Program random 256-bit HUK (CRC&KeyBlob) and default Crypto Officer ID in OTP (AES-DRBG)\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = WriteRandomHuk(VAL_ASSETNUMBER_HUK, false, true, &argv[i][14], false);
#else
            FuncRes = WriteRandomHuk(0, false, true, &argv[i][14], false);
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        }
#ifdef DA_VAL_SPECIAL_ARIA_DRBG
        else if (strncmp(argv[i], "-ariaprovrndhuk128", 14) == 0)
        {
            LOG_CRIT(DA_PREFIX "Program random 128-bit HUK (no CRC&KeyBlob) and default Crypto Officer ID in OTP (ARIA-DRBG)\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = WriteRandomHuk(VAL_ASSETNUMBER_HUK, true, true, &argv[i][14], true);
#else
            FuncRes = WriteRandomHuk(0, true, true, &argv[i][14], true);
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        }
        else if (strncmp(argv[i], "-ariaprovrndhuk256", 14) == 0)
        {
            LOG_CRIT(DA_PREFIX "Program random 256-bit HUK (CRC&KeyBlob) and default Crypto Officer ID in OTP (ARIA-DRBG)\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = WriteRandomHuk(VAL_ASSETNUMBER_HUK, false, true, &argv[i][14], true);
#else
            FuncRes = WriteRandomHuk(0, false, true, &argv[i][14], true);
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        }
#endif /* DA_VAL_SPECIAL_ARIA_DRBG */
#ifdef EIP130_FW_ASSETPOLICY_V2
        else if (strcmp(argv[i], "-counterotp") == 0)
        {
            ValAssetNumber_t AssetNumber = 33;

            FuncRes = WriteOTP(AssetNumber,
                               EIP130_ASSET_POLICY_MODIFIABLE |
                               ((ValPolicyMask_t)AssetNumber << 1) |
                               EIP130_ASSET_POLICY_NODOMAIN |
                               EIP130_ASSET_POLICY_PUBLICDATA |
                               EIP130_ASSET_POLICY_GENERICDATA |
                               EIP130_ASSET_POLICY_GDMONOTONIC,
                               true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                               gl_monotoniccounter_keyblob, sizeof(gl_monotoniccounter_keyblob));
            if (FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: Init OTP monotonic counter (OTPwrite=%d)\n", (int)FuncRes);
                break;
            }
        }
#endif /* EIP130_FW_ASSETPOLICY_V2 */
        else if (strncmp(argv[i], "-counterinc", 11) == 0)
        {
            ValAssetNumber_t AssetNumber;
            ValAssetId_t AssetId = VAL_ASSETID_INVALID;
            uint8_t DataBuffer1[16];
            uint8_t DataBuffer2[16];
            ValSize_t DataSize;

            if (argv[i][11] == '=')
            {
                char *EndPtr_p;

                AssetNumber = (ValAssetNumber_t)strtoull(&argv[i][12], &EndPtr_p, 0);
                if ((EndPtr_p == argv[i + 1]) || (*EndPtr_p != '\0') ||
                    (AssetNumber == 0) || (AssetNumber > VAL_ASSET_NUMBER_MAX))
                {
                    LOG_CRIT(DA_PREFIX "FAILED: Invalid AssetNumber\n");
                    break;
                }
            }
            else
            {
                AssetNumber = 33;
            }

            LOG_CRIT(DA_PREFIX "Monotonic Counter increment (AssetNumber=%u)\n",
                     AssetNumber);

            // Get Asset ID of the Monotonic Counter
            FuncRes = val_AssetSearch(AssetNumber, &AssetId, &DataSize);
            if(FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_AssetSearch()=%d\n", (int)FuncRes);
                break;
            }

            FuncRes = val_MonotonicCounterRead(AssetId, DataBuffer1, DataSize);
            if(FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_MonotonicCounterRead(Before)=%d\n", (int)FuncRes);
                break;
            }

            FuncRes = val_MonotonicCounterIncrement(AssetId);
            if(FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_MonotonicCounterIncrement()=%d\n", (int)FuncRes);
                break;
            }

            FuncRes = val_MonotonicCounterRead(AssetId, DataBuffer2, DataSize);
            if(FuncRes == VAL_SUCCESS)
            {
                Log_HexDump("Before increment:", 0, DataBuffer1, (unsigned int)DataSize);
                Log_HexDump("After increment :", 0, DataBuffer2, (unsigned int)DataSize);
                if (memcmp(DataBuffer1, DataBuffer2, DataSize) != 0)
                {
                    LOG_CRIT(DA_PREFIX "PASSED: Monotonic Counter increment\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "FAILED: Monotonic Counter increment\n");
                    break;
                }
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_MonotonicCounterRead(After)=%d\n", (int)FuncRes);
            }
        }
        else if (strcmp(argv[i], "-zerootp") == 0)
        {
            LOG_CRIT(DA_PREFIX "Zeroize (destroy) OTP\n");
            FuncRes = val_ServiceSelectOTPZeroize();
            if(FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_ServiceSelectOTPZeroize()=%d\n", (int)FuncRes);
                break;
            }

            FuncRes = val_ServiceZeroizeOTP();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: OTP Zeroize\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_ServiceZeroizeOTP()=%d\n", (int)FuncRes);
            }
        }
#ifdef DA_VAL_SPECIAL_EMMC
        else if (strcmp(argv[i], "-emmcauthkey") == 0)
        {
            ValAssetId_t AuthKeyAssetId = VAL_ASSETID_INVALID;

            LOG_CRIT(DA_PREFIX "Program eMMC Authentication Key in OTP\n");

            // Check if Authentication Key is already written
            FuncRes = val_AssetSearch(gl_eMMCAuthKey_AssetNumber, &AuthKeyAssetId, NULL);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "SKIPPED: eMMC Authentication Key already exists\n");
            }
            else
            {
                FuncRes = WriteOTP(gl_eMMCAuthKey_AssetNumber, 6, true,
                                   gl_ADLabel, sizeof(gl_ADLabel)-1,
                                   gl_eMMCAuthKey_OTPKeyBlob, sizeof(gl_eMMCAuthKey_OTPKeyBlob));
            }
        }
        else if (strcmp(argv[i], "-emmccheck") == 0)
        {
            FuncRes = eMMCCheckSequence();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: eMMC functions/sequence check\n");
            }
        }
#endif /* DA_VAL_SPECIAL_EMMC */
        else if (strcmp(argv[i], "-sleep") == 0)
        {
            FuncRes = val_SystemSleep();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: hardware is in Sleep mode\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SystemSleep()=%d\n", (int)FuncRes);
            }
        }
        else if (strcmp(argv[i], "-stopsleep") == 0)
        {
            FuncRes = val_SystemResumeFromSleep();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: Resume hardware from Sleep mode\n");

                FuncRes = GetVersion();
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SystemResumeFromSleep()=%d\n", (int)FuncRes);
            }
        }
#ifdef DA_VAL_SPECIAL_HIBERNATION
        else if (strcmp(argv[i], "-hibernation") == 0)
        {
            FuncRes = Hibernation();
        }
        else if (strcmp(argv[i], "-stophibernation") == 0)
        {
            FuncRes = ResumeFromHibernation();
        }
#endif /* DA_VAL_SPECIAL_HIBERNATION */
        else if (strcmp(argv[i], "-selftest") == 0)
        {
            LOG_CRIT(DA_PREFIX "Running self-test\n");
            FuncRes = val_SystemSelfTest();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: self-test Passed\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SystemSelfTest()=%d\n", (int)FuncRes);
            }
        }
#ifdef DA_VAL_SPECIAL_ENTERFIPSMODE
        else if (strcmp(argv[i], "-fips") == 0)
        {
            LOG_CRIT(DA_PREFIX "Running Enter-FIPS-Mode\n");
            FuncRes = val_SystemEnterFipsMode();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: Enter-FIPS-Mode Passed\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SystemEnterFipsMode()=%d\n", (int)FuncRes);
            }
        }
#endif /* DA_VAL_SPECIAL_ENTERFIPSMODE */
        else if (strcmp(argv[i], "-system") == 0)
        {
            uint8_t OtpErrorCode = 0;
            uint16_t OtpErrorLocation = 0;
            uint8_t Mode = 0;
            uint8_t ErrorTest = 0;
            uint8_t CryptoOfficer = 0;
            uint8_t HostID = 0;
            uint8_t NonSecure = 0;
            uint32_t Identity = 0;

            FuncRes = val_SystemGetState(&OtpErrorCode, &OtpErrorLocation,
                                         &Mode, &ErrorTest, &CryptoOfficer,
                                         &HostID, &NonSecure, &Identity);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: System State read\n");
                LOG_CRIT(DA_PREFIX "OTP State      : %u (%u)\n", OtpErrorCode, OtpErrorLocation);
                LOG_CRIT(DA_PREFIX "Mode           : %u (0x%02X)\n", Mode, ErrorTest);
                LOG_CRIT(DA_PREFIX "CryptoOfficerId: %s\n", CryptoOfficer ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "HostId         : %u\n", HostID);
                LOG_CRIT(DA_PREFIX "Secure         : %s\n", NonSecure ? "No" : "Yes");
                LOG_CRIT(DA_PREFIX "Identity       : 0x%X\n", Identity);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_SystemGetState()=%d\n", (int)FuncRes);
            }
        }
        else if (strcmp(argv[i], "-hwmodulestatus") == 0)
        {
            uint8_t FIPSmode;
            uint8_t NonFIPSmode;
            uint8_t CRC24Ok;
            uint8_t CRC24Busy;
            uint8_t CRC24Error;
            uint8_t FwImageWritten;
            uint8_t FwImageCheck;
            uint8_t FwImageAccepted;
            uint8_t FatalError;

            FuncRes = val_HW_ModuleStatus(&FIPSmode, &NonFIPSmode, &FatalError,
                                          &CRC24Ok, &CRC24Busy, &CRC24Error,
                                          &FwImageWritten, &FwImageCheck, &FwImageAccepted);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: Hardware Module Status read\n");
                LOG_CRIT(DA_PREFIX "FIPS mode                : %sctive\n", FIPSmode ? "A" : "Ina");
                LOG_CRIT(DA_PREFIX "non-FIPS mode            : %sctive\n", NonFIPSmode ? "A" : "Ina");
                LOG_CRIT(DA_PREFIX "Fatal error              : %s\n", FatalError ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "CRC24 Ok                 : %s\n", CRC24Ok ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "CRC24 busy               : %s\n", CRC24Busy ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "CRC24 error              : %s\n", CRC24Error ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "Firmware image written   : %s\n", FwImageWritten ? "Yes" : "No");
#ifdef EIP130_HW_V3
                LOG_CRIT(DA_PREFIX "Firmware image check busy: %s\n", FwImageCheck ? "Yes" : "No");
#else
                LOG_CRIT(DA_PREFIX "Firmware image check done: %s\n", FwImageCheck ? "Yes" : "No");
#endif /* EIP130_HW_V3 */
                LOG_CRIT(DA_PREFIX "Firmware image accepted  : %s\n", FwImageAccepted ? "Yes" : "No");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_HW_ModuleStatus()=%d\n", (int)FuncRes);
            }
        }
        else if (strcmp(argv[i], "-hwoptions") == 0)
        {
            uint8_t nMailboxes;
            uint16_t MailboxSize;
            uint8_t HostId;
            uint8_t SecureHostId;
            uint8_t MasterId;
            uint8_t MyHostId;
            uint8_t ProtectionAvailable;
            uint8_t Protection;
            uint16_t StandardEngines;
            uint16_t CustomEngines;

            FuncRes = val_HW_EIP_Options(&nMailboxes, &MailboxSize,
                                         &HostId, &SecureHostId, &MasterId, &MyHostId,
                                         &ProtectionAvailable, &Protection,
                                         &StandardEngines, &CustomEngines);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: Hardware EIP Options read\n");
                LOG_CRIT(DA_PREFIX "Number of mailboxes   : %d\n", nMailboxes);
                LOG_CRIT(DA_PREFIX "Mailbox size in bytes : %d\n", MailboxSize);
                LOG_CRIT(DA_PREFIX "HostId                : 0x%X\n", HostId);
                LOG_CRIT(DA_PREFIX "SecureHostId          : 0x%X\n", SecureHostId);
                LOG_CRIT(DA_PREFIX "MasterId              : %d\n", MasterId);
                LOG_CRIT(DA_PREFIX "MyHostId              : %d\n", MyHostId);
                LOG_CRIT(DA_PREFIX "Protection available  : %s\n", ProtectionAvailable ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "Protection set        : %s\n", Protection ? "Yes" : "No");
                LOG_CRIT(DA_PREFIX "Standard Engines      : 0x%04X\n", StandardEngines);
                LOG_CRIT(DA_PREFIX "Custom Engines        : 0x%04X\n", CustomEngines);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_HW_EIP_Options()=%d\n", (int)FuncRes);
            }
        }
        else if (strcmp(argv[i], "-hwversion") == 0)
        {
            uint8_t EipNumber;
            uint8_t MajorVersion;
            uint8_t MinorVersion;
            uint8_t PatchLevel;

            FuncRes = val_HW_EIP_Version(&MajorVersion, &MinorVersion,
                                         &PatchLevel, &EipNumber);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "PASSED: Hardware EIP version read\n");
                LOG_CRIT(DA_PREFIX "EIP HW number    : 0x%X (%d)\n", EipNumber, EipNumber);
                LOG_CRIT(DA_PREFIX "HW major version : %d\n", MajorVersion);
                LOG_CRIT(DA_PREFIX "HW minor version : %d\n", MinorVersion);
                LOG_CRIT(DA_PREFIX "HW patch level   : %d\n", PatchLevel);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "FAILED: val_HW_EIP_Version()=%d\n", (int)FuncRes);
            }
        }
#ifdef DA_VAL_SPECIAL_SF_MILENAGE
        else if (strcmp(argv[i], "-milenageotp") == 0)
        {
            ValAssetId_t AssetId = VAL_ASSETID_INVALID;

            LOG_CRIT(DA_PREFIX "Program Milenage K and OPc in OTP\n");

            // Check if Milenage K and OPc is already written
            FuncRes = val_AssetSearch(gl_Milenage_AssetNumber, &AssetId, NULL);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "SKIPPED: Milenage K and OPc already exists\n");
            }
            else
            {
#ifdef EIP130_FW_ASSETPOLICY_V2
                FuncRes = WriteOTP(gl_Milenage_AssetNumber,
                                   EIP130_ASSET_POLICY_NONMODIFIABLE |
                                   ((ValPolicyMask_t)gl_Milenage_AssetNumber << 1) |
                                   EIP130_ASSET_POLICY_PRIVATEDATA |
                                   EIP130_ASSET_POLICY_NODOMAIN |
                                   EIP130_ASSET_POLICY_GDSPECIALFUNC |
                                   EIP130_ASSET_POLICY_GDSF_MILENAGEK_OPC,
                                   true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                                   gl_Milenage_TS1_OTPBlob, sizeof(gl_Milenage_TS1_OTPBlob));
#else
                FuncRes = WriteOTP(gl_Milenage_AssetNumber, 7, true,
                                   gl_ADLabel, sizeof(gl_ADLabel) - 1,
                                   gl_Milenage_TS1_OTPBlob, sizeof(gl_Milenage_TS1_OTPBlob));
#endif /* EIP130_FW_ASSETPOLICY_V2 */
            }
        }
        else if (strcmp(argv[i], "-milenageautn") == 0)
        {
            FuncRes = MilenageAutnCheck();
        }
        else if (strcmp(argv[i], "-milenageauts") == 0)
        {
            FuncRes = MilenageAutsCheck();
        }
#endif /* DA_VAL_SPECIAL_SF_MILENAGE */
#ifdef DA_VAL_SPECIAL_FIRMWARE_LOAD
        else if (strcmp(argv[i], "-sfwbotp") == 0)
        {
            ValAssetId_t AssetId = VAL_ASSETID_INVALID;

            LOG_CRIT(DA_PREFIX "Program SFWBPKD, SFWBCR and SFWBVER in OTP\n");

            // Check if SFWBPKD is already written
            FuncRes = val_AssetSearch(VAL_ASSETNUMBER_SFWBPKD, &AssetId, NULL);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "SKIPPED: SFWBPKD already exists\n");
            }
            else
            {
                // Write SFWBVER
                FuncRes = WriteOTP(VAL_ASSETNUMBER_SFWBPKD,
                                   EIP130_ASSET_POLICY_NONMODIFIABLE |
                                   (VAL_ASSETNUMBER_SFWBPKD << 1) |
                                   EIP130_ASSET_POLICY_NODOMAIN |
                                   EIP130_ASSET_POLICY_PUBLICDATA |
                                   EIP130_ASSET_POLICY_GENERICDATA |
                                   EIP130_ASSET_POLICY_GDPUBLICDATA,
                                   true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                                   gl_SFWBPKD_keyblob, sizeof(gl_SFWBPKD_keyblob));
                if(FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "SFWBPKD written\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "SFWBPKD not written\n");
                }
            }

            // Check if SFWBCR is already written
            FuncRes = val_AssetSearch(VAL_ASSETNUMBER_SFWBCR, &AssetId, NULL);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "SKIPPED: SFWBCR already exists\n");
            }
            else
            {
                // Write SFWBCR
                FuncRes = WriteOTP(VAL_ASSETNUMBER_SFWBCR,
                                   EIP130_ASSET_POLICY_NONMODIFIABLE |
                                   (VAL_ASSETNUMBER_SFWBCR << 1) |
                                   EIP130_ASSET_POLICY_NODOMAIN |
                                   EIP130_ASSET_POLICY_PRIVATEDATA |
                                   EIP130_ASSET_POLICY_SYMCRYPTO |
                                   EIP130_ASSET_POLICY_SCUIWRAP |
                                   EIP130_ASSET_POLICY_SCDIRDECVRFY |
                                   EIP130_ASSET_POLICY_SCACAES |
                                   EIP130_ASSET_POLICY_SCAWKEY,
                                   true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                                   gl_SFWBCR_keyblob, sizeof(gl_SFWBCR_keyblob));
                if(FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "SFWBCR written\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "SFWBCR not written\n");
                }
            }

            // Check if SFWBVER is already written
            FuncRes = val_AssetSearch(VAL_ASSETNUMBER_SFWBVER, &AssetId, NULL);
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "SKIPPED: SFWBVER already exists\n");
            }
            else
            {
                // Write SFWBVER
                FuncRes = WriteOTP(VAL_ASSETNUMBER_SFWBVER,
                                   EIP130_ASSET_POLICY_MODIFIABLE |
                                   (VAL_ASSETNUMBER_SFWBVER << 1) |
                                   EIP130_ASSET_POLICY_NODOMAIN |
                                   EIP130_ASSET_POLICY_PUBLICDATA |
                                   EIP130_ASSET_POLICY_GENERICDATA |
                                   EIP130_ASSET_POLICY_GDMONOTONIC,
                                   true, gl_ADLabel, sizeof(gl_ADLabel) - 1,
                                   gl_SFWBVER_keyblob, sizeof(gl_SFWBVER_keyblob));
                if(FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "SFWBVER written\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "SFWBVER not written\n");
                }
            }
        }
        else if(strncmp(argv[i], "-firmwarecheck", 14) == 0)
        {
            if (argv[i][14] == '=')
            {
                FuncRes = FirmwareCheck(&argv[i][15]);
                if(FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "PASSED: FirmwareImage is valid\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "FAILED: FirmwareCheck()=%d\n", (int)FuncRes);
                }
            }
        }
        else if(strncmp(argv[i], "-updaterollbackid", 17) == 0)
        {
            if (argv[i][17] == '=')
            {
                char *EndPtr_p;
                uint16_t RollbackId = 0;
                RollbackId = (uint16_t)strtoull(&argv[i][18], &EndPtr_p, 0);
                if (RollbackId > 33)
                {
                    LOG_CRIT(DA_PREFIX "FAILED: Invalid RollbackID\n");
                    break;
                }
                FuncRes = val_ServiceUpdateRollbackID(RollbackId);
                if (FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "PASSED: RollbackID updated to %d\n", RollbackId);
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "FAILED: val_ServiceUpdateRollbackID()=%d\n", (int)FuncRes);
                }
            }
        }

#endif /* DA_VAL_SPECIAL_FIRMWARE_LOAD */
        if(FuncRes != VAL_SUCCESS)
        {
            // Always exit on an error
            break;
        }
    }

    Driver130_Exit();

    return 0;
}


#ifndef DA_VAL_MAIN_REMOVE
/*----------------------------------------------------------------------------
 * main
 *
 * Program entry.
 */
int
main(
        int argc,
        char **argv)
{
    return da_val_specials_main(argc, argv);
}
#endif // !DA_VAL_MAIN_REMOVE


/* end of file da_val_specials.c */


