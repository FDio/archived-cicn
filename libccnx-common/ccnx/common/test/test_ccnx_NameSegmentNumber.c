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
#include "../ccnx_NameSegmentNumber.c"

#include <LongBow/unit-test.h>

#include <stdio.h>
#include <inttypes.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(test_ccnx_NameSegmentNumber)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_ccnx_NameSegmentNumber)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_ccnx_NameSegmentNumber)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create64bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create56bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create48bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create40bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create32bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create24bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create16bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_Create8bits);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_BorderCases);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_IsValid_False);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegmentNumber_AssertValid);
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

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create64bits)
{
    uint64_t expected = 0x123456789ABCDEF0;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);

    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create56bits)
{
    uint64_t expected = 0x123456789ABCDE;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create48bits)
{
    uint64_t expected = 0x123456789ABC;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create40bits)
{
    uint64_t expected = 0x123456789A;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create32bits)
{
    uint64_t expected = 0x12345678;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create24bits)
{
    uint64_t expected = 0x123456;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create16bits)
{
    uint64_t expected = 0x1234;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_Create8bits)
{
    uint64_t expected = 0x12;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    uint64_t actual = ccnxNameSegmentNumber_Value(segment);

    assertTrue(expected == actual, "Expected 0x%" PRIX64 " actual 0x%" PRIX64 "", expected, actual);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_BorderCases)
{
    struct test_struct {
        uint64_t value;
        size_t length;
        uint8_t *encoded;
    } test_vector[] = {
        { .value = 0x0000000000000000ULL, .length = 1, .encoded = (uint8_t[1]) { 0x00 } },
        { .value = 0x0000000000000001ULL, .length = 1, .encoded = (uint8_t[1]) { 0x01 } },
        { .value = 0x00000000000000FFULL, .length = 1, .encoded = (uint8_t[1]) { 0xFF } },
        { .value = 0x0000000000000100ULL, .length = 2, .encoded = (uint8_t[2]) { 0x01, 0x00} },
        { .value = 0x0100000000000100ULL, .length = 8, .encoded = (uint8_t[8]) { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00} },
        { .value = 0x8000000000000100ULL, .length = 8, .encoded = (uint8_t[8]) { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00} },
        { .value = 0xFFFFFFFFFFFFFFFFULL, .length = 8, .encoded = (uint8_t[8]) { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} },
        { .value = 0,                     .length = 0, .encoded = NULL }
    };

    for (int i = 0; test_vector[i].encoded != NULL; i++) {
        PARCBuffer *buffer =
            parcBuffer_Wrap(test_vector[i].encoded, test_vector[i].length, 0, test_vector[i].length);
        CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);

        CCNxNameSegment *actual = ccnxNameSegmentNumber_Create(CCNxNameLabelType_NAME, test_vector[i].value);

        assertTrue(ccnxNameSegment_Equals(expected, actual),
                   "Buffers do not match: test_vector[%d] value %" PRIX64 " Expected %" PRIX64 " actual %" PRIX64 "",
                   i,
                   test_vector[i].value,
                   ccnxNameSegmentNumber_Value(expected),
                   ccnxNameSegmentNumber_Value(actual));

        ccnxNameSegment_Release(&expected);
        ccnxNameSegment_Release(&actual);
        parcBuffer_Release(&buffer);
    }
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_AssertValid)
{
    uint64_t expected = 0x12;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    ccnxNameSegmentNumber_AssertValid(segment);
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_IsValid)
{
    uint64_t expected = 0x12;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    assertTrue(ccnxNameSegmentNumber_IsValid(segment), "Expected the CCNxNameSegment to be valid.");
    ccnxNameSegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegmentNumber_IsValid_False)
{
    uint64_t expected = 0x12;
    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, expected);

    PARCBuffer *value = ccnxNameSegment_GetValue(segment);
    parcBuffer_SetPosition(value, parcBuffer_Limit(value)); // Wreck the buffer by making it zero length.

    assertFalse(ccnxNameSegmentNumber_IsValid(segment), "Expected the CCNxNameSegment to be valid.");
    ccnxNameSegment_Release(&segment);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_ccnx_NameSegmentNumber);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
