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
#include "../ccnx_NameLabel.c"

#include <LongBow/unit-test.h>
#include <stdio.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>


LONGBOW_TEST_RUNNER(ccnx_NameType)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Errors);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_NameType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_NameType)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_Create);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_GetType);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_GetParameter);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameLabel_Copy);

    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_NULL);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_Empty);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_App0);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_App);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Numeric_Decimal);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Numeric_Hex);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Resolve_Unknown_Mnemonic);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_BuildString_KnownLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_BuildString_UnknownLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_BuildString_AppLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_BuildString_AppLabel4096);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_DefaultLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_EmptyLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_KnownLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_DecimalParameterLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_HexadecimalParameterLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_OutOfRangeLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_BadHexLabel);
    LONGBOW_RUN_TEST_CASE(Global, ccnxNameType_Parse_UknownMnemonicLabel);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic)
{
    char *mnemonic = "Name";
    PARCBuffer *label = parcBuffer_WrapCString(mnemonic);
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == CCNxNameLabelType_NAME,
               "Expected an CCNxNameType_NAME type for the mnemonic '%s'", mnemonic);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_NULL)
{
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(NULL);
    assertTrue(actual == CCNxNameLabelType_NAME,
               "Expected an CCNxNameType_NAME type for a NULL mnemonic.");
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_Empty)
{
    PARCBuffer *label = parcBuffer_Allocate(0);
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == CCNxNameLabelType_NAME,
               "Expected an CCNxNameType_NAME type for an empty mnemonic.");
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_App0)
{
    PARCBuffer *label = parcBuffer_WrapCString("app");
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == CCNxNameLabelType_App(0),
               "Expected 0x%04x type, actual 0x%04x", CCNxNameLabelType_App(0), actual);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Mnemonic_App)
{
    PARCBuffer *label = parcBuffer_WrapCString("app");
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == CCNxNameLabelType_App(0),
               "Expected 0x%04x type, actual 0x%04x", CCNxNameLabelType_App(0), actual);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Numeric_Decimal)
{
    PARCBuffer *label = parcBuffer_WrapCString("16");
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == CCNxNameLabelType_CHUNK,
               "Expected type %d, actual %d", CCNxNameLabelType_CHUNK, actual);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Numeric_Hex)
{
    CCNxNameLabelType expected = 0xF000;
    PARCBuffer *label = parcBuffer_WrapCString("0xF000");
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == expected,
               "Expected type %d, actual %d", expected, actual);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Resolve_Unknown_Mnemonic)
{
    CCNxNameLabelType expected = CCNxNameLabelType_Unknown;
    PARCBuffer *label = parcBuffer_WrapCString("xyzzy");
    CCNxNameLabelType actual = _ccnxNameLabelType_Resolve(label);
    assertTrue(actual == expected,
               "Expected type %d, actual %d", expected, actual);
    parcBuffer_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_BuildString_KnownLabel)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_CHUNK, NULL);
    ccnxNameLabel_BuildString(label, composer);
    ccnxNameLabel_Release(&label);

    PARCBuffer *expected = parcBuffer_WrapCString(CCNxNameLabel_Chunk "=");
    PARCBuffer *actual = parcBuffer_Flip(parcBufferComposer_GetBuffer(composer));

    assertTrue(parcBuffer_Equals(expected, actual),
               "Expected a successful label lookup.");
    parcBuffer_Release(&expected);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_BuildString_UnknownLabel)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    CCNxNameLabel *label = ccnxNameLabel_Create(1111, NULL);
    ccnxNameLabel_BuildString(label, composer);
    ccnxNameLabel_Release(&label);

    PARCBuffer *expected = parcBuffer_WrapCString("1111" "=");
    PARCBuffer *actual = parcBuffer_Flip(parcBufferComposer_GetBuffer(composer));

    assertTrue(parcBuffer_Equals(expected, actual),
               "Expected a successful label lookup.");

    parcBuffer_Release(&expected);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_BuildString_AppLabel)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    PARCBuffer *parameter = parcBuffer_WrapCString("0");
    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_App(0), parameter);
    ccnxNameLabel_BuildString(label, composer);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&parameter);

    PARCBuffer *expected = parcBuffer_WrapCString("App:0" "=");
    PARCBuffer *actual = parcBuffer_Flip(parcBufferComposer_GetBuffer(composer));

    assertTrue(parcBuffer_Equals(expected, actual),
               "Expected a successful label lookup.");

    parcBuffer_Release(&expected);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_BuildString_AppLabel4096)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    PARCBuffer *parameter = parcBuffer_WrapCString("4096");
    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_App(4096), parameter);
    ccnxNameLabel_BuildString(label, composer);

    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&parameter);

    PARCBuffer *expected = parcBuffer_WrapCString("App:4096" "=");
    PARCBuffer *actual = parcBuffer_Flip(parcBufferComposer_GetBuffer(composer));

    assertTrue(parcBuffer_Equals(expected, actual),
               "Expected a successful label lookup.");

    parcBuffer_Release(&expected);
    parcBufferComposer_Release(&composer);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_GetType)
{
    CCNxNameLabelType type = CCNxNameLabelType_NAME;
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *label = ccnxNameLabel_Create(type, parameter);

    CCNxNameLabelType actual = ccnxNameLabel_GetType(label);

    assertTrue(type == actual, "Expected type %u, actual %u", type, actual);

    parcBuffer_Release(&parameter);
    ccnxNameLabel_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_GetParameter)
{
    CCNxNameLabelType type = CCNxNameLabelType_NAME;
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *label = ccnxNameLabel_Create(type, parameter);
    PARCBuffer *actual = ccnxNameLabel_GetParameter(label);

    assertTrue(parcBuffer_Equals(parameter, actual), "Expected parameter to be equal to the initial parameter.");
    parcBuffer_Release(&parameter);
    ccnxNameLabel_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_Create)
{
    CCNxNameLabelType type = CCNxNameLabelType_NAME;
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *label = ccnxNameLabel_Create(type, parameter);
    parcBuffer_Release(&parameter);
    ccnxNameLabel_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_Copy)
{
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *label = ccnxNameLabel_Create(CCNxNameLabelType_NAME, parameter);
    parcBuffer_Release(&parameter);

    CCNxNameLabel *copy = ccnxNameLabel_Copy(label);

    assertTrue(ccnxNameLabel_Equals(label, copy), "Expected copy to the equal to the original.");

    assertTrue(label != copy, "Expected a copy to be distinct from the original.");
    ccnxNameLabel_Release(&label);
    ccnxNameLabel_Release(&copy);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_Equals)
{
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *x = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, parameter);
    parcBuffer_Release(&parameter);

    parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *y = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, parameter);
    parcBuffer_Release(&parameter);

    parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *z = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, parameter);
    parcBuffer_Release(&parameter);

    parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *u1 = ccnxNameLabel_Create(CCNxNameLabelType_CHUNK, parameter);
    parcBuffer_Release(&parameter);

    parameter = parcBuffer_WrapCString("Goodbye");
    CCNxNameLabel *u2 = ccnxNameLabel_Create(CCNxNameLabelType_SERIAL, parameter);
    parcBuffer_Release(&parameter);

    assertEqualsContract(ccnxNameLabel_Equals, x, y, z, u1, u2, NULL);

    ccnxNameLabel_Release(&x);
    ccnxNameLabel_Release(&y);
    ccnxNameLabel_Release(&z);
    ccnxNameLabel_Release(&u1);
    ccnxNameLabel_Release(&u2);
}

LONGBOW_TEST_CASE(Global, ccnxNameLabel_AcquireRelease)
{
    CCNxNameLabelType type = CCNxNameLabelType_NAME;
    PARCBuffer *parameter = parcBuffer_WrapCString("Hello");
    CCNxNameLabel *label = ccnxNameLabel_Create(type, parameter);

    parcObjectTesting_AssertAcquireReleaseContract(ccnxNameLabel_Acquire, label);

    parcBuffer_Release(&parameter);
    ccnxNameLabel_Release(&label);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse)
{
    char *expected = "App:1=value";
    PARCBuffer *buffer = parcBuffer_WrapCString(expected);
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);
    assertTrue(parcBuffer_Position(buffer) == 6, "Expected position to be 6, actual %zd", parcBuffer_Position(buffer));
    assertTrue(strcmp("App:1=", actual) == 0, "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);

    expected = "10:param=value";
    buffer = parcBuffer_WrapCString(expected);
    label = ccnxNameLabel_Parse(buffer);
    actual = ccnxNameLabel_ToString(label);
    assertTrue(parcBuffer_Position(buffer) == 9, "Expected position to be 9, actual %zd", parcBuffer_Position(buffer));
    assertTrue(strcmp("10:param=", actual) == 0, "Expected %s, actual %s", "10:param=", actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_DecimalParameterLabel)
{
    char *expected = "10:param=value";
    PARCBuffer *buffer = parcBuffer_WrapCString(expected);
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);
    assertTrue(parcBuffer_Position(buffer) == 9, "Expected position to be 9, actual %zd", parcBuffer_Position(buffer));
    assertTrue(strcmp("10:param=", actual) == 0, "Expected %s, actual %s", "10:param=", actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_HexadecimalParameterLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("0xaa:param=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);
    assertTrue(parcBuffer_Position(buffer) == 11, "Expected position to be 11, actual %zd", parcBuffer_Position(buffer));
    char *expected = "170:param=";
    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_BadHexLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("0xgg:param=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    assertFalse(ccnxNameLabel_IsValid(label), "Expected an invalid CCNxNameLabel from an invalid specification.");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_OutOfRangeLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("0x123456=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);

    assertTrue(parcBuffer_Position(buffer) == 9, "Expected position to be 9, actual %zd", parcBuffer_Position(buffer));
    char *expected = "1193046=";
    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_UknownMnemonicLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("abc=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);

    assertFalse(ccnxNameLabel_IsValid(label), "Expected an invalid CCNxNameLabel from an invalid specification.");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_KnownLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("Serial=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_DefaultLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    char *actual = ccnxNameLabel_ToString(label);
    char *expected = "Name=";
    assertTrue(strcmp(expected, actual) == 0, "Expected %s, actual %s", expected, actual);
    parcMemory_Deallocate((void **) &actual);
    ccnxNameLabel_Release(&label);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxNameType_Parse_EmptyLabel)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("=value");
    CCNxNameLabel *label = ccnxNameLabel_Parse(buffer);
    assertNull(label, "Expected a NULL return value from ccnxNameLabel_Parse for the invalid string '=value'");
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_FIXTURE(Errors)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Errors)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Errors)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_NameType);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
