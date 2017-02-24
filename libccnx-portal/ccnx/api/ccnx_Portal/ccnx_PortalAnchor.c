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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Time.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalAnchor.h>

struct CCNxPortalAnchor {
    CCNxName *prefix;
    time_t expireTime;
};

static bool
_ccnxPortalAnchor_Destructor(CCNxPortalAnchor **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a CCNxPortalAnchor pointer.");
    CCNxPortalAnchor *instance = *instancePtr;

    ccnxName_Release(&instance->prefix);

    /* cleanup the instance fields here */
    return true;
}

parcObject_ImplementAcquire(ccnxPortalAnchor, CCNxPortalAnchor);

parcObject_ImplementRelease(ccnxPortalAnchor, CCNxPortalAnchor);

#define parcObject_OverrideCopy(_name_) .copy = (PARCObjectCopy *) _name_
parcObject_Override(CCNxPortalAnchor, PARCObject,
                    parcObject_OverrideCopy(ccnxPortalAnchor_Copy),
                    .toString = (PARCObjectToString *) ccnxPortalAnchor_ToString,
                    .equals = (PARCObjectEquals *) ccnxPortalAnchor_Equals,
                    .compare = (PARCObjectCompare *) ccnxPortalAnchor_Compare,
                    .hashCode = (PARCObjectHashCode *) ccnxPortalAnchor_HashCode,
                    .toJSON = (PARCObjectToJSON *) ccnxPortalAnchor_ToJSON,
                    .display = (PARCObjectDisplay *) ccnxPortalAnchor_Display,
                    .destructor = (PARCObjectDestructor *) _ccnxPortalAnchor_Destructor
                    );

void
ccnxPortalAnchor_AssertValid(const CCNxPortalAnchor *instance)
{
    assertTrue(ccnxPortalAnchor_IsValid(instance),
               "CCNxPortalAnchor is not valid.");
}


CCNxPortalAnchor *
ccnxPortalAnchor_Create(const CCNxName *name, time_t expireTime)
{
    CCNxPortalAnchor *result = parcObject_CreateInstance(CCNxPortalAnchor);

    if (result != NULL) {
        result->prefix = ccnxName_Acquire(name);
        result->expireTime = expireTime;
    }

    return result;
}

CCNxPortalAnchor *
ccnxPortalAnchor_CreateFromJSON(const PARCJSON *json)
{
    CCNxPortalAnchor *result = parcObject_CreateInstance(CCNxPortalAnchor);

    if (result != NULL) {
        result->prefix = ccnxName_CreateFromCString(parcBuffer_Overlay(parcJSONValue_GetString(parcJSON_GetByPath(json, "/namePrefix")), 0));
        result->expireTime = parcJSONValue_GetInteger(parcJSON_GetByPath(json, "/expireTime"));

        ccnxPortalAnchor_OptionalAssertValid(result);
    }

    return result;
}

int
ccnxPortalAnchor_Compare(const CCNxPortalAnchor *instance, const CCNxPortalAnchor *other)
{
    int result = 0;

    return result;
}

CCNxPortalAnchor *
ccnxPortalAnchor_Copy(const CCNxPortalAnchor *original)
{
    CCNxName *prefixCopy = ccnxName_Copy(original->prefix);
    CCNxPortalAnchor *result = ccnxPortalAnchor_Create(prefixCopy, original->expireTime);
    ccnxName_Release(&prefixCopy);

    return result;
}

void
ccnxPortalAnchor_Display(const CCNxPortalAnchor *anchor, int indentation)
{
    char *prefix = ccnxName_ToString(anchor->prefix);

    parcDisplayIndented_PrintLine(indentation, "CCNxPortalAnchor@%p {", anchor);
    parcDisplayIndented_PrintLine(indentation + 1, ".prefix=%s}", prefix);
    parcDisplayIndented_PrintLine(indentation + 1, ".expireTime=%ld", anchor->expireTime);

    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
    parcMemory_Deallocate(&prefix);
}

bool
ccnxPortalAnchor_Equals(const CCNxPortalAnchor *x, const CCNxPortalAnchor *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (ccnxName_Equals(x->prefix, y->prefix)) {
            if (x->expireTime == y->expireTime) {
                result = true;
            }
        }
    }

    return result;
}

PARCHashCode
ccnxPortalAnchor_HashCode(const CCNxPortalAnchor *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
ccnxPortalAnchor_IsValid(const CCNxPortalAnchor *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
ccnxPortalAnchor_ToJSON(const CCNxPortalAnchor *anchor)
{
    ccnxPortalAnchor_OptionalAssertValid(anchor);

    PARCJSON *result = parcJSON_Create();
    if (result != NULL) {
        char *prefix = ccnxName_ToString(anchor->prefix);

        parcJSON_AddString(result, "namePrefix", prefix);
        parcJSON_AddInteger(result, "expireTime", (int64_t) anchor->expireTime);

        parcMemory_Deallocate(&prefix);
    }

    return result;
}

PARCBufferComposer *
ccnxPortalAnchor_BuildString(const CCNxPortalAnchor *anchor, PARCBufferComposer *composer)
{
    char *name = ccnxName_ToString(anchor->prefix);

    char expireTime[64];
    if (anchor->expireTime == -1) {
        strcpy(expireTime, "                      never");
    } else {
        parcTime_TimeAsRFC3339(anchor->expireTime, expireTime);
    }
    parcBufferComposer_Format(composer, "{ %s %s }", name, expireTime);

    parcMemory_Deallocate(&name);

    return composer;
}

char *
ccnxPortalAnchor_ToString(const CCNxPortalAnchor *anchor)
{
    ccnxPortalAnchor_OptionalAssertValid(anchor);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    ccnxPortalAnchor_BuildString(anchor, composer);
    char *result = parcBufferComposer_ToString(composer);
    parcBufferComposer_Release(&composer);

    return result;
}

CCNxPortalAnchor *
ccnxPortalAnchor_Deserialize(PARCBuffer *buffer)
{
    PARCJSON *json = parcJSON_ParseBuffer(buffer);

    CCNxPortalAnchor *result = ccnxPortalAnchor_CreateFromJSON(json);
    parcJSON_Release(&json);
    return result;
}

PARCBufferComposer *
ccnxPortalAnchor_Serialize(const CCNxPortalAnchor *namePrefix, PARCBufferComposer *composer)
{
    PARCJSON *json = ccnxPortalAnchor_ToJSON(namePrefix);

    char *string = parcJSON_ToString(json);
    parcBufferComposer_PutString(composer, string);
    parcMemory_Deallocate(&string);
    parcJSON_Release(&json);
    return composer;
}

CCNxName *
ccnxPortalAnchor_GetNamePrefix(const CCNxPortalAnchor *anchor)
{
    return anchor->prefix;
}

time_t
ccnxPortalAnchor_GetExpireTime(const CCNxPortalAnchor *anchor)
{
    return anchor->expireTime;
}

time_t
ccnxPortalAnchor_SetExpireTime(CCNxPortalAnchor *anchor, const time_t expireTime)
{
    time_t previousExpireTime = anchor->expireTime;
    anchor->expireTime = expireTime;
    return previousExpireTime;
}
