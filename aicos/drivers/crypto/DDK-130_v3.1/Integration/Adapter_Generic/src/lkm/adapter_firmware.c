/* adapter_firmware.c
 *
 * Kernel-space implementation of Interface for obtaining the firmware image.
 * Read from file system using Linux firmware API.
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

#include "adapter_firmware.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_adapter_firmware.h"

// Driver Framework Basic Defs API
#include "basic_defs.h"

#include <linux/firmware.h>

#include "adapter_alloc.h"

// Logging API
#include "log.h"

#include "device_mgmt.h"  // Device_GetReference()

/*----------------------------------------------------------------------------
 * Adapter_Firmware_NULL
 *
 */
const Adapter_Firmware_t Adapter_Firmware_NULL = NULL;


/*----------------------------------------------------------------------------
 * Adapter_Firmware_Acquire
 */
Adapter_Firmware_t
Adapter_Firmware_Acquire(
    const char * Firmware_Name_p,
    const uint32_t ** Firmware_p,
    unsigned int  * Firmware_Word32Count)
{
    uint32_t * Firmware_Data_p;
    const struct firmware  *firmware;
    int rc;
    unsigned int i;

    LOG_CRIT("%s: %s\n", __func__, Firmware_Name_p);

    // Initialize output parameters.
    *Firmware_p = NULL;
    *Firmware_Word32Count = 0;

    // Request firmware via kernel API.
    rc = request_firmware(&firmware,
                          Firmware_Name_p,
                          Device_GetReference(NULL, NULL));
    if (rc < 0)
    {
        LOG_CRIT("%s: request of %s failed, rc=%d\n",
                 __func__, Firmware_Name_p, rc);
        return NULL;
    }

    if (firmware->data == NULL ||
        firmware->size == 0 ||
        firmware->size >= 256*1024 ||
        firmware->size % sizeof(uint32_t) != 0)
    {
        LOG_CRIT("%s: Invalid image size %d\n", __func__, (int)firmware->size);
        release_firmware(firmware);
        return NULL;
    }

    // Allocate buffer for data
    Firmware_Data_p = Adapter_Alloc(firmware->size);
    if (Firmware_Data_p == NULL)
    {
        LOG_CRIT("%s: Failed to allocate\n", __func__);
        release_firmware(firmware);
        return NULL;
    }

    // Convert bytes from file to array of 32-bit words.
    {
        const uint8_t *p = firmware->data;
        for (i=0; i<firmware->size / sizeof(uint32_t); i++)
        {
            Firmware_Data_p[i] = p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
            p += sizeof(uint32_t);
        }
    }

    // Pass results to caller
    *Firmware_p = Firmware_Data_p;
    *Firmware_Word32Count = firmware->size / sizeof(uint32_t);

    // Release firmware data structure.
    release_firmware(firmware);

    return (Adapter_Firmware_t)Firmware_Data_p;
}

/*----------------------------------------------------------------------------
 * Adapter_Firmware_Release
 */
void
Adapter_Firmware_Release(
    Adapter_Firmware_t FirmwareHandle)
{
    LOG_CRIT("Adapter_Firmware_Release\n");
    Adapter_Free((void*)FirmwareHandle);
}



/* end of file adapter_firmware.c */
