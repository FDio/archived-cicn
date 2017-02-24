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

#include <parc/algol/parc_Iterator.h>

struct parc_iterator {
    PARCObject *object;
    void *(*init)(PARCObject *);
    bool (*hasNext)(PARCObject *, void *);
    void *(*next)(PARCObject *, void *);
    void (*remove)(PARCObject *, void **);
    void *(*element)(PARCObject *, void *);
    void (*fini)(PARCObject *, void *);
    void (*assertValid)(const void *);
    bool initialized;
    void *state;
};

static void
_parcIterator_Destroy(PARCIterator **iteratorPtr)
{
    PARCIterator *iterator = *iteratorPtr;

    parcObject_Release(&(iterator->object));

    (iterator->fini(iterator->object, iterator->state));
}

parcObject_ExtendPARCObject(PARCIterator, _parcIterator_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

static void *
_parcIterator_Init(PARCIterator *iterator)
{
    if (iterator->init) {
        iterator->state = (iterator->init)(iterator->object);
    }

    if (iterator->assertValid) {
        (iterator->assertValid)(iterator->state);
    }

    iterator->initialized = true;
    return iterator->state;
}

static void
_parcIteratorState_AssertValid(const PARCIterator *iterator)
{
    if (iterator->assertValid) {
        (iterator->assertValid)(iterator->state);
    }
}

PARCIterator *
parcIterator_Create(PARCObject *object,
                    void *(*init)(PARCObject *),
                    bool (*hasNext)(PARCObject *, void *),
                    void *(*next)(PARCObject *, void *),
                    void (*remove)(PARCObject *, void **),
                    void *(*element)(PARCObject *, void *),
                    void (*fini)(PARCObject *, void *),
                    void (*assertValid)(const void *))
{
    assertNotNull(object, "PARCObject cannot be NULL.");
    assertNotNull(init, "'init' function cannot be NULL.");
    assertNotNull(hasNext, "'hasNext' function cannot be NULL.");
    assertNotNull(next, "'next' function cannot be NULL.");
    assertNotNull(element, "'element' function cannot be NULL.");
    assertNotNull(fini, "'fini' function cannot be NULL.");

    PARCIterator *result = parcObject_CreateInstance(PARCIterator);

    if (result != NULL) {
        result->object = parcObject_Acquire(object);
        result->init = init;
        result->hasNext = hasNext;
        result->next = next;
        result->remove = remove;
        result->element = element;
        result->fini = fini;
        result->assertValid = assertValid;

        result->initialized = false;
        _parcIterator_Init(result);
    }

    return result;
}

bool
parcIterator_IsValid(const PARCIterator *iterator)
{
    bool result = false;

    if (iterator != NULL) {
        if (parcObject_IsValid(iterator)) {
            result = true;
        }
    }

    return result;
}

void
parcIterator_AssertValid(const PARCIterator *iterator)
{
    assertTrue(parcIterator_IsValid(iterator), "PARCIterator is not valid.");
}

parcObject_ImplementAcquire(parcIterator, PARCIterator);

parcObject_ImplementRelease(parcIterator, PARCIterator);


void *
parcIterator_Next(PARCIterator *iterator)
{
    parcIterator_OptionalAssertValid(iterator);

    iterator->state = (iterator->next)(iterator->object, iterator->state);

    _parcIteratorState_AssertValid(iterator);

    return (iterator->element)(iterator->object, iterator->state);
}

bool
parcIterator_HasNext(const PARCIterator *iterator)
{
    parcIterator_OptionalAssertValid(iterator);

    return (iterator->hasNext)(iterator->object, iterator->state);
}

void
parcIterator_Remove(PARCIterator *iterator)
{
    if (iterator->remove != NULL) {
        (iterator->remove)(iterator->object, &iterator->state);
    }

    _parcIteratorState_AssertValid(iterator);
}
