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

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/algol/parc_String.h>

struct PARCString {
    char *string;
};

static bool
_parcString_Destructor(PARCString **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCString pointer.");
    PARCString *string = *instancePtr;

    parcMemory_Deallocate(&string->string);
    return true;
}

parcObject_ImplementAcquire(parcString, PARCString);

parcObject_ImplementRelease(parcString, PARCString);

parcObject_Override(PARCString, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcString_Destructor,
                    .copy = (PARCObjectCopy *) parcString_Copy,
                    .display = (PARCObjectDisplay *) parcString_Display,
                    .toString = (PARCObjectToString *) parcString_ToString,
                    .equals = (PARCObjectEquals *) parcString_Equals,
                    .compare = (PARCObjectCompare *) parcString_Compare,
                    .hashCode = (PARCObjectHashCode *) parcString_HashCode,
                    .toJSON = (PARCObjectToJSON *) parcString_ToJSON,
                    .display = (PARCObjectDisplay *) parcString_Display);

void
parcString_AssertValid(const PARCString *instance)
{
    parcAssertTrue(parcString_IsValid(instance),
               "PARCString is not valid.");
}

PARCString *
parcString_Create(const char *string)
{
    PARCString *result = parcObject_CreateInstance(PARCString);
    if (result != NULL) {
        result->string = parcMemory_StringDuplicate(string, strlen(string));
    }
    return result;
}

PARCString *
parcString_CreateFromBuffer(const PARCBuffer *buffer)
{
    PARCString *result = parcString_Create(parcBuffer_Overlay((PARCBuffer *) buffer, 0));

    return result;
}

int
parcString_Compare(const PARCString *string, const PARCString *other)
{
    int result = 0;

    if (string == NULL) {
        if (other != NULL) {
            result = -1;
        }
    } else if (other == NULL) {
        result = 1;
    } else {
        parcString_OptionalAssertValid(string);
        parcString_OptionalAssertValid(other);

        int comparison = strcmp(string->string, other->string);
        if (comparison < 0) {
            result = -1;
        } else if (comparison > 0) {
            result = 1;
        }
    }

    return result;
}

PARCString *
parcString_Copy(const PARCString *original)
{
    PARCString *result = parcString_Create(original->string);

    return result;
}

void
parcString_Display(const PARCString *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCString@%p {", instance);
    parcDisplayIndented_PrintLine(indentation + 1, "%s", instance->string);
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcString_Equals(const PARCString *x, const PARCString *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        parcString_OptionalAssertValid(x);
        parcString_OptionalAssertValid(y);

        result = strcmp(x->string, y->string) == 0;
    }

    return result;
}

PARCHashCode
parcString_HashCode(const PARCString *string)
{
    PARCHashCode result = 0;

    result = parcHashCode_Hash((uint8_t *) string->string, strlen(string->string));

    return result;
}

bool
parcString_IsValid(const PARCString *string)
{
    bool result = false;

    if (string != NULL) {
        if (string->string != NULL) {
            result = true;
        }
    }

    return result;
}

PARCJSON *
parcString_ToJSON(const PARCString *string)
{
    PARCJSON *result = parcJSON_Create();

    return result;
}

char *
parcString_ToString(const PARCString *string)
{
    char *result = parcMemory_StringDuplicate(string->string, strlen(string->string));

    return result;
}

const char *
parcString_GetString(const PARCString *string)
{
    parcString_OptionalAssertValid(string);

    return string->string;
}
