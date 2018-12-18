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

#include <LongBow/runtime.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <parc/algol/parc_Deque.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_BufferComposer.h>

#include <parc/algol/parc_PathName.h>

struct parc_pathname {
    bool isAbsolute;
    PARCDeque *path;
};

static void
_destroy(PARCPathName **pathNamePtr)
{
    PARCPathName *pathName = *pathNamePtr;

    for (size_t i = 0; i < parcDeque_Size(pathName->path); i++) {
        void *name = parcDeque_GetAtIndex(pathName->path, i);
        parcMemory_Deallocate((void **) &name);
    }
    parcDeque_Release(&pathName->path);
}

static bool
_pathNameSegmentEquals(const void *x, const void *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }
    return strcmp((char *) x, (char *) y) == 0;
}

static void *
_pathNameSegmentCopy(const void *x)
{
    return parcMemory_StringDuplicate(x, strlen(x));
}

static const PARCObjectDescriptor parcPathNameSegment_ObjectInterface = {
    .destroy  = (PARCObjectDestroy *) NULL,
    .copy     = (PARCObjectCopy *) _pathNameSegmentCopy,
    .toString = (PARCObjectToString *) NULL,
    .equals   = (PARCObjectEquals *) _pathNameSegmentEquals,
    .compare  = (PARCObjectCompare *) NULL
};

parcObject_ExtendPARCObject(PARCPathName, _destroy, parcPathName_Copy, parcPathName_ToString,
                            parcPathName_Equals, NULL, NULL, NULL);

PARCPathName *
parcPathName_ParseToLimit(size_t limit, const char path[limit])
{
    PARCPathName *result = parcPathName_Create();

    if (limit > 0) {
        size_t index = 0;

        if (path[index] == '/') {
            result->isAbsolute = true;
            index++;
        }
        while (path[index] != 0 && index < limit) {
            while (path[index] == '/' && index < limit) {
                index++;
            }
            if (path[index] != 0 && index < limit) {
                size_t segment = index;
                while (path[index] != 0 && path[index] != '/' && index < limit) {
                    index++;
                }

                parcDeque_Append(result->path, parcMemory_StringDuplicate(&path[segment], index - segment));
            }
        }
    }

    return result;
}

PARCPathName *
parcPathName_Parse(const char *path)
{
    return parcPathName_ParseToLimit(strlen(path), path);
}

PARCPathName *
parcPathName_Create(void)
{
    PARCPathName *result = parcObject_CreateInstance(PARCPathName);

    result->isAbsolute = false;
    result->path = parcDeque_CreateObjectInterface(&parcPathNameSegment_ObjectInterface);
    return result;
}

parcObject_ImplementAcquire(parcPathName, PARCPathName);

parcObject_ImplementRelease(parcPathName, PARCPathName);

PARCPathName *
parcPathName_Copy(const PARCPathName *pathName)
{
    PARCPathName *result = parcObject_CreateInstance(PARCPathName);

    result->isAbsolute = pathName->isAbsolute;
    result->path = parcDeque_Copy(pathName->path);
    return result;
}

bool
parcPathName_Equals(const PARCPathName *x, const PARCPathName *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->isAbsolute == y->isAbsolute) {
        return parcDeque_Equals(x->path, y->path);
    }
    return false;
}

bool
parcPathName_IsAbsolute(const PARCPathName *pathName)
{
    return pathName->isAbsolute;
}

bool
parcPathName_MakeAbsolute(PARCPathName *pathName, bool absolute)
{
    bool result = parcPathName_IsAbsolute(pathName);
    pathName->isAbsolute = absolute;

    return result;
}

PARCPathName *
parcPathName_Prepend(PARCPathName *pathName, const char *name)
{
    parcDeque_Prepend(pathName->path, parcMemory_StringDuplicate(name, strlen(name)));
    return pathName;
}

PARCPathName *
parcPathName_Append(PARCPathName *pathName, const char *name)
{
    parcDeque_Append(pathName->path, parcMemory_StringDuplicate(name, strlen(name)));
    return pathName;
}

char *
parcPathName_GetAtIndex(const PARCPathName *pathName, size_t index)
{
    return (char *) parcDeque_GetAtIndex(pathName->path, index);
}

PARCPathName *
parcPathName_Head(const PARCPathName *pathName, size_t size)
{
    PARCPathName *result = parcPathName_Create();
    size_t maximum = parcPathName_Size(pathName) < size ? parcPathName_Size(pathName) : size;

    for (size_t i = 0; i < maximum; i++) {
        parcPathName_Append(result, parcPathName_GetAtIndex(pathName, i));
    }

    parcPathName_MakeAbsolute(result, parcPathName_IsAbsolute(pathName));

    return result;
}

PARCPathName *
parcPathName_Tail(const PARCPathName *pathName, size_t size)
{
    PARCPathName *result = parcPathName_Create();
    if (size > parcPathName_Size(pathName)) {
        size = parcPathName_Size(pathName);
    }

    for (size_t i = parcPathName_Size(pathName) - size; i < parcPathName_Size(pathName); i++) {
        parcPathName_Prepend(result, parcPathName_GetAtIndex(pathName, i));
    }

    parcPathName_MakeAbsolute(result, false);

    return result;
}

size_t
parcPathName_Size(const PARCPathName *pathName)
{
    return parcDeque_Size(pathName->path);
}

PARCBufferComposer *
parcPathName_BuildString(const PARCPathName *pathName, PARCBufferComposer *composer)
{
    char *separator = "/";

    //    an absolute path with no segments should just be '/'
    if (parcPathName_IsAbsolute(pathName)) {
        parcBufferComposer_PutString(composer, separator);
    }

    size_t length = parcDeque_Size(pathName->path);
    if (length > 0) {
        parcBufferComposer_PutString(composer, parcDeque_GetAtIndex(pathName->path, 0));
        for (size_t i = 1; i < length; i++) {
            parcBufferComposer_PutStrings(composer, separator, parcDeque_GetAtIndex(pathName->path, i), NULL);
        }
    }

    return composer;
}

char *
parcPathName_ToString(const PARCPathName *pathName)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcPathName_BuildString(pathName, composer);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    parcBufferComposer_Release(&composer);

    return result;
}

void
parcPathName_Display(const PARCPathName *pathName, int indentation)
{
    if (pathName == NULL) {
        parcDisplayIndented_PrintLine(indentation, "PARCPathName@NULL\n");
    } else {
        parcDisplayIndented_PrintLine(indentation, "PARCPathName@%p {\n", (void *) pathName);
        parcDeque_Display(pathName->path, indentation + 1);
        parcDisplayIndented_PrintLine(indentation, "}\n");
    }
}
