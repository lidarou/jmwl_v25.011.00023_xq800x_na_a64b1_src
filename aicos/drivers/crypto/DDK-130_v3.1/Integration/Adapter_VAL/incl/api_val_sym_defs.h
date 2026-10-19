/**
 * @file api_val_sym_defs.h
 *
 * @brief VAL API - SymCrypto definintions
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

#ifndef INCLUDE_GUARD_API_VAL_SYM_DEFS_H
#define INCLUDE_GUARD_API_VAL_SYM_DEFS_H

/**
 * Special symmetric crypto related defines:
 */
/** SHA1/SHA224/SHA256/SM3 Blocksize */
#define VAL_SYM_ALGO_HASH_BLOCK_SIZE            (512/8)
/** SHA384/SHA512 Blocksize */
#define VAL_SYM_ALGO_HASH_SHA512_BLOCK_SIZE     (1024/8)
/** SHA3 related Blocksize */
#define VAL_SYM_ALGO_HASH_SHA3_224_BLOCK_SIZE   (1152/8)    // 144 bytes
#define VAL_SYM_ALGO_HASH_SHA3_256_BLOCK_SIZE   (1088/8)    // 136 bytes
#define VAL_SYM_ALGO_HASH_SHA3_384_BLOCK_SIZE   (832/8)     // 104 bytes
#define VAL_SYM_ALGO_HASH_SHA3_512_BLOCK_SIZE   (576/8)     // 72 bytes

/** AES Blocksize */
#define VAL_SYM_ALGO_AES_BLOCK_SIZE          (128/8)
/** DES Blocksize */
#define VAL_SYM_ALGO_DES_BLOCK_SIZE          (64/8)
/** ChaCha20 Blocksize */
#define VAL_SYM_ALGO_CHACHA20_BLOCK_SIZE     (512/8)
/** SM4 Blocksize */
#define VAL_SYM_ALGO_SM4_BLOCK_SIZE          (128/8)
/** ARIA Blocksize */
#define VAL_SYM_ALGO_ARIA_BLOCK_SIZE         (128/8)
/** Poly1305 Blocksize */
#define VAL_SYM_ALGO_POLY1305_BLOCK_SIZE     (128/8)

/** AES IV size */
#define VAL_SYM_ALGO_AES_IV_SIZE             (128/8)
/** DES IV size */
#define VAL_SYM_ALGO_DES_IV_SIZE             (64/8)
/** ChaCha20 IV size */
#define VAL_SYM_ALGO_CHACHA20_IV_SIZE        (128/8)
/** SM4 IV size */
#define VAL_SYM_ALGO_SM4_IV_SIZE             (128/8)
/** ARIA IV size */
#define VAL_SYM_ALGO_ARIA_IV_SIZE            (128/8)
/** Maximum IV size used in the interface */
#define VAL_SYM_ALGO_MAX_IV_SIZE             VAL_SYM_ALGO_AES_IV_SIZE

/** System parameter size */
#define VAL_SYM_ALGO_PARAMETER_SIZE          (128/8)

/** AES Tag size */
#define VAL_SYM_ALGO_AES_TAG_SIZE            (128/8)
/** ChaCha20 Tag size */
#define VAL_SYM_ALGO_CHACHA20_TAG_SIZE       (128/8)
/** Maximum Tag size used in the interface */
#define VAL_SYM_ALGO_MAX_TAG_SIZE            VAL_SYM_ALGO_AES_TAG_SIZE

/** Maximum block cipher key size */
#define VAL_SYM_ALGO_MAX_CIPHER_KEY_SIZE     (256/8)
/** Maximum SHA1/SHA224/SHA256 key size */
#define VAL_SYM_ALGO_MAX_SHA2_MAC_KEY_SIZE   (512/8)
/** Maximum SHA384/SHA512 key size */
#define VAL_SYM_ALGO_MAX_SHA512_MAC_KEY_SIZE (1024/8)

/** Maximum MAC key size used in the interface */
#define VAL_SYM_ALGO_MAX_MAC_KEY_SIZE        VAL_SYM_ALGO_MAX_SHA512_MAC_KEY_SIZE
/** Maximum digest/MAC size used in the interface */
#define VAL_SYM_ALGO_MAX_DIGEST_SIZE         (512/8)


/** ValSymContextPtr_t - Pointer to the Symmetric Crypto context */
typedef void * ValSymContextPtr_t;
/** ValSymContextPtr_Optional_t - Pointer to the optional Symmetric Crypto
 *  context */
typedef ValSymContextPtr_t ValSymContextPtr_Optional_t;

/** ValSymAlgo_t - Enumeration for symmetric crypto algorithms */
typedef enum
{
    VAL_SYM_ALGO_NONE = 0,              // Not Used/Initialized

    // Hash algorithms
    VAL_SYM_ALGO_HASH = 10,             // Begin marker
    VAL_SYM_ALGO_HASH_SHA1,             // SHA-1
    VAL_SYM_ALGO_HASH_SHA224,           // SHA-224
    VAL_SYM_ALGO_HASH_SHA256,           // SHA-256
    VAL_SYM_ALGO_HASH_SHA384,           // SHA-384
    VAL_SYM_ALGO_HASH_SHA512,           // SHA-512
    VAL_SYM_ALGO_HASH_SHA3_224,         // SHA3-224
    VAL_SYM_ALGO_HASH_SHA3_256,         // SHA3-256
    VAL_SYM_ALGO_HASH_SHA3_384,         // SHA3-384
    VAL_SYM_ALGO_HASH_SHA3_512,         // SHA3-512
    VAL_SYM_ALGO_HASH_SM3,              // SM3
    VAL_SYM_ALGO_HASH_MAX,              // must be last

    // MAC algorithms
    VAL_SYM_ALGO_MAC = 20,              // Begin marker
    VAL_SYM_ALGO_MAC_HMAC_SHA1,         // HMAC-SHA-1
    VAL_SYM_ALGO_MAC_HMAC_SHA224,       // HMAC-SHA-224
    VAL_SYM_ALGO_MAC_HMAC_SHA256,       // HMAC-SHA-256
    VAL_SYM_ALGO_MAC_HMAC_SHA384,       // HMAC-SHA-384
    VAL_SYM_ALGO_MAC_HMAC_SHA512,       // HMAC-SHA-512
    VAL_SYM_ALGO_MAC_HMAC_SHA3_224,     // HMAC-SHA-3-224
    VAL_SYM_ALGO_MAC_HMAC_SHA3_256,     // HMAC-SHA-3-256
    VAL_SYM_ALGO_MAC_HMAC_SHA3_384,     // HMAC-SHA-3-384
    VAL_SYM_ALGO_MAC_HMAC_SHA3_512,     // HMAC-SHA-3-512
    VAL_SYM_ALGO_MAC_HMAC_SM3,          // HMAC-SM3
    VAL_SYM_ALGO_MAC_AES_CMAC,          // AES-CMAC
    VAL_SYM_ALGO_MAC_AES_CBC_MAC,       // AES-CBC-MAC
    VAL_SYM_ALGO_MAC_ARIA_CMAC,         // ARIA-CMAC
    VAL_SYM_ALGO_MAC_ARIA_CBC_MAC,      // ARIA-CBC-MAC
    VAL_SYM_ALGO_MAC_POLY1305,          // Poly1305
    VAL_SYM_ALGO_MAC_MAX,               // must be last

    // (Block)Cipher algorithms
    VAL_SYM_ALGO_CIPHER = 40,           // Begin marker
    VAL_SYM_ALGO_CIPHER_AES,            // AES
    VAL_SYM_ALGO_CIPHER_DES,            // DES
    VAL_SYM_ALGO_CIPHER_TRIPLE_DES,     // 3DES
    VAL_SYM_ALGO_CIPHER_CHACHA20,       // ChaCha20
    VAL_SYM_ALGO_CIPHER_SM4,            // SM4
    VAL_SYM_ALGO_CIPHER_ARIA,           // ARIA
    VAL_SYM_ALGO_CIPHER_MAX,            // must be last
} ValSymAlgo_t;


/** ValSymMode_t - Enumeration for symmetric crypto modes */
typedef enum
{
    VAL_SYM_MODE_NONE = 0,              // Not Used/Initialized

    // (Block)Cipher modes
    VAL_SYM_MODE_CIPHER = 30,           // Begin marker
    VAL_SYM_MODE_CIPHER_ECB,            // ECB
    VAL_SYM_MODE_CIPHER_CBC,            // CBC
    VAL_SYM_MODE_CIPHER_CTR,            // CTR
    VAL_SYM_MODE_CIPHER_ICM,            // ICM
    VAL_SYM_MODE_CIPHER_F8,             // F8
    VAL_SYM_MODE_CIPHER_CCM,            // CCM
    VAL_SYM_MODE_CIPHER_XTS,            // XTS
    VAL_SYM_MODE_CIPHER_GCM,            // GCM
    VAL_SYM_MODE_CIPHER_CHACHA20_ENC,   // ChaCha20 Encrypt
    VAL_SYM_MODE_CIPHER_CHACHA20_AEAD,  // ChaCha20 AEAD
    VAL_SYM_MODE_CIPHER_MAX,            // must be last
} ValSymMode_t;

/** ValSymGCMMode_t - Enumeration for AES-GCM modes */
typedef enum
{
    VAL_SYM_MODE_GCM_NONE = 0,          // Not Used/Initialized

    // GCM modes
    VAL_SYM_MODE_GCM_GHASH = 40,        // GHASH operation with H pre-calculated and Y0-encrypted forced to 0 (IV Truncation operation)
    VAL_SYM_MODE_GCM_H_PRE_Y0_0,        // GCM operation with H pre-calculated and Y0-encrypted forced to 0
    VAL_SYM_MODE_GCM_H_PRE_Y0_CALC,     // GCM operation with pre-calculated H and Y0-encrypted calculated internally
    VAL_SYM_MODE_GCM_AUTO,              // Autonomous GCM operation (both H and Y0-encrypted calculated internally)
    VAL_SYM_MODE_GCM_MAX,               // must be last
} ValSymGCMMode_t;


#endif /* INCLUDE_GUARD_API_VAL_SYM_DEFS_H */

/* end of file api_val_sym.h */
