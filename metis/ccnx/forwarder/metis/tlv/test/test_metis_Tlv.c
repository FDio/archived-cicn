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
#include "../metis_Tlv.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

LONGBOW_TEST_RUNNER(metis_Tlv)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Tlv)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Tlv)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_NameSegments);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_NameSegments_Realloc);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_ExtentToVarInt);

    LONGBOW_RUN_TEST_CASE(Global, metisTlv_FixedHeaderLength);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_TotalHeaderLength_V0);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_TotalHeaderLength_V1);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_TotalPacketLength_V0);
    LONGBOW_RUN_TEST_CASE(Global, metisTlv_TotalPacketLength_V1);

    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, metisTlv_EncodeControlPlaneInformation_V0);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, metisTlv_EncodeControlPlaneInformation_V1);
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


LONGBOW_TEST_CASE(Global, metisTlv_NameSegments)
{
    uint8_t name[] = {
        0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
        'h',  'e',  'l',  'l',
        'o',  // "hello"
        0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
        'o',  'u',  'c',  'h'
    };

    MetisTlvExtent *nameExtents;
    size_t nameExtentsLength;

    MetisTlvExtent truthExtents[] = { { .offset = 0, .length = 9 }, { .offset = 9, .length = 8 } };
    size_t truthExtentsLength = 2;

    metisTlv_NameSegments(name, sizeof(name), &nameExtents, &nameExtentsLength);

    assertTrue(nameExtentsLength == truthExtentsLength, "nameExtentsLength wrong, expected %zu got %zu", truthExtentsLength, nameExtentsLength);
    for (int i = 0; i < nameExtentsLength; i++) {
        assertTrue(truthExtents[i].offset == nameExtents[i].offset,
                   "nameExtents[%d].offset wrong, expected %u got %u",
                   i,
                   truthExtents[i].offset,
                   nameExtents[i].offset);

        assertTrue(truthExtents[i].length == nameExtents[i].length,
                   "nameExtents[%d].offset wrong, expected %u got %u",
                   i,
                   truthExtents[i].length,
                   nameExtents[i].length);
    }

    parcMemory_Deallocate((void **) &nameExtents);
}

/**
 * Create a name with enough name components to cause a re-alloc in the parser
 */
LONGBOW_TEST_CASE(Global, metisTlv_NameSegments_Realloc)
{
    uint8_t oneSegment[] = {
        0x00, 0x02, 0x00, 0x04,     // type = binary, length = 4
        'h',  'e',  'l', 'l'
    };

    // build a name with neededComponents copies of oneSegment such that it will
    // exceed the initialLengthForNameExtents allocation in the parser

    size_t neededComponents = _initialLengthForNameExtents + 2;
    size_t nameBufferLength = neededComponents * sizeof(oneSegment);

    uint8_t *nameBuffer = parcMemory_Allocate(nameBufferLength);
    assertNotNull(nameBuffer, "parcMemory_Allocate(%zu) returned NULL", nameBufferLength);
    for (int i = 0; i < neededComponents; i++) {
        memcpy(nameBuffer + i * sizeof(oneSegment), oneSegment, sizeof(oneSegment));
    }

    MetisTlvExtent *nameExtents;
    size_t nameExtentsLength;

    metisTlv_NameSegments(nameBuffer, nameBufferLength, &nameExtents, &nameExtentsLength);

    assertTrue(nameExtentsLength == neededComponents,
               "metisTlv_NameSegments returned wrong number of segments, expected %zu got %zu",
               neededComponents,
               nameExtentsLength);


    parcMemory_Deallocate((void **) &nameExtents);
    parcMemory_Deallocate((void **) &nameBuffer);
}

LONGBOW_TEST_CASE(Global, metisTlv_ExtentToVarInt)
{
    uint8_t packet[] = { 0xff, 0xff, 0x00, 0x01, 0x02, 0xff, 0xff };
    MetisTlvExtent extent = { 2, 3 };
    uint64_t truth = 0x0102;
    uint64_t test = 0;

    bool success = metisTlv_ExtentToVarInt(packet, &extent, &test);
    assertTrue(success, "Failed to parse a good extent");
    assertTrue(truth == test, "Wrong value, expected %#" PRIx64 "got %#" PRIx64, truth, test);
}

LONGBOW_TEST_CASE(Global, metisTlv_FixedHeaderLength)
{
    size_t test = metisTlv_FixedHeaderLength();
    assertTrue(test == 8, "Wrong fixed header length, got %zu", test);
}

LONGBOW_TEST_CASE(Global, metisTlv_TotalHeaderLength_V0)
{
    size_t test = metisTlv_TotalHeaderLength(metisTestDataV0_EncodedInterest);
    assertTrue(test == 29, "Wrong total header length, expected 29 got %zu", test);
}

LONGBOW_TEST_CASE(Global, metisTlv_TotalHeaderLength_V1)
{
    size_t test = metisTlv_TotalHeaderLength(metisTestDataV1_Interest_AllFields);
    assertTrue(test == 14, "Wrong total header length, expected 14 got %zu", test);
}

LONGBOW_TEST_CASE(Global, metisTlv_TotalPacketLength_V0)
{
    size_t test = metisTlv_TotalPacketLength(metisTestDataV0_EncodedInterest);
    assertTrue(test == sizeof(metisTestDataV0_EncodedInterest), "Wrong total header length, expected %zu got %zu", sizeof(metisTestDataV0_EncodedInterest), test);
}

LONGBOW_TEST_CASE(Global, metisTlv_TotalPacketLength_V1)
{
    size_t test = metisTlv_TotalPacketLength(metisTestDataV1_Interest_AllFields);
    assertTrue(test == sizeof(metisTestDataV1_Interest_AllFields), "Wrong total header length, expected %zu got %zu", sizeof(metisTestDataV1_Interest_AllFields), test);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, metisTlv_EncodeControlPlaneInformation_V0)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();

    PARCBuffer *buffer = metisTlv_EncodeControlPlaneInformation(control);
    ccnxControl_Release(&control);

    assertNotNull(buffer, "Got null encoding buffer");
    uint8_t *overlay = parcBuffer_Overlay(buffer, 0);

    assertTrue(overlay[1] == 0xA4, "PacketType is not Control");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, metisTlv_EncodeControlPlaneInformation_V1)
{
    // there's no easy way to test this right now, cannot contruct a v1 CCNxControl
}


// ===================================================

LONGBOW_TEST_FIXTURE(Local)
{
    // computeHash and parseName Auth are tested called through by other tests

    LONGBOW_RUN_TEST_CASE(Local, _metisTlv_ParseName);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _metisTlv_ParseName)
{
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Tlv);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
