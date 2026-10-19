/**
 * @file api_val_symkeywrap.h
 *
 * @brief VAL API - Symmetric Crypto based key wrap related services
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

#ifndef INCLUDE_GUARD_API_VAL_SYMKEYWRAP_H
#define INCLUDE_GUARD_API_VAL_SYMKEYWRAP_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"                 // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"             // ValStatus_t
#include "api_val_buffers.h"            // ValOctetsIn_t, ValOctetsOut_t
#include "api_val_asset_defs.h"         // ValAssetId_t
#include "api_val_sym_defs.h"           // VAL_SYM_ALGO_CIPHER_AES

/* for backwards compatibility */
#define val_SymAesKeyUnwrap(a, b, c, d, e, f, g) val_SymKeyUnwrap(a, b, c, d, e, f, g, VAL_SYM_ALGO_CIPHER_AES)
#define val_SymAesKeyWrap(a, b, c, d, e, f, g) val_SymKeyWrap(a, b, c, d, e, f, g, VAL_SYM_ALGO_CIPHER_AES)

/*-----------------------------------------------------------------------------
 * val_SymKeyWrap
 */
/**
 * This function wraps the plaintext (key) data according to the method that
 * is specified in SP800-38F, RFC3394 or RFC5649 based on AES algorithm.
 *
 * @param [in] KeyAssetId
 *     If applicable, the AssetId of the key to use otherwise
 *     VAL_ASSETID_INVALID to indicate that a plaintext key is used
 *     specified with Key_p.
 *
 * @param [in] Key_p
 *     If applicable, the pointer to the plaintext key buffer otherwise NULL.
 *
 * @param [in] KeySize
 *     The size of the key (KeyLength).
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the key data that needs to be wrapped.
 *
 * @param [in] SrcDataSize
 *     The size of data to be wrapped.
 *
 * @param [out] DstData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in,out] DstDataSize_p
 *     Pointer to the size of data buffer, upon return it is updated with
 *     the actual size of the wrapped data.
 *
 * @param [in] Algorithm
 *     Algorithm field, use defines
 *     - VAL_SYM_ALGO_CIPHER_AES
 *     - VAL_SYM_ALGO_CIPHER_SM4
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymKeyWrap(
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        uint8_t Algorithm);


/*-----------------------------------------------------------------------------
 * val_SymKeyUnwrap
 */
/**
 * This function unwraps the encrypted (key) data according to the method that
 * is specified in SP800-38F, RFC3394 or RFC5649 based on AES algorithm.
 *
 * @param [in] KeyAssetId
 *     If applicable, the AssetId of the key to use otherwise
 *     VAL_ASSETID_INVALID to indicate that a plaintext key is used
 *     specified with Key_p.
 *
 * @param [in] Key_p
 *     If applicable, the pointer to the plaintext key buffer otherwise NULL.
 *
 * @param [in] KeySize
 *     The size of the key (KeyLength).
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the key data that needs to be unwrapped.
 *
 * @param [in] SrcDataSize
 *     The size of data to be unwrapped.
 *
 * @param [out] DstData_p
 *     Pointer to the buffer in which the unwrapped data needs to be written.
 *
 * @param [in,out] DstDataSize_p
 *     Pointer to the size of data buffer, upon return it is updated with
 *     the actual size of the unwrapped data.
 *
 * @param [in] Algorithm
 *     Algorithm to use, use defines
 *     - VAL_SYM_ALGO_CIPHER_AES
 *     - VAL_SYM_ALGO_CIPHER_SM4
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymKeyUnwrap(
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        uint8_t Algorithm);


/*-----------------------------------------------------------------------------
 * val_EncryptVectorForPKI
 */
/**
 * This function encrypts the specified asset with the Key Decrypt Key (Black
 * Key) when the exportable bit of the Asset Policy is set so that is can be
 * used as an encrypted vector for the PKI.
 * Note: The function can return a length error, in that case the required
 *       buffer size will be returned.
 *
 * @param [in] AssetId
 *     The AssetId of the vector (Key, etc.) to encrypt to get an encrypted
 *     vector.
 *
 * @param [in] AssetNumber
 *     The asset number of the Key Decryption Key (Black Key) that must be
 *     used to encrypt the vector to get an encrypted vector.
 *
 * @param [out] EncVectorData_p
 *     Pointer to the buffer in which the encrypted vector data needs to be
 *     written.
 *
 * @param [in,out] EncVectorDataSize_p
 *     Pointer to the size of encrypted vector data buffer, upon return it is
 *     updated with the actual or the required size of the encrypted vector
 *     data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_EncryptVectorForPKI(
        const ValAssetId_t AssetId,
        const ValAssetNumber_t AssetNumber,
        ValOctetsOut_t * const EncVectorData_p,
        ValSize_t * const EncVectorDataSize_p);


#endif /* INCLUDE_GUARD_API_VAL_SYMKEYWRAP_H */

/* end of file api_val_symkeywrap.h */
