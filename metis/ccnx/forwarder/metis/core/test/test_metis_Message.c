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
#include "../metis_Message.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <parc/logging/parc_LogReporterTextStdout.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

LONGBOW_TEST_RUNNER(metis_Message)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Message)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Message)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Create_InterestV0);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Create_ObjectV0);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Create_InterestV1);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Create_ObjectV1);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_CreateFromBuffer);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_CreateFromArray);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_CreateFromElasticBuffer);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_CreateFromBuffer_BadMessage);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_CreateFromArray_BadMessage);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Length);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Append);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Write);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetConnectionId);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetReceiveTime);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ReadFromBuffer);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_Copy);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetMessageType);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetName);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasName_True);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasName_False);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetKeyIdHash);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasKeyId_True);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasKeyId_False);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_KeyIdEquals_IsEqual);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_KeyIdEquals_DifferentLength);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_KeyIdEquals_DifferentValue);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsEqual_Precomputed);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsEqual_Lazy);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsNotEqual);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ObjectHashHashCode_Precomputed);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_ObjectHashHashCode_Lazy);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasContentObjectHash_True);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasContentObjectHash_False);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasHopLimit_True);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasHopLimit_False);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetHopLimit);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_SetHopLimit);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasInterestLifetime);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_GetInterestLifetimeTicks);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasExpirationTime);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasRecommendedCacheTime);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_SetGetExpirationTime);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_SetGetRecommendedCacheTime);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasGetPublicKey);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_IsPublicKeyVerified_True);
    LONGBOW_RUN_TEST_CASE(Global, metisMessage_IsPublicKeyVerified_False);

    LONGBOW_RUN_TEST_CASE(Global, metisMessage_HasGetCertificate);
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

LONGBOW_TEST_CASE(Global, metisMessage_Create_InterestV0)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest));
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_Create_ObjectV0)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_Create_InterestV1)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields));
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_Create_ObjectV1)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV1_ContentObject_NameA_Crc32c, sizeof(metisTestDataV1_ContentObject_NameA_Crc32c));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}


LONGBOW_TEST_CASE(Global, metisMessage_CreateFromArray)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);


    assertNotNull(message, "Got null from metisMessage_CreateFromArray");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_CreateFromArray_BadMessage)
{
    // Invalid version
    char message_str[] = "\xFFOnce upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";
    printf("metisMessage_CreateFromArray_BadMessage attempting to process a bad message which should report an error:\n");
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray((uint8_t *) message_str, sizeof(message_str), 1, 2, logger);
    metisLogger_Release(&logger);

    assertNull(message, "Got null from metisMessage_CreateFromArray");
}

LONGBOW_TEST_CASE(Global, metisMessage_CreateFromBuffer)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_CreateFromElasticBuffer)
{
    char message_str[] = "\0x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCBuffer *buff = parcBuffer_Wrap(message_str, sizeof(message_str), 0, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromParcBuffer(buff, 1, 2, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);

    metisMessage_Release(&message);
    parcBuffer_Release(&buff);
}

LONGBOW_TEST_CASE(Global, metisMessage_CreateFromBuffer_BadMessage)
{
    // Bad version
    char message_str[] = "\xFFOnce upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    printf("metisMessage_CreateFromBuffer_BadMessage attempting to process a bad message which should report an error:\n");
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNull(message, "Got null from metisMessage_CreateFromBuffer");
}

LONGBOW_TEST_CASE(Global, metisMessage_ReadFromBuffer)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_ReadFromBuffer(1, 2, buff, sizeof(message_str), logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");

    assertTrue(parcEventBuffer_GetLength(message->messageBytes) == sizeof(message_str),
               "Length of internal buffer wrong, expected %zu got %zu",
               sizeof(message_str),
               parcEventBuffer_GetLength(message->messageBytes));

    uint8_t *p = parcEventBuffer_Pullup(message->messageBytes, sizeof(message_str));
    assertTrue(memcmp(p, message_str, sizeof(message_str)) == 0, "Internal buffer contents does not match test");
    assertTrue(message->ingressConnectionId == 1, "IngressConnectionId wrong, expected %d got %u", 1, message->ingressConnectionId);
    assertTrue(message->receiveTime == 2, "receiveTime wrong, expected %u got %" PRIu64, 2, message->receiveTime);
    assertTrue(parcEventBuffer_GetLength(buff) == 0, "Origin buffer not drained, expected 0, got %zu", parcEventBuffer_GetLength(buff));

    metisMessage_Release(&message);
    parcEventBuffer_Destroy(&buff);
}

LONGBOW_TEST_CASE(Global, metisMessage_Append)
{
    char message_str[] = "\x00Once upon a time ...";

    PARCEventBuffer *buffer = parcEventBuffer_Create();
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    int result = metisMessage_Append(buffer, message);

    assertTrue(result == 0, "Got error from metisMessage_Append");
    metisLogger_Release(&logger);
    metisMessage_Release(&message);
    parcEventBuffer_Destroy(&buffer);
}

LONGBOW_TEST_CASE(Global, metisMessage_Write)
{
    char message_str[] = "\x00Once upon a time ...";

    PARCEventScheduler *scheduler = parcEventScheduler_Create();
    PARCEventQueue *queue = parcEventQueue_Create(scheduler, -1, PARCEventQueueOption_CloseOnFree);

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    int result = metisMessage_Write(queue, message);

    assertTrue(result == 0, "Got error from metisMessage_Write");

    // buff is deallocated by metisMessage_Release
    metisLogger_Release(&logger);
    metisMessage_Release(&message);
    parcEventQueue_Destroy(&queue);
    parcEventScheduler_Destroy(&scheduler);
}

LONGBOW_TEST_CASE(Global, metisMessage_Length)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    size_t length = metisMessage_Length(message);
    assertTrue(length == sizeof(message_str), "Wrong length, expected %zu got %zu", sizeof(message_str), length);
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetConnectionId)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    unsigned connid = metisMessage_GetIngressConnectionId(message);

    assertTrue(connid == 1, "Wrong length, expected %u got %u", 1, connid);
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetReceiveTime)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    MetisTicks time = metisMessage_GetReceiveTime(message);

    assertTrue(time == 2, "Wrong receive time, expected %u got %" PRIu64, 2, time);
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_Copy)
{
    char message_str[] = "\x00Once upon a time, in a stack far away, a dangling pointer found its way to the top of the heap.";

    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, message_str, sizeof(message_str));

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertNotNull(message, "Got null from metisMessage_CreateFromBuffer");
    assertTrue(message->refcount == 1, "Incorrect refcount, expected %u got %u", 1, message->refcount);

    MetisMessage *copy = metisMessage_Acquire(message);
    assertTrue(message->refcount == 2, "Incorrect refcount, expected %u got %u", 2, message->refcount);

    metisMessage_Release(&message);
    assertTrue(copy->refcount == 1, "Incorrect refcount, expected %u got %u", 1, message->refcount);

    metisMessage_Release(&copy);

    assertTrue(parcMemory_Outstanding() == 0, "Memory balance should be zero after destroying last copy, got %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisMessage_GetMessageType)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisMessagePacketType type = metisMessage_GetType(message);

    assertTrue(type == MetisMessagePacketType_ContentObject, "wrong type, expected %u got %u", MetisMessagePacketType_ContentObject, type);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetName)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);
    MetisTlvName *name = metisMessage_GetName(message);
    MetisTlvName *truth = metisTlvName_Create(&metisTestDataV0_EncodedObject[metisTestDataV0_EncodedObject_name.offset], metisTestDataV0_EncodedObject_name.length);

    assertTrue(metisTlvName_Equals(truth, name), "Did not get back the right name");

    metisTlvName_Release(&truth);
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasName_True)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasName = metisMessage_HasName(message);
    assertTrue(hasName, "Message with a name says it does not");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasName_False)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_CPIMessage, sizeof(metisTestDataV0_CPIMessage), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasName = metisMessage_HasName(message);
    assertFalse(hasName, "Message without a name says it does");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasContentObjectHash_True)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasHash = metisMessage_HasContentObjectHash(message);
    assertTrue(hasHash, "Message with a content object hash says it does not");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasContentObjectHash_False)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasHash = metisMessage_HasContentObjectHash(message);
    assertTrue(hasHash, "Message without a content object hash says it does");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetKeyIdHash)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    uint32_t truthhash = parcHash32_Data(&metisTestDataV0_EncodedObject[metisTestDataV0_EncodedObject_keyid.offset], metisTestDataV0_EncodedObject_keyid.length);
    uint32_t testhash;
    bool success = metisMessage_GetKeyIdHash(message, &testhash);

    assertTrue(success, "Failed metisMessage_GetKeyIdHash, returned false");
    assertTrue(truthhash == testhash, "Hash compared wrong, expected %08X, got %08X", truthhash, testhash);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasKeyId_True)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasKeyId = metisMessage_HasKeyId(message);
    assertTrue(hasKeyId, "Message with a keyid says it does not");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasKeyId_False)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    metisLogger_Release(&logger);
    bool hasKeyId = metisMessage_HasKeyId(message);
    assertFalse(hasKeyId, "Message without a keyid says it does");
    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_KeyIdEquals_IsEqual)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    assertTrue(metisMessage_KeyIdEquals(a, b), "Messages with equal keyids did not compare");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_KeyIdEquals_DifferentLength)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);
    metisLogger_Release(&logger);

    assertFalse(metisMessage_KeyIdEquals(a, b), "Messages with differnt length keyids did compared equal");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_KeyIdEquals_DifferentValue)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_SecondInterest, sizeof(metisTestDataV0_SecondInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    assertFalse(metisMessage_KeyIdEquals(a, b), "Messages with differnt keyids did compared equal");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsEqual_Precomputed)
{
    // create messages from Interests, as those are precomputed
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    assertTrue(metisMessage_ObjectHashEquals(a, b), "Messages with equal ContentObjectHash did not compare");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsEqual_Lazy)
{
    // create messages from content objects, as those are lazy computed
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    assertTrue(metisMessage_ObjectHashEquals(a, b), "Messages with equal ContentObjectHash did not compare");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_ObjectHashEquals_IsNotEqual)
{
    // create messages from different content objects
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    MetisMessage *b = metisMessage_CreateFromArray(metisTestDataV0_SecondObject, sizeof(metisTestDataV0_SecondObject), 1, 2, logger);
    metisLogger_Release(&logger);

    assertFalse(metisMessage_ObjectHashEquals(a, b), "Messages with unequal ContentObjectHash compared as equal");
    metisMessage_Release(&a);
    metisMessage_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisMessage_ObjectHashHashCode_Precomputed)
{
    // create messages from Interests, as those are precomputed
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    uint32_t hashcode;
    bool success = metisMessage_GetContentObjectHashHash(a, &hashcode);
    assertTrue(success, "Returned false trying to get hash of contentobject hash");

    metisMessage_Release(&a);
}

LONGBOW_TEST_CASE(Global, metisMessage_ObjectHashHashCode_Lazy)
{
    // create messages from content object, as those are precomputed
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *a = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    uint32_t hashcode;
    bool success = metisMessage_GetContentObjectHashHash(a, &hashcode);
    assertTrue(success, "Returned false trying to get hash of contentobject hash");

    metisMessage_Release(&a);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasHopLimit_True)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    bool hasHopLimit = metisMessage_HasHopLimit(message);
    assertTrue(hasHopLimit, "Message with a hop limit says it does not.");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasHopLimit_False)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_no_hoplimit, sizeof(metisTestDataV0_EncodedInterest_no_hoplimit), 1, 2, logger);
    metisLogger_Release(&logger);

    bool hasHopLimit = metisMessage_HasHopLimit(message);
    assertFalse(hasHopLimit, "Message without a hop limit says it does.");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetHopLimit)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    uint8_t hoplimit = metisMessage_GetHopLimit(message);
    assertTrue(hoplimit == 32, "Wrong hop limit, got %u expected %u.", hoplimit, 32);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_SetHopLimit)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);
    metisLogger_Release(&logger);

    metisMessage_SetHopLimit(message, 99);
    uint8_t hoplimit = metisMessage_GetHopLimit(message);
    assertTrue(hoplimit == 99, "Wrong hop limit, got %u expected %u.", hoplimit, 99);

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasInterestLifetime)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields));
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    assertTrue(metisMessage_HasInterestLifetime(message), "Should have returned true for interest lifetime");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_GetInterestLifetimeTicks)
{
    PARCEventBuffer *buff = parcEventBuffer_Create();
    parcEventBuffer_Append(buff, metisTestDataV1_Interest_AllFields, sizeof(metisTestDataV1_Interest_AllFields));
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisMessage *message = metisMessage_CreateFromBuffer(1, 2, buff, logger);
    metisLogger_Release(&logger);

    // don't check actual value.  It will vary based on METISHZ and rouding errors due to integer math
    MetisTicks ticks = metisMessage_GetInterestLifetimeTicks(message);
    assertTrue(ticks > 0, "Should have gotten positive value for interest lifetime ticks");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasExpirationTime)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    // Note: Assumes metisTestDataV0_EncodedObject doesn't have ExpiryTime.
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    bool hasExpiryTime = metisMessage_HasExpiryTime(message);
    assertFalse(hasExpiryTime, "Message without ExpiryTime says it has one.");

    metisMessage_SetExpiryTimeTicks(message, 10000);
    hasExpiryTime = metisMessage_HasExpiryTime(message);
    assertTrue(hasExpiryTime, "Message with ExpiryTime says it doesn't have one.");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasRecommendedCacheTime)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    // Note: Assumes metisTestDataV0_EncodedObject doesn't have RCT.
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    bool hasRCT = metisMessage_HasRecommendedCacheTime(message);
    assertFalse(hasRCT, "Message without hasRCT says it has one.");

    metisMessage_SetRecommendedCacheTimeTicks(message, 10000);
    hasRCT = metisMessage_HasRecommendedCacheTime(message);
    assertTrue(hasRCT, "Message with hasRCT says it doesn't have one.");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_SetGetExpirationTime)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    // Note: Assumes metisTestDataV0_EncodedObject doesn't have RCT.
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    uint64_t time = 12345;
    metisMessage_SetExpiryTimeTicks(message, time);
    assertTrue(time == metisMessage_GetExpiryTimeTicks(message), "Retrieved unexpected ExpiryTime");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_SetGetRecommendedCacheTime)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    // Note: Assumes metisTestDataV0_EncodedObject doesn't have RCT.
    MetisMessage *message = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisLogger_Release(&logger);

    uint64_t time = 12345;
    metisMessage_SetRecommendedCacheTimeTicks(message, time);
    assertTrue(time == metisMessage_GetRecommendedCacheTimeTicks(message), "Retrieved unexpected RCT");

    metisMessage_Release(&message);
}

LONGBOW_TEST_CASE(Global, metisMessage_HasGetPublicKey)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    MetisMessage *contentWithKey =
        metisMessage_CreateFromArray(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256,
                                     sizeof(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256), 1, 2, logger);

    MetisMessage *interestWithKeyIdRestriction =
        metisMessage_CreateFromArray(metisTestDataV1_Interest_NameAAndKeyId,
                                     sizeof(metisTestDataV1_Interest_NameAAndKeyId), 1, 2, logger);

    metisLogger_Release(&logger);

    assertTrue(metisMessage_HasPublicKey(contentWithKey), "Expected to see a public key");
    assertFalse(metisMessage_HasPublicKey(interestWithKeyIdRestriction), "Expected to not see a public key");

    PARCBuffer *key = metisMessage_GetPublicKey(contentWithKey);

    assertNotNull(key, "Expected to retrieve the public key");

    metisMessage_Release(&contentWithKey);
    metisMessage_Release(&interestWithKeyIdRestriction);
}

LONGBOW_TEST_CASE(Global, metisMessage_IsPublicKeyVerified_False)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);

    MetisMessage *contentWithKey =
        metisMessage_CreateFromArray(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256,
                                     sizeof(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256), 1, 2, logger);

    MetisMessage *interestWithKeyIdRestriction =
        metisMessage_CreateFromArray(metisTestDataV1_Interest_NameAAndKeyId,
                                     sizeof(metisTestDataV1_Interest_NameAAndKeyId), 1, 2, logger);

    metisLogger_Release(&logger);

    assertFalse(metisMessage_IsKeyIdVerified(contentWithKey), "Expected key to not be verified.");

    // This is an interest. The keyId is actually a KeyId restriction, so will never be verified.
    assertFalse(metisMessage_IsKeyIdVerified(interestWithKeyIdRestriction), "Expected key to not be verified.");

    PARCBuffer *key = metisMessage_GetPublicKey(contentWithKey);

    assertNotNull(key, "Expected to retrieve the public key");

    metisMessage_Release(&contentWithKey);
    metisMessage_Release(&interestWithKeyIdRestriction);
}

LONGBOW_TEST_CASE(Global, metisMessage_IsPublicKeyVerified_True)
{
    testUnimplemented("Verification of KeyIds in ContentObjects is not yet implemented.");
}

LONGBOW_TEST_CASE(Global, metisMessage_HasGetCertificate)
{
    testUnimplemented("Need test data with an encoded certificate.");
}

// ===================================================

LONGBOW_TEST_FIXTURE(Local)
{
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

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Message);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
