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
#include "../metisControl_AddListener.c"
#include "testrig_MetisControl.c"

LONGBOW_TEST_RUNNER(metisControl_AddListener)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metisControl_AddListener)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metisControl_AddListener)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddListener_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisControlAddListener_HelpCreate);
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

LONGBOW_TEST_CASE(Global, metisControlAddListener_Create)
{
    testCommandCreate(testCase, &metisControlAddListener_Create, __func__);
}

LONGBOW_TEST_CASE(Global, metisControlAddListener_HelpCreate)
{
    testCommandCreate(testCase, &metisControlAddListener_HelpCreate, __func__);
}

// ===========================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_WrongArgCount);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_Tcp);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_Udp);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_Udp6);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_Ether);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_UnknownProtocol);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_BadSymbolic);
    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_Execute_BadSymbolic_NotAlphaNum);

    LONGBOW_RUN_TEST_CASE(Local, _metisControlAddListener_HelpExecute);
    LONGBOW_RUN_TEST_CASE(Local, _createTcpListener);
    LONGBOW_RUN_TEST_CASE(Local, _createUdpListener);
    LONGBOW_RUN_TEST_CASE(Local, _createEtherListener);
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

LONGBOW_TEST_CASE(Local, _createTcpListener)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "tcp", "public0", "13.14.15.16", "9596", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _createTcpListener(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _createUdpListener)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "udp", "public0", "13.14.15.16", "9596", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _createUdpListener(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _createEtherListener)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "ether", "nic3", "eth3", "0x0801", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _createEtherListener(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_WrongArgCount)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "ether" "nic3", "eth3", "0x0801", "foobar" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 7, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Failure, "Command did not return failure: %d", test);
    assertTrue(data->writeread_count == 0, "Wrong write/read count, expected %d got %u", 0, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_Tcp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);
    metisControlState_SetDebug(data->state, true);

    const char *argv[] = { "add", "listener", "tcp", "public0", "13.14.15.16", "9596", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_Udp)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);
    metisControlState_SetDebug(data->state, true);

    const char *argv[] = { "add", "listener", "udp", "public0", "13.14.15.16", "9596", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_Udp6)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);
    metisControlState_SetDebug(data->state, true);

    // INET6 address
    const char *argv[] = { "add", "listener", "udp", "public0", "::1", "9596", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_Ether)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "ether", "nic3", "eth3", "0x0801", };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Success, "Command did not return success: %d", test);
    assertTrue(data->writeread_count == 1, "Wrong write/read count, expected %d got %u", 1, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_UnknownProtocol)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "pup", "nic3", "eth3", "0x0801" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Failure, "Command did not return failure: %d", test);
    assertTrue(data->writeread_count == 0, "Wrong write/read count, expected %d got %u", 0, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_BadSymbolic)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "ether" "111", "eth3", "0x0801" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Failure, "Command did not return failure: %d", test);
    assertTrue(data->writeread_count == 0, "Wrong write/read count, expected %d got %u", 0, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}


LONGBOW_TEST_CASE(Local, _metisControlAddListener_Execute_BadSymbolic_NotAlphaNum)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = metisControlAddListener_Create(data->state);

    const char *argv[] = { "add", "listener", "ether", "n()t", "eth3", "0x0801" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 6, (void **) &argv[0]);

    MetisCommandReturn test = _metisControlAddListener_Execute(data->state->parser, ops, args);

    assertTrue(test == MetisCommandReturn_Failure, "Command did not return failure: %d", test);
    assertTrue(data->writeread_count == 0, "Wrong write/read count, expected %d got %u", 0, data->writeread_count);

    parcList_Release(&args);
    ops->destroyer(&ops);
}

LONGBOW_TEST_CASE(Local, _metisControlAddListener_HelpExecute)
{
    _metisControlAddListener_HelpExecute(NULL, NULL, NULL);
}

// ===========================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metisControl_AddListener);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
