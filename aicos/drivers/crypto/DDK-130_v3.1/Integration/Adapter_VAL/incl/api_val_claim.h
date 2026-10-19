/**
 * @file api_val_claim.h
 *
 * @brief VAL API - Mailbox locking control services
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

#ifndef INCLUDE_GUARD_API_VAL_CLAIM_H
#define INCLUDE_GUARD_API_VAL_CLAIM_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "api_val_result.h"         // ValStatus_t


/**
 * Next functions are intended to control an exclusive lock of the EIP-13x
 * mailbox for the calling process. Generally, the mailbox is locked for each
 * operation and unlocked immediately afterwards, but with these functions the
 * mailbox can be locked exclusively for the calling process so that a sequence
 * of operations can be performed.
 * The main procedure is:
 * - val_Claim                  exclusively locks the mailbox for use
 * - ...                        various operations
 * - val_ServiceZeroizeMailbox  if needed, clears security relevant information
 * - val_ClaimRelease           releases the exclusive mailbox lock
 */

/*-----------------------------------------------------------------------------
 * val_Claim
 */
/**
 * This function claims exclusive use of the mailbox for the calling process,
 * if possible.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_Claim(void);

/*-----------------------------------------------------------------------------
 * val_ClaimOverrule
 */
/**
 * This function claims exclusive use of the mailboxfor the calling process.
 * Note that this function overrules any existing claim and can only be
 * performed by processes running on the master CPU (Host).
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ClaimOverrule(void);

/*-----------------------------------------------------------------------------
 * val_Release
 */
/**
 * This function releases the exclusive use of the mailbox, if one was pending
 * for the calling process.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_ClaimRelease(void);


#endif /* INCLUDE_GUARD_API_VAL_CLAIM_H */

/* end of file api_val_claim.h */
