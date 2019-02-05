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
#include <stdio.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_HashMap.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>

#include "parc_Properties.h"

struct PARCProperties {
    PARCHashMap *properties;
};

static void
_parcProperties_Finalize(PARCProperties **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCProperties pointer.");
    PARCProperties *instance = *instancePtr;

    parcProperties_OptionalAssertValid(instance);

    parcHashMap_Release(&instance->properties);
}

parcObject_ImplementAcquire(parcProperties, PARCProperties);

parcObject_ImplementRelease(parcProperties, PARCProperties);

parcObject_ExtendPARCObject(PARCProperties, _parcProperties_Finalize, parcProperties_Copy, parcProperties_ToString, parcProperties_Equals, parcProperties_Compare, parcProperties_HashCode, parcProperties_ToJSON);


void
parcProperties_AssertValid(const PARCProperties *instance)
{
    parcAssertTrue(parcProperties_IsValid(instance),
               "PARCProperties is not valid.");
}

PARCProperties *
parcProperties_Create(void)
{
    PARCProperties *result = parcObject_CreateInstance(PARCProperties);

    if (result != NULL) {
        result->properties = parcHashMap_Create();
    }

    return result;
}

int
parcProperties_Compare(const PARCProperties *instance, const PARCProperties *other)
{
    int result = 0;


    return result;
}

PARCProperties *
parcProperties_Copy(const PARCProperties *original)
{
    PARCProperties *result = parcObject_CreateInstance(PARCProperties);

    if (result != NULL) {
        result->properties = parcHashMap_Copy(original->properties);
    }

    return result;
}

void
parcProperties_Display(const PARCProperties *properties, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCProperties@%p {", properties);
    parcTrapCannotObtainLockIf(parcHashMap_Lock(properties->properties) == false, "Cannot lock PARCProperties object.");

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(properties->properties);
    while (parcIterator_HasNext(iterator)) {
        char *key = parcBuffer_ToString(parcIterator_Next(iterator));
        const char *value = parcProperties_GetProperty(properties, key);
        parcDisplayIndented_PrintLine(indentation + 1, "%s=%s", key, value);

        parcMemory_Deallocate(&key);
    }

    parcIterator_Release(&iterator);

    parcHashMap_Unlock(properties->properties);

    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcProperties_Equals(const PARCProperties *x, const PARCProperties *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        return parcHashMap_Equals(x->properties, y->properties);
    }

    return result;
}

PARCHashCode
parcProperties_HashCode(const PARCProperties *instance)
{
    return parcHashMap_HashCode(instance->properties);
}

bool
parcProperties_IsValid(const PARCProperties *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcProperties_ToJSON(const PARCProperties *properties)
{
    PARCJSON *result = parcJSON_Create();

    parcTrapCannotObtainLockIf(parcHashMap_Lock(properties->properties) == false, "Cannot lock PARCProperties object.");

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(properties->properties);
    while (parcIterator_HasNext(iterator)) {
        char *key = parcBuffer_ToString(parcIterator_Next(iterator));
        const char *value = parcProperties_GetProperty(properties, key);
        parcJSON_AddString(result, key, value);
        parcMemory_Deallocate(&key);
    }

    parcIterator_Release(&iterator);

    parcHashMap_Unlock(properties->properties);
    return result;
}

PARCBufferComposer *
parcProperties_BuildString(const PARCProperties *properties, PARCBufferComposer *composer)
{
    parcTrapCannotObtainLockIf(parcHashMap_Lock(properties->properties) == false, "Cannot lock PARCProperties object.");

    PARCIterator *iterator = parcHashMap_CreateKeyIterator(properties->properties);
    while (parcIterator_HasNext(iterator)) {
        char *key = parcBuffer_ToString(parcIterator_Next(iterator));
        const char *value = parcProperties_GetProperty(properties, key);
        parcBufferComposer_PutStrings(composer, key, "=", value, "\n", NULL);
        parcMemory_Deallocate(&key);
    }

    parcIterator_Release(&iterator);

    parcHashMap_Unlock(properties->properties);
    return composer;
}

char *
parcProperties_ToString(const PARCProperties *properties)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcProperties_BuildString(properties, composer);
    char *result = parcBufferComposer_ToString(composer);

    parcBufferComposer_Release(&composer);

    return result;
}

void
parcProperties_SetParsedProperty(PARCProperties *properties, char *string)
{
    char *equals = strchr(string, '=');
    *equals++ = 0;

    parcProperties_SetProperty(properties, string, equals);
}

bool
parcProperties_SetProperty(PARCProperties *properties, const char *name, const char *string)
{
    bool result = false;

    PARCBuffer *key = parcBuffer_AllocateCString(name);
    PARCBuffer *value = parcBuffer_AllocateCString(string);

    parcHashMap_Put(properties->properties, key, value);
    parcBuffer_Release(&key);
    parcBuffer_Release(&value);
    return result;
}

const char *
parcProperties_GetProperty(const PARCProperties *properties, const char *restrict name)
{
    char *result = NULL;

    PARCBuffer *key = parcBuffer_AllocateCString(name);
    PARCBuffer *value = (PARCBuffer *) parcHashMap_Get(properties->properties, key);
    if (value != NULL) {
        result = parcBuffer_Overlay(value, 0);
    }

    parcBuffer_Release(&key);
    return result;
}

const char *
parcProperties_GetPropertyDefault(const PARCProperties *properties, const char *restrict name, const char *restrict defaultValue)
{
    char *result = (char *) defaultValue;

    PARCBuffer *key = parcBuffer_AllocateCString(name);
    PARCBuffer *value = (PARCBuffer *) parcHashMap_Get(properties->properties, key);
    if (value != NULL) {
        result = parcBuffer_Overlay(value, 0);
    }

    parcBuffer_Release(&key);
    return result;
}

bool
parcProperties_GetAsBoolean(const PARCProperties *properties, const char *name, bool defaultValue)
{
    bool result = defaultValue;

    const char *value = parcProperties_GetProperty(properties, name);
    if (value != NULL) {
        if (strcmp(value, "true") == 0) {
            result = true;
        } else {
            result = false;
        }
    }

    return result;
}

int64_t
parcProperties_GetAsInteger(const PARCProperties *properties, const char *name, int64_t defaultValue)
{
    int64_t result = defaultValue;

    const char *value = parcProperties_GetProperty(properties, name);
    if (value != NULL) {
        result = strtol(value, NULL, 10);
    }

    return result;
}

typedef struct {
    PARCBuffer *element;
    PARCIterator *hashMapIterator;
} _PARCPropertiesIterator;

static _PARCPropertiesIterator *
_parcPropertiesIterator_Init(const PARCProperties *object)
{
    _PARCPropertiesIterator *state = parcMemory_AllocateAndClear(sizeof(_PARCPropertiesIterator));
    state->hashMapIterator = parcHashMap_CreateKeyIterator(object->properties);
    return state;
}

static bool
_parcPropertiesIterator_HasNext(PARCProperties *properties __attribute__((unused)), _PARCPropertiesIterator *state)
{
    return parcIterator_HasNext(state->hashMapIterator);
}

static _PARCPropertiesIterator *
_parcPropertiesIterator_Next(PARCProperties *properties __attribute__((unused)), _PARCPropertiesIterator *state)
{
    state->element = (PARCBuffer *) parcIterator_Next(state->hashMapIterator);
    return state;
}

static void
_parcPropertiesIterator_Remove(PARCProperties *properties __attribute__((unused)), _PARCPropertiesIterator **state)
{
    parcIterator_Remove((*state)->hashMapIterator);
}

static char *
_parcPropertiesIterator_Element(PARCProperties *properties __attribute__((unused)), _PARCPropertiesIterator *state)
{
    return parcBuffer_Overlay(state->element, 0);
}

static void
_parcPropertiesIterator_Fini(PARCProperties *properties __attribute__((unused)), _PARCPropertiesIterator *state)
{
    parcIterator_Release(&state->hashMapIterator);
    parcMemory_Deallocate(&state);
}

PARCIterator *
parcProperties_CreateIterator(const PARCProperties *properties)
{
    PARCIterator *iterator = parcIterator_Create((PARCObject *) properties,
                                                 (void *(*)(PARCObject *))_parcPropertiesIterator_Init,
                                                 (bool (*)(PARCObject *, void *))_parcPropertiesIterator_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcPropertiesIterator_Next,
                                                 (void (*)(PARCObject *, void **))_parcPropertiesIterator_Remove,
                                                 (void *(*)(PARCObject *, void *))_parcPropertiesIterator_Element,
                                                 (void (*)(PARCObject *, void *))_parcPropertiesIterator_Fini,
                                                 NULL);

    return iterator;
}
