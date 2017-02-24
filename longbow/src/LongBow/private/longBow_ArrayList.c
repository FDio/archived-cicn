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
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <LongBow/runtime.h>
#include <LongBow/private/longBow_ArrayList.h>
#include <LongBow/private/longBow_Memory.h>

struct longbow_array_list {
    void **array;
    size_t numberOfElements;
    size_t limit;
    void (*destroyElement)(void **elementAddress);
};

static LongBowArrayList *_longBowArrayList_EnsureRemaining(LongBowArrayList *array, size_t remnant);
static LongBowArrayList *_longBowArrayList_EnsureCapacity(LongBowArrayList *array, size_t newCapacity);

void
longBowArrayList_AssertValid(const LongBowArrayList *array)
{
    if (array == NULL) {
        printf("Parameter must be a non-null pointer to a LongBowArrayList instance\n");
        abort();
    }
}

LongBowArrayList *
longBowArrayList_Add(LongBowArrayList *array, const void *pointer)
{
    longBowArrayList_AssertValid(array);

    if (_longBowArrayList_EnsureRemaining(array, 1) == NULL) {
        return NULL;
    }
    array->array[array->numberOfElements++] = (void *) pointer;

    return array;
}

static size_t
_longBowArrayList_Remaining(const LongBowArrayList *array)
{
    longBowArrayList_AssertValid(array);

    return array->limit - array->numberOfElements;
}

static LongBowArrayList *
_longBowArrayList_EnsureCapacity(LongBowArrayList *array, size_t newCapacity)
{
    longBowArrayList_AssertValid(array);

    void *newArray = longBowMemory_Reallocate(array->array, newCapacity * sizeof(void *));

    if (newArray == NULL) {
        return NULL;
    }
    array->array = newArray;
    array->limit = newCapacity;

    return array;
}

static LongBowArrayList *
_longBowArrayList_EnsureRemaining(LongBowArrayList *array, size_t remnant)
{
    longBowArrayList_AssertValid(array);

    if (_longBowArrayList_Remaining(array) < remnant) {
        size_t newCapacity = longBowArrayList_Length(array) + remnant;
        return _longBowArrayList_EnsureCapacity(array, newCapacity);
    }
    return array;
}

bool
longBowArrayList_Equals(const LongBowArrayList *a, const LongBowArrayList *b)
{
    if (a != b) {
        if (a == NULL || b == NULL) {
            return false;
        }
        if (a->numberOfElements == b->numberOfElements) {
            for (size_t i = 0; i < a->numberOfElements; i++) {
                if (a->array[i] != b->array[i]) {
                    return false;
                }
            }
        }
    }

    return true;
}

void *
longBowArrayList_Get(const LongBowArrayList *array, size_t index)
{
    longBowArrayList_AssertValid(array);

    assert(index < array->numberOfElements);

    return array->array[index];
}

size_t
longBowArrayList_Length(const LongBowArrayList *array)
{
    longBowArrayList_AssertValid(array);

    return array->numberOfElements;
}

LongBowArrayList *
longBowArrayList_Create(void (*destroyElement)(void **elementAddress))
{
    LongBowArrayList *result = longBowMemory_Allocate(sizeof(LongBowArrayList));

    if (result != NULL) {
        result->numberOfElements = 0;
        result->limit = 0;
        result->array = NULL;
        result->destroyElement = destroyElement;
    }

    return result;
}

LongBowArrayList *
longBowArrayList_Create_Capacity(void (*destroyElement)(void **elementAddress), size_t size)
{
    LongBowArrayList *result = longBowArrayList_Create(destroyElement);
    if (result != NULL) {
        _longBowArrayList_EnsureRemaining(result, size);
    }

    return result;
}

void
longBowArrayList_Destroy(LongBowArrayList **arrayPtr)
{
    assertNotNull(arrayPtr, "Parameter must be a non-null pointer to a LongBow_ArrayList pointer.");

    LongBowArrayList *array = *arrayPtr;

    longBowArrayList_AssertValid(array);

    assertTrue(array->numberOfElements == 0 ? true : array->array != NULL, "LongBow_ArrayList is inconsistent.");

    if (array->destroyElement != NULL) {
        for (size_t i = 0; i < array->numberOfElements; i++) {
            array->destroyElement(&array->array[i]);
            array->array[i] = NULL;
        }
    }

    if (array->array != NULL) {
        longBowMemory_Deallocate((void **) &array->array);
    }

    longBowMemory_Deallocate((void **) arrayPtr);
}

void **
longBowArrayList_GetArray(const LongBowArrayList *array)
{
    longBowArrayList_AssertValid(array);
    return array->array;
}

LongBowArrayList *
longBowArrayList_Copy(const LongBowArrayList *original)
{
    longBowArrayList_AssertValid(original);

    LongBowArrayList *result = longBowMemory_Allocate(sizeof(LongBowArrayList));

    if (result != NULL) {
        for (size_t i = 0; i < original->numberOfElements; i++) {
            longBowArrayList_Add(result, original->array[i]);
        }
    }

    return result;
}

void
longBowArrayList_StdlibFreeFunction(void **elementPtr)
{
    if (elementPtr != NULL) {
        free(*elementPtr);
        *elementPtr = 0;
    }
}

LongBowArrayList *
longBowArrayList_RemoveAtIndex(LongBowArrayList *array, size_t index)
{
    longBowArrayList_AssertValid(array);

    size_t length = longBowArrayList_Length(array);
    assertTrue(index < length, "Index must be ( 0 <= index < %zd). Actual=%zd", length, index);

    if (index < length) {
        // Destroy the element at the given index.
        if (array->destroyElement != NULL) {
            array->destroyElement(&array->array[index]);
        }

        // Adjust the list to elide the element.
        for (size_t i = index; i < length; i++) {
            array->array[i] = array->array[i + 1];
        }
        array->numberOfElements--;
    }

    return array;
}

LongBowArrayList *
longBowArrayList_Add_AtIndex(LongBowArrayList *array, const void *pointer, size_t index)
{
    longBowArrayList_AssertValid(array);
    size_t length = longBowArrayList_Length(array);

    if (index > array->limit) {
        // We need to grow the array to fit this element.
        _longBowArrayList_EnsureCapacity(array, index + 1);
        array->numberOfElements = index + 1;
    } else {
        // Create space and grow the array if needed
        _longBowArrayList_EnsureRemaining(array, length + 1);
        for (size_t i = index; i < length; i++) {
            array->array[i + 1] = array->array[i];
        }
        array->numberOfElements++;
    }

    array->array[index] = (void *) pointer;


    return array;
}

bool
longBowArrayList_Replace(LongBowArrayList *array, const void *old, void *new)
{
    for (size_t i = 0; i < longBowArrayList_Length(array); i++) {
        if (array->array[i] == old) {
            array->array[i] = new;
            return true;
        }
    }
    return false;
}
