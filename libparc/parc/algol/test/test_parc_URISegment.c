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


#include "../parc_URISegment.c"
#include <LongBow/unit-test.h>

#include <stdint.h>

#include <parc/algol/parc_URI.h>

#include "_test_parc_URI.h"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parcURISegment)
{
    LONGBOW_RUN_TEST_FIXTURE(parcURISegment);
}

LONGBOW_TEST_RUNNER_SETUP(parcURISegment)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parcURISegment)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(parcURISegment)
{
    LONGBOW_RUN_TEST_CASE(parcURISegment, _parcURISegment_fromHexDigit);
    LONGBOW_RUN_TEST_CASE(parcURISegment, _parcURISegment_parsePercentEncoded);

    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Acquire);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Create);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Parse);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Parse_WithExtraSlashes);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Parse_WithInvalidPercentage);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Release);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Length);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_ToString);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Equals_Contract);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Compare_Contract);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_Clone);
    LONGBOW_RUN_TEST_CASE(parcURISegment, parcURISegment_GetBuffer);
}

LONGBOW_TEST_FIXTURE_SETUP(parcURISegment)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(parcURISegment)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(parcURISegment, _parcURISegment_fromHexDigit)
{
    const char test = 'G';
    signed char actual = _fromHexDigit(test);
    assertTrue(-1 == (int) actual, "Invalid hex digit should not be changed to a decimal value, we expect -1 as the result");
}

LONGBOW_TEST_CASE(parcURISegment, _parcURISegment_parsePercentEncoded)
{
    unsigned char buffer;

    const char *test1 = "0G";
    const char *result1 = _parsePercentEncoded(test1, &buffer);
    assertTrue(NULL == result1, "Expected NULL parsed byte from invalid encoded percentage string, got %s", result1);
    const char *test2 = "GG";
    const char *result2 = _parsePercentEncoded(test2, &buffer);
    assertTrue(NULL == result2, "Expected NULL parsed byte from invalid encoded percentage string, got %s", result2);
    const char *test3 = "";
    const char *result3 = _parsePercentEncoded(test3, &buffer);
    assertTrue(NULL == result3, "Expected NULL parsed byte from empty encoded percentage string, got %s", result3);
    const char *test4 = "0";
    const char *result4 = _parsePercentEncoded(test4, &buffer);
    assertTrue(NULL == result4, "Expected NULL parsed byte from half-empty encoded percentage string, got %s", result4);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Acquire)
{
    char *expected = URI_PATH_SEGMENT;

    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(expected, &pointer);
    PARCURISegment *handle = parcURISegment_Acquire(segment);
    assertTrue(parcURISegment_Equals(segment, handle), "Expected URI segments to be equal: %s - %s", parcURISegment_ToString(segment), parcURISegment_ToString(handle));

    parcURISegment_Release(&segment);
    parcURISegment_Release(&handle);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Create)
{
    char *expected = URI_PATH_SEGMENT;

    PARCURISegment *segment = parcURISegment_Create(strlen(expected), (unsigned char *) expected);
    assertNotNull(segment, "Expected non-null result.");

    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Parse)
{
    char *expected = URI_PATH_SEGMENT;

    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(expected, &pointer);
    assertNotNull(segment, "Expected non-null result.");

    char *expectedBytes = URI_PATH_SEGMENT;

    char *actualBytes = parcURISegment_ToString(segment);

    assertTrue(strcmp(expectedBytes, actualBytes) == 0,
               "Expected %s actual %s", expectedBytes, actualBytes);
    parcMemory_Deallocate((void **) &actualBytes);

    assertTrue(parcURISegment_Length(segment) == 39,
               "Expected 39, actual %zd", parcURISegment_Length(segment));
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Parse_WithExtraSlashes)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT_WITH_SLASHES, &pointer);
    assertNotNull(segment, "Expected non-null result.");

    char *expectedBytes = URI_PATH_SEGMENT;

    char *actualBytes = parcURISegment_ToString(segment);

    assertTrue(strcmp(expectedBytes, actualBytes) == 0,
               "Expected %s actual %s", expectedBytes, actualBytes);
    parcMemory_Deallocate((void **) &actualBytes);

    assertTrue(parcURISegment_Length(segment) == 39,
               "Expected 39, actual %zd", parcURISegment_Length(segment));
    assertTrue(*pointer == '/', "Expected pointer to point to the slash character: %c", *pointer);

    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Parse_WithInvalidPercentage)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT "%G", &pointer);

    assertNull(segment, "Parsed segment should be NULL since the last percent-encoded byte is invalid");
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Release)
{
    char *expected = URI_PATH_SEGMENT;

    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(expected, &pointer);
    assertNotNull(segment, "Expected non-null result.");

    parcURISegment_Release(&segment);
    assertNull(segment, "Expected destroy to null the pointer");
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Equals_Contract)
{
    char *expected = URI_PATH_SEGMENT;

    const char *pointer;
    PARCURISegment *x = parcURISegment_Parse(expected, &pointer);
    PARCURISegment *y = parcURISegment_Parse(expected, &pointer);
    PARCURISegment *z = parcURISegment_Parse(expected, &pointer);

    parcObjectTesting_AssertEqualsFunction(parcURISegment_Equals, x, y, z, NULL);

    parcURISegment_Release(&x);
    parcURISegment_Release(&y);
    parcURISegment_Release(&z);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Clone)
{
    char *expected = URI_PATH_SEGMENT;

    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(expected, &pointer);
    PARCURISegment *copy = parcURISegment_Clone(segment);

    assertTrue(segment != copy, "Expected different instances of equal segments.");

    int comparison = parcURISegment_Compare(segment, copy);
    assertTrue(comparison == 0, "Expected equal segments.");

    assertTrue(parcURISegment_Equals(segment, copy), "Expected equal segments");

    parcURISegment_Release(&copy);
    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Length)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT, &pointer);
    assertNotNull(segment,
                  "Expected non-null result.");
    assertTrue(*pointer == 0,
               "Expected pointer to point to the null terminating byte.");

    size_t actual = parcURISegment_Length(segment);

    assertTrue(actual == 39,
               "Expected 39, actual %zd", actual);

    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_Compare_Contract)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse("MMM", &pointer);

    PARCURISegment *equivalents[] = {
        segment,
        parcURISegment_Parse("MMM",&pointer),
        NULL,
    };
    PARCURISegment *lessers[] = {
        parcURISegment_Parse("MM",  &pointer),
        parcURISegment_Parse("MML", &pointer),
        NULL,
    };
    PARCURISegment *greaters[] = {
        parcURISegment_Parse("MMMM", &pointer),
        parcURISegment_Parse("MMN",  &pointer),
        NULL,
    };
    parcObjectTesting_AssertCompareTo(parcURISegment_Compare, segment, equivalents, lessers, greaters);

    for (int i = 0; equivalents[i] != NULL; i++) {
        parcURISegment_Release(&equivalents[i]);
    }
    for (int i = 0; lessers[i] != NULL; i++) {
        parcURISegment_Release(&lessers[i]);
    }
    for (int i = 0; greaters[i] != NULL; i++) {
        parcURISegment_Release(&greaters[i]);
    }
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_ToString)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT, &pointer);
    assertNotNull(segment, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    char *actual = parcURISegment_ToString(segment);

    assertTrue(strcmp(URI_PATH_SEGMENT, actual) == 0, "Expected %s, actual %s", URI_PATH_SEGMENT, actual);

    parcURISegment_Release(&segment);

    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURISegment, parcURISegment_GetBuffer)
{
    const char *pointer;
    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT, &pointer);
    assertNotNull(segment, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    PARCBuffer *buffer = parcURISegment_GetBuffer(segment);

    char *expected = URI_PATH_SEGMENT;
    char *actual = (char *) parcBuffer_Overlay(buffer, 0);
    size_t compareLength = strlen(URI_PATH_SEGMENT);
    assertTrue(strncmp(expected, actual, compareLength), "Buffer does not contain original data.");

    parcURISegment_Release(&segment);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcURISegment);
    int status = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(status);
}
