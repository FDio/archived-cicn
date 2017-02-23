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
#include "../parc_Object.c"

#include <inttypes.h>
#include <sys/time.h>

#include <LongBow/unit-test.h>

#include <parc/testing/parc_ObjectTesting.h>
#include <parc/testing/parc_MemoryTesting.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_StdlibMemory.h>

typedef struct {
    int calledCount;
    int val;
} _dummy_object;

typedef _dummy_object _DummyObject;

static void
_dummy_Destroy(_DummyObject **obj __attribute__((unused)))
{
}

static _DummyObject *
_dummy_Copy(const _DummyObject *obj);


static bool
_dummy_Equals(const _DummyObject *x, const _DummyObject *y)
{
    const _DummyObject *dummy1 = x;
    const _DummyObject *dummy2 = y;
    return (dummy1->calledCount == dummy2->calledCount);
}

static int
_dummy_Compare(const _DummyObject *x, const _DummyObject *y)
{
    const _DummyObject *dummy1 = x;
    const _DummyObject *dummy2 = y;
    if (dummy1->calledCount == dummy2->calledCount) {
        return 0;
    } else if (dummy1->calledCount < dummy2->calledCount) {
        return -1;
    } else {
        return 1;
    }
}

static uint32_t
_dummy_HashCode(const _DummyObject *obj)
{
    _DummyObject *dummy = (_DummyObject *) obj;
    dummy->calledCount++;
    return 1337;
}

static char *
_dummy_ToString(const _DummyObject *x __attribute__((unused)))
{
    char *str = (char *) parcMemory_Allocate(6);
    char *test = "dummy";
    sprintf(str, "%s", test);
    return str;
}

static PARCJSON *
_dummy_ToJSON(const _DummyObject *x __attribute__((unused)))
{
    PARCJSON *json = parcJSON_ParseString("{ \"type\" : \"dummy\"  }");
    return json;
}

parcObject_Override(_DummyObject, PARCObject,
                    .destroy = (PARCObjectDestroy *) _dummy_Destroy,
                    .copy = (PARCObjectCopy *) _dummy_Copy,
                    .toString = (PARCObjectToString *) _dummy_ToString,
                    .equals = (PARCObjectEquals *) _dummy_Equals,
                    .compare = (PARCObjectCompare *) _dummy_Compare,
                    .hashCode = (PARCObjectHashCode *) _dummy_HashCode,
                    .toJSON = (PARCObjectToJSON *) _dummy_ToJSON);

static _DummyObject *
_dummy_Copy(const _DummyObject *obj)
{
    _DummyObject *newDummy = parcObject_CreateInstance(_DummyObject);
    const _DummyObject *dummy = obj;
    newDummy->calledCount = dummy->calledCount;
    return newDummy;
}

typedef _dummy_object _DummyObjectNoHash;
parcObject_ExtendPARCObject(_DummyObjectNoHash,
                            _dummy_Destroy,
                            _dummy_Copy,
                            _dummy_ToString,
                            _dummy_Equals,
                            _dummy_Compare,
                            NULL,
                            _dummy_ToJSON);

static bool
_meta_destructor_true(PARCObject **objPtr)
{
    return true;
}

static bool
_meta_destructor_false(PARCObject **objPtr)
{
    (*objPtr) = NULL;
    return false;
}

static PARCObject *
_meta_copy(const PARCObject *ptr)
{
    _DummyObject *d = parcMemory_AllocateAndClear(sizeof(_DummyObject));
    _DummyObject *xx = (_DummyObject *) ptr;
    d->val = xx->val;
    return d;
}

static bool
_meta_equals(const PARCObject *x, const PARCObject *y)
{
    _DummyObject *xx = (_DummyObject *) x;
    _DummyObject *yy = (_DummyObject *) y;
    return (xx->val == yy->val);
}

static int
_meta_compare(const PARCObject *x, const PARCObject *y)
{
    _DummyObject *xx = (_DummyObject *) x;
    _DummyObject *yy = (_DummyObject *) y;

    if (xx->val == yy->val) {
        return 0;
    } else if (xx->val < yy->val) {
        return -1;
    } else {
        return 1;
    }
}

static PARCHashCode
_meta_hashCode(const PARCObject *ptr)
{
    _DummyObject *xx = (_DummyObject *) ptr;
    return xx->val;
}

static char *
_meta_toString(const PARCObject *ptr)
{
    _DummyObject *xx = (_DummyObject *) ptr;

    char *result;
    parcMemory_MemAlign((void **) &result, sizeof(void *), sizeof(char));
    assertNotNull(result, "parcMemory_Allocate returned NULL");

    char *p = result;
    sprintf(result, "%d", xx->val);

    return p;
}

static PARCJSON *
_meta_toJson(const PARCObject *ptr)
{
    _DummyObject *xx = (_DummyObject *) ptr;

    PARCJSON *json = parcJSON_Create();
    parcJSON_AddInteger(json, "value", xx->val);

    return json;
}

static const PARCMemoryInterface *_originalMemoryProvider;

LONGBOW_TEST_RUNNER(parcObject)
{
    LONGBOW_RUN_TEST_FIXTURE(Performance);
    LONGBOW_RUN_TEST_FIXTURE(PARCObjectDescriptor);
    LONGBOW_RUN_TEST_FIXTURE(Static);
    LONGBOW_RUN_TEST_FIXTURE(StaticObjects);
    LONGBOW_RUN_TEST_FIXTURE(Meta);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Subclasses);
    LONGBOW_RUN_TEST_FIXTURE(Fail);
    LONGBOW_RUN_TEST_FIXTURE(Locking);
    LONGBOW_RUN_TEST_FIXTURE(WaitNotify);
    LONGBOW_RUN_TEST_FIXTURE(Synchronization);
}

LONGBOW_TEST_RUNNER_SETUP(parcObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parcObject)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _objectHeaderIsValid);
    LONGBOW_RUN_TEST_CASE(Static, _parcObject_PrefixLength);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    parcMemory_SetInterface(_originalMemoryProvider);
    return LONGBOW_STATUS_SUCCEEDED;
}

struct timeval _testObject;

parcObject_Override(_testObject, PARCObject);

LONGBOW_TEST_CASE(Static, _objectHeaderIsValid)
{
    PARCObject *object = parcObject_CreateInstanceImpl(&_testObject_Descriptor);

    _PARCObjectHeader *header = _parcObject_Header(object);

    assertTrue(_parcObjectHeader_IsValid(header, object), "Expected _parcObject_HeaderHeaderIsValid to be valid");

    parcObject_Release(&object);
}

LONGBOW_TEST_CASE(Static, _parcObject_PrefixLength)
{
    // Test that the result is a multiple of the alignment value and greater than the size of _PARCObjectHeader.

    // Compute the power of 2 value of sizeof(void *)
    unsigned int v = sizeof(void *);
    unsigned int r = 0; // r will be lg(v)

    while (v >>= 1) {
        r++;
    }
    PARCObjectDescriptor descriptor;

    for (int i = r; i < 20; i++) {
        descriptor.objectAlignment = 1 << i;
        size_t actual = _parcObject_PrefixLength(&descriptor);
        assertTrue((actual & (descriptor.objectAlignment - 1)) == 0,
                   "Alignment needs to be a multiple of %u", descriptor.objectAlignment);
    }
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcObject_Acquire);
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcObject_Release);
//    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcObject_Acquire_Invalid);
}

LONGBOW_TEST_FIXTURE_SETUP(AcquireRelease)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(AcquireRelease)
{
    parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    parcMemory_SetInterface(_originalMemoryProvider);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(AcquireRelease, parcObject_Acquire)
{
    struct timeval *expected = parcObject_CreateInstanceImpl(&_testObject_Descriptor);

    parcObjectTesting_AssertAcquireReleaseContract(parcObject_Acquire, expected);
    parcObject_Release((void **) &expected);
}

LONGBOW_TEST_CASE(AcquireRelease, parcObject_Release)
{
    struct timeval *time = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    PARCReferenceCount count = parcObject_Release((PARCObject **) &time);
    assertTrue(count == 0, "Expected reference count to be zero");
    assertTrue(time == 0, "Expected memory pointer to be NULL after destroy.");
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Create);
//    LONGBOW_RUN_TEST_CASE(Global, parcObject_CreateAndClear);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_IsValid);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_IsValid_NotValid);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_IsInstanceOf);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Copy_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Copy);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Compare_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Compare_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Compare);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Equals_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Equals_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_HashCode_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_HashCode_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_HashCode);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToString_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToString_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToString);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToJSON_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToJSON_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_ToJSON);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_GetReferenceCount);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Display_Default);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Display_NoOverride);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_Display);
    LONGBOW_RUN_TEST_CASE(Global, parcObject_GetDescriptor);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);

    parcMemory_SetInterface(_originalMemoryProvider);

    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcObject_Release)
{
    struct timeval *time = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    PARCReferenceCount count = parcObject_Release((PARCObject **) &time);
    assertTrue(count == 0, "Expected reference count to be zero");
    assertTrue(time == 0, "Expected memory pointer to be NULL after destroy.");
}

LONGBOW_TEST_CASE(Global, parcObject_Create)
{
    struct timeval *time = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    PARCReferenceCount count = parcObject_Release((PARCObject **) &time);
    assertTrue(count == 0, "Expected reference count to be zero");
    assertTrue(time == 0, "Expected memory pointer to be NULL after destroy.");
}

//LONGBOW_TEST_CASE(Global, parcObject_CreateAndClear)
//{
//    struct timeval *time = parcObject_CreateAndClear(struct timeval);
//    parcObject_AssertValid(time);
//
//    time->tv_sec = 1;
//    time->tv_usec = 2;
//
//    PARCReferenceCount count = parcObject_Release((PARCObject **) &time);
//    assertTrue(count == 0, "Expected reference count to be zero");
//    assertTrue(time == 0, "Expected memory pointer to be NULL after destroy.");
//}

LONGBOW_TEST_CASE(Global, parcObject_IsValid)
{
    PARCObject *object = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    assertTrue(parcObject_IsValid(object), "Expected valid PARCObject");

    parcObject_Release(&object);
}

LONGBOW_TEST_CASE(Global, parcObject_IsInstanceOf)
{
    _DummyObject *dummy1 = parcObject_CreateInstance(_DummyObject);
    assertTrue(parcObject_IsInstanceOf(dummy1, &PARCObject_Descriptor),
               "Expected _DummyObject to be an instance of PARCObject");

    parcObject_Release((PARCObject **) &dummy1);
}

LONGBOW_TEST_CASE(Global, parcObject_IsValid_NotValid)
{
    PARCObject *object = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    PARCObject *alias = object;
    parcObject_Release(&object);
    assertFalse(parcObject_IsValid(object), "Expected invalid PARCObject");
    assertFalse(parcObject_IsValid(alias), "Expected invalid PARCObject");
}

LONGBOW_TEST_CASE(Global, parcObject_Copy_Default)
{
    struct timeval *time = parcObject_CreateInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    struct timeval *copy = parcObject_Copy(time);

    parcObject_AssertValid(copy);

    assertTrue(copy->tv_sec == 1, "Expected tv_sec to equal 1");
    assertTrue(copy->tv_usec == 2, "Expected tv_usec to equal 2");

    PARCReferenceCount count = parcObject_Release((PARCObject **) &copy);
    assertTrue(count == 0, "Expected reference count to be zero");
    assertTrue(copy == 0, "Expected memory pointer to be NULL after destroy.");

    parcObject_Release((PARCObject *) &time);
}

LONGBOW_TEST_CASE(Global, parcObject_Copy)
{
    _DummyObject *dummy1 = parcObject_CreateInstance(_DummyObject);
    dummy1->calledCount = 100;

    _DummyObject *dummy2 = parcObject_Copy(dummy1);

    assertTrue(dummy2->calledCount == dummy1->calledCount,
               "Expected called counts to be the same. Got %d, expected %d.", dummy1->calledCount, dummy2->calledCount);

    parcObject_Release((PARCObject **) &dummy1);
    parcObject_Release((PARCObject **) &dummy2);
}

LONGBOW_TEST_CASE(Global, parcObject_Compare_Default)
{
    struct timeval *time1 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time1);

    time1->tv_sec = 1;
    time1->tv_usec = 2;

    struct timeval *time2 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time2);

    time2->tv_sec = 1;
    time2->tv_usec = 2;

    int compareResult = parcObject_Compare(time1, time2);
    assertTrue(compareResult == 0, "Expected objects to compare equal. Comparison result: %d", compareResult);
    compareResult = parcObject_Compare(time1, time1);
    assertTrue(compareResult == 0, "Expected same object to be equal since they have identical pointer addresses. Comparison result: %d", compareResult);

    parcObject_Release((PARCObject *) &time1);
    parcObject_Release((PARCObject *) &time2);
}

LONGBOW_TEST_CASE(Global, parcObject_Compare_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override",
                                    sizeof(struct timeval), sizeof(void*),
                                    true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);
    struct timeval *time1 = parcObject_CreateAndClearInstanceImpl(descriptor);
    parcObject_AssertValid(time1);

    time1->tv_sec = 1;
    time1->tv_usec = 2;

    struct timeval *time2 = parcObject_CreateAndClearInstanceImpl(descriptor);
    parcObject_AssertValid(time2);

    time2->tv_sec = 1;
    time2->tv_usec = 2;

    int compareResult = parcObject_Compare(time1, time2);
    assertTrue(compareResult == 0, "Expected objects to compare equal. Comparison result: %d", compareResult);
    compareResult = parcObject_Compare(time1, time1);
    assertTrue(compareResult == 0, "Expected same object to be equal since they have identical pointer addresses. Comparison result: %d", compareResult);

    parcObject_Release((PARCObject *) &time1);
    parcObject_Release((PARCObject *) &time2);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_Compare)
{
    _DummyObject *value = parcObject_CreateAndClearInstance(_DummyObject);
    _DummyObject *equality[2];
    equality[0] = parcObject_CreateAndClearInstance(_DummyObject);
    equality[1] = NULL;
    _DummyObject *lesser[2];
    lesser[0] = parcObject_CreateAndClearInstance(_DummyObject);
    lesser[1] = NULL;

    _DummyObject *greater[2];
    greater[0] = parcObject_CreateAndClearInstance(_DummyObject);
    greater[1] = NULL;

    value->calledCount = 50;
    equality[0]->calledCount = 50;
    lesser[0]->calledCount = 10;
    greater[0]->calledCount = 80;

    parcObjectTesting_AssertCompareTo(parcObject_Compare, value, equality, lesser, greater);

    parcObject_Release((void **) &value);
    parcObject_Release((void **) &equality[0]);
    parcObject_Release((void **) &lesser[0]);
    parcObject_Release((void **) &greater[0]);
}

LONGBOW_TEST_CASE(Global, parcObject_Equals_Default)
{
    struct timeval *x = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    memset(x, 0, sizeof(struct timeval));
    x->tv_sec = 1;
    x->tv_usec = 2;

    struct timeval *y = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    memset(y, 0, sizeof(struct timeval));
    y->tv_sec = 1;
    y->tv_usec = 2;

    assertTrue(parcObject_Equals(x, y), "Expected equality");

    struct timeval *z = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    z->tv_sec = 1;
    z->tv_usec = 2;

    struct timeval *unequal1 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    unequal1->tv_sec = 1;
    unequal1->tv_usec = 1;

    struct timeval *unequal2 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    unequal2->tv_sec = 0;
    unequal2->tv_usec = 0;

    parcObjectTesting_AssertEqualsFunction(parcObject_Equals, x, y, z, unequal1, unequal2, NULL);

    parcObject_Release((PARCObject *) &x);
    parcObject_Release((PARCObject *) &y);
    parcObject_Release((PARCObject *) &z);
    parcObject_Release((PARCObject *) &unequal1);
    parcObject_Release((PARCObject *) &unequal2);
}

LONGBOW_TEST_CASE(Global, parcObject_Equals_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override", sizeof(struct timeval), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);

    struct timeval *x = parcObject_CreateAndClearInstanceImpl(descriptor);
    memset(x, 0, sizeof(struct timeval));
    x->tv_sec = 1;
    x->tv_usec = 2;

    struct timeval *y = parcObject_CreateAndClearInstanceImpl(descriptor);
    memset(y, 0, sizeof(struct timeval));
    y->tv_sec = 1;
    y->tv_usec = 2;

    assertTrue(parcObject_Equals(x, y), "Expected equality");

    struct timeval *z = parcObject_CreateAndClearInstanceImpl(descriptor);
    z->tv_sec = 1;
    z->tv_usec = 2;

    struct timeval *unequal1 = parcObject_CreateAndClearInstanceImpl(descriptor);
    unequal1->tv_sec = 1;
    unequal1->tv_usec = 1;

    struct timeval *unequal2 = parcObject_CreateAndClearInstanceImpl(descriptor);
    unequal2->tv_sec = 0;
    unequal2->tv_usec = 0;

    parcObjectTesting_AssertEqualsFunction(parcObject_Equals, x, y, z, unequal1, unequal2, NULL);

    parcObject_Release((PARCObject *) &x);
    parcObject_Release((PARCObject *) &y);
    parcObject_Release((PARCObject *) &z);
    parcObject_Release((PARCObject *) &unequal1);
    parcObject_Release((PARCObject *) &unequal2);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_Equals)
{
    _DummyObject *x = parcObject_CreateInstance(_DummyObject);
    x->calledCount = 100;
    _DummyObject *y = parcObject_CreateInstance(_DummyObject);
    y->calledCount = 100;
    _DummyObject *z = parcObject_CreateInstance(_DummyObject);
    z->calledCount = 100;

    _DummyObject *unequal1 = parcObject_CreateInstance(_DummyObject);
    unequal1->calledCount = 50;

    PARCObject *unequal2 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);

    PARCObjectDescriptor dummyMeta2 = parcObject_DescriptorName(_DummyObject);
    _DummyObject *unequal3 = parcObject_CreateAndClearInstanceImpl(&dummyMeta2);
    unequal3->calledCount = 100;

    PARCObject *unequal4 = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);

    parcObjectTesting_AssertEqualsFunction(parcObject_Equals, x, y, z, unequal1, unequal2, unequal3, unequal4, NULL);

    parcObject_Release((PARCObject **) &x);
    parcObject_Release((PARCObject **) &y);
    parcObject_Release((PARCObject **) &z);
    parcObject_Release((PARCObject **) &unequal1);
    parcObject_Release((PARCObject **) &unequal2);
    parcObject_Release((PARCObject **) &unequal3);
    parcObject_Release((PARCObject **) &unequal4);
}

LONGBOW_TEST_CASE(Global, parcObject_HashCode_Default)
{
    struct timeval *time = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    PARCHashCode hashCode = parcObject_HashCode(time);
    PARCHashCode expected = parcHashCode_Hash((void *) time, sizeof(struct timeval));
    assertTrue(hashCode == expected, "Hash codes do not match. Got %" PRIPARCHashCode ", expected %" PRIPARCHashCode ".", hashCode, expected);

    parcObject_Release((PARCObject *) &time);
}

LONGBOW_TEST_CASE(Global, parcObject_HashCode_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override", sizeof(struct timeval), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);
    struct timeval *time = parcObject_CreateAndClearInstanceImpl(descriptor);
    parcObject_AssertValid(time);

    time->tv_sec = 1;
    time->tv_usec = 2;

    PARCHashCode hashCode = parcObject_HashCode(time);
    PARCHashCode expected = parcHashCode_Hash((void *) time, sizeof(struct timeval));
    assertTrue(hashCode == expected, "Hash codes do not match. Got %" PRIPARCHashCode ", expected %" PRIPARCHashCode ".", hashCode, expected);

    parcObject_Release((PARCObject *) &time);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_HashCode)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    PARCHashCode hashCode = parcObject_HashCode(dummy);
    assertTrue(hashCode == 1337, "Expected hashcode to be 1337, got %" PRIPARCHashCode, hashCode);

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_ToString)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    char *strRep = parcObject_ToString(dummy);
    assertTrue(strcmp(strRep, "dummy") == 0, "Expected 'dummy' string representation, got %s", strRep);

    parcMemory_Deallocate((void **) &strRep);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_ToString_Default)
{
    _DummyObject *dummy = parcObject_CreateAndClearInstanceImpl(&_DummyObject_Descriptor);

    char *strRep = parcObject_ToString(dummy);

    parcMemory_Deallocate((void **) &strRep);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_ToString_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override", sizeof(struct timeval), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);
    _DummyObject *dummy = parcObject_CreateAndClearInstanceImpl(descriptor);

    char *strRep = parcObject_ToString(dummy);

    parcMemory_Deallocate((void **) &strRep);
    parcObject_Release((PARCObject **) &dummy);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_ToJSON_Default)
{
    size_t expectedSize = sizeof(struct timeval);
    PARCObject *memory = parcObject_CreateAndClearInstanceImpl(&_testObject_Descriptor);

    PARCJSON *json = parcObject_ToJSON(memory);

    const PARCJSONPair *lengthPair = parcJSON_GetPairByName(json, "objectLength");
    PARCJSONValue *lengthValue = parcJSONPair_GetValue(lengthPair);
    uint64_t actualLength = parcJSONValue_GetInteger(lengthValue);

    const PARCJSONPair *alignmentPair = parcJSON_GetPairByName(json, "objectAlignment");
    PARCJSONValue *alignmentValue = parcJSONPair_GetValue(alignmentPair);
    int alignment = (int) parcJSONValue_GetInteger(alignmentValue);

    assertTrue(actualLength >= expectedSize,
               "Expected length to be >= %zd, actual %" PRIu64 "", expectedSize, actualLength);
    assertTrue(alignment == sizeof(void *), "Expected objectAlignment to be %zd, got %d",
               sizeof(void *), alignment);

    parcJSON_Release(&json);
    parcObject_Release(&memory);
}

LONGBOW_TEST_CASE(Global, parcObject_ToJSON)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    PARCJSON *json = parcObject_ToJSON(dummy);
    char *strRep = parcJSON_ToString(json);
    assertTrue(strcmp(strRep, "{ \"type\" : \"dummy\" }") == 0, "Expected fixed JSON object with a specific string representation, got %s", strRep);

    parcMemory_Deallocate((void **) &strRep);
    parcJSON_Release(&json);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_ToJSON_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override", sizeof(struct timeval), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);

    size_t expectedSize = sizeof(struct timeval);
    PARCObject *memory = parcObject_CreateAndClearInstanceImpl(descriptor);

    PARCJSON *json = parcObject_ToJSON(memory);
    const PARCJSONPair *lengthPair = parcJSON_GetPairByName(json, "objectLength");
    PARCJSONValue *lengthValue = parcJSONPair_GetValue(lengthPair);
    uint64_t actualLength = parcJSONValue_GetInteger(lengthValue);

    const PARCJSONPair *alignmentPair = parcJSON_GetPairByName(json, "objectAlignment");
    PARCJSONValue *alignmentValue = parcJSONPair_GetValue(alignmentPair);
    int alignment = (int) parcJSONValue_GetInteger(alignmentValue);

    assertTrue(actualLength >= expectedSize,
               "Expected length to be >= %zd, actual %" PRIu64 "", expectedSize, actualLength);
    assertTrue(alignment == sizeof(void *), "Expected objectAlignment to be %zd, got %d",
               sizeof(void *), alignment);

    parcJSON_Release(&json);
    parcObject_Release(&memory);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_GetReferenceCount)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);
    PARCReferenceCount refCount = parcObject_GetReferenceCount(dummy);
    assertTrue(refCount == 1, "Expected reference count to be 1, got %" PRIu64 "", refCount);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_Display_Default)
{
    _DummyObject *dummy = parcObject_CreateAndClearInstanceImpl(&_DummyObject_Descriptor);
    parcObject_Display(dummy, 0);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_Display_NoOverride)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("override", sizeof(struct timeval), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);

    _DummyObject *dummy = parcObject_CreateAndClearInstanceImpl(descriptor);
    parcObject_Display(dummy, 0);
    parcObject_Release((PARCObject **) &dummy);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Global, parcObject_Display)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);
    parcObject_Display(dummy, 0);
    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(Global, parcObject_GetDescriptor)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);
    const PARCObjectDescriptor *descriptor = parcObject_GetDescriptor(dummy);

    assertTrue(descriptor == &_DummyObject_Descriptor, "Expected pointer to _DummyObject_Descriptor");

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_FIXTURE(Subclasses)
{
    LONGBOW_RUN_TEST_CASE(Subclasses, parcObject_Copy);
}

LONGBOW_TEST_FIXTURE_SETUP(Subclasses)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Subclasses)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    parcMemory_SetInterface(_originalMemoryProvider);

    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Subclasses, parcObject_Copy)
{
    const PARCObjectDescriptor *objectType =
        parcObjectDescriptor_Create("Dummy", sizeof(_DummyObject), sizeof(void*), true,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &PARCObject_Descriptor, NULL);

    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);
    parcObject_SetDescriptor(dummy, objectType);

    dummy->calledCount = 100;

    _DummyObject *dummy2 = parcObject_Copy(dummy);

    assertTrue(dummy2->calledCount == dummy->calledCount,
               "Expected called counts to be the same. Got %d, expected %d.", dummy->calledCount, dummy2->calledCount);

    parcObject_Release((PARCObject **) &dummy);
    parcObject_Release((PARCObject **) &dummy2);
    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &objectType);
}

LONGBOW_TEST_FIXTURE(Locking)
{
    LONGBOW_RUN_TEST_CASE(Locking, parcObject_TryLock_Unlock);
    LONGBOW_RUN_TEST_CASE(Locking, parcObject_Lock_Unlock);
    LONGBOW_RUN_TEST_CASE(Locking, parcObject_TryLock_AlreadyLockedSameThread);
    LONGBOW_RUN_TEST_CASE(Locking, parcObject_Lock_AlreadyLocked);
}
static uint32_t initialAllocations;

LONGBOW_TEST_FIXTURE_SETUP(Locking)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    initialAllocations = parcMemory_Outstanding();

    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    longBowTestCase_SetClipBoardData(testCase, dummy);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Locking)
{
    _DummyObject *dummy = longBowTestCase_GetClipBoardData(testCase);

    parcObject_Release((PARCObject **) &dummy);

    if (!parcMemoryTesting_ExpectedOutstanding(0, "%s mismanaged memory.", longBowTestCase_GetFullName(testCase))) {
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        parcMemory_SetInterface(_originalMemoryProvider);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    parcMemory_SetInterface(_originalMemoryProvider);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Locking, parcObject_TryLock_Unlock)
{
    _DummyObject *dummy = longBowTestCase_GetClipBoardData(testCase);

    bool actual = parcObject_TryLock(dummy);

    assertTrue(actual, "Expected parcObject_TryLock to succeed.");

    actual = parcObject_IsLocked(dummy);
    assertTrue(actual, "Expected parcObject_IsLocked to be true.");

    actual = parcObject_Unlock(dummy);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    actual = parcObject_IsLocked(dummy);
    assertFalse(actual, "Expected parcObject_IsLocked to be false.");
}

LONGBOW_TEST_CASE(Locking, parcObject_Lock_Unlock)
{
    _DummyObject *dummy = longBowTestCase_GetClipBoardData(testCase);

    bool actual = parcObject_Lock(dummy);

    assertTrue(actual, "Expected parcObject_Lock to succeed.");

    actual = parcObject_IsLocked(dummy);
    assertTrue(actual, "Expected parcObject_IsLocked to be true.");

    actual = parcObject_Unlock(dummy);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");

    actual = parcObject_IsLocked(dummy);
    assertFalse(actual, "Expected parcObject_IsLocked to be false.");
}

LONGBOW_TEST_CASE_EXPECTS(Locking, parcObject_TryLock_AlreadyLockedSameThread, .event = &LongBowTrapCannotObtainLockEvent)
{
    _DummyObject *dummy = longBowTestCase_GetClipBoardData(testCase);

    bool actual = parcObject_TryLock(dummy);

    assertTrue(actual, "Expected parcObject_TryLock to succeed.");

    actual = parcObject_TryLock(dummy);

    assertFalse(actual, "Expected parcObject_TryLock to fail when already locked by the same thread.");

    actual = parcObject_Unlock(dummy);
    assertTrue(actual, "Expected parcObject_Unlock to succeed.");
}

LONGBOW_TEST_CASE_EXPECTS(Locking, parcObject_Lock_AlreadyLocked, .event = &LongBowTrapCannotObtainLockEvent)
{
    _DummyObject *dummy = longBowTestCase_GetClipBoardData(testCase);

    bool actual = parcObject_Lock(dummy);

    assertTrue(actual, "Expected parcObject_Lock to succeed.");

    actual = parcObject_IsLocked(dummy);
    assertTrue(actual, "Expected locked object to indicate being locked.");

    parcObject_Lock(dummy);
}

LONGBOW_TEST_FIXTURE(WaitNotify)
{
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcObject_WaitNotify);
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcObject_WaitNotify2);
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcObject_WaitUntil);
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcObject_WaitFor);
    LONGBOW_RUN_TEST_CASE(WaitNotify, parcObject_WaitNotifyAll);
}

LONGBOW_TEST_FIXTURE_SETUP(WaitNotify)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(WaitNotify)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);

    parcMemory_SetInterface(_originalMemoryProvider);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// Just wait until told to wakeup, then increment a counter and unlock.
static void *
waiter(void *data)
{
    _DummyObject *dummy = data;

    while (parcObject_TryLock(dummy) == false) {
        ;
    }
    assertTrue(parcObject_IsLocked(dummy), "%p object %p not locked.", (void *) pthread_self(), (void *) dummy);
    parcObject_Wait(dummy);

    dummy->val++;
    parcObject_Unlock(dummy);

    return data;
}

LONGBOW_TEST_CASE(WaitNotify, parcObject_WaitNotify)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    dummy->val = 0;

    pthread_t thread_A;
    pthread_t thread_B;
    pthread_t thread_C;
    pthread_create(&thread_A, NULL, waiter, dummy);
    pthread_create(&thread_B, NULL, waiter, dummy);
    pthread_create(&thread_C, NULL, waiter, dummy);

    while (dummy->val != 3) {
        while (parcObject_TryLock(dummy) == false) {
            ;
        }
        parcObject_Notify(dummy);
        parcObject_Unlock(dummy);
    }

    pthread_join(thread_A, NULL);
//    pthread_join(thread_B, NULL);
//    pthread_join(thread_C, NULL);

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(WaitNotify, parcObject_WaitNotifyAll)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    dummy->val = 0;

    pthread_t thread_A;
    pthread_t thread_B;
    pthread_t thread_C;
    pthread_create(&thread_A, NULL, waiter, dummy);
    pthread_create(&thread_B, NULL, waiter, dummy);
    pthread_create(&thread_C, NULL, waiter, dummy);

    while (dummy->val != 3) {
        while (parcObject_TryLock(dummy) == false) {
            ;
        }
        parcObject_NotifyAll(dummy);
        parcObject_Unlock(dummy);
    }

    pthread_join(thread_A, NULL);
//    pthread_join(thread_B, NULL);
//    pthread_join(thread_C, NULL);

    assertTrue(dummy->val == 3, "Expected the counter to be 3, actual %d", dummy->val);

    parcObject_Release((PARCObject **) &dummy);
}

static void *
decrement(void *data)
{
    _DummyObject *dummy = data;

    while (parcObject_TryLock(dummy) == false) {
        ;
    }
    while (dummy->val < 12) {
        parcObject_Wait(dummy);
        dummy->val--;
    }
    parcObject_Unlock(dummy);

    return data;
}

LONGBOW_TEST_CASE(WaitNotify, parcObject_WaitNotify2)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    dummy->val = 0;

    pthread_t thread_A;
    pthread_create(&thread_A, NULL, decrement, dummy);

    dummy->val = 2;
    while (parcObject_TryLock(dummy) == false) {
        ;
    }
    while (dummy->val <= 12) {
        parcObject_Notify(dummy);
        dummy->val += 2;
    }
    parcObject_Unlock(dummy);

    pthread_join(thread_A, NULL);

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(WaitNotify, parcObject_WaitUntil)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    dummy->val = 0;

    time_t now;
    time_t then;
    time(&then);
    if (parcObject_Lock(dummy)) {
        struct timespec future;
        future.tv_sec = then + 3;
        future.tv_nsec = 0;
        parcObject_WaitUntil(dummy, &future);

        time(&now);
        long expected = now - 1;  // Subtract 1 because the future may have been computed at the 999,999,999 nanosecond mark.
        assertTrue(now >= expected, "Expected now %ld, to be later than than %ld", now, expected);
        parcObject_Unlock(dummy);
    }

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_CASE(WaitNotify, parcObject_WaitFor)
{
    _DummyObject *dummy = parcObject_CreateInstance(_DummyObject);

    dummy->val = 0;

    time_t now;
    time_t then;
    time(&then);
    if (parcObject_Lock(dummy)) {
        uint64_t nanoSeconds = 1000000000;
        parcObject_WaitFor(dummy, nanoSeconds);

        time(&now);
        now++;  // Advance now by 1 because of the precision mismatch between gettimeofday and nanosecond resolution of parcObject_WaitFor
        assertTrue(now >= then + (nanoSeconds / 1000000000),
                   "Expected now %ld, to be later than time %" PRIu64, now, then + (nanoSeconds / 1000000000));
        parcObject_Unlock(dummy);
    }

    parcObject_Release((PARCObject **) &dummy);
}

LONGBOW_TEST_FIXTURE(Fail)
{
}

typedef struct {
    PARCObject *value;
} TestData;

LONGBOW_TEST_FIXTURE_SETUP(Fail)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    data->value = parcObject_CreateInstance(_DummyObjectNoHash);
    if (data->value == NULL) {
        return LONGBOW_STATUS_SETUP_FAILED;
    }
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Fail)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    parcObject_Release(&data->value);
    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);

    parcMemory_SetInterface(_originalMemoryProvider);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_OPTIONS(Performance, .enabled = false)
{
    LONGBOW_RUN_TEST_CASE(Performance, _parcObject_PrefixLength_10000000);
    LONGBOW_RUN_TEST_CASE(Performance, parcObject_CreateRelease);
    LONGBOW_RUN_TEST_CASE(Performance, parcObject_Create);
    LONGBOW_RUN_TEST_CASE(Performance, parcObject_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(Performance)
{
    parcMemory_SetInterface(&PARCStdlibMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Performance)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Performance, _parcObject_PrefixLength_10000000)
{
    // Test that the result is a multiple of the alignment value and greater than the size of _PARCObjectHeader.

    // Compute the power of 2 value of sizeof(void *)
    unsigned int v = sizeof(void *);
    unsigned int r = 0; // r will be lg(v)

    while (v >>= 1) {
        r++;
    }

    PARCObjectDescriptor descriptor;

    for (int i = r; i < 20; i++) {
        descriptor.objectAlignment = 1 << i;
        size_t actual = _parcObject_PrefixLength(&descriptor);
        assertTrue((actual & (descriptor.objectAlignment - 1)) == 0,
                   "Alignment needs to be a multiple of %u", descriptor.objectAlignment);
    }
}

#define OBJECT_COUNT 10000000
#define OBJECT_SIZE 1200

typedef struct { char bytes[OBJECT_SIZE]; } PerformanceObject;

parcObject_Override(PerformanceObject, PARCObject);

LONGBOW_TEST_CASE(Performance, parcObject_CreateRelease)
{
    for (int i = 0; i < OBJECT_COUNT; i++) {
        PARCObject *object = parcObject_CreateInstanceImpl(&PerformanceObject_Descriptor);

        PARCObject *object1 = parcObject_Acquire(object);
        PARCObject *object2 = parcObject_Acquire(object);
        parcObject_Release(&object1);
        parcObject_Release(&object2);

        parcObject_Release(&object);
    }
}

void *objects[OBJECT_COUNT];

LONGBOW_TEST_CASE(Performance, parcObject_AcquireRelease)
{
    PARCObject *object = parcObject_CreateInstanceImpl(&PerformanceObject_Descriptor);

    for (int i = 0; i < OBJECT_COUNT; i++) {
        objects[i] = parcObject_Acquire(object);
    }

    for (int i = 0; i < OBJECT_COUNT; i++) {
        parcObject_Release(&objects[i]);
    }

    parcObject_Release(&object);
}

LONGBOW_TEST_CASE(Performance, parcObject_Create)
{
    for (int i = 0; i < OBJECT_COUNT; i++) {
        objects[i] = parcObject_CreateInstanceImpl(&PerformanceObject_Descriptor);
    }

    for (int i = 0; i < OBJECT_COUNT; i++) {
        parcObject_Release(&objects[i]);
    }
}

LONGBOW_TEST_FIXTURE(Meta)
{
    LONGBOW_RUN_TEST_CASE(Meta, _metaDestructor_True);
    LONGBOW_RUN_TEST_CASE(Meta, _metaDestructor_False);
    LONGBOW_RUN_TEST_CASE(Meta, _metaDestructor_None);

    LONGBOW_RUN_TEST_CASE(Meta, parcObjectDescriptor_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Meta)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    _DummyObject *data;
    parcMemory_MemAlign((void **) &data, sizeof(void *), sizeof(_DummyObject));
    data->val = 10;
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Meta)
{
    _DummyObject *data = longBowTestCase_GetClipBoardData(testCase);
    parcMemory_Deallocate((void **) &data);

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    parcMemory_SetInterface(_originalMemoryProvider);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Meta, parcObjectDescriptor_Create)
{
    const PARCObjectDescriptor *interface = parcObjectDescriptor_Create("Meta",
                                                                        sizeof(struct timeval), sizeof(void*),
                                                                        true,
                                                                        _meta_destructor_true, NULL, _meta_copy, _meta_toString,
                                                                        _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL,
                                                                        &PARCObject_Descriptor, NULL);

    assertNotNull(interface, "Expected interface instance to be allocated correctly.");

    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &interface);
    assertNull(interface, "Expected parcObjectDescriptor_Destroy to NULL the input pointer");
}

LONGBOW_TEST_CASE(Meta, _metaDestructor_True)
{
    const PARCObjectDescriptor *interface =
        parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                    _meta_destructor_true, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL,
                                    &PARCObject_Descriptor, NULL);
    _DummyObject *data = longBowTestCase_GetClipBoardData(testCase);
    bool actual = _parcObject_Destructor(interface, (PARCObject **) &data);

    assertTrue(actual, "Expected destructor to return true.");

    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &interface);
}

LONGBOW_TEST_CASE(Meta, _metaDestructor_False)
{
    const PARCObjectDescriptor *descriptor =
        parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                    _meta_destructor_false, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL,
                                    &PARCObject_Descriptor, NULL);
    _DummyObject *data = longBowTestCase_GetClipBoardData(testCase);
    bool actual = _parcObject_Destructor(descriptor, (PARCObject **) &data);

    assertNull(data, "Expected destructor function to have been called to nullify the reference.");
    assertFalse(actual, "Expected destructor to return false.");

    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(Meta, _metaDestructor_None)
{
    const PARCObjectDescriptor *interface = parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                                                        NULL, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL, &PARCObject_Descriptor, NULL);
    _DummyObject *data = longBowTestCase_GetClipBoardData(testCase);
    _parcObject_Destructor(interface, (void **) &data);

    assertNotNull(data, "Expected destructor function to have been called to nullify the reference.");

    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &interface);
}

LONGBOW_TEST_FIXTURE(PARCObjectDescriptor)
{
    LONGBOW_RUN_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_Create);
    LONGBOW_RUN_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_CreateExtension);
    LONGBOW_RUN_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_GetSuperType);
    LONGBOW_RUN_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_GetTypeState);
}

LONGBOW_TEST_FIXTURE_SETUP(PARCObjectDescriptor)
{
    longBowTestCase_SetInt(testCase, "initialAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(PARCObjectDescriptor)
{
    int initialAllocations = longBowTestCase_GetInt(testCase, "initialAllocations");

    uint32_t outstandingAllocations = parcMemory_Outstanding() - initialAllocations;

    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        parcSafeMemory_ReportAllocation(STDOUT_FILENO);
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_Create)
{
    const PARCObjectDescriptor *descriptor = parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                                                         NULL, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL, &PARCObject_Descriptor, NULL);

    parcObjectDescriptor_Destroy((PARCObjectDescriptor **) &descriptor);
}

LONGBOW_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_CreateExtension)
{
    PARCObjectDescriptor *descriptor = parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                                                   NULL, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL, &PARCObject_Descriptor, NULL);

    PARCObjectDescriptor *extension = parcObjectDescriptor_CreateExtension(descriptor, "Extension");

    parcObjectDescriptor_Destroy(&extension);
    parcObjectDescriptor_Destroy(&descriptor);
}

LONGBOW_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_GetSuperType)
{
    PARCObjectDescriptor *descriptor = parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                                                   NULL, NULL, _meta_copy, _meta_toString, _meta_equals, _meta_compare, _meta_hashCode, _meta_toJson, NULL, &PARCObject_Descriptor, NULL);

    const PARCObjectDescriptor *superType = parcObjectDescriptor_GetSuperType(descriptor);

    assertTrue(superType == &PARCObject_Descriptor, "Expected a pointer to PARCObject_Descriptor");

    parcObjectDescriptor_Destroy(&descriptor);
}

LONGBOW_TEST_CASE(PARCObjectDescriptor, parcObjectDescriptor_GetTypeState)
{
    PARCObjectDescriptor *descriptor = parcObjectDescriptor_Create("Meta", sizeof(struct timeval), sizeof(void*), true,
                                                                   NULL, NULL, _meta_copy, _meta_toString, _meta_equals,
                                                                   _meta_compare, _meta_hashCode, _meta_toJson, NULL,
                                                                   &PARCObject_Descriptor,
                                                                   (PARCObjectTypeState *) &PARCObject_Descriptor);

    PARCObjectTypeState *state = parcObjectDescriptor_GetTypeState(descriptor);

    assertTrue(state == &PARCObject_Descriptor, "Expected a pointer to PARCObject_Descriptor");

    parcObjectDescriptor_Destroy(&descriptor);
}

LONGBOW_TEST_FIXTURE(StaticObjects)
{
    LONGBOW_RUN_TEST_CASE(StaticObjects, parcObject_WrapImpl);
    LONGBOW_RUN_TEST_CASE(StaticObjects, parcObject_InitInstanceImpl);
    LONGBOW_RUN_TEST_CASE(StaticObjects, parcObject_InitAndClearInstanceImpl);
}

LONGBOW_TEST_FIXTURE_SETUP(StaticObjects)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(StaticObjects)
{
    parcMemory_SetInterface(_originalMemoryProvider);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(StaticObjects, parcObject_WrapImpl)
{
    char *origin = (char[parcObject_TotalSize(sizeof(void*), 10)]) { 0 };

    PARCObject *result = parcObject_WrapImpl(origin, &parcObject_DescriptorName(PARCObject));

    parcObject_AssertValid(result);

    parcObject_Release(&result);
}

PARCObject *globalObject = parcObject_Instance(PARCObject, sizeof(void*), 10);

LONGBOW_TEST_CASE(StaticObjects, parcObject_InitInstanceImpl)
{
    parcObject_InitInstanceImpl(globalObject, &PARCObject_Descriptor);

    parcObject_AssertValid(globalObject);

//    parcObject_Release(&globalObject);
}

LONGBOW_TEST_CASE(StaticObjects, parcObject_InitAndClearInstanceImpl)
{
    parcObject_InitAndClearInstanceImpl(globalObject, &PARCObject_Descriptor);

    parcObject_AssertValid(globalObject);

//    parcObject_Release(&globalObject);
}

LONGBOW_TEST_FIXTURE(Synchronization)
{
    LONGBOW_RUN_TEST_CASE(Synchronization, parcObject_SynchronizeBegin);
}

LONGBOW_TEST_FIXTURE_SETUP(Synchronization)
{
    _originalMemoryProvider = parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Synchronization)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);

    parcMemory_SetInterface(_originalMemoryProvider);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestRunner_GetName(testRunner), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Synchronization, parcObject_SynchronizeBegin)
{
    PARCObject *dummy = parcObject_CreateInstance(_DummyObject);

    bool result = parcObject_BarrierSet(dummy);
    assertTrue(result, "Expected parcObject_BarrierSet to always return true.");

    _PARCObjectHeader *header = _parcObject_Header(dummy);
    assertTrue(header->barrier, "Expected the header barrier to be set.");

    result = parcObject_BarrierUnset(dummy);
    assertFalse(result, "Expected parcObject_BarrierUnset to always return false.");
    assertFalse(header->barrier, "Expected the header barrier to NOT be set.");

    parcObject_Release(&dummy);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcObject);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}

