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
#include <stdarg.h>

#include <parc/algol/parc_Object.h>

#include <parc/algol/parc_List.h>
#include <parc/algol/parc_Memory.h>

struct parc_list {
    void *instance;
    const PARCListInterface *interface;
};

static void
_destroy(PARCList **listPtr)
{
    PARCList *list = *listPtr;

    (list->interface->Destroy)(&list->instance);
}

parcObject_ExtendPARCObject(PARCList, _destroy, parcList_Copy, NULL, parcList_Equals, NULL, parcList_HashCode, NULL);

PARCList *
parcList(void *instance, PARCListInterface *interface)
{
    PARCList *result = parcObject_CreateInstance(PARCList);
    if (result != NULL) {
        result->instance = instance;
        result->interface = interface;
    }

    return result;
}

PARCList *
parcList_Create(PARCObject *instance, PARCListInterface *interface)
{
    PARCList *result = parcObject_CreateInstance(PARCList);
    if (result != NULL) {
        result->instance = parcObject_Acquire(instance);
        result->interface = interface;
    }

    return result;
}

parcObject_ImplementAcquire(parcList, PARCList);

parcObject_ImplementRelease(parcList, PARCList);

PARCList *
parcList_Copy(const PARCList *list)
{
    PARCList *result = parcObject_CreateInstance(PARCList);
    if (result != NULL) {
        result->instance = (list->interface->Copy)(list->instance);
        result->interface = list->interface;
    }

    return result;
}

bool
parcList_IsEmpty(const PARCList *list)
{
    bool result = false;
    if (list->interface->IsEmpty) {
        result = (list->interface->IsEmpty)(list->instance);
    } else {
        result = (parcList_Size(list) == 0);
    }
    return result;
}

bool
parcList_Add(PARCList *list, void *element)
{
    return (list->interface->Add)(list->instance, element);
}

bool
parcList_AddAll(PARCList *list, size_t argc, void **argv)
{
    for (int i = 0; i < argc; i++) {
        (list->interface->Add)(list->instance, argv[i]);
    }
    return true;
}

void
parcList_AddAtIndex(PARCList *list, int index, void *element)
{
    (list->interface->AddAtIndex)(list->instance, index, element);
}

bool
parcList_AddCollection(PARCList *list, PARCCollection *collection)
{
    return (list->interface->AddCollection)(list->instance, collection);
}

bool
parcList_AddCollectionAtIndex(PARCList *list, int index, PARCCollection *collection)
{
    return (list->interface->AddCollectionAtIndex)(list->instance, index, collection);
}

void
parcList_Clear(PARCList *list)
{
    if (!list->interface->Clear) {
        for (size_t i = 0; i < parcList_Size(list); i++) {
            parcList_RemoveAtIndex(list, i);
        }
    } else {
        (list->interface->Clear)(list->instance);
    }
}

bool
parcList_Contains(const PARCList *list, void *element)
{
    return (list->interface->Contains)(list->instance, element);
}

bool
parcList_ContainsCollection(PARCList *list, PARCCollection *collection)
{
    return (list->interface->ContainsCollection)(list->instance, collection);
}

bool
parcList_Equals(const PARCList *x, const PARCList *y)
{
    return (x->interface->Equals)(x->instance, y->instance);
}

void *
parcList_GetAtIndex(const PARCList *list, size_t index)
{
    return (list->interface->GetAtIndex)(list->instance, index);
}

int
parcList_HashCode(const PARCList *list)
{
    return (int)(list->interface->HashCode)(list->instance);
}

ssize_t
parcList_IndexOf(const PARCList *list, PARCObject *element)
{
    ssize_t result = -1;

    if (list->interface->IndexOf) {
        result = (list->interface->IndexOf)(list->instance, element);
    } else {
        for (size_t i = 0; i < parcList_Size(list); i++) {
            PARCObject *e = parcList_GetAtIndex(list, i);
            if (parcObject_Equals(e, element)) {
                result = i;
                break;
            }
        }
    }

    return result;
}

ssize_t
parcList_LastIndexOf(const PARCList *list, PARCObject *element)
{
    ssize_t result = -1;

    if (list->interface->LastIndexOf) {
        result = (list->interface->LastIndexOf)(list->instance, element);
    } else {
        for (ssize_t i = parcList_Size(list) - 1; i >= 0; i--) {
            PARCObject *e = parcList_GetAtIndex(list, i);
            if (parcObject_Equals(e, element)) {
                result = i;
                break;
            }
        }
    }

    return result;
}

PARCObject *
parcList_RemoveAtIndex(PARCList *list, size_t index)
{
    if (list->interface->RemoveAtIndex) {
        return (list->interface->RemoveAtIndex)(list->instance, index);
    } else {
        return NULL;
    }
}

bool
parcList_Remove(PARCList *list, PARCObject *element)
{
    bool result = false;

    if (list->interface->Remove != NULL) {
        result = (list->interface->Remove)(list->instance, element);
    } else {
        for (size_t i = 0; i < parcList_Size(list); i++) {
            void *e = parcList_GetAtIndex(list, i);
            if (parcObject_Equals(e, element)) {
                parcList_RemoveAtIndex(list, i);
                result = true;
                break;
            }
        }
    }

    return result;
}

bool
parcList_RemoveCollection(PARCList *list, PARCCollection *collection)
{
    return (list->interface->RemoveCollection)(list->instance, collection);
}

bool
parcList_RetainCollection(PARCList *list, PARCCollection *collection)
{
    return (list->interface->RetainCollection)(list->instance, collection);
}

PARCObject *
parcList_SetAtIndex(PARCList *list, size_t index, void *element)
{
    return (list->interface->SetAtIndex)(list->instance, index, element);
}

size_t
parcList_Size(const PARCList *list)
{
    return (list->interface->Size)(list->instance);
}

PARCList *
parcList_SubList(PARCList *list, size_t fromIndex, size_t toIndex)
{
    return (list->interface->SubList)(list->instance, fromIndex, toIndex);
}

void**
parcList_ToArray(PARCList *list)
{
    return (list->interface->ToArray)(list->instance);
}
