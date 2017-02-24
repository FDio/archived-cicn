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

#include "../parc_AtomicUint64.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <inttypes.h>

#include <parc/algol/parc_Memory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_AtomicUint64)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(CreateAcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Macros);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_AtomicUint64)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_AtomicUint64)
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
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);
    assertNotNull(instance, "Expeced non-null result from parcAtomicUint64_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcAtomicUint64_Acquire, instance);

    parcAtomicUint64_Release(&instance);
    assertNull(instance, "Expected null result from parcAtomicUint64_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_IsValid);

    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_AddImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint64_CompareAndSwapImpl);
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

LONGBOW_TEST_CASE(Global, parcAtomicUint64_Compare)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);
    PARCAtomicUint64 *high = parcAtomicUint64_Create(8);
    PARCAtomicUint64 *low = parcAtomicUint64_Create(6);
    PARCAtomicUint64 *equal = parcAtomicUint64_Create(7);

    int actual = parcAtomicUint64_Compare(instance, high);
    assertTrue(actual < 0, "Expected < 0");
    actual = parcAtomicUint64_Compare(instance, low);
    assertTrue(actual > 0, "Expected > 0");
    actual = parcAtomicUint64_Compare(instance, equal);
    assertTrue(actual == 0, "Expected == 0");

    parcAtomicUint64_Release(&instance);
    parcAtomicUint64_Release(&high);
    parcAtomicUint64_Release(&low);
    parcAtomicUint64_Release(&equal);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_Copy)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);
    PARCAtomicUint64 *copy = parcAtomicUint64_Copy(instance);

    assertTrue(parcAtomicUint64_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcAtomicUint64_Release(&instance);
    parcAtomicUint64_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_Equals)
{
    PARCAtomicUint64 *x = parcAtomicUint64_Create(7);
    PARCAtomicUint64 *y = parcAtomicUint64_Create(7);
    PARCAtomicUint64 *z = parcAtomicUint64_Create(7);
    PARCAtomicUint64 *u1 = parcAtomicUint64_Create(6);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcAtomicUint64_Release(&x);
    parcAtomicUint64_Release(&y);
    parcAtomicUint64_Release(&z);
    parcAtomicUint64_Release(&u1);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_HashCode)
{
    PARCAtomicUint64 *x = parcAtomicUint64_Create(7);
    parcAtomicUint64_HashCode(x);
    parcAtomicUint64_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_IsValid)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);
    assertTrue(parcAtomicUint64_IsValid(instance), "Expected parcAtomicUint64_Create to result in a valid instance.");

    parcAtomicUint64_Release(&instance);
    assertFalse(parcAtomicUint64_IsValid(instance), "Expected parcAtomicUint64_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_SubtractImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    parcAtomicUint64_SubtractImpl(instance, 1);

    uint64_t actual = parcAtomicUint64_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu64, actual);
    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_AddImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    parcAtomicUint64_AddImpl(instance, 1);

    uint64_t actual = parcAtomicUint64_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu64, actual);
    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint64_CompareAndSwapImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    bool actual = parcAtomicUint64_CompareAndSwapImpl(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint64_CompareAndSwapImpl to return true");
    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Macros)
{
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint64_Subtract);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint64_Add);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint64_CompareAndSwap);
}

LONGBOW_TEST_FIXTURE_SETUP(Macros)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Macros)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint64_Subtract)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    parcAtomicUint64_Subtract(instance, 1);

    uint64_t actual = parcAtomicUint64_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu64, actual);
    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint64_Add)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    parcAtomicUint64_Add(instance, 1);

    uint64_t actual = parcAtomicUint64_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu64, actual);
    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint64_CompareAndSwap)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(7);

    bool actual = parcAtomicUint64_CompareAndSwap(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint64_CompareAndSwap to return true");
    parcAtomicUint64_Release(&instance);
}


LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_Subtract_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_Add_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_CompareAndSwap_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_AddImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint64_CompareAndSwapImpl);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s leaked memory.", longBowTestCase_GetFullName(testCase))) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_Subtract_MACRO)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(100000000);

    while (parcAtomicUint64_Subtract(instance, 1) > 0) {
        ;
    }

    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_Add_MACRO)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(1);

    while (parcAtomicUint64_Add(instance, 1) < 100000000) {
        ;
    }

    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_CompareAndSwap_MACRO)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(0);

    for (uint64_t i = 0; i < 100000000; i++) {
        bool actual = parcAtomicUint64_CompareAndSwap(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint64_CompareAndSwap to return true");
    }

    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_SubtractImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(100000000);

    while (parcAtomicUint64_SubtractImpl(instance, 1) > 0) {
        ;
    }

    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_AddImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(1);

    while (parcAtomicUint64_AddImpl(instance, 1) < 100000000) {
        ;
    }

    parcAtomicUint64_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint64_CompareAndSwapImpl)
{
    PARCAtomicUint64 *instance = parcAtomicUint64_Create(0);

    for (uint64_t i = 0; i < 100000000; i++) {
        bool actual = parcAtomicUint64_CompareAndSwapImpl(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint64_CompareAndSwapImpl to return true");
    }

    parcAtomicUint64_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_AtomicUint64);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
