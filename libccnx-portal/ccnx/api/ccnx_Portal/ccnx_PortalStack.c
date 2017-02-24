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
#include <sys/errno.h>

#include <LongBow/runtime.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalStack.h>

#include <parc/algol/parc_Object.h>

struct CCNxPortalStack {
    CCNxPortalFactory *factory;

    const CCNxPortalAttributes *attributes;

    void *privateData;

    void (*start)(void *privateData);

    void (*stop)(void *privateData);

    CCNxMetaMessage *(*read)(void *privateData, const CCNxStackTimeout *microSeconds);

    bool (*write)(void *privateData, const CCNxMetaMessage *portalMessage, const CCNxStackTimeout *microSeconds);

    bool (*listen)(void *privateData, const CCNxName *restrict name, const CCNxStackTimeout *microSeconds);

    bool (*ignore)(void *privateData, const CCNxName *restrict name, const CCNxStackTimeout *microSeconds);

    int (*getFileId)(void *privateData);

    bool (*setAttributes)(void *privateData, const CCNxPortalAttributes *attributes);

    CCNxPortalAttributes * (*getAttributes)(void *privateData);

    void (*releasePrivateData)(void **privateData);
};

static void
_destroy(CCNxPortalStack **instancePtr)
{
    CCNxPortalStack *instance = *instancePtr;

    if (instance->privateData != NULL) {
        instance->releasePrivateData(&instance->privateData);
    }

    ccnxPortalFactory_Release(&instance->factory);
}

parcObject_ExtendPARCObject(CCNxPortalStack, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxPortalStack, CCNxPortalStack);

parcObject_ImplementRelease(ccnxPortalStack, CCNxPortalStack);

CCNxPortalStack *
ccnxPortalStack_Create(const CCNxPortalFactory *factory,
                       const CCNxPortalAttributes *attributes,
                       void (*start)(void *privateData),
                       void (*stop)(void *privateData),
                       CCNxMetaMessage *(*receive)(void *privateData, const CCNxStackTimeout *microSeconds),
                       bool (*send)(void *privateData, const CCNxMetaMessage *message, const CCNxStackTimeout *microSeconds),
                       bool (*listen)(void *privateData, const CCNxName *name, const CCNxStackTimeout *microSeconds),
                       bool (*ignore)(void *privateData, const CCNxName *name, const CCNxStackTimeout *microSeconds),
                       int (*getFileId)(void *privateData),
                       bool (*setAttributes)(void *privateData, const CCNxPortalAttributes *attributes),
                       CCNxPortalAttributes * (*getAttributes)(void *privateData),
                       void *privateData,
                       void (*releasePrivateData)(void **privateData))
{
    CCNxPortalStack *result = parcObject_CreateInstance(CCNxPortalStack);

    if (result != NULL) {
        result->factory = ccnxPortalFactory_Acquire(factory);
        result->attributes = attributes;
        result->start = start;
        result->stop = stop;
        result->read = receive;
        result->write = send;
        result->getFileId = getFileId;
        result->listen = listen;
        result->ignore = ignore;
        result->setAttributes = setAttributes;
        result->getAttributes = getAttributes;
        result->privateData = privateData;
        result->releasePrivateData = releasePrivateData;
    }
    return result;
}

bool
ccnxPortalStack_Start(const CCNxPortalStack *portalStack)
{
    portalStack->start(portalStack->privateData);
    return true;
}

bool
ccnxPortalStack_Stop(const CCNxPortalStack *portalStack)
{
    portalStack->stop(portalStack->privateData);
    return true;
}

CCNxMetaMessage *
ccnxPortalStack_Receive(const CCNxPortalStack *restrict portalStack, const CCNxStackTimeout *microSeconds)
{
    CCNxMetaMessage *result = portalStack->read(portalStack->privateData, microSeconds);

    return result;
}

bool
ccnxPortalStack_Send(const CCNxPortalStack *portalStack, const CCNxMetaMessage *portalMessage, const CCNxStackTimeout *microSeconds)
{
    return portalStack->write(portalStack->privateData, portalMessage, microSeconds);
}

bool
ccnxPortalStack_SetAttributes(const CCNxPortalStack *portalStack, const CCNxPortalAttributes *attributes)
{
    return portalStack->setAttributes(portalStack->privateData, attributes);
}

bool
ccnxPortalStack_Listen(const CCNxPortalStack *portalStack, const CCNxName *name, const CCNxStackTimeout *microSeconds)
{
    return portalStack->listen(portalStack->privateData, name, microSeconds);
}

bool
ccnxPortalStack_Ignore(const CCNxPortalStack *portalStack, const CCNxName *name, const CCNxStackTimeout *microSeconds)
{
    return portalStack->ignore(portalStack->privateData, name, microSeconds);
}

int
ccnxPortalStack_GetErrorCode(const CCNxPortalStack *portalStack)
{
#ifndef _ANDROID_
    extern int errno;
#endif
    return errno;
}

const CCNxPortalAttributes *
ccnxPortalStack_GetAttributes(const CCNxPortalStack *portalStack)
{
    return portalStack->attributes;
}

int
ccnxPortalStack_GetFileId(const CCNxPortalStack *portalStack)
{
    return portalStack->getFileId(portalStack->privateData);
}

const PARCKeyId *
ccnxPortalStack_GetKeyId(const CCNxPortalStack *portalStack)
{
    return ccnxPortalFactory_GetKeyId(portalStack->factory);
}

PARCProperties *
ccnxPortalStack_GetProperties(const CCNxPortalStack *portalStack)
{
    return ccnxPortalFactory_GetProperties(portalStack->factory);
}

const char *
ccnxPortalStack_GetProperty(const CCNxPortalStack *portalStack, const char *restrict name, const char *restrict defaultValue)
{
    return ccnxPortalFactory_GetProperty(portalStack->factory, name, defaultValue);
}
