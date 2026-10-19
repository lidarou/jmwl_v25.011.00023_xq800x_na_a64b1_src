/** @file cs_adapter_val.h
 *
 * @brief Configuration Settings for the VAL API driver.
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

#ifndef INCLUDE_GUARD_CS_ADAPTER_VAL_H
#define INCLUDE_GUARD_CS_ADAPTER_VAL_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "cs_driver.h"
#include "cs_adapter.h"
#include "cs_eip130.h"


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
/** Strict argument checking use */
#ifdef ADAPTER_STRICT_ARGS
#define VAL_STRICT_ARGS
#endif

#ifndef EIP130_FW_ASSETPOLICY_V2
/** Root key (HUK) Asset Number (typically in OTP)  */
#define VAL_ROOT_KEY_INDEX    0
#endif

/** Extend the VAL API with functions that can only be used in a Secure
 *  environment and by the Crypto Officer. By default, the functions that can be
 *  used in non-secure and secure environment are enabled */
#define VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS

// - Hardware related
//#define VAL_REMOVE_HARDWARE_MODULESTATUS
//#define VAL_REMOVE_HARDWARE_EIP_OPTIONS
//#define VAL_REMOVE_HARDWARE_EIP_VERSION

/*----------------------------------------------------------------
 * Defines (switches) that can be used to remove features (functionality)
 */
// - Asset Management related
//#define VAL_REMOVE_ASSET_ALLOC
//#define VAL_REMOVE_ASSET_FREE
//#define VAL_REMOVE_ASSET_SEARCH
//#define VAL_REMOVE_GET_ROOT_KEY
//#define VAL_REMOVE_ASSET_LOAD_IMPORT
//#define VAL_REMOVE_ASSET_LOAD_DERIVE
//#define VAL_REMOVE_ASSET_LOAD_PLAINTEXT
//#define VAL_REMOVE_ASSET_LOAD_PLAINTEXT_EXPORT
//#define VAL_REMOVE_ASSET_LOAD_RANDOM
//#define VAL_REMOVE_ASSET_LOAD_RANDOM_EXPORT
//#define VAL_REMOVE_ASSET_LOAD_SYMWRAP
//#define VAL_REMOVE_SECURETIMER
//#define VAL_REMOVE_PUBLICDATAREAD
//#define VAL_REMOVE_MONOTONIC_COUNTER_READ
//#define VAL_REMOVE_MONOTONIC_COUNTER_INC

#ifndef EIP130_ENABLE_CPIF
#define VAL_REMOVE_ASSET_EXPORT_VIA_CPIF
#endif

#ifndef EIP130_ENABLE_ASSET_STORE_RESET
#define VAL_REMOVE_ASSET_STORE_RESET
#endif

#ifndef VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS
#define VAL_REMOVE_OTPDATAWRITE
#define VAL_REMOVE_PROVISIONRANDOMROOTKEY
#define VAL_REMOVE_PROVISIONRANDOMROOTKEYARIA
#else
#ifndef EIP130_ENABLE_SYM_ALGO_ARIA_DRBG
#define VAL_REMOVE_PROVISIONRANDOMROOTKEYARIA
#endif
#endif

// - Symmetric crypto related
//#define VAL_REMOVE_SYM_HASH
//#define VAL_REMOVE_SYM_MAC
#ifndef EIP130_ENABLE_SYM_ALGO_SHA512
#define VAL_REMOVE_SYM_ALGO_SHA512
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_SHA3
#define VAL_REMOVE_SYM_ALGO_SHA3
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_SM3
#define VAL_REMOVE_SYM_ALGO_SM3
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_POLY1305
#define VAL_REMOVE_SYM_ALGO_POLY1305
#endif

//#define VAL_REMOVE_SYM_CIPHER
#ifndef EIP130_ENABLE_SYM_ALGO_AES_XTS
#define VAL_REMOVE_SYM_ALGO_AES_XTS
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_AES_CCM
#define VAL_REMOVE_SYM_ALGO_AES_CCM
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_AES_GCM
#define VAL_REMOVE_SYM_ALGO_AES_GCM
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_AES_F8
#define VAL_REMOVE_SYM_ALGO_AES_F8
#endif

#ifndef EIP130_ENABLE_SYM_ALGO_DES
#define VAL_REMOVE_SYM_ALGO_DES
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_3DES
#define VAL_REMOVE_SYM_ALGO_3DES
#endif

#ifndef EIP130_ENABLE_SYM_ALGO_CHACHA20
#define VAL_REMOVE_SYM_ALGO_CHACHA20
#endif

#ifndef EIP130_ENABLE_SYM_ALGO_SM4
#define VAL_REMOVE_SYM_ALGO_SM4
#define VAL_REMOVE_SYM_SM4KEYWRAP
#define VAL_REMOVE_SYM_ALGO_SM4_GCM
#else
#ifndef EIP130_ENABLE_SYM_ALGO_SM4_GCM
#define VAL_REMOVE_SYM_ALGO_SM4_GCM
#endif
#endif

#ifndef EIP130_ENABLE_SYM_ALGO_ARIA
#define VAL_REMOVE_SYM_ALGO_ARIA
#define VAL_REMOVE_SYM_ALGO_ARIA_XTS
#define VAL_REMOVE_SYM_ALGO_ARIA_CCM
#define VAL_REMOVE_SYM_ALGO_ARIA_GCM
#define VAL_REMOVE_SYM_ALGO_ARIA_F8
#define VAL_REMOVE_TRNG_CONFIG_ARIA
#else
#define VAL_REMOVE_SYM_ALGO_ARIA_F8
#define VAL_REMOVE_SYM_ALGO_ARIA_XTS
#ifndef EIP130_ENABLE_SYM_ALGO_ARIA_CCM
#define VAL_REMOVE_SYM_ALGO_ARIA_CCM
#endif
#ifndef EIP130_ENABLE_SYM_ALGO_ARIA_GCM
#define VAL_REMOVE_SYM_ALGO_ARIA_GCM
#endif
#endif

#if !defined(EIP130_ENABLE_SYM_KEYWRAP_SM4) && !defined(EIP130_ENABLE_SYM_KEYWRAP_AES)
#define VAL_REMOVE_SYM_KEYWRAP
#endif

#ifndef EIP130_ENABLE_ENCRYPTED_VECTOR
#define VAL_REMOVE_ENCRYPTED_VECTOR
#endif

// - Asymmetric crypto related
#if !defined(EIP130_ENABLE_ASYM_ALGO_ECC_ELGAMAL) && \
    !defined(EIP130_ENABLE_ASYM_ALGO_ECDH) && \
    !defined(EIP130_ENABLE_ASYM_ALGO_ECDSA)
#define VAL_REMOVE_ASYM_ECC_GEN_KEYPAIR
#define VAL_REMOVE_ASYM_ECC_GEN_PUBLICKEY
#define VAL_REMOVE_ASYM_ECC_KEY_CHECK
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_ECC_ELGAMAL
#define VAL_REMOVE_ASYM_ECC_ELGAMAL_DECRYPT
#define VAL_REMOVE_ASYM_ECC_ELGAMAL_ENCRYPT
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_ECDH
#define VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE
#define VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_ECDSA
#define VAL_REMOVE_ASYM_ECDSA_SIGN
#define VAL_REMOVE_ASYM_ECDSA_VERIFY
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_CURVE25519
#define VAL_REMOVE_ASYM_CURVE25519
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_EDDSA
#define VAL_REMOVE_ASYM_EDDSA_SIGN
#define VAL_REMOVE_ASYM_EDDSA_VERIFY
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_DH
#define VAL_REMOVE_ASYM_DH_GEN_KEYPAIR
#define VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY
#define VAL_REMOVE_ASYM_DH_KEY_CHECK

#define VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE
#define VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL
#endif

#define VAL_REMOVE_ASYM_DSA_SIGN
#define VAL_REMOVE_ASYM_DSA_VERIFY
#define VAL_REMOVE_ASYM_DSA_KEY_CHECK

#ifndef EIP130_ENABLE_ASYM_ALGO_RSA
#define VAL_REMOVE_ASYM_RSAPKCS1V15_SIGN
#define VAL_REMOVE_ASYM_RSAPKCS1V15_VERIFY
#define VAL_REMOVE_ASYM_RSAPSS_SIGN
#define VAL_REMOVE_ASYM_RSAPSS_VERIFY
#define VAL_REMOVE_ASYM_RSA_OAEP
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_SM2
#define VAL_REMOVE_ASYM_SM2DSA_SIGN
#define VAL_REMOVE_ASYM_SM2DSA_VERIFY
#define VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET
#define VAL_REMOVE_ASYM_SM2ENC
#endif

#ifndef EIP130_ENABLE_ASYM_ALGO_ECIES
#define VAL_REMOVE_ASYM_ECIES
#endif

//#define VAL_REMOVE_ASYM_PK_WITHOUT_ASSET

// - Authenticated Unlock related
//#define VAL_REMOVE_AUTH_UNLOCK
//#define VAL_REMOVE_SECURE_DEBUG

// - Random related
//#define VAL_REMOVE_RANDOM_DATA
//#define VAL_REMOVE_RANDOM_RESEED
//#define VAL_REMOVE_VERIFY_DRBG_POSTPROCCESSING
//#define VAL_REMOVE_VERIFY_NRBG_SELFTEST
//#define VAL_REMOVE_TRNG_RAW_DATA
#ifndef VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS
#define VAL_REMOVE_TRNG_CONFIG
#define VAL_REMOVE_TRNG_RAW_DATA
#endif
#ifndef EIP130_ENABLE_DRBG_FORCESEED
#define VAL_REMOVE_DRBG_FORCESEED
#endif

// - Miscellaneous
//#define VAL_REMOVE_NOP

// - Special Functions related
#ifndef EIP130_ENABLE_SF_MILENAGE
#define VAL_REMOVE_SF_MILENAGE
#endif

// - Service related
//#define VAL_REMOVE_SERVICE_REGISTERREAD
#ifndef VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS
#define VAL_REMOVE_SERVICE_REGISTERWRITE
#endif
#ifndef EIP130_ENABLE_FIRMWARE_LOAD
#define VAL_REMOVE_SERVICE_FIRMWARECHECK
#define VAL_REMOVE_SERVICE_UPDATEROLLBACKID
#endif
//#define VAL_REMOVE_SERVICE_ZEROIZEMAILBOX
//#define VAL_REMOVE_SERVICE_ZEROIZEOTP
#ifndef VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS
#define VAL_REMOVE_CLOCKSWITCH
#endif

// - System related
//#define VAL_REMOVE_SYSTEM_GETVERSION
//#define VAL_REMOVE_SYSTEM_RESET
//#define VAL_REMOVE_SYSTEM_SELFTEST
#ifndef VAL_API_ADD_CRYPTO_OFFICER_FUNCTIONS
#define VAL_REMOVE_SYSTEM_DEFINE_USERS
#define VAL_REMOVE_SYSTEM_SLEEP
#define VAL_REMOVE_SYSTEM_HIBERNATION
#define VAL_REMOVE_SYSTEM_SET_TIME
#else
//#define VAL_REMOVE_SYSTEM_DEFINE_USERS
#ifndef EIP130_ENABLE_FIRMWARE_SLEEP
#define VAL_REMOVE_SYSTEM_SLEEP
#endif
#if !defined(EIP130_ENABLE_FIRMWARE_HIBERNATION) || \
    !defined(EIP130_ENABLE_EMMC)
#define VAL_REMOVE_SYSTEM_HIBERNATION
#endif
#ifndef EIP130_FW_ASSET_LIFETIME_MANAGEMENT
#define VAL_REMOVE_SYSTEM_SET_TIME
#endif
#endif
#ifndef EIP130_ENABLE_SYSTEM_ENTERFIPSMODE
#define VAL_REMOVE_SYSTEM_ENTERFIPSMODE
#endif

// - eMMC related
#ifndef EIP130_ENABLE_EMMC
#define VAL_REMOVE_EMMC
#define VAL_REMOVE_EXTSERVICE
#else
#ifdef EIP130_ENABLE_EXTSERVICE
#define VAL_REMOVE_EXTSERVICE
#endif
#endif

// - Claim control related
//#define VAL_REMOVE_CLAIM


#endif /* INCLUDE_GUARD_CS_ADAPTER_VAL_H */

/* end of file cs_adapter_val.h */
