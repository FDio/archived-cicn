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
#include "../parc_LogEntry.c"

#include <LongBow/unit-test.h>

#include <stdio.h>
#include <pthread.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <parc/logging/parc_LogReporter.h>
#include <parc/logging/parc_LogReporterTextStdout.h>
#include <parc/logging/parc_LogFormatText.h>

LONGBOW_TEST_RUNNER(parc_LogEntry)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Creation);
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(Global);

    LONGBOW_RUN_TEST_FIXTURE(MultiThreaded);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_LogEntry)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_LogEntry)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Creation)
{
    LONGBOW_RUN_TEST_CASE(Creation, parcLogEntry_CreateRelease);
}

uint32_t CreationInitialMemoryOutstanding = 0;

LONGBOW_TEST_FIXTURE_SETUP(Creation)
{
    CreationInitialMemoryOutstanding = parcMemory_Outstanding();

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Creation)
{
    if (parcMemory_Outstanding() != CreationInitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("'%s' leaks memory by %u\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - CreationInitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Creation, parcLogEntry_CreateRelease)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 12345, timeStamp, payload);
    parcBuffer_Release(&payload);

    parcObjectTesting_AssertAcquireReleaseContract(parcLogEntry_Acquire, entry);
    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetTimeStamp);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetMessageId);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetApplicationName);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetHostName);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetLevel);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetProcessName);
    LONGBOW_RUN_TEST_CASE(Global, parcLogEntry_GetVersion);
}

uint32_t GlobalInitialMemoryOutstanding = 0;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    GlobalInitialMemoryOutstanding = parcMemory_Outstanding();

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcMemory_Outstanding() != GlobalInitialMemoryOutstanding) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("'%s' leaks memory by %u\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - GlobalInitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcLogEntry_AcquireRelease)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry = parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    parcBuffer_Release(&payload);

    parcObjectTesting_AssertAcquireReleaseContract(parcLogEntry_Acquire, entry);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetBuffer)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry = parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    PARCBuffer *actual = parcLogEntry_GetPayload(entry);

    assertTrue(payload == actual, "Expected %p, actual %p", (void *) payload, (void *) actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetTimeStamp)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    const struct timeval *actual = parcLogEntry_GetTimeStamp(entry);

    assertTrue(memcmp(&timeStamp, actual, sizeof(struct timeval)) == 0, "Expected timeStamp to be identical");
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetLevel)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    const PARCLogLevel actual = parcLogEntry_GetLevel(entry);

    assertTrue(PARCLogLevel_Info == actual, "Expected %d, actual %d", PARCLogLevel_Info, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetVersion)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    const PARCLogLevel actual = parcLogEntry_GetVersion(entry);

    assertTrue(_parcLog_Version == actual, "Expected %d, actual %d", _parcLog_Version, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetHostName)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");
    char *expected = "hostname";

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, expected, "applicationname", "processid", 1234, timeStamp, payload);
    const char *actual = parcLogEntry_GetHostName(entry);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetApplicationName)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");
    char *expected = "applicationname";

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);
    const char *actual = parcLogEntry_GetApplicationName(entry);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetProcessName)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");
    char *expected = "processid";

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", expected, 1234, timeStamp, payload);
    const char *actual = parcLogEntry_GetProcessName(entry);

    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_GetMessageId)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);

    uint64_t expected = 1234;
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", expected, timeStamp, payload);
    const uint64_t actual = parcLogEntry_GetMessageId(entry);

    assertTrue(expected == actual, "Expected %" PRId64 " actual %" PRId64 "", expected, actual);
    parcBuffer_Release(&payload);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_CASE(Global, parcLogEntry_ToString)
{
    PARCBuffer *payload = parcBuffer_AllocateCString("hello");

    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    PARCLogEntry *entry =
        parcLogEntry_Create(PARCLogLevel_Info, "hostname", "applicationname", "processid", 1234, timeStamp, payload);

    parcBuffer_Release(&payload);

    char *actual = parcLogEntry_ToString(entry);

    parcMemory_Deallocate((void **) &actual);

    parcLogEntry_Release(&entry);
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _parcLogEntry_Destroy);
    LONGBOW_RUN_TEST_CASE(Static, _toString);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, _parcLogEntry_Destroy)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _toString)
{
    testUnimplemented("");
}

// Multi-threaded test

LONGBOW_TEST_FIXTURE(MultiThreaded)
{
    LONGBOW_RUN_TEST_CASE(MultiThreaded, fgThreadTest);
    LONGBOW_RUN_TEST_CASE(MultiThreaded, bgThreadTest);
}

LONGBOW_TEST_FIXTURE_SETUP(MultiThreaded)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(MultiThreaded)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        //parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        printf("'%s' leaks memory by %u allocations\n",
               longBowTestCase_GetName(testCase), parcMemory_Outstanding() - CreationInitialMemoryOutstanding);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


void *_runInThread(void *threadLabel);
void *_runInThread_Stripped(void *threadLabel);

static int _loopCount = INT32_MAX;

void *
_runInThread(void *threadLabel)  // Look at _runInThread_Stripped(), below, instead of this one.
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    PARCBuffer *payload = parcBuffer_AllocateCString(threadLabel);
    struct timeval timeStamp;
    gettimeofday(&timeStamp, NULL);

    PARCLogEntry *entry = parcLogEntry_Create(PARCLogLevel_Info, "hostName", "applicationName", "processName", _loopCount, timeStamp, payload);

    while (_loopCount > 0) {
        PARCBuffer *buf = parcLogFormatText_FormatEntry(entry);

        parcLogReporterTextStdout_Report(reporter, entry);
        parcBuffer_Release(&buf);

        _loopCount--;

        usleep(10 * 1000); // yield for a bit to let another thread have at it.
    }

    parcLogEntry_Release(&entry);

    parcBuffer_Release(&payload);
    parcLogReporter_Release(&reporter);

    return threadLabel; // Unchanged from what was passed in.
}


void *
_runInThread_Stripped(void *threadLabel)
{
    PARCBuffer *payload = parcBuffer_AllocateCString(threadLabel);

    while (_loopCount > 0) {
        //
        // The code below taken from parcLogReporterTextStdout_Report(). I stripped it down some.
        // If you switch the thread's job from _runInThread_Stripped to _runInThread you can run
        // the original, which shows the same thing.
        //

        PARCBufferComposer *composer = parcBufferComposer_Allocate(128);

        parcBufferComposer_Format(composer, "%d [ ", _loopCount);
        parcBufferComposer_PutBuffer(composer, payload);
        parcBufferComposer_PutStrings(composer, " ]\n", NULL);

        PARCBuffer *result = parcBuffer_Flip(parcBuffer_Acquire(parcBufferComposer_GetBuffer(composer)));
        parcBufferComposer_Release(&composer);

        char *string = parcBuffer_ToString(result);
        parcBuffer_Release(&result);

        printf("%s", string);

        parcMemory_Deallocate((void **) &string);

        _loopCount--;

        usleep(10 * 1000); // yield for a bit to let another thread have at it.
    }

    parcBuffer_Release(&payload);

    return threadLabel; // Unchanged from what was passed in.
}


LONGBOW_TEST_CASE(MultiThreaded, bgThreadTest)
{
    int numThreads = 2;
    pthread_t workerThreads[numThreads];
    char *threadLabel[numThreads];

    _loopCount = INT32_MAX; // We'll set it to 0 after a second
    for (int i = 0; i < numThreads; i++) {
        if (asprintf(&threadLabel[i], "bg thread #%d", i) > 0) {
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

            pthread_create(&workerThreads[i], &attr, _runInThread_Stripped, threadLabel[i]);
        }
        //pthread_create(&workerThreads[i], &attr, _runInThread, threadLabel[i]);
    }

    sleep(2);  // Let the bg threads run

    _loopCount = 0;  // tell the bg threads to stop

    for (int i = 0; i < numThreads; i++) {
        int status = pthread_join(workerThreads[i], NULL);
        printf("Child %d (out of %d) joined with status %d\n", i, numThreads, status);
        free(threadLabel[i]);
    }
}

LONGBOW_TEST_CASE(MultiThreaded, fgThreadTest)
{
    _loopCount = 10;
    //_runInThread("main thread");   // Run the same logging loop, but in a single thread
    _runInThread_Stripped("main thread");   // Run the same logging loop, but in a single thread
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_LogEntry);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
