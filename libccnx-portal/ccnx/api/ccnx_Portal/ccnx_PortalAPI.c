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

#include <ccnx/api/ccnx_Portal/ccnx_PortalAPI.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Deque.h>

typedef struct {
    PARCDeque *messageAddressBuffer;
} _CCNxPortalAPIContext;

static void
_ccnxPortalAPIContext_Destroy(_CCNxPortalAPIContext **instancePtr)
{
    _CCNxPortalAPIContext *instance = *instancePtr;

    parcDeque_Release(&instance->messageAddressBuffer);
}

parcObject_ExtendPARCObject(_CCNxPortalAPIContext, _ccnxPortalAPIContext_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

//static parcObject_ImplementAcquire(_ccnxPortalAPIContext, _CCNxPortalAPIContext);

static parcObject_ImplementRelease(_ccnxPortalAPIContext, _CCNxPortalAPIContext);

static _CCNxPortalAPIContext *
_ccnxPortalAPIContext_Create(void)
{
    _CCNxPortalAPIContext *result = parcObject_CreateInstance(_CCNxPortalAPIContext);
    result->messageAddressBuffer = parcDeque_Create();
    return result;
}

static void
_ccnxPortalAPI_Start(void *privateData)
{
}

static void
_ccnxPortalAPI_Stop(void *privateData)
{
}

static bool
_ccnxPortalAPI_Send(void *privateData, const CCNxMetaMessage *portalMessage, const CCNxStackTimeout *microSeconds)
{
    const _CCNxPortalAPIContext *transportContext = (_CCNxPortalAPIContext *) privateData;

    // Save the address of the portal message on our queue. We don't need to copy the whole message.
    parcDeque_Append(transportContext->messageAddressBuffer, (void *) ccnxMetaMessage_Acquire(portalMessage));

    return true;
}

static CCNxMetaMessage *
_ccnxPortalAPI_Receive(void *privateData, const CCNxStackTimeout *microSeconds)
{
    const _CCNxPortalAPIContext *transportContext = (_CCNxPortalAPIContext *) privateData;

    if (parcDeque_IsEmpty(transportContext->messageAddressBuffer)) {
        return NULL;
    }

    CCNxMetaMessage *result = (CCNxMetaMessage *) parcDeque_RemoveFirst(transportContext->messageAddressBuffer);

    return result;
}

static int
_ccnxPortalAPI_GetFileId(void *privateData)
{
    return 3;
}

static CCNxPortalAttributes *
_ccnxPortalAPI_GetAttributes(void *privateData)
{
    return NULL;
}

static bool
_ccnxPortalAPI_SetAttributes(void *privateData, const CCNxPortalAttributes *attributes)
{
    return false;
}

static bool
_ccnxPortalAPI_Listen(void *privateData, const CCNxName *name, const CCNxStackTimeout *microSeconds)
{
    return true;
}

static bool
_ccnxPortalAPI_Ignore(void *privateData, const CCNxName *name, const CCNxStackTimeout *microSeconds)
{
    return true;
}

CCNxPortal *
ccnxPortalAPI_LoopBack(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes)
{
    _CCNxPortalAPIContext *apiContext = _ccnxPortalAPIContext_Create();

    CCNxPortalStack *stack =
        ccnxPortalStack_Create(factory,
                               attributes,
                               _ccnxPortalAPI_Start,
                               _ccnxPortalAPI_Stop,
                               _ccnxPortalAPI_Receive,
                               _ccnxPortalAPI_Send,
                               _ccnxPortalAPI_Listen,
                               _ccnxPortalAPI_Ignore,
                               _ccnxPortalAPI_GetFileId,
                               _ccnxPortalAPI_SetAttributes,
                               _ccnxPortalAPI_GetAttributes,
                               apiContext,
                               (void (*)(void **))_ccnxPortalAPIContext_Release);

    CCNxPortal *result = ccnxPortal_Create(attributes, stack);
    return result;
}
