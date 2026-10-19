/* adapter_val_claim.c
 *
 * Implementation of the VAL API.
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

#include "c_adapter_val.h"              // configuration

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "api_val_claim.h"              // the API to implement
#include "adapter_val_internal.h"       // val_ExchangeToken()
#include "adapter_vex.h"                // VexToken_Command_t, VexToken_Result_t


/*----------------------------------------------------------------------------
 * val_Claim
 */
#ifndef VAL_REMOVE_CLAIM
ValStatus_t
val_Claim(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_CLAIMCONTROL;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE;

    // Exchange service request with the next driver level
    return val_ExchangeToken(&t_cmd, &t_res);
}
#endif /* !VAL_REMOVE_CLAIM */


/*----------------------------------------------------------------------------
 * val_ClaimOverrule
 */
#ifndef VAL_REMOVE_CLAIM
ValStatus_t
val_ClaimOverrule(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_CLAIMCONTROL;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE_OVERRULE;

    // Exchange service request with the next driver level
    return val_ExchangeToken(&t_cmd, &t_res);
}
#endif /* !VAL_REMOVE_CLAIM */


/*----------------------------------------------------------------------------
 * val_ClaimRelease
 */
#ifndef VAL_REMOVE_CLAIM
ValStatus_t
val_ClaimRelease(void)
{
    VexToken_Command_t t_cmd;
    VexToken_Result_t t_res;

    // Format service request
    t_cmd.OpCode  = VEXTOKEN_OPCODE_CLAIMCONTROL;
    t_cmd.SubCode = VEXTOKEN_SUBCODE_CLAIM_RELEASE;

    // Exchange service request with the next driver level
    return val_ExchangeToken(&t_cmd, &t_res);
}
#endif /* !VAL_REMOVE_CLAIM */


/* end of file adapter_val_claim.c */
