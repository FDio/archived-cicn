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
#include <stdlib.h>
#include <sys/time.h>

#include <ccnx/api/control/cpi_ForwardingStrategy.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <parc/algol/parc_Memory.h>

#include <limits.h>

#include <LongBow/runtime.h>

static const char *cpiPrefix = "PREFIX";
static const char *cpiStrategy = "STRATEGY";

struct cpi_forwarding_strategy {
    CCNxName *prefix;
    char *strategy;
};

void
cpiForwardingStrategy_Destroy(CPIForwardingStrategy **fwdStrategyPtr)
{
    assertNotNull(fwdStrategyPtr, "Parameter must be non-null double pointer");
    assertNotNull(*fwdStrategyPtr, "Parameter must dereference to non-null pointer");
    CPIForwardingStrategy *fwdStrategy = *fwdStrategyPtr;

    ccnxName_Release(&fwdStrategy->prefix);
    parcMemory_Deallocate((void **) &fwdStrategy->strategy);

    parcMemory_Deallocate((void **) &fwdStrategy);
    *fwdStrategyPtr = NULL;
}

CPIForwardingStrategy *
cpiForwardingStrategy_Create(CCNxName *prefix, char *strategy)
{
    CPIForwardingStrategy *fwdStrategy = parcMemory_AllocateAndClear(sizeof(fwdStrategy));
    assertNotNull(fwdStrategy, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(fwdStrategy));

    fwdStrategy->prefix = prefix;
    fwdStrategy->strategy = parcMemory_StringDuplicate(strategy, strlen(strategy));

    return fwdStrategy;
}

char *
cpiForwardingStrategy_ToString(CPIForwardingStrategy *fwdStrategy)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    char *ccnxName = ccnxName_ToString(cpiForwardingStrategy_GetPrefix(fwdStrategy));
    parcBufferComposer_PutString(composer, ccnxName);
    parcMemory_Deallocate((void **) &ccnxName);

    parcBufferComposer_PutString(composer, cpiForwardingStrategy_GetStrategy(fwdStrategy));

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    parcBufferComposer_Release(&composer);
    return result;
}


CPIForwardingStrategy *
cpiForwardingStrategy_Copy(const CPIForwardingStrategy *original)
{
    assertNotNull(original, "Parameter a must be non-null");
    CPIForwardingStrategy *copy = cpiForwardingStrategy_Create(ccnxName_Copy(original->prefix),
                                                               parcMemory_StringDuplicate(original->strategy, strlen(original->strategy)));

    return copy;
}



bool
cpiForwardingStrategy_Equals(const CPIForwardingStrategy *a, const CPIForwardingStrategy *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");
    if (a == b) {
        return true;
    }

    if (ccnxName_Equals(a->prefix, b->prefix) && (strcmp(a->strategy, b->strategy) == 0)) {
        return true;
    }

    return false;
}

const CCNxName *
cpiForwardingStrategy_GetPrefix(const CPIForwardingStrategy *fwdStrategy)
{
    assertNotNull(fwdStrategy, "Parameter must be non-null");
    return fwdStrategy->prefix;
}

const char *
cpiForwardingStrategy_GetStrategy(const CPIForwardingStrategy *fwdStrategy)
{
    assertNotNull(fwdStrategy, "Parameter must be non-null");
    return fwdStrategy->strategy;
}

PARCJSON *
cpiForwardingStrategy_ToJson(const CPIForwardingStrategy *fwdStrategy)
{
    assertNotNull(fwdStrategy, "Parameter must be non-null");

    PARCJSON *fwdStrategyJson = parcJSON_Create();
    char *uri = ccnxName_ToString(fwdStrategy->prefix);
    parcJSON_AddString(fwdStrategyJson, cpiPrefix, uri);
    parcMemory_Deallocate((void **) &uri);

    parcJSON_AddString(fwdStrategyJson, cpiStrategy, fwdStrategy->strategy);

    return fwdStrategyJson;
}

CPIForwardingStrategy *
cpiForwardingStrategy_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter json must be non-null");
    PARCJSON *fwdStrategyJson = json;

    PARCJSONValue *value = parcJSON_GetValueByName(fwdStrategyJson, cpiPrefix);
    assertNotNull(value, "Couldn't locate tag %s in: %s", cpiPrefix, parcJSON_ToString(json));
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    CCNxName *prefix = ccnxName_CreateFromCString(parcBuffer_Overlay(sBuf, 0));

    value = parcJSON_GetValueByName(fwdStrategyJson, cpiStrategy);
    assertNotNull(value, "Couldn't locate tag %s in: %s", cpiStrategy, parcJSON_ToString(json));
    sBuf = parcJSONValue_GetString(value);
    char *strategy = parcBuffer_Overlay(sBuf, 0);

    CPIForwardingStrategy *fwdStrategy = cpiForwardingStrategy_Create(prefix, strategy);

    return fwdStrategy;
}
