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
 * @brief A structure of functions representing a Manifest implementation.
 *
 * The underlying implementation should support the CCNxManifest API.
 *
 */

#ifndef CCNx_Common_ccnx_internal_ManifestImpl_h
#define CCNx_Common_ccnx_internal_ManifestImpl_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/ccnx_ManifestHashGroup.h>
#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_KeyLocator.h>

#include "ccnx_TlvDictionary.h"

typedef struct ccnx_manifest_interface {
    /** A human-readable label for this implementation */
    char                 *description;

    /** @see ccnxManifest_Create */
    CCNxTlvDictionary *(*create)(const CCNxName * name);

    /** @see ccnxManifest_AddHashGroup */
    void (*addHashGroup)(CCNxTlvDictionary *dict, const CCNxManifestHashGroup *group);

    /** @see ccnxManifest_GetHashGroup */
    CCNxManifestHashGroup *(*getHashGroup)(const CCNxTlvDictionary * dict, size_t index);

    /** @see ccnxManifest_GetNumberOfHashGroups */
    size_t (*getNumberOfHashGroups)(const CCNxTlvDictionary *dict);

    /** @see ccnxManifest_Equals */
    bool (*equals)(const CCNxTlvDictionary *objectA, const CCNxTlvDictionary *objectB);

    /** @see ccnxManifest_GetName */
    const CCNxName *(*getName)(const CCNxTlvDictionary * dict);
} CCNxManifestInterface;

CCNxManifestInterface *ccnxManifestInterface_GetInterface(const CCNxTlvDictionary *dictionary);

/**
 * The SchemaV1 Manifest implementaton
 */
extern CCNxManifestInterface CCNxManifestFacadeV1_Interface;

#endif
