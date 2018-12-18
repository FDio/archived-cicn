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

#include <config.h>
#include <stdio.h>
#include <pthread.h>

#include <arpa/inet.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_EventSocket.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_EventSocket.c"

LONGBOW_TEST_RUNNER(parc_EventSocket)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_EventSocket)
{
    parcEventSocket_EnableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_EventSocket)
{
    parcEventSocket_DisableDebug();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_EventSocket_Create_Destroy);
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

static int _test_event_called = 0;

static void
listener_callback(int fd, struct sockaddr *sa, int socklen, void *user_data)
{
    _test_event_called++;
}

static int _test_error_event_called = 0;

static void
listener_error_callback(PARCEventScheduler *base, int error, char *errorString, void *addr_unix)
{
    _test_error_event_called++;
}

LONGBOW_TEST_CASE(Global, parc_EventSocket_Create_Destroy)
{
    PARCEventScheduler *parcEventScheduler = parcEventScheduler_Create();
    assertNotNull(parcEventScheduler, "parcEventScheduler_Create returned a null reference");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(49009);
    inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));

    PARCEventSocket *parcEventSocket = parcEventSocket_Create(parcEventScheduler,
                                                              listener_callback,
                                                              listener_error_callback,
                                                              NULL, NULL, 0);
    assertNull(parcEventSocket, "parcEventSocket_Create didn't return an error when expected");

    parcEventSocket = parcEventSocket_Create(parcEventScheduler,
                                             listener_callback,
                                             listener_error_callback,
                                             NULL, (struct sockaddr *) &addr, sizeof(addr));
    assertNotNull(parcEventSocket, "parcEventSocket_Create returned a null reference");

    _parc_evconn_callback(NULL, 0, (struct sockaddr *) &addr, sizeof(addr), parcEventSocket);
    assertTrue(_test_event_called == 1, "Listener callback wasn't triggered");
    _parc_evconn_error_callback(NULL, parcEventSocket);
    assertTrue(_test_error_event_called == 1, "Listener error callback wasn't triggered");

    parcEventSocket_Destroy(&parcEventSocket);
    parcEventScheduler_Destroy(&parcEventScheduler);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_EventSocket);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
