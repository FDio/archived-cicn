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
#include <parc/algol/parc_JSONArray.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_JSONValue.h>
#include <parc/algol/parc_DisplayIndented.h>

struct parcJSONArray {
    PARCDeque *array;
};

static void
_destroy(PARCJSONArray **arrayPtr)
{
    PARCJSONArray *array = *arrayPtr;
    // Un-reference the JSONValue instances here because parcDeque doesn't (yet) acquire and release its own references.

    for (int i = 0; i < parcDeque_Size(array->array); i++) {
        PARCJSONValue *value = parcDeque_GetAtIndex(array->array, i);
        parcJSONValue_Release(&value);
    }
    parcDeque_Release(&array->array);
}

parcObject_ExtendPARCObject(PARCJSONArray, _destroy, NULL, parcJSONArray_ToString, parcJSONArray_Equals, NULL, NULL, NULL);

static const PARCObjectDescriptor parcArrayValue_ObjInterface = {
    .destroy  = (PARCObjectDestroy *) parcJSONValue_Release,
    .toString = (PARCObjectToString *) parcJSONValue_ToString,
    .equals   = (PARCObjectEquals *) parcJSONValue_Equals
};

void
parcJSONArray_AssertValid(const PARCJSONArray *array)
{
    parcAssertNotNull(array, "Must be a non-null pointer to a PARCJSONArray instance.");
    parcAssertNotNull(array->array, "Must be a non-null pointer to a PARCDeque instance.");
}

PARCJSONArray *
parcJSONArray_Create(void)
{
    PARCJSONArray *result = parcObject_CreateInstance(PARCJSONArray);
    result->array = parcDeque_CreateObjectInterface(&parcArrayValue_ObjInterface);
    return result;
}

parcObject_ImplementAcquire(parcJSONArray, PARCJSONArray);

parcObject_ImplementRelease(parcJSONArray, PARCJSONArray);

bool
parcJSONArray_Equals(const PARCJSONArray *x, const PARCJSONArray *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        result = parcDeque_Equals(x->array, y->array);
    }
    return result;
}

PARCJSONArray *
parcJSONArray_AddValue(PARCJSONArray *array, PARCJSONValue *value)
{
    parcDeque_Append(array->array, parcJSONValue_Acquire(value));
    return array;
}

size_t
parcJSONArray_GetLength(const PARCJSONArray *array)
{
    return parcDeque_Size(array->array);
}

PARCJSONValue *
parcJSONArray_GetValue(const PARCJSONArray *array, size_t index)
{
    return (PARCJSONValue *) parcDeque_GetAtIndex(array->array, index);
}

PARCBufferComposer *
parcJSONArray_BuildString(const PARCJSONArray *array, PARCBufferComposer *composer, bool compact)
{
#ifdef hasPARCIterator
    PARCIterator *iterator = parcDeque_GetIterator(array->array);

    parcBufferComposer_PutChar(composer, '[');

    char *separator = "";

    for (i = parcIterator_Start(); i < parcIterator_Limit(iterator); i = parcIterator_Next(iterator)) {
        PARCJSONValue *value = parcIterator_Get(iterator);
        parcBufferComposer_PutString(composer, separator);
        separator = ", ";
        if (compact) {
            separator = ",";
        }

        parcJSONValue_BuildString(value, composer);
    }
    parcBufferComposer_PutChar(composer, ']');
#else
    parcBufferComposer_PutChar(composer, '[');
    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }

    char *separator = "";

    for (int i = 0; i < parcDeque_Size(array->array); i++) {
        PARCJSONValue *value = parcDeque_GetAtIndex(array->array, i);
        parcBufferComposer_PutString(composer, separator);

        parcJSONValue_BuildString(value, composer, compact);
        separator = ", ";
        if (compact) {
            separator = ",";
        }
    }
    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }
    parcBufferComposer_PutChar(composer, ']');
#endif
    return composer;
}

void
parcJSONArray_Display(const PARCJSONArray *array, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCJSONArray@%p {", array);
    parcDisplayIndented_PrintLine(indentation, "}");
}

static char *
_parcJSONArray_ToString(const PARCJSONArray *array, bool compact)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcJSONArray_BuildString(array, composer, compact);
    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    return result;
}

char *
parcJSONArray_ToString(const PARCJSONArray *array)
{
    return _parcJSONArray_ToString(array, false);
}

char *
parcJSONArray_ToCompactString(const PARCJSONArray *array)
{
    return _parcJSONArray_ToString(array, true);
}
