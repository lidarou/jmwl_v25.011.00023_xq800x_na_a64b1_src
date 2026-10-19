/* adapter_vex_claim.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the services with which the exclusive mailbox locking
 * can be controlled.
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

#include "c_adapter_vex.h"          // configuration

#include "basic_defs.h"
#include "clib.h"

#include "adapter_vex_internal.h"   // API implementation


/*----------------------------------------------------------------------------
 * vex_Claim
 */
VexStatus_t
vex_Claim(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    uint32_t Identity;
    uint8_t MailboxNumber;
    int funcres;

    // Get and set identity of calling process
    Identity = vex_IdentityGet();
    if (Identity == 0)
    {
        return VEX_NO_IDENTITY;
    }

    // Get mailbox number to use
    MailboxNumber = vex_MailboxGet(Identity);
    if (MailboxNumber == 0)
    {
        return VEX_NO_MAILBOX;
    }

    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE:
        funcres = vex_DeviceLinkMailbox(MailboxNumber, Identity);
        if (funcres != 0)
        {
            if (funcres == -5)
            {
                return VEX_MAILBOX_IN_USE;
            }
            return VEX_INTERNAL_ERROR;
        }
        break;

    case VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE_OVERRULE:
        funcres = vex_DeviceLinkMailboxOverrule(MailboxNumber, Identity);
        if (funcres != 0)
        {
            return VEX_INTERNAL_ERROR;
        }
        break;

    case VEXTOKEN_SUBCODE_CLAIM_RELEASE:
        funcres = vex_DeviceUnlinkMailbox(MailboxNumber, Identity);
        if (funcres != 0)
        {
            return VEX_INTERNAL_ERROR;
        }
        break;

    default:
        return VEX_UNSUPPORTED;
    }

    ResultToken_p->Result = 0;
    return VEX_SUCCESS;
}

/* end of file adapter_vex_claim.c */
