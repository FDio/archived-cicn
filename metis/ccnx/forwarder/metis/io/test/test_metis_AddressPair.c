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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_AddressPair.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_AddressPair)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_AddressPair)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_AddressPair)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisAddressPair_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisAddressPair_Equals);
    LONGBOW_RUN_TEST_CASE(Global, metisAddressPair_Equals_NotEqual);
    LONGBOW_RUN_TEST_CASE(Global, metisAddressPair_EqualsAddresses);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisAddressPair_Create_Destroy)
{
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    CPIAddress *b = cpiAddress_CreateFromInterface(2);

    size_t allocbase = parcSafeMemory_Outstanding();
    MetisAddressPair *pair = metisAddressPair_Create(a, b);
    metisAddressPair_Release(&pair);

    assertTrue(parcSafeMemory_Outstanding() == allocbase,
               "Memory out of balance, expected %zu got %u",
               allocbase,
               parcSafeMemory_Outstanding());

    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, metisAddressPair_Equals)
{
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    CPIAddress *b = cpiAddress_CreateFromInterface(2);

    MetisAddressPair *pair_a = metisAddressPair_Create(a, b);
    MetisAddressPair *pair_b = metisAddressPair_Create(a, b);

    assertTrue(metisAddressPair_Equals(pair_a, pair_b), "Two equal address pairs did not compare equal");

    metisAddressPair_Release(&pair_a);
    metisAddressPair_Release(&pair_b);

    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, metisAddressPair_Equals_NotEqual)
{
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    CPIAddress *b = cpiAddress_CreateFromInterface(2);

    MetisAddressPair *pair_a = metisAddressPair_Create(a, b);
    MetisAddressPair *pair_b = metisAddressPair_Create(b, a);

    assertFalse(metisAddressPair_Equals(pair_a, pair_b), "Two unequal address pairs compared equal");

    metisAddressPair_Release(&pair_a);
    metisAddressPair_Release(&pair_b);
    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, metisAddressPair_EqualsAddresses)
{
    CPIAddress *a = cpiAddress_CreateFromInterface(1);
    CPIAddress *b = cpiAddress_CreateFromInterface(2);

    MetisAddressPair *pair_a = metisAddressPair_Create(a, b);

    assertTrue(metisAddressPair_EqualsAddresses(pair_a, a, b), "Two equal address pairs did not compare equal");

    metisAddressPair_Release(&pair_a);
    cpiAddress_Destroy(&a);
    cpiAddress_Destroy(&b);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_AddressPair);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
