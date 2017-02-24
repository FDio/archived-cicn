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
#include "../metis_Forwarder.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(metis_Forwarder)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_Forwarder)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_Forwarder)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, byteArrayToUnsignedLong);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_GetDispatcher);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_GetMessenger);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_GetNextConnectionId);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_GetTicks);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_Log);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_Receive);
    LONGBOW_RUN_TEST_CASE(Global, metis_run);
    LONGBOW_RUN_TEST_CASE(Global, metis_stop);


    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_NanosToTicks_1sec);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_NanosToTicks_1msec);
    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_NanosToTicks_LessThanHz);

    LONGBOW_RUN_TEST_CASE(Global, metisForwarder_TicksToNanos_1sec);
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

LONGBOW_TEST_CASE(Global, byteArrayToUnsignedLong)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_Create)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_Destroy)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_GetDispatcher)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_GetMessenger)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_GetNextConnectionId)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_GetTicks)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(metis);

    int msec = 50;
    struct timeval duration = { 0, msec * 1000 };

    // run for a bit to get things primed
    metisDispatcher_RunDuration(dispatcher, &duration);

    MetisTicks t0 = metisForwarder_GetTicks(metis);
    metisDispatcher_RunDuration(dispatcher, &duration);
    MetisTicks t1 = metisForwarder_GetTicks(metis);

    int64_t tickDelta = (int64_t) (t1 - t0);
    int64_t tickError = llabs(msec - tickDelta);

    metisForwarder_Destroy(&metis);

    printf("tickError = %" PRId64 "\n", tickError);

    // Test we're somewhere in the ballpark, betwen 40msec - 60msec
    assertTrue(tickError <= 10, "tickError %" PRId64
       " (tickDelta %" PRId64 ")",
       tickError, tickDelta);

}

LONGBOW_TEST_CASE(Global, metisForwarder_Log)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_Receive)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metis_run)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metis_stop)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, metisForwarder_NanosToTicks_1sec)
{
    // 1 second
    uint64_t nanos = 1000000000ULL;
    MetisTicks t = metisForwarder_NanosToTicks(nanos);

    assertTrue(t == METISHZ, "1 second in nanos did not equal METISHZ, expected %d, got %" PRIu64, METISHZ, t);
}

LONGBOW_TEST_CASE(Global, metisForwarder_NanosToTicks_1msec)
{
    // 1 second
    uint64_t nanos = 1000000ULL;
    MetisTicks t = metisForwarder_NanosToTicks(nanos);
    MetisTicks expected = METISHZ / 1000;
    if (expected == 0) {
        expected = 1;
    }

    assertTrue(t == expected, "1 msec in nanos did not equal METISHZ/1000, expected %" PRIu64 ", got %" PRIu64, expected, t);
}

LONGBOW_TEST_CASE(Global, metisForwarder_NanosToTicks_LessThanHz)
{
    // 1 second
    uint64_t nanos = 1ULL;
    MetisTicks t = metisForwarder_NanosToTicks(nanos);
    MetisTicks expected = 1;

    assertTrue(t == expected, "1 nsec in nanos did not equal 1, expected %" PRIu64 ", got %" PRIu64, expected, t);
}

LONGBOW_TEST_CASE(Global, metisForwarder_TicksToNanos_1sec)
{
    MetisTicks t = METISHZ;
    uint64_t expected = ((1000000000ULL) * t / METISHZ);

    uint64_t nanos = metisForwarder_TicksToNanos(t);
    assertTrue(nanos == expected, "METISHZ ticks does not equal 1sec, expected %" PRIu64 ", got %" PRIu64, expected, nanos);
}

// ======================================================================

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, metisForwarder_Seed);
    LONGBOW_RUN_TEST_CASE(Local, signal_cb);
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

LONGBOW_TEST_CASE(Local, metisForwarder_Seed)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, signal_cb)
{
    testUnimplemented("This test is unimplemented");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_Forwarder);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
