/* cs_test_val.h
 *
 * Configuration Settings for the VAL API test tool.
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

#ifndef INCLUDE_GUARD_CS_TEST_VAL_TEST_H
#define INCLUDE_GUARD_CS_TEST_VAL_TEST_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "cs_adapter_val.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
// Enable debug logging
#ifdef LOG_SEVERITY_MAX
#undef LOG_SEVERITY_MAX
#endif
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRIT
//#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
//#define LOG_SEVERITY_MAX  LOG_SEVERITY_INFO

/*----------------------------------------------------------------------------
 * Hardware and firmware configuration specifics
 */
/* TRNG HW behavior */
#ifdef EIP130_HW_V2
#define VALTEST_TRNG_HW_4K          /* 4K Window */
//#define VALTEST_TRNG_HW_512         /* 512 Window */
#else
#define VALTEST_TRNG_HW_512         /* 512 Window */
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SHA512
#define VALTEST_SYM_ALGO_SHA512
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SHA3
#define VALTEST_SYM_ALGO_SHA3
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SM3
#define VALTEST_SYM_ALGO_SM3
#endif

#ifndef VAL_REMOVE_SYM_ALGO_AES_XTS
#define VALTEST_SYM_ALGO_AES_XTS
#endif
#ifndef VAL_REMOVE_SYM_ALGO_AES_CCM
#define VALTEST_SYM_ALGO_AES_CCM
#endif
#ifndef VAL_REMOVE_SYM_ALGO_AES_GCM
#define VALTEST_SYM_ALGO_AES_GCM
#endif
#ifndef VAL_REMOVE_SYM_ALGO_AES_F8
#define VALTEST_SYM_ALGO_AES_F8
#endif

#ifndef VAL_REMOVE_SYM_ALGO_DES
#define VALTEST_SYM_ALGO_DES
#endif
#ifndef VAL_REMOVE_SYM_ALGO_3DES
#define VALTEST_SYM_ALGO_3DES
#endif

#ifndef VAL_REMOVE_SYM_ALGO_CHACHA20
#define VALTEST_SYM_ALGO_CHACHA20
#endif
#ifndef VAL_REMOVE_SYM_ALGO_POLY1305
#define VALTEST_SYM_ALGO_POLY1305
#endif

#ifndef VAL_REMOVE_SYM_ALGO_SM4
#define VALTEST_SYM_ALGO_SM4
#endif
#ifndef VAL_REMOVE_SYM_ALGO_SM4_GCM
#define VALTEST_SYM_ALGO_SM4_GCM
#endif

#ifndef VAL_REMOVE_SYM_ALGO_ARIA
#define VALTEST_SYM_ALGO_ARIA
#endif
#ifndef VAL_REMOVE_SYM_ALGO_ARIA_CCM
#define VALTEST_SYM_ALGO_ARIA_CCM
#endif
#ifndef VAL_REMOVE_SYM_ALGO_ARIA_GCM
#define VALTEST_SYM_ALGO_ARIA_GCM
#endif

#if !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_DECRYPT) && \
    !defined(VAL_REMOVE_ASYM_ECC_ELGAMAL_ENCRYPT)
#define VALTEST_ASYM_ECC_ELGAMAL
#endif

#ifndef VAL_REMOVE_ASYM_CURVE25519
#define VALTEST_ASYM_CURVE25519
#endif

#if !defined(VAL_REMOVE_ASYM_EDDSA_SIGN) && \
    !defined(VAL_REMOVE_ASYM_EDDSA_VERIFY)
#define VALTEST_ASYM_EDDSA
#endif

#if !defined(VAL_REMOVE_ASYM_SM2DSA_SIGN) && \
    !defined(VAL_REMOVE_ASYM_SM2DSA_VERIFY) && \
    !defined(VAL_REMOVE_ASYM_SM2KEX_GEN_SHARED_SECRET) && \
    !defined(VAL_REMOVE_ASYM_SM2ENC)
#define VALTEST_ASYM_SM2
#endif

#ifndef VAL_REMOVE_ASYM_ECIES
#define VALTEST_ECIES
#endif

#ifndef VAL_REMOVE_SYSTEM_ENTERFIPSMODE
#define VALTEST_SYSTEM_ENTERFIPSMODE
#endif

#ifndef VAL_REMOVE_ASSET_EXPORT_VIA_CPIF
#define VALTEST_ASSET_EXPORT_VIA_CPIF
#endif

#ifndef VAL_REMOVE_SF_MILENAGE
#define VALTEST_SF_MILENAGE
#endif

/*----------------------------------------------------------------------------
 * VALTEST_RANDOM_VERYLONG
 * Defines that very long random number generation is allowed
 */
//#define VALTEST_RANDOM_VERYLONG

/*----------------------------------------------------------------------------
 * VALTEST_TRACE_NOP_DATA
 * Defines that the NOP result must be trace
 */
//#define VALTEST_TRACE_NOP_DATA


#endif /* INCLUDE_GUARD_CS_TEST_VAL_TEST_H */

/* end of file cs_test_val.h */
