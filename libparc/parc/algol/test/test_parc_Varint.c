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

#include "../parc_Varint.c"

#include <LongBow/unit-test.h>

#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_BufferComposer.h>

LONGBOW_TEST_RUNNER(parc_VarInt)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

LONGBOW_TEST_RUNNER_SETUP(parc_VarInt)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parc_VarInt)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_And);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AndUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AndUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AndUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AndUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AsSize);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AsUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AsUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AsUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_AsUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_Destroy);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_Equals);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_EqualsUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_EqualsUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_EqualsUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_EqualsUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_FromByteBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_FromUTF8ByteBuffer);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_FromUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_FromUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_FromUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_New);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_Or);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_OrUint16);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_OrUint32);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_OrUint64);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_OrUint8);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_Set);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_ShiftLeft);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_ShiftRight);
    LONGBOW_RUN_TEST_CASE(Global, parcVarint_ToString);
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

LONGBOW_TEST_CASE(Global, parcVarint_And)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AndUint16)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AndUint32)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AndUint64)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AndUint8)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AsSize)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AsUint16)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AsUint32)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AsUint64)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_AsUint8)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_Destroy)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_Equals)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_EqualsUint16)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_EqualsUint32)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_EqualsUint64)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_EqualsUint8)
{
    unsigned char expected = 5;
    PARCVarint *a = parcVarint_FromUint8(expected);
    PARCVarint *b = parcVarint_FromUint8(expected);

    assertTrue(parcVarint_Equals(a, b), "Equal instances failed to be equal.");
    parcVarint_Destroy(&a);
    parcVarint_Destroy(&b);
}

LONGBOW_TEST_CASE(Global, parcVarint_FromByteBuffer)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_FromUTF8ByteBuffer)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcBufferComposer_PutString(composer, "10");
    PARCBuffer *buffer = parcBufferComposer_ProduceBuffer(composer);

    PARCVarint *varint = parcVarint_FromUTF8ByteBuffer(buffer);

    uint32_t actual = parcVarint_AsUint32(varint);

    assertTrue(10 == actual, "Expected 10 actual %u", actual);
    parcBuffer_Release(&buffer);
    parcBufferComposer_Release(&composer);
    parcVarint_Destroy(&varint);
}

LONGBOW_TEST_CASE(Global, parcVarint_FromUint32)
{
    uint32_t value = 0x12345678;
    PARCVarint *a = parcVarint_FromUint32(value);
    assertNotNull(a, "Probably out of memory.");

    uint32_t actual = parcVarint_AsUint32(a);

    assertEqual(value, actual, "%u");

    parcVarint_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, parcVarint_FromUint64)
{
    uint64_t value = 0x1234567812345678;
    PARCVarint *a = parcVarint_FromUint64(value);
    assertNotNull(a, "Probably out of memory.");

    uint64_t actual = parcVarint_AsUint64(a);

    assertEqual(value, actual, "%" PRIu64);

    parcVarint_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, parcVarint_FromUint8)
{
    uint8_t value = 0x12;
    PARCVarint *a = parcVarint_FromUint8(value);
    assertNotNull(a, "Probably out of memory.");

    uint8_t actual = parcVarint_AsUint8(a);

    assertEqual(value, actual, "%d");

    parcVarint_Destroy(&a);
}

LONGBOW_TEST_CASE(Global, parcVarint_New)
{
    PARCVarint *a = parcVarint_Create();
    assertNotNull(a, "Probably out of memory.");

    parcVarint_Destroy(&a);
    assertNull(a, "Destroy failed to nullify.");
}

LONGBOW_TEST_CASE(Global, parcVarint_Or)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_OrUint16)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_OrUint32)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_OrUint64)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_OrUint8)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_Set)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_ShiftLeft)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_ShiftRight)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Global, parcVarint_ToString)
{
    testUnimplemented("This test is unimplemented");
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
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_VarInt);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
