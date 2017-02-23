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

#include <stdio.h>
#include <string.h>

#include <ccnx/common/ccnx_Link.h>

#include <parc/algol/parc_Object.h>

struct ccnx_link {
    const CCNxName *name;
    PARCBuffer *keyId;
    PARCBuffer *contentHash;
};

static void
_destroy(CCNxLink **linkP)
{
    ccnxName_Release((CCNxName **) &(*linkP)->name);
    if ((*linkP)->keyId != NULL) {
        parcBuffer_Release(&(*linkP)->keyId);
    }
    if ((*linkP)->contentHash != NULL) {
        parcBuffer_Release(&(*linkP)->contentHash);
    }
}

parcObject_ExtendPARCObject(CCNxLink, _destroy, NULL, ccnxLink_ToString, ccnxLink_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxLink, CCNxLink);

parcObject_ImplementRelease(ccnxLink, CCNxLink);

CCNxLink *
ccnxLink_Create(const CCNxName *name, PARCBuffer *keyId, PARCBuffer *contentObjectHash)
{
    CCNxLink *link = parcObject_CreateInstance(CCNxLink);

    if (link != NULL) {
        link->name = ccnxName_Acquire(name);
        link->keyId = (keyId == NULL) ? NULL : parcBuffer_Acquire(keyId);
        link->contentHash = (contentObjectHash == NULL) ? NULL : parcBuffer_Acquire(contentObjectHash);
    }

    return link;
}

CCNxLink *
ccnxLink_Copy(const CCNxLink *original)
{
    CCNxLink *link = parcObject_CreateInstance(CCNxLink);

    if (link != NULL) {
        link->name = ccnxName_Copy(original->name);
        link->keyId = (original->keyId == NULL) ? NULL : parcBuffer_Copy(original->keyId);
        link->contentHash = (original->contentHash == NULL) ? NULL : parcBuffer_Copy(original->contentHash);
    }

    return link;
}

const CCNxName *
ccnxLink_GetName(const CCNxLink *link)
{
    return link->name;
}

PARCBuffer *
ccnxLink_GetKeyID(const CCNxLink *link)
{
    return link->keyId;
}

PARCBuffer *
ccnxLink_GetContentObjectHash(const CCNxLink *link)
{
    return link->contentHash;
}

bool
ccnxLink_Equals(const CCNxLink *objectA, const CCNxLink *objectB)
{
    bool result = false;

    if (objectA == objectB) {
        result = true;
    } else if (objectA != NULL && objectB != NULL) {
        if (ccnxName_Equals(objectA->name, objectB->name)) {
            if (parcBuffer_Equals(objectA->keyId, objectB->keyId)) {
                if (parcBuffer_Equals(objectA->contentHash, objectB->contentHash)) {
                    return true;
                }
            }
        }
    }
    return result;
}

char *
ccnxLink_ToString(const CCNxLink *link)
{
    char *nameString = ccnxName_ToString(link->name);
    char *keyIdString = "NULL";
    bool nullKeyIdString = true;

    if (link->keyId != NULL) {
        keyIdString = parcBuffer_ToString(link->keyId);
        nullKeyIdString = false;
    }

    char *contentObjectHashString = NULL;
    bool nullContentObjectHashString = true;
    if (link->contentHash != NULL) {
        contentObjectHashString = parcBuffer_ToString(link->contentHash);
        nullContentObjectHashString = false;
    }

    char *string;
    int failure = asprintf(&string, "CCNxLink { .name=\"%s\", .KeyID=\"%s\", .ContentObjectHash=\"%s\" }",
                           nameString,
                           keyIdString,
                           contentObjectHashString);
    assertTrue(failure > -1, "Error asprintf");

    parcMemory_Deallocate((void **) &nameString);
    if (!nullKeyIdString) {
        parcMemory_Deallocate((void **) &keyIdString);
    }
    if (!nullContentObjectHashString) {
        parcMemory_Deallocate((void **) &contentObjectHashString);
    }

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);

    return result;
}

bool
ccnxLink_IsValid(const CCNxLink *link)
{
    bool result = false;

    if (link != NULL) {
        if (ccnxName_IsValid(link->name)) {
            if (link->keyId == NULL || parcBuffer_IsValid(link->keyId)) {
                if (link->contentHash == NULL || parcBuffer_IsValid(link->contentHash)) {
                    result = true;
                }
            }
        }
    }

    return result;
}

void
ccnxLink_AssertValid(const CCNxLink *link)
{
    assertTrue(ccnxLink_IsValid(link), "CCNxLink instance is not valid.");
}
