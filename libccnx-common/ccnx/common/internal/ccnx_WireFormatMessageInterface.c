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

#include <ccnx/common/internal/ccnx_WireFormatMessageInterface.h>


CCNxWireFormatMessageInterface *
ccnxWireFormatMessageInterface_GetInterface(const CCNxTlvDictionary *dictionary)
{
    CCNxWireFormatMessageInterface *impl = NULL;

    int schemaVersion = ccnxTlvDictionary_GetSchemaVersion(dictionary);

    switch (schemaVersion) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            impl = &CCNxWireFormatFacadeV1_Implementation;
            break;
        default:
            trapUnexpectedState("Unknown SchemaVersion encountered in ccnxWireFormatMessageInterface_GetInterface()");
            break;
    }

    // We do not set the implementation pointer in the dictionary here. That is only done when accessing the dictionary
    // as a CCNxContentObject, CCNxInterest, CCNxInterestReturn, or CCNxControlMessage.

    return impl;
}
