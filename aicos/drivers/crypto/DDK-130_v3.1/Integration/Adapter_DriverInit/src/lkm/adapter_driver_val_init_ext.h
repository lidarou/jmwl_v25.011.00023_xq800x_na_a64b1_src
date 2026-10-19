/* adapter_driver_val_init_ext.h
 *
 * Linux kernel specific Adapter extensions
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

#ifndef INCLUDE_GUARD_ADAPTER_DRIVER_VAL_INIT_EXT_H
#define INCLUDE_GUARD_ADAPTER_DRIVER_VAL_INIT_EXT_H

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */



/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter.h"

// Linux Kernel API
#include <linux/init.h>             // module_init, module_exit
#include <linux/module.h>           // EXPORT_SYMBOL

// Adapter Initialization API
#include "adapter_init.h"           // Adapter_*

#ifdef ADAPTER_INIT_VEX_STUB_ENABLE
#include "vex_cmd.h"                // vexStub_HandleCmd
#include "umdevxs_chrdev.h"
#endif

#include "adapter_vex.h"            // vex_LogicalToken, vex_Init/UnInit


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifdef ADAPTER_INIT_VEX_STUB_ENABLE
EXPORT_SYMBOL(vex_LogicalToken);
#endif

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rambus");
MODULE_DESCRIPTION("VAL Driver for EIP-13x.");

module_init(Driver130_Init);
module_exit(Driver130_Exit);


/*----------------------------------------------------------------------------
 * Forward declarations
 */

#ifdef ADAPTER_INIT_VEX_STUB_ENABLE
// UMDevXS character Device support
int
UMDevXS_ChrDev_Init(void);

void
UMDevXS_ChrDev_UnInit(void);
#endif // ADAPTER_INIT_VEX_STUB_ENABLE


/*----------------------------------------------------------------------------
 * DriverVAL_Custom_Init
 */
int
DriverVAL_Custom_Init(void)
{
    Adapter_Report_Build_Params();

    if (!Adapter_Init())
    {
        return -1;
    }

    vex_Init();

#ifdef ADAPTER_INIT_VEX_STUB_ENABLE
    {
        int rc;

        UMDevXS_ChrDev_HandleCmdFunc_Set(vexStub_HandleCmd);

        rc = UMDevXS_ChrDev_Init();
        if (rc < 0)
        {
            vex_UnInit();
            Adapter_UnInit();
        }

        return rc;
    }
#else
    return 0; // success
#endif
}


/*----------------------------------------------------------------------------
 * DriverVAL_Custom_UnInit
 */
void
DriverVAL_Custom_UnInit(void)
{
#ifdef ADAPTER_INIT_VEX_STUB_ENABLE
    UMDevXS_ChrDev_HandleCmdFunc_Set(NULL);

    UMDevXS_ChrDev_UnInit();
#endif

    vex_UnInit();

    Adapter_UnInit();
}


// VAL API LKM implementation extensions
#include "adapter_val_lkm_ext.h"


#endif /* INCLUDE_GUARD_ADAPTER_DRIVER_VAL_INIT_EXT_H */

/* end of file adapter_driver_val_init_ext.h */
