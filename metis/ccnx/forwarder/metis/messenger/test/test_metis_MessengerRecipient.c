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
#include "../metis_MessengerRecipient.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_MessengerRecipient)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_MessengerRecipient)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_MessengerRecipient)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisMessengerRecipient_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisMessengerRecipient_Deliver);
    LONGBOW_RUN_TEST_CASE(Global, metisMessengerRecipient_GetRecipientContext);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

typedef struct my_context {
    MetisMissive *lastMessage;
} MyContext;

static void
testRecipientCallback(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    MyContext *mycontext = metisMessengerRecipient_GetRecipientContext(recipient);
    mycontext->lastMessage = missive;
}

LONGBOW_TEST_CASE(Global, metisMessengerRecipient_Create)
{
    MyContext mycontext;

    // create and destroy and make sure no memory leaks
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&mycontext, testRecipientCallback);
    metisMessengerRecipient_Destroy(&recipient);
    size_t balance = parcMemory_Outstanding();
    assertTrue(balance == 0, "Memory imbalance, expected 0, got %zu", balance);
}

LONGBOW_TEST_CASE(Global, metisMessengerRecipient_Deliver)
{
    MyContext mycontext;
    MetisMissive *truthMissive = metisMissive_Create(MetisMissiveType_ConnectionUp, 33);

    // create and destroy and make sure no memory leaks
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&mycontext, testRecipientCallback);

    metisMessengerRecipient_Deliver(recipient, truthMissive);

    assertTrue(mycontext.lastMessage == truthMissive, "Recipient callback not right missve, expected %p got %p", (void *) truthMissive, (void *) mycontext.lastMessage);

    metisMessengerRecipient_Destroy(&recipient);

    metisMissive_Release(&truthMissive);
}

LONGBOW_TEST_CASE(Global, metisMessengerRecipient_GetRecipientContext)
{
    MyContext mycontext;

    // create and destroy and make sure no memory leaks
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&mycontext, testRecipientCallback);

    void *testcontext = metisMessengerRecipient_GetRecipientContext(recipient);
    assertTrue(testcontext == &mycontext, "Got wrong context back, expected %p got %p", (void *) &mycontext, (void *) testcontext);
    metisMessengerRecipient_Destroy(&recipient);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_MessengerRecipient);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
