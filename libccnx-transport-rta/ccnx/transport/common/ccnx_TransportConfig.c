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

#include <config.h>
#include <stdio.h>
#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

#include <ccnx/transport/common/ccnx_TransportConfig.h>

struct transport_config {
    CCNxStackConfig *stackConfig;
    CCNxConnectionConfig *connConfig;
};

bool
ccnxTransportConfig_IsValid(const CCNxTransportConfig *transportConfig)
{
    bool result = false;

    if (transportConfig != NULL) {
        if (ccnxStackConfig_IsValid(transportConfig->stackConfig)) {
            if (ccnxConnectionConfig_IsValid(transportConfig->connConfig)) {
                result = true;
            }
        }
    }
    return result;
}

void
ccnxTransportConfig_AssertValid(const CCNxTransportConfig *config)
{
    assertTrue(ccnxTransportConfig_IsValid(config), "CCNxTransportConfig instance is invalid.");
}

CCNxTransportConfig *
ccnxTransportConfig_Create(CCNxStackConfig *stackConfig, CCNxConnectionConfig *connConfig)
{
    ccnxStackConfig_OptionalAssertValid(stackConfig);
    ccnxConnectionConfig_OptionalAssertValid(connConfig);

    CCNxTransportConfig *result = parcMemory_AllocateAndClear(sizeof(CCNxTransportConfig));
    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CCNxTransportConfig));
    result->stackConfig = ccnxStackConfig_Acquire(stackConfig);
    result->connConfig = connConfig;
    return result;
}

void
ccnxTransportConfig_Destroy(CCNxTransportConfig **transportConfigPtr)
{
    assertNotNull(transportConfigPtr, "Parameter must be non-null double pointer");
    ccnxTransportConfig_OptionalAssertValid(*transportConfigPtr);

    CCNxTransportConfig *transConfig = *transportConfigPtr;
    ccnxStackConfig_Release(&transConfig->stackConfig);
    ccnxConnectionConfig_Destroy(&transConfig->connConfig);
    parcMemory_Deallocate((void **) &transConfig);
    *transportConfigPtr = NULL;
}

CCNxStackConfig *
ccnxTransportConfig_GetStackConfig(const CCNxTransportConfig *transportConfig)
{
    ccnxTransportConfig_OptionalAssertValid(transportConfig);

    return transportConfig->stackConfig;
}

CCNxConnectionConfig *
ccnxTransportConfig_GetConnectionConfig(const CCNxTransportConfig *transportConfig)
{
    ccnxTransportConfig_OptionalAssertValid(transportConfig);

    return transportConfig->connConfig;
}

bool
ccnxTransportConfig_Equals(const CCNxTransportConfig *x, const CCNxTransportConfig *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (ccnxStackConfig_Equals(x->stackConfig, y->stackConfig)) {
            result = ccnxConnectionConfig_Equals(x->connConfig, y->connConfig);
        }
    }

    return result;
}

CCNxTransportConfig *
ccnxTransportConfig_Copy(const CCNxTransportConfig *original)
{
    ccnxTransportConfig_OptionalAssertValid(original);

    CCNxTransportConfig *copy = parcMemory_AllocateAndClear(sizeof(CCNxTransportConfig));
    assertNotNull(copy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(CCNxTransportConfig));

    copy->stackConfig = ccnxStackConfig_Copy(original->stackConfig);
    copy->connConfig = ccnxConnectionConfig_Copy(original->connConfig);
    return copy;
}
