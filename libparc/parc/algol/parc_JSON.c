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
#include <parc/algol/parc_PathName.h>

struct parc_json {
    PARCList *members;
};

static void
_destroyPARCJSON(PARCJSON **jsonPtr)
{
    PARCJSON *json = *jsonPtr;

    parcList_Release(&json->members);
}

parcObject_ExtendPARCObject(PARCJSON, _destroyPARCJSON, NULL, parcJSON_ToString, parcJSON_Equals, NULL, NULL, NULL);

PARCJSON *
parcJSON_Create(void)
{
    PARCJSON *result = parcObject_CreateInstance(PARCJSON);
    if (result != NULL) {
        result->members = parcList(parcArrayList_Create((void (*)(void **))parcJSONPair_Release), PARCArrayListAsPARCList);
    }

    return result;
}

parcObject_ImplementAcquire(parcJSON, PARCJSON);

parcObject_ImplementRelease(parcJSON, PARCJSON);

static bool
_memberListEquals(const PARCList *x, const PARCList *y)
{
    for (size_t i = 0; i < parcList_Size(x); i++) {
        PARCJSONPair *pairA = parcList_GetAtIndex(x, i);
        PARCJSONPair *pairB = parcList_GetAtIndex(y, i);
        if (parcJSONPair_Equals(pairA, pairB) == false) {
            return false;
        }
    }
    return true;
}

static PARCBuffer *
_toCreatedBuffer(const PARCJSON *json, bool compact)
{
    if (json == NULL) {
        return NULL;
    }
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSON_BuildString(json, composer, compact);

    PARCBuffer *result = parcBufferComposer_ProduceBuffer(composer);
    parcBufferComposer_Release(&composer);

    return result;
}

static char *
_toString(const PARCJSON *json, bool compact)
{
    PARCBuffer *tempBuffer = _toCreatedBuffer(json, compact);

    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    return result;
}

bool
parcJSON_Equals(const PARCJSON *x, const PARCJSON *y)
{
    bool result = false;

    if (x == NULL && y == NULL) {
        result = true;
    } else if (x != NULL && y != NULL) {
        if (parcList_Size(x->members) == parcList_Size(y->members)) {
            if (_memberListEquals(x->members, y->members)) {
                result = true;
            }
        }
    }

    return result;
}

PARCJSON *
parcJSON_Copy(const PARCJSON *src)
{
    //TODO - This is an ineffecient deep copy. The _Copy() operation needs to be implementd for all PARCJSON* types
    //before we can do an effecient deep copy.
    if (src == NULL) {
        return NULL;
    }
    PARCBuffer *temp = _toCreatedBuffer(src, true);
    PARCJSON *result = parcJSON_ParseBuffer(temp);
    parcBuffer_Release(&temp);

    return result;
}


PARCHashCode
parcJSON_HashCode(const PARCJSON *json)
{
    PARCBuffer *temp = _toCreatedBuffer(json, true);
    PARCHashCode result = parcBuffer_HashCode(temp);
    parcBuffer_Release(&temp);

    return result;
}

void
parcJSON_Display(const PARCJSON *json, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCJSON@%p {", json);
    for (size_t index = 0; index < parcList_Size(json->members); index++) {
        PARCJSONPair *pair = parcList_GetAtIndex(json->members, index);
        parcJSONPair_Display(pair, indentation + 1);
    }
    parcDisplayIndented_PrintLine(indentation, "}");
}

PARCJSONPair *
parcJSON_GetPairByIndex(const PARCJSON *json, size_t index)
{
    PARCJSONPair *result = NULL;

    if (parcList_Size(json->members) > index) {
        result = parcList_GetAtIndex(json->members, index);
    }

    return result;
}

PARCJSONValue *
parcJSON_GetValueByIndex(const PARCJSON *json, size_t index)
{
    PARCJSONValue *result = NULL;

    if (parcList_Size(json->members) > index) {
        result = parcJSONPair_GetValue(parcList_GetAtIndex(json->members, index));
    }

    return result;
}

const PARCJSONPair *
parcJSON_GetPairByName(const PARCJSON *json, const char *name)
{
    PARCJSONPair *result = NULL;

    PARCBuffer *nameBuffer = parcBuffer_Wrap((uint8_t *) name, strlen(name), 0, strlen(name));
    for (size_t index = 0; index < parcList_Size(json->members); index++) {
        PARCJSONPair *pair = parcList_GetAtIndex(json->members, index);
        if (parcBuffer_Equals(nameBuffer, parcJSONPair_GetName(pair))) {
            result = pair;
            break;
        }
    }
    parcBuffer_Release(&nameBuffer);
    return result;
}

PARCJSONValue *
parcJSON_GetValueByName(const PARCJSON *json, const char *name)
{
    PARCJSONValue *result = NULL;
    const PARCJSONPair *pair = parcJSON_GetPairByName(json, name);
    if (pair != NULL) {
        result = parcJSONPair_GetValue(pair);
    }

    return result;
}

PARCList *
parcJSON_GetMembers(const PARCJSON *json)
{
    return json->members;
}

PARCBufferComposer *
parcJSON_BuildString(const PARCJSON *json, PARCBufferComposer *composer, bool compact)
{
    parcBufferComposer_PutChar(composer, '{');
    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }

    char *separator = "";
    for (size_t i = 0; i < parcList_Size(json->members); i++) {
        parcBufferComposer_PutString(composer, separator);
        parcJSONPair_BuildString(parcList_GetAtIndex(json->members, i), composer, compact);
        separator = ", ";
        if (compact) {
            separator = ",";
        }
    }

    if (!compact) {
        parcBufferComposer_PutChar(composer, ' ');
    }
    parcBufferComposer_PutChar(composer, '}');
    return composer;
}


char *
parcJSON_ToString(const PARCJSON *json)
{
    return _toString(json, false);
}

char *
parcJSON_ToCompactString(const PARCJSON *json)
{
    return _toString(json, true);
}

const PARCJSONValue *
parcJSON_GetByPathName(const PARCJSONValue *pathNode, const PARCPathName *path)
{
    for (int i = 0; i < parcPathName_Size(path); i++) {
        char *name = parcPathName_GetAtIndex(path, i);
        if (parcJSONValue_IsJSON(pathNode)) {
            const PARCJSONPair *pair = parcJSON_GetPairByName(parcJSONValue_GetJSON(pathNode), name);
            if (pair == NULL) {
                pathNode = NULL;
                break;
            }
            pathNode = parcJSONPair_GetValue(pair);
        } else if (parcJSONValue_IsArray(pathNode)) {
            size_t index = strtoll(name, NULL, 10);
            if (index > parcJSONArray_GetLength(parcJSONValue_GetArray(pathNode))) {
                pathNode = NULL;
                break;
            }
            pathNode = parcJSONArray_GetValue(parcJSONValue_GetArray(pathNode), index);
        } else {
            pathNode = NULL;
            break;
        }
    }

    return pathNode;
}

const PARCJSONValue *
parcJSON_GetByPath(const PARCJSON *json, const char *path)
{
    PARCJSONValue *pathNode = parcJSONValue_CreateFromJSON((PARCJSON *) json);

    PARCPathName *pathName = parcPathName_Parse(path);
    const PARCJSONValue *result = parcJSON_GetByPathName(pathNode, pathName);
    parcPathName_Release(&pathName);

    parcJSONValue_Release(&pathNode);
    return result;
}

PARCJSON *
parcJSON_ParseString(const char *string)
{
    // The input string is read-only, therefore we can cast it here when calling the parcBuffer_WrapCString()
    // to work around the fact that the function does not take a 'const' attribute.
    // This function is not going to modify the input string, so the 'const' promise will be kept.
    PARCBuffer *buffer = parcBuffer_WrapCString((char *) string);

    PARCJSON *result = parcJSON_ParseBuffer(buffer);
    parcBuffer_Release(&buffer);

    return result;
}

PARCJSON *
parcJSON_ParseBuffer(PARCBuffer *buffer)
{
    PARCJSON *result = NULL;

    // The input string is read-only, therefore we can cast it here when calling the parcBuffer_WrapCString()
    // to work around the fact that the function does not take a 'const' attribute.
    // This function is not going to modify the input string, so the 'const' promise will be kept.

    PARCJSONParser *parser = parcJSONParser_Create(buffer);

    char firstCharacter = parcJSONParser_PeekNextChar(parser);
    if (firstCharacter == '{') {
        PARCJSONValue *value = parcJSONValue_ObjectParser(parser);

        result = parcJSON_Acquire(parcJSONValue_GetJSON(value));
        parcJSONValue_Release(&value);
    }

    parcJSONParser_Release(&parser);

    return result;
}

PARCJSON *
parcJSON_AddPair(PARCJSON *json, PARCJSONPair *pair)
{
    parcList_Add(json->members, parcJSONPair_Acquire(pair));
    return json;
}

PARCJSON *
parcJSON_AddValue(PARCJSON *json, const char *name, PARCJSONValue *value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromJSONValue(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}

PARCJSON *
parcJSON_AddString(PARCJSON *json, const char *name, const char *value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromString(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}

PARCJSON *
parcJSON_AddObject(PARCJSON *json, const char *name, PARCJSON *value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromJSON(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}

PARCJSON *
parcJSON_AddInteger(PARCJSON *json, const char *name, int64_t value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromInteger(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}

PARCJSON *
parcJSON_AddBoolean(PARCJSON *json, const char *name, bool value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromBoolean(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}

PARCJSON *
parcJSON_AddArray(PARCJSON *json, const char *name, PARCJSONArray *value)
{
    PARCJSONPair *pair = parcJSONPair_CreateFromJSONArray(name, value);
    parcJSON_AddPair(json, pair);
    parcJSONPair_Release(&pair);

    return json;
}
