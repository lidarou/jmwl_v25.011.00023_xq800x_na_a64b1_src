/* adapter_init_val.c
 *
 * Adapter module responsible for Adapter VAL initialization tasks.
 *
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

#include "adapter_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

#ifdef ADAPTER_EIP130_INTERRUPTS_ENABLE
#include "adapter_interrupts.h"         // Adapter_Interrupts_Init/UnInit
#endif

// Logging API
#include "log.h"                        // LOG_*

// Driver Framework Device API
#include "device_mgmt.h"                // Device_Initialize/UnInitialize
#include "device_rw.h"                  // Device_Read32, Device_Write32

// Driver Framework DMAResource API
#include "dmares_mgmt.h"                // DMAResource_Init/UnInit

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // bool, true, false


/*----------------------------------------------------------------------------
 * Local variables
 */

static bool Adapter_IsInitialized = false;
static int Device_IRQ;

/*----------------------------------------------------------------------------
 * Adapter_Init
 *
 * Return Value
 *     true   Success
 *     false  Failure (fatal!)
 */
bool
Adapter_Init(void)
{
    Device_IRQ = -1;

    if (Adapter_IsInitialized != false)
    {
        LOG_WARN("%s: Already initialized\n", __func__);
        return true;
    }

    // trigger first-time initialization of the adapter
    if (Device_Initialize(&Device_IRQ) < 0)
    {
        return false;
    }

    if (!DMAResource_Init())
    {
        Device_UnInitialize();
        return false;
    }

#ifdef ADAPTER_EIP130_INTERRUPTS_ENABLE
    LOG_WARN("%s: Adapter_Interrupts enabled\n", __func__);
    if (Adapter_Interrupts_Init(Device_IRQ) < 0)
    {
        LOG_CRIT("%s: Adapter_Interrupts_Init failed\n", __func__);

        DMAResource_UnInit();

        Device_UnInitialize();

        return false;
    }
#else
    LOG_WARN("%s: Adapter_Interrupts not enabled\n", __func__);
#endif

    Adapter_IsInitialized = true;

    return true;    // success
}


/*----------------------------------------------------------------------------
 * Adapter_UnInit
 */
void
Adapter_UnInit(void)
{
    if (!Adapter_IsInitialized)
    {
        LOG_WARN("%s: Adapter is not initialized\n", __func__);
        return;
    }

    Adapter_IsInitialized = false;

    DMAResource_UnInit();

#ifdef ADAPTER_EIP130_INTERRUPTS_ENABLE
    Adapter_Interrupts_UnInit(Device_IRQ);
#endif

    Device_UnInitialize();
}


/*----------------------------------------------------------------------------
 * Adapter_Report_Build_Params
 */
void
Adapter_Report_Build_Params(void)
{
#ifdef LOG_INFO_ENABLED
    int dummy;

    // This function is dependent on config file cs_adapter.h.
    // Please update this when Config file for Adapter is changed.
    Log_FormattedMessage("Adapter build configuration:\n");

#define REPORT_SET(_X) \
    Log_FormattedMessage("\t" #_X "\n")

#define REPORT_STR(_X) \
    Log_FormattedMessage("\t" #_X ": %s\n", _X)

#define REPORT_INT(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": %d\n", _X)

#define REPORT_HEX32(_X) \
    dummy = _X; Log_FormattedMessage("\t" #_X ": 0x%08X\n", _X)

#define REPORT_EQ(_X, _Y) \
    dummy = (_X + _Y); Log_FormattedMessage("\t" #_X " == " #_Y "\n")

#define REPORT_EXPL(_X, _Y) \
    Log_FormattedMessage("\t" #_X _Y "\n")

    // Adapter VAL
#ifdef ADAPTER_STRICT_ARGS
    REPORT_SET(ADAPTER_STRICT_ARGS);
#endif

#ifdef ADAPTER_REMOVE_BOUNCEBUFFERS
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is SET => Bounce DISABLED");
#else
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is NOT set => Bounce ENABLED");
#endif

#ifdef ADAPTER_ENABLE_SWAP
    REPORT_EXPL(ADAPTER_ENABLE_SWAP, " is SET => Byte swap in words ENABLED");
#else
    REPORT_EXPL(ADAPTER_ENABLE_SWAP, " is NOT set => Byte swap in words DISABLED");
#endif

#ifdef ADAPTER_EIP130_INTERRUPTS_ENABLE
    REPORT_EXPL(ADAPTER_EIP130_INTERRUPTS_ENABLE,
            " is SET => Interrupts ENABLED");
#else
    REPORT_EXPL(ADAPTER_EIP130_INTERRUPTS_ENABLE,
            " is NOT set => Interrupts DISABLED");
#endif

#ifdef ADAPTER_64BIT_HOST
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is SET => addresses are 64-bit");
#else
    REPORT_EXPL(ADAPTER_64BIT_HOST,
                " is NOT set => addresses are 32-bit");
#endif

#ifdef ADAPTER_64BIT_DEVICE
    REPORT_EXPL(ADAPTER_64BIT_DEVICE,
                " is SET => full 64-bit DMA addresses usable");
#else
    REPORT_EXPL(ADAPTER_64BIT_DEVICE,
                " is NOT set => DMA addresses must be below 4GB");
#endif

    // Log
    Log_FormattedMessage("Logging:\n");

#if (LOG_SEVERITY_MAX == LOG_SEVERITY_INFO)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_INFO);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_WARNING)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_WARNING);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_CRITICAL)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_CRITICAL);
#else
    REPORT_EXPL(LOG_SEVERITY_MAX, " - Unknown (not info/warn/crit)");
#endif

    // Adapter other
    Log_FormattedMessage("Other:\n");
    REPORT_STR(ADAPTER_DRIVER_NAME);

    IDENTIFIER_NOT_USED(dummy);

#endif //LOG_INFO_ENABLED
}


/* end of file adapter_init_val.c */
