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

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_NonThreaded.h>

#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.c>
#include <ccnx/transport/transport_rta/core/rta_Connection.c>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_PublicKeySignerPkcs12Store.h>
#include <ccnx/transport/transport_rta/config/config_All.h>

#include <ccnx/api/notify/notify_Status.h>

#include <ccnx/transport/test_tools/traffic_tools.h>

#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>

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
                testingLower_ConnectionConfig(ccnxConnectionConfig_Create()))));


    publicKeySignerPkcs12Store_ConnectionConfig(connConfig, keystore_name, keystore_passwd);

    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

static TestData *
_commonSetup(const char *name)
{
    parcSecurity_Init();

    TestData *data = parcMemory_Allocate(sizeof(TestData));

    assertNotNull(data, "Got null memory from parcMemory_Allocate");

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

LONGBOW_TEST_RUNNER(VegasSession)
{
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(IterateFinalChunkNumber);
}

LONGBOW_TEST_RUNNER_SETUP(VegasSession)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    randomFd = open("/dev/urandom", O_RDONLY);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(VegasSession)
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

    LONGBOW_RUN_TEST_CASE(Local, vegasSession_ReceiveContentObject_InOrder_LastBlockSetsFinalId);
    LONGBOW_RUN_TEST_CASE(Local, vegasSession_ReceiveContentObject_InOrder_FirstAndLastBlocksSetsFinalId);
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
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/some/name");
    PARCBuffer *payload = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), 11, (uint8_t *) "the payload"));
    CCNxTlvDictionary *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    parcBuffer_Release(&payload);
    ccnxName_Release(&name);

    PARCBuffer *keyid = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), 5, (uint8_t *) "keyid"));
    ccnxValidationRsaSha256_Set(contentObject, keyid, NULL);
    parcBuffer_Release(&keyid);

    PARCBuffer *sigbits = parcBuffer_Flip(parcBuffer_PutArray(parcBuffer_Allocate(20), 13, (uint8_t *) "the signature"));

    switch (ccnxTlvDictionary_GetSchemaVersion(contentObject)) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            ccnxValidationFacadeV1_SetPayload(contentObject, sigbits);
            break;
        default:
            trapNotImplemented("Unsupprted schema version in createSignedContentObject()");
            break;
    }

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
        { .valid = false, .segnum = 0,                  .uri = "ccnx:/foo/bar"                              },
        { .valid = true,  .segnum = 0,                  .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=%00"      },
        { .valid = true,  .segnum = 0x1020,             .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=%10%20"   },
        { .valid = true,  .segnum = 0x6162,             .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=ab"       },
        { .valid = true,  .segnum = 0x616263,           .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abc"      },
        { .valid = true,  .segnum = 0x61626364,         .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abcd"     },
        { .valid = true,  .segnum = 0x6162636465,       .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abcde"    },
        { .valid = true,  .segnum = 0x616263646566,     .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abcdef"   },
        { .valid = true,  .segnum = 0x61626364656667,   .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abcdefg"  },
        { .valid = true,  .segnum = 0x6162636465666768, .uri = "ccnx:/foo/" CCNxNameLabel_Chunk "=abcdefgh" },
        { .valid = false, .segnum = 0,                  .uri = NULL                                         }
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


// =================================================================
// Tests related to the FinalBlockId and how the publisher sets it in
// a stream of content objects

#define DO_NOT_SET  ((uint64_t) -1)
#define SENTINEL    ((uint64_t) -1)

typedef struct test_vector {
    uint64_t chunk;
    uint64_t setFinalBlockId;
    bool isLast;
    bool interestReceived;
    bool dataReceived;
} TestVector;


static void
_verifyFlowStartNotification(TestData *data, TransportMessage *notify)
{
    assertNotNull(notify, "got null transport message back up the queue, expecting status\n");

    assertTrue(transportMessage_IsControl(notify),
               "Transport message is not a control object")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(notify), 0);
    }

    CCNxTlvDictionary *test_dict = transportMessage_GetDictionary(notify);

    PARCJSON *json = ccnxControlFacade_GetJson(test_dict);

    NotifyStatus *status = notifyStatus_ParseJSON(json);

    assertNotNull(status, "Could not parse NotifyStatus JSON message");
    assertTrue(notifyStatus_GetFiledes(status) == data->mock->connection->api_fd,
               "Expected file descriptor %d, actual %d\n", data->mock->connection->api_fd, notifyStatus_GetFiledes(status));

    assertTrue(notifyStatus_IsFlowControlStarted(status),
               "Expected notifyStatus_IsFlowControlStarted to be true, actual code %d", notifyStatus_GetStatusCode(status));

    notifyStatus_Release(&status);
}


static CCNxName *
_startFlow(TestData *data)
{
    TransportMessage *downInterest = trafficTools_CreateTransportMessageWithInterest(data->mock->connection);
    CCNxName *sessionName = ccnxName_Acquire(ccnxInterest_GetName(transportMessage_GetDictionary(downInterest)));
    PARCEventQueue *upperQueue = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);

    rtaComponent_PutMessage(upperQueue, downInterest);
    rtaFramework_NonThreadedStepCount(data->mock->framework, 10);

    // we should see a status message up the stack and interests
    // going down the stack.

    TransportMessage *notify = rtaComponent_GetMessage(upperQueue);
    _verifyFlowStartNotification(data, notify);
    transportMessage_Destroy(&notify);

    return sessionName;
}

/*
 * Caveat: this only works because we create a single session
 */
static VegasSession *
_grabSession(TestData *data, CCNxName *name)
{
    VegasConnectionState *fc = rtaConnection_GetPrivateData(data->mock->connection, FC_VEGAS);

    FcSessionHolder *holder = vegas_LookupSessionByName(fc, name);

    assertNotNull(holder, "Could not find the session holder in the flow controller");
    return holder->session;
}

/*
 * a tick is 1 milli-second, but it could be different depending on how
 * the framework is started
 */
static void
_bumpTime(TestData *data, unsigned ticks, CCNxName *name)
{
    data->mock->framework->clock_ticks += ticks;
    vegasSession_TimerCallback(-1, PARCEventType_Timeout, _grabSession(data, name));
}

static uint64_t
_getChunkNumberFromName(const CCNxName *name)
{
    size_t segmentCount = ccnxName_GetSegmentCount(name);
    CCNxNameSegment *lastSegment = ccnxName_GetSegment(name, segmentCount - 1);
    CCNxNameLabelType nameType = ccnxNameSegment_GetType(lastSegment);
    assertTrue(nameType == CCNxNameLabelType_CHUNK, "Wrong segment type got %d expected %d", nameType, CCNxNameLabelType_CHUNK);
    uint64_t chunkNumber = ccnxNameSegmentNumber_Value(lastSegment);
    return chunkNumber;
}

static TestVector *
_getVector(TestVector *vectors, uint64_t chunkNumber)
{
    // find the test vector for this chunk
    for (int i = 0; vectors[i].chunk != SENTINEL; i++) {
        if (vectors[i].chunk == chunkNumber) {
            return &vectors[i];
        }
    }
    trapIllegalValue(chunkNumber, "Could not find chunk number in test vector");
}

static TransportMessage *
_createReponseContentObject(CCNxName *name, uint64_t finalBlockid)
{
    CCNxContentObject *obj = ccnxContentObject_CreateWithNameAndPayload(name, NULL);
    assertNotNull(obj, "Got null content object.");

    if (finalBlockid != DO_NOT_SET) {
        bool success = ccnxContentObject_SetFinalChunkNumber(obj, finalBlockid);
        assertTrue(success, "Failed to set final chunk number");
    }

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromContentObject(obj);
    TransportMessage *response = transportMessage_CreateFromDictionary(message);

    ccnxMetaMessage_Release(&message);
    ccnxContentObject_Release(&obj);

    return response;
}

/*
 * Returns true if the unit test is finished
 */
static bool
_respondToDownInterest(TestData *data, TestVector *vectors)
{
    PARCEventQueue *lowerQueue = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    bool finished = false;
    TransportMessage *msg = rtaComponent_GetMessage(lowerQueue);
    if (msg) {
        // it should be an Interest with a chunk number
        assertTrue(transportMessage_IsInterest(msg), "Got unexpected message")
        {
            ccnxTlvDictionary_Display(transportMessage_GetDictionary(msg), 3);
        }

        CCNxTlvDictionary *interestDictionary = transportMessage_GetDictionary(msg);
        CCNxName *name = ccnxInterest_GetName(interestDictionary);
        uint64_t chunkNumber = _getChunkNumberFromName(name);

        TestVector *vector = _getVector(vectors, chunkNumber);

        vector->interestReceived = true;

        // create a content object and set the FinalBlockId if vector says to
        TransportMessage *response = _createReponseContentObject(name, vector->setFinalBlockId);
        RtaConnection *connection = transportMessage_GetInfo(msg);
        RtaConnection *connectionRef = rtaConnection_Copy(connection);
        transportMessage_SetInfo(response, connectionRef, rtaConnection_FreeFunc);

        rtaComponent_PutMessage(lowerQueue, response);

        finished = vector->isLast;

        transportMessage_Destroy(&msg);
    }
    return finished;
}

/*
 * Returns true if received the last message
 */
static bool
_consumeUpperContentObject(TestData *data, TestVector *vectors)
{
    PARCEventQueue *upperQueue = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);

    bool finished = false;
    TransportMessage *msg = rtaComponent_GetMessage(upperQueue);
    if (msg) {
        // it should be a content object
        assertTrue(transportMessage_IsContentObject(msg), "Got unexpected message")
        {
            ccnxTlvDictionary_Display(transportMessage_GetDictionary(msg), 3);
        }

        CCNxTlvDictionary *objectDictionary = transportMessage_GetDictionary(msg);
        CCNxName *name = ccnxContentObject_GetName(objectDictionary);
        uint64_t chunkNumber = _getChunkNumberFromName(name);

        TestVector *vector = _getVector(vectors, chunkNumber);

        // we should not have seen it before
        assertFalse(vector->dataReceived, "Duplicate Content Object chunk %" PRIu64, chunkNumber)
        {
            ccnxName_Display(name, 3);
        }

        vector->dataReceived = true;

        finished = vector->isLast;

        transportMessage_Destroy(&msg);
    }

    return finished;
}

static void
_runTestVector(TestData *data, TestVector vectors[])
{
    CCNxName *sessionName = _startFlow(data);

    bool finished = false;

    while (!finished) {
        rtaFramework_NonThreadedStep(data->mock->framework);
        finished = _respondToDownInterest(data, vectors);

        rtaFramework_NonThreadedStep(data->mock->framework);
        finished &= _consumeUpperContentObject(data, vectors);

        if (!finished) {
            _bumpTime(data, 5, sessionName);
        }
    }

    ccnxName_Release(&sessionName);
}

/*
 * First chunk sets final block ID, last chunk does not.  Should keep reading until
 * the real last chunk set to itself.
 */
LONGBOW_TEST_CASE(Local, vegasSession_ReceiveContentObject_InOrder_FirstBlockSetsLastDoesNotFinalId)
{
    TestVector vectors[] = {
        { .chunk = 0,        .setFinalBlockId = 5,          .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 1,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 2,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 3,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 4,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 5,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 6,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 7,        .setFinalBlockId = 7,          .isLast = true,  .interestReceived = false, .dataReceived = false },
        { .chunk = SENTINEL, .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
    };

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _runTestVector(data, vectors);
}

/*
 * FinalBlockId unset until last chunk, which sets to itself
 */
LONGBOW_TEST_CASE(Local, vegasSession_ReceiveContentObject_InOrder_LastBlockSetsFinalId)
{
    TestVector vectors[] = {
        { .chunk = 0,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 1,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 2,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 3,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 4,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 5,        .setFinalBlockId = 5,          .isLast = true,  .interestReceived = false, .dataReceived = false },
        { .chunk = SENTINEL, .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
    };

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _runTestVector(data, vectors);
}

/*
 * First chunk sets FinalBlockId and last chunks, and last chunk sets it to itself
 */
LONGBOW_TEST_CASE(Local, vegasSession_ReceiveContentObject_InOrder_FirstAndLastBlocksSetsFinalId)
{
    TestVector vectors[] = {
        { .chunk = 0,        .setFinalBlockId = 7,          .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 1,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 2,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 3,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 4,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 5,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 6,        .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
        { .chunk = 7,        .setFinalBlockId = 7,          .isLast = true,  .interestReceived = false, .dataReceived = false },
        { .chunk = SENTINEL, .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false },
    };

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _runTestVector(data, vectors);
}

// ============================================

LONGBOW_TEST_FIXTURE(IterateFinalChunkNumber)
{
    LONGBOW_RUN_TEST_CASE(IterateFinalChunkNumber, vegasSession_ReceiveContentObject_InOrder_FirstSetsSecondIncreasesLastSetsFinalId);
}

LONGBOW_TEST_FIXTURE_SETUP(IterateFinalChunkNumber)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(IterateFinalChunkNumber)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * First chunk sets FinalBlockId, later chunk increases it by N, then final chunk set to self.
 *
 * In this test, we programmatically create the TestVector array so we can run different iterations of N.
 */
LONGBOW_TEST_CASE(IterateFinalChunkNumber, vegasSession_ReceiveContentObject_InOrder_FirstSetsSecondIncreasesLastSetsFinalId)
{
    const unsigned minsize = 5;
    const unsigned maxsize = 20;

    for (unsigned size = minsize; size < maxsize; size++) {
        longBowTestCase_SetClipBoardData(testCase, _commonSetup(longBowTestCase_GetName(testCase)));

        TestVector vectors[size];

        // set initial state
        for (int i = 0; i < size; i++) {
            vectors[i] = (TestVector) { .chunk = i, .setFinalBlockId = DO_NOT_SET, .isLast = false, .interestReceived = false, .dataReceived = false };
        }

        // first vectors sets it to minsize
        vectors[0].setFinalBlockId = minsize;

        // minsize sets it to the end
        vectors[minsize - 1].setFinalBlockId = size;

        // last one sets it to itself
        vectors[size - 1].setFinalBlockId = size;
        vectors[size - 1].isLast = true;

        TestData *data = longBowTestCase_GetClipBoardData(testCase);
        _runTestVector(data, vectors);

        _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

        uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
        assertTrue(outstandingAllocations == 0, "Memory leak for size %u", size);
    }
}



// ============================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(VegasSession);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
