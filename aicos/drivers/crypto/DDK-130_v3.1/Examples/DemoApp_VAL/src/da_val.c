/* da_val.c
 *
 * Demo Application for the VAL API
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

#ifdef DA_VAL_USERMODE
#include "da_val.h"                     // da_val_main()
#endif


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_da_val.h"                   // configuration

#include "log.h"
#include "api_driver_init.h"
#include "api_val.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#ifdef DA_VAL_USERMODE
#include <stdlib.h>  // malloc, free, atoi
#define da_val_malloc(s) malloc(s)
#define da_val_free(s)   free(s)
#else // DA_VAL_USERMODE
#define EXPORT_SYMTAB

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>       // printk
#include <linux/slab.h>
#include <asm/delay.h>          // udelay

#define da_val_malloc(s) kmalloc(s, GFP_KERNEL)
#define da_val_free(s)   kfree(s)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rambus");
MODULE_DESCRIPTION("Demo program for the VAL API.");
#endif // !DA_VAL_USERMODE

#define DA_PREFIX   "DA_VAL: "


/*----------------------------------------------------------------------------
 * da_val_module_init
 */
static int
da_val_module_init(void)
{
    int rc = -1;
    uint8_t * DataIn_p = NULL;
    const unsigned int DataInByteCount = 40;
    ValOctetsOut_t * DataOut_p = NULL;
    ValSize_t DataOutByteCount = 100;
    ValStatus_t FuncRes;
    int maxOTPAssetNumber = 0;
    uint8_t i;

    LOG_CRIT(DA_PREFIX "Starting Demo Application\n");

    // Allocate DMA buffers
    DataOut_p = da_val_malloc(DataOutByteCount);
    if (DataOut_p == NULL)
    {
        LOG_CRIT(DA_PREFIX "Failed to allocate DMA output buffer\n");
        goto exit;
    }
    memset(DataOut_p, 0, DataOutByteCount);

    DataIn_p = da_val_malloc(DataInByteCount);
    if (DataIn_p == NULL)
    {
        LOG_CRIT(DA_PREFIX "Failed to allocate DMA input buffer\n");
        goto exit;
    }

    // Get version information to check token functionality
    LOG_CRIT(DA_PREFIX "Version test: Start\n");

    FuncRes = val_SystemGetVersion(DataOut_p, &DataOutByteCount);
    if(FuncRes == VAL_SUCCESS)
    {
        LOG_CRIT(DA_PREFIX "Version: %s\n", DataOut_p);
        LOG_CRIT(DA_PREFIX "Version test: PASSED\n");
    }
    else
    {
        LOG_CRIT(DA_PREFIX "Version retrieving failed (%d)\n", FuncRes);
        LOG_CRIT(DA_PREFIX "Version test: FAILED\n");
        goto exit;
    }

    // Get state information
    LOG_CRIT(DA_PREFIX "State test: Start\n");
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
            LOG_CRIT(DA_PREFIX "  State:\n");
#ifdef EIP130_HW_V3
            {
                const char * ptr = "- Reserved???";

                switch (OtpErrorCode)
                {
                case 0:
                    ptr = "- No anomaly";
                    break;
                case 1:
                    ptr = "- Empty";
                    break;
                case 2:
                    ptr = "- Unsupported size";
                    break;
                case 3:
                    ptr = "- FAT Error";
                    break;
                case 8:
                    ptr = "- OTP is zeroized";
                    break;
                default:
                    break;
                }
                LOG_CRIT(DA_PREFIX "    OTP State     : %u %s\n", OtpErrorCode, ptr);
            }
#else
            LOG_CRIT(DA_PREFIX "    OTP State     : %u (%u)\n",
                     OtpErrorCode, OtpErrorLocation);
#endif
            switch (Mode)
            {
            case 0:
                LOG_CRIT(DA_PREFIX "    Mode          : non-FIPS\n");
                break;
            case 4:
                LOG_CRIT(DA_PREFIX "    Mode          : Fatal Error - Self-test (0x%02X)\n",
                         ErrorTest);
                break;
            case 5:
                LOG_CRIT(DA_PREFIX "    Mode          : Fatal Error - DMA\n");
                break;
            case 6:
                LOG_CRIT(DA_PREFIX "    Mode          : Fatal Error - NRBG or DRBG\n");
                break;
            case 7:
                LOG_CRIT(DA_PREFIX "    Mode          : Fatal Error - Key generation\n");
                break;
            case 15:
                LOG_CRIT(DA_PREFIX "    Mode          : FIPS\n");
                break;
            default:
                LOG_CRIT(DA_PREFIX "    Mode          : %u (0x%02X)\n", Mode, ErrorTest);
                break;
            }
            LOG_CRIT(DA_PREFIX "    CryptoOfficer : %sAvailable\n",
                      CryptoOfficer ? "" : "NOT ");
            LOG_CRIT(DA_PREFIX "    HostID        : %u\n", HostID);
            LOG_CRIT(DA_PREFIX "    Secure        : %s\n", NonSecure ? "No" : "Yes ");
            LOG_CRIT(DA_PREFIX "    Identity      : 0x%X\n", Identity);
            LOG_CRIT(DA_PREFIX "State test: PASSED\n");
        }
        else
        {
            LOG_CRIT(DA_PREFIX "State retrieving failed (%d)\n", FuncRes);
            LOG_CRIT(DA_PREFIX "State test: FAILED\n");
            goto exit;
        }
    }

    // Scan OTP objects
    LOG_CRIT(DA_PREFIX "OTP Scan: Start\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
    maxOTPAssetNumber = VAL_ASSET_NUMBER_MAX;
#else
    maxOTPAssetNumber = (VAL_ASSET_NUMBER_MAX + VAL_ASSET_NUMBER_CONSTANTS);
#endif
    for (i = 0; i < maxOTPAssetNumber; i++)
    {
        ValAssetId_t AssetId;

        FuncRes = val_AssetSearch(i, &AssetId, &DataOutByteCount);
        if (FuncRes == VAL_SUCCESS)
        {
            const char * pType = "Private Static Asset";
            uint8_t * pData = NULL;

            FuncRes = val_PublicDataRead(AssetId, DataOut_p, DataOutByteCount);
            if ((FuncRes == VAL_SUCCESS) ||
                (FuncRes == VAL_BUFFER_TOO_SMALL))
            {
                pType = "Public Data object";
                if (FuncRes == VAL_SUCCESS)
                {
                    pData = DataOut_p;
                }
            }
            else
            {
                FuncRes = val_MonotonicCounterRead(AssetId, DataOut_p, DataOutByteCount);
                if ((FuncRes == VAL_SUCCESS) ||
                    (FuncRes == VAL_BUFFER_TOO_SMALL))
                {
                    pType = "Monotonic Counter";
                    if (FuncRes == VAL_SUCCESS)
                    {
                        pData = DataOut_p;
                    }
                }
            }
            LOG_CRIT(DA_PREFIX "  Found Asset number %d (%s), size %d bytes\n",
                     (int)i, pType, (int)DataOutByteCount);
            if (pData != NULL)
            {
                Log_HexDump(DA_PREFIX "    Data", 0, pData, (unsigned int)DataOutByteCount);
            }
        }
        else if (FuncRes == VAL_INTERNAL_ERROR)
        {
            LOG_CRIT(DA_PREFIX "OTP Scan: FAILED\n");
            break;
        }
    }
    if (i == maxOTPAssetNumber)
    {
        LOG_CRIT(DA_PREFIX "OTP Scan: PASSED\n");
    }

    // Perform a NOP operation to check the DMA functionality
    LOG_CRIT(DA_PREFIX "NOP test: Start\n");

    for(i = 0; i < DataInByteCount; i++)
    {
        DataIn_p[i] = i;
    }

    FuncRes = val_NOP(DataIn_p, DataOut_p, DataInByteCount);
    if(FuncRes == VAL_SUCCESS)
    {
        for(i = 0; i < DataInByteCount; i++)
        {
            if (DataOut_p[i] != i)
            {
                LOG_CRIT(DA_PREFIX "Data check failed\n");
                LOG_CRIT(DA_PREFIX "NOP test FAILED\n");
                goto exit;
            }
        }
        LOG_CRIT(DA_PREFIX "NOP test: PASSED\n");
    }
    else
    {
        LOG_CRIT(DA_PREFIX "Operation failed (%d)\n", FuncRes);
        LOG_CRIT(DA_PREFIX "NOP test: FAILED\n");
        goto exit;
    }

    rc = 0; // success

exit:
    if (DataIn_p != NULL)
    {
        da_val_free(DataIn_p);
    }
    if (DataOut_p != NULL)
    {
        da_val_free(DataOut_p);
    }

    return rc;
}


/*----------------------------------------------------------------------------
 * da_val_module_exit
 */
static void
da_val_module_exit(void)
{
    LOG_CRIT(DA_PREFIX "Stopping Demo Application\n");
}


#ifdef DA_VAL_USERMODE
/*----------------------------------------------------------------------------
 * da_val_main
 */
int
da_val_main(
        int argc,
        char **argv)
{
    int i;
    bool fHelp = false;

    if (Driver130_Init() < 0)
    {
        LOG_CRIT(DA_PREFIX "Demo Application: Driver130_Init() failed\n");
        return -1;
    }

    // Process option arguments
    for (i = 1; argc > i; i++)
    {
        ValStatus_t FuncRes;

        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-help"))
        {
            LOG_CRIT(DA_PREFIX "Example application for functionality provided by the VAL API.\n\n");
            LOG_CRIT("Syntax     : %s [<option> ...]\n", argv[0]);
            LOG_CRIT("Description: Execute the Demo Application. Note that you can\n");
            LOG_CRIT("             use the options to perform an operation to set\n");
            LOG_CRIT("             the hardware in a certain state.\n");
            LOG_CRIT("Options    : -help              Display this information\n");
            LOG_CRIT("             -initotp           Initialize the OTP with a default COID and HUK\n");
            LOG_CRIT("             -reset             Reset firmware (hardware)\n");
            LOG_CRIT("             -trng[=<Samples>]  Configure/Start the TRNG (default=3072)\n");

            fHelp = true;
            continue;
        }

        if (strcmp(argv[i], "-reset") == 0)
        {
            // Reset system to have known starting state
            LOG_CRIT(DA_PREFIX "Reset system\n");
            FuncRes = val_SystemReset();
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "Reset system PASSED\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "Reset system FAILED\n");
            }
            fHelp = true;
        }
        else if (strncmp(argv[i], "-trng", 5) == 0)
        {
            uint16_t SampleCycles = 3072;

            if (argv[i][5] == '=')
            {
                SampleCycles = (uint16_t)atoi(&argv[i][6]);
                if (SampleCycles == 0)
                {
                    SampleCycles = 2;
                }
            }

            /* IntegrationItem:
             * The TRNG configuration needs to be adapted to meet the customer
             * specific FROs timing/behavior. */
            LOG_CRIT(DA_PREFIX "Configure TRNG (%u)\n", SampleCycles);
#ifdef EIP130_ENABLE_TRNG_CUTOFF
            FuncRes = val_TrngConfigCutoff(0, SampleCycles, 1, 8, 0, 31, 56, 325, 1);
#else
            FuncRes = val_TrngConfig(0, SampleCycles, 1, 8, 0, 1);
#endif
            if(FuncRes == VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "Configure TRNG PASSED\n");
            }
            else
            {
                LOG_CRIT(DA_PREFIX "Configure TRNG FAILED\n");
                fHelp = true;
            }
        }
        else if (strcmp(argv[i], "-initotp") == 0)
        {
            static const uint8_t AssetData[] =
            {
#ifdef EIP130_FW_ASSETPOLICY_V2
                0x7E, 0x5D, 0x36, 0x4E, 0x25, 0x7B, 0x27, 0xE4, 0x0B, 0x41, 0x05, 0xE9, 0x5E, 0xC0, 0xDE, 0x99,
                0xA0, 0xEA, 0x96, 0x0B, 0x72, 0xEB, 0x95, 0x37, 0x60, 0x34, 0x44, 0xA9, 0xFE, 0x62, 0xDB, 0x49,
                0x89, 0x63, 0x38, 0x2E, 0x24, 0x78, 0xE7, 0x17, 0x81, 0xE8, 0x63, 0x66, 0xF5, 0xE0, 0x67, 0xD8
#else
                0x18, 0x6E, 0x2E, 0xA2, 0x32, 0x09, 0x5B, 0x4A, 0x17, 0xCD, 0xA0, 0xDA, 0x8C, 0xB8, 0x88, 0xED,
                0x2B, 0x33, 0x8A, 0x33, 0xE6, 0x35, 0xC9, 0x8B, 0x20, 0x24, 0x3B, 0x44, 0x5B, 0x39, 0x4B, 0xDD,
                0x98, 0x11, 0x37, 0xF0, 0x96, 0x20, 0xB3, 0x34, 0x6E, 0xC4, 0xDE, 0xCB, 0xC4, 0x34, 0x53, 0x63,
#endif
            };
            static const uint8_t ADLabel[] = "SomeAssociatedDataForProvisioningWithKeyBlob";

            LOG_CRIT(DA_PREFIX "Program HUK and Crypto Officer ID\n");
#ifdef EIP130_FW_ASSETPOLICY_V2
            FuncRes = val_OTPDataWrite(VAL_ASSETNUMBER_HUK,
                                       EIP130_ASSET_POLICY_NONMODIFIABLE |
                                       (VAL_ASSETNUMBER_HUK << 1) |
                                       EIP130_ASSET_POLICY_NODOMAIN |
                                       EIP130_ASSET_POLICY_PRIVATEDATA |
                                       EIP130_ASSET_POLICY_GDHUK,
                                       true, ADLabel, sizeof(ADLabel) - 1,
                                       AssetData, sizeof(AssetData));
#else
            FuncRes = val_OTPDataWrite(0, 1, true, ADLabel, 44, AssetData, sizeof(AssetData));
#endif
            if (FuncRes != VAL_SUCCESS)
            {
                LOG_CRIT(DA_PREFIX "FAILED: OTP data write (%d).\n", (int)FuncRes);
                fHelp = true;
            }
            else
            {
                LOG_CRIT(DA_PREFIX "PASSED: OTP data written.\n");

                LOG_CRIT(DA_PREFIX "Reset system\n");
                FuncRes = val_SystemReset();
                if(FuncRes == VAL_SUCCESS)
                {
                    LOG_CRIT(DA_PREFIX "Reset system PASSED\n");
                }
                else
                {
                    LOG_CRIT(DA_PREFIX "Reset system FAILED\n");
                }
            }
        }
    }

    if (fHelp == false)
    {
        da_val_module_init();
        da_val_module_exit();
    }

    Driver130_Exit();

    return 0;
}


#ifndef DA_VAL_MAIN_REMOVE
/*----------------------------------------------------------------------------
 * main
 */
int
main(
        int argc,
        char ** argv)
{
    return da_val_main(argc, argv);
}
#endif // !DA_VAL_MAIN_REMOVE
#else // DA_VAL_USERMODE
module_init(da_val_module_init);
module_exit(da_val_module_exit);
#endif // !DA_VAL_USERMODE


/* end of file da_val.c */


