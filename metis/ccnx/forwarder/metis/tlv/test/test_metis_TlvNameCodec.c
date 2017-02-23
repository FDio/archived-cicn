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
#include "../metis_TlvNameCodec.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(tlv_NameCodec)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(tlv_NameCodec)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(tlv_NameCodec)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, tlvName_Decode_0_Length_Name);
    LONGBOW_RUN_TEST_CASE(Global, tlvName_Decode_0_Length_Segment);
    LONGBOW_RUN_TEST_CASE(Global, tlvName_Decode_Good);
    LONGBOW_RUN_TEST_CASE(Global, tlvName_Decode_Overflow);
    LONGBOW_RUN_TEST_CASE(Global, tlvName_Decode_UnderRun);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);

        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Buffer is 1 .. 3 bytes
 */
LONGBOW_TEST_CASE_EXPECTS(Global, tlvName_Decode_UnderRun, .event = &LongBowTrapIllegalValue)
{
    // offset               0     1     2     3     4     5     6     7     8   9   10   11   12     13
    //                        |-- type --|-- length --||
    uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x04, 0xFF };

    // This will assert
    // CCNxName *name =
    metisTlvNameCodec_Decode(buffer, 5, 6);
}

/**
 * Buffer exactly 0 bytes
 */
LONGBOW_TEST_CASE(Global, tlvName_Decode_0_Length_Name)
{
    // offset               0     1     2     3     4     5     6     7     8   9   10   11   12     13
    //                        |-- type --|-- length --||
    uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x04, 0xFF };

    // skip the two 0xFF bytes
    // name = "lci:/%02=abcd"
    CCNxName *test = metisTlvNameCodec_Decode(buffer, 5, 5);
    CCNxName *truth = ccnxName_Create();
    char *nameString = ccnxName_ToString(test);

    assertTrue(ccnxName_Equals(truth, test), "Names not equal, got %s", nameString);

    parcMemory_Deallocate((void **) &nameString);
    ccnxName_Release(&truth);
    ccnxName_Release(&test);
}

/**
 * Buffer exactly 4 bytes
 */
LONGBOW_TEST_CASE(Global, tlvName_Decode_0_Length_Segment)
{
    // offset               0     1     2     3     4     5     6     7     8   9   10   11   12     13
    //                        |-- type --|-- length --|-- type --|-- length --||
    uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0xFF };

    // skip the two 0xFF bytes
    // name = "lci:/%02=abcd"
    CCNxName *test = metisTlvNameCodec_Decode(buffer, 5, 9);
//    CCNxName *truth = ccnxName_CreateFromCString("lci:/%02=");
    CCNxName *truth = ccnxName_CreateFromCString("lci:/2=");
    char *nameString = ccnxName_ToString(test);

    assertTrue(ccnxName_Equals(truth, test), "Names not equal, got %s", nameString);

    parcMemory_Deallocate((void **) &nameString);
    ccnxName_Release(&truth);
    ccnxName_Release(&test);
}

/**
 * A good, normal name
 */
LONGBOW_TEST_CASE(Global, tlvName_Decode_Good)
{
    // offset       0     1     2     3     4     5     6     7     8   9   10   11   12     13
    //                |-- type --|-- length --|-- type --|-- length --| ----- value -----|
    uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x04, 'a', 'b', 'c', 'd', 0xFF };

    // skip the two 0xFF bytes
    // name = "lci:/%02=abcd"
    CCNxName *test = metisTlvNameCodec_Decode(buffer, 5, 13);

//    CCNxName *truth = ccnxName_CreateFromCString("lci:/%02=abcd");
    CCNxName *truth = ccnxName_CreateFromCString("lci:/2=abcd");
    char *nameString = ccnxName_ToString(test);

    assertTrue(ccnxName_Equals(truth, test), "Names not equal, got %s", nameString);

    parcMemory_Deallocate((void **) &nameString);
    ccnxName_Release(&truth);
    ccnxName_Release(&test);
}

/**
 * The name component length shoots beyond the end of the buffer.  Byte 8 is "5" instead of "4".
 */
LONGBOW_TEST_CASE_EXPECTS(Global, tlvName_Decode_Overflow, .event = &LongBowTrapIllegalValue)
{
    // offset               0     1     2     3     4     5     6     7     8   9   10   11   12     13
    //                        |-- type --|-- length --|-- type --|-- length --| ----- value -----|
    uint8_t buffer[] = { 0xFF, 0x00, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x05, 'a', 'b', 'c', 'd', 0xFF };

    // This will trap because the length 5 will go beyond 12
    metisTlvNameCodec_Decode(buffer, 5, 13);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(tlv_NameCodec);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
