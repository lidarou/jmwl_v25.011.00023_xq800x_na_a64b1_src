/* adapter_vex_system.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the system services.
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
#include "eip130_token_system.h"    // Eip130Token_Command_System*()
#include "eip130_token_service.h"   // Eip130Token_Command_ZeroizeOutputMailbox()
#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
#include "adapter_bufmanager.h"     // BufManager_*()
#endif


/*----------------------------------------------------------------------------
 * vex_System
 */
VexStatus_t
vex_System(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;
    Eip130Token_Command_t CommandToken;
    Eip130Token_Result_t ResultToken;
#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
    uint64_t DataBlobAddress = 0;
    bool fCopy = false;
#endif
    uint16_t TokenID = vex_DeviceGetTokenID();
    int NrCommands = 1;

    // Format command token
    ZEROINIT(CommandToken);
    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_SYSTEMINFO:
        Eip130Token_Command_SystemInfo(&CommandToken);
        break;

    case VEXTOKEN_SUBCODE_SELFTEST:
        Eip130Token_Command_SystemSelfTest(&CommandToken);
        break;

#ifdef VEX_ENABLE_SYSTEM_ENTERFIPSMODE
    case VEXTOKEN_SUBCODE_ENTERFIPSMODE:
        Eip130Token_Command_EnterFipsMode(&CommandToken);
        break;
#endif

    case VEXTOKEN_SUBCODE_RESET:
        Eip130Token_Command_SystemReset(&CommandToken);
        NrCommands = 2;                 // Perform additional verify command
        break;

    case VEXTOKEN_SUBCODE_DEFINEUSERS:
        Eip130Token_Command_SystemDefineUsers(&CommandToken,
                                              CommandToken_p->Service.SystemDefineUsers.User1,
                                              CommandToken_p->Service.SystemDefineUsers.User2,
                                              CommandToken_p->Service.SystemDefineUsers.User3,
                                              CommandToken_p->Service.SystemDefineUsers.User4);
        break;

#ifdef VEX_ENABLE_FIRMWARE_SLEEP
    case VEXTOKEN_SUBCODE_SLEEP:
    case VEXTOKEN_SUBCODE_RESUMEFROMSLEEP:
        {
            int rc;

            if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_SLEEP)
            {
                rc = vex_DeviceSleep();
            }
            else
            {
                rc = vex_DeviceResumeFromSleep();
            }

            ResultToken_p->Result = rc;  // Save result back to VexToken_Result structure

            switch (rc)
            {
            default:
                if (rc < -256)
                {
                    // Successful, but firmware error
                    ResultToken_p->Result = rc + 256;
                    break;
                }
                return VEX_INTERNAL_ERROR;
            case 0:
                // Successful
                break;
            case -1:
                return VEX_UNSUPPORTED;
            case -2:
                return VEX_OPERATION_NOT_ALLOWED;
            case -3:
                return VEX_OPERATION_FAILED;
            case -4:
                return VEX_POWER_STATE_ERROR;
            }
            return VEX_SUCCESS;
        }
#endif

#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
    case VEXTOKEN_SUBCODE_HIBERNATION:
        DataBlobAddress = BufManager_Map(CommandToken_p->fFromUserSpace,
                                         BUFMANAGER_BUFFERTYPE_OUT,
                                         CommandToken_p->Service.SystemHibernation.DataBlob_p,
                                         CommandToken_p->Service.SystemHibernation.DataBlobSize,
                                         (void *)&TokenID);
        if (DataBlobAddress == 0)
        {
            return VEX_NO_MEMORY;
        }

        Eip130Token_Command_SystemHibernation(&CommandToken,
                                              DataBlobAddress,
                                              (uint32_t)BufManager_GetSize(DataBlobAddress));
        Eip130Token_Command_SystemHibernationInfomation(&CommandToken,
                                                        CommandToken_p->Service.SystemHibernation.StateAssetId,
                                                        CommandToken_p->Service.SystemHibernation.KeyAssetId,
                                                        CommandToken_p->Service.SystemHibernation.AssociatedData,
                                                        CommandToken_p->Service.SystemHibernation.AssociatedDataSize);
        break;

    case VEXTOKEN_SUBCODE_RESUMEFROMHIBERNATION:
        // Convert input buffer and get address for token
        DataBlobAddress = BufManager_Map(CommandToken_p->fFromUserSpace,
                                         BUFMANAGER_BUFFERTYPE_IN,
                                         CommandToken_p->Service.SystemHibernation.DataBlob_p,
                                         CommandToken_p->Service.SystemHibernation.DataBlobSize,
                                         NULL);
        if (DataBlobAddress == 0)
        {
            return VEX_NO_MEMORY;
        }

        Eip130Token_Command_SystemResumeFromHibernation(&CommandToken,
                                                        DataBlobAddress,
                                                        CommandToken_p->Service.SystemHibernation.DataBlobSize);
        Eip130Token_Command_SystemHibernationInfomation(&CommandToken,
                                                        CommandToken_p->Service.SystemHibernation.StateAssetId,
                                                        CommandToken_p->Service.SystemHibernation.KeyAssetId,
                                                        CommandToken_p->Service.SystemHibernation.AssociatedData,
                                                        CommandToken_p->Service.SystemHibernation.AssociatedDataSize);
        break;
#endif

#ifdef EIP130_FW_ASSET_LIFETIME_MANAGEMENT
    case VEXTOKEN_SUBCODE_SETTIME:
        Eip130Token_Command_SystemSetTime(&CommandToken,
                                          CommandToken_p->Service.SystemSetTime.Seconds);
        break;
#endif

    default:
        return VEX_UNSUPPORTED;
    }

    // Initialize result token
    ZEROINIT(ResultToken);

    for (; NrCommands > 0; NrCommands--)
    {
        Vex_Command_SetTokenID(&CommandToken, TokenID);

        // Exchange token with the EIP-13x HW
        funcres = vex_PhysicalTokenExchange(&CommandToken, &ResultToken);
        if (funcres == VEX_SUCCESS)
        {
            ResultToken_p->Result = Eip130Token_Result_Code(&ResultToken);
            if (ResultToken_p->Result >= 0)
            {
                // Success
                if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_SYSTEMINFO)
                {
#ifdef VEX_ENABLE_FIRMWARE_LOAD
                    ResultToken_p->Service.SystemInfo.Firmware.RollbackId = (uint8_t)(ResultToken.W[1] >> 24);
#else
                    ResultToken_p->Service.SystemInfo.Firmware.RollbackId = 0;
#endif
                    ResultToken_p->Service.SystemInfo.Firmware.Major = (uint8_t)(ResultToken.W[1] >> 16);
                    ResultToken_p->Service.SystemInfo.Firmware.Minor = (uint8_t)(ResultToken.W[1] >> 8);
                    ResultToken_p->Service.SystemInfo.Firmware.Patch = (uint8_t)(ResultToken.W[1] >> 0);

                    ResultToken_p->Service.SystemInfo.Hardware.Major = (uint8_t)(ResultToken.W[2] >> 16);
                    ResultToken_p->Service.SystemInfo.Hardware.Minor = (uint8_t)(ResultToken.W[2] >> 8);
                    ResultToken_p->Service.SystemInfo.Hardware.Patch = (uint8_t)(ResultToken.W[2] >> 0);
                    ResultToken_p->Service.SystemInfo.Hardware.MemorySizeInBytes = (uint16_t)ResultToken.W[3];

                    ResultToken_p->Service.SystemInfo.OTP.ErrorLocation = (uint16_t)(ResultToken.W[5] & MASK_12_BITS);
                    ResultToken_p->Service.SystemInfo.OTP.ErrorCode = (uint8_t)((ResultToken.W[5] >> 12) & MASK_4_BITS);

                    ResultToken_p->Service.SystemInfo.Self.Identity = ResultToken.W[4];
                    ResultToken_p->Service.SystemInfo.Self.HostID = (uint8_t)((ResultToken.W[3] >> 16) & MASK_3_BITS);
                    ResultToken_p->Service.SystemInfo.Self.NonSecure = (uint8_t)((ResultToken.W[3] >> 19) & MASK_1_BIT);
                    ResultToken_p->Service.SystemInfo.Self.CryptoOfficer = (uint8_t)((ResultToken.W[3] >> 27) & MASK_1_BIT);
                    ResultToken_p->Service.SystemInfo.Self.Mode = (uint8_t)((ResultToken.W[3] >> 28) & MASK_4_BITS);
                    ResultToken_p->Service.SystemInfo.Self.ErrorTest = (uint8_t)((ResultToken.W[5] >> 16) & MASK_8_BITS);
                }
                else if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_RESET)
                {
                    // Wait until reset is finished
                    Eip130Token_Command_ZeroizeOutputMailbox(&CommandToken);
                    TokenID = vex_DeviceGetTokenID();
                }
#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
                else if (CommandToken_p->SubCode == VEXTOKEN_SUBCODE_HIBERNATION)
                {
                    Eip130Token_Result_SystemHibernation_BlobSize(&ResultToken,
                                                                  &ResultToken_p->Service.SystemHibernation.DataBlobSize);
                    fCopy = true;
                }
#endif
            }
        }
    }

#ifdef VEX_ENABLE_FIRMWARE_HIBERNATION
    if (DataBlobAddress != 0)
    {
        if (BufManager_Unmap(DataBlobAddress, true, fCopy) != 0)
        {
            funcres = VEX_INTERNAL_ERROR;
        }
    }
#endif

    return funcres;
}


/* end of file adapter_vex_system.c */
