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
 *
 */

#include <config.h>

#include <string.h>

#include "parc_KeyValue.h"
#include "parc_Object.h"

#include <LongBow/runtime.h>

struct parc_key_value {
    PARCObject *key;
    PARCObject *value;
};

static void
_parcKeyValue_Destroy(PARCKeyValue **keyValuePointer)
{
    PARCKeyValue *keyValue = *keyValuePointer;
    parcObject_Release(&keyValue->key);
    if (keyValue->value != NULL) {
        parcObject_Release(&keyValue->value);
    }
}

parcObject_ExtendPARCObject(PARCKeyValue, _parcKeyValue_Destroy, parcKeyValue_Copy, NULL, parcKeyValue_Equals,
                            parcKeyValue_Compare, parcKeyValue_HashCode, NULL);

parcObject_ImplementAcquire(parcKeyValue, PARCKeyValue);

parcObject_ImplementRelease(parcKeyValue, PARCKeyValue);

PARCKeyValue *
parcKeyValue_Create(const PARCObject *key,
                    const PARCObject *value)
{
    assertNotNull(key, "Key may not be null in a KeyValue element");

    PARCKeyValue *keyValue = parcObject_CreateInstance(PARCKeyValue);
    assertNotNull(keyValue, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCKeyValue));

    keyValue->key = parcObject_Acquire(key);
    keyValue->value = NULL;
    if (value != NULL) {
        keyValue->value = parcObject_Acquire(value);
    }

    return keyValue;
}

PARCKeyValue *
parcKeyValue_Copy(const PARCKeyValue *source)
{
    PARCKeyValue *newKV = parcObject_CreateInstance(PARCKeyValue);
    newKV->key = parcObject_Copy(source->key);
    newKV->value = NULL;
    if (source->value != NULL) {
        newKV->value = parcObject_Copy(source->value);
    }

    return newKV;
}

void
parcKeyValue_SetValue(PARCKeyValue *keyValue, PARCObject *value)
{
    assertNotNull(keyValue, "Not a valid keyValue");
    PARCObject *oldValue = keyValue->value;
    if (value != NULL) {
        keyValue->value = parcObject_Acquire(value);
    } else {
        keyValue->value = NULL;
    }
    if (oldValue != NULL) {
        parcObject_Release(&oldValue);
    }
}

void
parcKeyValue_SetKey(PARCKeyValue *keyValue, PARCObject *key)
{
    assertNotNull(keyValue, "Not a valid keyValue");
    PARCObject *oldKey = keyValue->key;
    keyValue->key = parcObject_Acquire(key);
    parcObject_Release(&oldKey);
}

PARCObject *
parcKeyValue_GetValue(PARCKeyValue *keyValue)
{
    assertNotNull(keyValue, "Not a valid keyValue");
    return keyValue->value;
}

PARCObject *
parcKeyValue_GetKey(PARCKeyValue *keyValue)
{
    assertNotNull(keyValue, "Not a valid keyValue");
    return keyValue->key;
}

bool
parcKeyValue_Equals(const PARCKeyValue *a, const PARCKeyValue *b)
{
    bool result = parcObject_Equals(a->key, b->key);
    if ((a->value == NULL) || (b->value == NULL)) {
        result &= (a->value == b->value); // Only true if both are NULL
    } else {
        result &= parcObject_Equals(a->value, b->value);
    }
    return result;
}

int
parcKeyValue_Compare(const PARCKeyValue *a, const PARCKeyValue *b)
{
    if (a == NULL && b == NULL) {
        return 0;
    }
    if (a != NULL && b == NULL) {
        return 1;
    }
    if (a == NULL && b != NULL) {
        return -1;
    } else {
        return parcObject_Compare(a->key, b->key);
    }
}

PARCHashCode
parcKeyValue_HashCode(const PARCKeyValue *keyValue)
{
    return parcObject_HashCode(keyValue->key);
}

bool
parcKeyValue_EqualKeys(const PARCKeyValue *a, const PARCKeyValue *b)
{
    return parcObject_Equals(a->key, b->key);
}
