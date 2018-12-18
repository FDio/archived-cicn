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

#include "../parc_URIPath.c"
#include <stdint.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_URI.h>

#include "_test_parc_URI.h"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parcURIPath)
{
    LONGBOW_RUN_TEST_FIXTURE(parcURIPath);
}

LONGBOW_TEST_RUNNER_SETUP(parcURIPath)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parcURIPath)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(parcURIPath)
{
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Acquire);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Parse);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Parse_WithQuery);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Release);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Count);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Append);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_ToString);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Length);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Trim);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Copy);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Equals);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Compare_Identity);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Compare_Equal);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Compare_Unequal);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_Compose);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_StartsWith);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_StartsWith_Equal);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_StartsWith_Fail);
    LONGBOW_RUN_TEST_CASE(parcURIPath, parcURIPath_BuildString);
}

LONGBOW_TEST_FIXTURE_SETUP(parcURIPath)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(parcURIPath)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Acquire)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "//////" URI_PATH_SEGMENT, &pointer);
    PARCURIPath *handle = parcURIPath_Acquire(path);

    assertTrue(parcURIPath_Equals(path, handle), "URI paths should be equal: %s - %s", parcURIPath_ToString(path), parcURIPath_ToString(handle));

    parcURIPath_Release(&path);
    parcURIPath_Release(&handle);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Parse)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "//////" URI_PATH_SEGMENT, &pointer);
    assertNotNull(path, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    char *actualPath = parcURIPath_ToString(path);

    char *expectedPath = URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    assertTrue(strcmp(expectedPath, actualPath) == 0, "Expected %s actual %s", expectedPath, actualPath);

    parcMemory_Deallocate((void **) &actualPath);
    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Release)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);
    assertNotNull(path, "Expected non-null result.");
    parcURIPath_Release(&path);
    assertNull(path, "Expected destroy to null the pointer.");
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Parse_WithQuery)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    assertNotNull(path, "Expected non-null result.");
    assertTrue(*pointer == '?', "Expected pointer to point to the terminating character.");

    char *actualPath = parcURIPath_ToString(path);
    char *expectedPath = URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    assertTrue(strcmp(expectedPath, actualPath) == 0, "Expected %s actual %s", expectedPath, actualPath);
    parcMemory_Deallocate((void **) &actualPath);
    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Equals)
{
    const char *pointer;
    PARCURIPath *x = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *y = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *z = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *u1 = parcURIPath_Parse("/" URI_PATH_SEGMENT "a/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *u2 = parcURIPath_Parse("/" URI_PATH_SEGMENT "?query", &pointer);

    parcObjectTesting_AssertEqualsFunction(parcURIPath_Equals, x, y, z, u1, u2);

    parcURIPath_Release(&x);
    parcURIPath_Release(&y);
    parcURIPath_Release(&z);
    parcURIPath_Release(&u1);
    parcURIPath_Release(&u2);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Copy)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *copy = parcURIPath_Copy(path);

    assertTrue(copy != path, "Expected distinct instances of the path.");

    int comparison = parcURIPath_Compare(path, copy);

    assertTrue(comparison == 0, "Expected the copy to compare equal to the original.");

    parcURIPath_Release(&path);
    parcURIPath_Release(&copy);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_StartsWith)
{
    const char *pointer;
    PARCURIPath *base = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *prefix = parcURIPath_Parse("/" URI_PATH_SEGMENT "?query", &pointer);

    bool actual = parcURIPath_StartsWith(base, prefix);

    assertTrue(actual, "Expected true, actual false.");

    parcURIPath_Release(&prefix);
    parcURIPath_Release(&base);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_StartsWith_Equal)
{
    const char *pointer;
    PARCURIPath *base = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *prefix = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);

    bool actual = parcURIPath_StartsWith(base, prefix);

    assertTrue(actual, "Expected true, actual false.");

    parcURIPath_Release(&prefix);
    parcURIPath_Release(&base);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_StartsWith_Fail)
{
    const char *pointer;
    PARCURIPath *base = parcURIPath_Parse("/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *prefix1 = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *prefix2 = parcURIPath_Parse("/" URI_PATH_SEGMENT "A?query", &pointer);

    assertFalse(parcURIPath_StartsWith(base, prefix1),
                "Expected false, actual true");

    assertFalse(parcURIPath_StartsWith(base, prefix2),
                "Expected false, actual true");

    parcURIPath_Release(&prefix1);
    parcURIPath_Release(&prefix2);
    parcURIPath_Release(&base);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Compose)
{
    const char *pointer;
    PARCURIPath *base = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *expected = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT  "?query", &pointer);

    PARCURISegment *a = parcURISegment_Parse(URI_PATH_SEGMENT, &pointer);
    PARCURIPath *actual = parcURIPath_Compose(base, a, a, NULL);
    parcURISegment_Release(&a);

    char *actualString = parcURIPath_ToString(actual);
    char *expectedString = parcURIPath_ToString(expected);
    assertTrue(parcURIPath_Compare(expected, actual) == 0, "Expected '%s' actual '%s'\n", expectedString, actualString);

    parcMemory_Deallocate((void **) &actualString);
    parcMemory_Deallocate((void **) &expectedString);

    parcURIPath_Release(&actual);

    parcURIPath_Release(&expected);
    parcURIPath_Release(&base);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Compare_Identity)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);

    PARCURIPath *equivalents[] = {
        path,
        parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query",&pointer),
        NULL,
    };
    PARCURIPath *lessers[] = {
        parcURIPath_Parse("/" URI_PATH_SEGMENT "?query", &pointer),
        NULL,
    };
    PARCURIPath *greaters[] = {
        parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer),
        parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "AAA?query",                   &pointer),
        NULL,
    };

    parcObjectTesting_AssertCompareTo(parcURIPath_Compare, path, equivalents, lessers, greaters);

    for (int i = 0; equivalents[i] != NULL; i++) {
        parcURIPath_Release(&equivalents[i]);
    }
    for (int i = 0; lessers[i] != NULL; i++) {
        parcURIPath_Release(&lessers[i]);
    }
    for (int i = 0; greaters[i] != NULL; i++) {
        parcURIPath_Release(&greaters[i]);
    }
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Compare_Equal)
{
    const char *pointer;
    PARCURIPath *pathA = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *pathB = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);

    int comparison = parcURIPath_Compare(pathA, pathB);

    assertTrue(comparison == 0, "Expected 0: equal paths to compare equal. Actual %d", comparison);
    parcURIPath_Release(&pathA);
    parcURIPath_Release(&pathB);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Compare_Unequal)
{
    const char *pointer;
    PARCURIPath *pathA = parcURIPath_Parse("/" URI_PATH_SEGMENT "?query", &pointer);
    PARCURIPath *pathB = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?query", &pointer);

    int comparison = parcURIPath_Compare(pathA, pathB);

    assertTrue(comparison < 0, "Expected  < 0: path A is less than path B. Actual %d", comparison);
    parcURIPath_Release(&pathA);
    parcURIPath_Release(&pathB);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Count)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);
    assertNotNull(path, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    size_t actual = parcURIPath_Count(path);
    assertTrue(3 == actual, "Expected %d actual %zd", 3, actual);

    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_ToString)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);
    assertNotNull(path, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    char *actualString = parcURIPath_ToString(path);

    char *expectedString = URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    assertTrue(strcmp(expectedString, actualString) == 0, "Expected %s actual %s", expectedString, actualString);

    parcMemory_Deallocate((void **) &actualString);
    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Length)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);
    assertNotNull(path, "Expected non-null result.");
    assertTrue(*pointer == 0, "Expected pointer to point to the null terminating byte.");

    size_t actualCount = parcURIPath_Length(path);

    size_t expectedCount = 79;
    assertTrue(expectedCount == actualCount, "Expected %zd actual %zd", expectedCount, actualCount);

    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Append)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);

    PARCURISegment *segment = parcURISegment_Parse(URI_PATH_SEGMENT, &pointer);

    PARCURIPath *result = parcURIPath_Append(path, segment);
    assertTrue(result == path, "Expected %p, actual %p", (void *) path, (void *) result);

    size_t actualCount = parcURIPath_Count(path);
    assertTrue(3 == actualCount, "Expected 3, actual %zd", actualCount);

    char *actualPath = parcURIPath_ToString(path);
    char *expectedPath = URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    assertTrue(strcmp(expectedPath, actualPath) == 0, "Expected %s actual %s", expectedPath, actualPath);

    parcMemory_Deallocate((void **) &actualPath);
    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_Trim)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT, &pointer);

    parcURIPath_Trim(path, 2);
    size_t actualCount = parcURIPath_Count(path);
    assertTrue(2 == actualCount, "Expected 2, actual %zd", actualCount);

    parcURIPath_Release(&path);
}

LONGBOW_TEST_CASE(parcURIPath, parcURIPath_BuildString)
{
    const char *pointer;
    PARCURIPath *path = parcURIPath_Parse("/" URI_PATH_SEGMENT, &pointer);
    PARCBufferComposer *target = parcBufferComposer_Create();

    PARCBufferComposer *string = parcBufferComposer_Create();
    parcBufferComposer_PutString(string, URI_PATH_SEGMENT);

    PARCBuffer *b1 = parcBufferComposer_ProduceBuffer(string);
    char *string1 = parcBuffer_ToString(b1);
    parcBuffer_Release(&b1);

    parcURIPath_BuildString(path, target);
    PARCBuffer *b2 = parcBufferComposer_ProduceBuffer(target);
    char *string2 = parcBuffer_ToString(b2);
    parcBuffer_Release(&b2);

    assertTrue(strncmp(string1, string2, strlen(string1)) == 0, "String representations of the paths should be equal: %s - %s", string1, string2);

    parcMemory_Deallocate((void **) &string1);
    parcMemory_Deallocate((void **) &string2);

    parcBufferComposer_Release(&string);
    parcBufferComposer_Release(&target);
    parcURIPath_Release(&path);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcURIPath);
    int status = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(status);
}
