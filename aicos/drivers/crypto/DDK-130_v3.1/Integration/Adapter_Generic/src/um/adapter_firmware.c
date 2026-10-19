/* adapter_firmware.c
 *
 * User-space implementation of Interface for obtaining the firmware image.
 * Read from file system using normal stdio calls.
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

// Logging API
#include "log.h"

#include "adapter_alloc.h"
#include "clib.h"
#include <stdio.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

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
    size_t Firmware_ByteCount;
    uint32_t * Firmware_Data_p;
    FILE * Firmware_File;
    char NameBuf[ADAPTER_FIRMWARE_NAMELEN_MAX];
    unsigned int i, j;
    static const char PathPrefix[] = ADAPTER_FIRMWARE_PATH_PREFIX;

    // Initialize output parameters.
    *Firmware_p = NULL;
    *Firmware_Word32Count = 0;

    // Compose file name
    i = 0;
    while ((PathPrefix[i] != 0) && (i < ADAPTER_FIRMWARE_NAMELEN_MAX))
    {
        NameBuf[i] = PathPrefix[i];
        i++;
    }
    j = 0;
    while ((Firmware_Name_p[j] != 0) && ((i+j) < ADAPTER_FIRMWARE_NAMELEN_MAX))
    {
        NameBuf[i+j] = Firmware_Name_p[j];
        j++;
    }
    if ((i + j) < ADAPTER_FIRMWARE_NAMELEN_MAX)
    {
        NameBuf[i+j] = 0;
    }
    else
    {
        LOG_CRIT("%s: Path + Firmware name too long\n", __func__);
        return NULL;
    }
    LOG_CRIT("%s: %s\n", __func__, NameBuf);

    // Open the file.
    Firmware_File = fopen(NameBuf, "rb");
    if (Firmware_File == NULL)
    {
        LOG_CRIT("%s: Could not open file %s\n", __func__, NameBuf);
        return NULL;
    }

    // Get file size.
    fseek(Firmware_File, 0, SEEK_END);
    Firmware_ByteCount = (size_t)ftell(Firmware_File);
    fseek(Firmware_File, 0, SEEK_SET);
    LOG_CRIT("%s: Firmware file size %d\n", __func__, (int)Firmware_ByteCount);

    if ((Firmware_ByteCount == 0) ||
        (Firmware_ByteCount >= (256 * 1024)) ||
        ((Firmware_ByteCount % sizeof(uint32_t)) != 0))
    {
        LOG_CRIT("%s: Invalid file size %d\n", __func__, (int)Firmware_ByteCount);
        fclose(Firmware_File);
        return NULL;
    }

    // Allocate buffer for data
    Firmware_Data_p = Adapter_Alloc((unsigned int)Firmware_ByteCount);
    if (Firmware_Data_p == NULL)
    {
        LOG_CRIT("%s: Failed to allocate\n", __func__);
        fclose(Firmware_File);
        return NULL;
    }

    // Read from file
    if (fread(Firmware_Data_p,
              sizeof(uint32_t),
              (Firmware_ByteCount / sizeof(uint32_t)),
              Firmware_File) != (unsigned int)(Firmware_ByteCount / sizeof(uint32_t)))
    {
        LOG_CRIT("%s: Failed to read firmware data\n", __func__);
        Adapter_Free(Firmware_Data_p);
        fclose(Firmware_File);
        return NULL;
    }

    // Convert bytes from file to array of 32-bit words (in-place).
    {
        uint8_t *p = (uint8_t *)Firmware_Data_p;
        for (i = 0; i < (unsigned int)(Firmware_ByteCount / sizeof(uint32_t)); i++)
        {
            Firmware_Data_p[i] = (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
            p += sizeof(uint32_t);
        }
    }

    // Close the file
    fclose(Firmware_File);

    // Pass results to caller
    *Firmware_p = Firmware_Data_p;
    *Firmware_Word32Count = (unsigned int)(Firmware_ByteCount / sizeof(uint32_t));
    return (Adapter_Firmware_t) Firmware_Data_p;
}

/*----------------------------------------------------------------------------
 * Adapter_Firmware_Release
 */
void
Adapter_Firmware_Release(
    Adapter_Firmware_t FirmwareHandle)
{
    LOG_CRIT("%s\n", __func__);
    Adapter_Free((void*)FirmwareHandle);
}




/* end of file adapter_firmware.c */
