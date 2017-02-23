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
#include "../ccnxCodecSchemaV1_TlvDictionary.c"
#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(ccnxCodecSchemaV1_TlvDictionary)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnxCodecSchemaV1_TlvDictionary)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnxCodecSchemaV1_TlvDictionary)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateInterest);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateContentObject);
    LONGBOW_RUN_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateControl);
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

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateInterest)
{
    CCNxTlvDictionary *test = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
    assertNotNull(test, "Got null return from ccnxCodecSchemaV1TlvDictionary_CreateInterest");
    assertTrue(ccnxTlvDictionary_IsInterest(test), "Dictionary is not an interest");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema version, expected %d got %d",
               CCNxTlvDictionary_SchemaVersion_V1,
               ccnxTlvDictionary_GetSchemaVersion(test));
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn)
{
    CCNxTlvDictionary *test = ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn();
    assertNotNull(test, "Got null return from ccnxCodecSchemaV1TlvDictionary_CreateInterest");
    assertTrue(ccnxTlvDictionary_IsInterestReturn(test), "Dictionary is not an interest");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema version, expected %d got %d",
               CCNxTlvDictionary_SchemaVersion_V1,
               ccnxTlvDictionary_GetSchemaVersion(test));
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateContentObject)
{
    CCNxTlvDictionary *test = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
    assertNotNull(test, "Got null return from ccnxCodecSchemaV1TlvDictionary_CreateContentObject");
    assertTrue(ccnxTlvDictionary_IsContentObject(test), "Dictionary is not a content object");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema version, expected %d got %d",
               CCNxTlvDictionary_SchemaVersion_V1,
               ccnxTlvDictionary_GetSchemaVersion(test));
    ccnxTlvDictionary_Release(&test);
}

LONGBOW_TEST_CASE(Global, ccnxCodecSchemaV1TlvDictionary_CreateControl)
{
    CCNxTlvDictionary *test = ccnxCodecSchemaV1TlvDictionary_CreateControl();
    assertNotNull(test, "Got null return from ccnxCodecSchemaV1TlvDictionary_CreateControl");
    assertTrue(ccnxTlvDictionary_IsControl(test), "Dictionary is not a control");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(test) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema version, expected %d got %d",
               CCNxTlvDictionary_SchemaVersion_V1,
               ccnxTlvDictionary_GetSchemaVersion(test));
    ccnxTlvDictionary_Release(&test);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnxCodecSchemaV1_TlvDictionary);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
