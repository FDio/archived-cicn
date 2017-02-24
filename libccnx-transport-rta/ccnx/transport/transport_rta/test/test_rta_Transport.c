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

#include "../rta_Transport.c"
#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_private.h>
#include <ccnx/transport/transport_rta/components/component_Testing.h>

#include <ccnx/common/ccnx_WireFormatMessage.h>

#include <parc/algol/parc_SafeMemory.h>
#include <ccnx/transport/test_tools/traffic_tools.h>

#include <LongBow/unit-test.h>

typedef struct test_data {
    RTATransport *transport;
    CCNxMetaMessage *msg;
} TestData;

static TestData *
_commonSetup(void)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->transport = rtaTransport_Create();
    return data;
}

static void
_commonTeardown(TestData *data)
{
    rtaTransport_Destroy(&data->transport);
    if (data->msg) {
        ccnxMetaMessage_Release(&data->msg);
    }

    parcMemory_Deallocate((void **) &data);
}

static CCNxTransportConfig *
createSimpleConfig(TestData *data)
{
    // API connector -> Testing Lower component

    CCNxStackConfig *stackConfig =
        testingLower_ProtocolStackConfig(apiConnector_ProtocolStackConfig(ccnxStackConfig_Create()));

    CCNxConnectionConfig *connConfig =
        testingLower_ConnectionConfig(
            tlvCodec_ConnectionConfig(
                apiConnector_ConnectionConfig(
                    ccnxConnectionConfig_Create())));

    protocolStack_ComponentsConfigArgs(stackConfig, apiConnector_GetName(), testingLower_GetName(), NULL);

    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

/**
 * Peek inside the RTA framework's connection table
 *
 * We will look inside the RTA framework's thread to find a connection by the API_FD.
 *
 * @param [in] data The test data, holding the transport
 * @param [in] api_fd The API FD to lookup
 * @param [in] usec_timeout How long to busy wait looking in the connection table (micro seconds)
 *
 * @return NULL It was not in the table after the timeout period
 * @return non-null The connection corresponding to api_fd
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static RtaConnection *
lookupRtaConnectionInsideFramework(TestData *data, int api_fd, unsigned usec_timeout)
{
    // busy loop looking for connection to give RTA thread time to process it.
    // Remember, we're operating in the "API" thread when issuing these commands.
    struct timeval t0;
    long timer_usec = 0;
    gettimeofday(&t0, NULL);
    bool timeout = false;
    RtaConnection *conn = NULL;
    while (conn == NULL && !timeout) {
        usleep(500);
        conn = rtaConnectionTable_GetByApiFd(data->transport->framework->connectionTable, api_fd);
        struct timeval t1;
        gettimeofday(&t1, NULL);
        timersub(&t1, &t0, &t1);
        timer_usec = t1.tv_sec * 1000000 + t1.tv_usec;
        timeout = timer_usec > usec_timeout ? true : false;
    }

    if (conn) {
        printf("Found connection %p after %.6f seconds\n", (void *) conn, timer_usec * 1E-6);
    }

    return conn;
}

/**
 * Wait for a connection to go away
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
lookupNullRtaConnectionInsideFramework(TestData *data, int api_fd, unsigned usec_timeout)
{
    // busy loop looking for connection to give RTA thread time to process it.
    // Remember, we're operating in the "API" thread when issuing these commands.
    struct timeval t0;
    long timer_usec = 0;
    gettimeofday(&t0, NULL);
    bool timeout = false;

    // initialize to non-null
    RtaConnection *conn = (void *) 1;
    while (conn != NULL && !timeout) {
        usleep(500);
        conn = rtaConnectionTable_GetByApiFd(data->transport->framework->connectionTable, api_fd);
        struct timeval t1;
        gettimeofday(&t1, NULL);
        timersub(&t1, &t0, &t1);
        timer_usec = t1.tv_sec * 1000000 + t1.tv_usec;
        timeout = timer_usec > usec_timeout ? true : false;
    }

    if (conn == NULL) {
        printf("Found no connection %p after %.6f seconds\n", (void *) conn, timer_usec * 1E-6);
    }

    // if its null, return true
    return (conn == NULL);
}


// ==================================================================================
// Runner

LONGBOW_TEST_RUNNER(rta_Transport)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_Transport)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_Transport)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================================================================
// Global

LONGBOW_TEST_FIXTURE(Global)
{
    // These are still static functions, but they are the function pointers used
    // in the transport function structure.   They comprise the public API.
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Close);
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Open);
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_PassCommand);

    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Recv_OK);
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Recv_WouldBlock);

    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Send_OK);
    LONGBOW_RUN_TEST_CASE(Global, rtaTransport_Send_WouldBlock);

//    LONGBOW_RUN_TEST_CASE(Global, unrecoverable);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, rtaTransport_Close)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    int api_fd = rtaTransport_Open(data->transport, config);

    RtaConnection *conn = lookupRtaConnectionInsideFramework(data, api_fd, 1E+6);
    assertNotNull(conn, "Could not find connection");

    rtaTransport_Close(data->transport, api_fd);

    // now wait until it's gone
    bool gone = lookupNullRtaConnectionInsideFramework(data, api_fd, 1E+6);
    assertTrue(gone, "Did not remove connection after 1 second timeout");

    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Global, rtaTransport_Create_Destroy)
{
    RTATransport *transport = rtaTransport_Create();
    assertNotNull(transport, "rtaTransport_Create() returns NULL");

    rtaTransport_Destroy(&transport);
    assertNull(transport, "rtaTransport_Destroy did not null paramter");
}

LONGBOW_TEST_CASE(Global, rtaTransport_Open)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    int api_fd = rtaTransport_Open(data->transport, config);

    RtaConnection *conn = lookupRtaConnectionInsideFramework(data, api_fd, 1E+6);
    assertNotNull(conn, "Could not find connection");

    ccnxTransportConfig_Destroy(&config);
}

/**
 * PassCommand sends a user RTA Command over the command channel.
 * This test will intercept the transport side of the command channel so
 * we can easily verify the command went through.
 */
LONGBOW_TEST_CASE(Global, rtaTransport_PassCommand)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCRingBuffer1x1 *previousRingBuffer = data->transport->commandRingBuffer;
    PARCNotifier *previousNotifier = data->transport->commandNotifier;

    PARCRingBuffer1x1 *testRingBuffer = parcRingBuffer1x1_Create(32, NULL);
    PARCNotifier *testNotifier = parcNotifier_Create();


    // Insert our new socket pair so we can intercept the commands
    // No acquire here because we will be resetting them and destroying all in this scope
    data->transport->commandRingBuffer = testRingBuffer;
    data->transport->commandNotifier = testNotifier;

    // Create a simple command to send
    RtaCommand *command = rtaCommand_CreateShutdownFramework();
    rtaTransport_PassCommand(data->transport, command);
    rtaCommand_Release(&command);

    RtaCommand *testCommand = rtaCommand_Read(testRingBuffer);
    assertNotNull(testCommand, "Got null command from the ring buffer.");
    assertTrue(rtaCommand_IsShutdownFramework(testCommand), "Command not a shutdown framework");

    // All's well

    rtaCommand_Release(&testCommand);

    // now restore the sockets so things close up nicely
    data->transport->commandRingBuffer = previousRingBuffer;
    data->transport->commandNotifier = previousNotifier;

    parcRingBuffer1x1_Release(&testRingBuffer);
    parcNotifier_Release(&testNotifier);
}

LONGBOW_TEST_CASE(Global, rtaTransport_Recv_OK)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    int api_fd, transport_fd;

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(data->transport, 128 * 1024);
    api_fd = pair.up;
    transport_fd = pair.down;

    // Set non-blocking flag
    int flags = fcntl(api_fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(api_fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    char *buffer = "born free, as free as the wind blows";
    ssize_t nwritten = write(transport_fd, &buffer, sizeof(&buffer));
    assertTrue(nwritten == sizeof(&buffer), "Wrong write size, expected %zu got %zd", sizeof(&buffer), nwritten);

    CCNxMetaMessage *msg = NULL;
    TransportIOStatus result = rtaTransport_Recv(data->transport, api_fd, &msg, CCNxStackTimeout_Never);
    assertTrue(result != TransportIOStatus_Error, "Failed to read a good socket");
    assertTrue((void *) msg == (void *) buffer, "Read wrong pointer, got %p expected %p", (void *) msg, (void *) buffer);

    close(api_fd);
    close(transport_fd);
}

LONGBOW_TEST_CASE(Global, rtaTransport_Recv_WouldBlock)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    int api_fd, transport_fd;

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(data->transport, 128 * 1024);
    api_fd = pair.up;
    transport_fd = pair.down;

    // Set non-blocking flag
    int flags = fcntl(api_fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(api_fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // Don't write anything

    CCNxMetaMessage *msg = NULL;
    TransportIOStatus result = rtaTransport_Recv(data->transport, api_fd, &msg, CCNxStackTimeout_Immediate);
    assertTrue(result == TransportIOStatus_Timeout, "Should have returned failure due to blocking");

    close(api_fd);
    close(transport_fd);
}


/**
 * This function will receive what the API Connector sends down the stack
 */
static void
mockDowncallRead(PARCEventQueue *queue, PARCEventType type, void *stack)
{
    TransportMessage *tm = rtaComponent_GetMessage(queue);
    assertNotNull(tm, "got null transport message");

    CCNxTlvDictionary *dictionary = transportMessage_GetDictionary(tm);
    CCNxCodecNetworkBufferIoVec *vec = ccnxWireFormatMessage_GetIoVec(dictionary);
    const struct iovec *iov = ccnxCodecNetworkBufferIoVec_GetArray(vec);

    // we encapsualted a pointer to this counter inside the wire format
    unsigned *downcallReadCountPtr = iov[0].iov_base;
    (*downcallReadCountPtr)++;

    transportMessage_Destroy(&tm);
}

CCNxCodecNetworkBufferMemoryBlockFunctions memfunc = {
    .allocator   = NULL,
    .deallocator = NULL
};

/**
 * This test does not actually need to receive the message in TestingLower.  It could have passed
 * any socket pair to rtaTransport_Send and inspected the result immediately.
 */
LONGBOW_TEST_CASE(Global, rtaTransport_Send_OK)
{
    testing_null_ops.downcallRead = mockDowncallRead;

    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    unsigned downcallReadCount = 0;

    CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&memfunc, NULL, sizeof(downcallReadCount), (uint8_t *) &downcallReadCount);
    CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
    CCNxTlvDictionary *wire = ccnxWireFormatMessage_FromInterestPacketTypeIoVec(CCNxTlvDictionary_SchemaVersion_V1, vec);

    int api_fd = rtaTransport_Open(data->transport, config);

    CCNxMetaMessage *msg = ccnxMetaMessage_Acquire(wire);
    bool success = rtaTransport_Send(data->transport, api_fd, msg, CCNxStackTimeout_Never);
    assertTrue(success, "Got error writing to api_fd %d\n", api_fd);
    ccnxMetaMessage_Release(&msg);

    // now spin on it
    unsigned maxTries = 2000;   // about 1 second
    while ((downcallReadCount == 0) && (maxTries > 0)) {
        maxTries--;
        usleep(500);
    }

    printf("Read message after %d tries\n", 2000 - maxTries);

    ccnxTlvDictionary_Release(&wire);
    ccnxCodecNetworkBufferIoVec_Release(&vec);
    ccnxCodecNetworkBuffer_Release(&netbuff);

    ccnxTransportConfig_Destroy(&config);
}

/**
 * Fill up the socket with junk, then make sure it would blocks
 */
LONGBOW_TEST_CASE(Global, rtaTransport_Send_WouldBlock)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    int api_fd, transport_fd;

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(data->transport, 128 * 1024);
    api_fd = pair.up;
    transport_fd = pair.down;

    // Set non-blocking flag
    int flags = fcntl(api_fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(api_fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // write junk until it would block
    char buffer[1024];
    while (write(api_fd, buffer, 1024) > 0) {
        ;
    }

    assertTrue(errno == EWOULDBLOCK, "wrote until it would block, but got some other error: (%d) %s", errno, strerror(errno));

    // now call the function to test and make sure it does the right thing
    // if it would block
    CCNxTlvDictionary *interest = trafficTools_CreateDictionaryInterest();
    CCNxMetaMessage *msg = ccnxMetaMessage_CreateFromInterest(interest);

    bool success = rtaTransport_Send(data->transport, api_fd, msg, CCNxStackTimeout_Immediate);
    printf("success %d, errno %d expected %d\n", success, errno, EWOULDBLOCK);

    assertFalse(success, "Send did not return a failure, even though it would have blocked");
    assertTrue(errno == EWOULDBLOCK, "wrote until it would block, but got some other error: (%d) %s", errno, strerror(errno));

    ccnxMetaMessage_Release(&msg);
    ccnxTlvDictionary_Release(&interest);

    close(api_fd);
    close(transport_fd);
}

/**
 * Pass it an invalid socket.  This will cause a trap in the send code.
 */
LONGBOW_TEST_CASE_EXPECTS(Global, rtaTransport_Send_Error, .event = &LongBowTrapUnrecoverableState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTlvDictionary *interest = trafficTools_CreateDictionaryInterest();
    data->msg = ccnxMetaMessage_Acquire(interest);
    ccnxTlvDictionary_Release(&interest);

    rtaTransport_Send(data->transport, 999, data->msg, CCNxStackTimeout_Immediate);
}

LONGBOW_TEST_CASE_EXPECTS(Global, unrecoverable, .event = &LongBowTrapUnrecoverableState)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTlvDictionary *interest = trafficTools_CreateDictionaryInterest();
    data->msg = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxTlvDictionary_Release(&interest);

    rtaTransport_Send(NULL, 999, data->msg, CCNxStackTimeout_Immediate);

    ccnxMetaMessage_Release(&(data->msg));
}

// ==================================================================================
// Local

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_AddStack);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_GetStack);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_GetStack_Missing);

    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_CreateSocketPair);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_GetProtocolStackEntry_Exists);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_GetProtocolStackEntry_NotExists);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_AddProtocolStackEntry);
    LONGBOW_RUN_TEST_CASE(Local, _rtaTransport_CreateConnection);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
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

LONGBOW_TEST_CASE(Local, _rtaTransport_CreateSocketPair)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    int a, b;

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(data->transport, 128 * 1024);
    a = pair.up;
    b = pair.down;
    assertFalse(a < 0, "socket a is error: %d", a);
    assertFalse(b < 0, "socket b is error: %d", b);

    ssize_t nwritten = write(a, &a, sizeof(a));
    assertTrue(nwritten == sizeof(a), "Wrong write size, expected %zu got %zd", sizeof(a), nwritten);

    int test;
    ssize_t nread = read(b, &test, sizeof(test));
    assertTrue(nread == sizeof(test), "Wrong read size, expected %zu got %zd", sizeof(test), nread);

    assertTrue(test == a, "read wrong value, got %d wrote %d", test, a);

    close(a);
    close(b);
}


LONGBOW_TEST_CASE(Local, _rtaTransport_GetProtocolStackEntry_Exists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

//    uint64_t hash = ccnxStackConfig_HashCode(ccnxTransportConfig_GetStackConfig(config));

    _StackEntry *truth = _rtaTransport_AddStack(data->transport, ccnxTransportConfig_GetStackConfig(config));

    _StackEntry *test = _rtaTransport_GetProtocolStackEntry(data->transport, config);

    assertTrue(test == truth, "Wrong pointer, got %p expected %p", (void *) test, (void *) truth);

    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Local, _rtaTransport_GetProtocolStackEntry_NotExists)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    _rtaTransport_AddStack(data->transport, ccnxTransportConfig_GetStackConfig(config));

    // Now create the missing one to lookup
    // this one will have 2x api connectors listed
    CCNxStackConfig *missingStackConfig =
        apiConnector_ProtocolStackConfig(apiConnector_ProtocolStackConfig(ccnxStackConfig_Create()));
    CCNxConnectionConfig *missingConnConfig = apiConnector_ConnectionConfig(ccnxConnectionConfig_Create());

    CCNxTransportConfig *missingConfig = ccnxTransportConfig_Create(missingStackConfig, missingConnConfig);
    ccnxStackConfig_Release(&missingStackConfig);

    _StackEntry *test = _rtaTransport_GetProtocolStackEntry(data->transport, missingConfig);

    assertNull(test, "Wrong pointer, got %p expected %p", (void *) test, (void *) NULL);
    ccnxTransportConfig_Destroy(&missingConfig);
    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Local, _rtaTransport_AddProtocolStackEntry)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    _StackEntry *entry = _rtaTransport_AddProtocolStackEntry(data->transport, config);
    assertNotNull(entry, "Got null entry from _rtaTransport_AddProtocolStackEntry");

    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Local, _rtaTransport_CreateConnection)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxTransportConfig *config = createSimpleConfig(data);

    _StackEntry *entry = _rtaTransport_AddProtocolStackEntry(data->transport, config);

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(data->transport, 128 * 1024);

    _rtaTransport_CreateConnection(data->transport, config, entry, pair);

    // wait up to 1 second
    RtaConnection *conn = lookupRtaConnectionInsideFramework(data, pair.up, 1E+6);
    assertNotNull(conn, "Could not find connection in connection table, timeout at %.6f seconds", 1.0);

    ccnxTransportConfig_Destroy(&config);
}

LONGBOW_TEST_CASE(Local, _rtaTransport_AddStack)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
    _StackEntry *entry = _rtaTransport_AddStack(data->transport, stackConfig);

    uint64_t hash = ccnxStackConfig_HashCode(stackConfig);
    _StackEntry *test = _rtaTransport_GetStack(data->transport, hash);
    assertTrue(test == entry, "Wrong pointer, got %p expected %p", (void *) test, (void *) entry);

    ccnxStackConfig_Release(&stackConfig);
}

LONGBOW_TEST_CASE(Local, _rtaTransport_GetStack)
{
    struct test_vector {
        uint64_t hash;
        int stackid;
        _StackEntry *entry;
    } vector[] = {
        { .hash = 20,  .stackid = 30, .entry = NULL },
        { .hash = 10,  .stackid = 77, .entry = NULL },
        { .hash = 990, .stackid = 31, .entry = NULL },
        { .hash = 0,   .stackid = 0,  .entry = NULL },
    };

    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();

    char key[10];
    for (int i = 0; vector[i].hash != 0; i++) {
        sprintf(key, "key%d", i);
        PARCJSONValue *json = parcJSONValue_CreateFromNULL();
        ccnxStackConfig_Add(stackConfig, key, json);
        parcJSONValue_Release(&json);
        vector[i].hash = ccnxStackConfig_HashCode(stackConfig);
        vector[i].entry = _rtaTransport_AddStack(data->transport, stackConfig);
    }
    ccnxStackConfig_Release(&stackConfig);

    // now look them up
    for (int i = 0; vector[i].hash != 0; i++) {
        _StackEntry *test = _rtaTransport_GetStack(data->transport, vector[i].hash);
        assertTrue(test == vector[i].entry, "Wrong pointer, got %p expected %p", (void *) test, (void *) vector[i].entry);
    }
}

LONGBOW_TEST_CASE(Local, _rtaTransport_GetStack_Missing)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();
    _rtaTransport_AddStack(data->transport, stackConfig);

    PARCJSONValue *json = parcJSONValue_CreateFromNULL();
    ccnxStackConfig_Add(stackConfig, "someKey", json);
    parcJSONValue_Release(&json);

    _StackEntry *test = _rtaTransport_GetStack(data->transport, ccnxStackConfig_HashCode(stackConfig));

    ccnxStackConfig_Release(&stackConfig);
    assertNull(test, "Wrong pointer, got %p expected %p", (void *) test, NULL);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_Transport);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
