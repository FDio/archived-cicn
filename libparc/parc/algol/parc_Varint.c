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

#include <LongBow/runtime.h>

#include <stdio.h>
#include <stdlib.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <parc/algol/parc_Varint.h>
#include <parc/algol/parc_Memory.h>

struct parc_varint {
    uint64_t value;
};

PARCVarint *
parcVarint_Create(void)
{
    PARCVarint *result = parcMemory_AllocateAndClear(sizeof(PARCVarint));
    if (result != NULL) {
        result->value = 0;
    }
    return result;
}

PARCVarint *
parcVarint_DecodeBuffer(PARCBuffer *buffer, size_t length)
{
    assertNotNull(buffer, "Parameter must be a non-null PARCBuffer pointer.");
    assertTrue(length < sizeof(size_t), "Length must be less then or equal to %zd", sizeof(size_t));
    assertTrue(length <= parcBuffer_Remaining(buffer), "Buffer does not contain at least %zd bytes", length);

    PARCVarint *result = parcVarint_Create();
    assertNotNull(result, "PARCVarint out of memory.");

    for (size_t i = 0; i < length; i++) {
        parcVarint_ShiftLeft(result, 8);
        parcVarint_OrUint8(result, parcBuffer_GetUint8(buffer));
    }

    return result;
}

PARCVarint *
parcVarint_DecodeElasticByteBuffer(const PARCBuffer *buffer, size_t length)
{
    assertNotNull(buffer, "Parameter must be a non-null PARCBuffer pointer.");
    assertTrue(length < sizeof(size_t), "Length must be less then or equal to %zd", sizeof(size_t));

    PARCVarint *result = parcVarint_Create();
    assertNotNull(result, "PARCVarint out of memory.");

    for (size_t i = 0; i < length; i++) {
        parcVarint_ShiftLeft(result, 8);
        parcVarint_OrUint8(result, parcBuffer_GetAtIndex(buffer, i));
    }

    return result;
}

PARCVarint *
parcVarint_Set(PARCVarint *varint, uint64_t newValue)
{
    varint->value = newValue;
    return varint;
}

PARCVarint *
parcVarint_FromElasticByteBuffer(const PARCBuffer *buffer)
{
    assertNotNull(buffer, "Parameter must be a non-null PARCBuffer pointer.");
    PARCVarint *result = parcVarint_Create();

    size_t length = parcBuffer_Remaining(buffer);

    for (size_t i = 0; i < length; i++) {
        parcVarint_ShiftLeft(result, 8);
        parcVarint_OrUint8(result, parcBuffer_GetAtIndex(buffer, i));
    }

    return result;
}

PARCVarint *
parcVarint_FromUTF8ByteBuffer(const PARCBuffer *buffer)
{
    assertNotNull(buffer, "Parameter must not be NULL.");
    PARCVarint *result = parcVarint_Create();

    if (result != NULL) {
        size_t length = parcBuffer_Remaining(buffer);

        for (size_t i = 0; i < length; i++) {
            parcVarint_Multiply(result, 10);
            parcVarint_Add(result, parcBuffer_GetAtIndex(buffer, i) - '0');
        }
    }

    return result;
}

PARCVarint *
parcVarint_FromUTF8Buffer(PARCBuffer *buffer)
{
    assertNotNull(buffer, "Parameter must be a non-null PARCBuffer pointer.");
    PARCVarint *result = parcVarint_Create();

    if (result != NULL) {
        size_t length = parcBuffer_Limit(buffer);

        for (size_t i = 0; i < length; i++) {
            parcVarint_Multiply(result, 10);
            parcVarint_Add(result, parcBuffer_GetAtIndex(buffer, i) - '0');
        }
    }

    return result;
}

/**
 *
 * @param uint
 * @return
 */
PARCVarint *
parcVarint_FromUint8(uint8_t uint)
{
    return parcVarint_FromUint32(uint);
}

/**
 *
 * @param uint
 * @return
 */
PARCVarint *
parcVarint_FromUint32(uint32_t uint)
{
    return parcVarint_FromUint64(uint);
}

/**
 *
 * @param uint
 * @return
 */
PARCVarint *
parcVarint_FromUint64(uint64_t uint)
{
    PARCVarint *result = parcMemory_AllocateAndClear(sizeof(PARCVarint));
    if (result != NULL) {
        result->value = uint;
    }
    return result;
}

/**
 *
 * @param varintP
 */
void
parcVarint_Destroy(PARCVarint **varintP)
{
    assertNotNull(varintP, "Parameter must be a non-null pointer to a pointer to a PARCVarint");
    assertNotNull(*varintP, "Parameter must be a non-null pointer to a PARCVarint");

    parcMemory_Deallocate((void **) varintP);
    *varintP = NULL;
}

/**
 * Shift the value {@code bits} to the left.
 *
 * @param varint
 * @param bits
 * @return
 */
PARCVarint *
parcVarint_ShiftLeft(PARCVarint *varint, int bits)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value <<= bits;

    return varint;
}

PARCVarint *
parcVarint_Add(PARCVarint *varint, int addend)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value += addend;

    return varint;
}

PARCVarint *
parcVarint_Subtract(PARCVarint *varint, int subtrahend)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value -= subtrahend;

    return varint;
}

PARCVarint *
parcVarint_Multiply(PARCVarint *varint, int multiplicand)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value *= multiplicand;

    return varint;
}

PARCVarint *
parcVarint_Divide(PARCVarint *varint, int divisor)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value /= divisor;

    return varint;
}

/**
 * Shift the value {@code bits} to the right.
 *
 * @param varint
 * @param bits
 * @return
 */
PARCVarint *
parcVarint_ShiftRight(PARCVarint *varint, int bits)
{
    assertNotNull(varint, "Parameter must be a non-null pointer to a PARCVarint.");
    varint->value >>= bits;
    return varint;
}

/**
 * Perform an AND operation on the given {@link PARCVarint} with the supplied {@code operand},
 * leaving the result in {@code varint}.
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_And(PARCVarint *varint, PARCVarint *operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    assertNotNull(operand, "Parameter operand must not be NULL.");
    varint->value &= operand->value;
    return varint;
}

/**
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_AndUint8(PARCVarint *varint, uint8_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value &= operand;
    return varint;
}

/**
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_AndUint16(PARCVarint *varint, uint16_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value &= operand;
    return varint;
}

/**
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_AndUint32(PARCVarint *varint, uint32_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value &= operand;
    return varint;
}

/**
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_AndUint64(PARCVarint *varint, uint64_t operand)
{
    assertNotNull(varint, "Parameter must be a non-null PARCVarint pointer.");
    varint->value &= operand;
    return varint;
}

/**
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_Or(PARCVarint *varint, PARCVarint *operand)
{
    assertNotNull(varint, "Parameter must be a non-null PARCVarint pointer.");
    varint->value |= operand->value;
    return varint;
}

/**
 * Perform an OR operation on the low 8-bits in the given {@link PARCVarint}.
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_OrUint8(PARCVarint *varint, uint8_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value |= operand;
    return varint;
}

/**
 * Perform an OR operation on the low 16-bits in the given {@link PARCVarint}.
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_OrUint16(PARCVarint *varint, uint16_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value |= operand;
    return varint;
}

/**
 * Perform an OR operation on the low 32-bits in the given {@link PARCVarint}.
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_OrUint32(PARCVarint *varint, uint32_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value |= operand;
    return varint;
}

/**
 * Perform an OR operation on the low 64-bits in the given {@link PARCVarint}.
 *
 * @param varint
 * @param operand
 * @return
 */
PARCVarint *
parcVarint_OrUint64(PARCVarint *varint, uint64_t operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    varint->value |= operand;
    return varint;
}

/**
 * Return {@code true} (non-zero) if the two {@link PARCVarint} structures contain equal data.
 *
 * @param varint
 * @param operand
 * @return
 */
int
parcVarint_Equals(PARCVarint *varint, PARCVarint *operand)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return varint->value == operand->value;
}

/**
 *
 * @param varint
 * @param value
 * @return
 */
int
parcVarint_EqualsUint64(PARCVarint *varint, uint64_t value)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return varint->value == value;
}

/**
 *
 * @param varint
 * @param value
 * @return
 */
int
parcVarint_EqualsUint32(PARCVarint *varint, uint32_t value)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return parcVarint_EqualsUint64(varint, (uint64_t) value);
}

/**
 *
 * @param varint
 * @param value
 * @return
 */
int
parcVarint_EqualsUint16(PARCVarint *varint, uint16_t value)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return parcVarint_EqualsUint64(varint, (uint64_t) value);
}

/**
 *
 * @param varint
 * @param value
 * @return
 */
int
parcVarint_EqualsUint8(PARCVarint *varint, uint8_t value)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return parcVarint_EqualsUint64(varint, (uint64_t) value);
}

/**
 * Produce the 8 low-order bits of this {@code PARCVarint}.
 *
 * @param varint
 * @return
 */
uint8_t
parcVarint_AsUint8(const PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return (uint8_t) varint->value;
}

/**
 * Produce the 16 low-order bits of this {@code PARCVarint}.
 *
 * @param varint
 * @return
 */
uint16_t
parcVarint_AsUint16(const PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return (uint16_t) varint->value;
}

/**
 * Produce the 32 low-order bits of this {@code PARCVarint}.
 *
 * @param varint
 * @return
 */
uint32_t
parcVarint_AsUint32(const PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return (uint32_t) varint->value;
}

uint64_t
parcVarint_AsUint64(const PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return varint->value;
}

/**
 * Produce the value of this {@code PARCVarint} as a {@code size_t} value.
 *
 * @param varint
 * @return
 */
size_t
parcVarint_AsSize(const PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    return (size_t) varint->value;
}

/**
 * Produce a string representation of this {@link PARCVarint}.
 * The returned value must be freed by the called using the {@code stdlib.h} {@code free}.
 * @param string
 * @param varint
 * @return
 */
char *
parcVarint_ToString(char **string, PARCVarint *varint)
{
    assertNotNull(varint, "Parameter varint must not be NULL.");
    int nwritten = asprintf(string, "%" PRIu64, varint->value);
    assertTrue(nwritten >= 0, "Error calling asprintf");
    return *string;
}
