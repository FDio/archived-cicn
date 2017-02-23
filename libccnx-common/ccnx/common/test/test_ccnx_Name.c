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

#include "../ccnx_Name.c"

#include <LongBow/unit-test.h>

#include <stdio.h>
#include <limits.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(ccnx_Name)
{
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Specialization);
    LONGBOW_RUN_TEST_FIXTURE(Performance);
}

LONGBOW_TEST_RUNNER_SETUP(ccnx_Name)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_Name)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromCString);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromCString_Root);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromCString_BadScheme);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromCString_NoScheme);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromCString_ZeroComponents);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateFromBuffer);

    LONGBOW_RUN_TEST_CASE(Global, ccnxName_IsValid_True);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_IsValid_False);

    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Equals);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_LeftMostHashCode);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_HashCode_LeftMostHashCode);

    LONGBOW_RUN_TEST_CASE(Global, ccnxName_ToString);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_ToString_Root);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_ToString_NoPath);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_ToString_LCI);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Copy_Zero);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Copy_NonZero);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_CreateAndDestroy);

    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Trim);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Trim_MAXINT);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_StartsWith_True);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_StartsWith_FalseShorterPrefix);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_StartsWith_FalseLongerPrefix);

    LONGBOW_RUN_TEST_CASE(Global, ccnxName_Compare);
    LONGBOW_RUN_TEST_CASE(Global, ccnxName_ComposeNAME);

    LONGBOW_RUN_TEST_CASE(Global, ParseTest1);
    LONGBOW_RUN_TEST_CASE(Global, ParseTest2);

    LONGBOW_RUN_TEST_CASE(Global, MemoryProblem);
}

static size_t _longBowGlobal_Global_outstanding;

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    size_t allocationsLeaked = parcMemory_Outstanding() - _longBowGlobal_Global_outstanding;

    if (allocationsLeaked > 0) {
        printf("%s leaks memory by %zd allocations\n", longBowTestCase_GetName(testCase), allocationsLeaked);
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_CASE(Global, ccnxName_ComposeNAME)
{
    char *string = "lci:/a/b/c";

    CCNxName *basename = ccnxName_CreateFromCString("lci:/a/b");
    CCNxName *expected = ccnxName_CreateFromCString(string);

    CCNxName *actual = ccnxName_ComposeNAME(basename, "c");
    assertTrue(ccnxName_Equals(expected, actual), "Failed.");

    ccnxName_Release(&basename);
    ccnxName_Release(&expected);
    ccnxName_Release(&actual);
}

LONGBOW_TEST_CASE(Global, ccnxName_IsValid_True)
{
    char *string = "lci:/a/b/c";
    CCNxName *name = ccnxName_CreateFromCString(string);
    assertTrue(ccnxName_IsValid(name), "Expected %s to be a valid CCNxName.", string);
    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_IsValid_False)
{
    assertFalse(ccnxName_IsValid(NULL), "Expected NULL to be an invalid CCNxName.");
}

LONGBOW_TEST_CASE(Global, ccnxName_Equals)
{
    CCNxName *x = ccnxName_CreateFromCString("lci:/a/b/c");
    CCNxName *y = ccnxName_CreateFromCString("lci:/a/b/c");
    CCNxName *z = ccnxName_CreateFromCString("lci:/a/b/c");
    CCNxName *u1 = ccnxName_CreateFromCString("lci:/a/b");
    CCNxName *u2 = ccnxName_CreateFromCString("lci:/a/b/d");

    assertEqualsContract(ccnxName_Equals, x, y, z, u1, u2);

    ccnxName_Release(&x);
    ccnxName_Release(&y);
    ccnxName_Release(&z);
    ccnxName_Release(&u1);
    ccnxName_Release(&u2);
}

LONGBOW_TEST_CASE(Global, ccnxName_ToString_Root)
{
    const char *expected = "ccnx:/";

    CCNxName *name = ccnxName_CreateFromCString(expected);

    char *actual = ccnxName_ToString(name);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxName_Release(&name);

    name = ccnxName_CreateFromCString("ccnx:");
    actual = ccnxName_ToString(name);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_ToString_NoPath)
{
    const char *expected = "ccnx:/";

    CCNxName *name = ccnxName_CreateFromCString("ccnx:");

    char *actual = ccnxName_ToString(name);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_Trim)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/a/b/c");

    ccnxName_Trim(name, 1);

    const char *expected = "ccnx:/a/b";
    char *actual = ccnxName_ToString(name);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_Trim_MAXINT)
{
    CCNxName *name = ccnxName_CreateFromCString("ccnx:/a/b/c");

    ccnxName_Trim(name, INT_MAX);

    const char *expected = "ccnx:/";
    char *actual = ccnxName_ToString(name);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);

    parcMemory_Deallocate((void **) &actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_Copy_Zero)
{
    const char *uri = "ccnx:/"; // A Name with 1 zero-length segment

    CCNxName *name = ccnxName_CreateFromCString(uri);

    CCNxName *copy = ccnxName_Copy(name);
    assertNotNull(copy, "Expect non-null result.");

    char *expected = ccnxName_ToString(name);
    char *actual = ccnxName_ToString(copy);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    ccnxName_Release(&name);
    ccnxName_Release(&copy);
    parcMemory_Deallocate((void **) &expected);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, ccnxName_Copy_NonZero)
{
    const char *uri = "ccnx:/a/b/c/d/e";

    CCNxName *name = ccnxName_CreateFromCString(uri);

    CCNxName *copy = ccnxName_Copy(name);
    assertNotNull(copy, "Expect non-null result.");

    char *expected = ccnxName_ToString(name);
    char *actual = ccnxName_ToString(copy);

    assertTrue(strcmp(expected, actual) == 0, "Expected '%s', actual '%s'", expected, actual);

    ccnxName_Release(&name);
    ccnxName_Release(&copy);
    parcMemory_Deallocate((void **) &expected);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(Global, ccnxName_HashCode)
{
    const char *uriA = "lci:/a/b/c/d/e/";
    const char *uriB = "lci:/a/b/c/d/e/";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(uriB);

    PARCHashCode codeA = ccnxName_HashCode(nameA);
    PARCHashCode codeB = ccnxName_HashCode(nameB);

    // We know the hashcode of uriA is not zero
    assertTrue(codeA != 0, "Expected a non-zero hash code");

    assertTrue(codeA == codeB, "Expected %" PRIPARCHashCode " == %" PRIPARCHashCode, codeA, codeB);

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

LONGBOW_TEST_CASE(Global, ccnxName_HashCode_LeftMostHashCode)
{
    const char *uriA = "lci:/a/b/c/d/e/";
    const char *uriB = "lci:/a/b/c/d/e/";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(uriB);

    PARCHashCode codeA = ccnxName_HashCode(nameA);
    PARCHashCode codeB = ccnxName_HashCode(nameB);
    PARCHashCode leftMostCodeA = ccnxName_LeftMostHashCode(nameA, INT_MAX);
    PARCHashCode leftMostCodeB = ccnxName_LeftMostHashCode(nameB, INT_MAX);

    // We know the hashcode of uriA is not zero
    assertTrue(codeA != 0, "Expected a non-zero hash code");

    assertTrue(codeA == codeB, "Expected %" PRIPARCHashCode " == %" PRIPARCHashCode, codeA, codeB);
    assertTrue(codeA == leftMostCodeA, "Expected %" PRIPARCHashCode " == %" PRIPARCHashCode, codeA, leftMostCodeA);
    assertTrue(codeA == leftMostCodeB, "Expected %" PRIPARCHashCode " == %" PRIPARCHashCode, codeA, leftMostCodeB);

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

LONGBOW_TEST_CASE(Global, ccnxName_LeftMostHashCode)
{
    const char *uriA = "lci:/a/b/c/d/e/";
    const char *uriB = "lci:/a/b/c/d/e/";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(uriB);

    PARCHashCode codeA = ccnxName_LeftMostHashCode(nameA, 2);
    PARCHashCode codeB = ccnxName_LeftMostHashCode(nameB, 2);

    assertTrue(codeA == codeB, "Expected %" PRIPARCHashCode " == %" PRIPARCHashCode, codeA, codeB);

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateAndDestroy)
{
    CCNxName *name = ccnxName_Create();
    assertNotNull(name, "Expected non-null");
    ccnxName_Release(&name);
    assertNull(name, "Expected null");
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromCString)
{
    const char *uri = "lci:/CCN-Python-Test";

    CCNxName *name = ccnxName_CreateFromCString(uri);
    ccnxName_Display(name, 0);
    assertNotNull(name, "Expected non-null");

    size_t expected = 1;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual, "Expected %zd segments, actual %zd", expected, actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromCString_BadScheme)
{
    const char *uri = "abcd:/CCN-Python-Test/Echo";

    CCNxName *name = ccnxName_CreateFromCString(uri);
    assertNull(name, "Expected null");
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromCString_NoScheme)
{
    const char *uri = "/paravion";

    CCNxName *name = ccnxName_CreateFromCString(uri);
    assertNull(name, "Expected null");
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromCString_ZeroComponents)
{
    const char *uri = "lci:";

    CCNxName *name = ccnxName_CreateFromCString(uri);
    assertNotNull(name, "Expected non-null result from ccnxName_CreateFromCString");

    size_t expected = 0;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual, "Expected %zd segments, actual %zd", expected, actual);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromCString_Root)
{
    const char *uri = "lci:/";

    CCNxName *name = ccnxName_CreateFromCString(uri);
    assertNotNull(name, "Expected non-null");

    size_t expected = 1;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual, "Expected %zd segments, actual %zd", expected, actual);

    CCNxNameSegment *segment = ccnxName_GetSegment(name, 0);

    size_t segmentLength = ccnxNameSegment_Length(segment);
    assertTrue(segmentLength == 0, "Expected a zero length segment, actual %zd", segmentLength);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_CreateFromBuffer)
{
    PARCBuffer *buffer = parcBuffer_WrapCString("lci:/CCN-Python-Test");
    CCNxName *name = ccnxName_CreateFromBuffer(buffer);
    assertNotNull(name, "Expected non-null");

    size_t expected = 1;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual, "Expected %zd segments, actual %zd", expected, actual);

    ccnxName_Release(&name);
    parcBuffer_Release(&buffer);
}

LONGBOW_TEST_CASE(Global, ccnxName_ToString_LCI)
{
    const char *lci = "lci:/a/b";
    const char *expectedURI = "ccnx:/a/b";

    CCNxName *name = ccnxName_CreateFromCString(lci);

    size_t expected = 2;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual,
               "Expected %zd segments, actual %zd", expected, actual);

    char *string = ccnxName_ToString(name);
    assertTrue(strcmp(expectedURI, string) == 0,
               "Expected '%s' actual '%s'", expectedURI, string);
    parcMemory_Deallocate((void **) &string);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_ToString)
{
    const char *uri = "ccnx:/a/b";

    CCNxName *name = ccnxName_CreateFromCString(uri);

    size_t expected = 2;
    size_t actual = ccnxName_GetSegmentCount(name);

    assertTrue(expected == actual,
               "Expected %zd segments, actual %zd", expected, actual);

    char *string = ccnxName_ToString(name);
    assertTrue(strcmp(uri, string) == 0,
               "Expected '%s' actual '%s'", uri, string);
    parcMemory_Deallocate((void **) &string);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ccnxName_Compare)
{
    CCNxName *value = ccnxName_CreateFromCString("lci:/a/b/c");
    CCNxName *equal1 = ccnxName_CreateFromCString("lci:/a/b/c");

    CCNxName **equivalents = (CCNxName *[]) {
        equal1,
        NULL
    };

    CCNxName *lesser1 = ccnxName_CreateFromCString("lci:/a/b");
    CCNxName *lesser2 = ccnxName_CreateFromCString("lci:/a/b/b");
    CCNxName **lesser = (CCNxName *[]) {
        lesser1,
        lesser2,
        NULL
    };

    CCNxName *greater1 = ccnxName_CreateFromCString("lci:/a/b/d");
    CCNxName *greater2 = ccnxName_CreateFromCString("lci:/a/b/c/d");
    CCNxName **greater = (CCNxName *[]) {
        greater1,
        greater2,
        NULL
    };

    assertCompareToContract(ccnxName_Compare, value, equivalents, lesser, greater);

    for (int i = 0; lesser[i] != NULL; i++) {
        ccnxName_Release(&lesser[i]);
    }
    for (int i = 0; greater[i] != NULL; i++) {
        ccnxName_Release(&greater[i]);
    }
    for (int i = 0; equivalents[i] != NULL; i++) {
        ccnxName_Release(&equivalents[i]);
    }
    ccnxName_Release(&value);
}

LONGBOW_TEST_CASE(Global, ccnxName_StartsWith_True)
{
    const char *uriA = "lci:/a/b/c/d/e/";
    const char *uriB = "lci:/a/b/c/d/e/";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(uriB);

    bool actual = ccnxName_StartsWith(nameA, nameA);

    assertTrue(actual, "Expected true");

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

LONGBOW_TEST_CASE(Global, ccnxName_StartsWith_FalseShorterPrefix)
{
    const char *uriA = "lci:/a/b/c/d/e";
    const char *prefix = "lci:/a/b/d";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(prefix);

    bool actual = ccnxName_StartsWith(nameA, nameB);

    assertFalse(actual, "Expected false");

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

LONGBOW_TEST_CASE(Global, ccnxName_StartsWith_FalseLongerPrefix)
{
    const char *uriA = "lci:/a/b/c/d/e";
    const char *prefix = "lci:/a/b/c/d/e/f";

    CCNxName *nameA = ccnxName_CreateFromCString(uriA);
    CCNxName *nameB = ccnxName_CreateFromCString(prefix);

    bool actual = ccnxName_StartsWith(nameA, nameB);

    assertFalse(actual, "Expected false");

    ccnxName_Release(&nameA);
    ccnxName_Release(&nameB);
}

static CCNxNameSegment *
createSegment(PARCBuffer *buffer, size_t start, size_t end)
{
    parcBuffer_SetPosition(buffer, start);
    PARCBuffer *slice = parcBuffer_Slice(buffer);
    parcBuffer_SetLimit(slice, end);

    CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, slice);
    parcBuffer_Release(&slice);

    return segment;
}

LONGBOW_TEST_CASE(Global, MemoryProblem)
{
    char memory[] = "abcdefghijklmnopqrstuvwxyz";
    PARCBuffer *buffer = parcBuffer_Wrap(memory, sizeof(memory), 0, sizeof(memory));

    CCNxName *name = ccnxName_Create();

    CCNxNameSegment *segment1 = createSegment(buffer, 2, 4);    // "cd"
    ccnxName_Append(name, segment1);

    CCNxNameSegment *segment2 = createSegment(buffer, 10, 14);    // "klmn"
    ccnxName_Append(name, segment2);

    CCNxName *name2 = ccnxName_Acquire(name);

    parcBuffer_Release(&buffer);
    ccnxName_Release(&name2);

    ccnxName_Release(&name);
    ccnxNameSegment_Release(&segment1);
    ccnxNameSegment_Release(&segment2);
}

LONGBOW_TEST_CASE(Global, ParseTest1)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/" CCNxNameLabel_Name "=foot/3=toe/4=nail");
    assertNotNull(name, "Expected non-null value from ccnxName_CreateFromCString");

    ccnxName_Display(name, 0);

    ccnxName_Release(&name);
}

LONGBOW_TEST_CASE(Global, ParseTest2)
{
    CCNxName *a = ccnxName_CreateFromCString("lci:/a/b/c");
    CCNxName *b = ccnxName_CreateFromCString("lci:/Name=a/Name=b/Name=c");
    assertTrue(ccnxName_Equals(a, b), "Expected to be equal");
    ccnxName_Release(&a);
    ccnxName_Release(&b);

    char *expected = "ccnx:/test/Name=MiISAg%3D%3D";
    CCNxName *name = ccnxName_CreateFromCString(expected);
    assertNotNull(name, "Expected non-null value from ccnxName_CreateFromCString");
    char *actual = ccnxName_ToString(name);
    printf("%s\n", actual);
    assertTrue(strcmp(expected, actual) == 0, "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate(&actual);

    ccnxName_Display(name, 0);

    ccnxName_Release(&name);
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

LONGBOW_TEST_FIXTURE(Specialization)
{
    LONGBOW_RUN_TEST_CASE(Specialization, ccnxName_Prefix);
    LONGBOW_RUN_TEST_CASE(Specialization, ccnxName_Prefix_Excess);
    LONGBOW_RUN_TEST_CASE(Specialization, ccnxName_Prefix_0);
}

LONGBOW_TEST_FIXTURE_SETUP(Specialization)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Specialization)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Specialization, ccnxName_Prefix)
{
    CCNxName *a = ccnxName_CreateFromCString("ccnx:/a/b/c");
    CCNxName *expected = ccnxName_CreateFromCString("ccnx:/a");

    CCNxName *actual = ccnxName_CreatePrefix(a, 1);

    assertTrue(ccnxName_Equals(expected, actual), "Mismatched results.");

    ccnxName_Release(&a);
    ccnxName_Release(&expected);
    ccnxName_Release(&actual);
}

LONGBOW_TEST_CASE(Specialization, ccnxName_Prefix_Excess)
{
    CCNxName *a = ccnxName_CreateFromCString("ccnx:/a/b/c");
    CCNxName *expected = ccnxName_CreateFromCString("ccnx:/a/b/c");

    CCNxName *actual = ccnxName_CreatePrefix(a, 100);

    assertTrue(ccnxName_Equals(expected, actual), "Mismatched results.");

    ccnxName_Release(&a);
    ccnxName_Release(&expected);
    ccnxName_Release(&actual);
}

LONGBOW_TEST_CASE(Specialization, ccnxName_Prefix_0)
{
    CCNxName *a = ccnxName_CreateFromCString("ccnx:/a/b/c");
    CCNxName *expected = ccnxName_CreateFromCString("ccnx:");

    CCNxName *actual = ccnxName_CreatePrefix(a, 0);

    assertTrue(ccnxName_Equals(expected, actual), "Mismatched results.");

    ccnxName_Release(&a);
    ccnxName_Release(&expected);
    ccnxName_Release(&actual);
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, ccnxName_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, ccnxName_Create)
{
    PARCBuffer *value = parcBuffer_WrapCString("Hello");

    for (int i = 0; i < 10000; i++) {
        CCNxName *name = ccnxName_Create();
        CCNxNameSegment *segment = ccnxNameSegment_CreateTypeValue(CCNxNameLabelType_NAME, value);
        for (int j = 0; j < 1000; j++) {
            ccnxName_Append(name, segment);
        }
        ccnxNameSegment_Release(&segment);
        ccnxName_Release(&name);
    }
    parcBuffer_Release(&value);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_Name);
    int exitStatus = longBowMain(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
