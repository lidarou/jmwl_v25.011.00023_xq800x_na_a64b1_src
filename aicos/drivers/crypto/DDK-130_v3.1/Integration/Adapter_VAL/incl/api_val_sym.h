/**
 * @file api_val_sym.h
 *
 * @brief VAL API - Symmetric Cryptography related services
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

#ifndef INCLUDE_GUARD_API_VAL_SYM_H
#define INCLUDE_GUARD_API_VAL_SYM_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"             // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"         // ValStatus_t
#include "api_val_buffers.h"        // ValOctetsIn_t, ValOctetsOut_t
#include "api_val_asset_defs.h"     // ValAssetId_t
#include "api_val_sym_defs.h"       // ValSymAlgo_t, ValSymMode_t, ValSymGCMMode_t

/*-----------------------------------------------------------------------------
 * val_SymAlloc
 */
/**
 * This function allocates and initializes a symmetric crypto context.
 * Note that decrypt is the default direction for a cipher operation.
 *
 * @param [in] Algorithm
 *     The symmetric crypto algorithm of operation.
 *
 * @param [in] Mode
 *     Applicable symmetric crypto mode of operation.
 *
 * @param [in] fUseTokenTemp
 *     Indication if possible intermediate data (eg. digest, IV) must be
 *     handled via the token. If 'false' the intermediate data is handled
 *     via the Asset Store.
 *
 * @param [out] Context_pp
 *     Pointer to the symmetric crypto context pointer in which the allocated
 *     and initialized symmetric crypto context is returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymAlloc(
        ValSymAlgo_t Algorithm,
        ValSymMode_t Mode,
        bool fUseTokenTemp,
        ValSymContextPtr_t * Context_pp);


/*-----------------------------------------------------------------------------
 * val_SymRelease
 */
/**
 * This function resets the symmetric crypto operation and releases the
 * symmetric context.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymRelease(
        ValSymContextPtr_t const Context_p);


/*-----------------------------------------------------------------------------
 * val_SymInitKey
 */
/**
 * This function initializes the key in the symmetric crypto context.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
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
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymInitKey(
        ValSymContextPtr_t const Context_p,
        ValAssetId_Optional_t KeyAssetId,
        ValOctetsIn_Optional_t * const Key_p,
        const ValSize_t KeySize);


/*-----------------------------------------------------------------------------
 * val_SymInitIV
 */
/**
 * This function initializes the IV in the symmetric crypto context.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] IV_p
 *     The pointer to the IV buffer.
 *
 * @param [in] IVSize
 *     The size of the IV.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymInitIV(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const IV_p,
        const ValSize_t IVSize);


/*-----------------------------------------------------------------------------
 * val_SymReadTokenTemp
 */
/**
 * This function reads/returns the intermediate Digest, MAC, IV, etc..
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [out] Temp_p
 *     Pointer to the buffer in which the intermediate data needs to be written.
 *
 * @param [in,out] TempSize_p
 *     Pointer to the size of data buffer, upon return it is updated with the
 *     actual size of the intermediate data.
 *
 * @param [out] TotalMessageLength_p
 *     Pointer to the total message length, upon return it is written with
 *     total message length. This information is only applicable for Hash and
 *     MAC related operations.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymReadTokenTemp(
        ValSymContextPtr_t const Context_p,
        ValOctetsOut_t * const Temp_p,
        ValSize_t * const TempSize_p,
        ValSize_t * const TotalMessageLength_p);


/*-----------------------------------------------------------------------------
 * val_SymWriteTokenTemp
 */
/**
 * This function writes/sets the intermediate Digest, MAC, IV, etc..
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] Temp_p
 *     Pointer to the buffer with the intermediate data.
 *
 * @param [in] TempSize
 *     Pointer to the size of the intermediate data.
 *
 * @param [in] TotalMessageLength
 *     The total message length, which is only applicable for Hash and MAC
 *     related operations.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymWriteTokenTemp(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Temp_p,
        const ValSize_t TempSize,
        const ValSize_t TotalMessageLength);


/*-----------------------------------------------------------------------------
 * val_SymHashUpdate
 */
/**
 * This function performs a partial hash update.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] Data_p
 *     Pointer to the buffer with the data that needs to be hashed.
 *
 * @param [in] DataSize
 *     The size of the data to be hashed.
 *     Note that the size must be a multiple of the hash block size.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymHashUpdate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize);


/*-----------------------------------------------------------------------------
 * val_SymHashFinal
 */
/**
 * This function performs the final hash update and returns the final digest.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *     Note: If the operation is successful, the symmetric crypto context
 *           is released.
 *
 * @param [in] Data_p
 *     Pointer to the buffer with the data that needs to be hashed.
 *
 * @param [in] DataSize
 *     The size of the data to be hashed.
 *
 * @param [out] Digest_p
 *     Pointer to a buffer in which the final digest must be returned.
 *     Note: The buffer needs to be big enough to hold the final digest.
 *
 * @param [in,out] DigestSize_p
 *     Pointer to the size of digest buffer, upon return it is updated with
 *     the actual size of the final digest.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymHashFinal(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsOut_t * const Digest_p,
        ValSize_t * const DigestSize_p);


/*-----------------------------------------------------------------------------
 * val_SymMacInitVerify
 */
/**
 * @brief  This function sets the direction of a MAC operation to Verify.
 *   Note: This function must be called before the actual MAC operation is
 *         performed
 *
 * @param [in] Context_p
 * @return ValStatus_t
 */

ValStatus_t
val_SymMacInitVerify(
        ValSymContextPtr_t const Context_p);


/*-----------------------------------------------------------------------------
 * val_SymMacUpdate
 */
/**
 * This function performs a partial MAC update.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] Data_p
 *     Pointer to the buffer with the data that needs to be MACed.
 *
 * @param [in] DataSize
 *     The size of data to be MACed.
 *     Note that the size must be a multiple of the block size of performed
 *     MAC operation.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymMacUpdate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize);


/*-----------------------------------------------------------------------------
 * val_SymMacGenerate
 */
/**
 * This function performs the final MAC update and returns the final MAC value.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *     Note: If the operation is successful, the symmetric crypto context
 *           is released.
 *
 * @param [in] Data_p
 *     Pointer to the buffer with the data that needs to be MACed.
 *
 * @param [in] DataSize
 *     The size of the data to be MACed.
 *
 * @param [out] MAC_p
 *     Pointer to a buffer in which the final MAC value must be returned.
 *     Note: The buffer needs to be big enough to hold the final MAC value.
 *
 * @param [in,out] MACSize_p
 *     Pointer to the size of MAC value buffer, upon return it is updated with
 *     the actual size of the final MAC value.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymMacGenerate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValOctetsOut_t * const MAC_p,
        ValSize_t * const MACSize_p);


/*-----------------------------------------------------------------------------
 * val_SymMacVerify
 */
/**
 * This function performs the final MAC update and compares the final MAC
 * value with the provided one.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *     Note: If the operation is successful, the symmetric crypto context
 *           is released.
 *
 * @param [in] Data_p
 *     Pointer to the buffer with the data that needs to be MACed.
 *
 * @param [in] DataSize
 *     The size of the data to be MACed.
 *
 * @param [in] MacAssetId
 *     Reference to the Asset that holds the final MAC value or
 *     VAL_ASSETID_INVALID if MAC_p is used to define it.
 *
 * @param [in] MAC_p
 *     Pointer to a buffer with the final MAC value that must be checked.
 *
 * @param [in] MACSize
 *     The size of the final MAC value.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymMacVerify(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Data_p,
        const ValSize_t DataSize,
        ValAssetId_Optional_t const MacAssetId,
        ValOctetsIn_Optional_t * const MAC_p,
        const ValSize_t MACSize);


/*-----------------------------------------------------------------------------
 * val_SymCipherInitEncrypt
 */
/**
 * This function sets the direction of cipher operation to Encrypt.
 * Note: This function must be called before the actual cipher operation is
 *       performed
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherInitEncrypt(
        ValSymContextPtr_t const Context_p);


/*-----------------------------------------------------------------------------
 * val_SymCipherInitNoRandomIV
 */
/**
 * This function sets the indication that the provided IV/nonce should be used.
 * Note: This function must be called before the actual cipher operation is
 *       performed
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherInitNoRandomIV(
        ValSymContextPtr_t const Context_p);


/*-----------------------------------------------------------------------------
 * val_SymCipherInitParameter
 */
/**
 * This function sets the system parameter information.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] Parameter_p
 *     Pointer to a buffer with the parameter.
 *
 * @param [in] ParameterSize
 *     The size of the parameter.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherInitParameter(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const Parameter_p,
        const ValSize_t ParameterSize);


/*-----------------------------------------------------------------------------
 * val_SymCipherInitF8
 */
/**
 * This function sets the specific AES-F8 information.
 * Note: This function must be called before the actual cipher operation is
 *       performed.
 *       This function can be disabled if the AES-F8 functionality is removed.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] f8SaltKey_p
 *     Pointer to the buffer with the SaltKey.
 *
 * @param [in] f8SaltKeySize
 *     The size of the SaltKey.
 *
 * @param [in] f8IV_p
 *     Pointer to a buffer with the IV.
 *
 * @param [in] f8IVSize
 *     The size of the IV.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherInitF8(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const f8SaltKey_p,
        const ValSize_t f8SaltKeySize,
        ValOctetsIn_t * const f8IV_p,
        const ValSize_t f8IVSize);


/*-----------------------------------------------------------------------------
 * val_SymCipherInitChaCha20
 */
/**
 * This function sets the specific ChaCha20 information.
 * Note: This function must be called before the actual cipher operation is
 *       performed.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] NonceLength
 *     The length of the Nonce.
 *     Notes:
 *     - Only Nonce length 8 and 12 are allowed.
 *     - The default Nonce length is 12.
 */
ValStatus_t
val_SymCipherInitChaCha20(
        ValSymContextPtr_t const Context_p,
        const ValSize_t NonceLength);


/*-----------------------------------------------------------------------------
 * val_SymCipherUpdate
 */
/**
 * This function performs a partial cipher update.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the data that needs to be de/encrypted.
 *
 * @param [in] SrcDataSize
 *     The size of data to be de/encrypted.
 *     Note that the size must be a multiple of the block size of performed
 *     cipher operation.
 *
 * @param [out] DstData_p
 *     Pointer to the buffer in which the de/encrypted data needs to be written.
 *
 * @param [in,out] DstDataSize_p
 *     Pointer to the size of data buffer, upon return it is updated with
 *     the actual size of the de/encrypted data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherUpdate(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p);


/*-----------------------------------------------------------------------------
 * val_SymCipherFinal
 */
/**
 * This function performs a final cipher update.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *     Note: If the operation is successful, the symmetric crypto context
 *           is released.
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the data that needs to be de/encrypted.
 *
 * @param [in] SrcDataSize
 *     The size of data to be de/encrypted.
 *     Note that the size must be a multiple of the block size of performed
 *     cipher operation.
 *
 * @param [out] DstData_p
 *     Pointer to the buffer in which the de/encrypted data needs to be written.
 *
 * @param [in,out] DstDataSize_p
 *     Pointer to the size of data buffer, upon return it is updated with
 *     the actual size of the de/encrypted data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherFinal(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p);


/*-----------------------------------------------------------------------------
 * val_SymCipherAEInit
 */
/**
 * This function initializes the Authenticated Encryption part in
 * the symmetric crypto context.
 *
 * @param [in,out] Context_p
 *     Pointer to the symmetric crypto context.
 *
 * @param [in] NonceHashKey_p
 *     The pointer to the Nonce or Hash key buffer.
 *
 * @param [in] NonceHashKeySize
 *     The size of the Nonce or Hash key.
 *
 * @param [in] TagSize
 *     The size of the Tag.
 *
 * @param [in] GCMMode
 *     The GCM mode to execute.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherAEInit(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const NonceHashKey_p,
        const ValSize_t NonceHashKeySize,
        const ValSize_t TagSize,
        const ValSymGCMMode_t GCMMode);


/*-----------------------------------------------------------------------------
 * val_SymCipherAEFinal
 */
/**
 * This function performs a final Authenticated Encryption cipher operation.
 * The direction is Encrypt if previously val_SymCipherInitEncrypt is called
 * otherwise it is Decrypt.
 *
 * @param [in] Context_p
 *     Pointer to the symmetric crypto context.
 *     Note: If the operation is successful, the symmetric crypto context
 *           is released.
 *
 * @param [in] AAD_p
 *     Pointer to the buffer with the Additional Authenticated Data.
 *
 * @param [in] AADSize
 *     The size of the Additional Authenticated Data.
 *
 * @param [in] SrcData_p
 *     Pointer to the buffer with the data that needs to be de/encrypted.
 *
 * @param [in] SrcDataSize
 *     The size of data to be de/encrypted.
 *     Note that the size must be a multiple of the block size of performed
 *     cipher operation.
 *
 * @param [out] DstData_p
 *     Pointer to the buffer in which the de/encrypted data needs to be written.
 *
 * @param [in,out] DstDataSize_p
 *     Pointer to the size of data buffer, upon return it is updated with
 *     the actual size of the de/encrypted data.
 *
 * @param [in,out] IV_p
 *     Pointer to the buffer in which the IV will be written for an Encrypt
 *     operation.
 *
 * @param [in,out] IVSize_p
 *     Pointer to the size of the IV buffer (IV_p), 16 bytes minimum.
 *     Upon return it is updated with the actual size of the IV.
 *
 * @param [in,out] Tag_p
 *     Pointer to the buffer in which the Tag is located for a Decrypt
 *     operation or where the Tag needs to be written for an Encrypt operation.
 *
 * @param [in,out] TagSize_p
 *     Pointer to the size of the Tag buffer. In, upon return it is updated with
 *     the actual size of the de/encrypted data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SymCipherAEFinal(
        ValSymContextPtr_t const Context_p,
        ValOctetsIn_t * const AAD_p,
        const ValSize_t AADSize,
        ValOctetsIn_t * const SrcData_p,
        const ValSize_t SrcDataSize,
        ValOctetsOut_t * const DstData_p,
        ValSize_t * const DstDataSize_p,
        ValOctetsOut_t * const IV_p,
        ValSize_t * const IVSize_p,
        ValOctetsInOut_t * const Tag_p,
        ValSize_t * const TagSize_p);


#endif /* INCLUDE_GUARD_API_VAL_SYM_H */

/* end of file api_val_sym.h */
