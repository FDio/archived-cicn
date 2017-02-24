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
#include <stdio.h>
#include <LongBow/unit-test.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnx_NameSegment.c"

LONGBOW_TEST_RUNNER(ccnx_NameComponent)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_NameComponent)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_NameComponent)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_CreateTypeValue);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Copy);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Copy_WithParameter);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Length);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_GetType);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_META);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_APP0);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_NAME);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_NAME_NotDefault);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_SERIAL);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_RawNAME);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_APP256);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ToString_PAYLOADHASH);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_NAME);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_META);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_list);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_UnknownLabel);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_ZeroLength);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Compare_Contract);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Equals_Contract);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_Display);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_IsValid_NULL);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameSegment_IsValid_InnerNULL1);
}

static uint32_t initialAllocationCount;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    initialAllocationCount = parcMemory_Outstanding();

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t remainingAllocations = parcMemory_Outstanding() - initialAllocationCount;
    if (remainingAllocations > 0) {
        printf("%s leaks memory by %u allocations\n", longBowTestCase_GetName(testCase), remainingAllocations);
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_IsValid)
{
    PARCBuffer *value = parcBuffer_WrapCString("Test");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, value);

    assertTrue(ccnxNameSegment_IsValid(segment), "Expected a valid CCNxNameSegment.");

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&value);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_IsValid_NULL)
{
    assertFalse(ccnxNameSegment_IsValid(NULL), "Expected NULL to be an invalid CCNxNameSegment.");
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_IsValid_InnerNULL1)
{
    PARCBuffer *buf = parcBuffer_WrapCString("Test");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    // Hit the NULL case
    PARCBuffer *oldBuffer = segment->value;
    segment->value = NULL;

    assertFalse(ccnxNameSegment_IsValid(segment),
                "Expected a name segment with a NULL value to be invalid.");

    segment->value = oldBuffer;

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_CreateTypeValue)
{
    PARCBuffer *buf = parcBuffer_WrapCString("Test");

    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);
    assertNotNull(segment, "Expected non-null");

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_CreateTypeParameterValue)
{
    PARCBuffer *parameter = parcBuffer_WrapCString("param");
    PARCBuffer *value = parcBuffer_WrapCString("Value");

    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, parameter);
    CCNxNameSegment *segment = ccnxNameSegment_CreateLabelValue(label, value);
    ccnxNameLabel_Release(&label);
    assertNotNull(segment, "Expected non-null");

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&parameter);
    parcBuffer_Release(&value);
}

// Convert the tests to use this table, instead of individual tests.
struct nameSegments {
    char *lciSegment;
    CCNxNameLabelType nameType;
    char *parameter;
    char *value;
} nameSegment[] = {
    { .lciSegment = "NAME",
      .nameType = CCNxNameLabelType_NAME, .parameter = NULL, .value = "NAME", },
    { .lciSegment = CCNxNameLabel_Name "=" "NAME",
      .nameType = CCNxNameLabelType_NAME, .parameter = NULL, .value = "NAME", },
    { .lciSegment = CCNxNameLabel_Chunk "=" "Chunk",
      .nameType = CCNxNameLabelType_CHUNK, .parameter = NULL, .value = "Chunk", },
    { .lciSegment = CCNxNameLabel_Chunk ":param=" "Chunk",
      .nameType = CCNxNameLabelType_CHUNK, .parameter = "param", .value = "Chunk", },
    { .lciSegment = CCNxNameLabel_App ":100=" "app100",
      .nameType = CCNxNameLabelType_App(100), .parameter = NULL, .value = "app100", },
    { .lciSegment = NULL },
};

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_list)
{
    for (struct nameSegments *p = &nameSegment[0]; p->lciSegment != NULL; p++) {
        PARCURISegment *segment = parcURISegment_Parse(p->lciSegment, NULL);

        CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

        CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
        assertTrue(p->nameType == type, "Expected %04x, actual %04x", p->nameType, type);

        PARCBuffer *valueValue = parcBuffer_WrapCString(p->value);
        PARCBuffer *expectedParam = p->parameter == NULL ? NULL : parcBuffer_WrapCString(p->parameter);
        CCNxNameLabel *label = ccnxNameLabel_Create(p->nameType, expectedParam);
        CCNxNameSegment *expected = ccnxNameSegment_CreateLabelValue(label, valueValue);
        ccnxNameLabel_Release(&label);

        parcBuffer_Release(&valueValue);
        if (expectedParam != NULL) {
            parcBuffer_Release(&expectedParam);
        }

        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(actual);
        assertTrue(ccnxNameSegment_Equals(expected, actual),
                   "Expected '%s' Actual, '%s", expectedString, actualString);
        parcMemory_Deallocate((void **) &expectedString);
        parcMemory_Deallocate((void **) &actualString);

        ccnxNameSegment_Release(&expected);
        ccnxNameSegment_Release(&actual);
        parcURISegment_Release(&segment);
    }
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_RawNAME)
{
    char *lciSegment = "NAME";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_NAME == type, "Expected %04x, actual %04x", CCNxNameLabelType_NAME, type);

    PARCBuffer *buf = parcBuffer_WrapCString("NAME");
    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    parcBuffer_Release(&buf);
    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_NAME)
{
    char *lciSegment = CCNxNameLabel_Name "=" "NAME";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_NAME == type, "Expected %04x, actual %04x", CCNxNameLabelType_NAME, type);

    PARCBuffer *buf = parcBuffer_WrapCString("NAME");
    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_META)
{
    char *lciSegment = CCNxNameLabel_ChunkMeta "=" "META";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_CHUNKMETA == type, "Expected 0x%04x, actual 0x%04x", CCNxNameLabelType_CHUNKMETA, type);

    PARCBuffer *buf = parcBuffer_WrapCString("META");

    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_CHUNKMETA, buf);

    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_UnknownLabel)
{
    char *lciSegment = "unknown:param" "=" "abcdef";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    parcURISegment_Release(&segment);

    assertNull(actual, "Expected NULL return from ccnxNameSegment_ParseURISegment");
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_APP0)
{
    char *lciSegment = CCNxNameLabelType_LabelApp(0) "=" "APP0";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_App(0) == type, "Expected %04x, actual %04x", CCNxNameLabelType_App(0), type);

    PARCBuffer *buf = parcBuffer_WrapCString("APP0");

    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_App(0), buf);
    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment_APP256)
{
    char *lciSegment = CCNxNameLabelType_LabelApp(255) "=" "APP255";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_App(255) == type, "Expected %04x, actual %04x", CCNxNameLabelType_App(255), type);

    PARCBuffer *buf = parcBuffer_WrapCString("APP255");
    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_App(255), buf);
    parcBuffer_Release(&buf);

    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ParseURISegment)
{
    char *lciSegment = CCNxNameLabel_Name "=" "abcde";
    PARCURISegment *segment = parcURISegment_Parse(lciSegment, NULL);

    CCNxNameSegment *actual = ccnxNameSegment_ParseURISegment(segment);

    CCNxNameLabelType type = ccnxNameSegment_GetType(actual);
    assertTrue(CCNxNameLabelType_NAME == type,
               "Expected %04x, actual %04x", 0x20, type);

    PARCBuffer *buf = parcBuffer_WrapCString("abcde");

    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    assertTrue(ccnxNameSegment_Equals(expected, actual), "Error in ccnxNameSegment_ParseURISegment")
    {
        char *expectedString = ccnxNameSegment_ToString(expected);
        char *actualString = ccnxNameSegment_ToString(expected);
        fprintf(stderr, "Expected '%s', actual '%s'", expectedString, actualString);
        parcMemory_Deallocate((void **) expectedString);
        parcMemory_Deallocate((void **) actualString);
    };

    ccnxNameSegment_Release(&expected);
    parcBuffer_Release(&buf);
    ccnxNameSegment_Release(&actual);
    parcURISegment_Release(&segment);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ZeroLength)
{
    char *begin = "";

    PARCBuffer *buffer = parcBuffer_WrapCString(begin);
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);

    assertNotNull(segment, "Expected non-null");
    assertTrue(ccnxNameSegment_Length(segment) == 0, "Failed to create a zero length segment");

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Equals_Contract)
{
    PARCBuffer *bufX = parcBuffer_WrapCString("Test");
    PARCBuffer *bufY = parcBuffer_WrapCString("Test");
    PARCBuffer *bufZ = parcBuffer_WrapCString("Test");
    PARCBuffer *bufU1 = parcBuffer_WrapCString("Test");
    PARCBuffer *bufU2 = parcBuffer_WrapCString("blah");

    CCNxNameSegment *x = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufX);
    CCNxNameSegment *y = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufY);
    CCNxNameSegment *z = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufZ);
    CCNxNameSegment *u1 = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_CHUNKMETA, bufU1);
    CCNxNameSegment *u2 = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufU2);

    assertEqualsContract(ccnxNameSegment_Equals, x, y, z, u1, u2, NULL);

    ccnxNameSegment_Release(&x);
    ccnxNameSegment_Release(&y);
    ccnxNameSegment_Release(&z);
    ccnxNameSegment_Release(&u1);
    ccnxNameSegment_Release(&u2);

    parcBuffer_Release(&bufX);
    parcBuffer_Release(&bufY);
    parcBuffer_Release(&bufZ);
    parcBuffer_Release(&bufU1);
    parcBuffer_Release(&bufU2);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Compare_Contract)
{
    char *aString = "foo";
    PARCBuffer *bufA = parcBuffer_WrapCString(aString);
    CCNxNameSegment *a = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufA);

    PARCBuffer *bufFoo = parcBuffer_WrapCString("foo");
    PARCBuffer *bufFon = parcBuffer_WrapCString("fon");
    PARCBuffer *bufFo = parcBuffer_WrapCString("fo");
    PARCBuffer *bufFop = parcBuffer_WrapCString("fop");
    PARCBuffer *bufFooa = parcBuffer_WrapCString("fooa");

    CCNxNameSegment **equivalents = (CCNxNameSegment *[]) {
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufFoo),
        NULL
    };
    CCNxNameSegment **lessers = (CCNxNameSegment *[]) {
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufFon),
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufFo),
        NULL
    };
    CCNxNameSegment **greaters = (CCNxNameSegment *[]) {
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufFop),
        ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufFooa),
        NULL
    };

    assertCompareToContract(ccnxNameSegment_Compare, a, equivalents, lessers, greaters);

    ccnxNameSegment_Release(&a);

    for (int i = 0; equivalents[i] != NULL; i++) {
        ccnxNameSegment_Release(&equivalents[i]);
    }
    for (int i = 0; lessers[i] != NULL; i++) {
        ccnxNameSegment_Release(&lessers[i]);
    }
    for (int i = 0; greaters[i] != NULL; i++) {
        ccnxNameSegment_Release(&greaters[i]);
    }

    parcBuffer_Release(&bufFoo);
    parcBuffer_Release(&bufFon);
    parcBuffer_Release(&bufFo);
    parcBuffer_Release(&bufFop);
    parcBuffer_Release(&bufFooa);
    parcBuffer_Release(&bufA);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Length)
{
    char *expected = "foo";

    PARCBuffer *buffer = parcBuffer_WrapCString(expected);
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buffer);

    size_t actual = ccnxNameSegment_Length(segment);

    ccnxNameSegment_Release(&segment);

    assertTrue(strlen(expected) == actual,
               "Expected %zd, actual %zd", strlen(expected), actual);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Copy)
{
    PARCBuffer *buf = parcBuffer_WrapCString("foo");
    CCNxNameSegment *expected = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);
    CCNxNameSegment *actual = ccnxNameSegment_Copy(expected);
    CCNxNameSegment *acquiredCopy = ccnxNameSegment_Acquire(expected);

    assertTrue(expected != actual, "Expected a distinct copy of the original.");

    char *expectedString = ccnxNameSegment_ToString(expected);
    char *actualString = ccnxNameSegment_ToString(actual);
    char *acquiredString = ccnxNameSegment_ToString(acquiredCopy);
    assertTrue(ccnxNameSegment_Equals(expected, actual), "Expected %s, actual %s", expectedString, actualString);
    assertTrue(ccnxNameSegment_Equals(expected, acquiredCopy), "Expected %s, actual %s", expectedString, acquiredString);

    parcMemory_Deallocate((void **) &expectedString);
    parcMemory_Deallocate((void **) &actualString);
    parcMemory_Deallocate((void **) &acquiredString);

    ccnxNameSegment_Release(&acquiredCopy);
    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Copy_WithParameter)
{
    PARCBuffer *value = parcBuffer_WrapCString("value");
    PARCBuffer *parameter = parcBuffer_WrapCString("param");
    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, parameter);
    CCNxNameSegment *expected = ccnxNameSegment_CreateLabelValue(label, value);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&value);
    parcBuffer_Release(&parameter);

    CCNxNameSegment *actual = ccnxNameSegment_Copy(expected);

    assertTrue(expected != actual, "Expected a distinct copy of the original.");

    char *expectedString = ccnxNameSegment_ToString(expected);
    char *actualString = ccnxNameSegment_ToString(actual);
    assertTrue(ccnxNameSegment_Equals(expected, actual), "Expected '%s', actual '%s'", expectedString, actualString);

    parcMemory_Deallocate((void **) &expectedString);
    parcMemory_Deallocate((void **) &actualString);

    ccnxNameSegment_Release(&expected);
    ccnxNameSegment_Release(&actual);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_GetType)
{
    PARCBuffer *buf = parcBuffer_WrapCString("hello");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    assertTrue(CCNxNameLabelType_NAME == ccnxNameSegment_GetType(segment),
               "Expected type %d, actual %d", CCNxNameLabelType_NAME, ccnxNameSegment_GetType(segment));

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_META)
{
    char *expected = CCNxNameLabel_ChunkMeta "=META";
    PARCBuffer *buf = parcBuffer_WrapCString("META");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_CHUNKMETA, buf);
    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_PAYLOADHASH)
{
    char *expected = CCNxNameLabel_InterestPayloadId "=PAYLOADHASH";
    PARCBuffer *buf = parcBuffer_WrapCString("PAYLOADHASH");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_PAYLOADID, buf);

    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_NAME)
{
    PARCBuffer *buf = parcBuffer_WrapCString("NAME");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    // Note that this is different than the other tests for segments because a NAME name segment
    // is the default type and as such the string representation doesn't include the leading label specification.
    char *expected = "NAME";
    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_NAME_NotDefault)
{
    PARCBuffer *value = parcBuffer_WrapCString("MiISAg==");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, value);

    // Note that this is different than the other tests for segments because a NAME name segment
    // is the default type and as such the string representation doesn't include the leading label specification.
    char *expected = CCNxNameLabel_Name "=" "MiISAg%3D%3D";
    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&value);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_APP0)
{
    char *expected = CCNxNameLabelType_LabelApp(0) "=APP0";
    PARCBuffer *buf = parcBuffer_WrapCString("APP0");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_App(0), buf);

    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_SERIAL)
{
    char *expected = CCNxNameLabel_Serial "=serialnumber";
    PARCBuffer *buf = parcBuffer_WrapCString("serialnumber");
    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_SERIAL, buf);

    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_ToString_APP256)
{
    char *expected = CCNxNameLabelType_LabelApp(255) "=APP255";
    PARCBuffer *buf = parcBuffer_WrapCString("APP255");

    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_App(255), buf);
    char *actual = ccnxNameSegment_ToString(segment);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_HashCode)
{
    PARCBuffer *bufA = parcBuffer_WrapCString("Test");

    CCNxNameSegment *segmentA = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufA);
    CCNxNameSegment *segmentB = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_CHUNKMETA, bufA);

    assertFalse(ccnxNameSegment_HashCode(segmentA) == ccnxNameSegment_HashCode(segmentB),
                "Expected different hash codes");

    PARCBuffer *bufC = parcBuffer_WrapCString("Not Test");

    CCNxNameSegment *segmentC = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufC);

    assertFalse(ccnxNameSegment_HashCode(segmentA) == ccnxNameSegment_HashCode(segmentC),
                "Expected different hash codes");

    CCNxNameSegment *segmentD = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, bufA);

    assertTrue(ccnxNameSegment_HashCode(segmentD) == ccnxNameSegment_HashCode(segmentA),
               "Expected same hash codes");

    ccnxNameSegment_Release(&segmentA);
    ccnxNameSegment_Release(&segmentB);
    ccnxNameSegment_Release(&segmentC);
    ccnxNameSegment_Release(&segmentD);

    parcBuffer_Release(&bufA);
    parcBuffer_Release(&bufC);
}

LONGBOW_TEST_CASE(Global, ccnxNameSegment_Display)
{
    PARCBuffer *buf = parcBuffer_WrapCString("Test");

    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, buf);

    ccnxNameSegment_Display(segment, 0);

    ccnxNameSegment_Release(&segment);
    parcBuffer_Release(&buf);
}

LONGBOW_TEST_CASE_EXPECTS(Global, ccnxNameSegment_AssertValid_Invalid, .event = &LongBowAssertEvent)
{
    ccnxNameSegment_AssertValid(NULL);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_NameComponent);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
