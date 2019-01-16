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
//#include <config.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include "parc_MyObject.h"

// Detect a compile time if a buffer is large enough to hold a structure.
#define parcObject_DefineXXX(_type_, ...) \
    typedef struct { __VA_ARGS__ } _ ## _type_; \
    enum { bytesCompileTimeAssertion = 1 / !!(sizeof(_type_) >= sizeof(_ ## _type_)) }

struct PARCMyObject {
    int x;
    double y;
    double z;
};

static bool
_parcMyObject_Destructor(PARCMyObject **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCMyObject pointer.");


    /* cleanup the instance fields here */
    return true;
}

parcObject_ImplementAcquire(parcMyObject, PARCMyObject);

parcObject_ImplementRelease(parcMyObject, PARCMyObject);

parcObject_Override(
    PARCMyObject, PARCObject,
    .destructor = (PARCObjectDestructor *) _parcMyObject_Destructor,
    .copy = (PARCObjectCopy *) parcMyObject_Copy,
    .toString = (PARCObjectToString *)  parcMyObject_ToString,
    .equals = (PARCObjectEquals *)  parcMyObject_Equals,
    .compare = (PARCObjectCompare *)  parcMyObject_Compare,
    .hashCode = (PARCObjectHashCode *)  parcMyObject_HashCode,
    .toJSON = (PARCObjectToJSON *)  parcMyObject_ToJSON);


void
parcMyObject_AssertValid(const PARCMyObject *instance)
{
    parcAssertTrue(parcMyObject_IsValid(instance),
               "PARCMyObject is not valid.");
}

PARCMyObject *
parcMyObject_Wrap(void *origin)
{
    PARCMyObject *result = parcObject_Wrap(origin, PARCMyObject);

    return result;
}

PARCMyObject *
parcMyObject_Init(PARCMyObject *object, int x, double y, double z)
{
    if (object != NULL) {
        object->x = x;
        object->y = y;
        object->z = z;
    }

    return object;
}

PARCMyObject *
parcMyObject_Create(int x, double y, double z)
{
    PARCMyObject *result = parcObject_CreateInstance(PARCMyObject);

    if (result != NULL) {
        result->x = x;
        result->y = y;
        result->z = z;
    }

    return (PARCMyObject *) result;
}

int
parcMyObject_Compare(const PARCMyObject *instance, const PARCMyObject *other)
{
    int result = 0;

    return result;
}

PARCMyObject *
parcMyObject_Copy(const PARCMyObject *original)
{
    PARCMyObject *result = NULL;

    return result;
}

void
parcMyObject_Display(const PARCMyObject *object, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCMyObject@%p { .x=%d .y=%f .z=%f }", object, object->x, object->y, object->z);
}

bool
parcMyObject_Equals(const PARCMyObject *x, const PARCMyObject *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        /* perform instance specific equality tests here. */
    }

    return result;
}

PARCHashCode
parcMyObject_HashCode(const PARCMyObject *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcMyObject_IsValid(const PARCMyObject *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcMyObject_ToJSON(const PARCMyObject *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcMyObject_ToString(const PARCMyObject *object)
{
    char *result = parcMemory_Format("PARCMyObject@%p { .x=%d .y=%f .z=%f }", object, object->x, object->y, object->z);

    return result;
}
