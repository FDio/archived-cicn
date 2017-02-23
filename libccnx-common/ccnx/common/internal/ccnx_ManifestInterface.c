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

#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/internal/ccnx_ManifestInterface.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

CCNxManifestInterface *
ccnxManifestInterface_GetInterface(const CCNxTlvDictionary *dictionary)
{
    assertTrue(ccnxTlvDictionary_IsManifest(dictionary), "Expected a Manifest");

    CCNxManifestInterface *impl = (CCNxManifestInterface *) ccnxTlvDictionary_GetMessageInterface(dictionary);

    if (impl == NULL) {
        // If we're here, we need to update the implementation pointer.
        // Figure out what the typeImplementation should be, based on the attributes we know.
        int schemaVersion = ccnxTlvDictionary_GetSchemaVersion(dictionary);

        switch (schemaVersion) {
            case CCNxTlvDictionary_SchemaVersion_V1:
                impl = &CCNxManifestFacadeV1_Interface;
                break;
            default:
                trapUnexpectedState("Unknown SchemaVersion encountered in ccnxInterestImplementation_GetImplementation()");
                break;
        }

        if (impl == NULL) {
            // The cast to (CCNxTlvDictionary *) is to break the const.
            ccnxTlvDictionary_SetMessageInterface((CCNxTlvDictionary *) dictionary, (CCNxMessageInterface *) impl);
        }
    }

    return impl;
}
