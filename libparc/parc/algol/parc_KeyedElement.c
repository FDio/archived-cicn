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

#include <LongBow/runtime.h>

#include <parc/algol/parc_KeyedElement.h>
#include <parc/algol/parc_Memory.h>

struct parc_keyed_element {
    size_t keylen;
    void *key;
    void *element;
};

PARCKeyedElement *
parcKeyedElement_Create(void *data, const void *key, size_t keylen)
{
    PARCKeyedElement *keyedElement = parcMemory_Allocate(sizeof(PARCKeyedElement));
    assertNotNull(keyedElement, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCKeyedElement));
    keyedElement->element = data;
    keyedElement->key = parcMemory_Allocate(sizeof(PARCKeyedElement));
    assertNotNull(keyedElement->key, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCKeyedElement));
    memcpy(keyedElement->key, key, keylen);
    keyedElement->keylen = keylen;
    return keyedElement;
}

void
parcKeyedElement_Destroy(PARCKeyedElement **keyedElementPointer)
{
    parcMemory_Deallocate((void **) &((*keyedElementPointer)->key));
    parcMemory_Deallocate((void **) keyedElementPointer);
    *keyedElementPointer = NULL;
}

void
parcKeyedElement_SetData(PARCKeyedElement *keyedElement, void *data)
{
    keyedElement->element = data;
}

void *
parcKeyedElement_GetData(PARCKeyedElement *keyedElement)
{
    return keyedElement->element;
}

void *
parcKeyedElement_GetKey(PARCKeyedElement *keyedElement)
{
    return keyedElement->key;
}

size_t
parcKeyedElement_GetKeyLen(PARCKeyedElement *keyedElement)
{
    return keyedElement->keylen;
}
