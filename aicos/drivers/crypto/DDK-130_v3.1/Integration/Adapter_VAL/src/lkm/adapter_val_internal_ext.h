/* adapter_val_internal_ext.h
 *
 * Linux kernel-space specific Adapter VAL internal extensions
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

#ifndef INCLUDE_GUARD_ADAPTER_VAL_INTERNAL_EXT_H
#define INCLUDE_GUARD_ADAPTER_VAL_INTERNAL_EXT_H


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_val.h"                    // Vault IP API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Top-level Adapter configuration
#include "cs_adapter_val.h"

// Linux Kernel API
#include <linux/init.h>     // module_init, module_exit
#include <linux/module.h>   // EXPORT_SYMBOL

#define ADAPTER_VAL_INTERNAL_USER_SPACE_FLAG        false


// Vault IP API
// - Symmetric Crypto related
#if !defined(VAL_REMOVE_SYM_HASH) || \
    !defined(VAL_REMOVE_SYM_MAC) || \
    !defined(VAL_REMOVE_SYM_CIPHER)
EXPORT_SYMBOL(val_SymAlloc);
EXPORT_SYMBOL(val_SymRelease);
EXPORT_SYMBOL(val_SymInitKey);
EXPORT_SYMBOL(val_SymInitIV);
#ifdef VAL_REMOVE_SYM_ALGO_SHA3
EXPORT_SYMBOL(val_SymReadTokenTemp);
EXPORT_SYMBOL(val_SymWriteTokenTemp);
#endif
#endif
#ifndef VAL_REMOVE_SYM_HASH
EXPORT_SYMBOL(val_SymHashUpdate);
EXPORT_SYMBOL(val_SymHashFinal);
#endif
#ifndef VAL_REMOVE_SYM_MAC
EXPORT_SYMBOL(val_SymMacInitVerify);
EXPORT_SYMBOL(val_SymMacUpdate);
EXPORT_SYMBOL(val_SymMacGenerate);
EXPORT_SYMBOL(val_SymMacVerify);
#endif
#ifndef VAL_REMOVE_SYM_CIPHER
EXPORT_SYMBOL(val_SymCipherInitEncrypt);
#ifndef VAL_REMOVE_SYM_ALGO_SM4
EXPORT_SYMBOL(val_SymCipherInitParameter);
#endif
#ifndef VAL_REMOVE_SYM_ALGO_AES_F8
EXPORT_SYMBOL(val_SymCipherInitF8);
#endif
#ifndef VAL_REMOVE_SYM_ALGO_CHACHA20
EXPORT_SYMBOL(val_SymCipherInitChaCha20);
#endif
EXPORT_SYMBOL(val_SymCipherUpdate);
EXPORT_SYMBOL(val_SymCipherFinal);
#if !defined(VAL_REMOVE_SYM_ALGO_AES_CCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_AES_GCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_ARIA_CCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_ARIA_GCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_SM4_GCM)
EXPORT_SYMBOL(val_SymCipherAEInit);
EXPORT_SYMBOL(val_SymCipherAEFinal);
#endif
#if !defined(VAL_REMOVE_SYM_ALGO_AES_GCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_ARIA_GCM) || \
    !defined(VAL_REMOVE_SYM_ALGO_SM4_GCM)
EXPORT_SYMBOL(val_SymCipherInitNoRandomIV);
#endif
#endif
#ifndef VAL_REMOVE_SYM_KEYWRAP
EXPORT_SYMBOL(val_SymKeyWrap);
EXPORT_SYMBOL(val_SymKeyUnwrap);
#endif
#ifndef VAL_REMOVE_ENCRYPTED_VECTOR
EXPORT_SYMBOL(val_EncryptVectorForPKI);
#endif

// - Asymmetric Crypto related
EXPORT_SYMBOL(val_AsymInitKey);
#if !defined(VAL_REMOVE_ASYM_ECC_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_ECC_GEN_PUBLICKEY) || \
    !defined(VAL_REMOVE_ASYM_ECDSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_ECDSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_EDDSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_EDDSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_ENCRYPT) || \
    !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_DECRYPT) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL) || \
    !defined(VAL_REMOVE_ASYM_CURVE25519)
EXPORT_SYMBOL(val_AsymEccAllocDomainAsset);
EXPORT_SYMBOL(val_AsymEccLoadDomainAssetPlaintext);
#endif
#if !defined(VAL_REMOVE_ASYM_ECDSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_DECRYPT) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL) || \
    !defined(VAL_REMOVE_ASYM_EDDSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_EDDSA_VERIFY)
EXPORT_SYMBOL(val_AsymEccLoadPrivateKeyAssetPlaintext);
#endif
#if !defined(VAL_REMOVE_ASYM_ECDSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_ENCRYPT) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL)
EXPORT_SYMBOL(val_AsymEccLoadPublicKeyAssetPlaintext);
#endif
#if !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL)
EXPORT_SYMBOL(val_AsymEcdhAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymEcdhAllocPublicKeyAsset);
#endif
#ifndef VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_SINGLE
EXPORT_SYMBOL(val_AsymEcdhGenSharedSecretSingle);
#endif
#ifndef VAL_REMOVE_ASYM_ECDH_GEN_SHAREDSECRET_DUAL
EXPORT_SYMBOL(val_AsymEcdhGenSharedSecretDual);
#endif
#ifndef VAL_REMOVE_ASYM_ECDSA_SIGN
EXPORT_SYMBOL(val_AsymEcdsaAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymEcdsaSign);
#endif
#ifndef VAL_REMOVE_ASYM_ECDSA_VERIFY
EXPORT_SYMBOL(val_AsymEcdsaAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymEcdsaVerify);
#endif
#ifndef VAL_REMOVE_ASYM_ECC_GEN_KEYPAIR
EXPORT_SYMBOL(val_AsymEccGenKeyPair);
#endif
#ifndef VAL_REMOVE_ASYM_ECC_GEN_PUBLICKEY
EXPORT_SYMBOL(val_AsymEccGenPublicKey);
#endif
#ifndef VAL_REMOVE_ASYM_ECC_KEY_CHECK
EXPORT_SYMBOL(val_AsymEccKeyCheck);
#endif
#ifndef VAL_REMOVE_ASYM_ECC_ELGAMAL_ENCRYPT
EXPORT_SYMBOL(val_AsymEccElGamalAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymEccElGamalEncrypt);
#endif
#ifndef VAL_REMOVE_ASYM_ECC_ELGAMAL_DECRYPT
EXPORT_SYMBOL(val_AsymEccElGamalAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymEccElGamalDecrypt);
#endif
#ifndef VAL_REMOVE_ASYM_CURVE25519
EXPORT_SYMBOL(val_AsymCurve25519AllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymCurve25519AllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymCurve25519LoadKeyAssetPlaintext);
EXPORT_SYMBOL(val_AsymCurve25519GenKeyPair);
EXPORT_SYMBOL(val_AsymCurve25519GenPublicKey);
EXPORT_SYMBOL(val_AsymCurve25519GenSharedSecret);
#endif
#if !defined(VAL_REMOVE_ASYM_EDDSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_EDDSA_VERIFY)
EXPORT_SYMBOL(val_AsymEddsaAllocKeyAsset);
EXPORT_SYMBOL(val_AsymEddsaLoadKeyAssetPlaintext);
EXPORT_SYMBOL(val_AsymEddsaGenKeyPair);
EXPORT_SYMBOL(val_AsymEddsaGenPublicKey);
EXPORT_SYMBOL(val_AsymEddsaSign);
EXPORT_SYMBOL(val_AsymEddsaVerify);
#endif
#if !defined(VAL_REMOVE_ASYM_DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_DSA_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL)
EXPORT_SYMBOL(val_AsymDhDsaAllocDomainAsset);
EXPORT_SYMBOL(val_AsymDhDsaLoadDomainAssetPlaintext);
EXPORT_SYMBOL(val_AsymDhDsaLoadKeyAssetPlaintext);
#endif
#if !defined(VAL_REMOVE_ASYM_DH_KEY_CHECK) || \
    !defined(VAL_REMOVE_ASYM_DSA_KEY_CHECK)
EXPORT_SYMBOL(val_AsymDhDsaKeyCheck);
#endif
#ifndef VAL_REMOVE_ASYM_DSA_SIGN
EXPORT_SYMBOL(val_AsymDsaAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymDsaSign);
#endif
#ifndef VAL_REMOVE_ASYM_DSA_VERIFY
EXPORT_SYMBOL(val_AsymDsaAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymDsaVerify);
#endif
#ifndef VAL_REMOVE_ASYM_DH_GEN_KEYPAIR
EXPORT_SYMBOL(val_AsymDhAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymDhGenKeyPair);
#endif
#ifndef VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY
EXPORT_SYMBOL(val_AsymDhGenPublicKey);
#endif
#if !defined(VAL_REMOVE_ASYM_DH_GEN_KEYPAIR) || \
    !defined(VAL_REMOVE_ASYM_DH_GEN_PUBLICKEY)
EXPORT_SYMBOL(val_AsymDhAllocPublicKeyAsset);
#endif
#ifndef VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_SINGLE
EXPORT_SYMBOL(val_AsymDhGenSharedSecretSingle);
#endif
#ifndef VAL_REMOVE_ASYM_DH_GEN_SHAREDSECRET_DUAL
EXPORT_SYMBOL(val_AsymDhGenSharedSecretDual);
#endif
#if !defined(VAL_REMOVE_ASYM_RSAPKCS1V15_SIGN) || \
    !defined(VAL_REMOVE_ASYM_RSAPKCS1V15_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_RSAPSS_SIGN) || \
    !defined(VAL_REMOVE_ASYM_RSAPSS_VERIFY) || \
    !defined(VAL_REMOVE_ASYM_RSA_OAEP) || \
    !defined(VAL_REMOVE_ASYM_RSA_PKCS1WRAP)
EXPORT_SYMBOL(val_AsymRsaLoadKeyAssetPlaintext);
#endif
#ifndef VAL_REMOVE_ASYM_RSAPKCS1V15_SIGN
EXPORT_SYMBOL(val_AsymRsaPkcs1v15AllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPkcs1v15Sign);
#endif
#ifndef VAL_REMOVE_ASYM_RSAPKCS1V15_VERIFY
EXPORT_SYMBOL(val_AsymRsaPkcs1v15AllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPkcs1v15Verify);
#endif
#ifndef VAL_REMOVE_ASYM_RSAPSS_SIGN
EXPORT_SYMBOL(val_AsymRsaPssAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPssSign);
#endif
#ifndef VAL_REMOVE_ASYM_RSAPSS_VERIFY
EXPORT_SYMBOL(val_AsymRsaPssAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPssVerify);
#endif
#ifndef VAL_REMOVE_ASYM_RSA_OAEP
EXPORT_SYMBOL(val_AsymRsaOaepAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymRsaOaepAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymRsaOaepWrapString);
EXPORT_SYMBOL(val_AsymRsaOaepWrapHash);
EXPORT_SYMBOL(val_AsymRsaOaepUnwrapString);
EXPORT_SYMBOL(val_AsymRsaOaepUnwrapHash);
#endif
#ifndef VAL_REMOVE_ASYM_RSA_PKCS1WRAP
EXPORT_SYMBOL(val_AsymRsaPkcs1v15WrapAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPkcs1v15WrapAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymRsaPkcs1v15Wrap);
EXPORT_SYMBOL(val_AsymRsaPkcs1v15Unwrap);
#endif
#if !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) || \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY)
EXPORT_SYMBOL(val_AsymSm2dsaCalculateZA);
#endif
#ifndef VAL_REMOVE_ASYM_SM2DSA_SIGN
EXPORT_SYMBOL(val_AsymSm2dsaAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymSm2dsaSign);
#endif
#ifndef VAL_REMOVE_ASYM_SM2DSA_VERIFY
EXPORT_SYMBOL(val_AsymSm2dsaAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymSm2dsaVerify);
#endif
#ifndef VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET
EXPORT_SYMBOL(val_AsymSm2kexAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymSm2kexAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymSm2kexGenSharedSecret);
#endif
#ifndef VAL_REMOVE_ASYM_SM2ENC
EXPORT_SYMBOL(val_AsymSm2encAllocPrivateKeyAsset);
EXPORT_SYMBOL(val_AsymSm2encAllocPublicKeyAsset);
EXPORT_SYMBOL(val_AsymSm2EncWrap);
EXPORT_SYMBOL(val_AsymSm2EncUnwrap);
#endif
#ifndef VAL_REMOVE_ASYM_ECIES
EXPORT_SYMBOL(val_AsymECIESWrap);
EXPORT_SYMBOL(val_AsymECIESUnwrap);
#endif
#ifndef VAL_REMOVE_ASYM_PK_WITHOUT_ASSET
EXPORT_SYMBOL(val_AsymPkaClaim);
EXPORT_SYMBOL(val_AsymPkaLoadVector);
EXPORT_SYMBOL(val_AsymPkaExecuteOperation);
EXPORT_SYMBOL(val_AsymPkaClaimRelease);
#endif

// - Authenticated Unlock & Secure Debug related
#ifndef VAL_REMOVE_AUTH_UNLOCK
EXPORT_SYMBOL(val_AuthenticatedUnlockStart);
EXPORT_SYMBOL(val_AuthenticatedUnlockVerify);
#ifndef VAL_REMOVE_SECURE_DEBUG
EXPORT_SYMBOL(val_SecureDebug);
#endif
#endif

// - No Operation (NOP) related
#ifndef VAL_REMOVE_NOP
EXPORT_SYMBOL(val_NOP);
#endif

// - Random related
#ifndef VAL_REMOVE_RANDOM_DATA
EXPORT_SYMBOL(val_RandomData);
#endif
#ifndef VAL_REMOVE_TRNG_RAW_DATA
EXPORT_SYMBOL(val_TrngRawData);
#endif
#ifndef VAL_REMOVE_RANDOM_RESEED
EXPORT_SYMBOL(val_RandomReseed);
#endif
#ifndef VAL_REMOVE_TRNG_CONFIG
#ifdef EIP130_ENABLE_TRNG_CUTOFF
EXPORT_SYMBOL(val_TrngConfigCutoff);
#else
EXPORT_SYMBOL(val_TrngConfig);
#endif
#endif
#ifndef VAL_REMOVE_TRNG_CONFIG_ARIA
#ifdef EIP130_ENABLE_TRNG_CUTOFF
EXPORT_SYMBOL(val_TrngConfigAriaDRBGCutoff);
#else
EXPORT_SYMBOL(val_TrngConfigAriaDRBG);
#endif
#endif
#ifndef VAL_REMOVE_VERIFY_DRBG_POSTPROCCESSING
EXPORT_SYMBOL(val_TrngDrbgPostProcessing);
#endif
#ifndef VAL_REMOVE_VERIFY_NRBG_SELFTEST
EXPORT_SYMBOL(val_TrngNrbgSelfTest);
#endif

// - Asset related
#ifndef VAL_REMOVE_ASSET_ALLOC
EXPORT_SYMBOL(val_AssetAlloc);
#endif
#ifndef VAL_REMOVE_ASSET_FREE
EXPORT_SYMBOL(val_AssetFree);
#endif
#ifndef VAL_REMOVE_ASSET_SEARCH
EXPORT_SYMBOL(val_AssetSearch);
#endif
#ifndef VAL_REMOVE_GET_ROOT_KEY
EXPORT_SYMBOL(val_AssetGetRootKey);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_IMPORT
EXPORT_SYMBOL(val_AssetLoadImport);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_DERIVE
EXPORT_SYMBOL(val_AssetLoadDerive);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_PLAINTEXT
EXPORT_SYMBOL(val_AssetLoadPlaintext);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_PLAINTEXT_EXPORT
EXPORT_SYMBOL(val_AssetLoadPlaintextExport);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_RANDOM
EXPORT_SYMBOL(val_AssetLoadRandom);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_RANDOM_EXPORT
EXPORT_SYMBOL(val_AssetLoadRandomExport);
#endif
#ifndef VAL_REMOVE_ASSET_LOAD_SYMWRAP
EXPORT_SYMBOL(val_AssetLoadSymUnwrap);
#endif
#ifndef VAL_REMOVE_SECURETIMER
EXPORT_SYMBOL(val_SecureTimerStart);
EXPORT_SYMBOL(val_SecureTimerStop);
EXPORT_SYMBOL(val_SecureTimerRead);
#endif
#ifndef VAL_REMOVE_PUBLICDATA_READ
EXPORT_SYMBOL(val_PublicDataRead);
#endif
#ifndef VAL_REMOVE_MONOTONIC_COUNTER_READ
EXPORT_SYMBOL(val_MonotonicCounterRead);
#endif
#ifndef VAL_REMOVE_MONOTONIC_COUNTER_INC
EXPORT_SYMBOL(val_MonotonicCounterIncrement);
#endif
#ifndef VAL_REMOVE_OTPDATAWRITE
EXPORT_SYMBOL(val_OTPDataWrite);
#endif
#ifndef VAL_REMOVE_PROVISIONRANDOMROOTKEY
EXPORT_SYMBOL(val_ProvisionRandomRootKey);
#endif
#ifndef VAL_REMOVE_PROVISIONRANDOMROOTKEYARIA
EXPORT_SYMBOL(val_ProvisionRandomRootKeyARIA);
#endif
#ifndef VAL_REMOVE_ASSET_EXPORT_VIA_CPIF
EXPORT_SYMBOL(val_AssetExportViaCPIF);
#endif
#ifndef VAL_REMOVE_ASSET_STORE_RESET
EXPORT_SYMBOL(val_AssetStoreReset);
#endif

// Special Functions related
#ifndef VAL_REMOVE_SF_MILENAGE
EXPORT_SYMBOL(val_SFMilenageSqnAdminCreate);
EXPORT_SYMBOL(val_SFMilenageSqnAdminReset);
EXPORT_SYMBOL(val_SFMilenageSqnAdminExport);
EXPORT_SYMBOL(val_SFMilenageAutnVerificationSqn);
EXPORT_SYMBOL(val_SFMilenageAutnVerification);
EXPORT_SYMBOL(val_SFMilenageAutsGeneration);
EXPORT_SYMBOL(val_SFMilenageConformance);
#endif

// - Service related
#ifndef VAL_REMOVE_SERVICE_REGISTERREAD
EXPORT_SYMBOL(val_ServiceRegisterRead);
#endif
#ifndef VAL_REMOVE_SERVICE_REGISTERWRITE
EXPORT_SYMBOL(val_ServiceRegisterWrite);
#endif
#ifndef VAL_REMOVE_SERVICE_ZEROIZEMAILBOX
EXPORT_SYMBOL(val_ServiceZeroizeMailbox);
#endif
#ifndef VAL_REMOVE_SERVICE_ZEROIZEOTP
EXPORT_SYMBOL(val_ServiceSelectOTPZeroize);
EXPORT_SYMBOL(val_ServiceZeroizeOTP);
#endif
#ifndef VAL_REMOVE_CLOCKSWITCH
EXPORT_SYMBOL(val_ServiceClockSwitch);
#endif
#ifndef VAL_REMOVE_FIRMWARECHECK
EXPORT_SYMBOL(val_ServiceFirmwareCheck);
#endif
#ifndef VAL_REMOVE_UPDATEROLLBACKID
EXPORT_SYMBOL (val_ServiceUpdateRollbackID);
#endif

// - System related
EXPORT_SYMBOL(val_SystemGetState);
EXPORT_SYMBOL(val_IsAccessSecure);
#ifndef VAL_REMOVE_SYSTEM_GETVERSION
EXPORT_SYMBOL(val_SystemGetVersion);
#endif
#ifndef VAL_REMOVE_SYSTEM_RESET
EXPORT_SYMBOL(val_SystemReset);
#endif
#ifndef VAL_REMOVE_SYSTEM_SELFTEST
EXPORT_SYMBOL(val_SystemSelfTest);
#endif
#ifndef VAL_REMOVE_SYSTEM_DEFINE_USERS
EXPORT_SYMBOL(val_SystemDefineUsers);
#endif
#ifndef VAL_REMOVE_SYSTEM_SET_TIME
EXPORT_SYMBOL(val_SystemSetTime);
#endif
#ifndef VAL_REMOVE_SYSTEM_ENTERFIPSMODE
EXPORT_SYMBOL(val_SystemEnterFipsMode);
#endif
#ifndef VAL_REMOVE_SYSTEM_SLEEP
EXPORT_SYMBOL(val_SystemSleep);
EXPORT_SYMBOL(val_SystemResumeFromSleep);
#endif
#ifndef VAL_REMOVE_SYSTEM_HIBERNATION
EXPORT_SYMBOL(val_SystemHibernation);
EXPORT_SYMBOL(val_SystemResumeFromHibernation);
#endif

// eMMC related
#ifndef VAL_REMOVE_EMMC
EXPORT_SYMBOL(val_eMMCReadRequest);
EXPORT_SYMBOL(val_eMMCReadVerify);
EXPORT_SYMBOL(val_eMMCReadWriteCounterRequest);
EXPORT_SYMBOL(val_eMMCReadWriteCounterVerify);
EXPORT_SYMBOL(val_eMMCWriteRequest);
EXPORT_SYMBOL(val_eMMCWriteVerify);
#endif

// - Hardware related
#ifndef VAL_REMOVE_HARDWARE_MODULESTATUS
EXPORT_SYMBOL(val_HW_ModuleStatus);
#endif
#ifndef VAL_REMOVE_HARDWARE_EIP_OPTIONS
EXPORT_SYMBOL(val_HW_EIP_Options);
#endif
#ifndef VAL_REMOVE_HARDWARE_EIP_VERSION
EXPORT_SYMBOL(val_HW_EIP_Version);
#endif

// - Claim control related
#ifndef VAL_REMOVE_CLAIM
EXPORT_SYMBOL(val_Claim);
EXPORT_SYMBOL(val_ClaimOverrule);
EXPORT_SYMBOL(val_ClaimRelease);
#endif


#endif /* INCLUDE_GUARD_ADAPTER_VAL_INTERNAL_EXT_H */

/* end of file adapter_val_internal_ext.h */
