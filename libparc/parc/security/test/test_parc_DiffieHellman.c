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
#include <parc/testing/parc_ObjectTesting.h>

#include "../parc_DiffieHellman.c"
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(parc_DiffieHellman)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_DiffieHellman)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_DiffieHellman)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellman_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellman_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcDiffieHellman_GenerateKeyShare);
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

LONGBOW_TEST_CASE(Global, parcDiffieHellman_Create)
{
    PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
    assertNotNull(dh, "Expected a non-NULL PARCDiffieHellman instance");
    parcDiffieHellman_Release(&dh);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellman_AcquireRelease)
{
    PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
    parcObjectTesting_AssertAcquireReleaseContract(parcDiffieHellman_Acquire, dh);
    parcDiffieHellman_Release(&dh);
}

LONGBOW_TEST_CASE(Global, parcDiffieHellman_GenerateKeyShare)
{
    PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
    PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellman_GenerateKeyShare(dh);
    assertNotNull(keyShare, "Expected a non-NULL PARCDiffieHellmanKeyShare instance");
    parcDiffieHellmanKeyShare_Release(&keyShare);
    parcDiffieHellman_Release(&dh);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_DiffieHellman);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
