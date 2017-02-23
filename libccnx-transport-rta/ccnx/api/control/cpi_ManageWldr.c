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

#include <ccnx/api/control/cpi_ManageWldr.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <parc/algol/parc_Memory.h>

#include <LongBow/runtime.h>

#include "cpi_private.h"

static const char *cpiWldrString = "WLDR";
static const char *cpiWldrConn = "CONN";
static const char *cpiWldrOn = "ON";
static const char *cpiWldrOff = "OFF";

struct cpi_manage_wldr {
    char *connectionId;
    bool active;
};

void
cpiManageWldr_Destroy(CPIManageWldr **cpiWldrPtr)
{
    assertNotNull(cpiWldrPtr, "Parameter must be non-null double pointer");
    assertNotNull(*cpiWldrPtr, "Parameter must dereference to non-null pointer");
    CPIManageWldr *cpiWldr = *cpiWldrPtr;

    parcMemory_Deallocate((void **) &cpiWldr->connectionId);
    parcMemory_Deallocate((void **) &cpiWldr);

    *cpiWldrPtr = NULL;
}

CPIManageWldr *
cpiManageWldr_Create(bool active, char *conn)
{
    CPIManageWldr *cpiWldr = parcMemory_AllocateAndClear(sizeof(cpiWldr));
    assertNotNull(cpiWldr, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(cpiWldr));

    cpiWldr->connectionId = parcMemory_StringDuplicate(conn, strlen(conn));
    cpiWldr->active = active;

    return cpiWldr;
}

char *
CPIManageWldr_ToString(CPIManageWldr *cpiWldr)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcBufferComposer_PutString(composer, cpiManageWldr_GetConnection(cpiWldr));

    parcBufferComposer_PutString(composer, cpiWldrString);

    if (cpiManageWldr_IsActive(cpiWldr)) {
        parcBufferComposer_PutString(composer, cpiWldrOn);
    } else  {
        parcBufferComposer_PutString(composer, cpiWldrOff);
    }

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);
    parcBufferComposer_Release(&composer);
    return result;
}

CPIManageWldr *
cpiManageWldr_Copy(const CPIManageWldr *original)
{
    assertNotNull(original, "Parameter a must be non-null");
    CPIManageWldr *copy = cpiManageWldr_Create(original->active,
                                               parcMemory_StringDuplicate(original->connectionId, strlen(original->connectionId)));

    return copy;
}

bool
cpiManageWldr_Equals(const CPIManageWldr *a, const CPIManageWldr *b)
{
    assertNotNull(a, "Parameter a must be non-null");
    assertNotNull(b, "Parameter b must be non-null");
    if (a == b) {
        return true;
    }

    if ((a->active == b->active) && (strcmp(a->connectionId, b->connectionId) == 0)) {
        return true;
    }

    return false;
}

bool
cpiManageWldr_IsActive(const CPIManageWldr *cpiWldr)
{
    assertNotNull(cpiWldr, "Parameter must be non-null");
    return cpiWldr->active;
}

const char *
cpiManageWldr_GetConnection(const CPIManageWldr *cpiWldr)
{
    assertNotNull(cpiWldr, "Parameter must be non-null");
    return cpiWldr->connectionId;
}


PARCJSON *
cpiManageWldr_ToJson(const CPIManageWldr *cpiWldr)
{
    PARCJSON *wldrJson = parcJSON_Create();

    parcJSON_AddString(wldrJson, cpiWldrConn, cpiWldr->connectionId);

    if (cpiWldr->active) {
        parcJSON_AddString(wldrJson, cpiWldrString, cpiWldrOn);
    } else {
        parcJSON_AddString(wldrJson, cpiWldrString, cpiWldrOff);
    }

    return wldrJson;
}
CPIManageWldr *
cpiManageWldr_FromJson(PARCJSON *json)
{
    assertNotNull(json, "Parameter json must be non-null");
    PARCJSON *cpiWldrJson = json;

    PARCJSONValue *value = parcJSON_GetValueByName(cpiWldrJson, cpiWldrConn);
    assertNotNull(value, "Couldn't locate tag %s in: %s", cpiWldrConn, parcJSON_ToString(json));

    PARCBuffer *wBuf = parcJSONValue_GetString(value);
    char *conn = parcBuffer_Overlay(wBuf, 0);

    value = parcJSON_GetValueByName(cpiWldrJson, cpiWldrString);
    assertNotNull(value, "Couldn't locate tag %s in: %s", cpiWldrString, parcJSON_ToString(json));

    wBuf = parcJSONValue_GetString(value);
    char *active = parcBuffer_Overlay(wBuf, 0);

    CPIManageWldr *cpiWldr;
    if (strcmp(active, cpiWldrOn) == 0) {
        cpiWldr = cpiManageWldr_Create(true, conn);
    } else  {
        cpiWldr = cpiManageWldr_Create(false, conn);
    }

    return cpiWldr;
}
