/* testvectors_aes_cmac_s2v.c
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

#ifdef VALTEST_USE_FUNCTION

#include "basic_defs.h"

#include "testvectors_aes_cmac_s2v.h"
#include "testvectors_aes_cmac_s2v_data.h"

/* Macros used to build the test vectors. */
#define TEST_VECTOR(key, keylen, msg, msglen, mac, maclen) \
    {                                                      \
        test_vector_keys + (key),                          \
        test_vector_msgs + (msg),                          \
        test_vector_macs + (mac),                          \
        keylen,                                            \
        msglen,                                            \
        maclen,                                            \
    }

/* Actual test vectors. */
static const TestVector_AES_CMAC_S2V_Rec_t cmac_s2v_test_vectors[] =
{
    /* First vector from RFC 5297, formed of two parts. */
    {
        true, 2,
        {
            TEST_VECTOR(0, 16, 0, 24, 0, 0),
            TEST_VECTOR(0, 16, 24, 14, 0, 16),
        }
    },
    /* Second vector from RFC 5297, formed of four parts. */
    {
        false, 4,
        {
            TEST_VECTOR(32, 16, 38, 40, 0, 0),
            TEST_VECTOR(32, 16, 78, 10, 0, 0),
            TEST_VECTOR(32, 16, 88, 16, 0, 0),
            TEST_VECTOR(32, 16, 104, 47, 30, 16)
        }
    }
};


/* The function API for accessing the vectors. */
int
test_vectors_aes_cmac_s2v_num(void)
{
    return sizeof(cmac_s2v_test_vectors) / sizeof(cmac_s2v_test_vectors[0]);
}

TestVector_AES_CMAC_S2V_t
test_vectors_aes_cmac_s2v_get(int Index)
{
    if (Index >= test_vectors_aes_cmac_s2v_num()) return NULL;
    return &cmac_s2v_test_vectors[Index];
}

void
test_vectors_aes_cmac_s2v_release(TestVector_AES_CMAC_S2V_t Vector_p)
{
    /* Test vectors are statically defined => nothing to do. */
    IDENTIFIER_NOT_USED(Vector_p);
}

#endif

/* end of file testvectors_aes_cmac_s2v.c */
