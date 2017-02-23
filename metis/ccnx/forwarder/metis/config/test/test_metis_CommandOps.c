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
#include "../metis_CommandOps.c"

#include <inttypes.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_CommandOps)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_CommandOps)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_CommandOps)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisCommandOps_Create);
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

static void
_init(struct metis_command_parser *parser, MetisCommandOps *ops)
{
}

static MetisCommandReturn
_execute(struct metis_command_parser *parser, MetisCommandOps *ops, PARCList *args)
{
    return MetisCommandReturn_Success;
}

static void
_destroyer(MetisCommandOps **opsPtr)
{
}

LONGBOW_TEST_CASE(Global, metisCommandOps_Create)
{
    char hello[] = "hello";
    char command[] = "test";

    MetisCommandOps *ops = metisCommandOps_Create(hello, command, _init, _execute, _destroyer);

    assertTrue(ops->closure == hello, "closure wrong expected %p got %p", (void *) hello, (void *) ops->closure);
    assertTrue(strcmp(ops->command, command) == 0, "command wrong expected '%s' got '%s'", command, ops->command);
    assertTrue(ops->init == _init, "Wrong init, expected %" PRIXPTR " got %" PRIXPTR, (uintptr_t) _init, (uintptr_t) ops->init);
    assertTrue(ops->execute == _execute, "Wrong execute, expected %" PRIXPTR " got %" PRIXPTR, (uintptr_t) _execute, (uintptr_t) ops->execute);
    assertTrue(ops->destroyer == _destroyer, "Wrong destroyer, expected %" PRIXPTR " got %" PRIXPTR, (uintptr_t) _destroyer, (uintptr_t) ops->destroyer);

    metisCommandOps_Destroy(&ops);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_CommandOps);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
