/**
 * @file api_val_specfuncs.h
 *
 * @brief VAL API - Special Functions related services
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

#ifndef INCLUDE_GUARD_API_VAL_SPECFUNCS_H
#define INCLUDE_GUARD_API_VAL_SPECFUNCS_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "api_val_asset.h"          // ValAssetId_t, val_AssetFree


/*-----------------------------------------------------------------------------
 * val_SFMilenageSqnAdminCreate
 */
/**
 * This function allocates and initializes (creates) a SQNms Administration
 * Asset that must be used for the val_SFMilenageAutnVerificationSqn function
 * to enable sequence number checking according 3GPP TS 33.102.
 *
 * @param [in] StaticAssetNumber
 *     The asset number to search for in the (static) Asset Store that refers
 *     to Milenage private data that contains the 128-bit subscriber key (K)
 *     and the 128-bit value derived from OP and K (OPc).
 *     Note that the supported values for StaticAssetNumber are defined by the
 *     device manufacturer and vendor and can differ from product to product.
 *
 * @param [in] fAMF_SBtest
 *     When set, the AMF (Authenication Management Field) Separation Bit test
 *     is enable when performing the sequence number check.
 *
 * @param [out] AssetId_p
 *     Pointer to the memory location where the AssetId for SQNms Administration
 *     will be written in case of successful creation. The AssetId is needed
 *     for the AUTN Verification operation when sequence number checking is
 *     required.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageSqnAdminCreate(
        const ValAssetNumber_t StaticAssetNumber,
        const bool fAMF_SBtest,
        ValAssetId_t * const AssetId_p);


/*-----------------------------------------------------------------------------
 * val_SFMilenageSqnAdminReset
 */
/**
 * This function resets the SQNms Administration to its initial state when it
 * was created.
 *
 * @param [in] AssetId
 *     Reference to the SQNms Administration Asset.
 *     This reference is provided by val_SFMilenageSqnAdminCreate().
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageSqnAdminReset(
        const ValAssetId_t AssetId);


/*-----------------------------------------------------------------------------
 * val_SFMilenageSqnAdminExport
 */
/**
 * This function exports the SQNms Administration Asset as a data blob that
 * can be load via val_AssetLoadImport() to restore the the SQNms Administration
 * after a hibernation or a device power cycle.
 * Note that val_AssetAlloc() must be used to create a SQNms Administration
 * Asset (Size: 200 Bytes; Policy: Private Data) before val_AssetLoadImport()
 * can be used.
 *
 * @param [in] AssetId
 *     Reference to the SQNms Administration Asset.
 *     This reference is provided by val_SFMilenageSqnAdminCreate().
 *
 * @param [in] KekAssetId
 *     Reference to the KEK Asset to be used when generating the data blob.
 *
 * @param [in] AssociatedData_p
 * @param [in] AssociatedDataSize
 *     Pointer and number of bytes for the Associated Data used in the data
 *     blob authentication. The exact same Associated Data must be provided
 *     when data blob is imported
 *
 * @param [out] DataBlob_p
 *     Pointer to the data blob buffer.
 *
 * @param [in,out] SizeDataBlob_p
 *     Size of the buffer intended for the data blob.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageSqnAdminExport(
        const ValAssetId_t AssetId,
        const ValAssetId_t KekAssetId,
        ValOctetsIn_t * const  AssociatedData_p,
        const ValSize_t AssociatedDataSize,
        ValOctetsOut_t * const DataBlob_p,
        ValSize_t * const SizeDataBlob_p);


/*-----------------------------------------------------------------------------
 * val_SFMilenageSqnAdminRelease
 */
/**
 * This function releases (frees) the SQNms Administration asset.
 *
 * @param [in] AssetId
 *     Reference to the SQNms Administration Asset.
 *     This reference is provided by val_SFMilenageSqnAdminCreate().
 *
 * @return One of the ValStatus_t values.
 */
#define val_SFMilenageSqnAdminRelease(AssetId) val_AssetFree(AssetId)


/*-----------------------------------------------------------------------------
 * val_SFMilenageAutnVerification
 */
/**
 * This function verifies the provided AUTN and returns RES, CK, IK, SQN and
 * AMF on success. In case of a MAC failure 'Verify Error' is returned and RES,
 * CK, IK, SQN and AMF are invalid.
 *
 * @param [in] StaticAssetNumber
 *     The asset number to search for in the (static) Asset Store that refers
 *     to Milenage private data that contains the 128-bit subscriber key (K)
 *     and the 128-bit value derived from OP and K (OPc).
 *     Note that the supported values for StaticAssetNumber are defined by the
 *     device manufacturer and vendor and can differ from product to product.
 *
 * @param [in] RAND_p
 *     Pointer to the 128-bit (16 Bytes) RAND.
 *
 * @param [in] AUTN_p
 *     Pointer to the 128-bit (16 Bytes) AUTN.
 *
 * @param [out] RES_p
 *     Pointer to the 64-bit (8 Bytes) buffer in which the RES must be written.
 *
 * @param [out] CK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the CK must be written.
 *
 * @param [out] IK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the IK must be written.
 *
 * @param [out] SQN_p
 *     Pointer to the 48-bit (6 Bytes) buffer in which the SQN must be written.
 *
 * @param [out] AMF_p
 *     Pointer to the 16-bit (2 Bytes) buffer in which the AMF must be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageAutnVerification(
        const ValAssetNumber_t StaticAssetNumber,
        ValOctetsIn_t * const RAND_p,
        ValOctetsIn_t * const AUTN_p,
        ValOctetsOut_t * const RES_p,
        ValOctetsOut_t * const CK_p,
        ValOctetsOut_t * const IK_p,
        ValOctetsOut_t * const SQN_p,
        ValOctetsOut_t * const AMF_p);


/*-----------------------------------------------------------------------------
 * val_SFMilenageAutnVerificationSqn
 */
/**
 * This function verifies the provided AUTN and returns RES, CK, IK on success.
 * The function returns 'Verify Error' in case of:
 * - MAC failure
 *   -> EMM Cause 20
 * - Synch failure
 *   -> EMM Cause 21 and an AUTS is returned based on SQNms and AMF(= {0,0})
 * - AMF Separation Bit failure - Non-EPS authentication unacceptable
 *   -> EMM Cause 26
 *
 * @param [in] AssetId
 *     Reference to the SQNms Administration Asset.
 *     This reference is provided by val_SFMilenageSqnAdminCreate().
 *
 * @param [in] RAND_p
 *     Pointer to the 128-bit (16 Bytes) RAND.
 *
 * @param [in] AUTN_p
 *     Pointer to the 128-bit (16 Bytes) AUTN.
 *
 * @param [out] EMMCause_p
 *     Pointer to a location where the EMM Cause must be written in case a
 *     'Verify Error' occurs.
 *
 * @param [out] RES_p
 *     Pointer to the 64-bit (8 Bytes) buffer in which the RES must be written.
 *
 * @param [out] CK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the CK must be written.
 *
 * @param [out] IK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the IK must be written.
 *
 * @param [out] AUTS_p
 *     Pointer to the 96-bit (12 Bytes) buffer in which the AUTS must be written
 *     in case of Sync failure (EMM Cause 21).
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageAutnVerificationSqn(
        const ValAssetId_t AssetId,
        ValOctetsIn_t * const RAND_p,
        ValOctetsIn_t * const AUTN_p,
        uint32_t * const EMMCause_p,
        ValOctetsOut_t * const RES_p,
        ValOctetsOut_t * const CK_p,
        ValOctetsOut_t * const IK_p,
        ValOctetsOut_t * const AUTS_p);


/*-----------------------------------------------------------------------------
 * val_SFMilenageAutsGeneration
 */
/**
 * This function generates an AUTS based on the provided SQNms and AMF.
 *
 * @param [in] StaticAssetNumber
 *     The asset number to search for in the (static) Asset Store that refers
 *     to Milenage private data that contains the 128-bit subscriber key (K)
 *     and the 128-bit value derived from OP and K (OPc).
 *     Note that the supported values for StaticAssetNumber are defined by the
 *     device manufacturer and vendor and can differ from product to product.
 *
 * @param [in] RAND_p
 *     Pointer to the 128-bit (16 Bytes) RAND.
 *
 * @param [in] SQNms_p
 *     Pointer to the 48-bit (6 Bytes) buffer in which the SQNms must be written.
 *
 * @param [in] AMF_p
 *     Pointer to the 16-bit (2 Bytes) buffer in which the AMF must be written.
 *
 * @param [out] AUTS_p
 *     Pointer to the 96-bit (12 Bytes) buffer in which the AUTS must be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageAutsGeneration(
        const ValAssetNumber_t StaticAssetNumber,
        ValOctetsIn_t * const RAND_p,
        ValOctetsIn_t * const SQNms_p,
        ValOctetsIn_t * const AMF_p,
        ValOctetsOut_t * const AUTS_p);


/*-----------------------------------------------------------------------------
 * val_SFMilenageConformance
 */
/**
 * This function performance a conformance check with a test set as specified
 * in 3GPP TS 35.208.
 *
 * @param [in] RAND_p
 *     Pointer to the 128-bit (16 Bytes) RAND.
 *
 * @param [in] SQN_p
 *     Pointer to the 48-bit (6 Bytes) SQN.
 *
 * @param [in] AMF_p
 *     Pointer to the 16-bit (2 Bytes) AMF.
 *
 * @param [in] K_p
 *     Pointer to the 128-bit (16 Bytes) K.
 *
 * @param [in] OP_p
 *     Pointer to the 128-bit (16 Bytes) OP.
 *
 * @param [out] RES_p
 *     Pointer to the 64-bit (8 Bytes) buffer in which the RES must be written.
 *
 * @param [out] CK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the CK must be written.
 *
 * @param [out] IK_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the IK must be written.
 *
 * @param [out] MACA_p
 *     Pointer to the 64-bit (8 Bytes) buffer in which the MAC-A must be written.
 *
 * @param [out] MACS_p
 *     Pointer to the 64-bit (8 Bytes) buffer in which the MAC-S must be written.
 *
 * @param [out] AK_p
 *     Pointer to the 48-bit (6 Bytes) buffer in which the AK must be written.
 *
 * @param [out] AKstar_p
 *     Pointer to the 48-bit (6 Bytes) buffer in which the AK* must be written.
 *
 * @param [out] OPc_p
 *     Pointer to the 128-bit (16 Bytes) buffer in which the OPc must be written.
 *
 * @return One of the ValStatus_t values.
 */
ValStatus_t
val_SFMilenageConformance(
        ValOctetsIn_t * const RAND_p,
        ValOctetsIn_t * const SQN_p,
        ValOctetsIn_t * const AMF_p,
        ValOctetsIn_t * const K_p,
        ValOctetsIn_t * const OP_p,
        ValOctetsOut_t * const RES_p,
        ValOctetsOut_t * const CK_p,
        ValOctetsOut_t * const IK_p,
        ValOctetsOut_t * const MACA_p,
        ValOctetsOut_t * const MACS_p,
        ValOctetsOut_t * const AK_p,
        ValOctetsOut_t * const AKstar_p,
        ValOctetsOut_t * const OPc_p);


#endif /* INCLUDE_GUARD_API_VAL_SPECFUNCS_H */

/* end of file api_val_specfuncs.h */
