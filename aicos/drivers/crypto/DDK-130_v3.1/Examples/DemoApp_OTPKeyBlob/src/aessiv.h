/* aessiv.h
 *
 * AES-SIV implementation using the VAL API.
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

#ifndef INCLUDE_GUARD_AESSIV_H
#define INCLUDE_GUARD_AESSIV_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "basic_defs.h"
#include "clib.h"


typedef struct
{
    uint8_t Key[2*(256/8)];             // AES_CMAC [256/8] + AES-CTR [256/8] key
    unsigned int KeySize;               // Key size in bytes
    uint8_t AD_Buffer[4096];            // Common buffer associated Data (labels)
    uint8_t * AD_List[9];               // Associated Data (label) array
    unsigned int AD_ListCount;          // Number of labels in the AD_List
    bool Verbose;                       // Verbose setting
} AESSIV_Context;


int
AESSIV_Init(
        AESSIV_Context * Context_p,
        const bool Verbose);

int
AESSIV_SetKey(
        AESSIV_Context * Context_p,
        const uint8_t * Key_p,
        const size_t KeySize);

int
AESSIV_SetAD(
        AESSIV_Context * Context_p,
        const uint8_t * AD_p,
        const size_t ADSize);

int
AESSIV_Encrypt(
        AESSIV_Context * Context_p,
        const uint8_t * InData_p,
        const size_t InDataSize,
        uint8_t * OutData_p,
        size_t * OutDataSize_p);


#endif /* INCLUDE_GUARD_AESSIV_H */

/* end of file aessiv.h */

