/* adapter_vex_init.c
 *
 * Adapter VEX initialization module.
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

// VEX Adapter API
#include "adapter_vex.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_adapter_vex.h"
#include "adapter_vex_internal.h"   // vex_DeviceInit/Exit, vex_InitBufManager
#include "log.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Global constants
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Local prototypes
 */



/*----------------------------------------------------------------------------
 * vex_Init
 */
int
vex_Init(void)
{
    int rc;

    rc = vex_LockInit();
    if (rc < 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "FAILED to initialize lock (%d)\n", rc);
        return -1;
    }

    rc = vex_DeviceInit();
    if (rc < 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "FAILED to initialize device (%d)\n", rc);
        if ((rc == -1) || (rc == -100))
        {
            vex_LockExit();
            return -2;
        }
    }

#if VEX_USE_INTERRUPTS > 0
    rc = vex_InitInterrupts();
    if (rc < 0)
    {
        LOG_CRIT(VEX_LOG_PREFIX "FAILED to initialize interrupts (%d)\n", rc);
        vex_DeviceExit();
        vex_LockExit();
        return -3;
    }
#endif
    vex_InitBufManager();
    vex_InitLogicalToken();

    return 0;
}


/*----------------------------------------------------------------------------
 * vex_UnInit
 */
void
vex_UnInit(void)
{
    vex_UnInitLogicalToken();
#if VEX_USE_INTERRUPTS > 0
    vex_UnInitInterrupts();
#endif
    vex_DeviceExit();
    vex_LockExit();

    return;
}


/* end of file adapter_vex_init.c */

