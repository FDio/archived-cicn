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
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_ObjectTesting.c"
#include <LongBow/unit-test.h>
#include <LongBow/debugging.h>

#include <inttypes.h>
#include <stdio.h>
#include <inttypes.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

LONGBOW_TEST_RUNNER(test_parc_ObjectTesting)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(CompareTo);
    LONGBOW_RUN_TEST_FIXTURE(Equals);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Conformance);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(test_parc_ObjectTesting)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(test_parc_ObjectTesting)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

static bool
_equalsFunction(const void *x, const void *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    return (strcmp(x, y) == 0);
}

static bool
_equalsFunction_NotReflexive(const void *x, const void *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }
    if (x < y) {
        return true;
    } else {
        return false;
    }
}

LONGBOW_TEST_FIXTURE(Equals)
{
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEquals);

    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl);
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXYsame);
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXZsame);
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXnotequalZ);
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXnotequalY);
    LONGBOW_RUN_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailNotSymmetric);
}

static uint32_t _longBowGlobal_Global_outstanding;

LONGBOW_TEST_FIXTURE_SETUP(Equals)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Equals)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    if (parcMemoryTesting_ExpectedOutstanding(_longBowGlobal_Global_outstanding, longBowTestCase_GetName(testCase)) == false) {
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }

    return result;
}

LONGBOW_TEST_CASE(Equals, parcObjectTesting_AssertEquals)
{
    PARCBuffer *x = parcBuffer_Allocate(sizeof(uint64_t));
    PARCBuffer *y = parcBuffer_Allocate(sizeof(uint64_t));
    PARCBuffer *z = parcBuffer_Allocate(sizeof(uint64_t));
    PARCBuffer *u1 = parcBuffer_Allocate(sizeof(uint64_t));
    PARCBuffer *u2 = parcBuffer_Allocate(sizeof(uint64_t));
    parcBuffer_Flip(parcBuffer_PutUint64(x, 1));
    parcBuffer_Flip(parcBuffer_PutUint64(y, 1));
    parcBuffer_Flip(parcBuffer_PutUint64(z, 1));
    parcBuffer_Flip(parcBuffer_PutUint64(u1, 2));
    parcBuffer_Flip(parcBuffer_PutUint64(u2, 3));

    parcObjectTesting_AssertEquals(x, y, z, u1, u2, NULL);
    parcBuffer_Release(&x);
    parcBuffer_Release(&y);
    parcBuffer_Release(&z);
    parcBuffer_Release(&u1);
    parcBuffer_Release(&u2);
}

LONGBOW_TEST_CASE(Equals, parcObjectTesting_AssertEqualsFunctionImpl)
{
    void *x = strdup("1");
    void *y = strdup("1");
    void *z = strdup("1");
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_CASE_EXPECTS(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXYsame, .event = &LongBowAssertEvent)
{
    void *x = strdup("1");
    void *y = x;
    void *z = strdup("1");
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_CASE_EXPECTS(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXZsame, .event = &LongBowAssertEvent)
{
    void *x = strdup("1");
    void *y = strdup("1");
    void *z = x;
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_CASE_EXPECTS(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXnotequalY, .event = &LongBowAssertEvent)
{
    void *x = strdup("1");
    void *y = strdup("xyzzy");
    void *z = strdup("1");
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_CASE_EXPECTS(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailXnotequalZ, .event = &LongBowAssertEvent)
{
    void *x = strdup("1");
    void *y = strdup("1");
    void *z = strdup("xyzzy");
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_CASE_EXPECTS(Equals, parcObjectTesting_AssertEqualsFunctionImpl_FailNotSymmetric, .event = &LongBowAssertEvent)
{
    void *x = strdup("1");
    void *y = strdup("1");
    void *z = strdup("1");
    void *u1 = "a";
    void *u2 = "b";

    parcObjectTesting_AssertEqualsFunctionImpl(_equalsFunction_NotReflexive, x, y, z, u1, u2, NULL);
    free(x);
    free(y);
    free(z);
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcObjectTesting_AssertAcquire);
}

LONGBOW_TEST_FIXTURE_SETUP(AcquireRelease)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(AcquireRelease)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    if (parcMemoryTesting_ExpectedOutstanding(_longBowGlobal_Global_outstanding, longBowTestCase_GetName(testCase)) == false) {
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }

    return result;
}

LONGBOW_TEST_CASE(AcquireRelease, parcObjectTesting_AssertAcquire)
{
    PARCBuffer *buffer = parcBuffer_Allocate(10);
    parcObjectTesting_AssertAcquireReleaseImpl(buffer);
    parcBuffer_Release(&buffer);
}


LONGBOW_TEST_FIXTURE(CompareTo)
{
    LONGBOW_RUN_TEST_CASE(CompareTo, parcObjectTesting_AssertCompareTo);
}

LONGBOW_TEST_FIXTURE_SETUP(CompareTo)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CompareTo)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    if (parcMemoryTesting_ExpectedOutstanding(_longBowGlobal_Global_outstanding, longBowTestCase_GetName(testCase)) == false) {
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }

    return result;
}

LONGBOW_TEST_CASE(CompareTo, parcObjectTesting_AssertCompareTo)
{
    PARCBuffer *x = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);
    PARCBuffer *y = parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10, 0, 10);

    PARCBuffer *equivalent[] = {
        x,
        y,
        NULL
    };
    PARCBuffer *lesser[] = {
        parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8 }, 10, 0, 10),
        parcBuffer_Wrap((uint8_t [9])  { 0, 1, 2, 3, 4, 5, 5, 7, 8,   }, 9,  0, 9),
        NULL
    };
    PARCBuffer *greater[] = {
        parcBuffer_Wrap((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10, 0, 10),
        parcBuffer_Wrap((uint8_t [11]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 11, 0, 11),
        NULL
    };

    parcObjectTesting_AssertCompareTo(parcBuffer_Compare, x, equivalent, lesser, greater);

    parcBuffer_Release(&x);
    parcBuffer_Release(&y);

    for (int i = 0; lesser[i] != NULL; i++) {
        parcBuffer_Release(&lesser[i]);
    }
    for (int i = 0; greater[i] != NULL; i++) {
        parcBuffer_Release(&greater[i]);
    }
}

LONGBOW_TEST_FIXTURE(Static)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    _longBowGlobal_Global_outstanding = parcMemory_Outstanding();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    size_t allocationsLeaked = parcMemory_Outstanding() - _longBowGlobal_Global_outstanding;

    if (allocationsLeaked) {
        printf("%s leaks memory by %zd allocations\n", longBowTestCase_GetName(testCase), allocationsLeaked);
        parcSafeMemory_ReportAllocation(STDERR_FILENO);
        result = LONGBOW_STATUS_MEMORYLEAK;
    }
    return result;
}

LONGBOW_TEST_FIXTURE(Conformance)
{
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_succeed);

    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_non_object);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_equals);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_compare);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_copy_junk);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_copy_same);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_hash);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_toString);
    LONGBOW_RUN_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_toJSON);
}


typedef struct {
    PARCBuffer *inst1;
    PARCBuffer *inst2;
    PARCBuffer *inst3;
    PARCBuffer *lesser;
    PARCBuffer *greater;
    PARCObjectDescriptor *descriptor;
} TestData_t;

static const PARCObjectDescriptor *
_copyDescriptor(const PARCObjectDescriptor *orig)
{
    return parcObjectDescriptor_Create("Name",
                                       orig->objectSize,
                                       orig->objectAlignment,
                                       orig->isLockable,
                                       orig->destructor,
                                       orig->release, orig->copy,
                                       orig->toString, orig->equals,
                                       orig->compare, orig->hashCode,
                                       orig->toJSON, orig->display,
                                       orig->super, orig->typeState);
}

LONGBOW_TEST_FIXTURE_SETUP(Conformance)
{
    TestData_t *data = parcMemory_AllocateAndClear(sizeof(TestData_t));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData_t));

    data->inst1 = parcBuffer_Flip(parcBuffer_CreateFromArray((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10));
    data->inst2 = parcBuffer_Flip(parcBuffer_CreateFromArray((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10));
    data->inst3 = parcBuffer_Flip(parcBuffer_CreateFromArray((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10));
    data->lesser = parcBuffer_Flip(parcBuffer_CreateFromArray((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 8 }, 10));
    data->greater = parcBuffer_Flip(parcBuffer_CreateFromArray((uint8_t [10]) { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10 }, 10));

    PARCObjectDescriptor *d = (PARCObjectDescriptor *) parcObject_GetDescriptor(data->inst1);
    data->descriptor = (PARCObjectDescriptor *) _copyDescriptor(d);

    longBowTestCase_SetClipBoardData(testCase, data);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Conformance)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    parcBuffer_Release(&data->inst1);
    parcBuffer_Release(&data->inst2);
    parcBuffer_Release(&data->inst3);
    parcBuffer_Release(&data->lesser);
    parcBuffer_Release(&data->greater);

    parcObjectDescriptor_Destroy(&data->descriptor);

    parcMemory_Deallocate(&data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Conformance, parcObjectTesting_AssertObjectConformance_succeed)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->lesser, data->greater);
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_non_object, .event = &LongBowAssertEvent)
{
    char *inst1 = "not an object";
    char *inst2 = "not an object";
    char *inst3 = "not an object";

    parcObjectTesting_AssertObjectConformance(inst1, inst2, inst3, NULL, NULL);
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_equals, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);
    PARCBuffer *inst1 = data->inst1;
    PARCBuffer *inst2 = inst1;
    PARCBuffer *inst3 = inst1;

    parcObjectTesting_AssertObjectConformance(inst1, inst2, inst3, data->lesser, data->greater);
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_compare, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

__attribute__ ((noinline))
static PARCObject *
badCopyJunk(const PARCObject *instance)
{
    return parcBuffer_Allocate(10);
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_copy_junk, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    data->descriptor->copy = badCopyJunk;
    parcObject_SetDescriptor(data->inst1, data->descriptor);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

static PARCObject *
badCopySame(const PARCObject *instance)
{
    return parcObject_Acquire(instance);
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_copy_same, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    data->descriptor->copy = badCopySame;
    parcObject_SetDescriptor(data->inst1, data->descriptor);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

static PARCHashCode
badHash(const PARCObject *instance)
{
    return (PARCHashCode) instance;
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_hash, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    data->descriptor->hashCode = badHash;
    parcObject_SetDescriptor(data->inst1, data->descriptor);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

static PARCJSON *
badToJSON(const PARCObject *instance)
{
    return NULL;
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_toJSON, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    data->descriptor->toJSON = badToJSON;
    parcObject_SetDescriptor(data->inst1, data->descriptor);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

static char *
badToString(const PARCObject *instance)
{
    return NULL;
}

LONGBOW_TEST_CASE_EXPECTS(Conformance, parcObjectTesting_AssertObjectConformance_fail_object_toString, .event = &LongBowAssertEvent)
{
    TestData_t *data = longBowTestCase_GetClipBoardData(testCase);

    data->descriptor->toString = badToString;
    parcObject_SetDescriptor(data->inst1, data->descriptor);

    parcObjectTesting_AssertObjectConformance(data->inst1, data->inst2, data->inst3, data->greater, data->lesser);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(test_parc_ObjectTesting);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
