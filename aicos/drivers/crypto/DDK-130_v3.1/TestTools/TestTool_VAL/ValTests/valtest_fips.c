/* valtest_fips.c
 *
 * Description: FIPS related tests
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

#include "valtest_internal.h"

static int
do_fips_system_define_users(void)
{
    ValStatus_t Status;
    uint8_t OtpErrorCode;
    uint16_t OtpErrorLocation;
    uint8_t Mode;
    uint8_t ErrorTest;
    uint8_t CryptoOfficer;
    uint8_t HostID;
    uint8_t NonSecure;
    uint32_t Identity;

    unsupported_unless(valtest_IsCOIDAvailable(), "COID not available");

    Status = val_SystemGetState(&OtpErrorCode, &OtpErrorLocation,
                                &Mode, &ErrorTest, &CryptoOfficer,
                                &HostID, &NonSecure, &Identity);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);

    Status = val_SystemDefineUsers(Identity, 0x609, 0x86090001, 0xA);
    fail_if(Status != VAL_SUCCESS, "val_SystemDefineUsers()=", Status);

    Status = val_SystemGetState(&OtpErrorCode, &OtpErrorLocation,
                                &Mode, &ErrorTest, &CryptoOfficer,
                                &HostID, &NonSecure, &Identity);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);

    return END_TEST_SUCCES;
}

START_TEST(test_fips_system_define_users1)
{
    int rc = do_fips_system_define_users();
    if (rc != END_TEST_SUCCES)
    {
        return rc;
    }
}
END_TEST

START_TEST(test_fips_system_define_users2)
{
    int rc = do_fips_system_define_users();
    if (rc != END_TEST_SUCCES)
    {
        return rc;
    }
}
END_TEST

#ifdef VALTEST_SYSTEM_ENTERFIPSMODE
START_TEST(test_fips_system_enterfipsmode)
{
    ValStatus_t Status;
    uint8_t Mode;

    unsupported_unless(valtest_IsCOIDAvailable(), "COID not available");

    Status = val_SystemGetState(NULL, NULL, &Mode, NULL, NULL, NULL, NULL, NULL);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);
    fail_if(Mode != 0, "Not non-FIPS - Mode=", Mode);

    Status = val_SystemEnterFipsMode();
    fail_if(Status != VAL_SUCCESS, "val_SystemEnterFipsMode()=", Status);

    Status = val_SystemGetState(NULL, NULL, &Mode, NULL, NULL, NULL, NULL, NULL);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);

    fail_if(Mode != 15, "Not FIPS - Mode=", Mode);
}
END_TEST
#endif

START_TEST(test_fips_system_selftest)
{
    ValStatus_t Status;
    uint8_t Mode;

    unsupported_unless(valtest_IsCOIDAvailable(), "COID not available");

    Status = val_SystemGetState(NULL, NULL, &Mode, NULL, NULL, NULL, NULL, NULL);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);
    fail_if(Mode != 0, "Not non-FIPS - Mode=", Mode);

    Status = val_SystemSelfTest();
    fail_if(Status != VAL_SUCCESS, "val_SystemSelfTest()=", Status);

    Status = val_SystemGetState(NULL, NULL, &Mode, NULL, NULL, NULL, NULL, NULL);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetState(INFO)=", Status);
#ifdef VALTEST_SYSTEM_ENTERFIPSMODE
    fail_if(Mode != 0, "Not non-FIPS - Mode=", Mode);
#else
    fail_if(Mode != 15, "Not FIPS - Mode=", Mode);
#endif
}
END_TEST

START_TEST(test_fips_system_reset)
{
    char versionString[VAL_TEST_MAX_VERSION_LENGTH + 1];
    ValSize_t versionStringLen;
    ValStatus_t Status;

    Status = val_SystemReset();
    fail_if(Status != VAL_SUCCESS, "val_SystemReset()=", Status);

    versionStringLen = VAL_TEST_MAX_VERSION_LENGTH;
    Status = val_SystemGetVersion((ValOctetsOut_t *)versionString,
                                  &versionStringLen);
    fail_if(Status != VAL_SUCCESS, "val_SystemGetVersion()=", Status);
}
END_TEST

int
suite_add_test_FIPS(
        struct TestSuite * TestSuite_p)
{
    struct TestCase * TestCase_p;

    TestCase_p = sfzutf_tcase_create(TestSuite_p, "FIPS_Tests");
    if (TestCase_p != NULL)
    {
        if (sfzutf_tcase_add_fixture(TestCase_p, valtest_initialize, valtest_terminate) != 0)
        {
             goto FuncErrorReturn;
        }

        if (sfzutf_test_add(TestCase_p, test_fips_system_define_users1) != 0) goto FuncErrorReturn;
        if (g_ValTestResetAllowed)
        {
            if (sfzutf_test_add(TestCase_p, test_fips_system_selftest) != 0) goto FuncErrorReturn;
#ifdef VALTEST_SYSTEM_ENTERFIPSMODE
            if (sfzutf_test_add(TestCase_p, test_fips_system_enterfipsmode) != 0) goto FuncErrorReturn;
#endif
            if (sfzutf_test_add(TestCase_p, test_fips_system_define_users2) != 0) goto FuncErrorReturn;

            // OPTIONAL: Added some FIPS specific tests, but does not add coverage.

            if (sfzutf_test_add(TestCase_p, test_fips_system_reset) != 0) goto FuncErrorReturn;
        }

        return 0;
    }

FuncErrorReturn:
    return -1;
}

/* end of file valtest_fips.c */
