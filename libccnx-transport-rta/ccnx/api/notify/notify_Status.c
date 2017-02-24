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
#include <string.h>

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_JSON.h>

#include <ccnx/api/notify/notify_Status.h>

// These string constants are used in the JSON configuration

static const char jsonNotifyStatus[] = "notifyStatus";
static const char param_CONNECTION[] = "connectionId";
static const char param_CODE[] = "statusCode";
static const char param_NAME[] = "name";
static const char param_MESSAGE[] = "message";

struct notify_status {
    int apiFd;
    NotifyStatusCode code;
    CCNxName *name;
    char *message;
};

static void
_notifyStatus_Destroy(NotifyStatus **notifyStatusPtr)
{
    NotifyStatus *status = *notifyStatusPtr;
    if (status->name) {
        ccnxName_Release(&status->name);
    }
    if (status->message) {
        parcMemory_Deallocate((void **) &status->message);
    }
}

parcObject_ExtendPARCObject(NotifyStatus, _notifyStatus_Destroy, NULL, NULL, notifyStatus_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(notifyStatus, NotifyStatus);

parcObject_ImplementRelease(notifyStatus, NotifyStatus);

NotifyStatus *
notifyStatus_Create(int apiFd, NotifyStatusCode code, CCNxName *name, const char *message)
{
    NotifyStatus *result = parcObject_CreateInstance(NotifyStatus);
    result->apiFd = apiFd;
    result->code = code;
    result->name = name == NULL ? NULL : ccnxName_Acquire(name);
    result->message = message == NULL ? NULL : parcMemory_StringDuplicate(message, strlen(message));

    return result;
}

static bool
_StringEquals(const char *x, const char *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }
    return strcmp(x, y) == 0;
}

bool
notifyStatus_Equals(const NotifyStatus *x, const NotifyStatus *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->apiFd == y->apiFd) {
        if (x->code == y->code) {
            if (_StringEquals(x->message, y->message)) {
                if (ccnxName_Equals(x->name, y->name)) {
                    return true;
                }
            }
        }
    }
    return false;
}

int
notifyStatus_GetFiledes(const NotifyStatus *status)
{
    return status->apiFd;
}

NotifyStatusCode
notifyStatus_GetStatusCode(const NotifyStatus *status)
{
    return status->code;
}

CCNxName *
notifyStatus_GetName(const NotifyStatus *status)
{
    return status->name;
}

char *
notifyStatus_GetMessage(const NotifyStatus *status)
{
    return status->message;
}

void
notifyStatus_Display(const NotifyStatus *status, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "NotifyStatus%p { .apiFd=%d, .code=%d ", status, status->apiFd, status->code);
    ccnxName_Display(status->name, indentation + 1);
    parcDisplayIndented_PrintLine(indentation, ".message=\"%s\"\n", status->message);
}

bool
notifyStatus_IsConnectionOpen(const NotifyStatus *status)
{
    return status->code == notifyStatusCode_CONNECTION_OPEN;
}

bool
notifyStatus_IsFlowControlStarted(const NotifyStatus *status)
{
    return status->code == notifyStatusCode_FLOW_CONTROL_STARTED;
}

NotifyStatus *
notifyStatus_ParseJSON(const PARCJSON *json)
{
    NotifyStatus *result = NULL;

    PARCJSONValue *status = parcJSON_GetValueByName(json, jsonNotifyStatus);
    if (status != NULL) {
        PARCJSON *status_json = parcJSONValue_GetJSON(status);
        int apiFd = (int) parcJSONValue_GetInteger(parcJSON_GetValueByName(status_json, param_CONNECTION));
        NotifyStatusCode code =
            (NotifyStatusCode) parcJSONValue_GetInteger(parcJSON_GetValueByName(status_json, param_CODE));

        CCNxName *name = NULL;

        PARCJSONValue *nameValue = parcJSON_GetValueByName(status_json, param_NAME);
        if (nameValue != NULL) {
            PARCBuffer *sBuf = parcJSONValue_GetString(nameValue);
            const char *p = parcBuffer_Overlay(sBuf, 0);
            if (p != NULL) {
                name = ccnxName_CreateFromCString(p);
            }
        }

        const char *message = NULL;
        PARCJSONValue *messageValue = parcJSON_GetValueByName(status_json, param_MESSAGE);
        if (messageValue != NULL) {
            PARCBuffer *sBuf = parcJSONValue_GetString(messageValue);
            message = parcBuffer_Overlay(sBuf, 0);
        }

        result = notifyStatus_Create(apiFd, code, name, message);
        if (name != NULL) {
            ccnxName_Release(&name);
        }
    }
    return result;
}

PARCJSON *
notifyStatus_ToJSON(const NotifyStatus *status)
{
    PARCJSON *json = parcJSON_Create();

    parcJSON_AddInteger(json, param_CONNECTION, notifyStatus_GetFiledes(status));
    parcJSON_AddInteger(json, param_CODE, notifyStatus_GetStatusCode(status));

    if (notifyStatus_GetName(status) != NULL) {
        char *nameAsString = ccnxName_ToString(notifyStatus_GetName(status));
        parcJSON_AddString(json, param_NAME, nameAsString);
        parcMemory_Deallocate((void **) &nameAsString);
    }
    if (notifyStatus_GetMessage(status) != NULL) {
        parcJSON_AddString(json, param_MESSAGE, notifyStatus_GetMessage(status));
    }

    PARCJSON *result = parcJSON_Create();
    parcJSON_AddObject(result, jsonNotifyStatus, json);
    parcJSON_Release(&json);

    return result;
}
