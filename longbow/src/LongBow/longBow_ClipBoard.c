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
#include <stdlib.h>
#include <string.h>

#include <LongBow/longBow_ClipBoard.h>

#include <LongBow/private/longBow_Memory.h>
#include <LongBow/private/longBow_ArrayList.h>

struct LongBowClipBoard {
    LongBowArrayList *list;
};

typedef struct Property {
    const char *name;
    char *value;
} _Property;

static void
_property_Destroy(_Property **pointer)
{
    _Property *property = *pointer;
    longBowMemory_Deallocate((void **) &property->name);
    longBowMemory_Deallocate((void **) property);
}

void
longBowClipBoard_Destroy(LongBowClipBoard **pointer)
{
    LongBowClipBoard *clipboard = *pointer;

    longBowArrayList_Destroy(&clipboard->list);
    longBowMemory_Deallocate((void **) pointer);
}

LongBowClipBoard *
longBowClipBoard_Create(void)
{
    LongBowClipBoard *result = longBowMemory_Allocate(sizeof(LongBowClipBoard));

    if (result != NULL) {
        result->list = longBowArrayList_Create((void (*)(void **))_property_Destroy);
    }

    return result;
}

static _Property *
_longBowClipBoard_Get(const LongBowClipBoard *clipBoard, const char *name)
{
    _Property *result = NULL;

    for (size_t index = 0; index < longBowArrayList_Length(clipBoard->list); index++) {
        _Property *property = longBowArrayList_Get(clipBoard->list, index);
        if (strcmp(property->name, name) == 0) {
            result = property;
            break;
        }
    }
    return result;
}

void *
longBowClipBoard_Get(const LongBowClipBoard *clipBoard, const char *name)
{
    _Property *property = _longBowClipBoard_Get(clipBoard, name);

    if (property != NULL) {
        return property->value;
    }

    return NULL;
}

char *
longBowClipBoard_GetAsCString(const LongBowClipBoard *clipBoard, const char *name)
{
    return (char *) longBowClipBoard_Get(clipBoard, name);
}

uint64_t
longBowClipBoard_GetAsInt(const LongBowClipBoard *clipBoard, const char *name)
{
    return (uint64_t) longBowClipBoard_Get(clipBoard, name);
}

void *
longBowClipBoard_Set(LongBowClipBoard *clipBoard, const char *name, void *value)
{
    void *result = NULL;

    _Property *property = _longBowClipBoard_Get(clipBoard, name);
    if (property == NULL) {
        property = longBowMemory_Allocate(sizeof(_Property));
        property->name = longBowMemory_StringCopy(name);
        property->value = value;
        longBowArrayList_Add(clipBoard->list, property);
    } else {
        result = property->value;
        property->value = value;
    }
    return result;
}

void *
longBowClipBoard_SetInt(LongBowClipBoard *clipBoard, const char *name, uint64_t value)
{
    return longBowClipBoard_Set(clipBoard, name, (void *) (uintptr_t) value);
}

void *
longBowClipBoard_SetCString(LongBowClipBoard *clipBoard, const char *name, char *value)
{
    return longBowClipBoard_Set(clipBoard, name, (char *) value);
}

bool
longBowClipBoard_Exists(const LongBowClipBoard *clipBoard, const char *name)
{
    return (_longBowClipBoard_Get(clipBoard, name) != NULL);
}

bool
longBowClipBoard_Delete(LongBowClipBoard *clipBoard, const char *name)
{
    bool result = false;

    for (size_t index = 0; index < longBowArrayList_Length(clipBoard->list); index++) {
        _Property *property = longBowArrayList_Get(clipBoard->list, index);
        if (strcmp(property->name, name) == 0) {
            longBowArrayList_RemoveAtIndex(clipBoard->list, index);
            result = true;
        }
    }

    return result;
}
