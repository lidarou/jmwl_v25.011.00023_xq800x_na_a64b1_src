/* umdevxs_ofdev.c
 *
 * Platform device support for the Linux UMDevXS driver.
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

#ifndef UMDEVXS_REMOVE_DEVICE_OF

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "umdevxs_internal.h"

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
#include "umdevxs_ofdev.h"          // UMDevXS_OFDev_GetDevice
#endif


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_umdevxs.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"             // uint8_t, MASK_xx_BITS, etc.

// Linux Kernel Module interface
#include "lkm.h"                    // LKM_*

// Linux Kernel API
#include <linux/platform_device.h>  // platform_*,
#include <linux/mm.h>               // remap_pfn_range

#ifdef UMDEVXS_USE_RPM
// Runtime Power Management Kernel Macros API
#include "rpm_kernel_macros.h"      // RPM_*
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

// Pointer to the array of IRQ devices
#ifndef UMDEVXS_REMOVE_INTERRUPT
static int * UMDEVXS_OFDev_Virtual_IRQ_p;
#endif


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_Init
 *
 * Returns <0 on error.
 * Returns >=0 on success. The number is the interrupt number associated
 * with this OF device.
 */
int
UMDevXS_OFDev_Init(void)
{
    LKM_Init_t LKMInit;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

    ZEROINIT(LKMInit);

    LKMInit.DriverName_p = UMDEVXS_MODULENAME;

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
    LKMInit.fRetainMap = true;
#endif

#ifdef UMDEVXS_USE_RPM
    LKMInit.PM_p = RPM_OPS_PM;
#endif

    if (LKM_Init(&LKMInit) < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: Failed to register the OF device\n",
                 __func__);
        return -1;
    }

#ifdef UMDEVXS_USE_RPM
    if (RPM_INIT_MACRO(LKM_DeviceGeneric_Get()) != RPM_SUCCESS)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: RPM_Init() failed\n", __func__);
        LKM_Uninit();
        return -2; // error
    }
#endif

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left, initialization successful\n",
             __func__);

#ifndef UMDEVXS_REMOVE_INTERRUPT
    UMDEVXS_OFDev_Virtual_IRQ_p = (int *)LKMInit.CustomInitData_p;

    return UMDEVXS_OFDev_Virtual_IRQ_p[0];
#else
    return 0; // success
#endif // UMDEVXS_REMOVE_INTERRUPT
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_UnInit
 */
void
UMDevXS_OFDev_UnInit(void)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: entered\n", __func__);

#ifdef UMDEVXS_USE_RPM
    // Check if a race condition is possible here with auto-suspend timer
    (void)RPM_UNINIT_MACRO();
#endif

    LKM_Uninit();

#ifndef UMDEVXS_REMOVE_INTERRUPT
    UMDEVXS_OFDev_Virtual_IRQ_p = NULL;
#endif

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: left\n", __func__);
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_Map
 */
int
UMDevXS_OFDev_Map(
        unsigned int SubsetStart,       // defined
        unsigned int SubsetSize,        // defined
        unsigned int Length,            // requested
        struct vm_area_struct * vma_p)
{
    int res;
    unsigned long address = 0;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Subset start=%u and size=%u, Length=%u\n",
             __func__, SubsetStart, SubsetSize, Length);

    // honor requested length if less than defined
    if (SubsetSize > Length)
    {
        SubsetSize = Length;
    }

    // reject requested length request if greater than defined
    if (Length > SubsetSize)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: length rejected (%u)\n",
                 __func__, Length);
        return -1;
    }

    // was the OF device enabled by the OS
    // this only happens when a compatible OF device is found
    if (LKM_DeviceSpecific_Get() == NULL)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: platform device not found\n",
                 __func__);
        return -2;
    }

    // get OF device physical address
    {
        struct resource * rsc_p = NULL;

        // Exported under GPL
        rsc_p = platform_get_resource(LKM_DeviceSpecific_Get(),
                                      IORESOURCE_MEM,
                                      0); // resource ID

        // device tree specific for this OF device
        // only 32-bit physical addresses are supported
        if(rsc_p)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX "%s: mem start=%08x, end=%08x, flags=%08x\n",
                     __func__,
                     (unsigned int)rsc_p->start,
                     (unsigned int)rsc_p->end,
                     (unsigned int)rsc_p->flags);

            if(Length > (rsc_p->end - rsc_p->start + 1))
            {
                LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: length rejected (%u)\n",
                         __func__, Length);
                return -3;
            }

            address = (unsigned long)rsc_p->start + (unsigned long)SubsetStart;
        }
        else
        {
            LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: memory resource not found\n",
                     __func__);
            return -4;
        }
    }

    // avoid caching and buffering
    vma_p->vm_page_prot = pgprot_noncached(vma_p->vm_page_prot);
    vma_p->vm_flags |= VM_IO;

    // map the range into application space
    res = remap_pfn_range(vma_p,
                          vma_p->vm_start,
                          address >> PAGE_SHIFT,
                          Length,
                          vma_p->vm_page_prot);
    if (res < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: remap_pfn_range failed (%d),"
                 " addr = %08x, len = %x\n",
                 __func__, res, (unsigned int)address, Length);
        return -5;
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: successfully mapped platform device\n",
             __func__);

    // return success
    return 0;
}


/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetReference
 */
void*
UMDevXS_OFDev_GetReference(void)
{
    return LKM_DeviceGeneric_Get();
}


#ifndef UMDEVXS_REMOVE_INTERRUPT
/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetInterrupts
 */
unsigned int
UMDevXS_OFDev_GetInterrupt(unsigned int index)
{
    if (index >= UMDEVXS_INTERRUPT_IC_DEVICE_COUNT)
    {
        return 0;
    }
    else
    {
        return UMDEVXS_OFDev_Virtual_IRQ_p[index];
    }
}
#endif


#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
/*----------------------------------------------------------------------------
 * UMDevXS_OFDev_GetDevice
 */
int
UMDevXS_OFDev_GetDevice(
              unsigned int DeviceID,
              struct platform_device ** OF_Device_pp,
              void __iomem ** MappedBaseAddr_pp)
{
    *OF_Device_pp      = LKM_DeviceSpecific_Get();
    *MappedBaseAddr_pp = LKM_MappedBaseAddr_Get();

    IDENTIFIER_NOT_USED(DeviceID);

    return 0;
}
#endif // UMDEVXS_ENABLE_KERNEL_SUPPORT


#endif // UMDEVXS_REMOVE_DEVICE_OF


/* end of file umdevxs_ofdev.c */
