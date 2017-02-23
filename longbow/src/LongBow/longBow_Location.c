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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <LongBow/longBow_Location.h>

#include <LongBow/private/longBow_Memory.h>

/** @cond private */
struct longbow_location {
    char *fileName;
    char *functionName;
    uint32_t lineNumber;
};
/** @endcond */

LongBowLocation *
longBowLocation_Create(const char *fileName, const char *functionName, uint32_t lineNumber)
{
    LongBowLocation *result = longBowMemory_Allocate(sizeof(LongBowLocation));

    if (result != NULL) {
        result->fileName = (fileName == NULL) ? NULL : strdup(fileName);
        result->functionName = (functionName == NULL) ? NULL : strdup(functionName);
        result->lineNumber = lineNumber;
    }

    return result;
}

void
longBowLocation_Destroy(LongBowLocation **locationPtr)
{
    assert(locationPtr != NULL);

    LongBowLocation *location = *locationPtr;

    if (location->fileName != NULL) {
        free(location->fileName);
    }
    if (location->functionName != NULL) {
        free(location->functionName);
    }
    longBowMemory_Deallocate((void **) locationPtr);
}

char *
longBowLocation_ToString(const LongBowLocation *location)
{
    assert(location != NULL);

    char *result;

    if (location->functionName == 0) {
        int status = asprintf(&result, "%s:%u", location->fileName, location->lineNumber);
        if (status == -1) {
            return NULL;
        }
    } else {
        int status = asprintf(&result, "%s:%u %s()", location->fileName, location->lineNumber, location->functionName);
        if (status == -1) {
            return NULL;
        }
    }

    return result;
}
