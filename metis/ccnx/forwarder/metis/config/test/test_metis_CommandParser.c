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
#include "../metis_CommandParser.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metis_CommandParser)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_CommandParser)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_CommandParser)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_Create_Destroy);

    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_DispatchCommand_Exact);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_DispatchCommand_Longer);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_DispatchCommand_Shorter);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_DispatchCommand_Sibling);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_GetDebug);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_Interactive);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_RegisterCommand_NullInit);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_RegisterCommand_WithInit);
    LONGBOW_RUN_TEST_CASE(Global, metisCommandParser_SetDebug);
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

LONGBOW_TEST_CASE(Global, metisCommandParser_Create_Destroy)
{
    MetisCommandParser *parser = metisCommandParser_Create();
    assertNotNull(parser, "Got null parser from metisCommandParser_Create");
    metisCommandParser_Destroy(&parser);
    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance!");
    assertNull(parser, "metisCommandParser_Destroy did not null pointer");
}

static MetisCommandReturn
test_execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    bool *execute_called_ptr = (bool *) ops->closure;
    *execute_called_ptr = true;
    return MetisCommandReturn_Success;
}

/**
 * argc = the exact number of args, don't include the command name
 * example: argc = 2, argv = {"Hello", "World"}
 *
 * expectedResult true means the execute function is called
 */
static void
dispatchCommand(const char *command_string, int argc, char **argv, bool expectedResult)
{
    MetisCommandParser *parser = metisCommandParser_Create();

    bool execute_called = false;

    MetisCommandOps *ops = metisCommandOps_Create(&execute_called, command_string, NULL, test_execute, metisCommandOps_Destroy);

    PARCList *args = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);
    parcList_AddAll(args, argc, (void **) &argv[0]);

    execute_called = false;
    metisCommandParser_RegisterCommand(parser, ops);
    metisCommandParser_DispatchCommand(parser, args);
    if (expectedResult) {
        assertTrue(execute_called, "Did not call the execute function");
    } else {
        assertFalse(execute_called, "The execute function should not have been called but was");
    }

    metisCommandParser_Destroy(&parser);
    parcList_Release(&args);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_DispatchCommand_Exact)
{
    // note that it is not case sensitive
    dispatchCommand("hello world", 2, (char *[]) { "Hello", "World" }, true);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_DispatchCommand_Sibling)
{
    // note that it is not case sensitive
    dispatchCommand("hello world", 2, (char *[]) { "Hello", "Universe" }, false);
}


LONGBOW_TEST_CASE(Global, metisCommandParser_DispatchCommand_Longer)
{
    // note that it is not case sensitive
    dispatchCommand("hello world", 3, (char *[]) { "Hello", "World", "Again" }, true);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_DispatchCommand_Shorter)
{
    // note that it is not case sensitive
    dispatchCommand("hello world", 1, (char *[]) { "Hello" }, false);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_GetDebug)
{
    MetisCommandParser *parser = metisCommandParser_Create();
    bool test = metisCommandParser_GetDebug(parser);
    assertTrue(test == parser->debugFlag, "Got %d expected %d", test, parser->debugFlag);
    metisCommandParser_Destroy(&parser);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_Interactive)
{
    testUnimplemented("");
}

static bool called_init = false;
static void
test_init_command(MetisCommandParser *parser, MetisCommandOps *ops)
{
    called_init = true;
}

LONGBOW_TEST_CASE(Global, metisCommandParser_RegisterCommand_WithInit)
{
    MetisCommandParser *parser = metisCommandParser_Create();

    MetisCommandOps *ops = metisCommandOps_Create(NULL, "hello world", test_init_command, test_execute, metisCommandOps_Destroy);

    called_init = false;
    metisCommandParser_RegisterCommand(parser, ops);

    MetisCommandOps *test = parcTreeRedBlack_Get(parser->commandTree, ops->command);
    assertNotNull(test, "Got null looking up command in tree");
    assertTrue(test == ops, "Wrong pointer, got %p expected %p", (void *) test, (void *) ops);
    assertTrue(called_init, "Did not call the init function");

    metisCommandParser_Destroy(&parser);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_RegisterCommand_NullInit)
{
    MetisCommandParser *parser = metisCommandParser_Create();

    MetisCommandOps command = {
        .command = "hello world",
        .init    = NULL,
        .execute = NULL
    };

    called_init = false;
    metisCommandParser_RegisterCommand(parser, &command);

    MetisCommandOps *test = parcTreeRedBlack_Get(parser->commandTree, command.command);
    assertNotNull(test, "Got null looking up command in tree");
    assertTrue(test == &command, "Wrong pointer, got %p expected %p", (void *) test, (void *) &command);
    assertFalse(called_init, "Somehow called the init function");

    metisCommandParser_Destroy(&parser);
}

LONGBOW_TEST_CASE(Global, metisCommandParser_SetDebug)
{
    MetisCommandParser *parser = metisCommandParser_Create();
    // flip the setting
    bool truth = ~parser->debugFlag;
    metisCommandParser_SetDebug(parser, truth);
    assertTrue(truth == parser->debugFlag, "Got %d expected %d", parser->debugFlag, truth);
    metisCommandParser_Destroy(&parser);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisCommandParser_MatchCommand);
    LONGBOW_RUN_TEST_CASE(Local, parseStringIntoTokens);
    LONGBOW_RUN_TEST_CASE(Local, stringCompare);
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

LONGBOW_TEST_CASE(Local, metisCommandParser_MatchCommand)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, parseStringIntoTokens)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, stringCompare)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_CommandParser);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
