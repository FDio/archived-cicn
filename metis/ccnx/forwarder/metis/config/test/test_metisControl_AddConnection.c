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
#include "../metisControl_AddConnection.c"
#include "testrig_MetisControl.c"

LONGBOW_TEST_RUNNER(metisControl_AddConnection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_AddConnection)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_AddConnection)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddConnection_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddConnection_HelpCreate);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    testrigMetisControl_commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    testrigMetisControl_CommonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisControlAddConnection_Create)
{
    testCommandCreate(testCase, &metisControlAddConnection_Create, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlAddConnection_HelpCreate)
{
    testCommandCreate(testCase, &metisControlAddConnection_HelpCreate, __func__);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_EtherCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_EtherExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_McastCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_McastExecute);

    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_TcpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_TcpExecute);

    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_UdpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_UdpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_Execute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_Init);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_ConvertStringsToCpiAddress);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_CreateTunnel);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_EtherHelpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_EtherHelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_McastHelpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_McastHelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_TcpHelpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_TcpHelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_UdpHelpCreate);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_UdpHelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_HelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddConnection_IpHelp);

    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_TooFewArgs);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_TooManyArgs);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_BadRemoteIp);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_GoodRemoteIp);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_WithLocalIp);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_WithLocalIpAndPort);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_BadLocalIp);
    LONGBOW_RUN_TEST_CASE(Local, metisControl_ParseIPCommandLine_MismatchLocalAndRemote);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    testrigMetisControl_commonSetup(testCase);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    testrigMetisControl_CommonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_EtherCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_EtherCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_EtherExecute)
{
    const char *argv[] = { "add", "connection", "ether", "conn3", "e8-06-88-cd-28-de", "em3" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);


    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = _metisControlAddConnection_EtherCreate(data->state);
    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);
    assertTrue(result == MetisCommandReturn_Success, "Valid command line should succeed");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_McastCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_McastCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_McastExecute)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = _metisControlAddConnection_McastCreate(data->state);
    MetisCommandReturn result = ops->execute(data->state->parser, ops, NULL);
    metisCommandOps_Destroy(&ops);
    assertTrue(result == MetisCommandReturn_Failure, "Unimplemented execute should have failed");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_TcpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_TcpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_TcpExecute)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "add", "connection", "tcp", "conn3", "1.2.3.4", "123" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);
    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);

    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Success, "Unimplemented execute should have failed");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_UdpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_UdpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_UdpExecute)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "add", "connection", "tcp", "conn3", "1.2.3.4", "123" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_UdpCreate(data->state);
    MetisCommandReturn result = ops->execute(data->state->parser, ops, args);

    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Success, "Unimplemented execute should have failed");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_Execute)
{
    // this just prints a Help message
    testHelpExecute(testCase, metisControlAddConnection_Create, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_Init)
{
    testInit(testCase, metisControlAddConnection_Create, __func__,
             (const char *[]) {
        _commandAddConnectionTcp, _commandAddConnectionUdp, _commandAddConnectionEther, _commandAddConnectionMcast,
        _commandAddConnectionTcpHelp, _commandAddConnectionUdpHelp, _commandAddConnectionEtherHelp, _commandAddConnectionMcastHelp,
        NULL
    });
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_ConvertStringsToCpiAddress)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_CreateTunnel)
{
    // this is actully testred in the Tcp_Execute and Udp_Execute
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_EtherHelpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_EtherHelpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_EtherHelpExecute)
{
    testHelpExecute(testCase, _metisControlAddConnection_EtherHelpCreate, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_McastHelpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_McastHelpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_McastHelpExecute)
{
    testHelpExecute(testCase, _metisControlAddConnection_McastHelpCreate, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_TcpHelpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_TcpHelpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_TcpHelpExecute)
{
    testHelpExecute(testCase, _metisControlAddConnection_TcpHelpCreate, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_UdpHelpCreate)
{
    testCommandCreate(testCase, &_metisControlAddConnection_UdpHelpCreate, __func__);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_UdpHelpExecute)
{
    testHelpExecute(testCase, _metisControlAddConnection_UdpHelpCreate, __func__, MetisCommandReturn_Success);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_HelpExecute)
{
    testHelpExecute(testCase, metisControlAddConnection_HelpCreate, __func__, MetisCommandReturn_Success);
}

/**
 * Expectes 5 to 7 options
 */
LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_TooFewArgs)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "a", "b", "c" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 3, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Failure, "ParseIPCommandLine with 3 args should have returned %d, got %d", MetisCommandReturn_Failure, result);
}

/**
 * Expects 5 to 7 options
 */
LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_TooManyArgs)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 9, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Failure, "ParseIPCommandLine with 3 args should have returned %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_BadRemoteIp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "a", "b", "c", "tun0", "555.555.555.555", "123", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Failure, "ParseIPCommandLine with invalid IP address should have returned %d, got %d", MetisCommandReturn_Failure, result);
}

/**
 * Pass a set of args to metisControl_ParseIPCommandLine, then verify:
 * Successful
 * remote_ip is what we gave it
 * remote_port is what we gave it
 * local_ip is 0.0.0.0 or what we gave it
 * local_pot is 0 or what we gave it.
 */
static void
verifyParseIpWithGoodAddress(TestData *data, int argc, const char *remote_ip, const char *remote_port, const char *local_ip, const char *local_port)
{
    const char *argv[] = { "a", "b", "c", "tun0", remote_ip, remote_port, local_ip, local_port };

    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Success, "ParseIPCommandLine with invalid IP address should have returned %d, got %d", MetisCommandReturn_Failure, result);

    struct sockaddr *sockaddr_remote = parcNetwork_SockAddress(remote_ip, atoi(remote_port));
    struct sockaddr *sockaddr_local = parcNetwork_SockAddress(local_ip, atoi(local_port));
    CPIAddress *truth_remote = cpiAddress_CreateFromInet((struct sockaddr_in *) sockaddr_remote);
    CPIAddress *truth_local = cpiAddress_CreateFromInet((struct sockaddr_in *) sockaddr_local);
    parcMemory_Deallocate((void **) &sockaddr_local);
    parcMemory_Deallocate((void **) &sockaddr_remote);

    assertTrue(cpiAddress_Equals(truth_remote, remote), "Got wrong remote address");
    assertTrue(cpiAddress_Equals(truth_local, local), "Got wrong local address");
    cpiAddress_Destroy(&truth_remote);
    cpiAddress_Destroy(&truth_local);
    cpiAddress_Destroy(&remote);
    cpiAddress_Destroy(&local);
}

LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_GoodRemoteIp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    verifyParseIpWithGoodAddress(data, 6, "1.2.3.4", "123", "0.0.0.0", "0");
}

LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_WithLocalIp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    verifyParseIpWithGoodAddress(data, 7, "1.2.3.4", "123", "10.11.12.13", "0");
}

LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_WithLocalIpAndPort)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    verifyParseIpWithGoodAddress(data, 8, "1.2.3.4", "123", "10.11.12.13", "456");
}

LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_BadLocalIp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "a", "b", "c", "tun0", "1.2.3.4", "123", "666.666.666.666", "123", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 8, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Failure, "ParseIPCommandLine with invalid local IP address should have returned %d, got %d", MetisCommandReturn_Failure, result);
}

/**
 * One's an IPv4 and one's an IPv6.
 */
LONGBOW_TEST_CASE(Local, metisControl_ParseIPCommandLine_MismatchLocalAndRemote)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    const char *argv[] = { "a", "b", "c", "tun0", "1.2.3.4", "123", "2001:720:1500:1::a100", "123", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 8, (void **) &argv[0]);

    MetisCommandOps *ops = _metisControlAddConnection_TcpCreate(data->state);

    CPIAddress *remote;
    CPIAddress *local;
    char *symbolic = NULL;

    MetisCommandReturn result = _metisControlAddConnection_ParseIPCommandLine(data->state->parser, ops, args, &remote, &local, &symbolic);
    metisCommandOps_Destroy(&ops);
    parcList_Release(&args);

    assertTrue(result == MetisCommandReturn_Failure, "ParseIPCommandLine with invalid local IP address should have returned %d, got %d", MetisCommandReturn_Failure, result);
}

LONGBOW_TEST_CASE(Local, _metisControlAddConnection_IpHelp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = _metisControlAddConnection_McastHelpCreate(data->state);
    MetisCommandReturn result = _metisControlAddConnection_IpHelp(NULL, ops, NULL, "WIZARD");
    assertTrue(result == MetisCommandReturn_Success, "Wrong return, got %d expected %d", result, MetisCommandReturn_Success);
    metisCommandOps_Destroy(&ops);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_AddConnection);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
