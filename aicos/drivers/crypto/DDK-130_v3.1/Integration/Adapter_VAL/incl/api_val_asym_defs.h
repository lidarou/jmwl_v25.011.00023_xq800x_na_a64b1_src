/**
 * @file api_val_asym_defs.h
 *
 * @brief VAL API - Asymmetric Cryptography services definitions
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

#ifndef INCLUDE_GUARD_API_VAL_ASYM_DEFS_H
#define INCLUDE_GUARD_API_VAL_ASYM_DEFS_H

#include "api_val_sym_defs.h"           // ValSymAlgo_t
#include "api_val_buffers.h"            // ValSize_t

/** General size conversion defines based on a bits size: */
/** - Converts a size in bits to a size in bytes */
#define VAL_ASYM_DATA_SIZE_B2B(x)   ((x+7)/8)
/** - Converts a size in bits to a size in bytes aligned on a word boundary */
#define VAL_ASYM_DATA_SIZE_B2WB(x)  (4*((x+31)/32))
/** - Converts a size in bits to a size in words */
#define VAL_ASYM_DATA_SIZE_B2W(x)   ((x+31)/32)

/** Various asymmetric crypto related limits/size defines: */
/** for ECP operations for the minimum allowed modulus length in bits */
#define VAL_ASYM_ECP_MIN_BITS          192

/** for ECP operations for the maximum allowed modulus length in bits */
#define VAL_ASYM_ECP_MAX_BITS          521

/** for RSA operations for the minimum allowed modulus length in bits */
#define VAL_ASYM_RSA_MIN_BITS          1024

/** for RSA operations for the maximum allowed modulus length in bits */
#define VAL_ASYM_RSA_MAX_BITS          3072

/** for RSA operations for the minimum allowed exponent length in bits */
#define VAL_ASYM_RSA_EXPO_MIN_BITS     17

/** for RSA operations for the maximum allowed exponent length in bits */
#define VAL_ASYM_RSA_EXPO_MAX_BITS     255

/** for DH and DSA operations for the minimum allowed prime (modulus) length
 *  in bits */
#define VAL_ASYM_DH_DSA_MIN_BITS       1024

/** for DH and DSA operations for the maximum allowed prime (modulus) length
 *  in bits */
#define VAL_ASYM_DH_DSA_MAX_BITS       3072

/** for DH and DSA operations for the minimum allowed divisor (order or
 *  sub-prime) length in bits */
#define VAL_ASYM_DH_DSA_DIVISOR_MIN_BITS 160

/** for DH and DSA operations for the maximum allowed divisor (order or
 *  sub-prime) length in bits */
#define VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS 256

/** For SM2ENC wrap and unwrap operations, maximum allowed asset data size.
 *  1Kbyte - point on curve size (64) - integrity check tag (32) */
#define VAL_ASYM_SM2ENC_MAX_BYTES 928

/** for ECP operations for the maximum size of buffer in bytes */
#define VAL_ASYM_ECP_BYTES          VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_ECP_MAX_BITS)

/** for ECP operations for the maximum size of buffer in words */
#define VAL_ASYM_ECP_WORDS          VAL_ASYM_DATA_SIZE_B2W(VAL_ASYM_ECP_MAX_BITS)

/** for ECDH operations for the maximum size of buffer in bytes */
#define VAL_ASYM_ECDH_BYTES         VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_ECP_MAX_BITS)

/** for ECDH operations for the maximum size of buffer in words (same as the
 *  ECP maximum) */
#define VAL_ASYM_ECDH_WORDS         VAL_ASYM_DATA_SIZE_B2W(VAL_ASYM_ECP_MAX_BITS)

/** for RSA operations for the maximum size of buffer in bytes */
#define VAL_ASYM_RSA_BYTES          VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_RSA_MAX_BITS)

/** for RSA operations for the maximum size of buffer in words */
#define VAL_ASYM_RSA_WORDS          VAL_ASYM_DATA_SIZE_B2W(VAL_ASYM_RSA_MAX_BITS)

/** for DH and DSA operations for the maximum size of buffer in bytes */
#define VAL_ASYM_DSA_BYTES          VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_DH_DSA_MAX_BITS)

/** for DH and DSA operations for the maximum size of buffer in words */
#define VAL_ASYM_DSA_WORDS          VAL_ASYM_DATA_SIZE_B2W(VAL_ASYM_DH_DSA_MAX_BITS)

/** for DH and DSA operations for the maximum size of buffer in bytes */
#define VAL_ASYM_DSA_DIVISOR_BYTES VAL_ASYM_DATA_SIZE_B2B(VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS)

/** for DH and DSA operations for the maximum size of buffer in words */
#define VAL_ASYM_DSA_DIVISOR_WORDS VAL_ASYM_DATA_SIZE_B2W(VAL_ASYM_DH_DSA_DIVISOR_MAX_BITS)

/** counter size in bytes utilized in PSS operation */
#define VAL_ASYM_PSS_CTR_SIZE               4

/** size in bytes used for tag and padding in PKCS #1 padding operation */
#define VAL_ASYM_PKCS1_FIX_PAD              11

/** value of tag used in PKCS #1 padding when done for sign/verification */
#define VAL_ASYM_PKCS1_SIGN_VERIFY_TAG      1

/** value of tag used in PKCS #1 padding when done for encryption/decryption */
#define VAL_ASYM_PKCS1_ENCRYPT_DECRYPT_TAG  2


/** ValAsymBigInt_t - Big number structure */
typedef struct
{
    uint8_t * Data_p;                   // Data part
    uint32_t ByteDataSize;              // Data size in bytes
} ValAsymBigInt_t;

/** ValAsymECCPoint_t - Elliptic Curve Cryptography point structure  */
typedef struct
{
    ValAsymBigInt_t x;                  // Represents the x coordinate
    ValAsymBigInt_t y;                  // Represents the y coordinate
} ValAsymECCPoint_t;

/** ValAsymECDomainParam_t - Elliptic Curve domain parameters */
typedef struct
{
    ValSize_t ModulusSizeBits;          // Modulus size in bits
    ValAsymBigInt_t Modulus;            // Curve modulus (p)
    ValAsymBigInt_t a;                  // Curve constant a
    ValAsymBigInt_t b;                  // Curve constant b
    ValAsymBigInt_t Order;              // Curve order (n)
    ValAsymECCPoint_t BasePoint;        // Curve base point (G)
    uint8_t CoFactor;                   // Curve cofactor (h)
                                        // - Preferably 1, 0 == 1
} ValAsymECDomainParam_t;

/** ValAsymDHDSADomainParam_t - Diffie-Hellman or Digital Signature Algorithm
 *                              domain parameters */
typedef struct
{
    ValSize_t PrimeSizeBits;            // Prime (modulus) size in bits
    ValSize_t DivisorSizeBits;          // Divisor (order or sub-prime) size in bits
    ValAsymBigInt_t Prime;              // Prime p
    ValAsymBigInt_t Divisor;            // Divisor (order or sub-prime) q
    ValAsymBigInt_t Base;               // Base g
} ValAsymDHDSADomainParam_t;

/** ValAsymRSADomainParam_t - RSA domain parameters for private key */
typedef struct
{
    ValAsymBigInt_t Modulus;            // Modulus m
    ValAsymBigInt_t primeP;             // Prime P
    ValAsymBigInt_t primeQ;             // Prime Q
    ValAsymBigInt_t dmodP;              // d % (P-1)
    ValAsymBigInt_t dmodQ;              // d % (Q-1)
    ValAsymBigInt_t cofQinv;            // cofQinv * Q == 1 (mod p)
} ValAsymRSADomainParam_t;


/** ValAsymKey_t - Asymmetric crypto key structure used for all public-key
 *                 algorithms to pass the public and private key part in the
 *                 specified algorithm. */
typedef struct
{
    ValAssetId_t KeyAssetId;            // AssetId of the private/public key
    ValAssetId_t DomainAssetId;         // AssetId of the domain parameters

    ValSize_t ModulusSizeBits;          // (Prime) Modulus size in bits
    ValSymAlgo_t HashAlgorithm;         // Hash Algorithm to use
} ValAsymKey_t;


/** ValAsymSign_t - Digital Signature  */
typedef struct
{
    ValAsymBigInt_t r;                  // Signature component r
    ValAsymBigInt_t s;                  // Signature component s
} ValAsymSign_t;


/** ValAsymPkaOperation_t - PKA operations without Asset use */
typedef enum
{
    VAL_ASYM_PKA_OP_MODEXPE = 0,
    VAL_ASYM_PKA_OP_MODEXPD,
    VAL_ASYM_PKA_OP_MODEXPCRT,
    VAL_ASYM_PKA_OP_ECMONTMUL,
    VAL_ASYM_PKA_OP_ECCMUL,
    VAL_ASYM_PKA_OP_ECCADD,
    VAL_ASYM_PKA_OP_DSASIGN,
    VAL_ASYM_PKA_OP_DSAVERIFY,
    VAL_ASYM_PKA_OP_ECDSASIGN,
    VAL_ASYM_PKA_OP_ECDSAVERIFY
} ValAsymPkaOperation_t;

#endif /* INCLUDE_GUARD_API_VAL_ASYM_DEFS_H */

/* end of file api_val_asym_defs.h */
