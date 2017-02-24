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
#include <ccnx/common/internal/ccnx_InterestReturnInterface.h>

#include <LongBow/runtime.h>

CCNxInterestReturnInterface *
ccnxInterestReturnInterface_GetInterface(const CCNxTlvDictionary *dictionary)
{
    assertTrue(ccnxTlvDictionary_IsInterestReturn(dictionary), "Expected an InterestReturn");

    CCNxInterestReturnInterface *impl = ccnxTlvDictionary_GetMessageInterface(dictionary);

    if (!impl) {
        // If we're here, we need to update the interface pointer. Break the const.
        // We're not changing data values, just initializing the Interface pointer.

        // Figure out what the typeInterface should be, based on the attributes we know.
        int schemaVersion = ccnxTlvDictionary_GetSchemaVersion(dictionary);

        switch (schemaVersion) {
            case CCNxTlvDictionary_SchemaVersion_V0:
                trapUnexpectedState("ccnxInterestReturnInterface_GetInterface() not implemented for V0");
            case CCNxTlvDictionary_SchemaVersion_V1:
                impl = &CCNxInterestReturnFacadeV1_Implementation;
                break;
            default:
                trapUnexpectedState("Unknown SchemaVersion encountered in ccnxInterestReturnInterface_GetInterface()");
                break;
        }

        if (impl) {
            ccnxTlvDictionary_SetMessageInterface((CCNxTlvDictionary *) dictionary, impl); // break the const.
        }
    }

    return impl;
}

