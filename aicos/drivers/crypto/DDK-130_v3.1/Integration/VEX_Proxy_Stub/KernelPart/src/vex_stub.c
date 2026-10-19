/* vex_stub.c
 *
 * Implementation of the VEX API Stub.
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
 * This module uses (requires) the following interface(s):
 */

// UMDevXS Command API
#include "umdevxs_cmd.h"        // UMDevXS_CmdRsp_t

// VEX Command API
#include "vex_cmd.h"            // VEX_OPCODE_*, vexStub_*

// VEX Adapter API
#include "adapter_vex.h"

// Default configuration
#include "c_vex_stub.h"

// Logging API
#include "log.h"

// Driver Framework C Run-Time Library API
//#include "clib.h"

// Linux kernel API
#include <linux/version.h>          // LINUX_VERSION_CODE, KERNEL_VERSION
#include <linux/uaccess.h>          // copy_to/from_user, access_ok
#include <linux/errno.h>            // EIO


/*----------------------------------------------------------------------------
 * Local vexStub_HandleCmd
 */
int
vexStub_HandleCmd(
        void * AppID,
        UMDevXS_CmdRsp_t * const CmdRsp_p)
{
    VexToken_Command_t CommandToken;
    VexToken_Result_t ResultToken;
    vexStub_DataIO_t DataIO;

    if (CmdRsp_p->uint1 != sizeof(vexStub_DataIO_t))
    {
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed, data size mismatch %d!=%d\n",
                 __func__, CmdRsp_p->uint1, (int)sizeof(vexStub_DataIO_t));
        return -EINVAL;     // ## RETURN ##
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    if (!access_ok(VERIFY_WRITE, CmdRsp_p->ptr1, sizeof(vexStub_DataIO_t)))
#else
    if (!access_ok(CmdRsp_p->ptr1, sizeof(vexStub_DataIO_t)))
#endif
    {
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on access_ok\n", __func__);
        return -EINVAL;     // ## RETURN ##
    }

    // copy the DataIO from application space into kernel space
    if (copy_from_user(&DataIO,
                       CmdRsp_p->ptr1,
                       sizeof(vexStub_DataIO_t)) != 0)
    {
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on copy_from_user\n",
                 __func__);
        return -EINVAL;     // ## RETURN ##
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    if (!access_ok(VERIFY_WRITE, DataIO.InData_p, DataIO.InDataByteCount) ||
        DataIO.InDataByteCount != sizeof(VexToken_Command_t))
#else
    if (!access_ok(DataIO.InData_p, DataIO.InDataByteCount) ||
        (DataIO.InDataByteCount != sizeof(VexToken_Command_t)))
#endif
    {
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on access_ok or data size mismatch %d/%d\n",
                 __func__, DataIO.InDataByteCount, (unsigned int)sizeof(VexToken_Command_t));
        return -EINVAL;     // ## RETURN ##
    }

    // copy the CommandToken from application space into kernel space
    if (copy_from_user(&CommandToken,
                       DataIO.InData_p,
                       sizeof(VexToken_Command_t)) != 0)
    {
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on copy_from_user\n",
                 __func__);
        return -EINVAL;     // ## RETURN ##
    }

    switch (CmdRsp_p->Opcode)
    {
    case VEX_OPCODE_TOKEN:
        CmdRsp_p->Error = vex_LogicalToken(&CommandToken, &ResultToken);
        if (CmdRsp_p->Error >= 0)
        {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
            if (!access_ok(VERIFY_WRITE, DataIO.OutData_p, DataIO.OutDataByteCount) ||
                (DataIO.OutDataByteCount != sizeof(VexToken_Result_t)))
#else
            if (!access_ok(DataIO.OutData_p, DataIO.OutDataByteCount) ||
                (DataIO.OutDataByteCount != sizeof(VexToken_Result_t)))
#endif
            {
                LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on access_ok or data size mismatch %d/%d\n",
                         __func__, DataIO.OutDataByteCount, (unsigned int)sizeof(VexToken_Result_t));
                return -EINVAL;
            }

            if (copy_to_user(DataIO.OutData_p,
                             &ResultToken,
                             sizeof(VexToken_Result_t)) != 0)
            {
                LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: failed on copy_to_user\n",
                         __func__);
                return -EINVAL;     // ## RETURN ##
            }
        }
        break;

    default:
        LOG_CRIT(VEX_STUB_LOG_PREFIX "%s: unsupported opcode %d\n",
                __func__, CmdRsp_p->Opcode);
        return -EINVAL;     // ## RETURN ##
    }

    return 0;
}


/* end of file vex_stub.c */
