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
#include <ccnx/common/ccnx_TimeStamp.c>

#include <inttypes.h>
#include <time.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>


LONGBOW_TEST_RUNNER(ccnx_TimeStamp)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(ccnx_TimeStamp)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_TimeStamp)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_CreateFromCurrentUTCTime);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_CreateFromTimespec);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_CreateFromMillisecondsSinceEpoch);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_Equals);

    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_AsNanoSeconds);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_CreateFromNanosecondsSinceEpoch);
    LONGBOW_RUN_TEST_CASE(Global, ccnxTimeStamp_ToString);
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

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_CreateFromCurrentUTCTime)
{
    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
    assertNotNull(timeStamp, "Expected a non-null response");

    ccnxTimeStamp_Release(&timeStamp);
    assertNull(timeStamp, "Release failed to NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_CreateFromTimespec)
{
    struct timespec time = { .tv_sec = 1, .tv_nsec = 1 };

    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromTimespec(&time);
    assertNotNull(timeStamp, "Expected a non-null response");

    struct timespec actualTime = ccnxTimeStamp_AsTimespec(timeStamp);

    assertTrue(time.tv_sec == actualTime.tv_sec && time.tv_nsec == actualTime.tv_nsec, "Expected timespec to be equal.");

    ccnxTimeStamp_Release(&timeStamp);
    assertNull(timeStamp, "Release failed to NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_CreateFromMillisecondsSinceEpoch)
{
    time_t theTimeInSeconds;
    time(&theTimeInSeconds);

    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch((uint64_t) theTimeInSeconds * 1000);
    assertNotNull(timeStamp, "Expected a non-null response");

    struct timespec timeSpec = ccnxTimeStamp_AsTimespec(timeStamp);

    assertTrue(theTimeInSeconds == timeSpec.tv_sec, "Expected %ld, actual %ld", theTimeInSeconds, timeSpec.tv_sec);

    assertTrue(0 == timeSpec.tv_nsec, "Expected %d, actual %ld", 0, timeSpec.tv_nsec);

    ccnxTimeStamp_Release(&timeStamp);
    assertNull(timeStamp, "Release failed to NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_CreateFromNanosecondsSinceEpoch)
{
    uint64_t expected = 1099511627776ULL;

    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromNanosecondsSinceEpoch(expected);

    uint64_t actual = ccnxTimeStamp_AsNanoSeconds(timeStamp);

    assertTrue(expected == actual, "Expected %" PRIu64 " actual %" PRIu64, expected, actual);

    ccnxTimeStamp_Release(&timeStamp);
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_Equals)
{
    time_t theTimeInSeconds;
    time(&theTimeInSeconds);

    CCNxTimeStamp *x = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(theTimeInSeconds * 1000);
    CCNxTimeStamp *y = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(theTimeInSeconds * 1000);
    CCNxTimeStamp *z = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(theTimeInSeconds * 1000);
    CCNxTimeStamp *u1 = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch((theTimeInSeconds + 1) * 1000);
    CCNxTimeStamp *u2 = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch((theTimeInSeconds + 2) * 1000);

    assertEqualsContract(ccnxTimeStamp_Equals, x, y, z, u1, u2)

    ccnxTimeStamp_Release(&x);
    ccnxTimeStamp_Release(&y);
    ccnxTimeStamp_Release(&z);
    ccnxTimeStamp_Release(&u1);
    ccnxTimeStamp_Release(&u2);
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_Copy)
{
    time_t theTime;
    time(&theTime);

    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromMillisecondsSinceEpoch(theTime);
    assertNotNull(timeStamp, "Expected a non-null response");

    CCNxTimeStamp *copy = ccnxTimeStamp_Copy(timeStamp);

    char *expected = ccnxTimeStamp_ToString(timeStamp);
    char *actual = ccnxTimeStamp_ToString(copy);
    assertTrue(ccnxTimeStamp_Equals(timeStamp, copy),
               "Expected %s actual %s.", expected, actual);
    parcMemory_Deallocate((void **) &expected);
    parcMemory_Deallocate((void **) &actual);

    ccnxTimeStamp_Release(&timeStamp);
    ccnxTimeStamp_Release(&copy);
    assertNull(timeStamp, "Destroy failed to NULL the pointer.");
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_AsNanoSeconds)
{
    uint64_t expected = 1099501627776ULL;

    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromNanosecondsSinceEpoch(expected);

    uint64_t actual = ccnxTimeStamp_AsNanoSeconds(timeStamp);

    assertTrue(expected == actual, "Expected %" PRIu64 " actual %" PRIu64, expected, actual);

    ccnxTimeStamp_Release(&timeStamp);
}

LONGBOW_TEST_CASE(Global, ccnxTimeStamp_ToString)
{
    CCNxTimeStamp *timeStamp = ccnxTimeStamp_CreateFromCurrentUTCTime();
    assertNotNull(timeStamp, "Expected a non-null response");

    char *string = ccnxTimeStamp_ToString(timeStamp);
    assertNotNull(string, "Expected non-null result.");

    parcMemory_Deallocate((void **) &string);

    ccnxTimeStamp_Release(&timeStamp);
    assertNull(timeStamp, "Destroy failed to NULL the pointer.");
    // See case 1016
    testUnimplemented("This test is unimplemented");
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
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_TimeStamp);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
