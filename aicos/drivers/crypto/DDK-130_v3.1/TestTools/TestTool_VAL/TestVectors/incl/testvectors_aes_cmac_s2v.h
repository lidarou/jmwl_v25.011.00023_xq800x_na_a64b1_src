/* testvectors_aes_cmac_s2v.h
 *
 * Description: Test vectors for AES CMAC_S2V.
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

#ifndef INCLUDE_GUARD_TESTVECTORS_AES_CMAC_S2V_H
#define INCLUDE_GUARD_TESTVECTORS_AES_CMAC_S2V_H

#include "basic_defs.h"

/* Structure for S2V Test Vectors builds upon CMAC test vectors. */
#include "testvectors_aes_cmac.h"

typedef struct
{
    bool Encrypt;   /* Support encryption as well. Can test SIV. */
    uint32_t Count; /* Number of valid vectors within the array */
    const TestVector_AES_CMAC_Rec_t Vector[10];
} TestVector_AES_CMAC_S2V_Rec_t;

typedef const TestVector_AES_CMAC_S2V_Rec_t * TestVector_AES_CMAC_S2V_t;

/* API for using AES CMAC_S2V test vectors. */

/* Request number of AES CMAC_S2V test vectors available. */
int test_vectors_aes_cmac_s2v_num(void);

/* Request test vector by index.
   If Index >= test_vectors_aes_cmac_s2v_num(), the function returns NULL.

   Note: The function returns zero only when invalid vector has been requested.
   Note: The received vector is constant and must not be altered by caller.
   The test vector is described by structure TestVector_AES_CMAC_S2V_t and
   the structure shall be accessed directly by the user of the test vector.
*/
TestVector_AES_CMAC_S2V_t test_vectors_aes_cmac_s2v_get(int Index);

/* Release a vector.
   It is mandatory to release a vector once user of test vector is finished
   with the vector. */
void test_vectors_aes_cmac_s2v_release(TestVector_AES_CMAC_S2V_t Vector_p);


#endif /* INCLUDE_GUARD_TESTVECTORS_AES_CMAC_S2V_H */

/* end of file testvectors_aes_cmac_s2v.h */
