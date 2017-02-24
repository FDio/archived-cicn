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
#include "../metis_TlvSchemaV1.c"
#include "../metis_TlvSkeleton.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>


LONGBOW_TEST_RUNNER(metis_TlvSchemaV1)
{
    LONGBOW_RUN_TEST_FIXTURE(TlvOpsFunctions);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_TlvSchemaV1)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_TlvSchemaV1)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ======================================================

LONGBOW_TEST_FIXTURE(TlvOpsFunctions)
{
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_Interest);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_ContentObject);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_Control);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_InterestReturn);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_Unknown);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _parse_HopByHopFragment);

    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _computeContentObjectHash);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _encodeControlPlaneInformation);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _fixedHeaderLength);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _totalHeaderLength);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _totalPacketLength);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _isPacketTypeInterest);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _isPacketTypeContentObject);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _isPacketTypeInterestReturn);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _isPacketTypeControl);
    LONGBOW_RUN_TEST_CASE(TlvOpsFunctions, _isPacketTypeHopByHopFragment);
}

LONGBOW_TEST_FIXTURE_SETUP(TlvOpsFunctions)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(TlvOpsFunctions)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_Interest)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, metisTestDataV1_Interest_AllFields, logger);
    bool success = _parse(&skeleton);
    assertTrue(success, "_parse(Interest) did not succeed");

    // spot check
    {
        MetisTlvExtent trueExtent = { .offset = 4, .length = 1 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetHopLimit(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong hoplimit extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }

    {
        MetisTlvExtent trueExtent = { .offset = 12, .length = 2 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetInterestLifetime(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong interest lifetime extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }

    {
        MetisTlvExtent trueExtent = { .offset = 22, .length = 8 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetName(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong name extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }

    {
        MetisTlvExtent trueExtent = { .offset = 34, .length = 16 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetKeyId(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong keyid extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }

    {
        MetisTlvExtent trueExtent = { .offset = 54, .length = 32 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetObjectHash(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong objhash extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_ContentObject)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, metisTestDataV1_ContentObject_NameA_Crc32c, logger);
    bool success = _parse(&skeleton);
    assertTrue(success, "_parse(ContentObject) did not succeed");

    // spot check
    {
        MetisTlvExtent trueExtent = { .offset = 36, .length = 8 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetCacheTimeHeader(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong cache time extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }

    {
        MetisTlvExtent trueExtent = { .offset = 52, .length = 17 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetName(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong name extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_Control)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, metisTestDataV1_CPI_AddRoute_Crc32c, logger);
    bool success = _parse(&skeleton);
    assertTrue(success, "_parse(Control) did not succeed");

    // spot check
    {
        MetisTlvExtent trueExtent = { .offset = 12, .length = 154 };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetCPI(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong CPI extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_HopByHopFragment)
{
    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, metisTestDataV1_HopByHopFrag_BeginEnd, logger);
    bool success = _parse(&skeleton);
    assertTrue(success, "_parse(Control) did not succeed");

    // spot check
    {
        MetisTlvExtent trueExtent = { .offset = 12, .length = sizeof(metisTestDataV1_HopByHopFrag_BeginEnd_Fragment) };
        MetisTlvExtent testExtent = metisTlvSkeleton_GetFragmentPayload(&skeleton);
        assertTrue(metisTlvExtent_Equals(&trueExtent, &testExtent), "Wrong fragment payload extent, expected {%u, %u} got {%u, %u}",
                   trueExtent.offset, trueExtent.length, testExtent.offset, testExtent.length);
    }
    metisLogger_Release(&logger);
}


LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_InterestReturn)
{
    // not implemented yet
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _parse_Unknown)
{
    uint8_t unknown[] = { 0x01, 0x77, 0x00, 8, 0x00, 0x00, 0x00, 8 };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, unknown, logger);
    bool success = _parse(&skeleton);
    assertFalse(success, "_parse(Unknown) should have failed");
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _computeContentObjectHash)
{
    _MetisTlvFixedHeaderV1 *hdr = (_MetisTlvFixedHeaderV1 *) metisTestDataV1_ContentObject_NameA_Crc32c;
    size_t endHeaders = _totalHeaderLength(metisTestDataV1_ContentObject_NameA_Crc32c);
    size_t endPacket = _totalPacketLength((uint8_t *) hdr);

    uint8_t *start = &metisTestDataV1_ContentObject_NameA_Crc32c[endHeaders];
    size_t length = endPacket - endHeaders;

    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, start, length);

    PARCCryptoHash *hash_truth = parcCryptoHasher_Finalize(hasher);

    PARCCryptoHash *hash_test = _computeContentObjectHash(metisTestDataV1_ContentObject_NameA_Crc32c);

    assertTrue(parcCryptoHash_Equals(hash_truth, hash_test),
               "Content object digests did not match")
    {
        parcBuffer_Display(parcCryptoHash_GetDigest(hash_truth), 3),
        parcBuffer_Display(parcCryptoHash_GetDigest(hash_test), 3);
    }

    parcCryptoHash_Release(&hash_truth);
    parcCryptoHash_Release(&hash_test);
    parcCryptoHasher_Release(&hasher);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _encodeControlPlaneInformation)
{
    CCNxControl *control = ccnxControl_CreateRouteListRequest();

    PARCBuffer *buffer = _encodeControlPlaneInformation(control);
    ccnxControl_Release(&control);

    assertNotNull(buffer, "Got null encoding buffer");
    uint8_t *overlay = parcBuffer_Overlay(buffer, 0);

    assertTrue(_isPacketTypeControl(overlay), "PacketType is not Control");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _fixedHeaderLength)
{
    uint8_t packet[] = { 1, 2, 3, 4, 5 };
    size_t test = _fixedHeaderLength(packet);
    assertTrue(test == sizeof(_MetisTlvFixedHeaderV1), "wrong fixed header lenght, expected %zu got %zu", sizeof(_MetisTlvFixedHeaderV1), test);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _totalHeaderLength)
{
    size_t test = _totalHeaderLength(metisTestDataV1_ContentObject_NameA_Crc32c);
    assertTrue(test == metisTestDataV1_ContentObject_NameA_Crc32c[7], "Wrong header length, expected %u got %zu", metisTestDataV1_ContentObject_NameA_Crc32c[7], test);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _totalPacketLength)
{
    size_t test = _totalPacketLength(metisTestDataV1_ContentObject_NameA_Crc32c);
    assertTrue(test == sizeof(metisTestDataV1_ContentObject_NameA_Crc32c), "Wrong packet length, expected %zu got %zu", sizeof(metisTestDataV1_ContentObject_NameA_Crc32c), test);
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _isPacketTypeInterest)
{
    bool match = _isPacketTypeInterest(metisTestDataV1_Interest_AllFields);
    assertTrue(match, "Interest did not match");
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _isPacketTypeContentObject)
{
    bool match = _isPacketTypeContentObject(metisTestDataV1_ContentObject_NameA_Crc32c);
    assertTrue(match, "Content object did not match");
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _isPacketTypeInterestReturn)
{
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _isPacketTypeControl)
{
    bool match = _isPacketTypeControl(metisTestDataV1_CPI_AddRoute_Crc32c);
    assertTrue(match, "Control did not match");
}

LONGBOW_TEST_CASE(TlvOpsFunctions, _isPacketTypeHopByHopFragment)
{
    bool match = _isPacketTypeHopByHopFragment(metisTestDataV1_HopByHopFrag_Begin);
    assertTrue(match, "HopByHop Fragment did not match");
}


// ======================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _parsePerHopV1);
    LONGBOW_RUN_TEST_CASE(Local, _parseSignatureParameters);
    LONGBOW_RUN_TEST_CASE(Local, _parseSignatureParameters_NoKeyid);
    LONGBOW_RUN_TEST_CASE(Local, _parseValidationType);
    LONGBOW_RUN_TEST_CASE(Local, _parseValidationType_NotSignature);
    LONGBOW_RUN_TEST_CASE(Local, _parseValidationAlg);
    LONGBOW_RUN_TEST_CASE(Local, _parseObjectV1);
    LONGBOW_RUN_TEST_CASE(Local, _parseInterestV1);
    LONGBOW_RUN_TEST_CASE(Local, _parseMessage);
    LONGBOW_RUN_TEST_CASE(Local, _computeHash);
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

LONGBOW_TEST_CASE(Local, _parsePerHopV1)
{
}

LONGBOW_TEST_CASE(Local, _parseSignatureParameters)
{
    uint8_t encoded[] = {
        0x00, T_KEYID,     0x00, 6,
        0xa0, 0xa1,        0xa2, 0xa3,0xa4,0xa5,

        0x00, T_PUBLICKEY, 0x00, 8,
        0xb1, 0xb2,        0xb3, 0xb4,
        0xb5, 0xb6,        0xb7, 0xb8,

        0x00, T_CERT,      0x00, 8,
        0xc1, 0xc2,        0xc3, 0xc4,
        0xc5, 0xc6,        0xc7, 0xc8,

        0x00, 0xFF,        0x00, 2,
        0xb0, 0xb1
    };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, encoded, logger);
    _parseSignatureParameters(encoded, 0, sizeof(encoded), &skeleton);

    MetisTlvExtent truth = { .offset = 4, .length = 6 };
    MetisTlvExtent keyid = metisTlvSkeleton_GetKeyId(&skeleton);
    assertTrue(metisTlvExtent_Equals(&truth, &keyid), "Wrong extent, expected {%u, %u} got {%u, %u}",
               truth.offset, truth.length, keyid.offset, keyid.length);

    // Check the public key was found.
    MetisTlvExtent pubKeyTruth = { .offset = 14, .length = 8 };
    MetisTlvExtent pubKey = metisTlvSkeleton_GetPublicKey(&skeleton);
    assertTrue(metisTlvExtent_Equals(&pubKeyTruth, &pubKey), "Wrong extent, expected {%u, %u} got {%u, %u}",
               pubKeyTruth.offset, pubKeyTruth.length, pubKey.offset, pubKey.length);

    // Check that the cert was found.
    MetisTlvExtent certTruth = { .offset = 26, .length = 8 };
    MetisTlvExtent cert = metisTlvSkeleton_GetCertificate(&skeleton);
    assertTrue(metisTlvExtent_Equals(&certTruth, &cert), "Wrong extent, expected {%u, %u} got {%u, %u}",
               certTruth.offset, certTruth.length, cert.offset, cert.length);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Local, _parseSignatureParameters_NoKeyid)
{
    uint8_t encoded[] = {
        0x00, 0xFF, 0x00, 2,
        0xb0, 0xb1
    };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, encoded, logger);
    _parseSignatureParameters(encoded, 0, sizeof(encoded), &skeleton);

    MetisTlvExtent keyid = metisTlvSkeleton_GetKeyId(&skeleton);
    assertTrue(metisTlvExtent_Equals(&metisTlvExtent_NotFound, &keyid), "Wrong extent, expected {%u, %u} got {%u, %u}",
               metisTlvExtent_NotFound.offset, metisTlvExtent_NotFound.length, keyid.offset, keyid.length);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Local, _parseValidationType)
{
    uint8_t encoded[] = {
        0x00, T_RSA_SHA256, 0x00, 10,
        0x00, T_KEYID,      0x00, 6,
        0xa0, 0xa1,         0xa2, 0xa3,0xa4,0xa5,
        0x00, 0xFF,         0x00, 2,
        0xb0, 0xb1
    };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, encoded, logger);
    _parseValidationType(encoded, 0, sizeof(encoded), &skeleton);

    MetisTlvExtent truth = { .offset = 8, .length = 6 };
    MetisTlvExtent keyid = metisTlvSkeleton_GetKeyId(&skeleton);
    assertTrue(metisTlvExtent_Equals(&truth, &keyid), "Wrong extent, expected {%u, %u} got {%u, %u}",
               truth.offset, truth.length, keyid.offset, keyid.length);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Local, _parseValidationType_NotSignature)
{
    uint8_t encoded[] = {
        0x00, 0xFF,    0x00, 10,
        0x00, T_KEYID, 0x00, 6,
        0xa0, 0xa1,    0xa2, 0xa3,0xa4,0xa5,
        0x00, 0xFF,    0x00, 2,
        0xb0, 0xb1
    };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, encoded, logger);
    _parseValidationType(encoded, 0, sizeof(encoded), &skeleton);

    MetisTlvExtent keyid = metisTlvSkeleton_GetKeyId(&skeleton);
    assertTrue(metisTlvExtent_Equals(&metisTlvExtent_NotFound, &keyid), "Wrong extent, expected {%u, %u} got {%u, %u}",
               metisTlvExtent_NotFound.offset, metisTlvExtent_NotFound.length, keyid.offset, keyid.length);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Local, _parseValidationAlg)
{
}

LONGBOW_TEST_CASE(Local, _parseObjectV1)
{
    uint8_t encoded[] = {
        0x00, 0x00,         0x00, 8, // type = name, length = 8
        0x00, 0x02,         0x00, 4, // type = binary, length = 4
        'c',  'o',          'o', 'l', // "cool"

        0x00, T_EXPIRYTIME, 0x00, 2,        // type = name, length = 2
        0xa0, 0xa1
    };

    MetisTlvSkeleton skeleton;
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    _initialize((_InternalSkeleton *) &skeleton, &MetisTlvSchemaV1_Ops, encoded, logger);
    _parseObjectV1(encoded, 0, sizeof(encoded), &skeleton);

    MetisTlvExtent truth = { .offset = 16, .length = 2 };
    MetisTlvExtent expiryTime = metisTlvSkeleton_GetExpiryTime(&skeleton);
    assertTrue(metisTlvExtent_Equals(&truth, &expiryTime), "Wrong extent, expected {%u, %u} got {%u, %u}",
               truth.offset, truth.length, expiryTime.offset, expiryTime.length);

    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Local, _parseInterestV1)
{
}

LONGBOW_TEST_CASE(Local, _parseMessage)
{
}

LONGBOW_TEST_CASE(Local, _computeHash)
{
}

// ======================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_TlvSchemaV1);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}



// ====================================



