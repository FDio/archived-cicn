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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_SortedList.h>
#include <parc/algol/parc_LinkedList.h>

struct PARCSortedList {
    PARCLinkedList *list;
    PARCSortedListEntryCompareFunction compare;
};

static void
_parcSortedList_Finalize(PARCSortedList **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCSortedList pointer.");
    PARCSortedList *instance = *instancePtr;

    parcSortedList_OptionalAssertValid(instance);

    parcLinkedList_Release(&instance->list);
}

parcObject_ImplementAcquire(parcSortedList, PARCSortedList);

parcObject_ImplementRelease(parcSortedList, PARCSortedList);

parcObject_ExtendPARCObject(PARCSortedList, _parcSortedList_Finalize, parcSortedList_Copy, parcSortedList_ToString, parcSortedList_Equals, NULL, parcSortedList_HashCode, parcSortedList_ToJSON);


void
parcSortedList_AssertValid(const PARCSortedList *instance)
{
    assertTrue(parcSortedList_IsValid(instance),
               "PARCSortedList is not valid.");
}


PARCSortedList *
parcSortedList_Create(void)
{
    PARCSortedList *result = parcSortedList_CreateCompare(parcObject_Compare);

    return result;
}

PARCSortedList *
parcSortedList_CreateCompare(PARCSortedListEntryCompareFunction compare)
{
    PARCSortedList *result = parcObject_CreateInstance(PARCSortedList);

    if (result != NULL) {
        result->list = parcLinkedList_Create();
        result->compare = compare;
    }

    return result;
}

PARCSortedList *
parcSortedList_Copy(const PARCSortedList *original)
{
    PARCSortedList *result = parcObject_CreateInstance(PARCSortedList);

    if (result != NULL) {
        result->list = parcLinkedList_Copy(original->list);
    }

    return result;
}

void
parcSortedList_Display(const PARCSortedList *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCSortedList@%p {", instance);
    parcLinkedList_Display(instance->list, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcSortedList_Equals(const PARCSortedList *x, const PARCSortedList *y)
{
    return parcLinkedList_Equals(x->list, y->list);
}

PARCHashCode
parcSortedList_HashCode(const PARCSortedList *instance)
{
    PARCHashCode result = parcLinkedList_HashCode(instance->list);

    return result;
}

bool
parcSortedList_IsValid(const PARCSortedList *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcSortedList_ToJSON(const PARCSortedList *instance)
{
    PARCJSON *result = parcJSON_Create();

    return result;
}

char *
parcSortedList_ToString(const PARCSortedList *instance)
{
    char *result = parcMemory_Format("PARCSortedList@%p\n", instance);

    return result;
}

size_t
parcSortedList_Size(const PARCSortedList *list)
{
    return parcLinkedList_Size(list->list);
}

PARCObject *
parcSortedList_GetAtIndex(const PARCSortedList *list, const size_t index)
{
    return parcLinkedList_GetAtIndex(list->list, index);
}

PARCObject *
parcSortedList_GetFirst(const PARCSortedList *list)
{
    return parcLinkedList_GetAtIndex(list->list, 0);
}

PARCObject *
parcSortedList_GetLast(const PARCSortedList *list)
{
    return parcLinkedList_GetAtIndex(list->list, parcLinkedList_Size(list->list) - 1);
}

PARCObject *
parcSortedList_RemoveFirst(PARCSortedList *list)
{
    PARCObject *result = parcLinkedList_RemoveFirst(list->list);

    return result;
}

PARCObject *
parcSortedList_RemoveLast(PARCSortedList *list)
{
    PARCObject *result = parcLinkedList_RemoveLast(list->list);

    return result;
}

bool
parcSortedList_Remove(PARCSortedList *list, const PARCObject *object)
{
    bool result = false;

    PARCIterator *iterator = parcSortedList_CreateIterator(list);

    while (parcIterator_HasNext(iterator)) {
        PARCObject *o = parcIterator_Next(iterator);
        if (parcObject_Equals(object, o)) {
            parcIterator_Remove(iterator);
            result = true;
            break;
        }
    }

    parcIterator_Release(&iterator);

    return result;
}

static size_t
_parcSortedList_GetInsertionIndex(const PARCSortedList *instance, const PARCObject *element)
{
    ssize_t low = 0;
    ssize_t high = parcLinkedList_Size(instance->list) - 1;

    if (high == -1) {
        return 0;
    }

    while (true) {
        ssize_t midpoint = (low + (high - low) / 2);
        PARCObject *e = parcLinkedList_GetAtIndex(instance->list, midpoint);
        int signum = instance->compare(element, e);
        if (high == low) {
            if (signum < 0) {
                return high;
            } else if (signum > 0) {
                return low + 1;
            } else {
                return low;
            }
        }

        if (signum < 0) {
            high = midpoint;
        } else if (signum > 0) {
            low = midpoint + 1;
        } else {
            return midpoint;
        }
    }

    return -1;
}

PARCIterator *
parcSortedList_CreateIterator(PARCSortedList *instance)
{
    return parcLinkedList_CreateIterator(instance->list);
}

void
parcSortedList_Add(PARCSortedList *instance, PARCObject *element)
{
    size_t insertionPoint = _parcSortedList_GetInsertionIndex(instance, element);
    assertTrue(insertionPoint >= 0 && insertionPoint <= parcLinkedList_Size(instance->list),
               "%zd is bad insertion point.  Must be >=0 and <= %zd", insertionPoint, parcLinkedList_Size(instance->list));

    parcLinkedList_InsertAtIndex(instance->list, insertionPoint, element);
}
