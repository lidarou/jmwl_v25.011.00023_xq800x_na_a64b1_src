/** @file cs_da_val_specials.h
 *
 * @brief VAL API specials example configuration settings
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

#ifndef INCLUDE_GUARD_CS_DA_VAL_SPECIALS_H
#define INCLUDE_GUARD_CS_DA_VAL_SPECIALS_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "cs_eip130.h"

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Maximum log severity */
#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRIT
//#define LOG_SEVERITY_MAX  LOG_SEVERITY_WARN
//#define LOG_SEVERITY_MAX  LOG_SEVERITY_INFO

/* Functionality enable/disable defines */
#ifdef EIP130_ENABLE_EMMC
#define DA_VAL_SPECIAL_EMMC
#endif

#if defined(EIP130_ENABLE_FIRMWARE_HIBERNATION) && \
    defined(EIP130_ENABLE_EMMC)
#define DA_VAL_SPECIAL_HIBERNATION
#endif

#ifdef EIP130_ENABLE_SF_MILENAGE
#define DA_VAL_SPECIAL_SF_MILENAGE
#endif

#ifdef EIP130_ENABLE_ASSET_STORE_RESET
#define DA_VAL_SPECIAL_ASSET_STORE_RESET
#endif

#ifdef EIP130_ENABLE_SYSTEM_ENTERFIPSMODE
#define DA_VAL_SPECIAL_ENTERFIPSMODE
#endif

#ifdef EIP130_ENABLE_DRBG_FORCESEED
#define DA_VAL_SPECIAL_DRBG_FORCESEED
#endif

#ifdef EIP130_ENABLE_SYM_ALGO_ARIA_DRBG
#define DA_VAL_SPECIAL_ARIA_DRBG
#endif

#ifdef EIP130_ENABLE_FIRMWARE_LOAD
#define DA_VAL_SPECIAL_FIRMWARE_LOAD
#endif


#endif /* INCLUDE_GUARD_CS_DA_VAL_SPECIALS_H */

/* end of file cs_da_val_specials.h */
