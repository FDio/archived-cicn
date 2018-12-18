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

#include <string.h>

#include <LongBow/private/longBow_String.h>
#include <LongBow/longBow_Properties.h>
#include <LongBow/private/longBow_ArrayList.h>
#include <LongBow/private/longBow_Memory.h>

struct LongBowProperties {
    LongBowArrayList *list;
};

typedef struct Property {
    const char *name;
    const char *value;
} _Property;

static void
_property_Destroy(_Property **pointer)
{
    _Property *property = *pointer;
    longBowMemory_Deallocate((void **) &property->name);
    longBowMemory_Deallocate((void **) &property->value);
    longBowMemory_Deallocate((void **) property);
}

LongBowProperties *
longBowProperties_Create(void)
{
    LongBowProperties *result = longBowMemory_Allocate(sizeof(LongBowProperties));

    if (result != NULL) {
        result->list = longBowArrayList_Create((void (*)(void **))_property_Destroy);
    }

    return result;
}

static _Property *
_longBowProperties_Get(const LongBowProperties *properties, const char *name)
{
    _Property *result = NULL;

    for (size_t index = 0; index < longBowArrayList_Length(properties->list); index++) {
        _Property *property = longBowArrayList_Get(properties->list, index);
        if (strcmp(property->name, name) == 0) {
            result = property;
            break;
        }
    }
    return result;
}

const char *
longBowProperties_Get(const LongBowProperties *properties, const char *name)
{
    _Property *property = _longBowProperties_Get(properties, name);

    if (property != NULL) {
        return property->value;
    }

    return NULL;
}

bool
longBowProperties_Set(LongBowProperties *properties, const char *name, const char *value)
{
    bool result = false;

    _Property *property = _longBowProperties_Get(properties, name);
    if (property == NULL) {
        property = longBowMemory_Allocate(sizeof(_Property));
        property->name = longBowMemory_StringCopy(name);
        property->value = longBowMemory_StringCopy(value);
        longBowArrayList_Add(properties->list, property);
        result = true;
    } else {
        longBowMemory_Deallocate((void **) &property->value);
        property->value = longBowMemory_StringCopy(value);
    }
    return result;
}

bool
longBowProperties_Exists(const LongBowProperties *properties, const char *name)
{
    return (_longBowProperties_Get(properties, name) == NULL) ? false : true;
}

bool
longBowProperties_Delete(LongBowProperties *properties, const char *name)
{
    bool result = false;

    for (size_t index = 0; index < longBowArrayList_Length(properties->list); index++) {
        _Property *property = longBowArrayList_Get(properties->list, index);
        if (strcmp(property->name, name) == 0) {
            longBowArrayList_RemoveAtIndex(properties->list, index);
            result = true;
        }
    }

    return result;
}

size_t
longBowProperties_Length(const LongBowProperties *properties)
{
    return longBowArrayList_Length(properties->list);
}

char *
longBowProperties_ToString(const LongBowProperties *properties)
{
    LongBowString *string = longBowString_Create(128);

    for (size_t index = 0; index < longBowArrayList_Length(properties->list); index++) {
        _Property *property = longBowArrayList_Get(properties->list, index);
        longBowString_Format(string, "%s=%s\n", property->name, property->value);
    }

    char *result = longBowString_ToString(string);
    longBowString_Destroy(&string);

    return result;
}

void
longBowProperties_Destroy(LongBowProperties **propertiesPtr)
{
    LongBowProperties *properties = *propertiesPtr;
    longBowArrayList_Destroy(&properties->list);
    longBowMemory_Deallocate((void **) propertiesPtr);
}
