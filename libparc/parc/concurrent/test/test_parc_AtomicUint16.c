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

#include "../parc_AtomicUint16.c"

#include <LongBow/testing.h>
#include <LongBow/debugging.h>

#include <inttypes.h>

#include <parc/algol/parc_Memory.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parc_AtomicUint16)
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
LONGBOW_TEST_RUNNER_SETUP(parc_AtomicUint16)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_AtomicUint16)
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
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);
    assertNotNull(instance, "Expeced non-null result from parcAtomicUint16_Create();");

    parcObjectTesting_AssertAcquireReleaseContract(parcAtomicUint16_Acquire, instance);

    parcAtomicUint16_Release(&instance);
    assertNull(instance, "Expected null result from parcAtomicUint16_Release();");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_IsValid);

    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_AddImpl);
    LONGBOW_RUN_TEST_CASE(Global, parcAtomicUint16_CompareAndSwapImpl);
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

LONGBOW_TEST_CASE(Global, parcAtomicUint16_Compare)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);
    PARCAtomicUint16 *high = parcAtomicUint16_Create(8);
    PARCAtomicUint16 *low = parcAtomicUint16_Create(6);
    PARCAtomicUint16 *equal = parcAtomicUint16_Create(7);

    int actual = parcAtomicUint16_Compare(instance, high);
    assertTrue(actual < 0, "Expected < 0");
    actual = parcAtomicUint16_Compare(instance, low);
    assertTrue(actual > 0, "Expected > 0");
    actual = parcAtomicUint16_Compare(instance, equal);
    assertTrue(actual == 0, "Expected == 0");

    parcAtomicUint16_Release(&instance);
    parcAtomicUint16_Release(&high);
    parcAtomicUint16_Release(&low);
    parcAtomicUint16_Release(&equal);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_Copy)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);
    PARCAtomicUint16 *copy = parcAtomicUint16_Copy(instance);

    assertTrue(parcAtomicUint16_Equals(instance, copy), "Expected the copy to be equal to the original");

    parcAtomicUint16_Release(&instance);
    parcAtomicUint16_Release(&copy);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_Equals)
{
    PARCAtomicUint16 *x = parcAtomicUint16_Create(7);
    PARCAtomicUint16 *y = parcAtomicUint16_Create(7);
    PARCAtomicUint16 *z = parcAtomicUint16_Create(7);
    PARCAtomicUint16 *u1 = parcAtomicUint16_Create(6);

    parcObjectTesting_AssertEquals(x, y, z, u1, NULL);

    parcAtomicUint16_Release(&x);
    parcAtomicUint16_Release(&y);
    parcAtomicUint16_Release(&z);
    parcAtomicUint16_Release(&u1);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_HashCode)
{
    PARCAtomicUint16 *x = parcAtomicUint16_Create(7);
    parcAtomicUint16_HashCode(x);
    parcAtomicUint16_Release(&x);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_IsValid)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);
    assertTrue(parcAtomicUint16_IsValid(instance), "Expected parcAtomicUint16_Create to result in a valid instance.");

    parcAtomicUint16_Release(&instance);
    assertFalse(parcAtomicUint16_IsValid(instance), "Expected parcAtomicUint16_Release to result in an invalid instance.");
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_SubtractImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    parcAtomicUint16_SubtractImpl(instance, 1);

    uint16_t actual = parcAtomicUint16_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu16, actual);
    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_AddImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    parcAtomicUint16_AddImpl(instance, 1);

    uint16_t actual = parcAtomicUint16_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu16, actual);
    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Global, parcAtomicUint16_CompareAndSwapImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    bool actual = parcAtomicUint16_CompareAndSwapImpl(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint16_CompareAndSwapImpl to return true");
    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_FIXTURE(Macros)
{
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint16_Subtract);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint16_Add);
    LONGBOW_RUN_TEST_CASE(Macros, parcAtomicUint16_CompareAndSwap);
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

LONGBOW_TEST_CASE(Macros, parcAtomicUint16_Subtract)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    parcAtomicUint16_Subtract(instance, 1);

    uint16_t actual = parcAtomicUint16_GetValue(instance);

    assertTrue(actual == 6, "Expected 6, actual %" PRIu16, actual);
    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint16_Add)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    parcAtomicUint16_Add(instance, 1);

    uint16_t actual = parcAtomicUint16_GetValue(instance);

    assertTrue(actual == 8, "Expected 8, actual %" PRIu16, actual);
    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Macros, parcAtomicUint16_CompareAndSwap)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(7);

    bool actual = parcAtomicUint16_CompareAndSwap(instance, 7, 8);

    assertTrue(actual, "Expected parcAtomicUint16_CompareAndSwap to return true");
    parcAtomicUint16_Release(&instance);
}


LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_Subtract_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_Add_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_CompareAndSwap_MACRO);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_SubtractImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_AddImpl);
    LONGBOW_RUN_TEST_CASE(Performance, parcAtomicUint16_CompareAndSwapImpl);
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

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_Subtract_MACRO)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(65535);

    while (parcAtomicUint16_Subtract(instance, 1) > 0) {
        ;
    }

    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_Add_MACRO)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(1);

    while (parcAtomicUint16_Add(instance, 1) < 65535) {
        ;
    }

    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_CompareAndSwap_MACRO)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(0);

    for (uint16_t i = 0; i < 65535; i++) {
        bool actual = parcAtomicUint16_CompareAndSwap(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint16_CompareAndSwap to return true");
    }

    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_SubtractImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(65535);

    while (parcAtomicUint16_SubtractImpl(instance, 1) > 0) {
        ;
    }

    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_AddImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(1);

    while (parcAtomicUint16_AddImpl(instance, 1) < 65535) {
        ;
    }

    parcAtomicUint16_Release(&instance);
}

LONGBOW_TEST_CASE(Performance, parcAtomicUint16_CompareAndSwapImpl)
{
    PARCAtomicUint16 *instance = parcAtomicUint16_Create(0);

    for (uint16_t i = 0; i < 65535; i++) {
        bool actual = parcAtomicUint16_CompareAndSwapImpl(instance, i, i + 1);
        assertTrue(actual, "Expected parcAtomicUint16_CompareAndSwapImpl to return true");
    }

    parcAtomicUint16_Release(&instance);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_AtomicUint16);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
