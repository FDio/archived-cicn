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

#define DEBUG_OUTPUT 0

#include "../component_Vegas.c"
#include "../vegas_Session.c"

#include <sys/un.h>
#include <strings.h>
#include <sys/queue.h>

#include <LongBow/unit-test.h>
#include <LongBow/runtime.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_PublicKeySignerPkcs12Store.h>
#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/api/notify/notify_Status.h>

#include <ccnx/transport/test_tools/traffic_tools.h>
#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_NameSegmentNumber.h>

#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_NonThreaded.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.c>
#include <ccnx/transport/transport_rta/core/rta_Connection.c>
#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include "../../test/testrig_MockFramework.c"

#ifndef MAXPATH
#define MAXPATH 1024
#endif

// file descriptor for random numbers, part of Fixture
static int randomFd;

typedef struct test_data {
    MockFramework *mock;
    char keystore_filename[MAXPATH];
    char keystore_password[MAXPATH];
} TestData;

static CCNxTransportConfig *
createParams(const char *keystore_name, const char *keystore_passwd)
{
    assertNotNull(keystore_name, "Got null keystore name\n");
    assertNotNull(keystore_passwd, "Got null keystore passwd\n");

    CCNxStackConfig *stackConfig = apiConnector_ProtocolStackConfig(
        testingUpper_ProtocolStackConfig(
            vegasFlowController_ProtocolStackConfig(
                testingLower_ProtocolStackConfig(
                    protocolStack_ComponentsConfigArgs(ccnxStackConfig_Create(),
                                                       apiConnector_GetName(),
                                                       testingUpper_GetName(),
                                                       vegasFlowController_GetName(),
                                                       testingLower_GetName(),
                                                       NULL)))));

    CCNxConnectionConfig *connConfig = apiConnector_ConnectionConfig(
        testingUpper_ConnectionConfig(
            vegasFlowController_ConnectionConfig(
                tlvCodec_ConnectionConfig(
                    testingLower_ConnectionConfig(ccnxConnectionConfig_Create())))));

    publicKeySignerPkcs12Store_ConnectionConfig(connConfig, keystore_name, keystore_passwd);

    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

static TestData *
_commonSetup(const char *name)
{
    parcSecurity_Init();

    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    sprintf(data->keystore_filename, "/tmp/keystore_%s_%d.p12", name, getpid());
    sprintf(data->keystore_password, "12345");

    unlink(data->keystore_filename);

    CCNxTransportConfig *config = createParams(data->keystore_filename, data->keystore_password);
    data->mock = mockFramework_Create(config);
    ccnxTransportConfig_Destroy(&config);
    return data;
}

static void
_commonTeardown(TestData *data)
{
    mockFramework_Destroy(&data->mock);
    unlink(data->keystore_filename);
    parcMemory_Deallocate((void **) &data);

    parcSecurity_Fini();
}

// ======================================================

LONGBOW_TEST_RUNNER(Fc_Vegas)
{
    LONGBOW_RUN_TEST_FIXTURE(Component);
}

LONGBOW_TEST_RUNNER_SETUP(Fc_Vegas)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    randomFd = open("/dev/urandom", O_RDONLY);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(Fc_Vegas)
{
    close(randomFd);
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==============================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, vegasSession_GetFinalBlockIdFromContentObject_None);
    LONGBOW_RUN_TEST_CASE(Local, vegasSession_GetFinalBlockIdFromContentObject_TestCases);

    LONGBOW_RUN_TEST_CASE(Local, vegasSession_GetSegnumFromObject);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup(longBowTestCase_GetName(testCase)));
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static CCNxTlvDictionary *
createSignedContentObject(void)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/some/name");
    PARCBuffer *payload = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), 11, (uint8_t *) "the payload"));
    CCNxTlvDictionary *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    parcBuffer_Release(&payload);
    ccnxName_Release(&name);

    PARCBuffer *keyid = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), 5, (uint8_t *) "keyid"));
    ccnxValidationRsaSha256_Set(contentObject, keyid, NULL);
    parcBuffer_Release(&keyid);

    PARCBuffer *sigbits = parcBuffer_WrapCString("the signature");
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, parcBuffer_Flip(sigbits));
    ccnxContentObject_SetSignature(contentObject, keyid, signature, NULL);

    parcSignature_Release(&signature);
    parcBuffer_Release(&sigbits);

    return contentObject;
}

static CCNxTlvDictionary *
createSignedContentObjectWithFinalBlockId(uint64_t fbid)
{
    CCNxTlvDictionary *obj = createSignedContentObject();
    ccnxContentObject_SetFinalChunkNumber(obj, fbid);

    return obj;
}

LONGBOW_TEST_CASE(Local, vegasSession_GetFinalBlockIdFromContentObject_None)
{
    CCNxTlvDictionary *contentObjectDictionary = createSignedContentObject();
    bool success = vegasSession_GetFinalBlockIdFromContentObject(contentObjectDictionary, NULL);
    assertFalse(success, "Should have failed getting FBID from content object");
    ccnxTlvDictionary_Release(&contentObjectDictionary);
}

LONGBOW_TEST_CASE(Local, vegasSession_GetFinalBlockIdFromContentObject_TestCases)
{
    struct test_struct {
        uint64_t value;
        size_t encodedBytes;
        uint8_t *encoded;
    } test_vector[] = {
        { .value = 0x0000000000000000ULL, .encodedBytes = 1, .encoded = (uint8_t[1]) { 0x00 } },
        { .value = 0x0000000000000001ULL, .encodedBytes = 1, .encoded = (uint8_t[1]) { 0x01 } },
        { .value = 0x00000000000000FFULL, .encodedBytes = 1, .encoded = (uint8_t[1]) { 0xFF } },
        { .value = 0x0000000000000100ULL, .encodedBytes = 2, .encoded = (uint8_t[2]) { 0x01, 0x00} },
        { .value = 0x0100000000000100ULL, .encodedBytes = 8, .encoded = (uint8_t[8]) { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00} },
        { .value = 0x8000000000000100ULL, .encodedBytes = 8, .encoded = (uint8_t[8]) { 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00} },
        { .value = 0xFFFFFFFFFFFFFFFFULL, .encodedBytes = 8, .encoded = (uint8_t[8]) { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} },
        { .value = 0,                     .encodedBytes = 0, .encoded = NULL }
    };

    for (int i = 0; test_vector[i].encoded != NULL; i++) {
        CCNxTlvDictionary *signed_with_fbid = createSignedContentObjectWithFinalBlockId(test_vector[i].value);

        uint64_t testvalue = -1;
        bool success = vegasSession_GetFinalBlockIdFromContentObject(signed_with_fbid, &testvalue);
        assertTrue(success, "Failed to get FBID from content object index %d value %016" PRIx64 "\n",
                   i,
                   test_vector[i].value)
        {
            ccnxTlvDictionary_Display(signed_with_fbid, 0);
        }


        assertTrue(testvalue == test_vector[i].value,
                   "Segment number does not match index %d value %016" PRIx64 ": got %" PRIx64 "\n",
                   i,
                   test_vector[i].value,
                   testvalue);

        ccnxTlvDictionary_Release(&signed_with_fbid);
    }
}

LONGBOW_TEST_CASE(Local, vegasSession_GetSegnumFromObject)
{
    struct test_struct {
        bool valid;
        uint64_t segnum;
        char *uri;
    } test_vectors[] = {
        { .valid = false, .segnum = 0,                  .uri = "lci:/foo/bar"                              },
        { .valid = true,  .segnum = 0,                  .uri = "lci:/foo/" CCNxNameLabel_Chunk "=%00"      },
        { .valid = true,  .segnum = 0x1020,             .uri = "lci:/foo/" CCNxNameLabel_Chunk "=%10%20"   },
        { .valid = true,  .segnum = 0x6162,             .uri = "lci:/foo/" CCNxNameLabel_Chunk "=ab"       },
        { .valid = true,  .segnum = 0x616263,           .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abc"      },
        { .valid = true,  .segnum = 0x61626364,         .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abcd"     },
        { .valid = true,  .segnum = 0x6162636465,       .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abcde"    },
        { .valid = true,  .segnum = 0x616263646566,     .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abcdef"   },
        { .valid = true,  .segnum = 0x61626364656667,   .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abcdefg"  },
        { .valid = true,  .segnum = 0x6162636465666768, .uri = "lci:/foo/" CCNxNameLabel_Chunk "=abcdefgh" },
        { .valid = false, .segnum = 0,                  .uri = NULL                                        }
    };

    for (int i = 0; test_vectors[i].uri != NULL; i++) {
        CCNxName *name = ccnxName_CreateFromCString(test_vectors[i].uri);
        CCNxTlvDictionary *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, NULL);

        uint64_t testSeqnum = -1;
        int failure = vegasSession_GetSegnumFromObject(contentObject, &testSeqnum);



        if (test_vectors[i].valid) {
            assertFalse(failure, "Incorrect success index %d: got %d expected %d",
                        i, failure, test_vectors[i].valid);

            assertTrue(testSeqnum == test_vectors[i].segnum, "Incorrect segnum index %d, got %" PRIu64 " expected %" PRIu64,
                       i, testSeqnum, test_vectors[i].segnum);
        } else {
            assertTrue(failure, "Incorrect success index %d: got %d expected %d",
                       i, failure, test_vectors[i].valid);
        }

        ccnxName_Release(&name);
        ccnxTlvDictionary_Release(&contentObject);
    }
}

// ==============================================================

LONGBOW_TEST_FIXTURE(Component)
{
    LONGBOW_RUN_TEST_CASE(Component, open_close);

    // these should all be pass through
    LONGBOW_RUN_TEST_CASE(Component, content_object_down);
    LONGBOW_RUN_TEST_CASE(Component, control_msg_down);
    LONGBOW_RUN_TEST_CASE(Component, interest_up);
    LONGBOW_RUN_TEST_CASE(Component, control_msg_up);
    LONGBOW_RUN_TEST_CASE(Component, cancel_flow);
}

LONGBOW_TEST_FIXTURE_SETUP(Component)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup(longBowTestCase_GetName(testCase)));
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Component)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ============================================

LONGBOW_TEST_CASE(Component, open_close)
{
    // dont actually do anything.  make sure no memory leaks in setup and teardown.
}


// ============================================
// Passthrough messages

LONGBOW_TEST_CASE(Component, content_object_down)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithSignedContentObject(data->mock->connection);

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_UP);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.downcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);

    TransportMessage *test_tm = rtaComponent_GetMessage(out);

    assertTrue(test_tm == truth_tm,
               "Got wrong transport message pointer, got %p expected %p",
               (void *) test_tm,
               (void *) truth_tm);

    transportMessage_Destroy(&truth_tm);
}

LONGBOW_TEST_CASE(Component, control_msg_down)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithControlMessage(data->mock->connection);

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_UP);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.downcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);
    TransportMessage *test_tm = rtaComponent_GetMessage(out);

    assertTrue(test_tm == truth_tm,
               "Got wrong transport message pointer, got %p expected %p",
               (void *) test_tm,
               (void *) truth_tm);

    transportMessage_Destroy(&truth_tm);
}

LONGBOW_TEST_CASE(Component, interest_up)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);

    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_DOWN);
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.upcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);
    TransportMessage *test_tm = rtaComponent_GetMessage(out);

    assertTrue(test_tm == truth_tm,
               "Got wrong transport message pointer, got %p expected %p",
               (void *) test_tm,
               (void *) truth_tm);

    transportMessage_Destroy(&truth_tm);
}

LONGBOW_TEST_CASE(Component, control_msg_up)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithControlMessage(data->mock->connection);

    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_DOWN);
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.upcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);
    TransportMessage *test_tm = rtaComponent_GetMessage(out);

    assertTrue(test_tm == truth_tm,
               "Got wrong transport message pointer, got %p expected %p",
               (void *) test_tm,
               (void *) truth_tm);

    transportMessage_Destroy(&test_tm);
}

// ============================================
// These should start a flow control session

/**
 * Creates an interest w/o a segment number
 * Sends it down the stack to the flow controller
 * Flow controller should append segment number 0 to the interest and send that down the stack
 */
LONGBOW_TEST_CASE(Component, interest_down)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);

    // If we can, add a payload to the Interest. Why not.
    PARCBuffer *payload = NULL;
    CCNxInterest *interest = transportMessage_GetDictionary(truth_tm);
    CCNxInterestInterface *impl = ccnxInterestInterface_GetInterface(interest);
    if (impl != NULL && impl != &CCNxInterestFacadeV1_Implementation) {
        // V1 or greater should support Interest payloads.
        payload = parcBuffer_WrapCString("This is a payload.");
        impl->setPayload(interest, payload);
    }

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_UP);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.downcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);

    // we should see a status message up the stack and interests
    // going down the stack.


    TransportMessage *test_tm = rtaComponent_GetMessage(in);
    assertNotNull(test_tm, "got null transport message back up the queue, expecting status\n");

    assertTrue(transportMessage_IsControl(test_tm),
               "Transport message is not a control object")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(test_tm), 0);
    }

    CCNxTlvDictionary *test_dict = transportMessage_GetDictionary(test_tm);

    PARCJSON *json = ccnxControlFacade_GetJson(test_dict);

    NotifyStatus *status = notifyStatus_ParseJSON(json);

    assertNotNull(status, "Could not parse NotifyStatus JSON message");
    assertTrue(notifyStatus_GetFiledes(status) == data->mock->connection->api_fd,
               "Expected file descriptor %d, actual %d\n", data->mock->connection->api_fd, notifyStatus_GetFiledes(status));

    assertTrue(notifyStatus_IsFlowControlStarted(status),
               "Expected notifyStatus_IsFlowControlStarted to be true, actual code %d", notifyStatus_GetStatusCode(status));

    notifyStatus_Release(&status);

    transportMessage_Destroy(&test_tm);

    // Read segment 0 interest
    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(transportMessage_GetDictionary(truth_tm)), 0, payload);

    // Now read segment 1
    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(transportMessage_GetDictionary(truth_tm)), 1, payload);

    if (payload != NULL) {
        parcBuffer_Release(&payload);
    }

    transportMessage_Destroy(&truth_tm);
}


LONGBOW_TEST_CASE(Component, interest_down_slow_retransmit)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);

    VegasConnectionState *fc;
    FcSessionHolder *holder;

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_UP);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.downcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);

    // --------------------------------------
    // Read segment 0 interest
    CCNxTlvDictionary *interest = transportMessage_GetDictionary(truth_tm);

    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(interest), 0, NULL);

    // Now read segment 1
    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(interest), 1, NULL);

    // --------------------------------------
    // now bump the time and see what happens.
    // these are normally set in the timer sallback
    fc = rtaConnection_GetPrivateData(data->mock->connection, FC_VEGAS);
    holder = TAILQ_FIRST(&fc->sessions_head);
    assertNotNull(holder, "got null session holder");

    printf("*** bump time\n");

    data->mock->framework->clock_ticks += 1001;

    // RTO timeout will be 1 second
    vegasSession_TimerCallback(-1, PARCEventType_Timeout, holder->session);
    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(interest), 0, NULL);

    transportMessage_Destroy(&truth_tm);
}


LONGBOW_TEST_CASE(Component, interest_down_fast_retransmit)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);

    CCNxName           *basename, *segmentname;
    VegasConnectionState *fc;
    FcSessionHolder *holder;

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *read = rtaProtocolStack_GetPutQueue(data->mock->stack, FC_VEGAS, RTA_UP);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStep(data->mock->framework);
    flow_vegas_ops.downcallRead(read, PARCEventType_Read, (void *) data->mock->stack);
    rtaFramework_NonThreadedStep(data->mock->framework);

    // --------------------------------------
    // Read segment 0 interest
    CCNxTlvDictionary *interest = transportMessage_GetDictionary(truth_tm);

    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(interest), 0, NULL);

    // Now read segment 1
    trafficTools_ReadAndVerifySegment(out, ccnxInterest_GetName(interest), 1, NULL);

    // --------------------------------------
    // now bump the time and see what happens.
    // these are normally set in the timer sallback
    fc = rtaConnection_GetPrivateData(data->mock->connection, FC_VEGAS);
    holder = TAILQ_FIRST(&fc->sessions_head);
    assertNotNull(holder, "got null session holder");


    data->mock->framework->clock_ticks += 20;
    printf("*** bump time %" PRIu64 "\n", data->mock->framework->clock_ticks);
    vegasSession_TimerCallback(-1, PARCEventType_Timeout, holder->session);

    // --------------------------------------
    // send an out-of-order content object, should see a fast retransmit

    basename = ccnxName_Copy(ccnxInterest_GetName(interest));
    segmentname = ccnxName_Copy(basename);

    CCNxNameSegment *segment = ccnxNameSegmentNumber_Create(CCNxNameLabelType_CHUNK, 1);
    ccnxName_Append(segmentname, segment);
    ccnxNameSegment_Release(&segment);

    transportMessage_Destroy(&truth_tm);

    // this takes ownership of segment name
    TransportMessage *reply =
        trafficTools_CreateTransportMessageWithSignedContentObjectWithName(data->mock->connection,
                                                                           segmentname, data->keystore_filename, data->keystore_password);

    rtaComponent_PutMessage(out, reply);

    data->mock->framework->clock_ticks += 40;
    printf("*** bump time %" PRIu64 "\n", data->mock->framework->clock_ticks);
    rtaFramework_NonThreadedStepCount(data->mock->framework, 5);
    vegasSession_TimerCallback(-1, PARCEventType_Timeout, holder->session);

    trafficTools_ReadAndVerifySegment(out, basename, 0, NULL);

    ccnxName_Release(&segmentname);
    ccnxName_Release(&basename);
}

/**
 * Send an interest down the stack to start a flow controller, then send
 * a control message to cancel it.
 */
LONGBOW_TEST_CASE(Component, cancel_flow)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    TransportMessage *truth_tm = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);

    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);

    CCNxName *flowName = ccnxName_Acquire(ccnxInterest_GetName(transportMessage_GetDictionary(truth_tm)));

    // ================================
    // This will signal to the flow controller that it should start a flow
    // We give up ownership of "truth_tm" at this point
    rtaComponent_PutMessage(in, truth_tm);
    rtaFramework_NonThreadedStepCount(data->mock->framework, 5);

    // ================================
    // we should see a status message up the stack

    TransportMessage *test_tm = rtaComponent_GetMessage(in);
    assertNotNull(test_tm, "got null transport message back up the queue, expecting status\n");

    assertTrue(transportMessage_IsControl(test_tm), "Transport message is not a Control")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(test_tm), 0);
    }

    CCNxTlvDictionary *controlDictionary = transportMessage_GetDictionary(test_tm);

    PARCJSON *json = ccnxControlFacade_GetJson(controlDictionary);

    NotifyStatus *status = notifyStatus_ParseJSON(json);
    assertTrue(notifyStatus_IsFlowControlStarted(status),
               "Expected notifyStatus_IsFlowControlStarted to be true.  Actual code %d\n", notifyStatus_GetStatusCode(status));
    notifyStatus_Release(&status);

    // ================================
    // After the notification, the flow is "started" and we can cancel it

    // Now that its started, send a cancel
    PARCJSON *cancelFlow = cpiCancelFlow_Create(flowName);
    CCNxTlvDictionary *cancelDictionary = ccnxControlFacade_CreateCPI(cancelFlow);
    parcJSON_Release(&cancelFlow);

    TransportMessage *cancelTm = transportMessage_CreateFromDictionary(cancelDictionary);
    transportMessage_SetInfo(cancelTm, rtaConnection_Copy(data->mock->connection), rtaConnection_FreeFunc);
    rtaComponent_PutMessage(in, cancelTm);
    rtaFramework_NonThreadedStepCount(data->mock->framework, 5);

    // now verify that its gone
    VegasConnectionState *fc = rtaConnection_GetPrivateData(data->mock->connection, FC_VEGAS);
    FcSessionHolder *holder = TAILQ_FIRST(&fc->sessions_head);
    assertNull(holder, "The session list is not empty!");

    ccnxTlvDictionary_Release(&cancelDictionary);
    transportMessage_Destroy(&test_tm);
    ccnxName_Release(&flowName);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(Fc_Vegas);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
