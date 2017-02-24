/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include <stdio.h>
#include <LongBow/unit-test.h>

#include "../parc_CertificateType.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(parc_CertificateType)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_CertificateType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CertificateType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateType_FromString);
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateType_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parc_CertificateType_FromString)
{
    char *inputString = "PARCCertificateType_X509";
    PARCCertificateType encoding = parcCertificateType_FromString(inputString);
    assertTrue(encoding == PARCCertificateType_X509, "Expected PARCCertificateType_X509 (%d), for %d", PARCCertificateType_X509, encoding);

    inputString = "the cake is a lie";
    encoding = parcCertificateType_FromString(inputString);
    assertTrue(encoding == PARCCertificateType_Invalid, "Expected PARCCertificateType_Invalid (%d), for %d", PARCCertificateType_Invalid, encoding);
}

LONGBOW_TEST_CASE(Global, parc_CertificateType_ToString)
{
    char *expected = "PARCCertificateType_X509";
    PARCCertificateType encoding = PARCCertificateType_X509;
    const char *actual = parcCertificateType_ToString(encoding);
    assertTrue(strcmp(actual, expected) == 0, "Expected %s, got %s", expected, actual);

    actual = parcCertificateType_ToString(PARCCertificateType_Invalid);
    assertNull(actual, "Expected NULL string to be returned with invalid certificate type");
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CertificateType);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
