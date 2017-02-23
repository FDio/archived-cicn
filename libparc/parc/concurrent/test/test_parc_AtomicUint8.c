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

#include "../parc_AtomicUint8.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <inttypes.h>

#include <parc/algol/parc_Memory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_AtomicUint8)
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
LONGBOW_TEST_RUNNER_SETUP(parc_AtomicUint8)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_AtomicUint8)
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
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);
    assertNotNull(instance, "Expeced non-null result from parcAtomicUint8_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcAtomicUint8_Acquire, instance);

    parcAtomicUint8_Release(&instance);
    assertNull(instance, "Expected null result from parcAtomicUint8_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_IsValid);

    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_AddImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint8_CompareAndSwapImpl);
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

LONGBOW_TEST_CASE(Global, parcAtomicUint8_Compare)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);
    PARCAtomicUint8 *high = parcAtomicUint8_Create(8);
    PARCAtomicUint8 *low = parcAtomicUint8_Create(6);
    PARCAtomicUint8 *equal = parcAtomicUint8_Create(7);

    int actual = parcAtomicUint8_Compare(instance, high);
    assertTrue(actual < 0, "Expected < 0");
    actual = parcAtomicUint8_Compare(instance, low);
    assertTrue(actual > 0, "Expected > 0");
    actual = parcAtomicUint8_Compare(instance, equal);
    assertTrue(actual == 0, "Expected == 0");

    parcAtomicUint8_Release(&instance);
    parcAtomicUint8_Release(&high);
    parcAtomicUint8_Release(&low);
    parcAtomicUint8_Release(&equal);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_Copy)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);
    PARCAtomicUint8 *copy = parcAtomicUint8_Copy(instance);

    assertTrue(parcAtomicUint8_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcAtomicUint8_Release(&instance);
    parcAtomicUint8_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_Equals)
{
    PARCAtomicUint8 *x = parcAtomicUint8_Create(7);
    PARCAtomicUint8 *y = parcAtomicUint8_Create(7);
    PARCAtomicUint8 *z = parcAtomicUint8_Create(7);
    PARCAtomicUint8 *u1 = parcAtomicUint8_Create(6);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcAtomicUint8_Release(&x);
    parcAtomicUint8_Release(&y);
    parcAtomicUint8_Release(&z);
    parcAtomicUint8_Release(&u1);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_HashCode)
{
    PARCAtomicUint8 *x = parcAtomicUint8_Create(7);
    parcAtomicUint8_HashCode(x);
    parcAtomicUint8_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_IsValid)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);
    assertTrue(parcAtomicUint8_IsValid(instance), "Expected parcAtomicUint8_Create to result in a valid instance.");

    parcAtomicUint8_Release(&instance);
    assertFalse(parcAtomicUint8_IsValid(instance), "Expected parcAtomicUint8_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_SubtractImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    parcAtomicUint8_SubtractImpl(instance, 1);

    uint8_t actual = parcAtomicUint8_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu8, actual);
    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_AddImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    parcAtomicUint8_AddImpl(instance, 1);

    uint8_t actual = parcAtomicUint8_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu8, actual);
    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint8_CompareAndSwapImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    bool actual = parcAtomicUint8_CompareAndSwapImpl(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint8_CompareAndSwapImpl to return true");
    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Macros)
{
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint8_Subtract);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint8_Add);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint8_CompareAndSwap);
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

LONGBOW_TEST_CASE(Macros, parcAtomicUint8_Subtract)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    parcAtomicUint8_Subtract(instance, 1);

    uint8_t actual = parcAtomicUint8_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu8, actual);
    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint8_Add)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    parcAtomicUint8_Add(instance, 1);

    uint8_t actual = parcAtomicUint8_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu8, actual);
    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint8_CompareAndSwap)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(7);

    bool actual = parcAtomicUint8_CompareAndSwap(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint8_CompareAndSwap to return true");
    parcAtomicUint8_Release(&instance);
}


LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_Subtract_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_Add_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_CompareAndSwap_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_AddImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint8_CompareAndSwapImpl);
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

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_Subtract_MACRO)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(255);

    while (parcAtomicUint8_Subtract(instance, 1) > 0) {
        ;
    }

    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_Add_MACRO)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(1);

    while (parcAtomicUint8_Add(instance, 1) < 255) {
        ;
    }

    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_CompareAndSwap_MACRO)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(0);

    for (uint8_t i = 0; i < 255; i++) {
        bool actual = parcAtomicUint8_CompareAndSwap(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint8_CompareAndSwap to return true");
    }

    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_SubtractImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(255);

    while (parcAtomicUint8_SubtractImpl(instance, 1) > 0) {
        ;
    }

    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_AddImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(1);

    while (parcAtomicUint8_AddImpl(instance, 1) < 255) {
        ;
    }

    parcAtomicUint8_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint8_CompareAndSwapImpl)
{
    PARCAtomicUint8 *instance = parcAtomicUint8_Create(0);

    for (uint8_t i = 0; i < 255; i++) {
        bool actual = parcAtomicUint8_CompareAndSwapImpl(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint8_CompareAndSwapImpl to return true");
    }

    parcAtomicUint8_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_AtomicUint8);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
