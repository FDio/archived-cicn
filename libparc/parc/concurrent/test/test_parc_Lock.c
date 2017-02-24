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
#include "../parc_Lock.c"

#include <stdio.h>

#include <LongBow/testing.h>
#include <LongBow/debugging.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_Lock)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Locking);
    LONGBOW_RUN_TEST_FIXTURE(WaitNotify);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Lock)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Lock)
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
    PARCLock *instance = parcLock_Create();
    assertNotNull(instance, "Expected non-null result from parcLock_Create().");

    parcObjectTesting_AssertAcquire(instance);

    parcObjectTesting_AssertAcquireReleaseContract(parcLock_Acquire, instance);

    parcLock_Release(&instance);
    assertNull(instance, "Expected null result from parcLock_Release().");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLock_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcLock_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, parcLock_ToString);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcLock_Display)
{
    PARCLock *lock = parcLock_Create();
    parcLock_Display(lock, 0);
    parcLock_Release(&lock);
}

LONGBOW_TEST_CASE(Global, parcLock_IsValid)
{
    PARCLock *instance = parcLock_Create();
    assertTrue(parcLock_IsValid(instance), "Expected parcLock_Create to result in a valid instance.");

    parcLock_Release(&instance);
    assertFalse(parcLock_IsValid(instance), "Expected parcLock_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcLock_ToString)
{
    PARCLock *instance = parcLock_Create();

    char *string = parcLock_ToString(instance);

    assertNotNull(string, "Expected non-NULL result from parcLock_ToString");

    parcMemory_Deallocate((void **) &string);
    parcLock_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Locking)
{
    LONGBOW_RUN_TEST_CASE(Locking, parcLock_TryLock_Unlock);
    LONGBOW_RUN_TEST_CASE(Locking, parcLock_TryLock_AlreadyLocked);
    LONGBOW_RUN_TEST_CASE(Locking, parcLock_Lock_Unlock);
    LONGBOW_RUN_TEST_CASE(Locking, parcLock_Lock_AlreadyLocked);
    LONGBOW_RUN_TEST_CASE(Locking, parcLock_Lock_AlreadyLocked);
}

LONGBOW_TEST_FIXTURE_SETUP(Locking)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Locking)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Locking, parcLock_TryLock_Unlock)
{
    PARCLock *lock = parcLock_Create();

    bool actual = parcLock_TryLock(lock);

    assertTrue(actual, "Expected parcObject_TryLock to succeed.");

    actual = parcLock_IsLocked(lock);
    assertTrue(actual, "Expected parcObject_IsLocked to be true.");

    actual = parcLock_Unlock(lock);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    actual = parcLock_IsLocked(lock);
    assertTrue(actual, "Expected parcObject_IsLocked to be false.");

    parcLock_Release((PARCLock **) &lock);
}

LONGBOW_TEST_CASE(Locking, parcLock_Lock_Unlock)
{
    PARCLock *lock = parcLock_Create();

    bool actual = parcLock_Lock(lock);

    assertTrue(actual, "Expected parcObject_Lock to succeed.");

    actual = parcLock_IsLocked(lock);
    assertTrue(actual, "Expected parcObject_IsLocked to be true.");

    actual = parcLock_Unlock(lock);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    actual = parcLock_IsLocked(lock);
    assertTrue(actual, "Expected parcObject_IsLocked to be false.");

    parcLock_Release((PARCLock **) &lock);
}

LONGBOW_TEST_CASE(Locking, parcLock_TryLock_AlreadyLocked)
{
    PARCLock *lock = parcLock_Create();

    bool actual = parcLock_TryLock(lock);

    assertTrue(actual, "Expected parcObject_TryLock to succeed.");

    actual = parcLock_TryLock(lock);

    assertFalse(actual, "Expected parcObject_TryLock to fail when already locked.");

    actual = parcLock_Unlock(lock);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    parcLock_Release((PARCLock **) &lock);
}

LONGBOW_TEST_CASE(Locking, parcLock_Lock_AlreadyLocked)
{
    PARCLock *lock = parcLock_Create();

    bool actual = parcLock_Lock(lock);

    assertTrue(actual, "Expected parcObject_Lock to succeed.");

    actual = parcLock_Lock(lock);

    assertFalse(actual, "Expected parcObject_Lock to fail when already locked by the same thread.");

    actual = parcLock_Unlock(lock);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    parcLock_Release((PARCLock **) &lock);
}

LONGBOW_TEST_FIXTURE(WaitNotify)
{
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcLock_WaitNotify);
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcLock_WaitNotify2);
}

LONGBOW_TEST_FIXTURE_SETUP(WaitNotify)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(WaitNotify)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static int _sharedValue;

static void *
waiter(void *data)
{
    PARCLock *lock = data;

    while (parcLock_TryLock(lock) == false) {
        ;
    }
    parcLock_Wait(lock);

    _sharedValue++;
    parcLock_Unlock(lock);

    return data;
}

LONGBOW_TEST_CASE(WaitNotify, parcLock_WaitNotify)
{
    PARCLock *lock = parcLock_Create();

    _sharedValue = 0;

    pthread_t thread_A;
    pthread_t thread_B;
    pthread_t thread_C;
    pthread_create(&thread_A, NULL, waiter, lock);
    pthread_create(&thread_B, NULL, waiter, lock);
    pthread_create(&thread_C, NULL, waiter, lock);

    while (_sharedValue != 3) {
        while (parcLock_TryLock(lock) == false) {
            ;
        }
        parcLock_Notify(lock);
        parcLock_Unlock(lock);
    }

    pthread_join(thread_A, NULL);

    parcLock_Release((PARCLock **) &lock);
}

static void *
decrement(void *data)
{
    PARCLock *lock = data;

    while (parcLock_TryLock(lock) == false) {
        assertTrue(write(1, ".", 1) == 1, "Write failed.");
    }
    while (_sharedValue < 12) {
        parcLock_Wait(lock);
        _sharedValue--;
    }
    parcLock_Unlock(lock);

    return data;
}

LONGBOW_TEST_CASE(WaitNotify, parcLock_WaitNotify2)
{
    PARCLock *lock = parcLock_Create();

    _sharedValue = 0;

    pthread_t thread_A;
    pthread_create(&thread_A, NULL, decrement, lock);

    _sharedValue = 2;
    while (parcLock_TryLock(lock) == false) {
        assertTrue(write(1, ".", 1) == 1, "Write failed.");
    }
    while (_sharedValue <= 12) {
        printf("%d\n", _sharedValue);
        parcLock_Notify(lock);
        _sharedValue += 2;
    }
    parcLock_Unlock(lock);

    parcLock_Notify(lock);
    pthread_join(thread_A, NULL);

    parcLock_Release((PARCLock **) &lock);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Lock);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
