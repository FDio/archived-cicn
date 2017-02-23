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
#include "../metis_Dispatcher.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventQueue.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

LONGBOW_TEST_RUNNER(metis_Dispatcher)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
#ifdef PARC_MEMORY
    // The CreateDestroy fixture diagnoses issues with the debug memory allocator,
    // which will fail if that allocator is not in use.
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
#endif
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(StreamBufferConnect);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Dispatcher)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Dispatcher)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// =================================================================================
LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, metisDispatcher_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, metisDispatcher_Memory);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, metisDispatcher_Create_Destroy)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisDispatcher_Destroy(&dispatcher);
    metisLogger_Release(&logger);
    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Got memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

/**
 * Ensure that the dispatcher is using parc memory inside event scheduler
 */
LONGBOW_TEST_CASE(CreateDestroy, metisDispatcher_Memory)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    size_t baseline = parcMemory_Outstanding();

    PARCEventBuffer *buffer = parcEventBuffer_Create();

    assertTrue(parcMemory_Outstanding() > baseline,
               "parcEventBuffer_Create() did not increase parcMemory_Outstanding: baseline %zu now %u",
               baseline,
               parcMemory_Outstanding());

    parcEventBuffer_Destroy(&buffer);

    assertTrue(parcMemory_Outstanding() == baseline,
               "parcEventBuffer_Destroy() did reduce to baseline: baseline %zu now %u",
               baseline,
               parcMemory_Outstanding());

    metisDispatcher_Destroy(&dispatcher);
    metisLogger_Release(&logger);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Got memory imbalance on create/destroy: %u", parcMemory_Outstanding());
}

// =================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisDispatcher_CreateTimer_Oneshot);
    LONGBOW_RUN_TEST_CASE(Global, metisDispatcher_CreateTimer_Periodic);
    LONGBOW_RUN_TEST_CASE(Global, metisDispatcher_StopTimer);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);

    longBowTestCase_SetClipBoardData(testCase, dispatcher);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    MetisDispatcher *dispatcher = longBowTestCase_GetClipBoardData(testCase);
    metisDispatcher_Destroy(&dispatcher);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static void
timerCallback(int fd, PARCEventType which_event, void *user_data)
{
    assertTrue(which_event & PARCEventType_Timeout, "Event incorrect, expecting %X set, got %X", PARCEventType_Timeout, which_event);
    (*(unsigned *) user_data)++;
}

LONGBOW_TEST_CASE(Global, metisDispatcher_CreateTimer_Oneshot)
{
    MetisDispatcher *dispatcher = longBowTestCase_GetClipBoardData(testCase);

    unsigned timerCallbackCount = 0;
    PARCEventTimer *event = metisDispatcher_CreateTimer(dispatcher, false, timerCallback, &timerCallbackCount);
    assertNotNull(event, "Got null event from metisDispatcher_CreateTimer");

    // 10 msec
    struct timeval timeout = { 0, 10000 };
    metisDispatcher_StartTimer(dispatcher, event, &timeout);

    // run for 250 msec;
    struct timeval runtime = { 0, 250000 };
    metisDispatcher_RunDuration(dispatcher, &runtime);

    assertTrue(timerCallbackCount == 1, "Incorrect timer callbacks, expected %u got %u", 1, timerCallbackCount);
    metisDispatcher_DestroyTimerEvent(dispatcher, &event);
}

LONGBOW_TEST_CASE(Global, metisDispatcher_CreateTimer_Periodic)
{
#ifdef __ARMEL__
    testUnimplemented("Test not implemented on ARMEL, timers too inaccurate");
#else
    MetisDispatcher *dispatcher = longBowTestCase_GetClipBoardData(testCase);

    unsigned timerCallbackCount = 0;
    PARCEventTimer *event = metisDispatcher_CreateTimer(dispatcher, true, timerCallback, &timerCallbackCount);
    assertNotNull(event, "Got null event from metisDispatcher_CreateTimer");

    // 10 msec
    struct timeval timeout = { 0, 10000 };
    metisDispatcher_StartTimer(dispatcher, event, &timeout);

    // run for 255 msec.  Use an offset time to run so its clear what we should be after
    // the 25th  event and before the 26th event.

    struct timeval runtime = { 0, 255000 };
    metisDispatcher_RunDuration(dispatcher, &runtime);

    // make sure it runs at least twice (is periodic).  Could run as many as 25.
    assertTrue(timerCallbackCount >= 2, "Incorrect timer callbacks, expected at least %u got %u", 2, timerCallbackCount);
    metisDispatcher_DestroyTimerEvent(dispatcher, &event);
#endif
}

LONGBOW_TEST_CASE(Global, metisDispatcher_StopTimer)
{
    MetisDispatcher *dispatcher = longBowTestCase_GetClipBoardData(testCase);

    unsigned timerCallbackCount = 0;
    PARCEventTimer *event = metisDispatcher_CreateTimer(dispatcher, true, timerCallback, &timerCallbackCount);
    assertNotNull(event, "Got null event from metisDispatcher_CreateTimer");

    // 10 msec
    struct timeval timeout = { 0, 10000 };
    metisDispatcher_StartTimer(dispatcher, event, &timeout);

    // run for 55 msec (5 events), then stop the timer and run another 55 msec

    struct timeval runtime = { 0, 55000 };
    metisDispatcher_RunDuration(dispatcher, &runtime);

    metisDispatcher_StopTimer(dispatcher, event);
    metisDispatcher_RunDuration(dispatcher, &runtime);

    // not sure how many times it will fire, but it should not fire more than 5 times
    assertTrue(timerCallbackCount <= 5, "Incorrect timer callbacks, expected no more than %u got %u", 5, timerCallbackCount);
    metisDispatcher_DestroyTimerEvent(dispatcher, &event);
}

// =================================================================================

typedef struct open_connection_state {
//    MetisForwarder * metis;
    MetisDispatcher *dispatcher;

    CPIAddress *a;
    CPIAddress *b;
    MetisAddressPair *pair;
    size_t baselineMemoryBalance;

    // if serverSocket > 0, then its allocated
    int serverSocket;
    struct sockaddr *serverAddr;
    socklen_t serverAddrLength;
} OpenConnectionState;

static void
listenToInet(OpenConnectionState *ocs)
{
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = INPORT_ANY;

    int fd = socket(PF_INET, SOCK_STREAM, 0);
    assertFalse(fd < 0, "error on bind: (%d) %s", errno, strerror(errno));

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    failure = bind(fd, (struct sockaddr *) &server, sizeof(server));
    assertFalse(failure, "error on bind: (%d) %s", errno, strerror(errno));

    failure = listen(fd, 16);
    assertFalse(failure, "error on listen: (%d) %s", errno, strerror(errno));

    ocs->serverSocket = fd;
    ocs->serverAddrLength = sizeof(struct sockaddr_in);
    ocs->serverAddr = parcMemory_Allocate(ocs->serverAddrLength);
    assertNotNull(ocs->serverAddr, "parcMemory_Allocate(%u) returned NULL", ocs->serverAddrLength);

    failure = getsockname(fd, ocs->serverAddr, &ocs->serverAddrLength);
    assertFalse(failure, "error on getsockname: (%d) %s", errno, strerror(errno));
}

static void
listenToInet6(OpenConnectionState *ocs)
{
    struct sockaddr_in6 server;
    memset(&server, 0, sizeof(server));
    server.sin6_family = PF_INET6;
    server.sin6_addr = in6addr_any;
    server.sin6_port = INPORT_ANY;

    int fd = socket(PF_INET6, SOCK_STREAM, 0);
    assertFalse(fd < 0, "error on bind: (%d) %s", errno, strerror(errno));

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    failure = bind(fd, (struct sockaddr *) &server, sizeof(server));
    assertFalse(failure, "error on bind: (%d) %s", errno, strerror(errno));

    failure = listen(fd, 16);
    assertFalse(failure, "error on listen: (%d) %s", errno, strerror(errno));

    ocs->serverSocket = fd;
    ocs->serverAddrLength = sizeof(struct sockaddr_in6);
    ocs->serverAddr = parcMemory_Allocate(ocs->serverAddrLength);
    assertNotNull(ocs->serverAddr, "parcMemory_Allocate(%u) returned NULL", ocs->serverAddrLength);

    failure = getsockname(fd, ocs->serverAddr, &ocs->serverAddrLength);
    assertFalse(failure, "error on getsockname: (%d) %s", errno, strerror(errno));
}

LONGBOW_TEST_FIXTURE(StreamBufferConnect)
{
    // --------
    // these two tests will cause assertions
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_Invalid);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_DifferentTypes);
    // --------

    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6);


    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET_Success);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET_Failure);

    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6_Success);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6_Failure);

    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindFail);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindOk_ConnectFail);
    LONGBOW_RUN_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindOk_ConnectOk);
}

LONGBOW_TEST_FIXTURE_SETUP(StreamBufferConnect)
{
    size_t baselineMemoryBalance = parcMemory_Outstanding();

    OpenConnectionState *ocs = parcMemory_AllocateAndClear(sizeof(OpenConnectionState));
    assertNotNull(ocs, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(OpenConnectionState));
    memset(ocs, 0, sizeof(OpenConnectionState));

    longBowTestCase_SetClipBoardData(testCase, ocs);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    ocs->dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    ocs->baselineMemoryBalance = baselineMemoryBalance;

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(StreamBufferConnect)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    metisDispatcher_Destroy(&ocs->dispatcher);

    if (ocs->a) {
        cpiAddress_Destroy(&ocs->a);
    }

    if (ocs->b) {
        cpiAddress_Destroy(&ocs->b);
    }

    if (ocs->pair) {
        metisAddressPair_Release(&ocs->pair);
    }

    if (ocs->serverSocket > 0) {
        close(ocs->serverSocket);
    }

    if (ocs->serverAddr) {
        parcMemory_Deallocate((void **) &(ocs->serverAddr));
    }

    size_t baselineMemoryBalance = ocs->baselineMemoryBalance;
    parcMemory_Deallocate((void **) &ocs);

    if (parcMemory_Outstanding() != baselineMemoryBalance) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        assertTrue(parcMemory_Outstanding() == ocs->baselineMemoryBalance,
                   "memory imbalance in %s: exepcted %zu got %u",
                   longBowTestCase_GetName(testCase),
                   baselineMemoryBalance,
                   parcMemory_Outstanding());
    }


    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Tests invalid protocol family
 */
LONGBOW_TEST_CASE_EXPECTS(StreamBufferConnect, metisDispatcher_StreamBufferConnect_Invalid, .event = &LongBowTrapIllegalValue)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    ocs->a = cpiAddress_CreateFromInterface(1);
    ocs->b = cpiAddress_CreateFromInterface(2);
    ocs->pair = metisAddressPair_Create(ocs->a, ocs->b);

    // this will throw a trap
    fprintf(stderr, "\n\nTHIS IS NOT AN ERROR, EXPECTED TRAP: Assertion Illegal\n\n");
    metisDispatcher_StreamBufferConnect(ocs->dispatcher, ocs->pair);
}

/**
 * Tests different protocol families
 */
LONGBOW_TEST_CASE_EXPECTS(StreamBufferConnect, metisDispatcher_StreamBufferConnect_DifferentTypes, .event = &LongBowAssertEvent)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    ocs->a = cpiAddress_CreateFromInet(&((struct sockaddr_in) { .sin_family = PF_INET }));
    ocs->b = cpiAddress_CreateFromInet6(&((struct sockaddr_in6) { .sin6_family = PF_INET6 }));
    ocs->pair = metisAddressPair_Create(ocs->a, ocs->b);

    // this will throw a trap
    fprintf(stderr, "\n\nTHIS IS NOT AN ERROR, EXPECTED ASSERTION: Assertion cpiAddress_GetType...\n\n");
    metisDispatcher_StreamBufferConnect(ocs->dispatcher, ocs->pair);
}

/**
 * Use a port that is already in use
 */
LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindFail)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet(ocs);

    PARCEventQueue *buffer = parcEventQueue_Create(ocs->dispatcher->Base, -1, PARCEventQueueOption_CloseOnFree);

    // use the server address for our bind address.  will cause a failure.
    bool success = metisDispatcher_StreamBufferBindAndConnect(ocs->dispatcher, buffer,
                                                              ocs->serverAddr, ocs->serverAddrLength,
                                                              ocs->serverAddr, ocs->serverAddrLength);
    parcEventQueue_Destroy(&buffer);
    assertFalse(success, "metisDispatcher_StreamBufferBindAndConnect succedded with bind to in use address");
}

/**
 * Good bind address, but bad connect to address
 */
LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindOk_ConnectFail)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);

    struct sockaddr_in goodAddress;
    memset(&goodAddress, 0, sizeof(goodAddress));
    goodAddress.sin_family = PF_INET;
    goodAddress.sin_addr.s_addr = INADDR_ANY;
    goodAddress.sin_port = INPORT_ANY;

    struct sockaddr_in badAddress;
    memset(&badAddress, 0xFF, sizeof(badAddress));
    badAddress.sin_family = PF_INET;

    PARCEventQueue *buffer = parcEventQueue_Create(ocs->dispatcher->Base, -1, PARCEventQueueOption_CloseOnFree);

    // use the server address for our bind address.  will cause a failure.
    bool success = metisDispatcher_StreamBufferBindAndConnect(ocs->dispatcher, buffer,
                                                              (struct sockaddr *) &goodAddress, sizeof(goodAddress),
                                                              (struct sockaddr *) &badAddress, sizeof(badAddress));

    parcEventQueue_Destroy(&buffer);
    assertFalse(success, "metisDispatcher_StreamBufferBindAndConnect succedded with bind to in use address");
}

/**
 * Everything good, should succeed!
 */
LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferBindAndConnect_BindOk_ConnectOk)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet(ocs);

    struct sockaddr_in goodAddress;
    memset(&goodAddress, 0, sizeof(goodAddress));
    goodAddress.sin_family = PF_INET;
    goodAddress.sin_addr.s_addr = INADDR_ANY;
    goodAddress.sin_port = INPORT_ANY;

    PARCEventQueue *buffer = parcEventQueue_Create(ocs->dispatcher->Base, -1, PARCEventQueueOption_CloseOnFree);

    // use the server address for our bind address.  will cause a failure.
    bool success = metisDispatcher_StreamBufferBindAndConnect(ocs->dispatcher, buffer,
                                                              (struct sockaddr *) &goodAddress, sizeof(goodAddress),
                                                              ocs->serverAddr, ocs->serverAddrLength);

    parcEventQueue_Destroy(&buffer);
    assertTrue(success, "metisDispatcher_StreamBufferBindAndConnect did not succeed with good addresses");
}

LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET_Success)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet(ocs);

    uint16_t localPort = 9698;
    printf("local port = %u\n", localPort);

    // Connection "from" will use localPort as the local port number
    struct sockaddr_in goodLocalAddress;
    memset(&goodLocalAddress, 0, sizeof(goodLocalAddress));
    goodLocalAddress.sin_family = PF_INET;
    goodLocalAddress.sin_addr.s_addr = INADDR_ANY;
    goodLocalAddress.sin_port = htons(localPort);

    ocs->a = cpiAddress_CreateFromInet(&goodLocalAddress);

    // ocs->serverAddr will have "0.0.0.0" as the address, so need to create
    // something to 127.0.0.1
    struct sockaddr_in goodRemoteAddress;
    memset(&goodRemoteAddress, 0, sizeof(goodRemoteAddress));
    goodRemoteAddress.sin_family = PF_INET;
    goodRemoteAddress.sin_port = ((struct sockaddr_in *) ocs->serverAddr)->sin_port;
    inet_pton(AF_INET, "127.0.0.1", &(goodRemoteAddress.sin_addr));

    ocs->b = cpiAddress_CreateFromInet(&goodRemoteAddress);

    PARCEventQueue *result = metisDispatcher_StreamBufferConnect_INET(ocs->dispatcher, ocs->a, ocs->b);

    assertNotNull(result, "result buffer should be non-null for good local bind address 0.0.0.0 port %u", localPort) {
        int res;
        res = system("netstat -an -p tcp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }

    // turn the crank a few times, then accept and make sure the bind address is correct
    metisDispatcher_RunDuration(ocs->dispatcher, &((struct timeval) { 0, 1000 }));

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(struct sockaddr_in);
    int clientfd = accept(ocs->serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);
    assertFalse(clientfd < 0, "Error on accept: (%d) %s", errno, strerror(errno));

    assertTrue(clientAddr.sin_port == goodLocalAddress.sin_port,
               "Ports do not match, expecting %u got %u",
               htons(goodLocalAddress.sin_port),
               htons(clientAddr.sin_port));

    close(clientfd);
    metisDispatcher_RunCount(ocs->dispatcher, 1);
    metisStreamBuffer_Destroy(&result);
    metisDispatcher_RunCount(ocs->dispatcher, 1);
}

/**
 * Pass in a bad local address for the bind, will cause failure.
 * should receive NULL back from call
 */
LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET_Failure)
{
    // This test only works on OSX, as linux will accept the 0xFFFFFFF address as localhost
#if !defined(__APPLE__)
    testUnimplemented("Platform not supported");
#else
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet(ocs);

    struct sockaddr_in badAddress;
    memset(&badAddress, 0xFF, sizeof(badAddress));
    badAddress.sin_family = PF_INET;

    ocs->a = cpiAddress_CreateFromInet(&badAddress);
    ocs->b = cpiAddress_CreateFromInet((struct sockaddr_in *) ocs->serverAddr);

    // use the server address for our bind address.  will cause a failure.
    PARCEventQueue *result = metisDispatcher_StreamBufferConnect_INET(ocs->dispatcher, ocs->a, ocs->b);

    assertNull(result, "result buffer should be null for bad local address");
#endif
}

LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6_Success)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet6(ocs);

    struct sockaddr_in6 goodLocalAddress;
    memset(&goodLocalAddress, 0, sizeof(goodLocalAddress));
    goodLocalAddress.sin6_family = PF_INET6;
    goodLocalAddress.sin6_addr = in6addr_any;
    goodLocalAddress.sin6_port = INPORT_ANY;

    ocs->a = cpiAddress_CreateFromInet6(&goodLocalAddress);

    // ocs->serverAddr will have "0.0.0.0" as the address, so need to create
    // something to 127.0.0.1
    struct sockaddr_in6 goodRemoteAddress;
    memset(&goodRemoteAddress, 0, sizeof(goodRemoteAddress));
    goodRemoteAddress.sin6_family = PF_INET6;
    goodRemoteAddress.sin6_port = ((struct sockaddr_in6 *) ocs->serverAddr)->sin6_port;
    inet_pton(AF_INET6, "::1", &(goodRemoteAddress.sin6_addr));

    ocs->b = cpiAddress_CreateFromInet6(&goodRemoteAddress);

    PARCEventQueue *result = metisDispatcher_StreamBufferConnect_INET6(ocs->dispatcher, ocs->a, ocs->b);

    assertNotNull(result, "result buffer should be non-null for good local address");


    metisStreamBuffer_Destroy(&result);
}

/**
 * Pass in a bad local address for the bind, will cause failure.
 * should receive NULL back from call
 */
LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6_Failure)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet6(ocs);

    struct sockaddr_in6 badAddress;
    memset(&badAddress, 0xFF, sizeof(badAddress));
    badAddress.sin6_family = PF_INET6;

    ocs->a = cpiAddress_CreateFromInet6(&badAddress);
    ocs->b = cpiAddress_CreateFromInet6((struct sockaddr_in6 *) ocs->serverAddr);

    // use the server address for our bind address.  will cause a failure.
    PARCEventQueue *result = metisDispatcher_StreamBufferConnect_INET6(ocs->dispatcher, ocs->a, ocs->b);

    assertNull(result, "result buffer should be null for bad local address");
}

LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet(ocs);

    struct sockaddr_in goodLocalAddress;
    memset(&goodLocalAddress, 0, sizeof(goodLocalAddress));
    goodLocalAddress.sin_family = PF_INET;
    goodLocalAddress.sin_addr.s_addr = INADDR_ANY;
    goodLocalAddress.sin_port = INPORT_ANY;

    ocs->a = cpiAddress_CreateFromInet(&goodLocalAddress);

    // ocs->serverAddr will have "0.0.0.0" as the address, so need to create
    // something to 127.0.0.1
    struct sockaddr_in goodRemoteAddress;
    memset(&goodRemoteAddress, 0, sizeof(goodRemoteAddress));
    goodRemoteAddress.sin_family = PF_INET;
    goodRemoteAddress.sin_port = ((struct sockaddr_in *) ocs->serverAddr)->sin_port;
    inet_pton(AF_INET, "127.0.0.1", &(goodRemoteAddress.sin_addr));

    ocs->b = cpiAddress_CreateFromInet(&goodRemoteAddress);

    MetisAddressPair *pair = metisAddressPair_Create(ocs->a, ocs->b);
    PARCEventQueue *result = metisDispatcher_StreamBufferConnect(ocs->dispatcher, pair);
    metisAddressPair_Release(&pair);
    assertNotNull(result, "result buffer should be non-null for good local address");
    metisStreamBuffer_Destroy(&result);
}

LONGBOW_TEST_CASE(StreamBufferConnect, metisDispatcher_StreamBufferConnect_INET6)
{
    OpenConnectionState *ocs = longBowTestCase_GetClipBoardData(testCase);
    listenToInet6(ocs);

    struct sockaddr_in6 goodLocalAddress;
    memset(&goodLocalAddress, 0, sizeof(goodLocalAddress));
    goodLocalAddress.sin6_family = PF_INET6;
    goodLocalAddress.sin6_addr = in6addr_any;
    goodLocalAddress.sin6_port = INPORT_ANY;

    ocs->a = cpiAddress_CreateFromInet6(&goodLocalAddress);

    // ocs->serverAddr will have "0.0.0.0" as the address, so need to create
    // something to 127.0.0.1
    struct sockaddr_in6 goodRemoteAddress;
    memset(&goodRemoteAddress, 0, sizeof(goodRemoteAddress));
    goodRemoteAddress.sin6_family = PF_INET6;
    goodRemoteAddress.sin6_port = ((struct sockaddr_in6 *) ocs->serverAddr)->sin6_port;
    inet_pton(AF_INET6, "::1", &(goodRemoteAddress.sin6_addr));

    ocs->b = cpiAddress_CreateFromInet6(&goodRemoteAddress);

    MetisAddressPair *pair = metisAddressPair_Create(ocs->a, ocs->b);
    PARCEventQueue *result = metisDispatcher_StreamBufferConnect(ocs->dispatcher, pair);
    metisAddressPair_Release(&pair);

    assertNotNull(result, "result buffer should be non-null for good local address");

    metisStreamBuffer_Destroy(&result);
}

// =================================================================================

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    longBowTestCase_SetClipBoardData(testCase, dispatcher);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    MetisDispatcher *dispatcher = longBowTestCase_GetClipBoardData(testCase);
    metisDispatcher_Destroy(&dispatcher);

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Dispatcher);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
