/* adapter_global_init.c
 *
 * Global Control initialization module.
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
#include "adapter_global_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter_global.h"           // ADAPTER_GLOBAL_PRNG_SEED

// Global Control API: Packet I/O
#include "api_global_eip130.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"                 // uint8_t, uint32_t, bool

// Driver Framework C Library API
#include "clib.h"                       // memcpy

// EIP-130 Driver Library Global Control API
#include "eip130_global_event.h"        // Event Management
#include "eip130_global_init.h"         // Init/Uninit

#include "device_types.h"               // Device_Handle_t
#include "device_mgmt.h"                // Device_find
#include "log.h"                        // Log API


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Adapter_Global_StatusReport()
 *
 * Obtain all available global status information from the EIP-130 driver
 * and report it.
 */
static void
Adapter_Global_StatusReport(void)
{
    LOG_INFO("%s\n", __func__);

    LOG_CRIT("Global Status of the EIP-130\n");
}


/*----------------------------------------------------------------------------
 * Adapter_Global_Init()
 *
 */
bool
Adapter_Global_Init(void)
{
    LOG_INFO("%s\n", __func__);

    return true; // success
}


/*----------------------------------------------------------------------------
 * Adapter_Global_UnInit()
 *
 */
void
Adapter_Global_UnInit(void)
{
    LOG_INFO("%s\n", __func__);

    Adapter_Global_StatusReport();
}


/* end of file adapter_global_init.c */

