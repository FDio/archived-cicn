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
#include <config.h>

#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>

#include <LongBow/unit-test.h>

#include "../parc_HashCodeTable.c"

#include <parc/algol/parc_SafeMemory.h>

// ==============================
// The objects to put in the hash table.  We have a separate key class and data class

typedef struct test_key_class {
    unsigned key_value;
    unsigned hash_value;
} TestKeyClass;

typedef struct test_data_class {
    unsigned data_value;
} TestDataClass;

static bool
TestKeyClass_Equals(const void *a, const void *b)
{
    return ((TestKeyClass *) a)->key_value == ((TestKeyClass *) b)->key_value;
}

static HashCodeType
TestKeyClass_Hash(const void *a)
{
    return ((TestKeyClass *) a)->hash_value;
}

static void
TestKeyClassDestroy(void **aPtr)
{
    parcMemory_Deallocate((void **) aPtr);
    aPtr = NULL;
}

static void
TestDataClassDestroy(void **aPtr)
{
    parcMemory_Deallocate((void **) aPtr);
    aPtr = NULL;
}

typedef struct truth_table_entry {
    unsigned key_value;
    unsigned hash_code;
    unsigned data_value;
} TruthTableEntry;

// ==============================


LONGBOW_TEST_RUNNER(parc_HashCodeTable)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_HashCodeTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_HashCodeTable)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Add_Get);
    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Create_Size);
    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Del);

    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Add_DuplicateHashes);
    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_Add_DuplicateValues);

    LONGBOW_RUN_TEST_CASE(Global, parcHashCodeTable_BigTable);
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

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Add_Get)
{
    const int testsize = 4096;
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);
    TruthTableEntry *truthtable = parcMemory_Allocate(sizeof(TruthTableEntry) * testsize);
    assertNotNull(truthtable, "parcMemory_Allocate(%zu) returned NULL", sizeof(TruthTableEntry) * testsize);
    int i;
    int fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1) {
        assertFalse(fd == -1, "Error opening random number generator: %s", strerror(errno));
    }

    ssize_t nread = read(fd, truthtable, sizeof(TruthTableEntry) * testsize);
    assertTrue(nread > 0, "Error using read");
    close(fd);

    for (i = 0; i < testsize; i++) {
        TestKeyClass *key = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
        assertNotNull(key, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
        TestDataClass *data = parcMemory_AllocateAndClear(sizeof(TestDataClass));
        assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));

        key->key_value = truthtable[i].key_value;
        key->hash_value = truthtable[i].hash_code;
        data->data_value = truthtable[i].data_value;

        bool success = parcHashCodeTable_Add(table, key, data);
        assertTrue(success, "Failed inserting value");
    }

    // now retrieve them
    for (i = 0; i < testsize; i++) {
        TestKeyClass lookupkey;
        lookupkey.key_value = truthtable[i].key_value;
        lookupkey.hash_value = truthtable[i].hash_code;

        TestDataClass *data = parcHashCodeTable_Get(table, &lookupkey);
        assertTrue(data->data_value == truthtable[i].data_value, "Data value incorrect");
    }

    parcHashCodeTable_Destroy(&table);
    parcMemory_Deallocate((void **) &truthtable);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Create)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);

    assertNotNull(table, "table came back as null");
    assertTrue(table->hashtable.tableSize == 0, "Hash table initialized to wrong size");
    assertTrue(table->hashtable.tableLimit == MIN_SIZE, "Initial table limit size is wrong");
    assertTrue(table->keyEqualsFunc == TestKeyClass_Equals, "KeyEqualsFunc wrong");
    assertTrue(table->keyHashCodeFunc == TestKeyClass_Hash, "KeyHashFunc wrong");
    assertTrue(table->keyDestroyer == TestKeyClassDestroy, "KeyDestroyer wrong");
    assertTrue(table->dataDestroyer == TestDataClassDestroy, "DataDestroyer wrong");

    parcHashCodeTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Create_Size)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create_Size(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy, 16);

    assertNotNull(table, "table came back as null");
    assertTrue(table->hashtable.tableLimit == 16, "Initial table limit size is wrong");
    parcHashCodeTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Del)
{
    const int testsize = 6;
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);
    TruthTableEntry *truthtable = parcMemory_Allocate(sizeof(TruthTableEntry) * testsize);
    assertNotNull(truthtable, "parcMemory_Allocate(%zu) returned NULL", sizeof(TruthTableEntry) * testsize);
    int i;
    int fd = open("/dev/urandom", O_RDONLY);

    if (fd == -1) {
        assertFalse(fd == -1, "Error opening random number generator: %s", strerror(errno));
    }

    ssize_t nread = read(fd, truthtable, sizeof(TruthTableEntry) * testsize);
    assertTrue(nread > 0, "Error using read");
    close(fd);

    for (i = 0; i < testsize; i++) {
        TestKeyClass *key = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
        assertNotNull(key, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
        TestDataClass *data = parcMemory_AllocateAndClear(sizeof(TestDataClass));
        assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));

        key->key_value = truthtable[i].key_value;
        key->hash_value = truthtable[i].hash_code;
        data->data_value = truthtable[i].data_value;

        bool success = parcHashCodeTable_Add(table, key, data);
        assertTrue(success, "Failed inserting value");
    }

    // delete the last one
    {
        TestKeyClass lookupkey;

        lookupkey.key_value = truthtable[testsize - 1].key_value;
        lookupkey.hash_value = truthtable[testsize - 1].hash_code;

        parcHashCodeTable_Del(table, &lookupkey);
        assertTrue(table->hashtable.tableSize == testsize - 1, "tableSize wrong");
    }

    for (i = 0; i < testsize - 1; i++) {
        TestKeyClass lookupkey;
        lookupkey.key_value = truthtable[i].key_value;
        lookupkey.hash_value = truthtable[i].hash_code;

        TestDataClass *data = parcHashCodeTable_Get(table, &lookupkey);
        assertTrue(data->data_value == truthtable[i].data_value, "Data value incorrect");
    }

    for (i = testsize - 1; i < testsize; i++) {
        TestKeyClass lookupkey;
        lookupkey.key_value = truthtable[i].key_value;
        lookupkey.hash_value = truthtable[i].hash_code;

        TestDataClass *data = parcHashCodeTable_Get(table, &lookupkey);
        assertNull(data, "Should not have returned deleted value");
    }
    parcHashCodeTable_Destroy(&table);
    parcMemory_Deallocate((void **) &truthtable);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Add_DuplicateHashes)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);

    TestKeyClass *key1 = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
    assertNotNull(key1, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
    TestKeyClass *key2 = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
    assertNotNull(key2, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
    TestDataClass *data1 = parcMemory_AllocateAndClear(sizeof(TestDataClass));
    assertNotNull(data1, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));
    TestDataClass *data2 = parcMemory_AllocateAndClear(sizeof(TestDataClass));
    assertNotNull(data2, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));
    TestDataClass *test;
    bool success;

    *key1 = (TestKeyClass) { .key_value = 1, .hash_value = 2 };
    *key2 = (TestKeyClass) { .key_value = 3, .hash_value = 2 };
    data1->data_value = 11;
    data2->data_value = 22;

    success = parcHashCodeTable_Add(table, key1, data1);
    assertTrue(success, "Failed to add value");

    success = parcHashCodeTable_Add(table, key2, data2);
    assertTrue(success, "Failed to add value");

    // value 3 should be in position 3
    test = parcHashCodeTable_Get(table, key1);
    assertNotNull(test, "returned null on get");
    assertTrue(test->data_value == 11, "Got wrong value back for key1");

    test = parcHashCodeTable_Get(table, key2);
    assertNotNull(test, "returned null on get");
    assertTrue(test->data_value == 22, "Got wrong value back for key1");

    parcHashCodeTable_Destroy(&table);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_Add_DuplicateValues)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);

    TestKeyClass *key1 = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
    assertNotNull(key1, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
    TestKeyClass *key2 = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
    assertNotNull(key2, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
    TestDataClass *data1 = parcMemory_AllocateAndClear(sizeof(TestDataClass));
    assertNotNull(data1, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));
    TestDataClass *data2 = parcMemory_AllocateAndClear(sizeof(TestDataClass));
    assertNotNull(data2, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));
    TestDataClass *test;
    bool success;

    *key1 = (TestKeyClass) { .key_value = 1, .hash_value = 2 };
    *key2 = (TestKeyClass) { .key_value = 1, .hash_value = 2 };
    data1->data_value = 11;
    data2->data_value = 22;

    success = parcHashCodeTable_Add(table, key1, data1);
    assertTrue(success, "Failed to add value");

    success = parcHashCodeTable_Add(table, key2, data2);
    assertFalse(success, "Second add should have failed on duplicate key");

    // value 3 should be in position 3
    test = parcHashCodeTable_Get(table, key1);
    assertNotNull(test, "returned null on get");
    assertTrue(test->data_value == 11, "Got wrong value back for key1");

    parcHashCodeTable_Destroy(&table);
    parcMemory_Deallocate((void **) &key2);
    parcMemory_Deallocate((void **) &data2);
}

LONGBOW_TEST_CASE(Global, parcHashCodeTable_BigTable)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);

    struct timeval t0, t1;

    gettimeofday(&t0, NULL);

    int loops = 1000;
    for (int i = 0; i < loops; i++) {
        TestKeyClass *key = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
        assertNotNull(key, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
        TestDataClass *data = parcMemory_AllocateAndClear(sizeof(TestDataClass));
        assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));

        *key = (TestKeyClass) { .key_value = i, .hash_value = i };
        data->data_value = i;

        bool success = parcHashCodeTable_Add(table, key, data);
        assertTrue(success, "Failed to add value");
    }
    gettimeofday(&t1, NULL);

    timersub(&t1, &t0, &t1);
    double sec = t1.tv_sec + t1.tv_usec * 1E-6;
    printf("expand count %u, sec = %.3f, sec/add = %.9f\n", table->expandCount, sec, sec / loops);

    gettimeofday(&t0, NULL);
    parcHashCodeTable_Destroy(&table);
    gettimeofday(&t1, NULL);

    timersub(&t1, &t0, &t1);
    sec = t1.tv_sec + t1.tv_usec * 1E-6;
    printf("destroy sec = %.3f, sec/add = %.9f\n", sec, sec / loops);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _findIndex);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _findIndex)
{
    PARCHashCodeTable *table = parcHashCodeTable_Create(TestKeyClass_Equals, TestKeyClass_Hash, TestKeyClassDestroy, TestDataClassDestroy);

    // stick a data element in the middle of the table
    TestKeyClass *key = parcMemory_AllocateAndClear(sizeof(TestKeyClass));
    assertNotNull(key, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestKeyClass));
    TestDataClass *data = parcMemory_AllocateAndClear(sizeof(TestDataClass));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestDataClass));

    key->key_value = 1;
    key->hash_value = 37;
    data->data_value = 7;

    table->hashtable.entries[37].key = key;
    table->hashtable.entries[37].hashcode = key->hash_value;
    table->hashtable.entries[37].data = data;

    size_t index;
    bool success = _findIndex(table, key, &index);
    assertTrue(success, "FindIndex did not find known value");
    assertTrue(index == 37, "FindIndex returned wrong value");


    parcHashCodeTable_Destroy(&table);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_HashCodeTable);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
