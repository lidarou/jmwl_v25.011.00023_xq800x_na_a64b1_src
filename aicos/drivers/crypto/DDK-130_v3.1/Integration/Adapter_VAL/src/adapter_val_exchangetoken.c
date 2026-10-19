/* adapter_val_exchangetoken.c
 *
 * Implementation of the VAL API.
 *
 * This file contains the token exchange with the next driver level.
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

#include "basic_defs.h"                 // uint16_t
#include "log.h"

#include "adapter_val_internal.h"       // the API to implement
#include "adapter_val_internal_ext.h"   // User or Kernel mode indication
#include "adapter_vex.h"                // VexStatus_t, vex_ExchangeToken()


/*----------------------------------------------------------------------------
 * val_ExchangeToken
 *
 * Allocate an asset and set its policy. Its content is setup later.
 */
ValStatus_t
val_ExchangeToken(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres;

#ifdef VAL_STRICT_ARGS
    // In general an internal function, so do not check arguments
    //if (CommandToken_p == NULL ||
    //    ResultToken_p == NULL)
    //{
    //    return VAL_INTERNAL_ERROR;
    //}
#endif

    CommandToken_p->fFromUserSpace = ADAPTER_VAL_INTERNAL_USER_SPACE_FLAG;
    funcres = vex_LogicalToken(CommandToken_p, ResultToken_p);
    if (funcres != VEX_SUCCESS)
    {
        LOG_WARN("%s::vex_LogicalToken()=%d\n", __func__, funcres);
        if ((funcres == VEX_REPONSE_TIMEOUT) ||
            (funcres == VEX_DATA_TIMEOUT))
        {
            return VAL_TIMEOUT_ERROR;
        }
        return VAL_INTERNAL_ERROR;
    }

    return VAL_SUCCESS;
}


/* end of file adapter_val_exchangetoken.c */
