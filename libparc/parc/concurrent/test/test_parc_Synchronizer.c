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

/**
 */
#include "../parc_Synchronizer.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Synchronizer)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Synchronizer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Synchronizer)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(CreateAcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(CreateAcquireRelease, CreateRelease);
}


LONGBOW_TEST_FIXTURE_SETUP(CreateAcquireRelease)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateAcquireRelease)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateAcquireRelease, CreateRelease)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();
    assertNotNull(instance, "Expeced non-null result from parcSynchronizer_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcSynchronizer_Acquire, instance);

    parcSynchronizer_Release(&instance);
    assertNull(instance, "Expeced null result from parcSynchronizer_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_IsValid);

    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_TryLock);
    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_LockUnlock);
    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_TryLock_Fail);
    LONGBOW_RUN_TEST_CASE(Global, parcSynchronizer_IsLocked);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_Display)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();
    assertTrue(parcSynchronizer_IsValid(instance), "Expected parcSynchronizer_Create to result in a valid instance.");

    parcSynchronizer_Display(instance, 0);
    parcSynchronizer_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_IsValid)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();
    assertTrue(parcSynchronizer_IsValid(instance), "Expected parcSynchronizer_Create to result in a valid instance.");

    parcSynchronizer_Release(&instance);
    assertFalse(parcSynchronizer_IsValid(instance), "Expected parcSynchronizer_Create to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_TryLock)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();

    bool actual = parcSynchronizer_TryLock(instance);
    assertTrue(actual, "Expected parcSynchronizer_TryLock to be successful.");

    parcSynchronizer_Unlock(instance);
    parcSynchronizer_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_TryLock_Fail)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();

    parcSynchronizer_Lock(instance);
    bool actual = parcSynchronizer_TryLock(instance);
    assertFalse(actual, "Expected parcSynchronizer_TryLock to be unsuccessful.");

    parcSynchronizer_Unlock(instance);
    parcSynchronizer_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_LockUnlock)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();

    parcSynchronizer_Lock(instance);
    parcSynchronizer_Unlock(instance);

    parcSynchronizer_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcSynchronizer_IsLocked)
{
    PARCSynchronizer *instance = parcSynchronizer_Create();
    assertFalse(parcSynchronizer_IsLocked(instance), "Expected the synchronizer to be unlocked.");

    parcSynchronizer_Lock(instance);
    assertTrue(parcSynchronizer_IsLocked(instance), "Expected the synchronizer to be unlocked.");

    parcSynchronizer_Unlock(instance);
    assertFalse(parcSynchronizer_IsLocked(instance), "Expected the synchronizer to be unlocked.");

    parcSynchronizer_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Synchronizer);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}


