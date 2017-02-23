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

#include <LongBow/runtime.h>

#include <parc/algol/parc_JSON.h>

#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/internal/ccnx_ManifestInterface.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

static size_t _ccnxManifestFacadeV1_GetNumberOfHashGroups(const CCNxTlvDictionary *dict);

static CCNxTlvDictionary *
_ccnxManifestFacadeV1_Create(const CCNxName *name)
{
    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateManifest();

    if (dictionary != NULL) {
        if (name != NULL) {
            ccnxTlvDictionary_PutName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);
        }
    } else {
        trapOutOfMemory("Could not allocate an Manifest");
    }

    return dictionary;
}

static const CCNxName *
_ccnxManifestFacadeV1_GetName(const CCNxTlvDictionary *dict)
{
    return ccnxTlvDictionary_GetName(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
}

static void
_ccnxManifestFacadeV1_AddHashGroup(CCNxTlvDictionary *dict, const CCNxManifestHashGroup *group)
{
    PARCJSON *json = ccnxManifestHashGroup_ToJson(group);
    size_t numGroups = _ccnxManifestFacadeV1_GetNumberOfHashGroups(dict);

    char *jsonString = parcJSON_ToString(json);
    PARCBuffer *buffer = parcBuffer_AllocateCString(jsonString);
    parcMemory_Deallocate(&jsonString);
    parcJSON_Release(&json);

    ccnxTlvDictionary_PutListBuffer(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST, (uint32_t) numGroups, buffer);
    parcBuffer_Release(&buffer);
}

static CCNxManifestHashGroup *
_ccnxManifestFacadeV1_GetHashGroup(const CCNxTlvDictionary *dict, size_t index)
{
    PARCBuffer *buffer = NULL;
    uint32_t key;
    ccnxTlvDictionary_ListGetByPosition(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST, index, &buffer, &key);

    char *jsonString = parcBuffer_ToString(buffer);
    PARCJSON *json = parcJSON_ParseString(jsonString);
    parcMemory_Deallocate(&jsonString);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_CreateFromJson(json);
    parcJSON_Release(&json);

    return group;
}

static size_t
_ccnxManifestFacadeV1_GetNumberOfHashGroups(const CCNxTlvDictionary *dict)
{
    size_t numHashGroups = ccnxTlvDictionary_ListSize(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST);
    return numHashGroups;
}

static bool
_ccnxManifestFacadeV1_Equals(const CCNxTlvDictionary *dictA, const CCNxTlvDictionary *dictB)
{
    if (dictA == dictB) {
        return true;
    }
    if (dictA == NULL || dictB == NULL) {
        return false;
    }

    if (ccnxName_Equals(_ccnxManifestFacadeV1_GetName(dictA), _ccnxManifestFacadeV1_GetName(dictB))) {
        if (_ccnxManifestFacadeV1_GetNumberOfHashGroups(dictA) == _ccnxManifestFacadeV1_GetNumberOfHashGroups(dictB)) {
            for (size_t i = 0; i < _ccnxManifestFacadeV1_GetNumberOfHashGroups(dictA); i++) {
                CCNxManifestHashGroup *groupA = _ccnxManifestFacadeV1_GetHashGroup(dictA, i);
                CCNxManifestHashGroup *groupB = _ccnxManifestFacadeV1_GetHashGroup(dictB, i);

                if (!ccnxManifestHashGroup_Equals(groupA, groupB)) {
                    ccnxManifestHashGroup_Release(&groupA);
                    ccnxManifestHashGroup_Release(&groupB);
                    return false;
                }
            }
            return true;
        }
    }
    return false;
};

CCNxManifestInterface CCNxManifestFacadeV1_Interface = {
    .description           = "CCNxManifestFacadeV1_Implementation",
    .create                = &_ccnxManifestFacadeV1_Create,
    .getName               = &_ccnxManifestFacadeV1_GetName,
    .addHashGroup          = &_ccnxManifestFacadeV1_AddHashGroup,
    .getHashGroup          = &_ccnxManifestFacadeV1_GetHashGroup,
    .getNumberOfHashGroups = &_ccnxManifestFacadeV1_GetNumberOfHashGroups,
    .equals                = &_ccnxManifestFacadeV1_Equals,
};
