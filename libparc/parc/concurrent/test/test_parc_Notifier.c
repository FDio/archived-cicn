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

/**
 */

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Notifier.c"

#include <sys/time.h>
#include <pthread.h>
#include <poll.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(parc_Notifier)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Notifier)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Notifier)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_Create_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_PauseEvent_NotPaused);
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_PauseEvent_AlreadyPaused);
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_StartEvents);

    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_Notify_First);
    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_Notify_Twice);

    LONGBOW_RUN_TEST_CASE(Global, parcNotifier_ThreadedTest);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ------
typedef struct test_data {
    volatile int barrier;
    PARCNotifier *notifier;

    unsigned notificationsToSend;
    unsigned notificationsSent;

    unsigned notificationsToRecieve;
    unsigned notificationsReceived;

    pthread_t producerThread;
    pthread_t consumerThread;
} TestData;


void *
consumer(void *p)
{
    TestData *data = (TestData *) p;
    --data->barrier;
    while (data->barrier) {
        ;
    }

    struct pollfd pfd;
    pfd.fd = parcNotifier_Socket(data->notifier);
    pfd.events = POLLIN;

    while (data->notificationsReceived < data->notificationsToRecieve) {
        if (poll(&pfd, 1, -1)) {
            data->notificationsReceived++;
            parcNotifier_PauseEvents(data->notifier);
            usleep(rand() % 1024 + 1024);
            printf("skipped = %d\n", data->notifier->skippedNotify);
            parcNotifier_StartEvents(data->notifier);
        }
    }

    --data->barrier;

    printf("Consumer exiting: received %d\n", data->notificationsReceived);
    pthread_exit((void *) NULL);
}

void *
producer(void *p)
{
    TestData *data = (TestData *) p;
    --data->barrier;
    while (data->barrier) {
        ;
    }

    while (data->barrier == 0) {
        if (parcNotifier_Notify(data->notifier)) {
        }
        data->notificationsSent++;
        usleep(rand() % 1024 + 512);
    }

    printf("Producer exiting: sent %d\n", data->notificationsSent);
    pthread_exit((void *) NULL);
}

LONGBOW_TEST_CASE(Global, parcNotifier_Acquire)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcNotifier_Create_Release)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcNotifier_PauseEvent_NotPaused)
{
    PARCNotifier *notifier = parcNotifier_Create();

    parcNotifier_PauseEvents(notifier);
    assertTrue(notifier->paused == 1, "Not paused, got %d expected %d", notifier->paused, 1);
    assertTrue(notifier->skippedNotify == 0, "Wrong skipped, got %d expected %d", notifier->skippedNotify, 0);

    parcNotifier_Release(&notifier);
}

LONGBOW_TEST_CASE(Global, parcNotifier_PauseEvent_AlreadyPaused)
{
    PARCNotifier *notifier = parcNotifier_Create();

    parcNotifier_PauseEvents(notifier);

    // now pause again
    parcNotifier_PauseEvents(notifier);

    assertTrue(notifier->paused == 1, "Not paused, got %d expected %d", notifier->paused, 1);
    assertTrue(notifier->skippedNotify == 0, "Wrong skipped, got %d expected %d", notifier->skippedNotify, 0);

    parcNotifier_Release(&notifier);
}


LONGBOW_TEST_CASE(Global, parcNotifier_ThreadedTest)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    data->notifier = parcNotifier_Create();
    data->notificationsToSend = 10;
    data->notificationsToRecieve = data->notificationsToSend;
    data->notificationsSent = 0;
    data->notificationsReceived = 0;
    data->barrier = 2;

    pthread_create(&data->consumerThread, NULL, consumer, data);
    pthread_create(&data->producerThread, NULL, producer, data);

    // wait for them to exit
    pthread_join(data->producerThread, NULL);
    pthread_join(data->consumerThread, NULL);

    assertTrue(data->notificationsReceived >= data->notificationsToRecieve,
               "Did not write all items got %u expected %u\n",
               data->notificationsReceived,
               data->notificationsToRecieve);

    parcNotifier_Release(&data->notifier);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_CASE(Global, parcNotifier_StartEvents)
{
    testUnimplemented("unimplemented");
}

LONGBOW_TEST_CASE(Global, parcNotifier_Notify_First)
{
    PARCNotifier *notifier = parcNotifier_Create();

    bool success = parcNotifier_Notify(notifier);
    assertTrue(success, "Did not succeed on first notify");
    assertTrue(notifier->paused == 1, "Not paused, got %d expected %d", notifier->paused, 1);
    assertTrue(notifier->skippedNotify == 0, "Wrong skipped, got %d expected %d", notifier->skippedNotify, 0);

    parcNotifier_Release(&notifier);
}


LONGBOW_TEST_CASE(Global, parcNotifier_Notify_Twice)
{
    PARCNotifier *notifier = parcNotifier_Create();

    parcNotifier_Notify(notifier);

    bool success = parcNotifier_Notify(notifier);
    assertFalse(success, "Should have failed on second notify");
    assertTrue(notifier->paused == 1, "Not paused, got %d expected %d", notifier->paused, 1);
    assertTrue(notifier->skippedNotify == 1, "Wrong skipped, got %d expected %d", notifier->skippedNotify, 1);

    parcNotifier_Release(&notifier);
}

// ===============================================================

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
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Notifier);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
