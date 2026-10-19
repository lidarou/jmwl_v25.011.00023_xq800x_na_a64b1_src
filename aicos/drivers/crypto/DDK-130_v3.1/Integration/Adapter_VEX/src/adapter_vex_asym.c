/* adapter_vex_asym.c
 *
 * Implementation of the VEX API.
 *
 * This file implements the main entry for asymmetric crypto services.
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

#include "c_adapter_vex.h"          // configuration

#include "basic_defs.h"
#include "clib.h"
#include "log.h"

#include "adapter_vex_intern_asym.h"   // API implementation


/*----------------------------------------------------------------------------
 * vex_Asym
 */
VexStatus_t
vex_Asym(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p)
{
    VexStatus_t funcres = VEX_UNSUPPORTED;

    switch (CommandToken_p->SubCode)
    {
    case VEXTOKEN_SUBCODE_PK_NOASSETS:
        switch (CommandToken_p->Service.PkOperation.Operation)
        {
        case VEXTOKEN_PK_OP_NUMLOAD:
            funcres = vex_Asym_PkaNumLoad(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PK_OP_NUMSETN:
            funcres = vex_Asym_PkaNumSet(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PK_OP_MODEXPE:
        case VEXTOKEN_PK_OP_MODEXPD:
        case VEXTOKEN_PK_OP_MODEXPCRT:
        case VEXTOKEN_PK_OP_ECMONTMUL:
        case VEXTOKEN_PK_OP_ECCMUL:
        case VEXTOKEN_PK_OP_ECCADD:
        case VEXTOKEN_PK_OP_DSASIGN:
        case VEXTOKEN_PK_OP_DSAVERIFY:
        case VEXTOKEN_PK_OP_ECDSASIGN:
        case VEXTOKEN_PK_OP_ECDSAVERIFY:
            funcres = vex_Asym_PkaOperation(CommandToken_p, ResultToken_p);
            break;

        default:
            break;
        }
        break;

    case VEXTOKEN_SUBCODE_PK_WITHASSETS:
        switch (CommandToken_p->Service.PkAssetSignVerify.Method)
        {
        case VEXTOKEN_PKASSET_ECDH_ECDSA_KEYCHK:
        case VEXTOKEN_PKASSET_DH_DSA_KEYCHK:
            funcres = vex_Asym_AssetKeyCheck(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PKASSET_DSA_SIGN:
        case VEXTOKEN_PKASSET_ECDSA_SIGN:
        case VEXTOKEN_PKASSET_SM2DSA_SIGN:
        case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN:
        case VEXTOKEN_PKASSET_RSA_PSS_SIGN:
        case VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL:
        case VEXTOKEN_PKASSET_EDDSA_SIGN_UPDATE:
        case VEXTOKEN_PKASSET_EDDSA_SIGN_FINAL:
            funcres = vex_Asym_AssetSign(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PKASSET_DSA_VERIFY:
        case VEXTOKEN_PKASSET_ECDSA_VERIFY:
        case VEXTOKEN_PKASSET_SM2DSA_VERIFY:
        case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_VERIFY:
        case VEXTOKEN_PKASSET_RSA_PSS_VERIFY:
        case VEXTOKEN_PKASSET_EDDSA_VERIFY_INITIAL:
        case VEXTOKEN_PKASSET_EDDSA_VERIFY_FINAL:
            funcres = vex_Asym_AssetVerify(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PKASSET_DH_GEN_PUBKEY:
        case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_PUBKEY:
        case VEXTOKEN_PKASSET_CURVE25519_GEN_PUBKEY:
        case VEXTOKEN_PKASSET_EDDSA_GEN_PUBKEY:
            funcres = vex_Asym_AssetGenKeyPublic(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PKASSET_DH_GEN_KEYPAIR:
        case VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_KEYPAIR:
        case VEXTOKEN_PKASSET_CURVE25519_GEN_KEYPAIR:
        case VEXTOKEN_PKASSET_EDDSA_GEN_KEYPAIR:
            funcres = vex_Asym_AssetGenKeyPair(CommandToken_p, ResultToken_p);
            break;

#ifdef VEX_ENABLE_ASYM_ALGO_ECC_ELGAMAL
        case VEXTOKEN_PKASSET_ECC_ELGAMAL_ENCRYPT:
        case VEXTOKEN_PKASSET_ECC_ELGAMAL_DECRYPT:
            funcres = vex_Asym_AssetEncrypt(CommandToken_p, ResultToken_p);
            break;
#endif

        case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_SKEYPAIR:
        case VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_DKEYPAIR:
        case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_SKEYPAIR:
        case VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR:
        case VEXTOKEN_PKASSET_CURVE25519_GEN_SHARED_SECRET:
        case VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET:
            funcres = vex_Asym_AssetGenSharedSecret(CommandToken_p, ResultToken_p);
            break;

        case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING:
        case VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH:
        case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_STRING:
        case VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_HASH:
        case VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP:
        case VEXTOKEN_PKASSET_RSA_PKCS1V15_UNWRAP:
        case VEXTOKEN_PKASSET_SM2ENC_WRAP:
        case VEXTOKEN_PKASSET_SM2ENC_UNWRAP:
            funcres = vex_Asym_AssetKeyWrap(CommandToken_p, ResultToken_p);
            break;

#ifdef VEX_ENABLE_ASYM_ECIES
        case VEXTOKEN_PKASSET_ECIESWRAP:
        case VEXTOKEN_PKASSET_ECIESUNWRAP:
            funcres = vex_Asym_AssetEcies(CommandToken_p, ResultToken_p);
            break;
#endif

        case VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN_CRT:
        case VEXTOKEN_PKASSET_RSA_PSS_SIGN_CRT:
        default:
            break;
        }
        break;

    default:
        break;
    }

    return funcres;
}


/* end of file adapter_vex_asym.c */
