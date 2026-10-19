/* adapter_driver_init.c
 *
 * Adapter top level module, EIP-13x driver's entry point.
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

#include "api_driver_init.h"    // Driver Init API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "cs_adapter.h"         // ADAPTER_DRIVER_NAME,
                                // ADAPTER_INIT_VEX_STUB_ENABLE,
                                // ADAPTER_INIT_VEX_PROXY_ENABLE
#include "adapter_thread.h"     // Adapter_ThreadStop()

// Adapter Initialization API
//#include "adapter_init.h"     // Adapter_*
//#include "adapter_global_init.h"

// Logging API
#include "log.h"                // LOG_INFO


/*----------------------------------------------------------------------------
 * Local prototypes
 */

int
Driver_Custom_Init(void);

void
Driver_Custom_UnInit(void);


/*----------------------------------------------------------------------------
 * Driver130_Init
 */
int
Driver130_Init(void)
{
    LOG_INFO("%s: initialize driver %s\n", __func__, ADAPTER_DRIVER_NAME);

#ifdef DO_GLOBAL_INIT
    if (!Adapter_Global_Init())
    {
        Adapter_UnInit();
        return -1;
    }
#endif

    if (Driver_Custom_Init() < 0)
    {
        return -1;
    }

    LOG_INFO("%s: done\n", __func__);

    return 0;   // success
}


/*----------------------------------------------------------------------------
 * Driver130_Exit
 */
void
Driver130_Exit(void)
{
    LOG_INFO("%s: exit driver %s\n", __func__, ADAPTER_DRIVER_NAME);

    Driver_Custom_UnInit();

    //Adapter_Global_UnInit();

    /* Make sure the task environment is cleaned-up */
    Adapter_ThreadStop(NULL);

    LOG_INFO("%s: done\n", __func__);
}


#include "adapter_driver_init_ext.h"


/* end of file adapter_driver_init.c */
