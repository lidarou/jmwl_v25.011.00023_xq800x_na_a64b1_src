/**
 * @file api_val_aunlock.h
 *
 * @brief VAL API - Authenticated Unlock and Secure Debug related services
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

#ifndef INCLUDE_GUARD_API_VAL_AUNLOCK_H
#define INCLUDE_GUARD_API_VAL_AUNLOCK_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "api_val_buffers.h"            // ValOctetsIn_t, ValOctetsOut_t, ValSize_t
#include "api_val_asset_defs.h"         // ValAssetId_t, ValAssetNumber_t
#include "api_val_asym_defs.h"          // ValAsymBigInt_t
#include "api_val_result.h"             // ValStatus_t

/*-----------------------------------------------------------------------------
 * val_AuthenticatedUnlockStart
 */
/**
 * This function initializes and starts the Authenticated Unlocked scheme.
 *
 * @param [in] AuthKeyNumber
 *     The asset number to search for in the (static) Asset Store that contains
 *     the Authenticated Unlocked public key. Note that the supported values
 *     for AuthKeyNumber are defined by the device manufacturer and vendor
 *     and can differ from product to product.
 *
 * @param [out] AuthStateASId_p
 *     Pointer to the memory location where the AssetId for Authenticated
 *     Unlock state will be written. The AssetId is needed
 *     for all Authenticated Unlock related operations.
 *
 * @param [out] Nonce_p
 *     Pointer to the memory location where the HW Nonce will be written for
 *     the signing and is later on used for the verification.
 *
 * @param [in,out] NonceLength_p
 *     Pointer to the memory location where the size of the Nonce in bytes
 *     will be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AuthenticatedUnlockStart(
        ValAssetNumber_t AuthKeyNumber,
        ValAssetId_t * AuthStateASId_p,
        ValOctetsOut_t * Nonce_p,
        ValSize_t * NonceLength_p);


/*-----------------------------------------------------------------------------
 * val_AuthenticatedUnlockVerify
 */
/**
 * This function verifies the signature to get to the unlocked state.
 *
 * @param [in] AuthStateASId
 *     Reference to the Authenticated Unlock state Asset.
 *     This reference is provided by val_AuthenticatedUnlockStart().
 *
 * @param [in] Signature_p
 *     Pointer to the memory location where the signature is located for
 *     the verification.
 *
 * @param [in] Nonce_p
 *     Pointer to the memory location where the host Nonce is located for
 *     the verification.
 *
 * @param [in] NonceLength
 *     Size of the Nonce in bytes.
 *
 * @return One of the ValStatus_t values.
 */
#ifdef EIP130_HW_V2
ValStatus_t
val_AuthenticatedUnlockVerify(
        const ValAssetId_t AuthStateASId,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * Nonce_p,
        const ValSize_t NonceLength);
#else
ValStatus_t
val_AuthenticatedUnlockVerify(
        const ValAssetId_t AuthStateASId,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * Nonce_p,
        const ValSize_t NonceLength);
#endif




/*-----------------------------------------------------------------------------
 * val_AuthenticatedUnlockRelease
 */
/**
 * This function release the Authenticated Unlock state Asset.
 *
 * @param [in] AuthStateASId
 *     Reference to the Authenticated Unlock state Asset.
 *     This reference is provided by val_AuthenticatedUnlockStart().
 *
 * @return One of the ValStatus_t values.
 */
#define val_AuthenticatedUnlockRelease(AuthStateASId) val_AssetFree(AuthStateASId)


/*-----------------------------------------------------------------------------
 * val_SecureDebug
 */
/**
 * This function set the Secure Debug port state after a successfull
 * authenticated unlock operation.
 *
 * @param [in] AuthStateASId
 *     Reference to the Authenticated Unlock state Asset.
 *     This reference is provided by val_AuthenticatedUnlockStart().
 *
 * @param [in] fSet
 *     Indication to enable (true) or disable (false) the debug port.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SecureDebug(
        const ValAssetId_t AuthStateASId,
        const bool fSet);


#endif /* INCLUDE_GUARD_API_VAL_AUNLOCK_H */

/* end of file api_val_aunlock.h */
