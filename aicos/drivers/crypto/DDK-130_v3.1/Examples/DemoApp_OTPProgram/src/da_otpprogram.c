/* da_otpprogram.c
 *
 * Demo Application intended as an OTP programming use example.
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

#include "da_otpprogram.h"              // da_otpprogram_main()


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_da_otpprogram.h"            // configuration

#include "log.h"
#include "api_driver_init.h"
#include "api_val.h"

#ifndef DA_OTPPROGRAM_KEYBLOB_FS_REMOVE
#include <stdio.h>      // fopen, fclose, fread
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#include <stdlib.h>  // malloc, sleep
#include <unistd.h>  // usleep

#define da_malloc(s)    malloc(s)
#define da_free(s)      free(s)
#define da_usleep(s)    usleep(s)

#define DA_ARG_SEEN_BLOB    0x01
#define DA_ARG_SEEN_ASSET   0x02
#define DA_ARG_SEEN_POLICY  0x04
#define DA_ARG_SEEN_LABEL   0x08
#define DA_ARG_SEEN_CRC     0x10
#define DA_ARG_SEEN_OK      (DA_ARG_SEEN_BLOB|DA_ARG_SEEN_ASSET|DA_ARG_SEEN_POLICY|DA_ARG_SEEN_LABEL)

#define DA_PREFIX       "DA_OTPPRG: "


/*----------------------------------------------------------------------------
 * Local variables
 */

static char * gl_Program = NULL;

static uint8_t * gl_AD_Label = NULL;
static size_t gl_AD_LabelSize = 0;
static uint8_t gl_AssetNumber = 0;
#ifdef EIP130_FW_ASSETPOLICY_V2
static ValPolicyMask_t gl_AssetPolicy = 0;
#else
static uint8_t gl_AssetPolicy = 0;
#endif
static uint8_t gl_AssetData[2048+16];
static size_t gl_AssetDataSize = 0;

static bool gl_Verbose = false;
static int gl_ArgumentSeen = 0;


/*----------------------------------------------------------------------------
 * DisplayUsage
 *
 * Handle usage display & usage error situations.
 * This function never returns, instead calls exit with given code.
 */
static int
DisplayUsage(
        const char * error)
{
    if (error)
    {
        // On usage errors, give error message and point user to -help.
        LOG_CRIT(DA_PREFIX "FAILED: %s.\n"
                 DA_PREFIX "Try '%s -help' for usage.\n",
                 error, gl_Program);
    }
    else
    {
        // Otherwise, build usage instructions.
        LOG_CRIT(DA_PREFIX "Example application for OTP programming.\n\n"
                 "Usage: %s [-help] [-verbose] blob [file] asset <number> policy <number> label <data> [crc]\n\n"
                 "Arguments:\n"
                 " -help            Display this information.\n"
                 " -verbose         Enable logging.\n"
                 " -exit            Exit without performing OTP programming.\n"
                 " blob [file]      Specifies the file name of the key blob data file,\n"
                 "                  file name is optional for hosts without file-systems.\n"
#ifdef EIP130_FW_ASSETPOLICY_V2
                 " asset <number>   Specifies the asset number of the asset [1-%d].\n"
                 " policy <value>   Specifies the Asset policy.\n"
                 "                  When <value> starts with:\n"
                 "                    '0x'     a hexadecimal value is assumed\n"
                 "                    '0'      an octadecimal value is assumed\n"
                 "                    '[1..9]' a decimal value is assumed\n"
#else
                 " asset <number>   Specifies the asset number of the asset [0-%d].\n"
                 " policy <number>  Specifies the policy number of the asset:\n"
                 "                  0  - Monotonic Counter\n"
                 "                  1  - TRUSTED-KDK-DERIVE (HUK)\n"
                 "                  2  - AES key unwrap key (SB)\n"
                 "                  3  - TRUSTED-KEY-DERIVE (SB)\n"
                 "                  4  - Public Data (SB) for Public key or SHA256 Public key digest\n"
                 "                  6  - Auth Key for eMMC (AK)\n"
                 "                  7  - Private Data for Milenage K and OPc\n"
                 "                  8  - AES Unwrap key, (Secure Debug)\n"
                 "                  24 - AES key unwrap key (FWSB)\n"
                 "                  25 - TRUSTED-KEY-DERIVE (FWSB)\n"
                 "                  26 - SHA256 Public key digest (FWSB)\n"
                 "                  28 - JTAG Secure Debug Public Key (Secure Debug)\n"
#endif
                 " label <data>     Specifies the Associated Data that must be used.\n"
                 " crc              Specifies that the asset must be protected with an CRC.\n",
                 gl_Program, VAL_ASSET_NUMBER_MAX);
    }

    return -1;
}


/*----------------------------------------------------------------------------
 * HandleArguments
 *
 * Handles command line arguments.
 */
static int
HandleArguments(
        int argc,
        char * argv[])
{
    int i;

    gl_Program = argv[0];

    i = 1;
    while (i < argc)
    {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-help"))
        {
            (void)DisplayUsage(NULL);
            return 1;
        }

        if (!strcmp(argv[i], "-verbose"))
        {
            gl_Verbose = true;
            i += 1;
            continue;
        }
        else if (!strcmp(argv[i], "crc"))
        {
            // CRC Asset
            gl_ArgumentSeen |= DA_ARG_SEEN_CRC;
            i += 1;
            continue;
        }
        else if (!strcmp(argv[i], "-exit"))
        {
            return 1;
        }

        if ((i + 1) >= argc)
        {
            return DisplayUsage("Wrong number of arguments");
        }

        if (!strcmp(argv[i], "blob"))
        {
#ifdef DA_OTPPROGRAM_KEYBLOB_FS_REMOVE
            extern uint8_t _binary_da_otpprogram_keyblob_bin_start;
            extern uint8_t _binary_da_otpprogram_keyblob_bin_end;
            extern unsigned int _binary_da_otpprogram_keyblob_bin_size;

            uint8_t * start = (uint8_t*)&_binary_da_otpprogram_keyblob_bin_start;
            uint8_t * end = (uint8_t*)&_binary_da_otpprogram_keyblob_bin_end;
            uint32_t * size = (uint32_t*)&_binary_da_otpprogram_keyblob_bin_size;
            uint8_t * p = gl_AssetData;

            gl_AssetDataSize = (size_t)size;

            // Read key blob
            while(start < end)
            {
                *p = *start;
                start++;
                p++;
            }
#else
            FILE *fp;

            // Read the AES-SIV key from the specified file
            fp = fopen(argv[i + 1], "rb");
            if (fp == NULL)
            {
                return DisplayUsage("Cannot open key blob file");
            }

            gl_AssetDataSize = fread(gl_AssetData, 1, sizeof(gl_AssetData), fp);
            fclose(fp);
#endif
            if (gl_AssetDataSize > 0)
            {
                gl_ArgumentSeen |= DA_ARG_SEEN_BLOB;
            }
            else
            {
                return DisplayUsage("Cannot read key blob file");
            }
        }
        else if (!strcmp(argv[i], "asset"))
        {
            // Read and validate asset number
            gl_AssetNumber = (uint8_t)atoi(argv[i + 1]);
#ifdef EIP130_FW_ASSETPOLICY_V2
            if ((gl_AssetNumber < 1) || (gl_AssetNumber > VAL_ASSET_NUMBER_MAX))
#else
            if (gl_AssetNumber > VAL_ASSET_NUMBER_MAX)
#endif
            {
                return DisplayUsage("Wrong asset number");
            }
            gl_ArgumentSeen |= DA_ARG_SEEN_ASSET;
        }
        else if (!strcmp(argv[i], "policy"))
        {
#ifdef EIP130_FW_ASSETPOLICY_V2
            char *EndPtr_p;
            ValPolicyMask_t AssetPolicy = (ValPolicyMask_t)strtoull(argv[i + 1], &EndPtr_p, 0);

            if ((EndPtr_p == argv[i + 1]) || (*EndPtr_p != '\0'))
            {
                return DisplayUsage("Invalid policy value");
            }
            if (AssetPolicy == 0)
            {
                return DisplayUsage("Invalid policy, value can not be 0");
            }
            gl_AssetPolicy = AssetPolicy;
#else
            // Read and validate policy number
            gl_AssetPolicy = (uint8_t)atoi(argv[i + 1]);
            switch (gl_AssetPolicy)
            {
            default:
                return DisplayUsage("Wrong policy number");

            case 0:     // Monotonic Counter
            case 1:     // TRUSTED-KDK-DERIVE (HUK)
            case 2:     // SBCR: AES key unwrap key (SB)
            case 3:     // PUK: TRUSTED-KEY-DERIVE (SB)
            case 4:     // Public Data, general use
            case 6:     // Auth Key for eMMC (AK)
            case 7:     // Private Data for Milenage K and OPc
            case 8:     // AES Unwrap key, Secure Debug usage restriction
            case 24:    // Private Data, AES key unwrap key (FWSB)
            case 25:    // Private Data, TRUSTED-KEY-DERIVE (FWSB)
            case 26:    // Private Data, SHA256 Public key digest (FWSB)
            case 28:    // Private Data, JTAG Secure Debug Public Key
                break;
            }
#endif
            gl_ArgumentSeen |= DA_ARG_SEEN_POLICY;
        }
        else if (!strcmp(argv[i], "label"))
        {
            // Assign Associated Data (label)
            gl_AD_Label = (uint8_t *)argv[i + 1];
            gl_AD_LabelSize = strlen(argv[i + 1]);

            gl_ArgumentSeen |= DA_ARG_SEEN_LABEL;
        }
        else
        {
            return DisplayUsage("Invalid option specified.");
        }
        i += 2;
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * da_otpprogram_main
 */
int
da_otpprogram_main(
        int argc,
        char * argv[])
{
    int res, MainReturnCode = -1;
    ValStatus_t funcres;

    // Parse command line arguments.
    res = HandleArguments(argc, argv);
    if (res < 0)
    {
        return -1; // some error
    }
    else if (res > 0)
    {
        return 0; // exit normally
    }

    if ((gl_ArgumentSeen & DA_ARG_SEEN_OK) != DA_ARG_SEEN_OK)
    {
        return DisplayUsage("Not all required arguments specified");
    }

    if (Driver130_Init() < 0)
    {
        LOG_CRIT(DA_PREFIX "FAILED: Driver130 initialization\n");
    }
    else
    {
        if (val_IsAccessSecure())
        {
            if (gl_Verbose)
            {
                LOG_CRIT(DA_PREFIX "OTP Write information\n");
                LOG_CRIT("Asset number : %d\n", gl_AssetNumber);
#ifdef EIP130_FW_ASSETPOLICY_V2
                LOG_CRIT("Asset policy : 0x%llX\n", (long long unsigned int)gl_AssetPolicy);
#else
                LOG_CRIT("Asset policy : %d\n", gl_AssetPolicy);
#endif
                LOG_CRIT("AD Label     : %s\n", gl_AD_Label);
                LOG_CRIT("CRC          : %s\n", (gl_ArgumentSeen & DA_ARG_SEEN_CRC) ? "Yes" : "No");
                Log_HexDump("OTP BlobData", 0, gl_AssetData, (unsigned int)gl_AssetDataSize);
            }

            funcres = val_OTPDataWrite(gl_AssetNumber, gl_AssetPolicy,
                                       ((gl_ArgumentSeen & DA_ARG_SEEN_CRC) != 0),
                                       gl_AD_Label, gl_AD_LabelSize,
                                       gl_AssetData, gl_AssetDataSize);
            if (funcres != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: OTP data write (%d).\n", (int)funcres);
            }
            else
            {
                LOG_CRIT(DA_PREFIX "PASSED: OTP data written.\n");
                LOG_CRIT("NOTE: Hardware must be 'reset' to enable Asset for use.\n");
                MainReturnCode = 0;
            }
        }
        else
        {
            LOG_CRIT(DA_PREFIX "FAILED: No secure hardware connection.\n");
        }

        Driver130_Exit();
    }

    return MainReturnCode;
}


#ifndef DA_OTPPROGRAM_MAIN_REMOVE
/*----------------------------------------------------------------------------
 * main
 *
 * Program entry.
 */
int
main(
        int argc,
        char * argv[])
{
    return da_otpprogram_main(argc, argv);
}
#endif


/* end of file da_otpprogram.c */
