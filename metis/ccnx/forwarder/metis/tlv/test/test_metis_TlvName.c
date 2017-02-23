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
#include "../metis_TlvName.c"
#include <LongBow/unit-test.h>

#include <stdint.h>
#include <limits.h>
#include <parc/algol/parc_SafeMemory.h>

uint8_t encoded_name[] = {
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l',
    'o',  // "hello"
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h',  // value = "ouch"
    0xF0, 0x01, 0x00, 0x02, // type = app, length = 2
    0x01, 0xFF              // value = 0x01FF
};

uint8_t second_name[] = {
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l',
    'o',  // "hello"
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h',  // value = "ouch"
    0xF0, 0x01, 0x00, 0x02, // type = app, length = 2
    0xFF, 0xFF              // value = 0xFFFF
};

uint8_t prefixOf_name[] = {
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l',
    'o',  // "hello"
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h',  // value = "ouch"
};

uint8_t default_route_name[] = {
    0x00, 0x01, 0x00, 0x00, // type = name, length = 0
};

LONGBOW_TEST_RUNNER(metis_TlvName)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_TlvName)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_TlvName)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost0);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost1);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost2);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMostAll);

    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Create_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_CreateFromCCNxName);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_CreateFromCCNxName_DefaultRoute);

    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Equals_IsEqual);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Equals_SameCountDifferentBytes);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Equals_DifferentCount);

    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Compare);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Compare_DefaultRoute);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_Compare_DefaultRoute_Binary);

    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_HashCode);

    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_SegmentCount);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_StartsWith_SelfPrefix);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_StartsWith_IsPrefix);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_StartsWith_PrefixTooLong);
    LONGBOW_RUN_TEST_CASE(Global, metisTlvName_StartsWith_IsNotPrefix);
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

LONGBOW_TEST_CASE(Global, metisTlvName_Acquire)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));

    MetisTlvName *copy = metisTlvName_Acquire(name);
    assertTrue(_getRefCount(name) == 2, "Name created with wrong refcount, expected %u got %u", 2, _getRefCount(name));

    metisTlvName_Release(&copy);
    assertTrue(_getRefCount(name) == 1, "Name created with wrong refcount, expected %u got %u", 1, _getRefCount(name));

    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Create_Destroy)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    assertTrue(_getRefCount(name) == 1, "Name created with wrong refcount, expected %u got %u", 1, _getRefCount(name));
    metisTlvName_Release(&name);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance after create/destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisTlvName_CreateFromCCNxName)
{
    char uri[] = "lci:/2=hello/0xF000=ouch/0xF001=%01%FF";
    CCNxName *ccnxName = ccnxName_CreateFromCString(uri);

    MetisTlvName *truth = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *name = metisTlvName_CreateFromCCNxName(ccnxName);

    assertTrue(metisTlvName_Equals(truth, name), "MetisTlvName from ccnxName did not equal expected");
    metisTlvName_Release(&name);
    metisTlvName_Release(&truth);
    ccnxName_Release(&ccnxName);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance after create/destroy: %u", parcMemory_Outstanding());
}

LONGBOW_TEST_CASE(Global, metisTlvName_CreateFromCCNxName_DefaultRoute)
{
    char uri[] = "lci:/";
    CCNxName *ccnxName = ccnxName_CreateFromCString(uri);

    MetisTlvName *truth = metisTlvName_Create(default_route_name, sizeof(default_route_name));
    MetisTlvName *name = metisTlvName_CreateFromCCNxName(ccnxName);

    assertTrue(metisTlvName_Equals(truth, name), "MetisTlvName from ccnxName did not equal expected");
    metisTlvName_Release(&name);
    metisTlvName_Release(&truth);
    ccnxName_Release(&ccnxName);

    assertTrue(parcSafeMemory_ReportAllocation(STDOUT_FILENO) == 0, "Memory imbalance after create/destroy: %u", parcMemory_Outstanding());
}


LONGBOW_TEST_CASE(Global, metisTlvName_Equals_IsEqual)
{
    MetisTlvName *a = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *b = metisTlvName_Create(encoded_name, sizeof(encoded_name));

    assertTrue(metisTlvName_Equals(a, b), "Two equal names did not compare");
    metisTlvName_Release(&a);
    metisTlvName_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Equals_SameCountDifferentBytes)
{
    MetisTlvName *a = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *b = metisTlvName_Create(second_name, sizeof(second_name));

    assertFalse(metisTlvName_Equals(a, b), "Two names with same # component but differnet bytes compared the same.");
    metisTlvName_Release(&a);
    metisTlvName_Release(&b);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Equals_DifferentCount)
{
    MetisTlvName *a = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *b = metisTlvName_Create(prefixOf_name, sizeof(prefixOf_name));

    assertFalse(metisTlvName_Equals(a, b), "Two names with different # component compared the same.");
    metisTlvName_Release(&a);
    metisTlvName_Release(&b);
}

int
compareWrapper(void *a, void *b)
{
    return metisTlvName_Compare((MetisTlvName *) a, (MetisTlvName *) b);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Compare)
{
    CCNxName *basename = ccnxName_CreateFromCString("lci:/middle/of/6=the");
    CCNxName *equal_1 = ccnxName_CreateFromCString("lci:/middle/of/6=the");
    CCNxName *defaultRoute = ccnxName_CreateFromCString("lci:/");
    CCNxName *lesser_by_count = ccnxName_CreateFromCString("lci:/middle/of");
    CCNxName *lesser_by_value = ccnxName_CreateFromCString("lci:/middle/of/6=th");
    CCNxName *lesser_by_type_2 = ccnxName_CreateFromCString("lci:/middle/of/2=the");
    CCNxName *greater_by_count = ccnxName_CreateFromCString("lci:/middle/of/the/road");
    CCNxName *greater_by_type = ccnxName_CreateFromCString("lci:/middle/of/7=the");
    CCNxName *greater_by_value = ccnxName_CreateFromCString("lci:/middle/of/the/town");
    CCNxName *greater_2 = ccnxName_CreateFromCString("lci:/nox/arcana/occulta");

    void *equivalent[] = { equal_1, NULL };
    void *lesser[] = { defaultRoute, lesser_by_count, lesser_by_type_2, lesser_by_value, NULL };
    void *greater[] = { greater_by_count, greater_by_type, greater_by_value, greater_2, NULL };

    MetisTlvName *tlv_basename = metisTlvName_CreateFromCCNxName(basename);
    void **tlv_equivalent = parcMemory_AllocateAndClear(sizeof(equivalent) * sizeof(void *));
    assertNotNull(tlv_equivalent, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(equivalent) * sizeof(void *));
    void **tlv_lesser = parcMemory_AllocateAndClear(sizeof(lesser) * sizeof(void *));
    assertNotNull(tlv_lesser, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(lesser) * sizeof(void *));
    void **tlv_greater = parcMemory_AllocateAndClear(sizeof(greater) * sizeof(void *));
    assertNotNull(tlv_greater, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(greater) * sizeof(void *));

    for (int i = 0; equivalent[i] != NULL; i++) {
        tlv_equivalent[i] = metisTlvName_CreateFromCCNxName(equivalent[i]);
    }

    for (int i = 0; lesser[i] != NULL; i++) {
        tlv_lesser[i] = metisTlvName_CreateFromCCNxName(lesser[i]);
    }

    for (int i = 0; greater[i] != NULL; i++) {
        tlv_greater[i] = metisTlvName_CreateFromCCNxName(greater[i]);
    }

    // Use this:
    assertCompareToContract(compareWrapper, tlv_basename, tlv_equivalent, tlv_lesser, tlv_greater);
    // Not this, unless you provide the necessary casts to avoid warnings.
    // longbow_AssertCompareToContract(compareWrapper, tlv_basename, tlv_equivalent, tlv_lesser, tlv_greater);

    for (int i = 0; equivalent[i] != NULL; i++) {
        ccnxName_Release((CCNxName **) &equivalent[i]);
        metisTlvName_Release((MetisTlvName **) &tlv_equivalent[i]);
    }

    for (int i = 0; lesser[i] != NULL; i++) {
        ccnxName_Release((CCNxName **) &lesser[i]);
        metisTlvName_Release((MetisTlvName **) &tlv_lesser[i]);
    }

    for (int i = 0; greater[i] != NULL; i++) {
        ccnxName_Release((CCNxName **) &greater[i]);
        metisTlvName_Release((MetisTlvName **) &tlv_greater[i]);
    }

    ccnxName_Release(&basename);
    metisTlvName_Release(&tlv_basename);
    parcMemory_Deallocate((void **) &tlv_equivalent);
    parcMemory_Deallocate((void **) &tlv_greater);
    parcMemory_Deallocate((void **) &tlv_lesser);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Compare_DefaultRoute)
{
    CCNxName *defaultRoute = ccnxName_CreateFromCString("lci:/");
    MetisTlvName *metisDefaultRoute = metisTlvName_CreateFromCCNxName(defaultRoute);

    // THis name cannot be constructed via CCNxName, so do it as a byte array
    // Empty name with "0" type
    uint8_t shortest[] = { 0x00, 0x00, 0x00,    4,
                           0x00, 0x00, 0x00,    0 };

    MetisTlvName *metisShortest = metisTlvName_Create(shortest, sizeof(shortest));

    int compare = metisTlvName_Compare(metisDefaultRoute, metisShortest);
    assertTrue(compare < 0, "Default route should have compared less than shortest name, compared = %d", compare);

    metisTlvName_Release(&metisShortest);
    metisTlvName_Release(&metisDefaultRoute);
    ccnxName_Release(&defaultRoute);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Compare_DefaultRoute_Binary)
{
    // The empty name (default route)
    uint8_t defaultRoute[] = { 0x00, 0x00, 0x00,    0};
    MetisTlvName *metisDefaultRoute = metisTlvName_Create(defaultRoute, sizeof(defaultRoute));

    // THis name cannot be constructed via CCNxName, so do it as a byte array
    // Empty name with "0" type
    uint8_t shortest[] = { 0x00, 0x00, 0x00,    4,
        0x00, 0x00, 0x00,    0 };

    MetisTlvName *metisShortest = metisTlvName_Create(shortest, sizeof(shortest));

    int compare = metisTlvName_Compare(metisDefaultRoute, metisShortest);
    assertTrue(compare < 0, "Default route should have compared less than shortest name, compared = %d", compare);

    metisTlvName_Release(&metisShortest);
    metisTlvName_Release(&metisDefaultRoute);
}

LONGBOW_TEST_CASE(Global, metisTlvName_HashCode)
{
    // first, compute the hashes of the name
    uint32_t hash_0 = parcHash32_Data(&encoded_name[0], 9);
    uint32_t hash_1 = parcHash32_Data_Cumulative(&encoded_name[ 9], 8, hash_0);
    uint32_t hash_2 = parcHash32_Data_Cumulative(&encoded_name[17], 6, hash_1);

    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));

    uint32_t test_hash;
    test_hash = metisTlvName_HashCode(name);
    assertTrue(test_hash == hash_2, "Incorrect hash for segment %d, expected %08X got %08X", 2, hash_2, test_hash);

    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost0)
{
    unsigned copyLength = 0;
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *copy = metisTlvName_Slice(name, copyLength);

    // hash of a 0-length name is 0
    uint32_t hash_0 = 0;

    assertTrue(_getRefCount(name) == 2, "Wrong refcount in name, expected %u got %u", 2, _getRefCount(name));
    assertTrue(_getRefCount(copy) == 2, "Wrong refcount in copy, expected %u got %u", 2, _getRefCount(copy));
    assertTrue(copy->segmentArrayLength == copyLength, "Wrong array length, expected %u got %zu", copyLength, copy->segmentArrayLength);
    uint32_t test_hash = metisTlvName_HashCode(copy);
    assertTrue(test_hash == hash_0, "Incorrect hash for segment %d, expected %08X got %08X", 0, hash_0, test_hash);

    metisTlvName_Release(&copy);
    assertTrue(_getRefCount(name) == 1, "Wrong refcount in name, expected %u got %u", 1, _getRefCount(name));
    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost1)
{
    unsigned copyLength = 1;
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *copy = metisTlvName_Slice(name, copyLength);
    uint32_t hash_0 = parcHash32_Data(&encoded_name[0], 9);

    assertTrue(_getRefCount(name) == 2, "Wrong refcount in name, expected %u got %u", 2, _getRefCount(name));
    assertTrue(_getRefCount(copy) == 2, "Wrong refcount in copy, expected %u got %u", 2, _getRefCount(copy));
    assertTrue(copy->segmentArrayLength == copyLength, "Wrong array length, expected %u got %zu", copyLength, copy->segmentArrayLength);
    uint32_t test_hash = metisTlvName_HashCode(copy);
    assertTrue(test_hash == hash_0, "Incorrect hash for segment %d, expected %08X got %08X", 0, hash_0, test_hash);

    metisTlvName_Release(&copy);
    assertTrue(_getRefCount(name) == 1, "Wrong refcount in name, expected %u got %u", 1, _getRefCount(name));
    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMost2)
{
    unsigned copyLength = 2;
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *copy = metisTlvName_Slice(name, copyLength);
    uint32_t hash_0 = parcHash32_Data(&encoded_name[0], 9);
    uint32_t hash_1 = parcHash32_Data_Cumulative(&encoded_name[ 9], 8, hash_0);

    assertTrue(_getRefCount(name) == 2, "Wrong refcount in name, expected %u got %u", 2, _getRefCount(name));
    assertTrue(_getRefCount(copy) == 2, "Wrong refcount in copy, expected %u got %u", 2, _getRefCount(copy));
    assertTrue(copy->segmentArrayLength == copyLength, "Wrong array length, expected %u got %zu", copyLength, copy->segmentArrayLength);
    uint32_t test_hash = metisTlvName_HashCode(copy);
    assertTrue(test_hash == hash_1, "Incorrect hash for segment %d, expected %08X got %08X", 1, hash_1, test_hash);

    metisTlvName_Release(&copy);
    assertTrue(_getRefCount(name) == 1, "Wrong refcount in name, expected %u got %u", 1, _getRefCount(name));
    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_Acquire_CopyAtMostAll)
{
    unsigned copyLength = 3;
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *copy = metisTlvName_Slice(name, UINT_MAX);
    uint32_t hash_0 = parcHash32_Data(&encoded_name[0], 9);
    uint32_t hash_1 = parcHash32_Data_Cumulative(&encoded_name[ 9], 8, hash_0);
    uint32_t hash_2 = parcHash32_Data_Cumulative(&encoded_name[17], 6, hash_1);

    assertTrue(_getRefCount(name) == 2, "Wrong refcount in name, expected %u got %u", 2, _getRefCount(name));
    assertTrue(_getRefCount(copy) == 2, "Wrong refcount in copy, expected %u got %u", 2, _getRefCount(copy));
    assertTrue(copy->segmentArrayLength == copyLength, "Wrong array length, expected %u got %zu", copyLength, copy->segmentArrayLength);
    uint32_t test_hash = metisTlvName_HashCode(copy);
    assertTrue(test_hash == hash_2, "Incorrect hash for segment %d, expected %08X got %08X", 2, hash_2, test_hash);

    metisTlvName_Release(&copy);
    assertTrue(_getRefCount(name) == 1, "Wrong refcount in name, expected %u got %u", 1, _getRefCount(name));
    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_SegmentCount)
{
    MetisTlvName *a = metisTlvName_Create(encoded_name, sizeof(encoded_name));

    size_t count = metisTlvName_SegmentCount(a);
    assertTrue(count == 3, "Incorrect segment count, expected %u got %zu", 3, count);

    metisTlvName_Release(&a);
}

LONGBOW_TEST_CASE(Global, metisTlvName_StartsWith_SelfPrefix)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));

    // a name is always a prefix of itself
    bool success = metisTlvName_StartsWith(name, name);
    assertTrue(success, "Name is not prefix of self in metisTlvName_StartsWith");
    metisTlvName_Release(&name);
}

LONGBOW_TEST_CASE(Global, metisTlvName_StartsWith_IsPrefix)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *prefix = metisTlvName_Create(prefixOf_name, sizeof(prefixOf_name));

    bool success = metisTlvName_StartsWith(name, prefix);
    assertTrue(success, "Valid prefix did not test true in metisTlvName_StartsWith");
    metisTlvName_Release(&name);
    metisTlvName_Release(&prefix);
}

LONGBOW_TEST_CASE(Global, metisTlvName_StartsWith_PrefixTooLong)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *prefix = metisTlvName_Create(prefixOf_name, sizeof(prefixOf_name));

    // we just reversed the prefix and name from the test metisTlvName_StartsWith_IsPrefix,
    // so the prefix is longer than the name
    bool success = metisTlvName_StartsWith(prefix, name);
    assertFalse(success, "Invalid prefix tested true in metisTlvName_StartsWith");
    metisTlvName_Release(&name);
    metisTlvName_Release(&prefix);
}

LONGBOW_TEST_CASE(Global, metisTlvName_StartsWith_IsNotPrefix)
{
    MetisTlvName *name = metisTlvName_Create(encoded_name, sizeof(encoded_name));
    MetisTlvName *other = metisTlvName_Create(second_name, sizeof(second_name));

    // we just reversed the prefix and name from the test metisTlvName_StartsWith_IsPrefix
    bool success = metisTlvName_StartsWith(other, name);
    assertFalse(success, "Invalid prefix tested true in metisTlvName_StartsWith");
    metisTlvName_Release(&name);
    metisTlvName_Release(&other);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_TlvName);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
