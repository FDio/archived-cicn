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
#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <parc/algol/parc_SafeMemory.h>

#include "../parc_Vector.c"

LONGBOW_TEST_RUNNER(PARCVector)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(PARCVector)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(PARCVector)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        exit(1);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcVectorDestroy);
    LONGBOW_RUN_TEST_CASE(Global, parcVectorGetLength);
    LONGBOW_RUN_TEST_CASE(Global, parcVectorGetPointer);
    LONGBOW_RUN_TEST_CASE(Global, parcVectorCreate);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcVectorDestroy)
{
    const char *string = "Hello World";

    PARCVector *x = parcVector_Create((void *) string, strlen(string));
    parcVector_Destroy(&x);
    assertNull(x, "Destroy did not nullify.");
}

LONGBOW_TEST_CASE(Global, parcVectorGetLength)
{
    const char *string = "Hello World";

    PARCVector *x = parcVector_Create((void *) string, strlen(string));
    size_t expected = strlen(string);
    size_t actual = parcVector_GetLength(x);
    assertEqual(expected, actual, "%zd");
    parcVector_Destroy(&x);
}

LONGBOW_TEST_CASE(Global, parcVectorGetPointer)
{
    const char *expected = "Hello World";

    PARCVector *x = parcVector_Create((void *) expected, strlen(expected));
    const char *actual = parcVector_GetPointer(x);

    assertEqual((void *) expected, (void *) actual, "%p");

    parcVector_Destroy(&x);
}

LONGBOW_TEST_CASE(Global, parcVectorCreate)
{
    const char *string = "Hello World";

    PARCVector *x = parcVector_Create((void *) string, strlen(string));
    assertNotNull(x, "Probably out of memory.");
    parcVector_Destroy(&x);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(PARCVector);
    int status = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(status);
}
