/* hwpal_device_umdevxs.c
 *
 * This is the Linux User-mode Driver Framework v4 Device API
 * implementation for UMDevXS. The implementation is device-agnostic and
 * receives configuration details from the cs_hwpal_umdevxs.h file.
 * This implementation supports mapping two UMDevXS devices, to for example
 * allow control of the FPGA via another device.
 *
 * This implementation uses the UMDevXS Proxy and Kernel Driver to map a
 * memory region with the device registers into user space memory.
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

// Driver Framework Device API
#include "device_mgmt.h"            // API to implement
#include "device_rw.h"              // API to implement
#include "device_swap.h"

// Driver Framework Device Internal interface
#include "device_internal.h"        // Device_Internal_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_hwpal_device_umdevxs.h" // get the configuration options

// Driver Framework Basic Defs API
#include "basic_defs.h"             // uint32_t, NULL, inline, etc.

// Standard functions API
#include "clib.h"                   // memcmp

// Driver Framework Device Platform interface
#include "device_platform.h"        // Device_Platform_*

// UMDevXSProxy_Init
#include "umdevxsproxy.h"

// UMDevXSProxy_Device{Find,Map,Unmap}
#include "umdevxsproxy_device.h"    // UMDevXSProxy_Device{Find,Map,Unmap}

#ifndef HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
// UMDevXSProxy_Device_PciCfg API
#include "umdevxsproxy_device_pcicfg.h" // UMDevXSProxy_Device_PciCfg_Read32,
                                        // UMDevXSProxy_Device_PciCfg_Write32
#endif

// Logging API
#include "log.h"

// Linux user-space API
#include <unistd.h>                 // usleep

// C runtime library
#include <stdlib.h>                 // malloc, free


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#ifndef HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
#define IOMEM32_MAGIC_PCICONFIGSPACE  0xFF434647   // 43 46 47 = C F G
#endif

// definition of the Flags
#define HWPAL_FLAGS_READ   BIT_0
#define HWPAL_FLAGS_WRITE  BIT_1
#define HWPAL_FLAGS_SWAP   BIT_2

#define HWPAL_PAGE_SIZE 4096


/*----------------------------------------------------------------------------
 * Local variables
 */

static Device_Admin_Static_t HWPALLib_Devices_Static[] =
{
    HWPAL_DEVICES
};

#define HWPAL_DEVICE_STATIC_COUNT  \
            (sizeof(HWPALLib_Devices_Static) / sizeof(Device_Admin_Static_t))

// All supported devices
static Device_Admin_t * HWPALLib_Devices_p[HWPAL_DEVICE_COUNT];

// Global administration data
static Device_Global_Admin_t HWPALLib_Device_Global;


/*----------------------------------------------------------------------------
 * Forward declarations
 */

static inline void write32_volatile(uint32_t b, volatile void *addr)
{
    *(volatile uint32_t *) addr = b;
}

static inline uint32_t read32_volatile(const volatile void *addr)
{
    return *(const volatile uint32_t *) addr;
}


/*----------------------------------------------------------------------------
 * Device_RemapDeviceAddress
 *
 * This function remaps certain device addresses (relative within the whole
 * device address map) to other addresses. This is needed when the integration
 * has remapped some EIP device registers to other addresses. The EIP Driver
 * Libraries assume the devices always have the same internal layout.
 */

// the cs_hwpal_device_lkm_pci.h file defines a HWPAL_REMAP_ADDRESSES that
// depends on the following HWPAL_REMAP_ONE

#define HWPAL_REMAP_ONE(_old, _new) \
    case _old: \
        DeviceByteOffset = _new; \
        break;

static inline unsigned int
Device_RemapDeviceAddress(
        unsigned int DeviceByteOffset)
{
    switch(DeviceByteOffset)
    {
        // include the remap statements
        HWPAL_REMAP_ADDRESSES

        default:
            break;
    }

    return DeviceByteOffset;
}


/*----------------------------------------------------------------------------
 * HWPALLib_Device2RecPtr
 *
 * This function converts an Device_Handle_t received via one of the
 * Device API functions into a HWPALLib_Devices record pointer, if it is
 * valid.
 *
 * Return Value
 *     NULL    Provided Device Handle was not valid
 *     other   Pointer to a Device_Admin_t record
 */
static inline Device_Admin_t *
HWPALLib_Device2RecordPtr(
        Device_Handle_t Device)
{
    Device_Admin_t * p = (void *)Device;

    if (p == NULL)
    {
        return NULL;
    }

#ifdef HWPAL_DEVICE_MAGIC
    if (p->Magic != HWPAL_DEVICE_MAGIC)
    {
        return NULL;
    }
#endif

    return p;
}


/*----------------------------------------------------------------------------
 * HWPALLib_IsValid
 *
 * This function checks that the parameters are valid to make the access.
 *
 * Device_p is valid
 * ByteOffset is 32-bit aligned
 * ByteOffset is inside device memory range
 */
static inline bool
HWPALLib_IsValid(
        const Device_Admin_t * const Device_p,
        const unsigned int ByteOffset)
{
    if (Device_p == NULL)
    {
        return false;
    }

    if (ByteOffset & 3)
    {
        return false;
    }

    if (Device_p->Platform.FirstOfs + ByteOffset > Device_p->Platform.LastOfs)
    {
        return false;
    }

    return true;
}


/*-----------------------------------------------------------------------------
 * HWPALLib_MapDevice
 */
static int
HWPALLib_MapDevice(
        Device_Admin_t * const DesiredDevice_p)
{
    Device_Admin_t * Device_p;
    int res;
    int DeviceID;
    unsigned int Size;
    unsigned int i;
    void * p;

    if ( (DesiredDevice_p->FirstOfs & (HWPAL_PAGE_SIZE-1)) != 0 ||
         ((DesiredDevice_p->LastOfs+1) & (HWPAL_PAGE_SIZE-1)) != 0)
    {
        // Desired device is not on a page boundary, so find a larger
        // device instead that contains this device as a subrange.
        // and that has page aligned boundaries.
        Device_p = NULL;
        for (i = 0; i < HWPAL_DEVICE_COUNT; i++)
        {
            if (HWPALLib_Devices_p[i] != NULL &&
                HWPALLib_Devices_p[i]->DeviceNr == DesiredDevice_p->DeviceNr &&
                HWPALLib_Devices_p[i]->FirstOfs <= DesiredDevice_p->FirstOfs &&
                HWPALLib_Devices_p[i]->LastOfs >= DesiredDevice_p->LastOfs &&
                (HWPALLib_Devices_p[i]->FirstOfs & (HWPAL_PAGE_SIZE-1)) == 0 &&
                ((HWPALLib_Devices_p[i]->LastOfs + 1) & (HWPAL_PAGE_SIZE - 1)) == 0)
            {
                LOG_INFO("%s: Found substitute for non-aligned device\n",
                         __func__);
                Device_p = HWPALLib_Devices_p[i];
            }
        }

        if (Device_p == NULL)
        {
            LOG_CRIT("%s: Failed to find UMDevXS "
                     "substitute resource for BAR %d offset %x\n",
                     __func__,
                     DesiredDevice_p->DeviceNr,
                     DesiredDevice_p->FirstOfs);
            return -1;
        }
    }
    else
    {
        Device_p = DesiredDevice_p;
    }

    Size = Device_p->LastOfs + 1 - Device_p->FirstOfs;

    // Obtain a handle for the desired device range.
    res = UMDevXSProxy_Device_FindRange(Device_p->DeviceNr,
                                        Device_p->FirstOfs,
                                        Size,
                                        &DeviceID);
    if (res < 0)
    {
        LOG_CRIT("%s: Failed to find UMDevXS resource BAR %d offset %x, error %d\n",
                 __func__, Device_p->DeviceNr, Device_p->FirstOfs, res);
        return -1;
    }

    // add the device memory to our memory map
    p = UMDevXSProxy_Device_Map(DeviceID, Size);
    if (p == NULL)
    {
        LOG_CRIT("%s: Failed to map UMDevXS resource BAR %d offs %x, size %d\n",
                 __func__, Device_p->DeviceNr, Device_p->FirstOfs, Size);
        return -1;
    }

    Device_p->Platform.Mem32_p  = p;
    Device_p->Platform.FirstOfs = 0;
    Device_p->Platform.LastOfs  = Size - 1;

    LOG_INFO("%s: Successfully mapped UMDevXS resource %d offs=0x%08x of size=0x%08x\n",
             __func__, Device_p->DeviceNr, Device_p->FirstOfs, Size);

    /* Now this device has been mapped, initialize devices whose
       range is a subrange of the device just mapped and that can share the
       same register mapping. */
    for (i = 0; i < HWPAL_DEVICE_COUNT; i++)
    {
        if (HWPALLib_Devices_p[i] != NULL &&
            HWPALLib_Devices_p[i]->Platform.Mem32_p == NULL &&
            HWPALLib_Devices_p[i]->DeviceNr == Device_p->DeviceNr &&
            HWPALLib_Devices_p[i]->FirstOfs >= Device_p->FirstOfs &&
            HWPALLib_Devices_p[i]->LastOfs <= Device_p->LastOfs)
        {
            HWPALLib_Devices_p[i]->Platform.Mem32_p = p;
            HWPALLib_Devices_p[i]->Platform.FirstOfs =
                            HWPALLib_Devices_p[i]->FirstOfs - Device_p->FirstOfs;
            HWPALLib_Devices_p[i]->Platform.LastOfs =
                            HWPALLib_Devices_p[i]->Platform.FirstOfs + Size - 1;
        }
    }

    return 0; // success
}


/*-----------------------------------------------------------------------------
 * device_internal interface
 *
 */

/*----------------------------------------------------------------------------
 * Device_Internal_Static_Count_Get
 */
unsigned int
Device_Internal_Static_Count_Get(void)
{
    return HWPAL_DEVICE_STATIC_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Count_Get
 */
unsigned int
Device_Internal_Count_Get(void)
{
    return HWPAL_DEVICE_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Static_Get
 */
const Device_Admin_Static_t *
Device_Internal_Admin_Static_Get(void)
{
    return HWPALLib_Devices_Static;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Get
 *
 * Returns pointer to the memory location where the device list is stored.
 *
 */
Device_Admin_t **
Device_Internal_Admin_Get(void)
{
    return HWPALLib_Devices_p;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Global_Get
 */
Device_Global_Admin_t *
Device_Internal_Admin_Global_Get(void)
{
    return &HWPALLib_Device_Global;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Alloc
 */
void *
Device_Internal_Alloc(
        unsigned int ByteCount)
{
    return malloc(ByteCount);
}


/*----------------------------------------------------------------------------
 * Device_Internal_Free
 */
void
Device_Internal_Free(
        void * Ptr)
{
    free(Ptr);
}


/*-----------------------------------------------------------------------------
 * Device_Internal_Initialize
 */
int
Device_Internal_Initialize(
        void * CustomInitData_p)
{
    int res;

#ifdef HWPAL_INTERRUPTS
    int * const irqp = (int *)CustomInitData_p;
    *irqp = 0; // return the used interrupt through init data
#else
    IDENTIFIER_NOT_USED(CustomInitData_p);
#endif

    res = UMDevXSProxy_Init();
    if (res < 0)
    {
        LOG_CRIT("%s: UMDevXSProxy_Init() error %d, driver not loaded?\n",
                 __func__, res);
        return -1;
    }

    for (res = 0; res < HWPAL_DEVICE_COUNT; res++)
    {
        if (HWPALLib_Devices_p[res])
        {
            HWPALLib_Devices_p[res]->Platform.Mem32_p = NULL;
        }
    }

    return 0; // success
}


/*-----------------------------------------------------------------------------
 * Device_Internal_UnInitialize
 */
void
Device_Internal_UnInitialize(void)
{
    UMDevXSProxy_Shutdown();
}


/*-----------------------------------------------------------------------------
 * Device_Internal_Find
 */
Device_Handle_t
Device_Internal_Find(
        const char * DeviceName_p,
        const unsigned int Index)
{
    IDENTIFIER_NOT_USED(DeviceName_p);

#ifndef HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
    // Is it the config space?
    if (HWPALLib_Devices_p[Index]->FirstOfs == IOMEM32_MAGIC_PCICONFIGSPACE)
    {
        // Set the PCI Config Space Device offsets
        HWPALLib_Devices_p[Index]->Platform.FirstOfs = 0;
        HWPALLib_Devices_p[Index]->Platform.LastOfs  =
            HWPALLib_Devices_p[Index]->LastOfs + 1 - HWPALLib_Devices_p[Index]->FirstOfs;
    }  // is the device mapped?
    else
#endif // !HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
    if (HWPALLib_Devices_p[Index]->Platform.Mem32_p == NULL)
    {
        if (HWPALLib_MapDevice(HWPALLib_Devices_p[Index]) < 0)
        {
            return NULL;
        }
    }

    // Return the device handle
    return (Device_Handle_t)HWPALLib_Devices_p[Index];
}


/*-----------------------------------------------------------------------------
 * Device_Internal_GetIndex
 */
int
Device_Internal_GetIndex(
        const Device_Handle_t Device)
{
    Device_Admin_t * Device_p;

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, 0))
    {
        LOG_CRIT("%s: invalid device (%p) or ByteOffset (%u)\n",
                 __func__,
                 Device,
                 0);
        return -1;
    }
#endif

    return (int)Device_p->DeviceId;
}


/*-----------------------------------------------------------------------------
 * device_rw API
 *
 * These functions can be used to transfer a single 32bit word or an array of
 * 32bit words to or from a device.
 * Endianness swapping is performed on the fly based on the configuration for
 * this device.
 *
 */

/*-----------------------------------------------------------------------------
 * Device_Read32
 */
uint32_t
Device_Read32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset)
{
    uint32_t Value = 0;
    Device_Read32Check(Device, ByteOffset, &Value);
    return Value;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Check
 */
int
Device_Read32Check(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        uint32_t * const Value_p)
{
    Device_Admin_t * Device_p;
    uint32_t WordRead;
    unsigned int Idx;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return (int)DEVICE_NOT_INITIALIZED_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, ByteOffset))
    {
        LOG_CRIT("%s: Invalid Device (%p) or ByteOffset (%u)\n",
                 __func__,
                 Device,
                 ByteOffset);
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

#ifndef HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
    if (Device_p->FirstOfs == IOMEM32_MAGIC_PCICONFIGSPACE)
    {
        UMDevXSProxy_Device_PciCfg_Read32(ByteOffset, &WordRead);
    }
    else
#endif
    {
        unsigned int DeviceByteOffset = Device_p->Platform.FirstOfs + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

        Idx = DeviceByteOffset >> 2;

#ifdef HWPAL_DEVICE_READ_DELAY_US
        LOG_INFO("%s: delay %u us before read\n",
                 __func__,
                 HWPAL_DEVICE_READ_DELAY_US);
        usleep(HWPAL_DEVICE_READ_DELAY_US);
#endif

        WordRead = read32_volatile(Device_p->Platform.Mem32_p + Idx);

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAGS_SWAP)
            WordRead = Device_SwapEndian32(WordRead);
#endif
    }

#ifdef HWPAL_TRACE_DEVICE_READ
    if (Device_p->Flags & HWPAL_FLAGS_READ)
    {
        unsigned int DeviceByteOffset = Device_p->Platform.FirstOfs + ByteOffset;
        unsigned int DeviceByteOffset2 =
                Device_RemapDeviceAddress(DeviceByteOffset);

        if (DeviceByteOffset2 != DeviceByteOffset)
        {
            DeviceByteOffset2 -= Device_p->Platform.FirstOfs;
            Log_FormattedMessage("%s: 0x%x(was 0x%x) = 0x%08x (%s)\n",
                                 __func__,
                                 DeviceByteOffset2,
                                 ByteOffset,
                                 WordRead,
                                 Device_p->DevName);
        }
        else
        {
            Log_FormattedMessage("%s: %s@0x%08x => 0x%08x, dev nr = %d, "
                                 "addr = %p, offset = 0x%08x\n",
                                 __func__,
                                 Device_p->DevName,
                                 ByteOffset,
                                 WordRead,
                                 Device_p->DeviceNr,
                                 (void *)Device_p->Platform.Mem32_p,
                                 Device_p->Platform.FirstOfs + ByteOffset);
        }
    }
#endif

    *Value_p  = WordRead;

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Write32
 */
int
Device_Write32(
        const Device_Handle_t Device,
        const unsigned int ByteOffset,
        const uint32_t Value)
{
    Device_Admin_t * Device_p;
    uint32_t WordWrite = Value;
    unsigned int Idx;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(Device_p, ByteOffset))
    {
        LOG_CRIT("%s: Invalid Device (%p) or ByteOffset (%u)\n",
                 __func__, Device, ByteOffset);
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

#ifdef HWPAL_TRACE_DEVICE_WRITE
    if (Device_p->Flags & HWPAL_FLAGS_WRITE)
    {
        unsigned int DeviceByteOffset = Device_p->Platform.FirstOfs + ByteOffset;
        unsigned int DeviceByteOffset2 =
                Device_RemapDeviceAddress(DeviceByteOffset);

        if (DeviceByteOffset2 != DeviceByteOffset)
        {
            DeviceByteOffset2 -= Device_p->Platform.FirstOfs;
            Log_FormattedMessage("%s: 0x%x(was 0x%x) = 0x%08x (%s)\n",
                                 __func__, DeviceByteOffset2, ByteOffset,
                                 Value, Device_p->DevName);
        }
        else
        {
            Log_FormattedMessage("%s: %s@0x%08x = 0x%08x, dev nr = %d, "
                                 "addr = %p, offset = 0x%08x\n",
                                 __func__, Device_p->DevName,
                                 ByteOffset, Value,
                                 Device_p->DeviceNr,
                                 (void *)Device_p->Platform.Mem32_p,
                                 Device_p->Platform.FirstOfs + ByteOffset);
        }
    }
#endif

#ifndef HWPAL_REMOVE_DEVICE_PCICONFIGSPACE
    if (Device_p->FirstOfs == IOMEM32_MAGIC_PCICONFIGSPACE)
    {
        UMDevXSProxy_Device_PciCfg_Write32(ByteOffset, WordWrite);
    }
    else
#endif
    {
        uint32_t DeviceByteOffset = Device_p->Platform.FirstOfs + ByteOffset;

        DeviceByteOffset = Device_RemapDeviceAddress(DeviceByteOffset);

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAGS_SWAP)
            WordWrite = Device_SwapEndian32(WordWrite);
#endif

        Idx = DeviceByteOffset >> 2;
        write32_volatile(WordWrite, Device_p->Platform.Mem32_p + Idx);
    }

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Array
 */
int
Device_Read32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    Device_Admin_t * Device_p;
    uint32_t WordRead;
    unsigned int Idx;
    int Nwords;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

    if (Count == 0)
    {
        // avoid that 'Count-1' goes negative in test below
        return 0;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((Count < 0) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset + (((unsigned int)Count - 1) * (unsigned int)sizeof(uint32_t))))
    {
        LOG_CRIT("%s: Invalid Device (%p) or read area (%u-%u)\n",
                 __func__, Device, StartByteOffset,
                 (StartByteOffset + (((unsigned int)Count - 1) * (unsigned int)sizeof(uint32_t))));
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    Idx = (Device_p->Platform.FirstOfs + StartByteOffset) >> 2;
    for (Nwords = 0; Nwords < Count; ++Nwords, ++Idx)
    {
        WordRead = read32_volatile(Device_p->Platform.Mem32_p + Idx);

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAGS_SWAP)
            WordRead = Device_SwapEndian32(WordRead);
#endif

        MemoryDst_p[Nwords] = WordRead;

#ifdef HWPAL_TRACE_DEVICE_READ
        if (Device_p->Flags & HWPAL_FLAGS_READ)
        {
            Log_FormattedMessage("%s: rd %s@0x%08x => 0x%08x\n",
                                 __func__, Device_p->DevName,
                                 (Nwords << 2) + Device_p->Platform.FirstOfs + StartByteOffset,
                                 WordRead);
        }
#endif
    }

    return 0;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Array
 */
int
Device_Write32Array(
        const Device_Handle_t Device,
        const unsigned int StartByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    Device_Admin_t * Device_p;
    uint32_t WordWrite;
    unsigned int Idx;
    int Nwords;

    if (!HWPALLib_Device_Global.fInitialized)
    {
        LOG_CRIT("%s: failed, not initialized\n", __func__);
        return DEVICE_RW_PARAM_ERROR;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    Device_p = HWPALLib_Device2RecordPtr(Device);
#else
    Device_p = Device;
#endif

    if (Count == 0)
    {
        return 0; /* avoid that 'Count-1' goes negative in test below */
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((Count < 0) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset) ||
        !HWPALLib_IsValid(Device_p, StartByteOffset + (((unsigned int)Count - 1) *  (unsigned int)sizeof(uint32_t))))
    {
        LOG_CRIT("%s: Invalid Device (%p) or write area (%u-%u)\n",
                 __func__, Device, StartByteOffset,
                 (StartByteOffset + (((unsigned int)Count - 1) * (unsigned int)sizeof(uint32_t))));
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    Idx = (Device_p->Platform.FirstOfs + StartByteOffset) >> 2;
    for (Nwords = 0; Nwords < Count; ++Nwords, ++Idx)
    {
        WordWrite = MemorySrc_p[Nwords];

#ifdef HWPAL_DEVICE_ENABLE_SWAP
        if (Device_p->Flags & HWPAL_FLAGS_SWAP)
            WordWrite = Device_SwapEndian32(WordWrite);
#endif

        write32_volatile(WordWrite, Device_p->Platform.Mem32_p + Idx);

#ifdef HWPAL_TRACE_DEVICE_WRITE
        if (Device_p->Flags & HWPAL_FLAGS_WRITE)
        {
            Log_FormattedMessage("%s: wr %s@0x%08x = 0x%08x\n",
                                 __func__, Device_p->DevName,
                                 (Nwords << 2) + Device_p->Platform.FirstOfs + StartByteOffset,
                                 WordWrite);
        }
#endif
    }

    return 0;
}


/* end of file hwpal_device_umdevxs.c */
