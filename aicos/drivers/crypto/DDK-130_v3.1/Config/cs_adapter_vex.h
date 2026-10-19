/** @file cs_adapter_vex.h
 *
 * @brief Configuration Settings for the VEX API driver.
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

#ifndef INCLUDE_GUARD_CS_ADAPTER_VEX_H
#define INCLUDE_GUARD_CS_ADAPTER_VEX_H

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
//#define VEX_STRICT_ARGS
#endif

/** Token tracing use */
//#define VEX_TRACE_TOKENS 1        /* Trace all tokens send to EIP-130 (first word only) */
//#define VEX_TRACE_TOKENS 2        /* Trace all tokens send to EIP-130 (all words) */
//#define VEX_TRACE_TOKENS_ERROR 1  /* Trace all tokens with a result error (first word only) */
//#define VEX_TRACE_TOKENS_ERROR 2  /* Trace all tokens with a result error (all words)  */

/** Defines the device name to use
 *  Please, look at the cs_hwpal_ext.h for the usable device names */
#define VEX_DEVICE_NAME                 DRIVER_DEVICE_NAME

/** Firmware load related functionality use */
#ifdef EIP130_ENABLE_FIRMWARE_LOAD
/** Defines that the firmware load must be enabled */
#define VEX_ENABLE_FIRMWARE_LOAD

/** Defines the firmware filename */
#define VEX_FIRMWARE_FILE               "firmware_eip130ram.sbi"
#endif

/** HW/FW specific settings */
/** Defines the mailbox to use */
#define VEX_MAILBOX_NR 1

/** Defines if the mailbox must be link only once, otherwise the mailbox will
 *  be linked for every submitted token and released (unlinked) when the
 *  result token is read. */
#define VEX_MAILBOX_LINK_ONLY_ONCE

/** Check that DMA operation is finished based on a TokenID behind the data */
#define VEX_CHECK_DMA_WITH_TOKEN_ID

/** SHA3 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_SHA3
#define VEX_ENABLE_SYM_ALGO_SHA3
#endif

/** SHA512 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_SHA512
#define VEX_ENABLE_SYM_ALGO_SHA512
#endif

/** AES related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_AES_F8
#define VEX_ENABLE_SYM_ALGO_AES_F8
#endif

/** DES related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_DES
#define VEX_ENABLE_SYM_ALGO_DES
#endif

/** Triple-DES (3DES) related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_3DES
#define VEX_ENABLE_SYM_ALGO_3DES
#endif

/** ChaCha20 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_CHACHA20
#define VEX_ENABLE_SYM_ALGO_CHACHA20
#endif

/** Poly1305 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_POLY1305
#define VEX_ENABLE_SYM_ALGO_POLY1305
#endif

/** SM2 related functionality use */
#ifdef EIP130_ENABLE_ASYM_ALGO_SM2
#define VEX_ENABLE_ASYM_SM2
#endif

/** SM3 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_SM3
#define VEX_ENABLE_SYM_ALGO_SM3
#endif

/** SM4 related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_SM4
#define VEX_ENABLE_SYM_ALGO_SM4
#ifdef EIP130_ENABLE_SYM_ALGO_SM4_GCM
#define VEX_ENABLE_SYM_ALGO_SM4_GCM
#endif
#endif

/** ARIA related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_ARIA
#define VEX_ENABLE_SYM_ALGO_ARIA
#ifdef EIP130_ENABLE_SYM_ALGO_ARIA_CCM
#define VEX_ENABLE_SYM_ALGO_ARIA_CCM
#endif
#ifdef EIP130_ENABLE_SYM_ALGO_ARIA_GCM
#define VEX_ENABLE_SYM_ALGO_ARIA_GCM
#endif
#ifdef EIP130_ENABLE_SYM_ALGO_ARIA_DRBG
#define VEX_ENABLE_ARIA_DRBG
#endif
#endif

/** AES-GCM related functionality use */
#ifdef EIP130_ENABLE_SYM_ALGO_AES_GCM
#define VEX_ENABLE_SYM_ALGO_AES_GCM
#endif

/** ECIES related functionality use */
#ifdef EIP130_ENABLE_ASYM_ALGO_ECIES
#define VEX_ENABLE_ASYM_ECIES
#endif

/** ECC ElGamal functionality use */
#ifdef EIP130_ENABLE_ASYM_ALGO_ECC_ELGAMAL
#define VEX_ENABLE_ASYM_ALGO_ECC_ELGAMAL
#endif

/** Special Functions Milenage related functionality use */
#ifdef EIP130_ENABLE_SF_MILENAGE
#define VEX_ENABLE_SF_MILENAGE
#ifndef VEX_ENABLE_SF
#define VEX_ENABLE_SF
#endif
#endif

/** Encrypted Vector for PKI related functionality use */
#ifdef EIP130_ENABLE_ENCRYPTED_VECTOR
#define VEX_ENABLE_ENCRYPTED_VECTOR
#endif

/** Enter-FIPS-mode function use */
#ifdef EIP130_ENABLE_SYSTEM_ENTERFIPSMODE
#define VEX_ENABLE_SYSTEM_ENTERFIPSMODE
#endif

/** DRBG force seed functionaity use */
#ifdef EIP130_ENABLE_DRBG_FORCESEED
#define VEX_ENABLE_DRBG_FORCESEED
#endif

/** Defines the firmware sleep is enabled */
#ifdef EIP130_ENABLE_FIRMWARE_SLEEP
#define VEX_ENABLE_FIRMWARE_SLEEP
#endif

/** Defines the firmware hibernation is enabled */
#ifdef EIP130_ENABLE_FIRMWARE_HIBERNATION
#define VEX_ENABLE_FIRMWARE_HIBERNATION
#endif

/** eMMC functionality use */
#ifdef EIP130_ENABLE_EMMC
#define VEX_ENABLE_EMMC
#endif

/** External service functionality use */
#ifdef EIP130_ENABLE_EXTSERVICE
#define VEX_ENABLE_EXTSERVICE
#endif

/** Defines that HW Module status, options and version function must be
 *  enabled */
#define VEX_ENABLE_HW_FUNCTIONS

/** Indication define that the public key buffer when generating a keypair
 *  shall not include the TokenID size in case the DMA Ready Check is enabled */
#define VEX_KEYPAIR_PUBKEYSIZE_NOTOKENID

/** Indication define that the DMA Ready Check must be skipped (no TokenIDWrite
 *  indication) for: */
//#define VEX_DISABLE_TOKENID_WR_KEYPAIR  /* key pairs */

/** Defines the interrupt use   */
#ifdef DRIVER_INTERRUPTS
#define VEX_USE_INTERRUPTS 1
#endif

/** Token/Data poll timeout settings */
#ifdef DRIVER_POLLING_DELAY_MS
#define VEX_POLLING_DELAY_MS            DRIVER_POLLING_DELAY_MS
#endif
#ifdef DRIVER_POLLING_MAXLOOPS
#define VEX_POLLING_MAXLOOPS            DRIVER_POLLING_MAXLOOPS
#endif


#endif /* INCLUDE_GUARD_CS_ADAPTER_VEX_H */

/* end of file cs_adapter_vex.h */
