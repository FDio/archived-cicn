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
 * This does not properly implement the equals contract because the JSON object is stored in a PARCArrayList,
 * which cannot compare elements other than by equal memory address.
 *
 */
#include <config.h>

#include <parc/assert/parc_Assert.h>

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_JSONPair.h>
#include <parc/algol/parc_JSONValue.h>
#include <parc/algol/parc_JSONParser.h>

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_BufferComposer.h>

struct parcJSONPair {
    PARCBuffer *name;
    PARCJSONValue *value;
};

static void
_destroyJSONPair(PARCJSONPair **pairPtr)
{
    if (pairPtr != NULL) {
        PARCJSONPair *pair = *pairPtr;
        parcAssertNotNull(pair, "Parameter must be a non-null pointer to a valid PARCJSONPair.");
        parcBuffer_Release(&pair->name);
        parcJSONValue_Release(&pair->value);
    }
}

parcObject_ExtendPARCObject(PARCJSONPair, _destroyJSONPair, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCJSONPair *
_createJSONPair(void)
{
    return parcObject_CreateInstance(PARCJSONPair);
}

PARCJSONPair *
parcJSONPair_Create(const PARCBuffer *name, PARCJSONValue *value)
{
    PARCJSONPair *result = _createJSONPair();
    if (result != NULL) {
        result->name = parcBuffer_Acquire(name);
        result->value = parcJSONValue_Acquire(value);
    }

    return result;
}

parcObject_ImplementAcquire(parcJSONPair, PARCJSONPair);

parcObject_ImplementRelease(parcJSONPair, PARCJSONPair);

void
parcJSONPair_Display(const PARCJSONPair *pair, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCJSONPair@%p {", pair);
    parcBuffer_Display(pair->name, indentation + 1);
    parcJSONValue_Display(pair->value, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}");
}

PARCBuffer *
parcJSONPair_GetName(const PARCJSONPair *pair)
{
    return pair->name;
}

PARCJSONValue *
parcJSONPair_GetValue(const PARCJSONPair *pair)
{
    return pair->value;
}

PARCJSONPair *
parcJSONPair_CreateFromJSONValue(const char *name, PARCJSONValue *value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONPair *result = parcJSONPair_Create(nameBuffer, value);
    parcBuffer_Release(&nameBuffer);

    return result;
}

PARCJSONPair *
parcJSONPair_CreateFromString(const char *name, const char *value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);

    PARCBuffer *valueBuffer = parcBuffer_AllocateCString(value);

    PARCJSONValue *jsonValue = parcJSONValue_CreateFromString(valueBuffer);
    parcBuffer_Release(&valueBuffer);

    PARCJSONPair *result = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return result;
}

PARCJSONPair *
parcJSONPair_CreateFromNULL(const char *name)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromNULL();
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

PARCJSONPair *
parcJSONPair_CreateFromBoolean(const char *name, bool value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromBoolean(value);
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

PARCJSONPair *
parcJSONPair_CreateFromInteger(const char *name, int64_t value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromInteger(value);
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

PARCJSONPair *
parcJSONPair_CreateFromDouble(const char *name, double value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromFloat(value);
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

PARCJSONPair *
parcJSONPair_CreateFromJSONArray(const char *name, PARCJSONArray *value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromJSONArray(value);
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

PARCJSONPair *
parcJSONPair_CreateFromJSON(const char *name, PARCJSON *value)
{
    PARCBuffer *nameBuffer = parcBuffer_AllocateCString(name);
    PARCJSONValue *jsonValue = parcJSONValue_CreateFromJSON(value);
    PARCJSONPair *pair = parcJSONPair_Create(nameBuffer, jsonValue);
    parcBuffer_Release(&nameBuffer);
    parcJSONValue_Release(&jsonValue);

    return pair;
}

bool
parcJSONPair_Equals(const PARCJSONPair *objA, const PARCJSONPair *objB)
{
    if (objA == NULL && objB == NULL) {
        return true;
    } else if (objA != NULL && objB != NULL) {
        if (parcBuffer_Equals(objA->name, objB->name)) {
            if (parcJSONValue_Equals(objA->value, objB->value)) {
                return true;
            }
        }
    }

    return false;
}

PARCBufferComposer *
parcJSONPair_BuildString(const PARCJSONPair *pair, PARCBufferComposer *composer, bool compact)
{
    parcBufferComposer_PutUint8(composer, '"');

    parcBufferComposer_PutBuffer(composer, pair->name);
    parcBuffer_Rewind(pair->name);
    if (compact) {
        parcBufferComposer_PutString(composer, "\":");
    } else {
        parcBufferComposer_PutString(composer, "\" : ");
    }
    parcJSONValue_BuildString(pair->value, composer, compact);

    return composer;
}

char *
parcJSONPair_ToString(const PARCJSONPair *pair)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcJSONPair_BuildString(pair, composer, false);
    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);

    return result;
}

PARCJSONPair *
parcJSONPair_Parser(PARCJSONParser *parser)
{
    PARCJSONPair *result = NULL;

    // This makes an unnecessary copy.  I think this could just be a buffer slice.
    PARCBuffer *name = parcJSONParser_ParseString(parser);
    char c = parcJSONParser_NextChar(parser);
    if (c == ':') {
        PARCJSONValue *value = parcJSONValue_Parser(parser);
        if (value != NULL) {
            result = parcJSONPair_Create(name, value);
            parcJSONValue_Release(&value);
        }
    }
    parcBuffer_Release(&name);

    return result;
}
