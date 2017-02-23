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
#include "../metis_ControlState.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_Control)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Control)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Control)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================================

static CCNxMetaMessage *_testWriteMessage = NULL;
static CCNxMetaMessage *_testReadMessage = NULL;

/**
 * For testing purposes writes a message to a local buffer and reads response from local buffer
 *
 * _testWriteMessage will be an allocated reference to what is written
 * _testReadMessage will be sent back (returend).  You must put an allocated message there
 * before calling this test function.
 */
static CCNxMetaMessage *
_testWriteRead(void *userdata, CCNxMetaMessage *msg)
{
    _testWriteMessage = ccnxMetaMessage_Acquire(msg);
    return ccnxMetaMessage_Acquire(_testReadMessage);
}

static unsigned _testCommandExecuteCount = 0;

static MetisCommandReturn
_testCommand(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    _testCommandExecuteCount++;
    return MetisCommandReturn_Success;
}

static MetisCommandOps _testCommandOps = {
    .command = "test", // empty string for root
    .init    = NULL,
    .execute = _testCommand
};

// ==================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_DispatchCommand);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_GetDebug);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_Interactive);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_RegisterCommand);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_SetDebug);
    LONGBOW_RUN_TEST_CASE(Global, metisControlState_WriteRead);
    LONGBOW_RUN_TEST_CASE(Global, _metisControlState_ParseStringIntoTokens);
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

LONGBOW_TEST_CASE(Global, metisControlState_Create)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);
    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, metisControlState_DispatchCommand)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);

    metisControlState_RegisterCommand(state, &_testCommandOps);

    const char *argv[] = { "test", "foobar" };
    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, 2, (void **) &argv[0]);

    _testCommandExecuteCount = 0;

    metisControlState_DispatchCommand(state, args);

    assertTrue(_testCommandExecuteCount == 1, "Incorrect execution count, expected 1 got %u", _testCommandExecuteCount);
    parcList_Release(&args);
    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, metisControlState_GetDebug)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);

    bool test = metisControlState_GetDebug(state);
    assertTrue(test == state->debugFlag, "debug flag in unexpected state");

    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, metisControlState_Interactive)
{
    // this reads commands from stdin.  not sure how to test this.
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisControlState_RegisterCommand)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);

    metisControlState_RegisterCommand(state, &_testCommandOps);

    bool match = metisCommandParser_ContainsCommand(state->parser, "test");
    assertTrue(match, "Command not found in parser");

    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, metisControlState_SetDebug)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);

    assertFalse(state->debugFlag, "debug flag in unexpected true state");
    metisControlState_SetDebug(state, true);
    assertTrue(state->debugFlag, "debug flag in unexpected false state");

    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, metisControlState_WriteRead)
{
    char hello[] = "hello";
    MetisControlState *state = metisControlState_Create(hello, _testWriteRead);

    CCNxName *appleName = ccnxName_CreateFromCString("lci:/apple");
    CCNxInterest *appleInterest = ccnxInterest_CreateSimple(appleName);
    _testReadMessage = ccnxMetaMessage_CreateFromInterest(appleInterest);
    ccnxInterest_Release(&appleInterest);
    ccnxName_Release(&appleName);

    CCNxName *pieName = ccnxName_CreateFromCString("lci:/pie");
    CCNxInterest *pieInterest = ccnxInterest_CreateSimple(pieName);
    CCNxMetaMessage *writeMessage = ccnxMetaMessage_CreateFromInterest(pieInterest);;
    ccnxInterest_Release(&pieInterest);
    ccnxName_Release(&pieName);

    CCNxMetaMessage *test = metisControlState_WriteRead(state, writeMessage);

    assertTrue(_testWriteMessage == writeMessage, "write message incorrect, expected %p got %p", (void *) writeMessage, (void *) _testWriteMessage);
    assertTrue(_testReadMessage == test, "read message incorrect, expected %p got %p", (void *) _testReadMessage, (void *) test);

    ccnxMetaMessage_Release(&test);
    ccnxMetaMessage_Release(&writeMessage);

    ccnxMetaMessage_Release(&_testReadMessage);
    ccnxMetaMessage_Release(&_testWriteMessage);

    metisControlState_Destroy(&state);
}

LONGBOW_TEST_CASE(Global, _metisControlState_ParseStringIntoTokens)
{
    const char *string = "the quick brown fox";

    const char *argv[] = { "the", "quick", "brown", "fox" };
    PARCList *truth = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(truth, 4, (void **) &argv[0]);

    PARCList *test = _metisControlState_ParseStringIntoTokens(string);

    assertTrue(parcList_Size(test) == parcList_Size(truth), "list wrong size, expected %zu got %zu", parcList_Size(truth), parcList_Size(test));

    for (int i = 0; i < parcList_Size(truth); i++) {
        const char *testString = parcList_GetAtIndex(test, i);
        const char *truthString = parcList_GetAtIndex(truth, i);
        assertTrue(strcmp(testString, truthString) == 0, "index %d not equal, expected '%s' got '%s'", i, truthString, testString);
    }

    parcList_Release(&test);
    parcList_Release(&truth);
}

// ========================================================================

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Control);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
