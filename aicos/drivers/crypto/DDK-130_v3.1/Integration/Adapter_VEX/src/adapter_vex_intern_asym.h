/* adapter_vex_intern_asym.h
 *
 * VEX API: Internal interfaces and definitions specific for Asymmetric Crypto.
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

#ifndef INCLUDE_GUARD_ADAPTER_VEX_INTERN_ASYM_H
#define INCLUDE_GUARD_ADAPTER_VEX_INTERN_ASYM_H

#include "adapter_vex_internal.h"   // Generic internal interfaces and definitions


/*----------------------------------------------------------------------------
 * vex_Asym_AssetSign
 *
 * This function handles the signature generate operation for ECDSA, DSA and
 * RSA based on a token using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetSign(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetVerify
 *
 * This function handles the signature verify operation for ECDSA, DSA and
 * RSA based on a token using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetVerify(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetEncrypt
 *
 * This function handles an asymmetric encrypt or decrypt operation based on
 * a token using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetEncrypt(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetGenKeyPair
 *
 * This function handles the key pair generation operation based on a token
 * using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetGenKeyPair(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetGenKeyPublic
 *
 * This function handles the public key generation operation based on a token
 * using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetGenKeyPublic(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetKeyCheck
 *
 * This function handles the key check operation for ECDH/ECDSA based on
 * a token using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetKeyCheck(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetGenSharedSecret
 *
 * This function handles the generation of shared secret(s) operation based on
 * a token using Assets.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetGenSharedSecret(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetKeyWrap
 *
 * This function handles the key wrap and unwrap functionality for RSA-OAEP.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetKeyWrap(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_AssetEcies
 *
 * This function handles encryption / decryption of Asset data based
 * on the ECIES scheme.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_AssetEcies(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);


/*----------------------------------------------------------------------------
 * vex_Asym_PkaNumSet
 *
 * This function handles the claim and release of PKA/PkCP for an operation.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_PkaNumSet(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_PkaNumLoad
 *
 * This function handles loading of a PKA vector.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_PkaNumLoad(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);

/*----------------------------------------------------------------------------
 * vex_Asym_PkaOperation
 *
 * This function handles the execution of an PKA/PKCP operation.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_Asym_PkaOperation(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);


#endif /* INCLUDE_GUARD_ADAPTER_VEX_INTERN_ASYM_H */

/* end of file adapter_vex_intern_asym.h */
