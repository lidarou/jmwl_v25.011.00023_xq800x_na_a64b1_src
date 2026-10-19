/* adapter_vex_device.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the device related functionality.
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
#include "log.h"

#include "adapter_vex_internal.h"   // API implementation
#include "adapter_lock.h"           // Adapter_Lock_t, Adapter_Lock*()
#ifdef VEX_ENABLE_FIRMWARE_LOAD
#include "adapter_firmware.h"       // Adapter_Firmware_*
#endif
#include "adapter_sleep.h"          // Adapter_SleepMS()
#include "device_mgmt.h"            // Device_Find()
#include "eip130.h"                 // EIP130_Mailbox*()
#include "eip130_token_system.h"    // Eip130Token_Command_System*()

#define VEX_DEVICE_MAX_MAILBOXES    8
#define VEX_DEVICE_MAX_CLEANUPS     8

static Device_Handle_t gl_VexDevice = NULL;
static int gl_VexDeviceRefCnt = 0;              // vex_DeviceInit call reference counter
static int gl_VexDevicePWRState = VEX_DEVICE_POWER_UNKNOWN;
static uint8_t gl_VexDeviceMyHostID = 0;        // My Host ID
static uint8_t gl_VexDeviceMyProt = 0;          // My Protection bit setting
static uint8_t gl_VexDeviceMasterID = 0;        // Host ID of master
static uint8_t gl_VexDeviceProtAvailable = 0;   // Protection functionality available
static uint8_t gl_VexDeviceNrOfMailboxes = 0;   // Number of mailboxes
static uint16_t gl_VexDeviceMailboxSize = 256;  // Mailbox size (256 bytes)
static uint16_t gl_VexDeviceTokenID = 0;        // TokenID reference counter

// Mailbox linking state
// 0x0000   = Not Linked
// 0xFFFF   = Linked for the sequence ->
//                link - one operation - unlink
// identity = Linked by a calling process for the sequence ->
//                link - multiple operations - unlink
static uint32_t gl_VexDeviceMailboxLinked[VEX_DEVICE_MAX_MAILBOXES];


static int
vexLocal_DeviceInit(
        int FirmwareCheckInfo)
{
    int i;

#ifdef VEX_ENABLE_FIRMWARE_LOAD
    // FirmwareCheckInfo == 1 for HW3, HW2 does not specify an action when fwcheck info = 1
    if ((FirmwareCheckInfo == 0) || (FirmwareCheckInfo == 1))
    {
        Adapter_Firmware_t FirmwareHandle;
        const uint32_t * Firmware_p;
        unsigned int Firmware_Word32Count;

        // Get the firmware image
        FirmwareHandle = Adapter_Firmware_Acquire(VEX_FIRMWARE_FILE,
                                                  &Firmware_p,
                                                  &Firmware_Word32Count);
        if (FirmwareHandle == NULL)
        {
            return -3;
        }

        // Load the firmware image
        i = EIP130_FirmwareLoad(gl_VexDevice, VEX_MAILBOX_NR,
                                Firmware_p, Firmware_Word32Count);
        Adapter_Firmware_Release(FirmwareHandle);
        if (i < 0)
        {
            LOG_CRIT(VEX_LOG_PREFIX "FAILED Firmware load (%d)\n", i);
            gl_VexDevicePWRState = VEX_DEVICE_POWER_UNKNOWN;
            return -4;
        }
    }
    else if(FirmwareCheckInfo == 3)
    {
        LOG_INFO(VEX_LOG_PREFIX "ROM-Only detected -skipping FWRAM load\n");
    }
    else
    {
        LOG_CRIT(VEX_LOG_PREFIX "FAILED Firmware Check (%d)\n", FirmwareCheckInfo);
    }
#else
    IDENTIFIER_NOT_USED(FirmwareCheckInfo);
#endif

    gl_VexDevicePWRState = VEX_DEVICE_POWER_ACTIVE;

    // Get exclusive access to the requested mailbox
    if (EIP130_MailboxLink(gl_VexDevice, VEX_MAILBOX_NR) != 0)
    {
        return -6;
    }

    // Check if OUT mailbox is unexpectedly FULL?
    // -> Remove result tokens
    i = 0;
    while (EIP130_MailboxCanReadToken(gl_VexDevice, VEX_MAILBOX_NR))
    {
        Eip130Token_Result_t ResultToken;

        if (EIP130_MailboxReadToken(gl_VexDevice, VEX_MAILBOX_NR, &ResultToken) != 0)
        {
            return -7;
        }

        i++;
        if (i > 3)
        {
            return -7;
        }
    }

    // Check if IN mailbox is unexpectedly FULL?
    if (!EIP130_MailboxCanWriteToken(gl_VexDevice, VEX_MAILBOX_NR, NULL))
    {
        return -8;
    }

#ifndef VEX_MAILBOX_LINK_ONLY_ONCE
    // Release exclusive access to the requested mailbox
    if (EIP130_MailboxUnlink(gl_VexDevice, VEX_MAILBOX_NR) != 0)
    {
        return -9;
    }
#endif

    return 0;
}

/*----------------------------------------------------------------------------
 * vex_DeviceInit
 */
int
vex_DeviceInit(void)
{
    int rc = 0;

    if(vex_LockAcquire() < 0)
    {
        return -100;
    }

    if (gl_VexDevice == NULL)
    {
        int i;

        // Get the EIP-13x device reference
        gl_VexDevice = Device_Find(VEX_DEVICE_NAME);
        if (gl_VexDevice == NULL)
        {
            rc = -1;
            goto func_return;
        }
        gl_VexDeviceRefCnt++;
        gl_VexDevicePWRState = VEX_DEVICE_POWER_UNKNOWN;

        // Reset mailbox linked indications
        for (i = 0; i < VEX_DEVICE_MAX_MAILBOXES; i++)
        {
            gl_VexDeviceMailboxLinked[i] = 0;
        }

        // Check device and firmware state
        rc = EIP130_FirmwareCheck(gl_VexDevice);
        if (rc == -1)
        {
            // Not supported hardware
            rc = -2;
            goto func_return;
        }

        // Get mailbox related information and check given mailbox
        EIP130_MailboxGetOptions(gl_VexDevice,
                                 &gl_VexDeviceMyHostID,
                                 &gl_VexDeviceMasterID,
                                 &gl_VexDeviceMyProt,
                                 &gl_VexDeviceProtAvailable,
                                 &gl_VexDeviceNrOfMailboxes,
                                 &gl_VexDeviceMailboxSize);
        if (VEX_MAILBOX_NR < gl_VexDeviceNrOfMailboxes)
        {
            rc = vexLocal_DeviceInit(rc);
        }
        else
        {
            rc = -2;
        }
    }
    else
    {
        gl_VexDeviceRefCnt++;
    }

func_return:
    vex_LockRelease();
    return rc;
}


/*----------------------------------------------------------------------------
 * vex_DeviceExit
 */
void
vex_DeviceExit(void)
{
    if (vex_LockAcquire() == 0)
    {
        if (gl_VexDevice != NULL)
        {
            gl_VexDeviceRefCnt--;
            if (gl_VexDeviceRefCnt <= 0)
            {
                int i;

                for (i = 0; i < VEX_DEVICE_MAX_MAILBOXES; i++)
                {
                    if (gl_VexDeviceMailboxLinked[i])
                    {
                        (void)EIP130_MailboxUnlink(gl_VexDevice, (uint8_t)(i + 1));
                        gl_VexDeviceMailboxLinked[i] = 0;
                    }
                }

                gl_VexDevice = NULL;
                gl_VexDeviceRefCnt = 0;
                gl_VexDeviceMyProt = 0; // Protection bit setting reset
            }
        }

        vex_LockRelease();
    }
    else if (gl_VexDevice != NULL)
    {
        LOG_CRIT(VEX_LOG_PREFIX "FAILED to un-initialize device\n");
    }
}


/*----------------------------------------------------------------------------
 * vex_DeviceGetHandle
 */
Device_Handle_t
vex_DeviceGetHandle(void)
{
    if ((gl_VexDevice != NULL) &&
        (gl_VexDevicePWRState == VEX_DEVICE_POWER_UNKNOWN))
    {
        // Check device and firmware state
        int rc = EIP130_FirmwareCheck(gl_VexDevice);
        switch (rc)
        {
        case 3:
            // Firmware ready for use (firmware is accepted)
            break;
        case 2:
            // Firmware checks busy - assume all will go OK
            break;

        case 1:
        case 0:
            // Firmware needs to loaded
            if (vexLocal_DeviceInit(rc) != 0)
            {
                // - Device initialization failed
                return NULL;
            }
            break;

        default:
            // Possible return situations:
            // - No device connection
            // - Device initialization failed
            // - Device initialization busy (other host)
            // - Host not allowed to do the image load
            return NULL;
        }
    }
    return gl_VexDevice;
}


/*----------------------------------------------------------------------------
 * vex_DeviceIsSecureConnected
 */
bool
vex_DeviceIsSecureConnected(void)
{
    // Device connection available and protection bit set
    if (gl_VexDeviceMyProt)
    {
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------
 * vex_DeviceStateIssue
 */
void
vex_DeviceStateIssue(void)
{
    gl_VexDevicePWRState = VEX_DEVICE_POWER_UNKNOWN;
}

/*----------------------------------------------------------------------------
 * vex_DeviceLinkMailbox
 */
int
vex_DeviceMailboxes(void)
{
    return gl_VexDeviceNrOfMailboxes;
}

/*----------------------------------------------------------------------------
 * vex_DeviceLinkMailbox
 */
int
vex_DeviceLinkMailbox(
           const uint8_t MailboxNr,
           uint32_t Identity)
{
    // Device connection available
    if (gl_VexDevice == NULL)
    {
        return -1;
    }

    // Valid mailbox
    if ((MailboxNr == 0) || (MailboxNr > gl_VexDeviceNrOfMailboxes))
    {
        return -2;
    }

    if (gl_VexDeviceMailboxLinked[MailboxNr - 1] == 0)
    {
#ifndef VEX_MAILBOX_LINK_ONLY_ONCE
        // Get exclusive access to the requested mailbox
        if (EIP130_MailboxLink(gl_VexDevice, MailboxNr) != 0)
        {
            // Mailbox could be locked by another host
            return -3;
        }
#endif

        gl_VexDeviceMailboxLinked[MailboxNr - 1] = Identity;
    }
    else if (gl_VexDeviceMailboxLinked[MailboxNr - 1] != Identity)
    {
        // Mailbox lock for another process
        return -4;
    }

    return 0;
}

/*----------------------------------------------------------------------------
 * vex_DeviceLinkMailboxOverrule
 */
int
vex_DeviceLinkMailboxOverrule(
           const uint8_t MailboxNr,
           uint32_t Identity)
{
    // Device connection available
    if (gl_VexDevice == NULL)
    {
        return -1;
    }

    // Valid mailbox
    if ((MailboxNr == 0) || (MailboxNr > gl_VexDeviceNrOfMailboxes))
    {
        return -2;
    }

#ifndef VEX_MAILBOX_LINK_ONLY_ONCE
    // Get exclusive access to the requested mailbox
    if (EIP130_MailboxLink(gl_VexDevice, MailboxNr) != 0)
    {
        // Mailbox could be locked by another host
        return -3;
    }
#endif
    gl_VexDeviceMailboxLinked[MailboxNr - 1] = Identity;

    return 0;
}

/*----------------------------------------------------------------------------
 * vex_DeviceUnlinkMailbox
 */
int
vex_DeviceUnlinkMailbox(
           const uint8_t MailboxNr,
           uint32_t Identity)
{
    // Device connection available
    if (gl_VexDevice == NULL)
    {
        return -1;
    }

    // Valid mailbox
    if ((MailboxNr == 0) || (MailboxNr > gl_VexDeviceNrOfMailboxes))
    {
        return -2;
    }

    if (gl_VexDeviceMailboxLinked[MailboxNr - 1] == Identity)
    {
        gl_VexDeviceMailboxLinked[MailboxNr - 1] = 0;

#ifndef VEX_MAILBOX_LINK_ONLY_ONCE
        // Release exclusive access to the requested mailbox
        if (EIP130_MailboxUnlink(gl_VexDevice, MailboxNr) != 0)
        {
            // Probably forced link by another host
            return -3;
        }
#endif
    }
    else if (gl_VexDeviceMailboxLinked[MailboxNr - 1] != 0)
    {
        // Not allowed to unlink the mailbox that is locked by another process
        return -4;
    }

    return 0;
}

/*----------------------------------------------------------------------------
 * vex_DeviceGetTokenID
 */
uint16_t
vex_DeviceGetTokenID(void)
{
    // Generate a TokenID
    gl_VexDeviceTokenID++;
    if (gl_VexDeviceTokenID == 0)
    {
        gl_VexDeviceTokenID++;
    }
    return gl_VexDeviceTokenID;
}

/*----------------------------------------------------------------------------
 * vex_DeviceSleep
 */
#ifdef VEX_ENABLE_FIRMWARE_SLEEP
int
vex_DeviceSleep(void)
{
    int rc;

    // Device connection available
    if (gl_VexDevice == NULL)
    {
        return VEX_NOT_CONNECTED;
    }

    if (gl_VexDevicePWRState == VEX_DEVICE_POWER_ACTIVE)
    {
        Eip130Token_Command_t CommandResult;
        int nRetries;
        uint32_t Identity;
        uint16_t TokenId;
        uint8_t MailboxNumber;

        // Check if firmware needs to be loaded
        rc = EIP130_FirmwareCheck(gl_VexDevice);
        switch (rc)
        {
        case 3:
            // Firmware ready for use (firmware is accepted)
            break;
        case -1:
            vex_DeviceStateIssue();
            return VEX_NOT_CONNECTED;
        case -2:
            return VEX_OPERATION_NOT_ALLOWED;
        default:
            // The firmware is not in accepted state
            vex_DeviceStateIssue();
            return VEX_POWER_STATE_ERROR;
        }

        // Get and set identity of calling process
        Identity = vex_IdentityGet();
        if (Identity == 0)
        {
            return VEX_NO_IDENTITY;
        }

        // Get mailbox number to use
        MailboxNumber = vex_MailboxGet(Identity);
        if (MailboxNumber == 0)
        {
            return VEX_NO_MAILBOX;
        }

        // Make sure the mailbox is linked for use
        for (nRetries = 0; ; nRetries++)
        {
            rc = EIP130_MailboxLink(gl_VexDevice, MailboxNumber);
            if (rc == 0)
            {
                break;
            }
            if (nRetries)
            {
                return VEX_MAILBOX_IN_USE;
            }

            // Reset link assume that mailbox is in use by another Host.
            rc = EIP130_MailboxLinkReset(gl_VexDevice, MailboxNumber);
            if (rc < 0)
            {
                return VEX_MAILBOX_IN_USE; // Link reset error
            }
        }
        gl_VexDeviceMailboxLinked[MailboxNumber - 1] = Identity;

        // Check if mailbox is ready for the token
        if (!EIP130_MailboxCanWriteToken(gl_VexDevice, MailboxNumber, NULL))
        {
            rc = VEX_MAILBOX_IN_USE;    // General error
            goto func_error;            // Unlink mailbox and return error
        }

        // Write Sleep token
        Eip130Token_Command_SystemSleep(&CommandResult);
        Eip130Token_Command_Identity(&CommandResult, Identity);
        TokenId = vex_DeviceGetTokenID();
        CommandResult.W[0] |= TokenId;
        rc = EIP130_MailboxWriteAndSubmitToken(gl_VexDevice, MailboxNumber,
                                               &CommandResult, false);
        switch (rc)
        {
        case 0:
            break;
        case -3:
            rc = VEX_MAILBOX_IN_USE;
            goto func_error;            // Unlink mailbox and return error
        case -4:
            vex_DeviceStateIssue();     // Report device state issue detected
            rc = VEX_POWER_STATE_ERROR;
            goto func_error;            // Unlink mailbox and return error
        default:
            rc = VEX_INTERNAL_ERROR;
            goto func_error;            // Unlink mailbox and return error
        }

        // Wait and read output token
        do
        {
            for (nRetries = 0x00FFFFFF; nRetries; nRetries--)
            {
                if (EIP130_MailboxCanReadToken(gl_VexDevice, MailboxNumber))
                {
                    break;
                }
            }

            rc = EIP130_MailboxReadToken(gl_VexDevice, MailboxNumber,
                                         (Eip130Token_Result_t *)&CommandResult);
            if (rc < 0)
            {
                rc = VEX_INTERNAL_ERROR;
                goto func_error;        // Unlink mailbox and return error
            }
        } while ((CommandResult.W[0] & MASK_16_BITS) != TokenId);

        rc = Eip130Token_Result_Code((Eip130Token_Result_t *)&CommandResult);
        if (rc < 0)
        {
            rc -= 256;
        }
        else
        {
            gl_VexDevicePWRState = VEX_DEVICE_POWER_SLEEP;
        }

func_error:
        // Unlink the mailbox
        (void)vex_DeviceUnlinkMailbox(MailboxNumber, Identity);
    }
    else
    {
        rc = VEX_POWER_STATE_ERROR;     // Invalid state
    }

    return rc;
}
#endif

/*----------------------------------------------------------------------------
 * vex_DeviceResumeFromSleep
 */
#ifdef VEX_ENABLE_FIRMWARE_SLEEP
int
vex_DeviceResumeFromSleep(void)
{
    Eip130Token_Command_t CommandResult;
    int rc;
    int nRetries;
    uint32_t Identity;
    uint16_t TokenId;
    uint8_t MailboxNumber;
    uint8_t fFWRam;

    // Device connection available
    if (gl_VexDevice == NULL)
    {
        return -1;
    }

    if (gl_VexDeviceMyHostID != gl_VexDeviceMasterID)
    {
        // This Host is NOT allowed to do the Sleep operation
        return -2;
    }

    // Check if firmware needs to be loaded
    rc = EIP130_FirmwareCheck(gl_VexDevice);
    switch (rc)
    {
    case 3:
    case 2:
        // Firmware ready for use (firmware is accepted) or
        // the firmware is loaded and the firmware checks are busy
        return 0;
    case 1:
    case 0:
        // Assume Firmware is loaded already (in sleep state)
        break;

    default:
        return rc;                      // General error
    }

    // Get and set identity of calling process
    Identity = vex_IdentityGet();
    if (Identity == 0)
    {
        return VEX_NO_IDENTITY;
    }

    // Get mailbox number to use
    MailboxNumber = vex_MailboxGet(Identity);
    if (MailboxNumber == 0)
    {
        return VEX_NO_MAILBOX;
    }

    // Link mailbox
    // Note: The HW is expected to come out of sleep state, so mailbox linking
    //       should be possible!
    for (nRetries = 0; ; nRetries++)
    {
        rc = EIP130_MailboxLink(gl_VexDevice, MailboxNumber);
        if (rc == 0)
        {
            break;
        }
        if (nRetries != 0)
        {
            return rc;                  // General error
        }

        // Reset link assume that mailbox is in use by another Host
        rc = EIP130_MailboxLinkReset(gl_VexDevice, MailboxNumber);
        if (rc < 0)
        {
            return rc;                  // Link reset error
        }
    }

    // Reset mailbox linked indications
    for (nRetries = 0; nRetries < VEX_DEVICE_MAX_MAILBOXES; nRetries++)
    {
        gl_VexDeviceMailboxLinked[nRetries] = 0;
    }

    // Check if mailbox is ready for the token
    // Note: The HW is expected to come out of HW reset state, so direct
    //       mailbox use should be possible!
    if (!EIP130_MailboxCanWriteToken(gl_VexDevice, MailboxNumber, NULL))
    {
        rc = -1;                        // General error
        goto func_error;                // Unlink mailbox and return error
    }

    // Write Resume From Sleep token
    Eip130Token_Command_SystemResumeFromSleep(&CommandResult);
    Eip130Token_Command_Identity(&CommandResult, Identity);
    TokenId = vex_DeviceGetTokenID();
    Eip130Token_Command_SetTokenID(&CommandResult, TokenId, false);
    rc = EIP130_MailboxWriteAndSubmitToken(gl_VexDevice, MailboxNumber,
                                           &CommandResult, false);
    if (rc < 0)
    {
        goto func_error;                // Unlink mailbox and return error
    }

    // Check firmware configuration
    EIP130_ModuleGetOptions(gl_VexDevice, &fFWRam, NULL, NULL, NULL);
    if (fFWRam != 0)
    {
        uint8_t fFirmwareAccepted;

#ifndef EIP130_HW_V3
        // Indicate that the firmware is written
        EIP130_ModuleFirmwareWritten(gl_VexDevice);
#endif

        // Check Resume from Sleep sequence
        // Note: The firmware check indication sequence can be missed due to
        //       environment behavior, thus only check the firmware accecpted
        //       indication.
        nRetries = 0x7FFFFFFF;
        while (1)
        {
            EIP130_ModuleGetStatus(gl_VexDevice, NULL, NULL, NULL, NULL, NULL,
                                   NULL, NULL, &fFirmwareAccepted);
            if (fFirmwareAccepted != 0)
            {
                /* Firmware accepted and ready for use */
                break;
            }

            if (EIP130_MailboxCanWriteToken(gl_VexDevice, MailboxNumber, NULL))
            {
                if (!EIP130_MailboxCanReadToken(gl_VexDevice, MailboxNumber))
                {
                    /* Assume the Resume from Sleep has failed due to FWRAM or DataRAM
                     * corruption. Try to recover with a System Boot (cold boot) */
                    LOG_CRIT(VEX_LOG_PREFIX "Resume from Sleep has failed, trying System Boot...\n");
#ifdef EIP130_HW_V3
                    rc = vexLocal_DeviceInit(1);
#else
                    rc = vexLocal_DeviceInit(0);
#endif
                    goto func_error;        // Unlink mailbox and return result
                }

                /* Expected that the Firmware was accepted */
                if (fFWRam != 0)
                {
                    /* Retry once */
                    fFWRam = 0;
                }
                else
                {
                    /* Safeguard */
                    break;
                }
            }

            /* Handle retries */
            nRetries--;
            if (nRetries <= 0)
            {
                /* Timeout (Is HW active?) */
                rc = -4;
                goto func_error;        // Unlink mailbox and return error
            }
        }
    }

    /* Wait for the Resume from Sleep result */
    do
    {
        for (nRetries = (10000/VEX_POLLING_DELAY_MS); nRetries ; nRetries--)
        {
            if (EIP130_MailboxCanReadToken(gl_VexDevice, MailboxNumber))
            {
                break;
            }

            /* Wait a bit for the next token available check */
            Adapter_SleepMS(VEX_POLLING_DELAY_MS);
        }

        rc = EIP130_MailboxReadToken(gl_VexDevice, MailboxNumber,
                                     (Eip130Token_Result_t *)&CommandResult);
        if (rc < 0)
        {
            goto func_error;            // Unlink mailbox and return error
        }
    } while ((CommandResult.W[0] & MASK_16_BITS) != TokenId);

    /* Check the result */
    rc = Eip130Token_Result_Code((Eip130Token_Result_t *)&CommandResult);
    if (rc < 0)
    {
        rc -= 256;
    }
    else
    {
        gl_VexDevicePWRState = VEX_DEVICE_POWER_ACTIVE;
    }

func_error:
    if (rc < 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "Resume from Sleep failed. (%d)\n", rc);
    }

#ifndef VEX_MAILBOX_LINK_ONLY_ONCE
    // Unlink mailbox
    (void)EIP130_MailboxUnlink(gl_VexDevice, MailboxNumber);
#endif

    return rc;
}
#endif

/* end of file adapter_vex_device.c */
