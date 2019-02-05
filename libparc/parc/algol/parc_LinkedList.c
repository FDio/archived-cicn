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

#ifndef _WIN32
#include <sys/queue.h>
#endif

#include <config.h>
#include <stdio.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

static PARCListInterface *PARCLinkedListAsPARCList = &(PARCListInterface) {
    .Add = (bool (*)(void *, void *))parcLinkedList_Append,
    .AddAtIndex = (void (*)(void *, int index, PARCObject *))parcLinkedList_InsertAtIndex,
    .AddCollection = (bool (*)(void *, PARCCollection *))NULL,
    .AddCollectionAtIndex = (bool (*)(void *, int index, PARCCollection *))NULL,
    .Clear = (void (*)(void *))NULL,
    .Contains = (bool (*)(const void *, const PARCObject *))parcLinkedList_Contains,
    .ContainsCollection = (bool (*)(const void *, const PARCCollection *))NULL,
    .Copy = (void *    (*)(const PARCList *))parcLinkedList_Copy,
    .Destroy = (void (*)(void **))parcLinkedList_Release,
    .Equals = (bool (*)(const void *, const void *))parcLinkedList_Equals,
    .GetAtIndex = (PARCObject * (*)(const void *, size_t))parcLinkedList_GetAtIndex,
    .HashCode = (PARCHashCode (*)(const void *))parcLinkedList_HashCode,
    .IndexOf = (size_t (*)(const void *, const PARCObject *element))NULL,
    .IsEmpty = (bool (*)(const void *))parcLinkedList_IsEmpty,
    .LastIndexOf = (size_t (*)(void *, const PARCObject *element))NULL,
    .Remove = (bool (*)(void *, const PARCObject *element))parcLinkedList_Remove,
    .RemoveAtIndex = (PARCObject * (*)(PARCList *, size_t))parcLinkedList_RemoveAtIndex,
    .RemoveCollection = (bool (*)(void *, const PARCCollection *))NULL,
    .RetainCollection = (bool (*)(void *, const PARCCollection *))NULL,
    .SetAtIndex = (PARCObject * (*)(void *, size_t index, PARCObject *))parcLinkedList_SetAtIndex,
    .Size = (size_t (*)(const void *))parcLinkedList_Size,
    .SubList = (PARCList * (*)(const void *, size_t, size_t))NULL,
    .ToArray = (void**      (*)(const void *))NULL,
};

typedef struct parc_linkedlist_node {
    PARCObject *object;
    struct parc_linkedlist_node *previous;
    struct parc_linkedlist_node *next;
} _PARCLinkedListNode;

struct parc_linkedlist {
    _PARCLinkedListNode *head;
    _PARCLinkedListNode *tail;
    size_t size;
};

static inline _PARCLinkedListNode *
_parcLinkedListNode_getByIndex(const PARCLinkedList *list, size_t index)
{
    _PARCLinkedListNode *node = list->head;
    while (index-- && node != NULL) {
        node = node->next;
    }
    return node;
}

static inline _PARCLinkedListNode *
_parcLinkedListNode_getByValue(const PARCLinkedList *list, const PARCObject *value)
{
    _PARCLinkedListNode *node = list->head;
    while (node != NULL && parcObject_Equals(node->object, value) == false) {
        node = node->next;
    }
    return node;
}

static bool
_parcLinkedListNode_IsValid(const _PARCLinkedListNode *node)
{
    bool result = false;

    if (node != NULL) {
        if (node->object != NULL) {
            if (parcObject_IsValid(node->object)) {
                if (node->previous) {
                    if (node->previous->next == node) {
                        if (parcObject_IsValid(node->previous->object)) {
                            result = true;
                        }
                    }
                } else {
                    result = true;
                }
                if (node->next != NULL) {
                    if (node->next->previous == node) {
                        if (parcObject_IsValid(node->next->object)) {
                            result = true;
                        }
                    }
                } else {
                    result = true;
                }
            }
        }
    }

    return result;
}

static inline _PARCLinkedListNode *
_parcLinkedListNode_Create(const PARCObject *object, _PARCLinkedListNode *previous, _PARCLinkedListNode *next)
{
    _PARCLinkedListNode *result = parcMemory_Allocate(sizeof(_PARCLinkedListNode));
    if (result != NULL) {
        result->object = parcObject_Acquire(object);
        result->next = next;
        result->previous = previous;
    }

    return result;
}

static void
_parcLinkedListIterator_IsValid(const _PARCLinkedListNode *node)
{
    if (node != NULL) {
        parcAssertTrue(_parcLinkedListNode_IsValid(node), "node is invalid");
    }
}

static void
_parcLinkedListNode_Destroy(PARCLinkedList *list __attribute__((unused)), _PARCLinkedListNode **nodePtr)
{
    _PARCLinkedListNode *node = *nodePtr;

    parcObject_Release(&node->object);
    parcMemory_Deallocate((void **) nodePtr);
}

static bool
_parcLinkedList_Destructor(PARCLinkedList **listPtr)
{
    PARCLinkedList *list = *listPtr;

    _PARCLinkedListNode *next = NULL;

    for (_PARCLinkedListNode *node = list->head; node != NULL; node = next) {
        next = node->next;
        _parcLinkedListNode_Destroy(list, &node);
    }
    return true;
}

static _PARCLinkedListNode *
_parcLinkedIterator_Init(PARCLinkedList *list __attribute__((unused)))
{
    return NULL;
}

static bool
_parcLinkedListNode_Fini(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node __attribute__((unused)))
{
    return true;
}

static struct parc_linkedlist_node *
_parcLinkedListNode_Next(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node)
{
    struct parc_linkedlist_node *result = NULL;

    if (node == NULL) {
        result = list->head;
    } else {
        parcAssertTrue(_parcLinkedListNode_IsValid(node), "node is invalid");
        parcTrapOutOfBoundsIf(node->next == NULL, "No more elements.");
        result = node->next;
    }

    parcAssertTrue(_parcLinkedListNode_IsValid(result), "result is invalid");
    parcObject_OptionalAssertValid(result->object);

    return result;
}

static inline PARCObject *
_parcLinkedListNode_Delete(PARCLinkedList *list, _PARCLinkedListNode *node)
{
    PARCObject *result = node->object;

    list->size--;

    if (node == list->head) {
        list->head = node->next;
    }
    if (node == list->tail) {
        list->tail = node->previous;
    }
    if (node->previous) {
        node->previous->next = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    }

    parcMemory_Deallocate((void **) &node);

    return result;
}

static void
_parcLinkedListNode_Remove(PARCLinkedList *list, _PARCLinkedListNode **nodePtr)
{
    parcLinkedList_OptionalAssertValid(list);

    _PARCLinkedListNode *node = *nodePtr;

    if (node != NULL) {
        *nodePtr = node->previous;

        PARCObject *object = _parcLinkedListNode_Delete(list, node);
        parcObject_Release(&object);

        parcLinkedList_OptionalAssertValid(list);
    }
}

static bool
_parcLinkedListNode_HasNext(PARCLinkedList *list, const _PARCLinkedListNode *node)
{
    bool result = false;

    if (node == NULL) {
        result = (list->head != NULL);
        if (result) {
            parcAssertTrue(_parcLinkedListNode_IsValid(list->head), "node is invalid");
        }
    } else {
        result = node->next != NULL;
        if (result) {
            parcAssertTrue(_parcLinkedListNode_IsValid(node->next), "node is invalid");
        }
    }


    return result;
}

static void *
_parcLinkedListNode_Element(PARCLinkedList *list __attribute__((unused)), const _PARCLinkedListNode *node)
{
    return node->object;
}

parcObject_Override(PARCLinkedList, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcLinkedList_Destructor,
                    .copy = (PARCObjectCopy *) parcLinkedList_Copy,
                    .equals = (PARCObjectEquals *) parcLinkedList_Equals,
                    .hashCode = (PARCObjectHashCode *) parcLinkedList_HashCode,
                    .display = (PARCObjectDisplay *) parcLinkedList_Display);

PARCIterator *
parcLinkedList_CreateIterator(PARCLinkedList *list)
{
    PARCIterator *iterator = parcIterator_Create(list,
                                                 (void *(*)(PARCObject *))_parcLinkedIterator_Init,
                                                 (bool (*)(PARCObject *, void *))_parcLinkedListNode_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcLinkedListNode_Next,
                                                 (void (*)(PARCObject *, void **))_parcLinkedListNode_Remove,
                                                 (void *(*)(PARCObject *, void *))_parcLinkedListNode_Element,
                                                 (void (*)(PARCObject *, void *))_parcLinkedListNode_Fini,
                                                 (void (*)(const void *))_parcLinkedListIterator_IsValid);

    return iterator;
}

PARCLinkedList *
parcLinkedList_Create(void)
{
    PARCLinkedList *result = parcObject_CreateInstance(PARCLinkedList);

    if (result != NULL) {
        result->head = NULL;
        result->tail = NULL;
        result->size = 0;
    }
    return result;
}

bool
parcLinkedList_IsValid(const PARCLinkedList *list)
{
    bool result = false;

    if (list != NULL) {
        if (parcObject_IsValid(list)) {
            if (list->size > 0) {
                if (list->head != NULL) {
                    if (list->tail != NULL) {
                        result = true;
                        for (_PARCLinkedListNode *node = list->head; node != NULL; node = node->next) {
                            if (_parcLinkedListNode_IsValid(node) == false) {
                                result = false;
                                break;
                            }
                        }
                    }
                }
            } else {
                if (list->head == NULL) {
                    if (list->tail == NULL) {
                        result = true;
                    }
                }
            }
        }
    }

    return result;
}

void
parcLinkedList_AssertValid(const PARCLinkedList *instance)
{
    parcAssertTrue(parcLinkedList_IsValid(instance),
               "PARCLinkedList is not valid.");
}

parcObject_ImplementAcquire(parcLinkedList, PARCLinkedList);

parcObject_ImplementRelease(parcLinkedList, PARCLinkedList);

PARCLinkedList *
parcLinkedList_Copy(const PARCLinkedList *list)
{
    PARCLinkedList *result = parcLinkedList_Create();

    struct parc_linkedlist_node *node = list->head;

    while (node != NULL) {
        parcLinkedList_Append(result, node->object);
        node = node->next;
    }

    return result;
}

bool
parcLinkedList_Contains(const PARCLinkedList *list, const PARCObject *element)
{
    bool result = false;

    struct parc_linkedlist_node *node = list->head;

    while (node != NULL) {
        if (parcObject_Equals(node->object, element)) {
            result = true;
            break;
        }
        node = node->next;
    }

    return result;
}

PARCLinkedList *
parcLinkedList_Append(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *node = _parcLinkedListNode_Create(element, list->tail, NULL);

    if (list->tail == NULL) {
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    if (list->head == NULL) {
        list->head = list->tail;
    }

    list->size++;

    return list;
}

PARCLinkedList *
parcLinkedList_AppendAll(PARCLinkedList *list, const PARCLinkedList *other)
{
    PARCIterator *iterator = parcLinkedList_CreateIterator((PARCLinkedList *) other);
    while (parcIterator_HasNext(iterator)) {
        PARCObject *object = parcIterator_Next(iterator);
        parcLinkedList_Append(list, object);
    }
    parcIterator_Release(&iterator);

    return list;
}

PARCLinkedList *
parcLinkedList_Prepend(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *node = _parcLinkedListNode_Create(element, NULL, list->head);

    if (list->head == NULL) {
        list->head = node;
    } else {
        list->head->previous = node;
        list->head = node;
    }

    if (list->tail == NULL) {
        list->tail = list->head;
    }
    list->size++;

    parcLinkedList_OptionalAssertValid(list);

    return list;
}

PARCObject *
parcLinkedList_RemoveFirst(PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->head != NULL) {
        _PARCLinkedListNode *node = list->head;
        result = _parcLinkedListNode_Delete(list, node);
    }

    parcLinkedList_OptionalAssertValid(list);

    return result;
}

PARCObject *
parcLinkedList_RemoveLast(PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->tail != NULL) {
        _PARCLinkedListNode *node = list->tail;
        result = _parcLinkedListNode_Delete(list, node);
    }

    parcLinkedList_OptionalAssertValid(list);
    return result;
}

bool
parcLinkedList_Remove(PARCLinkedList *list, const PARCObject *element)
{
    parcAssertTrue(element != NULL, "Element must not be NULL");
    bool result = false;

    _PARCLinkedListNode *node = _parcLinkedListNode_getByValue(list, element);
    if (node != NULL) {
        PARCObject *e = _parcLinkedListNode_Delete(list, node);
        parcObject_Release(&e);
        result = true;
    }

    parcLinkedList_OptionalAssertValid(list);

    return result;
}

PARCObject *
parcLinkedList_RemoveAtIndex(PARCLinkedList *list, size_t index)
{
    PARCObject *result = NULL;

    _PARCLinkedListNode *node = _parcLinkedListNode_getByIndex(list, index);
    if (node != NULL) {
        result = _parcLinkedListNode_Delete(list, node);
    }

    return result;
}

PARCObject *
parcLinkedList_GetFirst(const PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->head != NULL) {
        _PARCLinkedListNode *node = list->head;
        result = node->object;
    }
    return result;
}

PARCObject *
parcLinkedList_GetLast(const PARCLinkedList *list)
{
    PARCObject *result = NULL;

    if (list->tail != NULL) {
        _PARCLinkedListNode *node = list->tail;
        result = node->object;
    }
    return result;
}

PARCHashCode
parcLinkedList_HashCode(const PARCLinkedList *list)
{
    PARCHashCode result = 0;

    _PARCLinkedListNode *node = list->head;
    if (node != NULL) {
        while (node != NULL) {
            result += parcObject_HashCode(node->object);
            node = node->next;
        }
    }

    return result;
}

size_t
parcLinkedList_Size(const PARCLinkedList *list)
{
    return list->size;
}

bool
parcLinkedList_IsEmpty(const PARCLinkedList *list)
{
    return (parcLinkedList_Size(list) == 0);
}

static void
_parcLinkedList_InsertInitialNode(PARCLinkedList *list, const PARCObject *element)
{
    _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, NULL, NULL);
    list->head = newNode;
    list->tail = newNode;
}

PARCLinkedList *
parcLinkedList_InsertAtIndex(PARCLinkedList *list, size_t index, const PARCObject *element)
{
    if (index == 0) {
        if (list->head == NULL) {
            _parcLinkedList_InsertInitialNode(list, element);
        } else {
            _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, NULL, list->head);

            list->head->previous = newNode;
            list->tail = list->head;
            list->head = newNode;
        }

        list->size++;
    } else if (index == list->size) {
        _PARCLinkedListNode *node = list->tail;
        node->next = _parcLinkedListNode_Create(element, node, NULL);
        list->tail = node->next;
        list->size++;
    } else {
        _PARCLinkedListNode *node = list->head;
        while (index-- && node->next != NULL) {
            node = node->next;
        }
        _PARCLinkedListNode *newNode = _parcLinkedListNode_Create(element, node->previous, node);

        node->previous->next = newNode;
        node->previous = newNode;
        list->size++;
    }

    parcLinkedList_OptionalAssertValid(list);
    return list;
}

PARCObject *
parcLinkedList_SetAtIndex(PARCLinkedList *list, size_t index, PARCObject *element)
{
    PARCObject *result = NULL;

    if (index > (parcLinkedList_Size(list) - 1)) {
        parcTrapOutOfBounds(index, "[0, %zd]", parcLinkedList_Size(list) - 1);
    }

    _PARCLinkedListNode *node = _parcLinkedListNode_getByIndex(list, index);
    if (node != NULL) {
        result = node->object;
        node->object = parcObject_Acquire(element);
    }
    return result;
}

PARCObject *
parcLinkedList_GetAtIndex(const PARCLinkedList *list, size_t index)
{
    if (index > (parcLinkedList_Size(list) - 1)) {
        parcTrapOutOfBounds(index, "[0, %zd]", parcLinkedList_Size(list) - 1);
    }

    _PARCLinkedListNode *node = _parcLinkedListNode_getByIndex(list, index);
    return (node == NULL) ? NULL : node->object;
}

bool
parcLinkedList_Equals(const PARCLinkedList *x, const PARCLinkedList *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->size == y->size) {
        _PARCLinkedListNode *xNode = x->head;
        _PARCLinkedListNode *yNode = y->head;

        while (xNode != NULL) {
            if (parcObject_Equals(xNode->object, yNode->object) == false) {
                return false;
            }
            xNode = xNode->next;
            yNode = yNode->next;
        }
        return true;
    }
    return false;
}

void
parcLinkedList_Display(const PARCLinkedList *list, const int indentation)
{
    if (list == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCLinkedList@NULL");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCLinkedList@%p { .size=%016zp, .head=%016zp, .tail=%016zp",
                                      (void *) list, list->size, list->head, list->tail);

        _PARCLinkedListNode *node = list->head;

        while (node != NULL) {
            parcDisplayIndented_PrintLine(indentation + 1,
                                          "%016zp { .previous=%016zp, %016zp, .next=%016zp }",
                                          node, node->previous, node->object, node->next);
            parcObject_Display(node->object, indentation + 2);
            node = node->next;
        }

        parcDisplayIndented_PrintLine(indentation, "}\n");
    }
}

bool
parcLinkedList_SetEquals(const PARCLinkedList *x, const PARCLinkedList *y)
{
    bool result = false;

    if (x->size == y->size) {
        for (size_t i = 0; i < x->size; i++) {
            PARCObject *xObject = parcLinkedList_GetAtIndex(x, i);
            for (size_t j = 0; j < x->size; j++) {
                PARCObject *yObject = parcLinkedList_GetAtIndex(y, j);
                if (parcObject_Equals(xObject, yObject) == false) {
                    break;
                }
            }
        }
        result = true;
    }

    return result;
}

PARCList *
parcLinkedList_AsPARCList(PARCLinkedList *list)
{
    PARCList *result = parcList_Create(list, PARCLinkedListAsPARCList);
    return result;
}

void
parcLinkedList_ApplyImpl(PARCLinkedList *list, void (*function)(PARCObject *, const void *), const void *parameter)
{
    PARCIterator *iterator = parcLinkedList_CreateIterator(list);

    while (parcIterator_HasNext(iterator)) {
        PARCObject *object = parcIterator_Next(iterator);
        function(object, parameter);
    }

    parcIterator_Release(&iterator);
}
