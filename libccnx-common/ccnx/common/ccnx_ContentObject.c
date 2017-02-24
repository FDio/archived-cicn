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

#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>

#include <ccnx/common/ccnx_ContentObject.h>

#include <ccnx/common/validation/ccnxValidation_HmacSha256.h>
#include <ccnx/common/validation/ccnxValidation_RsaSha256.h>

#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>

static const CCNxContentObjectInterface *_defaultImplementation = &CCNxContentObjectFacadeV1_Implementation;

/**
 * A CCNxContentObject is a wrapper class providing a consistent interface for applications.
 * Internally, the data of a ContentObject is stored in the underlying ccnxTlvDictionary.
 * Because we may have different schemas for a ContentObject, an CCNxContentObjectInterface pointer
 * is also stored in the underlying ccnxTlvDictionary. This provides access to the functions
 * used to create and access a particular implementation of a ContentObject.
 */

CCNxContentObject *
ccnxContentObject_CreateWithNameAndPayload(const CCNxName *contentName, const PARCBuffer *payload)
{
    return ccnxContentObject_CreateWithImplAndPayload(_defaultImplementation,
                                                      contentName,
                                                      CCNxPayloadType_DATA,
                                                      payload);
}

CCNxContentObject *
ccnxContentObject_CreateWithPayload(const PARCBuffer *payload)
{
    return ccnxContentObject_CreateWithImplAndPayload(_defaultImplementation,
                                                      NULL,
                                                      CCNxPayloadType_DATA,
                                                      payload);
}

CCNxContentObject *
ccnxContentObject_CreateWithImplAndPayload(const CCNxContentObjectInterface *impl,
                                           const CCNxName *contentName,
                                           const CCNxPayloadType payloadType,
                                           const PARCBuffer *payload)
{
    CCNxContentObject *result = NULL;

    if (impl->createWithNameAndPayload != NULL) {
        if (contentName == NULL) {
            result = impl->createWithPayload(payloadType, payload);
        } else {
            result = impl->createWithNameAndPayload(contentName, payloadType, payload);
        }

        // And set the dictionary's interface pointer to the one we just used to create this.
        ccnxTlvDictionary_SetMessageInterface(result, impl);
    } else {
        trapNotImplemented("ContentObject implementations must implement createWithNameAndPayload()");
    }

    ccnxContentObject_SetPathLabel(result, 0);

    return result;
}

bool
ccnxContentObject_SetSignature(CCNxContentObject *contentObject, const PARCBuffer *keyId,
                               const PARCSignature *signature, const CCNxKeyLocator *keyLocator)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;

    if (impl->setSignature != NULL) {
        result = impl->setSignature(contentObject, keyId, signature, keyLocator);
    }

    return result;
}

PARCBuffer *
ccnxContentObject_GetKeyId(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);

    PARCBuffer *result = NULL;

    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);
    if (impl->getKeyId != NULL) {
        result = impl->getKeyId(contentObject);
    }

    return result;
}

CCNxName *
ccnxContentObject_GetName(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    CCNxName *result = NULL;

    if (impl->getName != NULL) {
        result = impl->getName(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_GetName");
    }

    return result;
}

PARCBuffer *
ccnxContentObject_GetPayload(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    PARCBuffer *result = NULL;

    if (impl->getPayload != NULL) {
        result = impl->getPayload(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_GetPayload");
    }

    return result;
}

CCNxPayloadType
ccnxContentObject_GetPayloadType(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    CCNxPayloadType result = CCNxPayloadType_DATA;

    if (impl->getPayloadType != NULL) {
        result = impl->getPayloadType(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_GetPayloadType");
    }

    return result;
}

bool
ccnxContentObject_SetPayload(CCNxContentObject *contentObject, CCNxPayloadType payloadType, const PARCBuffer *payload)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;;

    if (impl->setPayload != NULL) {
        result = impl->setPayload(contentObject, payloadType, payload);;
    } else {
        trapNotImplemented("ccnxContentObject_SetPayload");
    }

    return result;
}

bool
ccnxContentObject_SetExpiryTime(CCNxContentObject *contentObject, const uint64_t expiryTIme)
{
    bool result = false;

    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->setExpiryTime != NULL) {
        result = impl->setExpiryTime(contentObject, expiryTIme);
    } else {
        trapNotImplemented("ccnxContentObject_SetExpiryTime");
    }
    return result;
}

bool
ccnxContentObject_HasExpiryTime(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;

    if (impl->hasExpiryTime != NULL) {
        result = impl->hasExpiryTime(contentObject);
    } else {
        return false;
    }

    return result;
}

uint64_t
ccnxContentObject_GetExpiryTime(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->hasExpiryTime != NULL && !impl->hasExpiryTime((CCNxTlvDictionary *) contentObject)) {
        trapUnexpectedState("ContentObject has no ExpiryTime. Call HasExpiryTime() first.");
    }

    if (impl->getExpiryTime != NULL) {
        return impl->getExpiryTime(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_HasExpiryTime");
    }
}


uint64_t
ccnxContentObject_GetPathLabel(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->hasPathLabel != NULL && !impl->hasPathLabel((CCNxTlvDictionary *) contentObject)) {
        trapUnexpectedState("ContentObject has no PathLabel. Call HasPathLabel() first.");
    }

    if (impl->getPathLabel != NULL) {
        return impl->getPathLabel(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_GetPathLabel");
    }
}

bool
ccnxContentObject_SetPathLabel(CCNxContentObject *contentObject, const uint64_t pathLabel)
{
    bool result = false;

    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->setPathLabel != NULL) {
        result = impl->setPathLabel(contentObject, pathLabel);
    } else {
        trapNotImplemented("ccnxContentObject_SetPathLabel");
    }
    return result;
}

bool
ccnxContentObject_HasPathLabel(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;

    if (impl->hasPathLabel != NULL) {
        result = impl->hasPathLabel(contentObject);
    } else {
        return false;
    }

    return result;
}

bool
ccnxContentObject_SetFinalChunkNumber(CCNxContentObject *contentObject, const uint64_t finalChunkNumber)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;

    if (impl->setFinalChunkNumber != NULL) {
        result = impl->setFinalChunkNumber(contentObject, finalChunkNumber);
    } else {
        trapNotImplemented("ccnxContentObject_SetFinalChunkNumber");
    }
    return result;
}

bool
ccnxContentObject_HasFinalChunkNumber(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    bool result = false;

    if (impl->hasFinalChunkNumber != NULL) {
        result = impl->hasFinalChunkNumber(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_HasFinalChunkNumber");
    }

    return result;
}

uint64_t
ccnxContentObject_GetFinalChunkNumber(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->hasFinalChunkNumber != NULL && !impl->hasFinalChunkNumber((CCNxTlvDictionary *) contentObject)) {
        trapUnexpectedState("ContentObject has no final chunk number. Call ccnxContentObject_HasFinalChunkNumber() first.");
    }

    if (impl->getFinalChunkNumber != NULL) {
        return impl->getFinalChunkNumber(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_GetFinalChunkNumber");
    }
}

void
ccnxContentObject_Display(const CCNxContentObject *contentObject, int indentation)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    if (impl->display != NULL) {
        impl->display(contentObject, indentation);
    } else {
        ccnxTlvDictionary_Display(contentObject, indentation);
    }
}

char *
ccnxContentObject_ToString(const CCNxContentObject *contentObject)
{
    ccnxContentObject_OptionalAssertValid(contentObject);
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    char *result = NULL;

    if (impl->toString != NULL) {
        result = impl->toString(contentObject);
    } else {
        trapNotImplemented("ccnxContentObject_ToString");
    }
    return result;
}

bool
ccnxContentObject_Equals(const CCNxContentObject *objectA, const CCNxContentObject *objectB)
{
    CCNxContentObjectInterface *implA = ccnxContentObjectInterface_GetInterface(objectA);
    CCNxContentObjectInterface *implB = ccnxContentObjectInterface_GetInterface(objectB);

    assertNotNull(implA, "ContentObject must have an valid implementation pointer.");
    assertNotNull(implB, "ContentObject must have an valid implementation pointer.");

    if (implA != implB) {
        return false;
    }

    if (implA->equals != NULL) {
        return implA->equals(objectA, objectB);
    } else {
        trapNotImplemented("ccnxContentObject_Equals");
    }
}

CCNxContentObject *
ccnxContentObject_Acquire(const CCNxContentObject *contentObject)
{
    return ccnxTlvDictionary_Acquire(contentObject);
}

void
ccnxContentObject_Release(CCNxContentObject **contentObjectP)
{
    ccnxTlvDictionary_Release(contentObjectP);
}

void
ccnxContentObject_AssertValid(const CCNxContentObject *contentObject)
{
    assertNotNull(contentObject, "Parameter must be a non-null CCNxContentObject pointer");
    CCNxContentObjectInterface *impl = ccnxContentObjectInterface_GetInterface(contentObject);

    assertNotNull(impl, "ContentObject must have a non-NUll implementation");
    if (impl->assertValid != NULL) {
        impl->assertValid(contentObject);
    }
}
