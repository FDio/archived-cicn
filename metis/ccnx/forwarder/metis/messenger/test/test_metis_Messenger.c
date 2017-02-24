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
#include "../metis_Messenger.c"
#include <inttypes.h>
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

LONGBOW_TEST_RUNNER(metis_Messenger)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Messenger)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Messenger)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisMessenger_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisMessenger_Register);
    LONGBOW_RUN_TEST_CASE(Global, metisMessenger_Register_Twice);
    LONGBOW_RUN_TEST_CASE(Global, metisMessenger_Unregister);
    LONGBOW_RUN_TEST_CASE(Global, metisMessenger_Send);
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

LONGBOW_TEST_CASE(Global, metisMessenger_Create_Destroy)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);

    MetisMessenger *messenger = metisMessenger_Create(dispatcher);
    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Create/Destroy has memory leak");
}

// The callback will compare what its called back with these "truth" values
static const MetisMissive *truth_missive;
static const MetisMessengerRecipient *truth_recipient;

static void
test_notify(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    assertTrue(recipient == truth_recipient, "Got wrong recipient in callback expected %" PRIXPTR " got %" PRIXPTR, (uintptr_t) truth_recipient, (uintptr_t) recipient);
    assertTrue(missive == truth_missive, "Got wrong event in callback expected %p got %p", (void *) truth_missive, (void *) missive);
    metisMissive_Release(&missive);
}

LONGBOW_TEST_CASE(Global, metisMessenger_Register)
{
    int a = 1;
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&a, &test_notify);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    MetisMessenger *messenger = metisMessenger_Create(dispatcher);

    metisMessenger_Register(messenger, recipient);
    assertTrue(parcArrayList_Size(messenger->callbacklist) == 1,
               "messenger array list wrong size, expected %u got %zu",
               1,
               parcArrayList_Size(messenger->callbacklist));

    const void *p = parcArrayList_Get(messenger->callbacklist, 0);
    assertTrue(p == recipient,
               "Messenger callbacklist contained wrong pointer, expected %p got %p",
               (void *) recipient,
               p);

    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);
    metisMessengerRecipient_Destroy(&recipient);
}

/**
 * Register same callback twice, should only appear once in list
 */
LONGBOW_TEST_CASE(Global, metisMessenger_Register_Twice)
{
    int a = 1;
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&a, &test_notify);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    MetisMessenger *messenger = metisMessenger_Create(dispatcher);

    metisMessenger_Register(messenger, recipient);
    metisMessenger_Register(messenger, recipient);
    assertTrue(parcArrayList_Size(messenger->callbacklist) == 1,
               "messenger array list wrong size, expected %u got %zu",
               1,
               parcArrayList_Size(messenger->callbacklist));

    const void *p = parcArrayList_Get(messenger->callbacklist, 0);
    assertTrue(p == recipient,
               "Messenger callbacklist contained wrong pointer, expected %p got %p",
               (void *) recipient,
               p);

    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);
    metisMessengerRecipient_Destroy(&recipient);
}

LONGBOW_TEST_CASE(Global, metisMessenger_Unregister)
{
    int a = 1;
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&a, &test_notify);

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    MetisMessenger *messenger = metisMessenger_Create(dispatcher);

    metisMessenger_Register(messenger, recipient);
    metisMessenger_Unregister(messenger, recipient);

    assertTrue(parcArrayList_Size(messenger->callbacklist) == 0,
               "messenger array list wrong size, expected %u got %zu",
               0,
               parcArrayList_Size(messenger->callbacklist));

    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);
    metisMessengerRecipient_Destroy(&recipient);
}

LONGBOW_TEST_CASE(Global, metisMessenger_Send)
{
    int a = 1;
    MetisMessengerRecipient *recipient = metisMessengerRecipient_Create(&a, &test_notify);

    truth_recipient = recipient;

    MetisMissive *missive = metisMissive_Create(MetisMissiveType_ConnectionUp, 12);
    truth_missive = missive;

    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    MetisMessenger *messenger = metisMessenger_Create(dispatcher);

    metisMessenger_Send(messenger, missive);

    metisDispatcher_RunDuration(dispatcher, &((struct timeval) {0, 10000}));

    // if we didn't assert, it is correct.

    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);
    metisMessengerRecipient_Destroy(&recipient);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, removeCallback);
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

LONGBOW_TEST_CASE(Local, removeCallback)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    MetisDispatcher *dispatcher = metisDispatcher_Create(logger);
    metisLogger_Release(&logger);
    MetisMessenger *messenger = metisMessenger_Create(dispatcher);

    parcArrayList_Add(messenger->callbacklist, (void *) 1);
    parcArrayList_Add(messenger->callbacklist, (void *) 2);
    parcArrayList_Add(messenger->callbacklist, (void *) 3);

    removeRecipient(messenger, (void *) 2);

    assertTrue(parcArrayList_Size(messenger->callbacklist) == 2,
               "messenger array list wrong size, expected %u got %zu",
               2,
               parcArrayList_Size(messenger->callbacklist));

    const void *p = parcArrayList_Get(messenger->callbacklist, 0);
    assertTrue(p == (void *) 1,
               "Messenger callbacklist contained wrong pointer at 0, expected %p got %p",
               (void *) 1,
               p);

    p = parcArrayList_Get(messenger->callbacklist, 1);
    assertTrue(p == (void *) 3,
               "Messenger callbacklist contained wrong pointer at 1, expected %p got %p",
               (void *) 3,
               p);


    metisMessenger_Destroy(&messenger);
    metisDispatcher_Destroy(&dispatcher);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Messenger);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
