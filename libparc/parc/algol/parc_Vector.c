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

#include <parc/assert/parc_Assert.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Vector.h>

struct parc_vector {
    const void *pointer;
    size_t length;
};

PARCVector *
parcVector_Create(const void *pointer, const size_t length)
{
    PARCVector *result = parcMemory_AllocateAndClear(sizeof(PARCVector));
    if (result != NULL) {
        parcVector_Init(result, pointer, length);
    }

    return result;
}

PARCVector *
parcVector_Init(PARCVector *vector, const void *pointer, const size_t length)
{
    parcAssertNotNull(vector, "Parameter must be a non-null PARCVector pointer");

    vector->pointer = pointer;
    vector->length = length;
    return vector;
}

void
parcVector_Destroy(PARCVector **vectorPtr)
{
    parcAssertNotNull(vectorPtr, "Parameter must be a non-null PARCVector pointer");
    PARCVector *vector = *vectorPtr;
    parcAssertNotNull(vector, "Vector is already free or was not set.\n");

    parcMemory_Deallocate((void **) &vector);
    *vectorPtr = NULL;
}

const void *
parcVector_GetPointer(const PARCVector *vector)
{
    parcAssertNotNull(vector, "Parameter must be a non-null PARCVector pointer.");
    return vector->pointer;
}

size_t
parcVector_GetLength(const PARCVector *vector)
{
    parcAssertNotNull(vector, "Parameter must be a non-null PARCVector pointer.");

    return vector->length;
}

