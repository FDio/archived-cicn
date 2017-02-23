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

#include <pthread.h>

#include <LongBow/runtime.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalAnchor.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <ccnx/api/control/controlPlaneInterface.h>

#include <ccnx/transport/common/transport.h>

struct ccnx_portal_status {
    int error;
    bool eof;
};

struct ccnx_portal {
    CCNxPortalStatus status;

    const CCNxPortalStack *stack;
};

static CCNxMetaMessage *
_ccnxPortal_ComposeAnchorMessage(const CCNxName *routerName, const CCNxName *name, int secondsToLive)
{
    time_t expireTime = time(0) + secondsToLive;
    CCNxPortalAnchor *namePrefix = ccnxPortalAnchor_Create(name, expireTime);

    PARCBufferComposer *composer = parcBufferComposer_Create();
    ccnxPortalAnchor_Serialize(namePrefix, composer);
    PARCBuffer *payload = parcBufferComposer_ProduceBuffer(composer);

    CCNxInterest *interest = ccnxInterest_CreateSimple(routerName);
    ccnxInterest_SetPayload(interest, payload);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);

    parcBuffer_Release(&payload);
    ccnxInterest_Release(&interest);
    parcBufferComposer_Release(&composer);
    ccnxPortalAnchor_Release(&namePrefix);

    return message;
}

static int
_ccnxPortal_SetAnchor(CCNxPortal *portal, const CCNxName *name, time_t secondsToLive)
{
    int64_t timeOutMicroSeconds = parcProperties_GetAsInteger(ccnxPortalStack_GetProperties(portal->stack), CCNxPortalFactory_LocalRouterTimeout, 1000000);

    CCNxName *routerName = ccnxName_CreateFromCString(ccnxPortalStack_GetProperty(portal->stack, CCNxPortalFactory_LocalRouterName, "lci:/local/dcr"));
    CCNxName *fullName = ccnxName_ComposeNAME(routerName, "anchor");

    CCNxMetaMessage *message = _ccnxPortal_ComposeAnchorMessage(fullName, name, secondsToLive);

    ccnxPortal_Send(portal, message, CCNxStackTimeout_MicroSeconds(timeOutMicroSeconds));
    ccnxMetaMessage_Release(&message);

    CCNxMetaMessage *response = ccnxPortal_Receive(portal, CCNxStackTimeout_MicroSeconds(timeOutMicroSeconds));
    if (response != NULL) {
        ccnxMetaMessage_Release(&response);
    }

    ccnxName_Release(&fullName);
    ccnxName_Release(&routerName);

    return 0;
}

bool
ccnxPortal_Flush(CCNxPortal *portal, const CCNxStackTimeout *timeout)
{
    CCNxControl *control = ccnxControl_CreateFlushRequest();

    // this needs to be better wrapped in ccnxControl
    PARCJSON *json = ccnxControl_GetJson(control);
    uint64_t expectedSequenceNumber = controlPlaneInterface_GetSequenceNumber(json);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(control);
    bool result = ccnxPortal_Send(portal, message, CCNxStackTimeout_Never);

    ccnxMetaMessage_Release(&message);
    ccnxControl_Release(&control);

    if (result == true) {
        bool matchedSequenceNumber = false;
        while (!matchedSequenceNumber) {
            message = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
            if (message) {
                if (ccnxMetaMessage_IsControl(message)) {
                    control = ccnxMetaMessage_GetControl(message);
                    if (ccnxControl_IsCPI(control) && ccnxControl_IsACK(control)) {
                        uint64_t sequenceNumber = ccnxControl_GetAckOriginalSequenceNumber(control);
                        if (sequenceNumber == expectedSequenceNumber) {
                            matchedSequenceNumber = true;
                        }
                    }
                }
                ccnxMetaMessage_Release(&message);
            } else {
                // this is likely some sort of error with the connection.
                // should report this somehow.  This case shows up from test_ccnx_PortalAPI.
                result = false;
                break;
            }
        }
    }

    return result;
}

static void
_ccnxPortal_Destroy(CCNxPortal **portalPtr)
{
    CCNxPortal *portal = *portalPtr;

    ccnxPortal_Flush(portal, CCNxStackTimeout_Never);

    ccnxPortalStack_Stop(portal->stack);
    ccnxPortalStack_Release((CCNxPortalStack **) &portal->stack);
}

parcObject_ExtendPARCObject(CCNxPortal, _ccnxPortal_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxPortal, CCNxPortal);

parcObject_ImplementRelease(ccnxPortal, CCNxPortal);

CCNxPortal *
ccnxPortal_Create(const CCNxPortalAttributes *attributes, const CCNxPortalStack *portalStack)
{
    CCNxPortal *result = parcObject_CreateInstance(CCNxPortal);

    if (result != NULL) {
        result->stack = portalStack;
        result->status.eof = false;
        result->status.error = 0;
    }

    if (ccnxPortalStack_Start(portalStack) == false) {
        parcObject_Release((void **) &result);
    }

    return result;
}

const CCNxPortalStatus *
ccnxPortal_GetStatus(const CCNxPortal *portal)
{
    return &portal->status;
}

bool
ccnxPortal_SetAttributes(CCNxPortal *portal, const CCNxPortalAttributes *attributes)
{
    return ccnxPortalStack_SetAttributes(portal->stack, attributes);
}

int
ccnxPortal_GetFileId(const CCNxPortal *portal)
{
    return ccnxPortalStack_GetFileId(portal->stack);
}

bool
ccnxPortal_Listen(CCNxPortal *restrict portal, const CCNxName *restrict name, const time_t secondsToLive, const CCNxStackTimeout *microSeconds)
{
    bool result = ccnxPortalStack_Listen(portal->stack, name, microSeconds);

    if (result == true) {
        _ccnxPortal_SetAnchor(portal, name, secondsToLive);
    }

    portal->status.error = (result == true) ? 0 : ccnxPortalStack_GetErrorCode(portal->stack);

    return result;
}

bool
ccnxPortal_Ignore(CCNxPortal *portal, const CCNxName *name, const CCNxStackTimeout *microSeconds)
{
    bool result = ccnxPortalStack_Ignore(portal->stack, name, microSeconds);

    portal->status.error = (result == true) ? 0 : ccnxPortalStack_GetErrorCode(portal->stack);

    return result;
}

bool
ccnxPortal_Send(CCNxPortal *restrict portal, const CCNxMetaMessage *restrict message, const CCNxStackTimeout *timeout)
{
    bool result = ccnxPortalStack_Send(portal->stack, message, timeout);

    portal->status.error = result ? 0 : ccnxPortalStack_GetErrorCode(portal->stack);
    return result;
}

CCNxMetaMessage *
ccnxPortal_Receive(CCNxPortal *portal, const CCNxStackTimeout *timeout)
{
    CCNxMetaMessage *result = ccnxPortalStack_Receive(portal->stack, timeout);

    // This modal operation of Portal is awkward.
    // Messages are interest = content-object, while Chunked is interest = {content-object_1, content-object_2, ...}
    // If chunked.
    //   If Content Object
    //      If this content object is the final chunk
    //          Set EOF

    portal->status.error = (result != NULL) ? 0 : ccnxPortalStack_GetErrorCode(portal->stack);
    return result;
}

const PARCKeyId *
ccnxPortal_GetKeyId(const CCNxPortal *portal)
{
    return ccnxPortalStack_GetKeyId(portal->stack);
}

bool
ccnxPortal_IsEOF(const CCNxPortal *portal)
{
    return portal->status.eof;
}

bool
ccnxPortal_IsError(const CCNxPortal *portal)
{
    return portal->status.error != 0;
}

int
ccnxPortal_GetError(const CCNxPortal *portal)
{
    return portal->status.error;
}
