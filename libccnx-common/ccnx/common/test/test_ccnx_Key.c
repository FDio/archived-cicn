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
#include <config.h>
#include "../ccnx_Key.c"
#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(ccnx_Key)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(ccnx_Key)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_Key)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxKey_CreateRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxKey_CreateFromHexString);
    LONGBOW_RUN_TEST_CASE(Global, ccnxKey_FromByteBuffer);
    LONGBOW_RUN_TEST_CASE(Global, ccnxKey_ToString);
}

typedef struct {
    char *hexString;
} TestData;

static TestData*
commonSetup()
{
    char *hexString = "30819F300D06092A864886F70D010101050003818D0030818902818100A826C09E01FF4970428213C96312B46050514FD5F87E670A4784C75D8B23CD073B1CBEF328E538584E442A769DF77299192BCF3603F50F14C5664994250E5C24DF47B86EA5C7CA99B3584E9A63BC5993569FF3612C71AD46A088CDC7346B9BE021D4CA1764CF5434F993E6120363C551E2979BDB3F0345B4994BCED9CB260EEB0203010001";

    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->hexString = parcMemory_StringDuplicate(hexString, strlen(hexString));
    return data;
}

static void
commonTearDown(TestData *data)
{
    parcMemory_Deallocate((void **) &(data->hexString));
    parcMemory_Deallocate((void **) &data);
}


LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = commonSetup();
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    commonTearDown(data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}


LONGBOW_TEST_CASE(Global, ccnxKey_FromByteBuffer)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCBuffer *hexBuf = parcBuffer_ParseHexString(data->hexString);
    CCNxKey *key = ccnxKey_Create(hexBuf);
    parcBuffer_Release(&hexBuf);
    ccnxKey_Release(&key);
}

LONGBOW_TEST_CASE(Global, ccnxKey_CreateRelease)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxKey *key = ccnxKey_CreateFromHexString(data->hexString);
    ccnxKey_AssertValid(key);

    char *string = ccnxKey_ToString(key);

    ccnxKey_Release(&key);
    assertNull(key, "Key was not nulled out after Release()");

    parcMemory_Deallocate((void **) &string);
}

LONGBOW_TEST_CASE(Global, ccnxKey_CreateFromHexString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxKey *key = ccnxKey_CreateFromHexString(data->hexString);
    ccnxKey_AssertValid(key);

    char *string = ccnxKey_ToHexString(key);

    assertTrue(strcasecmp(data->hexString, string) == 0,
               "Expected '%s' actual '%s'", data->hexString, string);

    ccnxKey_Release(&key);
    assertNull(key, "Key was not nulled out after Release()");

    parcMemory_Deallocate((void **) &string);
}

LONGBOW_TEST_CASE(Global, ccnxKey_ToString)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    CCNxKey *key = ccnxKey_CreateFromHexString(data->hexString);
    ccnxKey_AssertValid(key);

    char *string = ccnxKey_ToString(key);

    ccnxKey_Release(&key);
    assertNull(key, "Key was not nulled out after Release()");

    parcMemory_Deallocate((void **) &string);
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
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_Key);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
