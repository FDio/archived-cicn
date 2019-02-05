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

#include "../parc_Hash.c"

#include <LongBow/testing.h>
#include <stdio.h>

#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(parc_Hash)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Hash)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Hash)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_Create);
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_Acquire);
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_Release);
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_Update);
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_UpdateUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcHash32Bit_Hash);

    LONGBOW_RUN_TEST_CASE(Global, parc_Hash32_Data);
    LONGBOW_RUN_TEST_CASE(Global, parc_Hash32_Int32);
    LONGBOW_RUN_TEST_CASE(Global, parc_Hash32_Int64);
    LONGBOW_RUN_TEST_CASE(Global, parc_Hash64_Data);
    LONGBOW_RUN_TEST_CASE(Global, parc_Hash64_Int32);
    LONGBOW_RUN_TEST_CASE(Global, parc_Hash64_Int64);
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

LONGBOW_TEST_CASE(Global, parcHash32Bit_Create)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcHash32Bit_Acquire)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcHash32Bit_Release)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parcHash32Bit_Update)
{
    PARCHash32Bits *hash = parcHash32Bits_Create();
    parcHash32Bits_Update(hash, "123", 3);
    uint32_t value = parcHash32Bits_Hash(hash);
    assertTrue(value != 0, "Expected a non-zero (non-initial) value");

    parcHash32Bits_Release(&hash);
}

LONGBOW_TEST_CASE(Global, parcHash32Bit_UpdateUint32)
{
    PARCHash32Bits *hash = parcHash32Bits_Create();
    parcHash32Bits_UpdateUint32(hash, 123);
    uint32_t value = parcHash32Bits_Hash(hash);
    assertTrue(value != 0, "Expected a non-zero (non-initial) value");

    parcHash32Bits_Release(&hash);
}

LONGBOW_TEST_CASE(Global, parcHash32Bit_Hash)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, parc_Hash32_Data)
{
    char *data1 = "Hello World";
    char *data2 = "Hello World1";
    char *data3 = "Hello World2";

    char data4[20];
    strncpy(data4, data1, sizeof(data4));

    uint32_t hash1 = parcHash32_Data(data1, strlen(data1));
    uint32_t hash2 = parcHash32_Data(data2, strlen(data2));
    uint32_t hash3 = parcHash32_Data(data3, strlen(data3));
    uint32_t hash4 = parcHash32_Data(data4, strlen(data4));

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash4 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash3 != hash2, "Hash collision, unlikely");
    assertTrue(hash3 != hash1, "Hash collision, unlikely");
    assertTrue(hash1 == hash4, "Hash different for same content");
}

LONGBOW_TEST_CASE(Global, parc_Hash32_Int32)
{
    uint32_t data1 = 12345;
    uint32_t data2 = 12346;
    uint32_t data3 = 12345;

    uint32_t hash1 = parcHash32_Int32(data1);
    uint32_t hash2 = parcHash32_Int32(data2);
    uint32_t hash3 = parcHash32_Int32(data3);

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash1 == hash3, "Hash different for same content");
}

LONGBOW_TEST_CASE(Global, parc_Hash32_Int64)
{
    uint64_t data1 = 10010010012345;
    uint64_t data2 = 10010010012346;
    uint64_t data3 = 10010010012345;

    uint32_t hash1 = parcHash32_Int64(data1);
    uint32_t hash2 = parcHash32_Int64(data2);
    uint32_t hash3 = parcHash32_Int64(data3);

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash1 == hash3, "Hash different for same content");
}

LONGBOW_TEST_CASE(Global, parc_Hash64_Data)
{
    char *data1 = "Hello World";
    char *data2 = "Hello World1";
    char *data3 = "Hello World2";

    char data4[20];
    strncpy(data4, data1, sizeof(data4));

    uint64_t hash1 = parcHash64_Data(data1, strlen(data1));
    uint64_t hash2 = parcHash64_Data(data2, strlen(data2));
    uint64_t hash3 = parcHash64_Data(data3, strlen(data3));
    uint64_t hash4 = parcHash64_Data(data4, strlen(data4));

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash4 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash3 != hash2, "Hash collision, unlikely");
    assertTrue(hash3 != hash1, "Hash collision, unlikely");
    assertTrue(hash1 == hash4, "Hash different for same content");
}

LONGBOW_TEST_CASE(Global, parc_Hash64_Int32)
{
    uint32_t data1 = 12345;
    uint32_t data2 = 12346;
    uint32_t data3 = 12345;

    uint64_t hash1 = parcHash64_Int32(data1);
    uint64_t hash2 = parcHash64_Int32(data2);
    uint64_t hash3 = parcHash64_Int32(data3);

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash1 == hash3, "Hash different for same content");
}

LONGBOW_TEST_CASE(Global, parc_Hash64_Int64)
{
    uint64_t data1 = 10010010012345;
    uint64_t data2 = 10010010012346;
    uint64_t data3 = 10010010012345;

    uint64_t hash1 = parcHash64_Int64(data1);
    uint64_t hash2 = parcHash64_Int64(data2);
    uint64_t hash3 = parcHash64_Int64(data3);

    assertTrue(hash1 != 0, "Hash is 0, unlikely");
    assertTrue(hash2 != 0, "Hash is 0, unlikely");
    assertTrue(hash3 != 0, "Hash is 0, unlikely");
    assertTrue(hash1 != hash2, "Hash collision, unlikely");
    assertTrue(hash1 == hash3, "Hash different for same content");
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
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Hash);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
