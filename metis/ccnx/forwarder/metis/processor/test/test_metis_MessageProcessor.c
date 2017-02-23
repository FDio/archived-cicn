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
#include "../metis_MessageProcessor.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>
#include "../../core/test/testrig_MetisIoOperations.h"

// Include this so we can step the clock forward without waiting real time
#include "../../core/metis_Forwarder.c"

#include "testrig_MockTap.h"

// =========================================================================

LONGBOW_TEST_RUNNER(metis_MessageProcessor)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_MessageProcessor)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_MessageProcessor)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_AddTap);

    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_Receive_WithTap);
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_Receive_Interest_WithoutTap);
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_Receive_Object_WithoutTap);

    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_RemoveTap_RemoveCurrentTap);
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_RemoveTap_RemoveOtherTap);

    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_AddOrUpdateRoute);
    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_RemoveRoute);

    LONGBOW_RUN_TEST_CASE(Global, metisMessageProcessor_SetContentStoreSize);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    memset(&testTap, 0, sizeof(testTap));
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

LONGBOW_TEST_CASE(Global, metisMessageProcessor_Create_Destroy)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    uint32_t beforeBalance = parcMemory_Outstanding();
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    metisMessageProcessor_Destroy(&processor);
    uint32_t afterBalance = parcMemory_Outstanding();

    metisForwarder_Destroy(&metis);
    assertTrue(beforeBalance == afterBalance, "Memory imbalance on create/destroy: before %u after %u", beforeBalance, afterBalance);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_AddTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    metisMessageProcessor_AddTap(processor, &testTapTemplate);
    void *currentTap = processor->tap;

    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(currentTap == &testTapTemplate, "tap did not get set correctly, expected %p got %p", (void *) &testTapTemplate, currentTap);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_Receive_WithTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    testTap.callOnReceive = true;
    metisMessageProcessor_AddTap(processor, &testTapTemplate);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 4, 5, logger);

    // now test the function and measure results
    metisMessageProcessor_Receive(processor, interest);

    // cleanup
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(testTap.onReceiveCount == 1, "Incorrect testTap.onReceiveCount, expected %u got %u", 1, testTap.onReceiveCount);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_Receive_Interest_WithoutTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 4, 5, logger);

    // now test the function and measure results
    uint32_t beforeCountReceived = processor->stats.countReceived;
    uint32_t beforeCountInterestsReceived = processor->stats.countInterestsReceived;
    metisMessageProcessor_Receive(processor, interest);
    uint32_t afterCountInterestsReceived = processor->stats.countInterestsReceived;
    uint32_t afterCountReceived = processor->stats.countReceived;

    // cleanup
    // do not cleanup interest, metisMessageProcessor_Receive() takes ownership
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountReceived == beforeCountReceived + 1,
               "Incorrect afterCountReceived, expected %u got %u",
               beforeCountReceived + 1,
               afterCountReceived);

    assertTrue(afterCountInterestsReceived == beforeCountInterestsReceived + 1,
               "Incorrect afterCountInterestsReceived, expected %u got %u",
               beforeCountInterestsReceived + 1,
               afterCountInterestsReceived);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_Receive_Object_WithoutTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 4, 5, logger);


    // now test the function and measure results
    uint32_t beforeCountReceived = processor->stats.countReceived;
    uint32_t beforeCountObjectsReceived = processor->stats.countObjectsReceived;
    metisMessageProcessor_Receive(processor, object);
    uint32_t afterCountObjectsReceived = processor->stats.countObjectsReceived;
    uint32_t afterCountReceived = processor->stats.countReceived;

    // cleanup
    // do not cleanup object, metisMessageProcessor_Receive() takes ownership
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountReceived == beforeCountReceived + 1,
               "Incorrect afterCountReceived, expected %u got %u",
               beforeCountReceived + 1,
               afterCountReceived);

    assertTrue(afterCountObjectsReceived == beforeCountObjectsReceived + 1,
               "Incorrect afterCountInterestsReceived, expected %u got %u",
               afterCountObjectsReceived,
               beforeCountObjectsReceived + 1);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_RemoveTap_RemoveCurrentTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    metisMessageProcessor_AddTap(processor, &testTapTemplate);
    metisMessageProcessor_RemoveTap(processor, &testTapTemplate);
    void *currentTap = processor->tap;

    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(currentTap == NULL, "tap did not get removed correctly, expected %p got %p", NULL, currentTap);
}

/**
 * If we remove a tap that is not currently set, should have no effect.
 */
LONGBOW_TEST_CASE(Global, metisMessageProcessor_RemoveTap_RemoveOtherTap)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    MetisTap otherTap;

    metisMessageProcessor_AddTap(processor, &testTapTemplate);
    metisMessageProcessor_RemoveTap(processor, &otherTap);
    void *currentTap = processor->tap;

    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(currentTap == &testTapTemplate, "tap incorrectly removed, expected %p got %p", (void *) &testTapTemplate, currentTap);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_AddOrUpdateRoute)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    unsigned interfaceIndex = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    CPIRouteEntry *route = cpiRouteEntry_Create(ccnxName, interfaceIndex, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);

    metisMessageProcessor_AddOrUpdateRoute(processor, route);

    size_t hashCodeTableLength = metisFIB_Length(processor->fib);

    cpiRouteEntry_Destroy(&route);
    metisTlvName_Release(&tlvName);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(hashCodeTableLength == 1, "Wrong hash table length, expected %u got %zu", 1, hashCodeTableLength);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_RemoveRoute)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    CCNxName *ccnxName = ccnxName_CreateFromCString("lci:/foo/bar");
    MetisTlvName *tlvName = metisTlvName_CreateFromCCNxName(ccnxName);
    unsigned interfaceIndex = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;

    CPIRouteEntry *route = cpiRouteEntry_Create(ccnxName, interfaceIndex, nexthop, cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);

    metisMessageProcessor_AddOrUpdateRoute(processor, route);
    metisMessageProcessor_RemoveRoute(processor, route);

    size_t hashCodeTableLength = metisFIB_Length(processor->fib);

    cpiRouteEntry_Destroy(&route);
    metisTlvName_Release(&tlvName);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(hashCodeTableLength == 0, "Wrong hash table length, expected %u got %zu", 0, hashCodeTableLength);
}

LONGBOW_TEST_CASE(Global, metisMessageProcessor_SetContentStoreSize)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    size_t newCapacity = 1234;
    metisForwarder_SetContentObjectStoreSize(metis, newCapacity);

    MetisContentStoreInterface *storeImpl = metisMessageProcessor_GetContentObjectStore(metis->processor);

    size_t testCapacity = metisContentStoreInterface_GetObjectCapacity(storeImpl);
    assertTrue(testCapacity == newCapacity, "Expected the new store capacity");

    metisForwarder_Destroy(&metis);
}

// ===================================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_Drop_TestTapNoDrop);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_Drop_TestTapWithDrop);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_Drop_Interest);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_Drop_Object);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_NoConnection);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendFails);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendInterest);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendObject);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_ZeroHopLimit_Remote);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_ZeroHopLimit_Local);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToNexthops);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardToNexthops_DontForwardToIngress);


    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV0_InPit);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV0_NotInPit);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV1_InPit);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV1_NotInPit);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InPit);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInPit);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InCache);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InCacheButExpired);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInCache);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InFib);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInFib);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NoHopLimit);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_AggregateInterestInPit_NewEntry);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_AggregateInterestInPit_ExistingEntry);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_IsInStore);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_IsNotInStore);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_WithKeyIdNotVerified_WithoutVerification);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardViaFib_IsNotInFib);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_ForwardViaFib_IsInFib_EmptyEgressSet);

    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_NoHopLimit);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Local_Zero);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Local_NonZero);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Remote_Zero);
    LONGBOW_RUN_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Remote_NonZero);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    memset(&testTap, 0, sizeof(testTap));
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

/**
 * Test that the tap does not fire if testTap.callOnDrop is false
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_Drop_TestTapNoDrop)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    testTap.callOnDrop = false;
    metisMessageProcessor_AddTap(processor, &testTapTemplate);

    // should not increment a counter
    metisMessageProcessor_Drop(processor, interest);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(testTap.onDropCount == 0, "Incorrect onDropCount, expecting %u, got %u", 0, testTap.onDropCount);
}

/**
 * Test that the tap does fire if testTap.callOnDrop is true
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_Drop_TestTapWithDrop)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    testTap.callOnDrop = true;
    metisMessageProcessor_AddTap(processor, &testTapTemplate);

    // should increment a counter
    metisMessageProcessor_Drop(processor, interest);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(testTap.onDropCount == 1, "Incorrect onDropCount, expecting %u, got %u", 1, testTap.onDropCount);
}

/**
 * Test that when we drop an interest it is counted
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_Drop_Interest)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    // should increment a counter
    metisMessageProcessor_Drop(processor, interest);

    unsigned countDropped = processor->stats.countDropped;
    unsigned countInterestsDropped = processor->stats.countInterestsDropped;

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(countInterestsDropped == 1, "Incorrect countInterestsDropped, expecting %u, got %u", 1, countInterestsDropped);
    assertTrue(countDropped == 1, "Incorrect countDropped, expecting %u, got %u", 1, countDropped);
}

/**
 * Test that when we drop an object it is counted
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_Drop_Object)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    // should increment a counter
    metisMessageProcessor_Drop(processor, object);

    unsigned countDropped = processor->stats.countDropped;
    unsigned countObjectsDropped = processor->stats.countObjectsDropped;

    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(countObjectsDropped == 1, "Incorrect countInterestsDropped, expecting %u, got %u", 1, countObjectsDropped);
    assertTrue(countDropped == 1, "Incorrect countDropped, expecting %u, got %u", 1, countDropped);
}

/**
 * Send a message to a connection that does not exist in the connection table
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_NoConnection)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    metisMessageProcessor_ForwardToInterfaceId(processor, object, 99);

    uint32_t countDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;
    uint32_t countObjectsDropped = processor->stats.countObjectsDropped;

    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(countDroppedConnectionNotFound == 1, "Incorrect countDroppedConnectionNotFound, expecting %u, got %u", 1, countDroppedConnectionNotFound);
    assertTrue(countObjectsDropped == 1, "Incorrect countDropped, expecting %u, got %u", 1, countObjectsDropped);
}

/**
 * Send to a connection that is down
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendFails)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 99, false, false, false);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);
    metisMessageProcessor_ForwardToInterfaceId(processor, object, 99);

    uint32_t countSendFailures = processor->stats.countSendFailures;
    uint32_t countObjectsDropped = processor->stats.countObjectsDropped;

    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);

    assertTrue(countSendFailures == 1, "Incorrect countSendFailures, expecting %u, got %u", 1, countSendFailures);
    assertTrue(countObjectsDropped == 1, "Incorrect countDropped, expecting %u, got %u", 1, countObjectsDropped);
}

/**
 * Send an interest out a good connection
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendInterest)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), 1, 2, logger);

    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 99, true, true, false);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);
    metisMessageProcessor_ForwardToInterfaceId(processor, object, 99);

    uint32_t countInterestForwarded = processor->stats.countInterestForwarded;
    uint32_t sendCount = data->sendCount;

    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);

    assertTrue(countInterestForwarded == 1, "Incorrect countInterestForwarded, expecting %u, got %u", 1, countInterestForwarded);
    assertTrue(sendCount == 1, "Incorrect sendCount, expecting %u, got %u", 1, sendCount);
}

/**
 * Send a content object out a good connection
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_SendObject)
{
    // setup
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);

    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 99, true, true, false);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);
    MetisConnection *conn = metisConnection_Create(ops);

    // test
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);
    metisMessageProcessor_ForwardToInterfaceId(processor, object, 99);

    // measure
    uint32_t countObjectsForwarded = processor->stats.countObjectsForwarded;
    uint32_t sendCount = data->sendCount;

    // cleanup
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);

    // validate
    assertTrue(countObjectsForwarded == 1, "Incorrect countObjectsForwarded, expecting %u, got %u", 1, countObjectsForwarded);
    assertTrue(sendCount == 1, "Incorrect sendCount, expecting %u, got %u", 1, sendCount);
}

/*
 * Try to forward an interest with a 0 hop limit to a remote.  Should fail
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_ZeroHopLimit_Remote)
{
    // setup
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_zero_hoplimit, sizeof(metisTestDataV0_EncodedInterest_zero_hoplimit), 1, 2, logger);


    unsigned connId = 99;
    bool isLocal = false;
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, connId, true, true, isLocal);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);
    metisMessageProcessor_ForwardToInterfaceId(processor, object, connId);

    // measure
    uint32_t countDropZeroToRemote = processor->stats.countDroppedZeroHopLimitToRemote;
    uint32_t sendCount = data->sendCount;

    // cleanup
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);

    // validate
    assertTrue(countDropZeroToRemote == 1, "Incorrect countDropZeroToRemote, expecting %u, got %u", 1, countDropZeroToRemote);
    assertTrue(sendCount == 0, "Incorrect sendCount, expecting %u, got %u", 0, sendCount);
}

/*
 * Try to forward an interest with a 0 hop limit to a local.  Should succeed.
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToInterfaceId_ZeroHopLimit_Local)
{
    // setup
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_zero_hoplimit, sizeof(metisTestDataV0_EncodedInterest_zero_hoplimit), 1, 2, logger);


    unsigned connId = 99;
    bool isLocal = true;
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, connId, true, true, isLocal);
    MockIoOperationsData *data = metisIoOperations_GetClosure(ops);
    MetisConnection *conn = metisConnection_Create(ops);

    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);
    metisMessageProcessor_ForwardToInterfaceId(processor, object, connId);

    // measure
    uint32_t countDropZeroToRemote = processor->stats.countDroppedZeroHopLimitToRemote;
    uint32_t sendCount = data->sendCount;

    // cleanup
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops);

    // validate
    assertTrue(countDropZeroToRemote == 0, "Incorrect countDropZeroToRemote, expecting %u, got %u", 0, countDropZeroToRemote);
    assertTrue(sendCount == 1, "Incorrect sendCount, expecting %u, got %u", 1, sendCount);
}


/**
 * Create 2 connections, and try to forwarder to both of them
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToNexthops)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    // 2 connections
    MetisIoOperations *ops_42 = mockIoOperationsData_CreateSimple(1, 2, 42, true, true, false);
    MockIoOperationsData *data_42 = metisIoOperations_GetClosure(ops_42);
    MetisConnection *conn_42 = metisConnection_Create(ops_42);

    MetisIoOperations *ops_43 = mockIoOperationsData_CreateSimple(1, 2, 43, true, true, false);
    MockIoOperationsData *data_43 = metisIoOperations_GetClosure(ops_43);
    MetisConnection *conn_43 = metisConnection_Create(ops_43);

    // Add the connections to the connection table
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn_42);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn_43);

    // Setup the next hops
    MetisNumberSet *nexthops = metisNumberSet_Create();
    metisNumberSet_Add(nexthops, 42);
    metisNumberSet_Add(nexthops, 43);

    // forward the content object to both of them
    metisMessageProcessor_ForwardToNexthops(processor, object, nexthops);

    // there should be 2 object forwards and each IoOps should have gotten 1 send
    uint32_t countObjectsForwarded = processor->stats.countObjectsForwarded;
    uint32_t sendCount_42 = data_42->sendCount;
    uint32_t sendCount_43 = data_43->sendCount;

    // cleanup
    metisNumberSet_Release(&nexthops);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops_42);
    mockIoOperationsData_Destroy(&ops_43);

    // validate
    assertTrue(countObjectsForwarded == 2, "Incorrect countObjectsForwarded, expecting %u, got %u", 2, countObjectsForwarded);
    assertTrue(sendCount_42 == 1, "Incorrect sendCount_42, expecting %u, got %u", 1, sendCount_42);
    assertTrue(sendCount_43 == 1, "Incorrect sendCount_43, expecting %u, got %u", 1, sendCount_43);
}

/**
 * There is a route in the FIB that points to the ingress interface of an interest.
 * Ensure that we don't forward to that interface
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardToNexthops_DontForwardToIngress)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    // ingress interface is #42, so it should not get forwarded out there
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 42, 1, logger);


    // 2 connections
    MetisIoOperations *ops_42 = mockIoOperationsData_CreateSimple(1, 2, 42, true, true, false);
    MockIoOperationsData *data_42 = metisIoOperations_GetClosure(ops_42);
    MetisConnection *conn_42 = metisConnection_Create(ops_42);

    MetisIoOperations *ops_43 = mockIoOperationsData_CreateSimple(1, 2, 43, true, true, false);
    MockIoOperationsData *data_43 = metisIoOperations_GetClosure(ops_43);
    MetisConnection *conn_43 = metisConnection_Create(ops_43);

    // Add the connections to the connection table
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn_42);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn_43);

    // Setup the next hops
    MetisNumberSet *nexthops = metisNumberSet_Create();
    metisNumberSet_Add(nexthops, 42);
    metisNumberSet_Add(nexthops, 43);

    // forward the content object to both of them
    metisMessageProcessor_ForwardToNexthops(processor, object, nexthops);

    // there should be 2 object forwards and each IoOps should have gotten 1 send
    uint32_t countObjectsForwarded = processor->stats.countObjectsForwarded;
    uint32_t sendCount_42 = data_42->sendCount;
    uint32_t sendCount_43 = data_43->sendCount;

    // cleanup
    metisNumberSet_Release(&nexthops);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ops_42);
    mockIoOperationsData_Destroy(&ops_43);

    // validate
    assertTrue(countObjectsForwarded == 1, "Incorrect countObjectsForwarded, expecting %u, got %u", 1, countObjectsForwarded);
    assertTrue(sendCount_42 == 0, "Incorrect sendCount_42, expecting %u, got %u", 0, sendCount_42);
    assertTrue(sendCount_43 == 1, "Incorrect sendCount_43, expecting %u, got %u", 1, sendCount_43);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV0_InPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 3, 4, logger);


    // receive the interst to add it to PIT
    metisMessageProcessor_ReceiveInterest(processor, interest);

    // now test the function and measure results
    // There is no actual connection "1" (the interest ingress port), so the forwarding
    // will show up as a countDroppedConnectionNotFound.

    uint32_t beforeCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;
    metisMessageProcessor_ReceiveContentObject(processor, object);
    uint32_t afterCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;

    // cleanup
    metisMessage_Release(&interest);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountDroppedConnectionNotFound == beforeCountDroppedConnectionNotFound + 1,
               "Incorrect afterCountDroppedConnectionNotFound, expected %u got %u",
               beforeCountDroppedConnectionNotFound + 1,
               afterCountDroppedConnectionNotFound);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV0_NotInPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    // now test the function and measure results
    uint32_t beforeCountDroppedNoReversePath = processor->stats.countDroppedNoReversePath;
    metisMessageProcessor_ReceiveContentObject(processor, object);
    uint32_t afterCountDroppedNoReversePath = processor->stats.countDroppedNoReversePath;

    // cleanup
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountDroppedNoReversePath == beforeCountDroppedNoReversePath + 1,
               "Incorrect afterCountDroppedNoReversePath, expected %u got %u",
               beforeCountDroppedNoReversePath + 1,
               afterCountDroppedNoReversePath);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV1_InPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    MetisLogger *logger = metisForwarder_GetLogger(metis);
//    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Message, PARCLogLevel_Debug);
//    metisLogger_SetLogLevel(logger, MetisLoggerFacility_Processor, PARCLogLevel_Debug);

    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV1_Interest_NameA_Crc32c, sizeof(metisTestDataV1_Interest_NameA_Crc32c), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV1_ContentObject_NameA_Crc32c, sizeof(metisTestDataV1_ContentObject_NameA_Crc32c), 3, 4, logger);

    // receive the interst to add it to PIT
    metisMessageProcessor_ReceiveInterest(processor, interest);

    // now test the function and measure results
    // There is no actual connection "1" (the interest ingress port), so the forwarding
    // will show up as a countDroppedConnectionNotFound.

    uint32_t beforeCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;
    metisMessageProcessor_ReceiveContentObject(processor, object);
    uint32_t afterCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;

    // cleanup
    metisMessage_Release(&interest);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountDroppedConnectionNotFound == beforeCountDroppedConnectionNotFound + 1,
               "Incorrect afterCountDroppedConnectionNotFound, expected %u got %u",
               beforeCountDroppedConnectionNotFound + 1,
               afterCountDroppedConnectionNotFound);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveContentObjectV1_NotInPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);


    // now test the function and measure results
    uint32_t beforeCountDroppedNoReversePath = processor->stats.countDroppedNoReversePath;
    metisMessageProcessor_ReceiveContentObject(processor, object);
    uint32_t afterCountDroppedNoReversePath = processor->stats.countDroppedNoReversePath;

    // cleanup
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountDroppedNoReversePath == beforeCountDroppedNoReversePath + 1,
               "Incorrect afterCountDroppedNoReversePath, expected %u got %u",
               beforeCountDroppedNoReversePath + 1,
               afterCountDroppedNoReversePath);
}


/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed.  The second interest must come from
 * a different reverse path to be aggregated.
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *interest2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 2, 2, logger);


    // add it once
    metisMessageProcessor_AggregateInterestInPit(processor, interest1);

    // now test the function and measure results
    uint32_t beforeCountInterestsAggregated = processor->stats.countInterestsAggregated;
    metisMessageProcessor_ReceiveInterest(processor, interest2);
    uint32_t afterCountInterestsAggregated = processor->stats.countInterestsAggregated;

    // cleanup
    metisMessage_Release(&interest1);
    metisMessage_Release(&interest2);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountInterestsAggregated == beforeCountInterestsAggregated + 1,
               "Incorrect afterCountInterestsAggregated, expected %u got %u",
               beforeCountInterestsAggregated + 1,
               afterCountInterestsAggregated);
}

/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInPit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    // now test the function and measure results
    uint32_t beforeCountInterestsAggregated = processor->stats.countInterestsAggregated;
    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t afterCountInterestsAggregated = processor->stats.countInterestsAggregated;

    // also check its in the PIT now
    MetisPitEntry *pitEntry = metisPIT_GetPitEntry(processor->pit, interest);
    bool foundInPit = false;
    if (pitEntry) {
        foundInPit = true;
    }

    // cleanup
    metisPitEntry_Release(&pitEntry);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountInterestsAggregated == beforeCountInterestsAggregated,
               "Incorrect afterCountInterestsAggregated, expected %u got %u",
               beforeCountInterestsAggregated,
               afterCountInterestsAggregated);

    assertTrue(foundInPit, "Did not find interest in the PIT");
}

/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InCache)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 4, 5, logger);

    // add it to the cache
    metisContentStoreInterface_PutContent(processor->contentStore, object, 0l);

    // now test the function and measure results
    uint32_t beforeCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;
    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t afterCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&object);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountObjectsForwardedFromStore == beforeCountObjectsForwardedFromStore + 1,
               "Incorrect afterCountObjectsForwardedFromStore, expected %u got %u",
               beforeCountObjectsForwardedFromStore + 1,
               afterCountObjectsForwardedFromStore);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InCacheButExpired)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);

    uint64_t currentTimeInTicks = metisForwarder_GetTicks(processor->metis);

    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName,
                                                          sizeof(metisTestDataV0_InterestWithName), 1, currentTimeInTicks, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject,
                                                        sizeof(metisTestDataV0_EncodedObject), 4, currentTimeInTicks, logger);

    // add it to the cache. It's already expired, so should not be forwarded.
    metisMessage_SetExpiryTimeTicks(object, currentTimeInTicks + 1000ULL);
    metisContentStoreInterface_PutContent(processor->contentStore, object, currentTimeInTicks);

    // Crank metis clock.
    metis->clockOffset = metisForwarder_NanosToTicks(5000000000ULL); // Add 5 seconds. Content is now expired.

    // now test the function and measure results.
    uint32_t beforeCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;
    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t afterCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&object);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate. Nothing should have been forwarded.
    assertTrue(afterCountObjectsForwardedFromStore == beforeCountObjectsForwardedFromStore,
               "Incorrect afterCountObjectsForwardedFromStore, expected %u got %u",
               beforeCountObjectsForwardedFromStore,
               afterCountObjectsForwardedFromStore);
}


/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInCache)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 4, 5, logger);


    // now test the function and measure results
    uint32_t beforeCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;
    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t afterCountObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&object);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountObjectsForwardedFromStore == beforeCountObjectsForwardedFromStore,
               "Incorrect afterCountObjectsForwardedFromStore, expected %u got %u",
               beforeCountObjectsForwardedFromStore,
               afterCountObjectsForwardedFromStore);
}

/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_InFib)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    // ----- Add Route
    CCNxName *ccnxNameToAdd =
        ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd,
                                                   interfaceIndex_1,
                                                   nexthop,
                                                   cpiNameRouteProtocolType_STATIC,
                                                   cpiNameRouteType_LONGEST_MATCH,
                                                   lifetime,
                                                   cost);
    metisFIB_AddOrUpdate(processor->fib, routeAdd, "random");

    // now test the function and measure results
    // We will see it in countDroppedConnectionNotFound, because we didnt mock up the interface 22 connection
    uint32_t beforeCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;
    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t afterCountDroppedConnectionNotFound = processor->stats.countDroppedConnectionNotFound;

    // cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(afterCountDroppedConnectionNotFound == beforeCountDroppedConnectionNotFound + 1,
               "Incorrect afterCountDroppedConnectionNotFound, expected %u got %u",
               beforeCountDroppedConnectionNotFound + 1,
               afterCountDroppedConnectionNotFound);
}

/**
 * There's already a detailed test for this, we just check the stats counter
 * to make sure the right logic flow is executed
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NotInFib)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_ReceiveInterest_NoHopLimit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_no_hoplimit,
                                                          sizeof(metisTestDataV0_EncodedInterest_no_hoplimit), 1, 2, logger);


    metisMessageProcessor_ReceiveInterest(processor, interest);
    uint32_t dropCount = processor->stats.countDroppedNoHopLimit;

    // cleanup
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(dropCount == 1,
               "Incorrect countDroppedNoHopLimit, expected %u got %u",
               1,
               dropCount);
}

/**
 * Add an interest to the PIT when it does not exist.  Should not increment the "stats.countInterestsAggregated' counter
 * and should return FALSE, meaning not aggregated
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_AggregateInterestInPit_NewEntry)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    uint32_t beforeCountInterestsAggregated = processor->stats.countInterestsAggregated;
    bool aggregated = metisMessageProcessor_AggregateInterestInPit(processor, interest);
    uint32_t afterCountInterestsAggregated = processor->stats.countInterestsAggregated;

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(afterCountInterestsAggregated == beforeCountInterestsAggregated,
               "Incorrect afterCountInterestsAggregated, expected %u got %u",
               beforeCountInterestsAggregated,
               afterCountInterestsAggregated);
    assertFalse(aggregated, "Interest aggregated when no interests in table!");
}

/**
 * Add an interest to the PIT, then add it again. SHould increment the "stats.countInterestsAggregated" counter and
 * should return TRUE meaning, it was aggregated.  The second interest needs to come from a different interface.
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_AggregateInterestInPit_ExistingEntry)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest1 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *interest2 = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 2, 2, logger);


    // add it once
    metisMessageProcessor_AggregateInterestInPit(processor, interest1);

    // now add it again
    uint32_t beforeCountInterestsAggregated = processor->stats.countInterestsAggregated;
    bool aggregated = metisMessageProcessor_AggregateInterestInPit(processor, interest2);
    uint32_t afterCountInterestsAggregated = processor->stats.countInterestsAggregated;

    metisMessage_Release(&interest1);
    metisMessage_Release(&interest2);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    assertTrue(afterCountInterestsAggregated == beforeCountInterestsAggregated + 1,
               "Incorrect afterCountInterestsAggregated, expected %u got %u",
               beforeCountInterestsAggregated + 1,
               afterCountInterestsAggregated);
    assertTrue(aggregated, "Interest not aggregated with self!");
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_WithKeyIdNotVerified_WithoutVerification)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);

    MetisMessage *contentObjectWithKeyId =
        metisMessage_CreateFromArray(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256,
                                     sizeof(metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256), 4, 5, logger);

    // add it to the cache
    metisContentStoreInterface_PutContent(processor->contentStore, contentObjectWithKeyId, 1l);

    // Now create an Interest with the same name and a KeyId.
    MetisMessage *interestWithKeyIdRestriction =
        metisMessage_CreateFromArray(metisTestDataV1_Interest_NameAAndKeyId,
                                     sizeof(metisTestDataV1_Interest_NameAAndKeyId), 4, 5, logger);

    // Now test the code. We should NOT match it, due to the content store not currently verifying keyIds.
    bool success = _satisfyFromContentStore(processor, interestWithKeyIdRestriction);
    unsigned countObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&interestWithKeyIdRestriction);
    metisMessage_Release(&contentObjectWithKeyId);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertFalse(success, "Expected Interest to not be satisfied from cache!");
    assertTrue(countObjectsForwardedFromStore == 0,
               "Incorrect countObjectsForwardedFromStore, expected %u got %u",
               0,
               countObjectsForwardedFromStore);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_IsInStore)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 4, 5, logger);

    // add it to the cache
    metisContentStoreInterface_PutContent(processor->contentStore, object, 1l);

    // now test the code
    bool success = _satisfyFromContentStore(processor, interest);
    unsigned countObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&interest);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertTrue(success, "Interest not satisfied from cache!");
    assertTrue(countObjectsForwardedFromStore == 1,
               "Incorrect countObjectsForwardedFromStore, expected %u got %u",
               1,
               countObjectsForwardedFromStore);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_SatisfyFromContentStore_IsNotInStore)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *object = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 4, 5, logger);


    // don't add it to the cache

    // now test the code
    bool success = _satisfyFromContentStore(processor, interest);
    unsigned countObjectsForwardedFromStore = processor->stats.countInterestsSatisfiedFromStore;

    // cleanup
    metisMessage_Release(&interest);
    metisMessage_Release(&object);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // validate
    assertFalse(success, "Interest satisfied from cache, when we didn't put it there!");
    assertTrue(countObjectsForwardedFromStore == 0,
               "Incorrect countObjectsForwardedFromStore, expected %u got %u",
               0,
               countObjectsForwardedFromStore);
}

/**
 * Add fib entry /hello/ouch and ask for /party/ouch
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardViaFib_IsNotInFib)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    // ----- Add
    CCNxName *ccnxNameToAdd =
        ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");

    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop,
                                                   cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(processor->fib, routeAdd,"random" );

    // ----- Measure
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithOtherName, sizeof(metisTestDataV0_InterestWithOtherName), 1, 2, logger);


    bool success = metisMessageProcessor_ForwardViaFib(processor, interest);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // ----- Validate
    assertFalse(success, "Returned true even though no route");
}

/**
 * Forward to an existing FIB entry.  The PIT entry has an empty egress set.
 */
LONGBOW_TEST_CASE(Local, metisMessageProcessor_ForwardViaFib_IsInFib_EmptyEgressSet)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    // ----- Add Route
    CCNxName *ccnxNameToAdd =
        ccnxName_CreateFromCString("lci:/2=hello/0xF000=ouch");
    unsigned interfaceIndex_1 = 22;
    CPIAddress *nexthop = NULL;
    struct timeval *lifetime = NULL;
    unsigned cost = 12;
    CPIRouteEntry *routeAdd = cpiRouteEntry_Create(ccnxNameToAdd, interfaceIndex_1, nexthop,
                                                   cpiNameRouteProtocolType_STATIC, cpiNameRouteType_LONGEST_MATCH, lifetime, cost);
    metisFIB_AddOrUpdate(processor->fib, routeAdd, "random");

    // ----- Add PIT entry
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);


    metisPIT_ReceiveInterest(processor->pit, interest);

    // ----- Measure
    bool success = metisMessageProcessor_ForwardViaFib(processor, interest);

    // ----- Cleanup
    cpiRouteEntry_Destroy(&routeAdd);
    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    metisForwarder_Destroy(&metis);

    // ----- Validate
    assertTrue(success, "Returned false with existing PIT entry");
}

MetisConnection *
setupMockConnection(MetisForwarder *metis, bool isLocal)
{
    MetisIoOperations *ops = mockIoOperationsData_CreateSimple(1, 2, 99, false, false, isLocal);
    MetisConnection *conn = metisConnection_Create(ops);
    metisConnectionTable_Add(metisForwarder_GetConnectionTable(metis), conn);

    return conn;
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_NoHopLimit)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);

    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    bool isLocal = false;
    MetisConnection *conn = setupMockConnection(metis, isLocal);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest =
        metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_no_hoplimit,
                                     sizeof(metisTestDataV0_EncodedInterest_no_hoplimit), metisConnection_GetConnectionId(conn), 2, logger);


    bool success = metisMessageProcessor_CheckAndDecrementHopLimitOnIngress(processor, interest);
    assertFalse(success, "Should have failed for an interest without hoplimit");

    assertTrue(processor->stats.countDroppedNoHopLimit == 1,
               "Wrong countDroppedNoHopLimit, got %u expected %u", processor->stats.countDroppedNoHopLimit, 1);
    assertTrue(processor->stats.countDroppedZeroHopLimitFromRemote == 0,
               "Wrong countDroppedZeroHopLimitFromRemote, got %u expected %u", processor->stats.countDroppedZeroHopLimitFromRemote, 0);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ioOps);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Local_Zero)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    bool isLocal = true;
    MetisConnection *conn = setupMockConnection(metis, isLocal);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest =
        metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_zero_hoplimit,
                                     sizeof(metisTestDataV0_EncodedInterest_zero_hoplimit), metisConnection_GetConnectionId(conn), 2, logger);


    bool success = metisMessageProcessor_CheckAndDecrementHopLimitOnIngress(processor, interest);
    assertTrue(success, "Local with 0 hoplimit should have been ok");
    assertTrue(processor->stats.countDroppedNoHopLimit == 0,
               "Wrong countDroppedNoHopLimit, got %u expected %u", processor->stats.countDroppedNoHopLimit, 0);
    assertTrue(processor->stats.countDroppedZeroHopLimitFromRemote == 0,
               "Wrong countDroppedZeroHopLimitFromRemote, got %u expected %u", processor->stats.countDroppedZeroHopLimitFromRemote, 0);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ioOps);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Local_NonZero)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    bool isLocal = true;
    MetisConnection *conn = setupMockConnection(metis, isLocal);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest =
        metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest,
                                     sizeof(metisTestDataV0_EncodedInterest), metisConnection_GetConnectionId(conn), 2, logger);


    bool success = metisMessageProcessor_CheckAndDecrementHopLimitOnIngress(processor, interest);
    assertTrue(success, "Local with non-0 hoplimit should have been ok");
    assertTrue(processor->stats.countDroppedNoHopLimit == 0,
               "Wrong countDroppedNoHopLimit, got %u expected %u", processor->stats.countDroppedNoHopLimit, 0);
    assertTrue(processor->stats.countDroppedZeroHopLimitFromRemote == 0,
               "Wrong countDroppedZeroHopLimitFromRemote, got %u expected %u", processor->stats.countDroppedZeroHopLimitFromRemote, 0);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ioOps);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Remote_Zero)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    bool isLocal = false;
    MetisConnection *conn = setupMockConnection(metis, isLocal);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest =
        metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest_zero_hoplimit,
                                     sizeof(metisTestDataV0_EncodedInterest_zero_hoplimit), metisConnection_GetConnectionId(conn), 2, logger);


    bool success = metisMessageProcessor_CheckAndDecrementHopLimitOnIngress(processor, interest);
    assertFalse(success, "Remote with 0 hoplimit should have been failure");
    assertTrue(processor->stats.countDroppedNoHopLimit == 0,
               "Wrong countDroppedNoHopLimit, got %u expected %u", processor->stats.countDroppedNoHopLimit, 0);
    assertTrue(processor->stats.countDroppedZeroHopLimitFromRemote == 1,
               "Wrong countDroppedZeroHopLimitFromRemote, got %u expected %u", processor->stats.countDroppedZeroHopLimitFromRemote, 1);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ioOps);
}

LONGBOW_TEST_CASE(Local, metisMessageProcessor_CheckAndDecrementHopLimitOnIngress_Remote_NonZero)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisMessageProcessor *processor = metisMessageProcessor_Create(metis);

    bool isLocal = false;
    MetisConnection *conn = setupMockConnection(metis, isLocal);
    MetisLogger *logger = metisForwarder_GetLogger(metis);
    MetisMessage *interest =
        metisMessage_CreateFromArray(metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest), metisConnection_GetConnectionId(conn), 2, logger);


    bool success = metisMessageProcessor_CheckAndDecrementHopLimitOnIngress(processor, interest);
    assertTrue(success, "Remote with non-0 hoplimit should have been ok");
    assertTrue(processor->stats.countDroppedNoHopLimit == 0,
               "Wrong countDroppedNoHopLimit, got %u expected %u", processor->stats.countDroppedNoHopLimit, 0);
    assertTrue(processor->stats.countDroppedZeroHopLimitFromRemote == 0,
               "Wrong countDroppedZeroHopLimitFromRemote, got %u expected %u", processor->stats.countDroppedZeroHopLimitFromRemote, 0);

    metisMessage_Release(&interest);
    metisMessageProcessor_Destroy(&processor);
    MetisIoOperations *ioOps = metisConnection_GetIoOperations(conn);
    metisForwarder_Destroy(&metis);
    mockIoOperationsData_Destroy(&ioOps);
}


// ========================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_MessageProcessor);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
