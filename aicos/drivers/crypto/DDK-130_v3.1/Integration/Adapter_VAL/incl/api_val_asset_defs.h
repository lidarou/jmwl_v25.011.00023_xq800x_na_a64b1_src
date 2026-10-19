/**
 * @file api_val_asset_defs.h
 *
 * @brief VAL API - Asset Management services definitions
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

#ifndef INCLUDE_GUARD_API_VAL_ASSET_DEFS_H
#define INCLUDE_GUARD_API_VAL_ASSET_DEFS_H

#include "basic_defs.h"             // uint8_t, uint16_t, uint32_t, uint64_t
#include "cs_eip130.h"              // EIP-130 configuration

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifdef EIP130_FW_ASSET_LIFETIME_MANAGEMENT
#define VAL_ASSET_LIFETIME_MANAGEMENT   /* Asset lifetime functionality */
#endif

/** Asset Identifier, refers to an Asset in the Asset Store.\n
 *  The Asset Identifier (AssetId) value is implementation specific and its
 *  value should not be used directly, except for comparing against
 *  VAL_ASSETID_INVALID. */
typedef uint32_t ValAssetId_t;
/** Optional Asset Identifier. Refers to an Asset in the Asset Store or has
 *  the value VAL_ASSETID_INVALID. */
typedef ValAssetId_t ValAssetId_Optional_t;

/** Guaranteed unused AssetId value */
#define VAL_ASSETID_INVALID  0


/** Asset Number, refers to a non-volatile Asset number.\n
 *  The AssetNumber is system specific and is used for identifying a Static
 *  Asset or (Static) Public Data object. */
typedef uint16_t ValAssetNumber_t;


/** The maximum value for the AssetSize parameter used in this API.\n
 *  Note: DH/DSA Domain parameters are taken as maximum reference */
#define VAL_ASSET_SIZE_MAX   (((3 * 32) + 3072 + 3072 + 256) / 8)


/** The maximum value for the AssetNumber parameter used in this API. */
#ifdef EIP130_FW_ASSETPOLICY_V2
#define VAL_ASSET_NUMBER_MAX        126
#else
#define VAL_ASSET_NUMBER_MAX        30
#endif
/** The maximum number of FW constants that needs to be added to the
 *  VAL_ASSET_NUMBER_MAX to get the actual maximum AssetNumber value. */
#ifdef EIP130_FW_ASSETPOLICY_V2
#define VAL_ASSET_NUMBER_CONSTANTS  3
#else
#define VAL_ASSET_NUMBER_CONSTANTS  10
#endif

/** The maximum value for the PolicyNumber parameter used in this API. */
#define VAL_POLICY_NUMBER_MAX   30


/** The minimum length of the label used in the Key Derivation Function (KDF). */
#define VAL_KDF_LABEL_MIN_SIZE       53

/** The maximum length of the label used in the Key Derivation Function (KDF). */
#define VAL_KDF_LABEL_MAX_SIZE       (224 - 20)


/** The minimum length of the label used in the AES-SIV keyblob export/import. */
#define VAL_KEYBLOB_AAD_MIN_SIZE     33

/** The maximum length of the label used in the AES-SIV keyblob export/import. */
#define VAL_KEYBLOB_AAD_MAX_SIZE     224


/** The expected size of an AES-SIV keyblob.\n
 *  Note: AssetSize is the size of the Asset in octects (bytes). */
#ifdef VAL_ASSET_LIFETIME_MANAGEMENT
#define VAL_KEYBLOB_SIZE(AssetSize) ((128/8) + AssetSize + (32/8))
#else
#define VAL_KEYBLOB_SIZE(AssetSize) ((128/8) + AssetSize)
#endif
#define VAL_OTP_KEYBLOB_SIZE(AssetSize) ((128/8) + AssetSize)

typedef enum
{
    VAL_ASSET_LIFETIME_INFINITE = 0,        /* Not used */
#ifdef VAL_ASSET_LIFETIME_MANAGEMENT
    VAL_ASSET_LIFETIME_SECONDS,
    VAL_ASSET_LIFETIME_MILLISECONDS,
    VAL_ASSET_LIFETIME_COUNTER,
#endif
} ValAssetLifetimeUse_t;

/** Asset Policy is a bitmask that defines the type and use of an Asset.\n
 *  The VAL_POLICY_* defines should be used to construct the Asset Policy
 *  bitmask.\n
 *  Note: The Asset Policy limitations depend on the implementation. So, please
 *        check the documentation.\n
 *  Note 2: As of HW V3 / AssetPolicy V2, the VAL_POLICY_* defines
 *         are deprecrated and only exist for backwards compatibility.\n
 *         Please use the EIP130_ASSET_POLICY_* defines
 *         from the DriverLibrary (eip130_asset_policy.h)\n
 * */
typedef uint64_t ValPolicyMask_t;


#ifdef EIP130_FW_ASSETPOLICY_V2
/* for asset policy V2 */
#define VAL_POLICY_SOURCE_NON_SECURE          EIP130_ASSET_POLICY_SOURCENONSECURE
#define VAL_POLICY_CROSS_DOMAIN               EIP130_ASSET_POLICY_CROSSDOMAIN
#define VAL_POLICY_EXPORT                     EIP130_ASSET_POLICY_EXPORTABLE

/* Asset numbers for special Static Assets */
/* - OTP */
#define VAL_ASSETNUMBER_NOTUSED     0x0       /* Not used/invalid number */
#define VAL_ASSETNUMBER_COID        0x60      /* Crypto Officer Identifer */
#define VAL_ASSETNUMBER_HUK         0x61      /* Hardware Unique Key */
#define VAL_ASSETNUMBER_SFWBPKD     0x62      /* Secure FirmWare Boot Public Key Digest */
#define VAL_ASSETNUMBER_SFWBCR      0x63      /* Secure FirmWare Boot Confidentiality Root */
#define VAL_ASSETNUMBER_SFWBVER     0x64      /* Secure FirmWare Boot Version (rollback value) */
#define VAL_ASSETNUMBER_ECI_SDKey   0x68      /* ECI Secure Debug ECDSA256 Public key */
#define VAL_ASSETNUMBER_Invalid     0x7F      /* Invalid number */

/* - Firmware (constants) */
#define VAL_ASSETNUMBER_AUTHKEY1    0x81      /*# Secure Debug Authentication key 1 */
#define VAL_ASSETNUMBER_AUTHKEY2    0x82      /*# Secure Debug Authentication key 2 */
#define VAL_ASSETNUMBER_AUTHKEY3    0x83      /*# Secure Debug Authentication key 3 */

#else
/** Asset policies related to hash/HMAC and CMAC algorithms */
#define VAL_POLICY_SHA1                       EIP130_ASSET_POLICY_SHA1
#define VAL_POLICY_SHA224                     EIP130_ASSET_POLICY_SHA224
#define VAL_POLICY_SHA256                     EIP130_ASSET_POLICY_SHA256
#define VAL_POLICY_SHA384                     EIP130_ASSET_POLICY_SHA384
#define VAL_POLICY_SHA512                     EIP130_ASSET_POLICY_SHA512
#define VAL_POLICY_CMAC                       EIP130_ASSET_POLICY_CMAC
#define VAL_POLICY_POLY1305                   EIP130_ASSET_POLICY_POLY1305
#define VAL_POLICY_SM3                        EIP130_ASSET_POLICY_SM3

/** Asset policies related to symmetric cipher algorithms */
#define VAL_POLICY_ALGO_CIPHER_MASK           EIP130_ASSET_POLICY_ALGO_CIPHER_MASK
#define VAL_POLICY_ALGO_CIPHER_AES            EIP130_ASSET_POLICY_ALGO_CIPHER_AES
#define VAL_POLICY_ALGO_CIPHER_TRIPLE_DES     EIP130_ASSET_POLICY_ALGO_CIPHER_TRIPLE_DES
#define VAL_POLICY_ALGO_CIPHER_CHACHA20       EIP130_ASSET_POLICY_ALGO_CHACHA20
#define VAL_POLICY_ALGO_CIPHER_SM4            EIP130_ASSET_POLICY_ALGO_SM4
#define VAL_POLICY_ALGO_CIPHER_ARIA           EIP130_ASSET_POLICY_ALGO_ARIA

/** Asset policies related to symmetric cipher modes */
#define VAL_POLICY_MODE1                      EIP130_ASSET_POLICY_MODE1
#define VAL_POLICY_MODE2                      EIP130_ASSET_POLICY_MODE2
#define VAL_POLICY_MODE3                      EIP130_ASSET_POLICY_MODE3
#define VAL_POLICY_MODE4                      EIP130_ASSET_POLICY_MODE4
#define VAL_POLICY_MODE5                      EIP130_ASSET_POLICY_MODE5
#define VAL_POLICY_MODE6                      EIP130_ASSET_POLICY_MODE6
#define VAL_POLICY_MODE7                      EIP130_ASSET_POLICY_MODE7
#define VAL_POLICY_MODE8                      EIP130_ASSET_POLICY_MODE8
#define VAL_POLICY_MODE9                      EIP130_ASSET_POLICY_MODE9
#define VAL_POLICY_MODE10                     EIP130_ASSET_POLICY_MODE10
/* leave these, these are convenient */
/** Asset policies specialized per symmetric cipher algorithm */
#define VAL_POLICY_AES_MODE_ECB               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE1)
#define VAL_POLICY_AES_MODE_CBC               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE2)
#define VAL_POLICY_AES_MODE_CTR               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE4)
#define VAL_POLICY_AES_MODE_CTR32             (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE4)
#define VAL_POLICY_AES_MODE_ICM               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE5)
#define VAL_POLICY_AES_MODE_CCM               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE7 | VAL_POLICY_CMAC)
#define VAL_POLICY_AES_MODE_F8                (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE8)
#define VAL_POLICY_AES_MODE_XTS               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE9)
#define VAL_POLICY_AES_MODE_GCM               (VAL_POLICY_ALGO_CIPHER_AES | VAL_POLICY_MODE10)

#define VAL_POLICY_3DES_MODE_ECB              (VAL_POLICY_ALGO_CIPHER_TRIPLE_DES | VAL_POLICY_MODE1)
#define VAL_POLICY_3DES_MODE_CBC              (VAL_POLICY_ALGO_CIPHER_TRIPLE_DES | VAL_POLICY_MODE2)

#define VAL_POLICY_CHACHA20_ENCRYPT           (VAL_POLICY_ALGO_CIPHER_CHACHA20)
#define VAL_POLICY_CHACHA20_AEAD              (VAL_POLICY_ALGO_CIPHER_CHACHA20 | VAL_POLICY_POLY1305)

#define VAL_POLICY_SM4_MODE_ECB               (VAL_POLICY_ALGO_CIPHER_SM4 | VAL_POLICY_MODE1)
#define VAL_POLICY_SM4_MODE_CBC               (VAL_POLICY_ALGO_CIPHER_SM4 | VAL_POLICY_MODE2)
#define VAL_POLICY_SM4_MODE_CTR               (VAL_POLICY_ALGO_CIPHER_SM4 | VAL_POLICY_MODE4)

#define VAL_POLICY_ARIA_MODE_ECB              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE1)
#define VAL_POLICY_ARIA_MODE_CBC              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE2)
#define VAL_POLICY_ARIA_MODE_CTR              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE4)
#define VAL_POLICY_ARIA_MODE_CTR32            (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE4)
#define VAL_POLICY_ARIA_MODE_ICM              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE5)
#define VAL_POLICY_ARIA_MODE_CCM              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE7 | VAL_POLICY_CMAC)
#define VAL_POLICY_ARIA_MODE_GCM              (VAL_POLICY_ALGO_CIPHER_ARIA | VAL_POLICY_MODE10)

/** Asset policies related to Algorithm/cipher/MAC operations */
#define VAL_POLICY_MAC_GENERATE               EIP130_ASSET_POLICY_MAC_GENERATE
#define VAL_POLICY_MAC_VERIFY                 EIP130_ASSET_POLICY_MAC_VERIFY
#define VAL_POLICY_ENCRYPT                    EIP130_ASSET_POLICY_ENCRYPT
#define VAL_POLICY_DECRYPT                    EIP130_ASSET_POLICY_DECRYPT

/** Asset policy related to Export via Coprocessor operation */
#define VAL_POLICY_COPROCESSOR                0x0000000040000000ULL

/** Asset policies related to temporary values
 *  Note that the VAL_POLICY_TEMP_MAC should be used for intermediate
 *  hash digest as well. */
#define VAL_POLICY_TEMP_IV                    EIP130_ASSET_POLICY_TEMP_IV
#define VAL_POLICY_TEMP_COUNTER               EIP130_ASSET_POLICY_TEMP_COUNTER
#define VAL_POLICY_TEMP_MAC                   EIP130_ASSET_POLICY_TEMP_MAC
#define VAL_POLICY_TEMP_AUTH_STATE            EIP130_ASSET_POLICY_TEMP_AUTH_STATE

/** Asset policy related to monotonic counter */
#define VAL_POLICY_MONOTONIC                  EIP130_ASSET_POLICY_MONOTONIC

/** Asset policies related to key derive functionality */
#define VAL_POLICY_TRUSTED_ROOT_KEY           EIP130_ASSET_POLICY_TRUSTED_ROOT_KEY
#define VAL_POLICY_TRUSTED_KEY_DERIVE         EIP130_ASSET_POLICY_TRUSTED_KEY_DERIVE
#define VAL_POLICY_KEY_DERIVE                 EIP130_ASSET_POLICY_KEY_DERIVE

/** Asset policies related to AES key wrap functionality\n
 *  Note: Must be combined with operations bits */
#define VAL_POLICY_TRUSTED_WRAP               EIP130_ASSET_POLICY_TRUSTED_WRAP
#define VAL_POLICY_AES_WRAP                   EIP130_ASSET_POLICY_AES_WRAP

/** Asset policies related to PK operations */
#define VAL_POLICY_PUBLIC_KEY                 EIP130_ASSET_POLICY_PUBLIC_KEY
#define VAL_POLICY_PK_RSA_OAEP_WRAP           EIP130_ASSET_POLICY_PK_RSA_OAEP_WRAP
#define VAL_POLICY_PK_RSA_PKCS1_WRAP          EIP130_ASSET_POLICY_PK_RSA_PKCS1_WRAP
#define VAL_POLICY_PK_RSA_PKCS1_SIGN          EIP130_ASSET_POLICY_PK_RSA_PKCS1_SIGN
#define VAL_POLICY_PK_RSA_PSS_SIGN            EIP130_ASSET_POLICY_PK_RSA_PSS_SIGN
#define VAL_POLICY_PK_DSA_SIGN                EIP130_ASSET_POLICY_PK_DSA_SIGN
#define VAL_POLICY_PK_ECC_ECDSA_SIGN          EIP130_ASSET_POLICY_PK_ECC_ECDSA_SIGN
#define VAL_POLICY_PK_SM2_SIGN                EIP130_ASSET_POLICY_PK_SM2_SIGN
#define VAL_POLICY_PK_DH_KEY                  EIP130_ASSET_POLICY_PK_DH_KEY
#define VAL_POLICY_PK_ECDH_KEY                EIP130_ASSET_POLICY_PK_ECDH_KEY
#define VAL_POLICY_PK_SM2_KEY                 EIP130_ASSET_POLICY_PK_SM2_KEYEXCH
#define VAL_POLICY_PK_SM2_WRAP                EIP130_ASSET_POLICY_PK_SM2_WRAP
#define VAL_POLICY_PUBLIC_KEY_PARAM           EIP130_ASSET_POLICY_PUBLIC_KEY_PARAM

#define VAL_POLICY_PK_ECC_ELGAMAL_ENC         (VAL_POLICY_PK_ECC_ECDSA_SIGN | VAL_POLICY_PK_ECDH_KEY)

/** Asset policies related to Authentication */
#define VAL_POLICY_EMMC_AUTH_KEY              EIP130_ASSET_POLICY_EMMC_AUTH_KEY
#define VAL_POLICY_AUTH_KEY                   EIP130_ASSET_POLICY_AUTH_KEY

/** Asset policies related to the domain */
#define VAL_POLICY_SOURCE_NON_SECURE          EIP130_ASSET_POLICY_SOURCE_NON_SECURE
#define VAL_POLICY_CROSS_DOMAIN               EIP130_ASSET_POLICY_CROSS_DOMAIN

/** Asset policies related to general purpose data that can or must be used
 *  in an operation */
#define VAL_POLICY_PRIVATE_DATA               EIP130_ASSET_POLICY_PRIVATE_DATA
#define VAL_POLICY_PUBLIC_DATA                EIP130_ASSET_POLICY_PUBLIC_DATA

/** Asset policies related to export functionality */
#define VAL_POLICY_EXPORT                     EIP130_ASSET_POLICY_EXPORT
#define VAL_POLICY_TRUSTED_EXPORT             EIP130_ASSET_POLICY_TRUSTED_EXPORT

#endif /* EIP130_FW_ASSETPOLICY_V2 */

#endif /* INCLUDE_GUARD_API_VAL_ASSET_DEFS_H */

/* end of file api_val_asset_defs.h */
