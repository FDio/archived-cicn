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
#include <stdio.h>
#include <config.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/algol/parc_Unsigned.h>

struct PARCUnsigned {
    unsigned x;
};

static bool
_parcUnsigned_Destructor(PARCUnsigned **instancePtr)
{
    return true;
}

parcObject_ImplementAcquire(parcUnsigned, PARCUnsigned);

parcObject_ImplementRelease(parcUnsigned, PARCUnsigned);

parcObject_Override(PARCUnsigned, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcUnsigned_Destructor,
                    .copy = (PARCObjectCopy *) parcUnsigned_Copy,
                    .display = (PARCObjectDisplay *) parcUnsigned_Display,
                    .toString = (PARCObjectToString *) parcUnsigned_ToString,
                    .equals = (PARCObjectEquals *) parcUnsigned_Equals,
                    .compare = (PARCObjectCompare *) parcUnsigned_Compare,
                    .hashCode = (PARCObjectHashCode *) parcUnsigned_HashCode,
                    .toJSON = (PARCObjectToJSON *) parcUnsigned_ToJSON,
                    .display = (PARCObjectDisplay *) parcUnsigned_Display);

void
parcUnsigned_AssertValid(const PARCUnsigned *instance)
{
    assertTrue(parcUnsigned_IsValid(instance),
               "PARCUnsigned is not valid.");
}

PARCUnsigned *
parcUnsigned_Create(unsigned x)
{
    PARCUnsigned *result = parcObject_CreateInstance(PARCUnsigned);
    if (result != NULL) {
        result->x = x;
    }
    return result;
}

int
parcUnsigned_Compare(const PARCUnsigned *val, const PARCUnsigned *other)
{
    int result = 0;

    if (val == NULL) {
        if (other != NULL) {
            result = -1;
        }
    } else if (other == NULL) {
        result = 1;
    } else {
        parcUnsigned_OptionalAssertValid(val);
        parcUnsigned_OptionalAssertValid(other);

        if (val->x < other->x) {
            return -1;
        } else if (val->x > other->x) {
            return 1;
        }
    }

    return result;
}

PARCUnsigned *
parcUnsigned_Copy(const PARCUnsigned *original)
{
    PARCUnsigned *result = parcUnsigned_Create(original->x);

    return result;
}

void
parcUnsigned_Display(const PARCUnsigned *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCSUnsingned@%p {", instance);
    parcDisplayIndented_PrintLine(indentation + 1, "%d", instance->x);
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcUnsigned_Equals(const PARCUnsigned *x, const PARCUnsigned *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        parcUnsigned_OptionalAssertValid(x);
        parcUnsigned_OptionalAssertValid(y);

        if (x->x == y->x) {
            result = true;
        }
    }

    return result;
}

PARCHashCode
parcUnsigned_HashCode(const PARCUnsigned *x)
{
    PARCHashCode result = 0;

    result = parcHashCode_Hash((uint8_t *) &(x->x), sizeof(x->x));

    return result;
}

bool
parcUnsigned_IsValid(const PARCUnsigned *x)
{
    bool result = false;

    if (x != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcUnsigned_ToJSON(const PARCUnsigned *x)
{
    PARCJSON *result = parcJSON_Create();

    return result;
}

char *
parcUnsigned_ToString(const PARCUnsigned *x)
{
    int length = snprintf(NULL, 0, "%d", x->x);
    char*str = malloc(length + 1);
    snprintf(str, length + 1, "%d", x->x);
    return str;
}

unsigned
parcUnsigned_GetUnsigned(const PARCUnsigned *x)
{
    parcUnsigned_OptionalAssertValid(x);

    return x->x;
}
