/**
 * @file api_val_buffers.h
 *
 * @brief VAL API - Data Buffer definitions
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

#ifndef INCLUDE_GUARD_API_VAL_BUFFERS_H
#define INCLUDE_GUARD_API_VAL_BUFFERS_H

/*-----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "clib.h"                   // size_t
#include "basic_defs.h"             // uint8_t


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
/** ValOctets*_t macros\n
 *  The types can be used in pointer-parameter declarations to clarify the
 *  direction of the parameter (input-only, output-only or input-output).
 *  Use the _Optional variant when the pointer parameter may be NULL. */
typedef const uint8_t ValOctetsIn_t;
typedef uint8_t ValOctetsOut_t;
typedef uint8_t ValOctetsInOut_t;

/** ValOctets*_Optional_t macros\n
 *  Refers to a pointer-parameter declarations or has the value NULL */
typedef ValOctetsIn_t    ValOctetsIn_Optional_t;
typedef ValOctetsOut_t   ValOctetsOut_Optional_t;
typedef ValOctetsInOut_t ValOctetsInOut_Optional_t;


/** ValSize_t\n
 *  This type is large enough to hold the size of a buffer, or the length of
 *  the data in the buffer. The maximum size for the entire API is defined by
 *  VAL_SIZE_MAX and when data is DMAed it is restricted to VAL_SIZE_MAX_DMA */
typedef size_t ValSize_t;

/** The maximum size value */
#define VAL_SIZE_MAX         0x7FFFFFFF
/** The maximum DMA-able size value */
#define VAL_SIZE_MAX_DMA     0x001FFFFF


#endif /* INCLUDE_GUARD_API_VAL_BUFFERS_H */

/* end of file api_val_buffers.h */
