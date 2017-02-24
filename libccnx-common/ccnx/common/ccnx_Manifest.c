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
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Object.h>

#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/ccnx_ContentObject.h>

static const CCNxManifestInterface *_defaultImplementation = &CCNxManifestFacadeV1_Interface;

static CCNxManifest *
_ccnxManifest_InternalCreate(const CCNxManifestInterface *impl, const CCNxName *name)
{
    CCNxManifest *result = NULL;

    if (impl->create != NULL) {
        result = impl->create(name);

        // And set the dictionary's interface pointer to the one we just used to create this.
        ccnxTlvDictionary_SetMessageInterface(result, impl);
    } else {
        trapNotImplemented("Manifest implementations must implement create()");
    }

    return result;
}

parcObject_ImplementAcquire(ccnxManifest, CCNxManifest);
parcObject_ImplementRelease(ccnxManifest, CCNxManifest);

CCNxManifest *
ccnxManifest_Create(const CCNxName *name)
{
    return _ccnxManifest_InternalCreate(_defaultImplementation, name);
}

CCNxManifest *
ccnxManifest_CreateNameless(void)
{
    return _ccnxManifest_InternalCreate(_defaultImplementation, NULL);
}

void
ccnxManifest_AddHashGroup(CCNxManifest *manifest, const CCNxManifestHashGroup *group)
{
    CCNxManifestInterface *interface = ccnxManifestInterface_GetInterface(manifest);
    interface->addHashGroup(manifest, group);
}

CCNxManifestHashGroup *
ccnxManifest_GetHashGroupByIndex(const CCNxManifest *manifest, size_t index)
{
    CCNxManifestInterface *interface = ccnxManifestInterface_GetInterface(manifest);
    return interface->getHashGroup(manifest, index);
}

size_t
ccnxManifest_GetNumberOfHashGroups(const CCNxManifest *manifest)
{
    CCNxManifestInterface *impl = ccnxManifestInterface_GetInterface(manifest);
    size_t result = 0;
    if (impl->getNumberOfHashGroups != NULL) {
        result = (impl->getNumberOfHashGroups)(manifest);
    }
    return result;
}

const CCNxName *
ccnxManifest_GetName(const CCNxManifest *manifest)
{
    CCNxManifestInterface *impl = ccnxManifestInterface_GetInterface(manifest);

    const CCNxName *result = NULL;

    if (impl->getName != NULL) {
        result = impl->getName(manifest);
    } else {
        trapNotImplemented("ccnxManifest_GetName");
    }

    return result;
}

bool
ccnxManifest_Equals(const CCNxManifest *objectA, const CCNxManifest *objectB)
{
    if (objectA == objectB) {
        return true;
    }
    if (objectA == NULL || objectB == NULL) {
        return false;
    }

    CCNxManifestInterface *impl = ccnxManifestInterface_GetInterface(objectA);
    return (impl->equals)(objectA, objectB);

    return false;
}

PARCJSON *
ccnxManifest_ToJSON(const CCNxManifest *manifest)
{
    PARCJSON *root = parcJSON_Create();

    char *nameString = ccnxName_ToString(ccnxManifest_GetName(manifest));
    parcJSON_AddString(root, "locator", nameString);
    parcMemory_Deallocate(&nameString);

    PARCJSONArray *array = parcJSONArray_Create();
    for (size_t i = 0; i < ccnxManifest_GetNumberOfHashGroups(manifest); i++) {
        CCNxManifestHashGroup *group = ccnxManifest_GetHashGroupByIndex(manifest, i);
        PARCJSON *groupJson = ccnxManifestHashGroup_ToJson(group);
        PARCJSONValue *jsonValue = parcJSONValue_CreateFromJSON(groupJson);

        parcJSONArray_AddValue(array, jsonValue);

        parcJSONValue_Release(&jsonValue);
        parcJSON_Release(&groupJson);
        ccnxManifestHashGroup_Release(&group);
    }
    parcJSON_AddArray(root, "HashGroups", array);
    parcJSONArray_Release(&array);

    return root;
}

char *
ccnxManifest_ToString(const CCNxManifest *manifest)
{
    PARCJSON *json = ccnxManifest_ToJSON(manifest);
    char *string = parcJSON_ToString(json);
    parcJSON_Release(&json);
    return string;
}

void
ccnxManifest_AssertValid(const CCNxManifest *manifest)
{
    assertNotNull(manifest, "Non-NULL manifest");
}

PARCLinkedList *
ccnxManifest_CreateInterestList(const CCNxManifest *manifest, const CCNxName *locator)
{
    PARCLinkedList *interests = parcLinkedList_Create();

    for (size_t i = 0; i < ccnxManifest_GetNumberOfHashGroups(manifest); i++) {
        CCNxManifestHashGroup *group = ccnxManifest_GetHashGroupByIndex(manifest, i);

        const CCNxName *name = ccnxManifest_GetName(manifest) == NULL ? locator : ccnxManifest_GetName(manifest);
        PARCLinkedList *groupInterests = ccnxManifestHashGroup_CreateInterestList(group, name);
        parcLinkedList_AppendAll(interests, groupInterests);
        parcLinkedList_Release(&groupInterests);

        ccnxManifestHashGroup_Release(&group);
    }

    return interests;
}
