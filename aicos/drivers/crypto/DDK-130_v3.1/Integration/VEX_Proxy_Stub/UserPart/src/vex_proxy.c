/* vex_proxy.c
 *
 * Implementation of the VEX API Proxy.
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
#include "c_vex_proxy.h"

// UMDevXS Command API
#include "umdevxs_cmd.h"        // UMDevXS_CmdRsp_t

// VEX Command API
#include "vex_cmd.h"            // VEX_OPCODE_*, vexStub_*

#include <fcntl.h>              // open, O_RDWR
#include <unistd.h>             // close, write
#include <string.h>             // memset


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define ZEROINIT(_x)  memset(&_x, 0, sizeof(_x))


/*----------------------------------------------------------------------------
 * Global constants
 */

static const char vexProxy_NodeName[] = VEX_PROXY_NODE_NAME;


/*----------------------------------------------------------------------------
 * Local variables
 */

// character device file descriptor
static int vexProxy_fd = -1;


/*----------------------------------------------------------------------------
 * Local prototypes
 */



/*----------------------------------------------------------------------------
 * vex_LogicalToken
 */
VexStatus_t
vex_LogicalToken(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    int res;
    UMDevXS_CmdRsp_t CmdRsp;
    vexStub_DataIO_t DataIO;

    if ((CommandToken_p == NULL) ||
        (ResultToken_p == NULL))
    {
        return VEX_BAD_ARGUMENT;
    }

    // Try to open the character device
    if (vexProxy_fd < 0)
    {
        vexProxy_fd = open(vexProxy_NodeName, O_RDWR);
        if (vexProxy_fd < 0)
        {
            return VEX_NOT_CONNECTED;
        }
    }

    // Indicate call is from User Space
    CommandToken_p->fFromUserSpace = true;

    // Prepare input data
    DataIO.InData_p         = CommandToken_p;
    DataIO.InDataByteCount  = sizeof(VexToken_Command_t);
    DataIO.OutData_p        = ResultToken_p;
    DataIO.OutDataByteCount = sizeof(VexToken_Result_t);

    // zero-init also protects against future extensions
    ZEROINIT(CmdRsp);

    CmdRsp.Magic  = UMDEVXS_CMDRSP_MAGIC;
    CmdRsp.Opcode = VEX_OPCODE_TOKEN;
    CmdRsp.ptr1   = &DataIO;
    CmdRsp.uint1  = sizeof(vexStub_DataIO_t);

    // Hand-over the call to Kernel Space with write()
    // - write() is a blocking call
    // - it takes the pointer to the CmdRsp structure
    // - processes the CmdRsp structure and fills it with the result
    res = (int)write(vexProxy_fd, &CmdRsp, sizeof(UMDevXS_CmdRsp_t));
    if (res != sizeof(UMDevXS_CmdRsp_t))
    {
        return VEX_INTERNAL_ERROR;
    }

    // Return the VEX result
    return CmdRsp.Error;
}


/* end of file vex_proxy.c */
