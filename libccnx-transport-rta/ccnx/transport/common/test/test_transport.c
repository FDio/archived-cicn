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
// This permits internal static functions to be visible to this Test Runner.
#include "../transport.c"

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_TestingMemory.h>

LONGBOW_TEST_RUNNER(transport)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(transport)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(transport)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, Transport_Create_RTA);
    LONGBOW_RUN_TEST_CASE(Global, Transport_Close);
    LONGBOW_RUN_TEST_CASE(Global, Transport_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, Transport_Open);
    LONGBOW_RUN_TEST_CASE(Global, Transport_PassCommand);
    LONGBOW_RUN_TEST_CASE(Global, Transport_Recv);
    LONGBOW_RUN_TEST_CASE(Global, Transport_Send);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;
    if (parcTestingMemory_ExpectedOutstanding(0, "%s", longBowTestCase_GetFullName(testCase)) == false) {
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_CASE(Global, Transport_Close)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, Transport_Create_RTA)
{
    TransportContext *transport = Transport_Create(TRANSPORT_RTA);

    Transport_Destroy(&transport);
}

LONGBOW_TEST_CASE(Global, Transport_Destroy)
{
    TransportContext *transport = Transport_Create(TRANSPORT_RTA);

    Transport_Destroy(&transport);
}

LONGBOW_TEST_CASE(Global, Transport_Open)
{
    TransportContext *transport = Transport_Create(TRANSPORT_RTA);


    Transport_Destroy(&transport);
}

LONGBOW_TEST_CASE(Global, Transport_PassCommand)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, Transport_Recv)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, Transport_Send)
{
    testUnimplemented("");
}

LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(transport);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
