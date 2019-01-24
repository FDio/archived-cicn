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
#include <sys/queue.h>

#include <parc/algol/parc_Deque.h>

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

PARCListInterface *PARCDequeAsPARCList = &(PARCListInterface) {
    .Add = (bool (*)(void *, void *))parcDeque_Append,
    .AddAtIndex = (void (*)(void *, int index, void *))NULL,
    .AddCollection = (bool (*)(void *, PARCCollection *))NULL,
    .AddCollectionAtIndex = (bool (*)(void *, int index, PARCCollection *))NULL,
    .Clear = (void (*)(void *))NULL,
    .Contains = (bool (*)(const void *, const PARCObject *))NULL,
    .ContainsCollection = (bool (*)(const void *, const PARCCollection *))NULL,
    .Copy = (void *    (*)(const PARCList *))parcDeque_Copy,
    .Destroy = (void (*)(void **))parcDeque_Release,
    .Equals = (bool (*)(const void *, const void *))parcDeque_Equals,
    .GetAtIndex = (void *    (*)(const void *, size_t))parcDeque_GetAtIndex,
    .HashCode = (PARCHashCode (*)(const void *))NULL,
    .IndexOf = (size_t (*)(const void *, const PARCObject *element))NULL,
    .IsEmpty = (bool (*)(const void *))parcDeque_IsEmpty,
    .LastIndexOf = (size_t (*)(void *, const PARCObject *element))NULL,
    .Remove = (bool (*)(void *, const PARCObject *element))NULL,
    .RemoveAtIndex = (void *    (*)(PARCList *, size_t))NULL,
    .RemoveCollection = (bool (*)(void *, const PARCCollection *))NULL,
    .RetainCollection = (bool (*)(void *, const PARCCollection *))NULL,
    .SetAtIndex = (void *    (*)(void *, size_t index, void *))NULL,
    .Size = (size_t (*)(const void *))parcDeque_Size,
    .SubList = (PARCList * (*)(const void *, size_t, size_t))NULL,
    .ToArray = (void**    (*)(const void *))NULL,
};

struct parc_deque_node {
    void *element;
    struct parc_deque_node *previous;
    struct parc_deque_node *next;
};

struct parc_deque {
    PARCObjectDescriptor object;
    struct parc_deque_node *head;
    struct parc_deque_node *tail;
    size_t size;
};

static void *
_defaultElementCopy(const void *x)
{
    return (void *) x;
}

static bool
_defaultElementEquals(const void *x, const void *y)
{
    return (x == y);
}

static inline struct parc_deque_node *
_parcDequeNode_Create(void *element, struct parc_deque_node *previous, struct parc_deque_node *next)
{
    struct parc_deque_node *result = parcMemory_Allocate(sizeof(struct parc_deque_node));
    if (result != NULL) {
        result->element = element;
        result->next = next;
        result->previous = previous;
    }

    return result;
}

static void
_parcDequeNode_Destroy(PARCDeque *deque __attribute__((unused)), struct parc_deque_node **nodePtr)
{
    struct parc_deque_node *node = *nodePtr;

    parcMemory_Deallocate((void **) &node);
    *nodePtr = 0;
}

static void
_parcDequeNode_AssertInvariants(struct parc_deque_node *node)
{
    parcAssertNotNull(node, "Expected non-null node pointer.");
    if (node->next != NULL) {
        parcAssertTrue(node->next->previous == node, "Expected next node to point to this node.");
    }
    if (node->previous != NULL) {
        parcAssertTrue(node->previous->next == node, "Expected previous node to point to this node.");
    }
}

static void
_parcDeque_AssertInvariants(const PARCDeque *deque)
{
    parcAssertNotNull(deque, "Parameter cannot be null.");
    if (deque->head != NULL) {
        parcAssertTrue(deque->size != 0, "PARCDeque head is not-null, but size is zero.");
        parcAssertNotNull(deque->tail, "PARCDeque head is not-null, but tail is null.");
        _parcDequeNode_AssertInvariants(deque->head);
        _parcDequeNode_AssertInvariants(deque->tail);
    } else {
        parcAssertNull(deque->tail, "PARCDeque head is null, but tail is not null.");
        parcAssertTrue(deque->size == 0, "PARCDeque head is null, but size is not zero.");
    }
}

static void
_parcDeque_Destroy(PARCDeque **dequePtr)
{
    PARCDeque *deque = *dequePtr;

    struct parc_deque_node *next = NULL; //deque->head;

    for (struct parc_deque_node *node = deque->head; node != NULL; node = next) {
        next = node->next;
        _parcDequeNode_Destroy(deque, &node);
    }
}

static struct parc_deque_node *
_parcDequeNode_Init(PARCDeque *deque __attribute__((unused)))
{
    return NULL;
}

static bool
_parcDequeNode_Fini(PARCDeque *deque __attribute__((unused)), const struct parc_deque_node *node __attribute__((unused)))
{
    return true;
}

static struct parc_deque_node *
_parcDequeNode_Next(PARCDeque *deque __attribute__((unused)), const struct parc_deque_node *node)
{
    if (node == NULL) {
        return deque->head;
    }
    parcTrapOutOfBoundsIf(node->next == NULL, "No more elements.");
    return node->next;
}

static bool
_parcDequeNode_HasNext(PARCDeque *deque __attribute__((unused)), const struct parc_deque_node *node)
{
    if (node == NULL) {
        return (deque->head != NULL);
    }
    return (node->next != NULL);
}

static void *
_parcDequeNode_Element(PARCDeque *deque __attribute__((unused)), const struct parc_deque_node *node)
{
    return node->element;
}

parcObject_ExtendPARCObject(PARCDeque, _parcDeque_Destroy, parcDeque_Copy, NULL, parcDeque_Equals, NULL, NULL, NULL);

static PARCDeque *
_create(const PARCObjectDescriptor *interface)
{
    PARCDeque *result = parcObject_CreateInstance(PARCDeque);

    if (result != NULL) {
        result->object = *interface;
        result->head = NULL;
        result->tail = NULL;
        result->size = 0;
    }
    return result;
}

PARCIterator *
parcDeque_Iterator(PARCDeque *deque)
{
    PARCIterator *iterator = parcIterator_Create(deque,
                                                 (void *(*)(PARCObject *))_parcDequeNode_Init,
                                                 (bool (*)(PARCObject *, void *))_parcDequeNode_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcDequeNode_Next,
                                                 NULL,
                                                 (void *(*)(PARCObject *, void *))_parcDequeNode_Element,
                                                 (void (*)(PARCObject *, void *))_parcDequeNode_Fini,
                                                 NULL);

    return iterator;
}

PARCDeque *
parcDeque_Create(void)
{
    static PARCObjectDescriptor defaultObjectInterface = {
        .destroy  = (PARCObjectDestroy *) NULL,
        .copy     = (PARCObjectCopy *) _defaultElementCopy,
        .toString = (PARCObjectToString *) NULL,
        .equals   = (PARCObjectEquals *) _defaultElementEquals,
        .compare  = (PARCObjectCompare *) NULL
    };
    return _create(&defaultObjectInterface);
}

PARCDeque *
parcDeque_CreateObjectInterface(const PARCObjectDescriptor *interface)
{
    return _create(interface);
}

PARCDeque *
parcDeque_CreateCustom(bool (*elementEquals)(const void *, const void *), void *(*elementCopy)(const void *))
{
    PARCObjectDescriptor objectInterface;
    parcObject_MetaInitialize(&objectInterface);

    objectInterface.equals = elementEquals != NULL ? elementEquals : _defaultElementEquals;
    objectInterface.copy = elementCopy != NULL ? elementCopy : _defaultElementCopy;

    return _create(&objectInterface);
}

parcObject_ImplementAcquire(parcDeque, PARCDeque);

parcObject_ImplementRelease(parcDeque, PARCDeque);

PARCDeque *
parcDeque_Copy(const PARCDeque *deque)
{
    PARCDeque *result = _create(&deque->object);

    struct parc_deque_node *node = deque->head;

    while (node != NULL) {
        parcDeque_Append(result, deque->object.copy(node->element));
        node = node->next;
    }

    return result;
}

PARCDeque *
parcDeque_Append(PARCDeque *deque, void *element)
{
    struct parc_deque_node *node = _parcDequeNode_Create(element, deque->tail, NULL);

    if (deque->tail == NULL) {
        deque->tail = node;
    } else {
        deque->tail->next = node;
        deque->tail = node;
    }

    if (deque->head == NULL) {
        deque->head = deque->tail;
    }

    deque->size++;

    return deque;
}

PARCDeque *
parcDeque_Prepend(PARCDeque *deque, void *element)
{
    struct parc_deque_node *node = _parcDequeNode_Create(element, NULL, deque->head);

    if (deque->head == NULL) {
        deque->head = node;
    } else {
        deque->head->previous = node;
        deque->head = node;
    }

    if (deque->tail == NULL) {
        deque->tail = deque->head;
    }
    deque->size++;

    _parcDequeNode_AssertInvariants(node);
    _parcDeque_AssertInvariants(deque);

    return deque;
}

void *
parcDeque_RemoveFirst(PARCDeque *deque)
{
    void *result = NULL;

    if (deque->head != NULL) {
        struct parc_deque_node *node = deque->head;
        result = node->element;
        if (deque->head == deque->tail) {
            deque->head = NULL;
            deque->tail = NULL;
        } else {
            deque->head = node->next;
            deque->head->previous = NULL;
        }
        parcMemory_Deallocate((void **) &node);
        deque->size--;
    }

    _parcDeque_AssertInvariants(deque);

    return result;
}

void *
parcDeque_RemoveLast(PARCDeque *deque)
{
    void *result = NULL;

    if (deque->tail != NULL) {
        struct parc_deque_node *node = deque->tail;
        deque->tail = node->previous;
        deque->tail->next = NULL;

        result = node->element;
        parcMemory_Deallocate((void **) &node);
        deque->size--;
    }

    _parcDeque_AssertInvariants(deque);
    return result;
}

void *
parcDeque_PeekFirst(const PARCDeque *deque)
{
    void *result = NULL;

    if (deque->head != NULL) {
        struct parc_deque_node *node = deque->head;
        result = node->element;
    }
    return result;
}

void *
parcDeque_PeekLast(const PARCDeque *deque)
{
    void *result = NULL;

    if (deque->tail != NULL) {
        struct parc_deque_node *node = deque->tail;
        result = node->element;
    }
    return result;
}

size_t
parcDeque_Size(const PARCDeque *deque)
{
    return deque->size;
}

bool
parcDeque_IsEmpty(const PARCDeque *deque)
{
    return (parcDeque_Size(deque) == 0);
}

void *
parcDeque_GetAtIndex(const PARCDeque *deque, size_t index)
{
    if (index > (parcDeque_Size(deque) - 1)) {
        parcTrapOutOfBounds(index, "[0, %zd]", parcDeque_Size(deque) - 1);
    }
    struct parc_deque_node *node = deque->head;
    while (index--) {
        node = node->next;
    }

    return node->element;
}

bool
parcDeque_Equals(const PARCDeque *x, const PARCDeque *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->object.equals == y->object.equals) {
        if (x->size == y->size) {
            struct parc_deque_node *xNode = x->head;
            struct parc_deque_node *yNode = y->head;

            while (xNode != NULL) {
                if (x->object.equals(xNode->element, yNode->element) == false) {
                    return false;
                }
                xNode = xNode->next;
                yNode = yNode->next;
            }
            return true;
        }
    }
    return false;
}

void
parcDeque_Display(const PARCDeque *deque, const int indentation)
{
    if (deque == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCDeque@NULL");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCDeque@%p {", (void *) deque);

        struct parc_deque_node *node = deque->head;

        while (node != NULL) {
            parcDisplayIndented_PrintLine(indentation + 1,
                                          ".previous=%11p, %11p=%11p, .next=%11p",
                                          node->previous, node, node->element, node->next);
            node = node->next;
        }

        parcDisplayIndented_PrintLine(indentation, "}\n");
    }
}
