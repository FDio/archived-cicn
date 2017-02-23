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
#include "../metis_TlvSkeleton.c"
#include <stdio.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

LONGBOW_TEST_RUNNER(metis_TlvSkeleton)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
   
    LONGBOW_RUN_TEST_FIXTURE(SchemaV1);
    LONGBOW_RUN_TEST_FIXTURE(Setters);
    LONGBOW_RUN_TEST_FIXTURE(Getters);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_TlvSkeleton)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_TlvSkeleton)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ===================================================

LONGBOW_TEST_FIXTURE(SchemaV1)
{
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_ComputeContentObjectHash);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_Skeleton_Interest);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_Skeleton_Object);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeInterest);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeContentObject);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeControl);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeInterestReturn);
    LONGBOW_RUN_TEST_CASE(SchemaV1, metisTlvSkeleton_TotalPacketLength);
}

LONGBOW_TEST_FIXTURE_SETUP(SchemaV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(SchemaV1)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static void
_schemaV1_verifyInterestPerHop(MetisTlvSkeleton *opaque)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    assertTrue(skeleton->array[INDEX_HOPLIMIT].offset == 4, "Incorrect hopLimit offset, expected %u got %u", 4, skeleton->array[INDEX_HOPLIMIT].offset);
    assertTrue(skeleton->array[INDEX_HOPLIMIT].length == 1, "Incorrect hopLimit length, expected %u got %u", 1, skeleton->array[INDEX_HOPLIMIT].length);
}

static void
_schemaV1_verifyInterestSkeleton(MetisTlvSkeleton *opaque)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    assertTrue(skeleton->array[INDEX_NAME].offset == 22, "Incorrect name offset, expected %u got %u", 2, skeleton->array[INDEX_NAME].offset);
    assertTrue(skeleton->array[INDEX_NAME].length == 8, "Incorrect name length, expected %u got %u", 8, skeleton->array[INDEX_NAME].length);

    assertTrue(skeleton->array[INDEX_KEYID].offset == 34, "Incorrect keyId offset, expected %u got %u", 34, skeleton->array[INDEX_KEYID].offset);
    assertTrue(skeleton->array[INDEX_KEYID].length == 16, "Incorrect keyId length, expected %u got %u", 16, skeleton->array[INDEX_KEYID].length);

    assertTrue(skeleton->array[INDEX_OBJHASH].offset == 54, "Incorrect objectHash offset, expected %u got %u", 54, skeleton->array[INDEX_OBJHASH].offset);
    assertTrue(skeleton->array[INDEX_OBJHASH].length == 32, "Incorrect objectHash length, expected %u got %u", 32, skeleton->array[INDEX_OBJHASH].length);

    assertTrue(skeleton->array[INDEX_INTLIFETIME].offset == 12, "Incorrect interestLifetime offset, expected %u got %u", 12, skeleton->array[INDEX_INTLIFETIME].offset);
    assertTrue(skeleton->array[INDEX_INTLIFETIME].length == 2, "Incorrect interestLifetime length, expected %u got %u", 2, skeleton->array[INDEX_INTLIFETIME].length);
}

static void
_schemaV1_verifyObjectSkeleton(MetisTlvSkeleton *opaque)
{
    _InternalSkeleton *skeleton = (_InternalSkeleton *) opaque;
    assertTrue(skeleton->array[INDEX_NAME].offset == 40, "Incorrect name offset, expected %u got %u", 40, skeleton->array[INDEX_NAME].offset);
    assertTrue(skeleton->array[INDEX_NAME].length == 17, "Incorrect name length, expected %u got %u", 17, skeleton->array[INDEX_NAME].length);

    assertTrue(skeleton->array[INDEX_KEYID].offset == 106, "Incorrect keyId offset, expected %u got %u", 106, skeleton->array[INDEX_KEYID].offset);
    assertTrue(skeleton->array[INDEX_KEYID].length == 32, "Incorrect keyId length, expected %u got %u", 32, skeleton->array[INDEX_KEYID].length);
}


LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_ComputeContentObjectHash)
{
    size_t endHeaders = metisTlv_TotalHeaderLength(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256);
    size_t endPacket = metisTlv_TotalPacketLength(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256);
    uint8_t *start = &metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256[endHeaders];


    size_t length = endPacket - endHeaders;

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, start, length);

    PARCCryptoHash *hash_truth = parcCryptoHasher_Finalize(hasher);

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256, logger);
    metisLogger_Release(&logger);

    PARCCryptoHash *hash_test = metisTlvSkeleton_ComputeContentObjectHash(&skeleton);

    assertTrue(parcCryptoHash_Equals(hash_truth, hash_test),
               "Content object digests did not match")
    {
        printf("Expected:\n");
        parcBuffer_Display(parcCryptoHash_GetDigest(hash_truth), 3);
        printf("Got:\n");
        parcBuffer_Display(parcCryptoHash_GetDigest(hash_test), 3);
    }

    parcCryptoHash_Release(&hash_truth);
    parcCryptoHash_Release(&hash_test);
    parcCryptoHasher_Release(&hasher);
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_Skeleton_Interest)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_Interest_AllFields, logger);
    metisLogger_Release(&logger);
    _schemaV1_verifyInterestPerHop(&skeleton);
    _schemaV1_verifyInterestSkeleton(&skeleton);
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_Skeleton_Object)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256, logger);
    metisLogger_Release(&logger);
    _schemaV1_verifyObjectSkeleton(&skeleton);
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeInterest)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_Interest_AllFields, logger);
    metisLogger_Release(&logger);
    bool match = metisTlvSkeleton_IsPacketTypeInterest(&skeleton);
    assertTrue(match, "Packet should have tested true as Interest");
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeContentObject)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256, logger);
    metisLogger_Release(&logger);
    bool match = metisTlvSkeleton_IsPacketTypeContentObject(&skeleton);
    assertTrue(match, "Packet should have tested true as Content Object");
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeControl)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_CPI_AddRoute_Crc32c, logger);
    metisLogger_Release(&logger);
    bool match = metisTlvSkeleton_IsPacketTypeControl(&skeleton);
    assertTrue(match, "Packet should have tested true as Control");
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_IsPacketTypeInterestReturn)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_CPI_AddRoute_Crc32c, logger);
    metisLogger_Release(&logger);
    bool match = metisTlvSkeleton_IsPacketTypeInterestReturn(&skeleton);
    assertFalse(match, "Packet should have tested false as Interest Return");
}

LONGBOW_TEST_CASE(SchemaV1, metisTlvSkeleton_TotalPacketLength)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    metisTlvSkeleton_Parse(&skeleton, metisTestDataV1_Interest_AllFields, logger);
    metisLogger_Release(&logger);
    size_t truth = sizeof(metisTestDataV1_Interest_AllFields);
    size_t test = metisTlvSkeleton_TotalPacketLength(&skeleton);

    assertTrue(truth == test, "Wrong value, expected %zu got %zu", truth, test);
}

// ======================================================

LONGBOW_TEST_FIXTURE(Setters)
{
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetName);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetKeyId);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetObjectHash);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetHopLimit);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetInterestLifetime);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetCacheTimeHeader);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetExpiryTime);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetCPI);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetFragmentPayload);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_UpdateHopLimit);

    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetKeyId);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetCertificate);
    LONGBOW_RUN_TEST_CASE(Setters, metisTlvSkeleton_SetPublicKey);
}

LONGBOW_TEST_FIXTURE_SETUP(Setters)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Setters)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetName)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_NAME;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetName(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetKeyId)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_KEYID;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetKeyId(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetObjectHash)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_OBJHASH;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetObjectHash(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetHopLimit)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 1;
    int element = INDEX_HOPLIMIT;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetHopLimit(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetInterestLifetime)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_INTLIFETIME;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetInterestLifetime(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetCacheTimeHeader)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_CACHETIME;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetCacheTimeHeader(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetExpiryTime)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_EXPIRYTIME;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetExpiryTime(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetCPI)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_CPI;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetCPI(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetFragmentPayload)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 4;
    int element = INDEX_FRAGMENTPAYLOAD;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetFragmentPayload(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_UpdateHopLimit)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 2;
    size_t length = 1;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetHopLimit(&opaque, offset, length);

    metisTlvSkeleton_UpdateHopLimit(&opaque, 77);

    assertTrue(packet[offset] == 77, "Wrong hop limit, expected 77 got %u", packet[offset]);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetCertificate)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 6;
    size_t length = 2;
    int element = INDEX_CERTIFICATE;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetCertificate(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u",
               element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u",
               element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Setters, metisTlvSkeleton_SetPublicKey)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    size_t offset = 5;
    size_t length = 3;
    int element = INDEX_PUBKEY;

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);

    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetPublicKey(&opaque, offset, length);

    assertTrue(skeleton->array[element].offset == offset, "Wrong offset for index %d, expected %zu got %u", element, offset, skeleton->array[element].offset);
    assertTrue(skeleton->array[element].length == length, "Wrong length for index %d, expected %zu got %u", element, length, skeleton->array[element].length);
    metisLogger_Release(&logger);
}

// ======================================================

LONGBOW_TEST_FIXTURE(Getters)
{
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetName);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetKeyId);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetObjectHash);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetHopLimit);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetInterestLifetime);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetCacheTimeHeader);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetExpiryTime);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetCPI);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetFragmentPayload);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetPacket);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetLogger);

    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetPublicKey);
    LONGBOW_RUN_TEST_CASE(Getters, metisTlvSkeleton_GetCertificate);
}

LONGBOW_TEST_FIXTURE_SETUP(Getters)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Getters)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetName)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetName(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetName(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetKeyId)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetKeyId(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetKeyId(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetObjectHash)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetObjectHash(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetObjectHash(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetHopLimit)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 1 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetHopLimit(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetHopLimit(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetInterestLifetime)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetInterestLifetime(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetInterestLifetime(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetCacheTimeHeader)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetCacheTimeHeader(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetCacheTimeHeader(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetExpiryTime)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetExpiryTime(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetExpiryTime(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetCPI)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetCPI(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetCPI(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetFragmentPayload)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 3, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);
    metisTlvSkeleton_SetFragmentPayload(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetFragmentPayload(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetPacket)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);

    const uint8_t *test = metisTlvSkeleton_GetPacket(&opaque);

    assertTrue(packet == test, "Wrong packet pointer, expected %p, got %p",
               (void *) packet, (void *) test);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetLogger)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);

    MetisLogger *test = metisTlvSkeleton_GetLogger(&opaque);
    assertNotNull(test, "Got null logger from skeleton");

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetPublicKey)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 5, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);

    metisTlvSkeleton_SetPublicKey(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetPublicKey(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Getters, metisTlvSkeleton_GetCertificate)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    MetisTlvExtent extent = { .offset = 5, .length = 2 };

    MetisTlvSkeleton opaque;
    _InternalSkeleton *skeleton = (_InternalSkeleton *) &opaque;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize(skeleton, &MetisTlvSchemaV1_Ops, packet, logger);

    metisTlvSkeleton_SetCertificate(&opaque, extent.offset, extent.length);

    MetisTlvExtent test = metisTlvSkeleton_GetCertificate(&opaque);

    assertTrue(metisTlvExtent_Equals(&extent, &test), "Wrong extent, expected {%u, %u}, got {%u, %u}",
               extent.offset, extent.length, test.offset, test.length);
    metisLogger_Release(&logger);
}



// ======================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_TlvSkeleton);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}



// ====================================


