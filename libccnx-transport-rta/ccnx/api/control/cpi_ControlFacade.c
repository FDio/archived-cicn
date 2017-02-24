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
#include <stdlib.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/api/control/cpi_ControlFacade.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

#include <ccnx/common/ccnx_Name.h>

static const char _NotificationIndicator[] = "notificationWrapper";
static const char _NotificationPayload[] = "notificationPayload";


// ===========================================================================================================


CCNxControl *
ccnxControlFacade_CreateCPI(PARCJSON *ccnx_json)
{
    assertNotNull(ccnx_json, "Parameter ccnx_json must be non-null");

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateControl();

    ccnxTlvDictionary_PutJson(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, ccnx_json);

    return dictionary;
}

CCNxControl *
ccnxControlFacade_CreateNotification(PARCJSON *payload)
{
    assertNotNull(payload, "Parameter ccnx_json must be non-null");

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateControl();

    // Create a new JSON object that indicates that this is a notification. Wrap it around
    // the supplied JSON object.

    PARCJSON *notificationWrapper = parcJSON_Create();
    parcJSON_AddBoolean(notificationWrapper, _NotificationIndicator, true);
    parcJSON_AddObject(notificationWrapper, _NotificationPayload, payload);
    ccnxTlvDictionary_PutJson(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD, notificationWrapper);
    parcJSON_Release(&notificationWrapper);

    return dictionary;
}

PARCJSON *
ccnxControlFacade_GetJson(const CCNxTlvDictionary *controlDictionary)
{
    ccnxControlFacade_AssertValid(controlDictionary);
    PARCJSON *controlJSON = ccnxTlvDictionary_GetJson(controlDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);

    if (ccnxControlFacade_IsNotification(controlDictionary)) {
        PARCJSONValue *wrappedJSON = parcJSON_GetValueByName(controlJSON, _NotificationPayload);
        controlJSON = parcJSONValue_GetJSON(wrappedJSON);
    }

    return controlJSON;
}

bool
ccnxControlFacade_IsCPI(const CCNxTlvDictionary *controlDictionary)
{
    bool result = false;
    ccnxControlFacade_AssertValid(controlDictionary);

    result = ccnxTlvDictionary_IsControl(controlDictionary);

    PARCJSON *controlJSON = ccnxTlvDictionary_GetJson(controlDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
    if (controlJSON != NULL) {
        if (parcJSON_GetValueByName(controlJSON, _NotificationIndicator) != NULL) {
            // this is a notification
            result = false;
        }
    }
    return result;
}

bool
ccnxControlFacade_IsNotification(const CCNxTlvDictionary *controlDictionary)
{
    bool result = false;

    ccnxControlFacade_AssertValid(controlDictionary);

    PARCJSON *controlJSON = ccnxTlvDictionary_GetJson(controlDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD);
    if (controlJSON != NULL && (parcJSON_GetValueByName(controlJSON, _NotificationIndicator) != NULL)) {
        result = true;
    }
    return result;
}

void
ccnxControlFacade_Display(const CCNxTlvDictionary *contentDictionary, int indentation)
{
    ccnxTlvDictionary_Display(contentDictionary, indentation);
}

char *
ccnxControlFacade_ToString(const CCNxTlvDictionary *contentDictionary)
{
    char *string;
    char *jsonString = NULL;

    PARCJSON *json = ccnxControlFacade_GetJson(contentDictionary);
    if (json != NULL) {
        jsonString = parcJSON_ToString(json);
    }

    int failure = asprintf(&string, "CCNxControl { isCPI=%s, isNotification=%s, JSON=\"%s\"}",
                           ccnxControlFacade_IsCPI(contentDictionary) ? "true" : "false",
                           ccnxControlFacade_IsNotification(contentDictionary) ? "true" : "false",
                           jsonString != NULL ? jsonString : "NULL");


    if (jsonString) {
        parcMemory_Deallocate((void **) &jsonString);
    }

    assertTrue(failure > -1, "Error asprintf");

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);

    return result;
}

void
ccnxControlFacade_AssertValid(const CCNxTlvDictionary *controlDictionary)
{
    assertNotNull(controlDictionary, "Parameter must be a non-null CCNxControlFacade pointer");


    assertTrue(ccnxTlvDictionary_IsValueJson(controlDictionary,
                                             CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD), "Does not have JSON payload");
    assertTrue(ccnxTlvDictionary_IsControl(controlDictionary), "Does not have type set");
}
