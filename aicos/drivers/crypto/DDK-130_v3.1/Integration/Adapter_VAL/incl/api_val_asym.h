/**
 * @file api_val_asym.h
 *
 * @brief VAL API - Asymmetric Cryptography services
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

#ifndef INCLUDE_GUARD_API_VAL_ASYM_H
#define INCLUDE_GUARD_API_VAL_ASYM_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"                 // uint8_t, uint16_t, uint32_t, uint64_t

#include "api_val_result.h"             // ValStatus_t
#include "api_val_asset_defs.h"         // ValAssetId_t
#include "api_val_asym_defs.h"          // ValAsymBigInt_t, ValAsymECCPoint_t, ...


/*-----------------------------------------------------------------------------
 * val_AsymEccAllocDomainAsset
 */
/**
 * This function allocates an ECC Domain parameters Asset.
 * Note: Can be used for ECDH, ECDSA and ECC El-Gamal.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccAllocDomainAsset(
        const ValSize_t ModulusSizeBits,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEcdhAllocPrivateKeyAsset
 */
/**
 * This function allocates an ECDH private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdhAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEcdhAllocPublicKeyAsset
 */
/**
 * This function allocates an ECDH public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdhAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEcdsaAllocPrivateKeyAsset
 */
/**
 * This function allocates an ECDSA private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdsaAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEcdsaAllocPublicKeyAsset
 */
/**
 * This function allocates an ECDSA public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdsaAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2dsaAllocPrivateKeyAsset
 */
/**
 * This function allocates an SM2DSA private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2dsaAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2dsaAllocPublicKeyAsset
 */
/**
 * This function allocates an SM2DSA public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2dsaAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2kexAllocPrivateKeyAsset
 */
/**
 * This function allocates an SM2KEX private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2kexAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2kexAllocPublicKeyAsset
 */
/**
 * This function allocates an SM2KEX public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2kexAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2encAllocPrivateKeyAsset
 */
/**
 * This function allocates an SM2ENC private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2encAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2encAllocPublicKeyAsset
 */
/**
 * This function allocates an SM2ENC public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2encAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaAllocKeyAsset
 */
/**
 * This function allocates an EDDSA private or public key Asset.
 *
 * @param [in] fPublicKey
 *     Indication to allocate a public key.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaAllocKeyAsset(
        const bool fPublicKey,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/** Please look at val_AsymEddsaAllocKeyAsset() for details */
#define val_AsymEddsaAllocPrivateKeyAsset(a,b,c) val_AsymEddsaAllocKeyAsset(false,a,b,c)
/** Please look at val_AsymEddsaAllocKeyAsset() for details */
#define val_AsymEddsaAllocPublicKeyAsset(a,b,c)  val_AsymEddsaAllocKeyAsset(true,a,b,c)

/*-----------------------------------------------------------------------------
 * val_AsymEccElGamalAllocPrivateKeyAsset
 */
/**
 * This function allocates an ECC El-Gamal private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccElGamalAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymEccElGamalAllocPublicKeyAsset
 */
/**
 * This function allocates an ECC El-Gamal public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccElGamalAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymCurve25519AllocPrivateKeyAsset
 */
/**
 * This function allocates a Curve25519 private key Asset.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519AllocPrivateKeyAsset(
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymCurve25519AllocPublicKeyAsset
 */
/**
 * This function allocates a Curve25519 public key Asset.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519AllocPublicKeyAsset(
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhDsaAllocDomainAsset
 */
/**
 * This function allocates a DH or DSA Domain parameters Asset.
 *
 * @param [in] PrimeSizeBits
 *     Size of the prime modulus in bits.
 *
 * @param [in] DivisorSizeBits
 *     Size of the divisor (order or sub-prime) in bits.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhDsaAllocDomainAsset(
        const ValSize_t PrimeSizeBits,
        const ValSize_t DivisorSizeBits,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymDsaAllocPrivateKeyAsset
 */
/**
 * This function allocates a DSA private key Asset.
 *
 * @param [in] DivisorSizeBits
 *     Size of the divisor (order or sub-prime) in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDsaAllocPrivateKeyAsset(
        const ValSize_t DivisorSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymDsaAllocPublicKeyAsset
 */
/**
 * This function allocates a DSA public key Asset.
 *
 * @param [in] PrimeSizeBits
 *     Size of the prime modulus in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDsaAllocPublicKeyAsset(
        const ValSize_t PrimeSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhAllocPrivateKeyAsset
 */
/**
 * This function allocates a DH private key Asset.
 *
 * @param [in] DivisorSizeBits
 *     Size of the divisor (order or sub-prime) in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhAllocPrivateKeyAsset(
        const ValSize_t DivisorSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhAllocPublicKeyAsset
 */
/**
 * This function allocates a DH public key Asset.
 *
 * @param [in] PrimeSizeBits
 *     Size of the prime modulus in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhAllocPublicKeyAsset(
        const ValSize_t PrimeSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15AllocPrivateKeyAsset
 */
/**
 * This function allocates a RSA PKCS#1v1.5 private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15AllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15AllocPublicKeyAsset
 */
/**
 * This function allocates a RSA PKCS#1v1.5 public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15AllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPssAllocPrivateKeyAsset
 */
/**
 * This function allocates a RSA PSS private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPssAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPssAllocPublicKeyAsset
 */
/**
 * This function allocates a RSA PSS public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPssAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);


/*-----------------------------------------------------------------------------
 * val_AsymEccLoadDomainAssetPlaintext
 */
/**
 * This function converts the plaintext ECC domain parameters to the EIP-13x
 * HW format and initializes the Asset for it in the Asset Store.
 * Note: Can be used for ECDH, ECDSA and ECC El-Gamal.
 *
 * @param [in] Domain_p
 *     Pointer to the buffer that holds the ECC domain parameter according
 *     ValAsymECDomainParam_t.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the ECC domain
 *     parameters.
 *     This reference is provided by val_AsymEccAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccLoadDomainAssetPlaintext(
        const ValAsymECDomainParam_t * const Domain_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymEccLoadPrivateKeyAssetPlaintext
 */
/**
 * This function converts the ECC private key to the EIP-13x HW format and
 * initializes the Asset for it in the Asset Store.
 * Note: Can be used for ECDH, ECDSA and ECC El-Gamal.
 *
 * @param [in] Key_p
 *     Pointer to the buffer that holds the ECC private key.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the ECC domain
 *     parameters.
 *     This reference is provided by val_AsymEccAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccLoadPrivateKeyAssetPlaintext(
        const ValAsymBigInt_t * const Key_p,
        const ValSize_t ModulusSizeBits,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymEccLoadPublicKeyAssetPlaintext
 */
/**
 * This function converts the ECC public key to the EIP-13x HW format and
 * initializes the Asset for it in the Asset Store.
 * Note: Can be used for ECDH, ECDSA and ECC El-Gamal.
 *
 * @param [in] KeyPoint_p
 *     Pointer to the buffer that holds the ECC public key (an ECC point).
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the ECC domain
 *     parameters.
 *     This reference is provided by val_AsymEccAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccLoadPublicKeyAssetPlaintext(
        const ValAsymECCPoint_t * const KeyPoint_p,
        const ValSize_t ModulusSizeBits,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaLoadKeyAssetPlaintext
 */
/**
 * This function converts the specified private or public key to the EIP-13x
 * HW format and initializes the Asset for it in the Asset Store.
 *
 * @param [in] KeyValue_p
 *     Pointer to the buffer that holds the key value.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the ECC domain
 *     parameters.
 *     This reference is provided by val_AsymEccAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaLoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const KeyValue_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymCurve25519LoadKeyAssetPlaintext
 */
/**
 * This function converts the specified private or public key to the EIP-13x
 * HW format and initializes the Asset for it in the Asset Store.
 *
 * @param [in] KeyValue_p
 *     Pointer to the buffer that holds the key value.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the ECC domain
 *     parameters.
 *     This reference is provided by val_AsymEccAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519LoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const KeyValue_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhDsaLoadDomainAssetPlaintext
 */
/**
 * This function converts the plaintext DH or DSA domain parameters to the
 * EIP-13x HW format and initializes the Asset for it in the Asset Store.
 *
 * @param [in] Domain_p
 *     Pointer to the buffer that holds the DSA domain parameter according
 *     ValAsymDHDSADomainParam_t.
 *
 * @param [in] TargetAssetId
 *     Reference to the Asset that needs to be loaded with the DH or DSA
 *     domain parameters.
 *     This reference is provided by val_AsymDhDsaAllocDomainAsset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhDsaLoadDomainAssetPlaintext(
        const ValAsymDHDSADomainParam_t * const Domain_p,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhDsaLoadKeyAssetPlaintext
 */
/**
 * This function converts the DSA private or public key data to the EIP-13x HW
 * format and initializes the Asset for it in the Asset Store.
 *
 * @param [in] Data_p
 *     Pointer to the buffer that holds the data (a big integer).
 *
 * @param [in] DataSizeBits
 *     Size of the data in bits.
 *
 * @param [in] TargetAssetId
 *     Reference to the key Asset that needs to be loaded.
 *     This reference is provided by val_AsymDsaAlloc*Asset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhDsaLoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const Data_p,
        const ValSize_t DataSizeBits,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaLoadKeyAssetPlaintext
 */
/**
 * This function converts the RSA key data to the EIP-13x HW format and
 * initializes the Asset for it in the Asset Store.
 * Note: Can be used for all RSA key types.
 *
 * @param [in] Modulus_p
 *     Pointer to the buffer that holds the modulus (a big integer).
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] Exponent_p
 *     Pointer to the buffer that holds the exponent (a big integer).
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] TargetAssetId
 *     Reference to the key Asset that needs to be loaded.
 *     This reference is provided by val_AsymRsa*Alloc*Asset() or
 *     val_AssetAlloc().
 *     Note: The Asset contains the desired length of the Asset.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] KeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] KeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaLoadKeyAssetPlaintext(
        const ValAsymBigInt_t * const Modulus_p,
        const ValSize_t ModulusSizeBits,
        const ValAsymBigInt_t * const Exponent_p,
        const ValSize_t ExponentSizeBits,
        const ValAssetId_t TargetAssetId,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_Optional_t * const KeyBlob_p,
        ValSize_t * const KeyBlobSize_p);


/*-----------------------------------------------------------------------------
 * val_AsymInitKey
 */
/**
 * This function initializes an asymmetric crypto key.
 * Note: This function must be called when the key part are loaded via a
 *       key blob into the Asset Store.
 *
 * @param [in] KeyAssetId
 *     Optional reference to the private/public key Asset. If not provided,
 *     the DomainAssetId must be provided.
 *     This reference is provided by val_AssetAlloc().
 *
 * @param [in] DomainAssetId
 *     Optional reference to the domain parameters Asset.
 *     This reference is provided by val_AssetAlloc().
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [out] KeyContext_p
 *     Pointer to the asymmetric crypto key structure that is initialized.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymInitKey(
        const ValAssetId_Optional_t KeyAssetId,
        const ValAssetId_Optional_t DomainAssetId,
        const ValSize_t ModulusSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        ValAsymKey_t * const KeyContext_p);


/*-----------------------------------------------------------------------------
 * val_AsymEcdsaSign
 */
/**
 * This function is used for generating the ECDSA signature of the supplied
 * message.  The signature is generated using the ECDSA algorithm as defined
 * in standard FIPS PUB 186-4 and IEEE-P1363.
 * The supported ECDSA curves vary from 192 bits to 521 bits.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for ECDSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *     Pointer to the symmetric crypto context from the already performed hash
 *     update operation(s) or NULL if HashMessage_p refers to the complete
 *     message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed on via HashMessage_p and
 *     HashMessageSize.
 *     The data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymEcdsaVerify
 */
/**
 * This function is used to verify the ECDSA signature of the supplied message.
 * The signature is verified using the ECDSA algorithm as defined in standard
 * FIPS PUB 186-4 and IEEE-P1363.
 * The supported ECDSA curves vary from 192 bits to 521 bits.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymSm2CalculateZA
 */
ValStatus_t
val_AsymSm2dsaCalculateZA(
        ValOctetsIn_t * const UserIdentity_p,
        const uint32_t UserIdentitySize,
        const ValAsymECDomainParam_t * const Domain_p,
        const ValAsymECCPoint_t * const PublicKey_p,
        ValOctetsOut_t * const ZA_p,
        ValSize_t * const ZA_size_p);

/*-----------------------------------------------------------------------------
 * val_AsymSm2dsaSign
 */
/**
 * This function is used for generating the SM2DSA signature of the supplied
 * message. The signature is generated using the SM2DSA algorithm as defined
 * in standard GM/T 0003.2-2012 and IETF draft-shen-sm2-ecdsa-02.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for SM2DSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2dsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const ZA_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymSm2dsaVerify
 */
/**
 * This function is used to verify the SM2DSA signature of the supplied message.
 * The signature is verified using the SM2DSA algorithm as defined in standard
 * GM/T 0003.2-2012 and IETF draft-shen-sm2-ecdsa-02.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     SM2DSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2dsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const ZA_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymSm2kexGenSharedSecret
 */
/**
 * This function generates the shared secret based on a dual key pair for the
 * Key Agreement Schemes as defined in GM/T 0003.3-2012.
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified or saved to be used to perform a key derivation at a later stage.
 * The KDF uses the SM3 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] Remote_Ephem_PubKey_p
 *     Pointer to the asymmetric crypto key structure of the SM2
 *     ephemeral public key received from the other party.
 *
 * @param [in] Local_Ephem_PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the SM2
 *     ephemeral private key generated at this side.
 *
 * @param [in] Remote_Static_PubKey_p
 *     Pointer to the asymmetric crypto key structure of the SM2
 *     static public key associated with the other party.
 *
 * @param [in] Local_Static_PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the SM2
 *     static private key associated with this side.
 *
 * @param [in] Local_Ephem_PubKey_p
 *     Pointer to the asymmetric crypto key structure of the SM2
 *     ephemeral public key associated with this side.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the string (label) "ZA || ZB" used by the Key
 *     Derivation Function.
 *
 * @param [in] OtherInfoSize
 *     Size of the string (label) used by the Key Derivation Function.
 *
 * @param [in] AssetIdList_p
 *     Pointer to the list of target Assets to fill.
 *
 * @param [in] NumberOfAssets
 *     The number of target Assets to fill.  This is 3.. up.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2kexGenSharedSecret(
        ValAsymKey_t * const Remote_Ephem_PubKey_p,
        ValAsymKey_t * const Local_Ephem_PrivKey_p,
        ValAsymKey_t * const Remote_Static_PubKey_p,
        ValAsymKey_t * const Local_Static_PrivKey_p,
        ValAsymECCPoint_t * const Local_Ephem_PubKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);

/*-----------------------------------------------------------------------------
 * val_AsymSm2EncWrap
 */
/**
 * This function wraps an Asset using the SM2Enc-WRAP method.
 *
 * @param [in] PubKeyContext_p
 *     Pointer to the asymmetric crypto key structure containing
 *     the public key (and domain params) to wrap the data with.
 *
 * @param [in] AssetId
 *     Reference to the asset to wrap.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in] WrappedDataSize
 *     The size of the WrappedData_p buffer.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2EncWrap(
        ValAsymKey_t * const PubKeyContext_p,
        ValAssetId_t AssetId,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);

 /*-----------------------------------------------------------------------------
 * val_AsymSm2EncUnwrap
 */
/**
 * This function unwraps an Asset using the SM2Enc-UNWRAP method
 *
 * @param PrvKeyContext_p
 *     Pointer to the asymmetric crypto key structure containing
 *     the private key (and domain params) to unwrap the data with.
 *
 * @param AssetId
 *     Reference to the asset in which the unwrapped data will be stored.
 *     The asset must have been created, but not filled.
 *
 * @param [in] WrappedData_p
 *     Pointer to the buffer that holds the crytogram to be decrypted.
 *
 * @param [in] WrappedDataSize
 *     The size of the WrappedData_p buffer.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymSm2EncUnwrap(
        ValAsymKey_t * const PrvKeyContext_p,
        ValAssetId_t AssetId,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaSign
 */
/**
 * This function is used for generating the EdDSA signature of the supplied
 * message. The signature is generated using the EdDSA algorithm as defined in:
 * - 'High-speed high-security signatures'
 *   https://ed25519.cr.yp.to/ed25519-20110926.pdf
 * - 'Edwards-curve Digital Signature Algorithm (EdDSA)'
 *   draft-irtf-cfrg-eddsa-08
 * The supported EdDSA curve is Ed25519 (255 bits).
 *
 * @param [in] PrvKeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for EdDSA.
 *
 * @param [in] PubKeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's public
 *     key for EdDSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaSign(
        ValAsymKey_t * const PrvKeyContext_p,
        ValAsymKey_t * const PubKeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaVerify
 */
/**
 * This function is used to verify the EdDSA signature of the supplied message.
 * The signature is verified using the EdDSA algorithm as defined in:
 * - 'High-speed high-security signatures'
 *   https://ed25519.cr.yp.to/ed25519-20110926.pdf
 * - 'Edwards-curve Digital Signature Algorithm (EdDSA)'
 *   draft-irtf-cfrg-eddsa-08
 * The supported EdDSA curve is Ed25519 (255 bits).
 *
 * @param [in] PubKeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     EdDSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified.
 *
 * @param [in] HashMessageSize
 *     The size od the message that needs to be hashed in bytes.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaVerify(
        ValAsymKey_t * const PubKeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize);


/*-----------------------------------------------------------------------------
 * val_AsymDsaSign
 */
/**
 * This function is used for generating the DSA signature of the supplied
 * message. The signature is generated using the DSA algorithm as defined in
 * standard FIPS PUB 186-4.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for DSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDsaSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p);

/*-----------------------------------------------------------------------------
 * val_AsymDsaVerify
 */
/**
 * This function is used to verify the DSA signature of the supplied message.
 * The signature is verified using the DSA algorithm as defined in standard
 * FIPS PUB 186-4.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     DSA.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDsaVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymSign_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p);


/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15Sign
 */
/**
 * This function is used for generating the signature using a RSA-PKCS#1v1.5
 * algorithm on the supplied message. The signature is generated as defined in
 * the standard PKCS#1.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for RSA-PKCS#1 v1.5.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15Sign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPssSign
 */
/**
 * This function is used for generating the signature using a RSA-PSS
 * algorithm on the supplied message. The signature is generated as defined in
 * the standard PKCS#1.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the signer's private
 *     key for RSA-PSS.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer in which the generated Signature must be returned.
 *     Note: The size of the r and s buffers must be each the curve modulus in
 *           bytes.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be generated or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] SaltSize
 *     The size of the salt in bytes.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPssSign(
        ValAsymKey_t * const KeyContext_p,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        const ValSize_t SaltSize,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15Verify
 */
/**
 * This function is used to verify the signature using the RSA-PKCS#1v1.5
 * algorithm on the supplied message. The signature is verified as defined
 * in the standard PKCS#1.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-PKCS#1 v1.5.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification for
 *     RSA-PKCS#1 v1.5.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15Verify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        bool fExplicitDigest);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPssVerify
 */
/**
 * This function is used to verify the signature using the RSA-PSS algorithm
 * on the supplied message. The signature is verified as defined in the
 * standard PKCS#1.
 * Note that also the last part of the message (no empty/zero-length message
 * allowed) may be provided but in that case the symmetric crypto context of
 * the already hashed message must be provided as well.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-PSS.
 *
 * @param [in] Signature_p
 *     Pointer to the buffer with Signature that must be verification.
 *
 * @param [in] HashMessage_p
 *     Pointer to the message that needs to be hashed before the signature can
 *     be verified or the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashMessageSize
 *     The size of the message that needs to be hashed in bytes or the size of
 *     the explicit digest when fExplicitDigest is enabled.
 *
 * @param [in] HashContext_p
 *      Pointer to the symmetric crypto context from the already performed hash
 *      update operation(s) or NULL if HashMessage_p refers to the complete
 *      message to hash.
 *
 * @param [in] SaltSize
 *     The size of the salt in bytes.
 *
 * @param [in] fExplicitDigest
 *     Indicates that an explicit digest is passed in HashMessage_p.
 *     Note that the data in HashMessage_p will not be hashed.
 *     Attention: Support depends on configuration.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPssVerify(
        ValAsymKey_t * const KeyContext_p,
        ValAsymBigInt_t * const Signature_p,
        ValOctetsIn_t * const HashMessage_p,
        const ValSize_t HashMessageSize,
        ValSymContextPtr_Optional_t const HashContext_p,
        const ValSize_t SaltSize,
        bool fExplicitDigest);


/*-----------------------------------------------------------------------------
 * val_AsymEccGenKeyPair
 */
/**
 * Generate an ECDH, ECDSA or ECC El-Gamal key pair as defined in standard FIPS
 * PUB 186-4 and IEEE-P1363.
 *
 * The key pair is generated as follows:
 *
 * An entity 'A's public and private key pair is associated with a particular
 * set of elliptic curve domain parameters, T = (p; a; b; G; n; h).
 *
 * To generate a key pair, entity 'A' does the following:
 *
 * 1. A random or pseudo-random integer d is selected in the interval [1, n-1].
 * 2. Q = dG is computed.
 * 3. A's public key is Q; A's private key is d.
 *
 * The maximum modulus value of PublicKey can be 521.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] PubKeyPoint_p
 *     Pointer to the buffer in which the public key (an ECC point) must be
 *     returned.
 *
 * @param [out] PrivKeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] PrivKeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccGenKeyPair(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValAsymECCPoint_t * const PubKeyPoint_p,
        ValOctetsOut_Optional_t * const PrivKeyBlob_p,
        ValSize_t * const PrivKeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhGenKeyPair
 */
/**
 * Generate a DH key pair as defined in standard NIST SP800-56A.
 *
 * An entity 'A's key pair is associated with a particular set of domain
 * parameters, T = (p; q; g).
 *
 * The DH key pair is generated as follows :
 *
 * 1. x = a randomly or pseudo randomly generated integer with 0 < x < q
 * 2. y = g^x mod p
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     DH.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     DH.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key (an big integer) must be
 *     returned.
 *
 * @param [out] PrivKeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] PrivKeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhGenKeyPair(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValAsymBigInt_t * const PubKeyValue_p,
        ValOctetsOut_Optional_t * const PrivKeyBlob_p,
        ValSize_t * const PrivKeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaGenKeyPair
 */
/**
 * Generate a EdDSA key pair as defined in:
 * - 'Edwards-curve Digital Signature Algorithm (EdDSA)'
 *   draft-irtf-cfrg-eddsa-08
 *
 * The key pair is generated as follows:
 *
 * The public and private key pair is associated with a particular set of
 * elliptic curve domain parameters, T = (p; d(=a); const(=b); G; n; h).
 *
 * To generate a key pair the following steps are performed:
 * - Private key (x) is a random value in the range [1..2^255].
 * - Public key (y) is calculated according following steps:
 *   1. Hash private key -> H = SHA-512(x)
 *   2. Derive k (prune hash) -> k = (H & (2^(256) -1) & ~(2^255 | 7)) | 2^254
 *   3. Perform a fixed-base scalar multiplication -> Y = [k]G
 *   4. Encode point to get the public key -> y = (Y.y % p) | ((Y.x & 1) << 254)
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     Curve25519.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     Curve25519.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key must be returned.
 *
 * @param [out] PrivKeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] PrivKeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaGenKeyPair(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValAsymBigInt_t * const PubKeyValue_p,
        ValOctetsOut_Optional_t * const PrivKeyBlob_p,
        ValSize_t * const PrivKeyBlobSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymCurve25519GenKeyPair
 */
/**
 * Generate a Curve25519 key pair as defined in RFC 7748 "Elliptic Curves for
 * Security" and Curve25519 "New Diffie-Hellman speed records"
 *
 * The key pair is generated as follows:
 *
 * An entity 'A's public and private key pair is associated with a particular
 * set of elliptic curve domain parameters, T = (p; a; b; G; n; h).
 *
 * To generate a key pair, entity 'A' does the following:
 *
 * 1. A random or pseudo-random integer u is selected in [0..2^254].
 * 2. x = ((u & (2^254-1) & ~7) | 2^254
 * 3. Q = xG is computed.
 * 3. A's public key is Q; A's private key is x.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     Curve25519.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     Curve25519.
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the keyblob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the keyblob
 *     authentication. The exact same authentication data must be provided
 *     when generating and importing a keyblob.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key must be returned.
 *
 * @param [out] PrivKeyBlob_p
 *     Pointer to the keyblob buffer.
 *     Note: If KeyBlob_p == NULL and KeyBlobSize_p != NULL the required
 *           size will be returned with as return value VAL_BUFFER_TOO_SMALL.
 *
 * @param [in,out] PrivKeyBlobSize_p
 *     Size of the buffer intended for the keyblob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519GenKeyPair(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const ValAssetId_Optional_t KekAssetId,
        ValOctetsIn_Optional_t * const AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValAsymBigInt_t * const PubKeyValue_p,
        ValOctetsOut_Optional_t * const PrivKeyBlob_p,
        ValSize_t * const PrivKeyBlobSize_p);


/*-----------------------------------------------------------------------------
 * val_AsymEccGenPublicKey
 */
/**
 * Generate an ECDH, ECDSA or ECC El-Gamal public key.
 *
 * An entity 'A's public key pair is associated with a particular set of
 * elliptic curve domain parameters, T = (p; a; b; G; n; h) and the entity
 * 'A's private key (d).
 *
 * To generate a public key, entity 'A' does the following:
 *
 * 1. Q = dG is computed.
 *
 * The maximum modulus value of PublicKey can be 521.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @param [out] PubKeyPoint_p
 *     Pointer to the buffer in which the public key (an ECC point) must be
 *     returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccGenPublicKey(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymECCPoint_t * PubKeyPoint_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhGenPublicKey
 */
/**
 * Generate a DH (Key Exchange) public key as defined in the standard
 * NIST SP800-56A.
 *
 * An entity 'A's public key pair is associated with a particular set of
 * domain parameters, T = (p; q; g) and the entity 'A's private key (x).
 *
 * To generate a public key, entity 'A' does the following:
 *
 * 1. y = g^x mod p.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     DSA.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     DSA.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key (an big integer) must be
 *     returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhGenPublicKey(
    ValAsymKey_t * const PubKey_p,
    ValAsymKey_t * const PrivKey_p,
    ValAsymBigInt_t * PubKeyValue_p);

/*-----------------------------------------------------------------------------
 * val_AsymEddsaGenPublicKey
 */
/**
 * Generate a EdDSA public key as defined in:
 * - 'Edwards-curve Digital Signature Algorithm (EdDSA)'
 *   draft-irtf-cfrg-eddsa-08
 *
 * The public key is generated as follows:
 *
 * The public and private key pair is associated with a particular set of
 * elliptic curve domain parameters, T = (p; d(=a); const(=b); G; n; h).
 *
 * The Public key (y) is calculated according following steps:
 *   1. Hash private key -> H = SHA-512(private key)
 *   2. Derive k (prune hash) -> k = (H & (2^(256) -1) & ~(2^255 | 7)) | 2^254
 *   3. Perform a fixed-base scalar multiplication -> Y = [k]G
 *   4. Encode point to get the public key -> y = (Y.y % p) | ((Y.x & 1) << 254)
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     EdDSA.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     EdDSA.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key must be returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEddsaGenPublicKey(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymBigInt_t * const PubKeyValue_p);

/*-----------------------------------------------------------------------------
 * val_AsymCurve25519GenPublicKey
 */
/**
 * Generate a Curve25519 public key.
 *
 * An entity 'A's public key pair is associated with a particular set of
 * elliptic curve domain parameters, T = (p; a; b; G; n; h) and the entity
 * 'A's private key (x).
 *
 * To generate a public key, entity 'A' does the following:
 *
 * 1. Q = xG is computed.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     Curve25519.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     Curve25519.
 *
 * @param [out] PubKeyValue_p
 *     Pointer to the buffer in which the public key must be returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519GenPublicKey(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymBigInt_t * const PubKeyValue_p);


/*-----------------------------------------------------------------------------
 * val_AsymEccKeyCheck
 */
/**
 * This function checks the provided keys are a pair or if only the public
 * key is given a point on curve check is performed only.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH, ECDSA or ECC El-Gamal.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccKeyCheck(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p);

/*-----------------------------------------------------------------------------
 * val_AsymDhDsaKeyCheck
 */
/**
 * This function checks the provided keys are a pair or if only one
 * key (private or public) is given, the basic checks are performed.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for DH
 *     or DSA.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for DH
 *     or DSA.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhDsaKeyCheck(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p);


/*-----------------------------------------------------------------------------
 * val_AsymEcdhGenSharedSecretSingle
 */
/**
 * This function generates the shared secret based on a single key pair for the
 * Key Agreement Schemes as defined in NIST SP800-56A.
 * - 'Ephemeral Unified Model' = C(2e, 0s, ECC CDH)
 * - 'One-Pass Diffie-Hellman' = C(1e, 1s, ECC CDH)
 * - 'Static Unified Model' = C(0e, 2s, ECC CDH)
 *   Note that NonceU must be embedded in the 'OtherInfo' string (OtherInfo_p).
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified in NIST SP800-56A section 5.8.1 ('Single-step Key-Derivation
 * Function') or saved to be used to perform a key derivation at a later stage.
 * The NIST SP800-56A KDF uses the SHA-256 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the string (label) used by the Key Derivation Function.
 *
 * @param [in] OtherInfoSize
 *     Size of the string (label) used by the Key Derivation Function.
 *
 * @param [in] AssetIdList_p
 *     Pointer to the list of target Assets to fill.
 *
 * @param [in] NumberOfAssets
 *     The number of target Assets to fill.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdhGenSharedSecretSingle(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);

/*-----------------------------------------------------------------------------
 * val_AsymEcdhGenSharedSecretDual
 */
/**
 * This function generates the shared secret based on a dual key pair for the
 * Key Agreement Schemes as defined in NIST SP800-56A.
 * - 'Full Unified Model' = C(2e, 2s, ECC CDH)
 * - 'One-Pass Unified Model' = C(1e, 2s, ECC CDH)
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified in NIST SP800-56A section 5.8.1 ('Single-step Key-Derivation
 * Function') or saved to be used to perform a key derivation at a later stage.
 * The NIST SP800-56A KDF uses the SHA-256 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH.
 *
 * @param [in] PubKey2_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECDH.
 *
 * @param [in] PrivKey2_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECDH.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the string (label) used by the Key Derivation Function.
 *
 * @param [in] OtherInfoSize
 *     Size of the string (label) used by the Key Derivation Function.
 *
 * @param [in] AssetIdList_p
 *     Pointer to the list of target Assets to fill.
 *
 * @param [in] NumberOfAssets
 *     The number of target Assets to fill.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEcdhGenSharedSecretDual(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymKey_t * const PubKey2_p,
        ValAsymKey_t * const PrivKey2_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);


/*-----------------------------------------------------------------------------
 * val_AsymDhGenSharedSecretSingle
 */
/**
 * This function generates the shared secret based on a single key pair for the
 * Key Agreement Schemes as defined in NIST SP800-56A.
 * - dhEphem   C(2e, 0s, FFC DH)
 * - dhOneFlow C(1e, 1s, FFC DH)
 * - dhStatic  C(0e, 2s, FFC DH)
 *   Note that NonceU must be embedded in the 'OtherInfo' string (OtherInfo_p).
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified in NIST SP800-56A section 5.8.1 ('Single-step Key-Derivation
 * Function') or saved to be used to perform a key derivation at a later stage.
 * The NIST SP800-56A KDF uses the SHA-256 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     DH.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     DH.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the string (label) used by the Key Derivation Function.
 *
 * @param [in] OtherInfoSize
 *     Size of the string (label) used by the Key Derivation Function.
 *
 * @param [in] AssetIdList_p
 *     Pointer to the list of target Assets to fill.
 *
 * @param [in] NumberOfAssets
 *     The number of target Assets to fill.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhGenSharedSecretSingle(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);

/*-----------------------------------------------------------------------------
 * val_AsymDhGenSharedSecretDual
 */
/**
 * This function generates the shared secret based on a dual key pair for the
 * Key Agreement Schemes as defined in NIST SP800-56A.
 * - dhHybrid1       C(2e, 2s, FFC DH)
 * - dhHybridOneFlow C(1e, 2s, FFC DH)
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified in NIST SP800-56A section 5.8.1 ('Single-step Key-Derivation
 * Function') or saved to be used to perform a key derivation at a later stage.
 * The NIST SP800-56A KDF uses the SHA-256 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for DH.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     DH.
 *
 * @param [in] PubKey2_p
 *     Pointer to the asymmetric crypto key structure of the public key for DH.
 *
 * @param [in] PrivKey2_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     DH.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the string (label) used by the Key Derivation Function.
 *
 * @param [in] OtherInfoSize
 *     Size of the string (label) used by the Key Derivation Function.
 *
 * @param [in] AssetIdList_p
 *     Pointer to the list of target Assets to fill.
 *
 * @param [in] NumberOfAssets
 *     The number of target Assets to fill.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymDhGenSharedSecretDual(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        ValAsymKey_t * const PubKey2_p,
        ValAsymKey_t * const PrivKey2_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);


/*-----------------------------------------------------------------------------
 * val_AsymCurve25519GenSharedSecret
 */
/**
 * This function generates the shared secret as defined in RFC 7748 "Elliptic
 * Curves for Security" and Curve25519 "New Diffie-Hellman speed records".
 *
 * The generated shared secret can either be used as key in the KDF as
 * specified in NIST SP800-56A section 5.8.1 ('Single-step Key-Derivation
 * Function') or saved to be used to perform a key derivation at a later stage.
 * The NIST SP800-56A KDF uses the SHA-256 hash algorithm and distributes the
 * generated data over each target Asset starting at a 4-Byte boundary. When
 * the shared secret is saved another key derivation scheme could be used.
 *
 * @param [in] PubKey_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     Curve25519.
 *
 * @param [in] PrivKey_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     Curve25519.
 *
 * @param [in] fSaveSharedSecret
 *     Indication that the shared secret must be saved in an Asset rather than
 *     used for the key derivation to initialize the provided target Assets.
 *     In case fSaveSharedSecret=true the number of target Assets shall be 1.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the the 'OtherInfo' string used by the Key Derivation
 *     Function.
 *
 * @param [in] OtherInfoSize
 *     The 'OtherInfo' string  length.
 *
 * @param [in] AssetIdList_p
 *     Pointer to a list of Asset Id's that need to be loaded.
 *
 * @param [in] NumberOfAssets
 *     The number of Assets in the list.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymCurve25519GenSharedSecret(
        ValAsymKey_t * const PubKey_p,
        ValAsymKey_t * const PrivKey_p,
        const bool fSaveSharedSecret,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAssetId_t * const AssetIdList_p,
        const ValSize_t NumberOfAssets);


/*-----------------------------------------------------------------------------
 * val_AsymEccElGamalEncrypt
 */
/**
 * This function is used for encrypting a message point. The encryption is
 * done according the ECC El-Gamal en/decryption scheme.
 * The supported ECC curves vary from 192 bits to 521 bits.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECC El-Gamal.
 *
 * @param [in] MessagePoint_p
 *     Pointer to the message point that must be encrypted.
 *
 * @param [out] EncryptedPoint1_p
 *     Pointer to the buffer in which the encrypted message point part 1 must
 *     be returned.
 *
 * @param [out] EncryptedPoint2_p
 *     Pointer to the buffer in which the encrypted message point part 2 must
 *     be returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccElGamalEncrypt(
        ValAsymKey_t * const KeyContext_p,
        ValAsymECCPoint_t * const MessagePoint_p,
        ValAsymECCPoint_t * const EncryptedPoint1_p,
        ValAsymECCPoint_t * const EncryptedPoint2_p);

/*-----------------------------------------------------------------------------
 * val_AsymEccElGamalDecrypt
 */
/**
 * This function is used for decrypting an encrypted message point. The
 * decryption is done according the ECC El-Gamal en/decryption scheme.
 * The supported ECC curves vary from 192 bits to 521 bits.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the private key for
 *     ECC El-Gamal.
 *
 * @param [in] EncryptedPoint1_p
 *     Pointer to the encrypted message point part 1 that must be decrypted.
 *
 * @param [in] EncryptedPoint2_p
 *     Pointer to the encrypted message point part 2 that must be decrypted.
 *
 * @param [out] MessagePoint_p
 *     Pointer to the buffer in which the decrypted message point must be
 *     returned.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymEccElGamalDecrypt(
        ValAsymKey_t * const KeyContext_p,
        ValAsymECCPoint_t * const EncryptedPoint1_p,
        ValAsymECCPoint_t * const EncryptedPoint2_p,
        ValAsymECCPoint_t * const MessagePoint_p);


/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepAllocPrivateKeyAsset
 */
/**
 * This function allocates a RSA OAEP private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepAllocPublicKeyAsset
 */
/**
 * This function allocates a RSA OAEP public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] HashAlgorithm
 *     The related hash algorithm to use.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const ValSymAlgo_t HashAlgorithm,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);


/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepWrapString
 */
/**
 * This function wraps an Asset using the RSA-OAEP method, as defined in NIST
 * SP800-56B. This function uses a string as Additional Input.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-OAEP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [in] AdditionalInput_p
 *     Pointer to the string to use as Additional Input.
 *
 * @param [in] AdditionalInputSize
 *     The size of the string to use as Additional Input.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepWrapString(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const AdditionalInput_p,
        const ValSize_t AdditionalInputSize,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);

/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepWrapHash
 */
/**
 * This function wraps an Asset using the RSA-OAEP method, as defined in NIST
 * SP800-56B. This function uses a hash digest as Additional Input.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-OAEP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [in] AdditionalInput_p
 *     Pointer to the hash digest to use as Additional Input.
 *
 * @param [in] AdditionalInputSize
 *     The size of the hash digest to use as Additional Input.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepWrapHash(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const AdditionalInput_p,
        const ValSize_t AdditionalInputSize,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);

/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepUnwrapString
 */
/**
 * This function unwraps RSA-OAEP wrapped data and loads (initializes) an Asset
 * with it. The RSA-OAEP method as defined in NIST SP800-56B. This function
 * uses a string as Additional Input.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-OAEP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [in] AdditionalInput_p
 *     Pointer to the string to use as Additional Input.
 *
 * @param [in] AdditionalInputSize
 *     The size of the string to use as Additional Input.
 *
 * @param [in] WrappedData_p
 *     Pointer to the buffer with the wrapped data for the Asset.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data for the Asset.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepUnwrapString(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const AdditionalInput_p,
        const ValSize_t AdditionalInputSize,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);

/*-----------------------------------------------------------------------------
 * val_AsymRsaOaepUnwrapHash
 */
/**
 * This function unwraps RSA-OAEP wrapped data and loads (initializes) an Asset
 * with it. The RSA-OAEP method as defined in NIST SP800-56B. This function
 * uses a hash digest as Additional Input.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-OAEP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [in] AdditionalInput_p
 *     Pointer to the hash digest to use as Additional Input.
 *
 * @param [in] AdditionalInputSize
 *     The size of the hash digest to use as Additional Input.
 *
 * @param [in] WrappedData_p
 *     Pointer to the buffer with the wrapped data for the Asset.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data for the Asset.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaOaepUnwrapHash(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const AdditionalInput_p,
        const ValSize_t AdditionalInputSize,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);


/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15WrapAllocPrivateKeyAsset
 */
/**
 * This function allocates a RSAES-PKCS1-v1_5 private key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [in] fAllowAny
 *     When this flag is set, the data may be stored in public (data) and
 *     exportable Assets otherwise it is prevented
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15WrapAllocPrivateKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        const bool fAllowAny,
        ValAssetId_t * const AssetId_p);

/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15WrapAllocPublicKeyAsset
 */
/**
 * This function allocates a RSAES-PKCS1-v1_5 public key Asset.
 *
 * @param [in] ModulusSizeBits
 *     Size of the modulus in bits.
 *
 * @param [in] ExponentSizeBits
 *     Size of the exponent in bits.
 *
 * @param [in] fCrossDomain
 *     Indication to allow cross domain use.
 *
 * @param [in] fExportAllowed
 *     Indication to allow asset export.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId will be written on
 *     success.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15WrapAllocPublicKeyAsset(
        const ValSize_t ModulusSizeBits,
        const ValSize_t ExponentSizeBits,
        const bool fCrossDomain,
        const bool fExportAllowed,
        ValAssetId_t * const AssetId_p);


/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15Wrap
 */
/**
 * This function wraps an Asset using the RSAES-PKCS1-v1_5 encryption scheme,
 * as defined in PKCS#1.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-PKCS1-V15-WRAP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15Wrap(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);


/*-----------------------------------------------------------------------------
 * val_AsymRsaPkcs1v15Unwrap
 */
/**
 * This function unwraps RSAES-PKCS1-v1_5 wrapped data and loads (initializes)
 * an Asset with it. The RSAES-PKCS1-v1_5 encryption scheme is defined in PKCS#1.
 *
 * @param [in] KeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     RSA-PKCS1-V15-WRAP.
 *
 * @param [in] AssetId
 *     Reference to the Asset to wrap.
 *
 * @param [in] WrappedData_p
 *     Pointer to the buffer with the wrapped data for the Asset.
 *
 * @param [in] WrappedDataSize
 *     The size of the wrapped data for the Asset.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymRsaPkcs1v15Unwrap(
        ValAsymKey_t * const KeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);


/*-----------------------------------------------------------------------------
 * val_AsymECIESWrap
 */
/**
 *
 * This function wraps an Asset using the ECIES scheme
 *
 * @param [in] PubKeyContext_p
 *     Pointer to the asymmetric crypto key structure of the public key for
 *     ECIES-WRAP. (contains PK-Parameters asset and DH-KEYEXCH public key asset)
 *
 * @param [in] AssetId
 *     Reference to the Asset containing the data to encrypt/wrap.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the buffer with the 'other info' string used by the
 *     Key Derivation Function. This data block may have a zero length.
 *
 * @param [in] OtherInfoSize
 *     The size of the InputData_p buffer.
 *
 * @param [out] EphemeralPubkey_p
 *     Pointer to the buffer for storing the public key result sub-vectors
 *     'pubkey.x' and 'pubkey.y'.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer in which the wrapped data needs to be written.
 *
 * @param [in] WrappedDataSize
 *     The size of the WrappedData_p buffer.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymECIESWrap(
        ValAsymKey_t * const PubKeyContext_p,
        const ValAssetId_t AssetId,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValAsymECCPoint_t * const EphemeralPubkey_p,
        ValOctetsOut_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);


/*-----------------------------------------------------------------------------
 * val_AsymECIESUnwrap
 */
/**
 *
 * This function unwraps an ECIES-WRAP cryptogram to an Asset.
 *
 * @param [in] PrivKeyContext_p
 *     Pointer to the asymmetric crypto key structure containing the
 *     DH-KEYECH private key asset containing the 'privkey' sub-vector.
 *     Also contains the PK-Parameters curve parameters asset.
 *
 * @param [in] AssetId
 *     Asset in which the decrypted/unwrapped data shall be stored.
 *
 * @param [in] EphemeralPubkeyAssetId
 *     AssetId of Asset, which contains the DH-KEYEXCH public key that was
 *     returned by the ECIES-Wrap operation.
 *
 * @param [in] OtherInfo_p
 *     Pointer to the buffer with the 'other info' string used by the
 *     Key Derivation Function. This data block may have a zero length.
 *
 * @param [in] OtherInfoSize
 *     The size of the InputData_p buffer.
 *
 * @param [out] WrappedData_p
 *     Pointer to the buffer that holds the encrypted/wrapped data.
 *
 * @param [in] WrappedDataSize
 *     Size of the wrapped data
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymECIESUnwrap(
        ValAsymKey_t * const PrivKeyContext_p,
        const ValAssetId_t AssetId,
        const ValAssetId_t EphemeralPubkeyAssetId,
        ValOctetsIn_Optional_t * const OtherInfo_p,
        const ValSize_t OtherInfoSize,
        ValOctetsIn_t * const WrappedData_p,
        const ValSize_t WrappedDataSize);


/*-----------------------------------------------------------------------------
 * val_AsymPkaClaim
 */
/**
 * This function claims the PKA (PKCP) for a sequence of operations.
 *
 * @param [in] Nwords
 *     Number of 32-bit words, which specifies a default vector size.
 *
 * @param [in] Mwords
 *     Number of 32-bit words, which specifies an alternate vector size.
 *
 * @param [in] Mmask
 *     Bitmask specifying (with 1b's) which of the first 8 vectors
 *     that are loaded via val_AsymPkaLoadVector() are Mwords.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymPkaClaim(
        const uint8_t Nwords,
        const uint8_t Mwords,
        const uint8_t Mmask);

/*-----------------------------------------------------------------------------
 * val_AsymPkaLoadVector
 */
/**
 * This function loads a PKA vector at the specified location in PKA memory.
 *
 * @param [in] Index
 *     The location index where the vector must be loaded [0..15].
 *
 * @param [in] Vector_p
 *     Pointer to the vector data.
 *
 * @param [in] VectorSize
 *     The size of the vector data in bytes.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymPkaLoadVector(
        const uint8_t Index,
        ValOctetsIn_t * const Vector_p,
        const ValSize_t VectorSize);

/*-----------------------------------------------------------------------------
 * val_AsymPkaExecuteOperation
 */
/**
 * This function claims the PKA (PKCP) for a sequence of operations.
 *
 * @param [in] Operation
 *     PKA operation to execute.
 *
 * @param [in] PublicExponent
 *     Public Exponent, only applicable for the ModExpE operation.
 *
 * @param [in] InData_p
 *     Pointer to the input data.
 *
 * @param [in] InDataSize
 *     The size of the input data in bytes.
 *
 * @param [out] OutData_p
 *     Pointer to the buffer, where the output data must be written
 *
 * @param [in,out] OutDataSize_p
 *     The size of the output data buffer in bytes. Note that the size will be
 *     updated.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymPkaExecuteOperation(
        const ValAsymPkaOperation_t Operation,
        const uint32_t PublicExponent,
        ValOctetsIn_t * const InData_p,
        const ValSize_t InDataSize,
        ValOctetsOut_t * const OutData_p,
        ValSize_t * const OutDataSize_p);

/*-----------------------------------------------------------------------------
 * val_AsymPkaClaimRelease
 */
/**
 * This function releases the PKA (PKCP) claim.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_AsymPkaClaimRelease(void);


#endif /* INCLUDE_GUARD_API_VAL_ASYM_H */

/* end of file api_val_asym.h */
