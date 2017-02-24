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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_KeyLocator.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <parc/security/parc_Key.h>

struct ccnx_key_locator {
    union {
        PARCKey *key;
        CCNxLink *keyLink;
    } locator;
    CCNxKeyLocatorType type;
};


static void
_Destroy(CCNxKeyLocator **keyLocatorP)
{
    if (keyLocatorP != NULL) {
        CCNxKeyLocator *keyLocator = *keyLocatorP;
        if (ccnxKeyLocator_IsKey(keyLocator)) {
            parcKey_Release(&keyLocator->locator.key);
        } else if (ccnxKeyLocator_IsKeyLink(keyLocator)) {
            ccnxLink_Release(&keyLocator->locator.keyLink);
        } else {
            assertTrue(0, "KeyLocator is not one of %s.", "Key or KeyLink");
        }
    }
}

parcObject_ExtendPARCObject(CCNxKeyLocator, _Destroy, ccnxKeyLocator_Copy, ccnxKeyLocator_ToString, ccnxKeyLocator_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxKeyLocator, CCNxKeyLocator);

parcObject_ImplementRelease(ccnxKeyLocator, CCNxKeyLocator);

static CCNxKeyLocator *
_Create(void)
{
    CCNxKeyLocator *result = parcObject_CreateInstance(CCNxKeyLocator);
    if (NULL != result) {
        result->locator.key = NULL;
        result->type = CCNxKeyLocatorType_None;
    }
    return result;
}

CCNxKeyLocator *
ccnxKeyLocator_Copy(const CCNxKeyLocator *original)
{
    CCNxKeyLocator *copy = NULL;

    switch (original->type) {
        case CCNxKeyLocatorType_Key: {
            PARCKey *keyCopy = parcKey_Copy(original->locator.key);
            copy = ccnxKeyLocator_CreateFromKey(keyCopy);
            parcKey_Release(&keyCopy);
        }
        break;

        case CCNxKeyLocatorType_Link: {
            CCNxLink *keyLinkCopy = ccnxLink_Copy(original->locator.keyLink);
            copy = ccnxKeyLocator_CreateFromKeyLink(keyLinkCopy);
            ccnxLink_Release(&keyLinkCopy);
        }
        break;

        case CCNxKeyLocatorType_None:
            copy = _Create();
            break;
    }
    return copy;
}

CCNxKeyLocator *
ccnxKeyLocator_CreateFromKey(PARCKey *key)
{
    assertNotNull(key, "Parameter must be a non-null CCNxKey pointer.");

    CCNxKeyLocator *result = _Create();
    if (result != NULL) {
        result->locator.key = parcKey_Acquire(key);
        result->type = CCNxKeyLocatorType_Key;
    }

    return result;
}

CCNxKeyLocator *
ccnxKeyLocator_CreateFromKeyLink(CCNxLink *keyLink)
{
    CCNxKeyLocator *result = _Create();
    if (result != NULL) {
        result->locator.keyLink = ccnxLink_Acquire(keyLink);
        result->type = CCNxKeyLocatorType_Link;
    }

    return result;
}

bool
ccnxKeyLocator_Equals(const CCNxKeyLocator *a, const CCNxKeyLocator *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->type == b->type) {
        switch (a->type) {
            case CCNxKeyLocatorType_Key:
                return parcKey_Equals(a->locator.key, b->locator.key);

            case CCNxKeyLocatorType_Link:
                return ccnxLink_Equals(a->locator.keyLink, b->locator.keyLink);

            default:
                break;
                // fall through
        }
    }
    return false;
}

CCNxKeyLocatorType
ccnxKeyLocator_GetType(const CCNxKeyLocator *ccnxKeyLocator)
{
    assertNotNull(ccnxKeyLocator, "Parameter must be a non-null CCNxKeyLocator pointer.");

    return ccnxKeyLocator->type;
}

bool
ccnxKeyLocator_IsKey(const CCNxKeyLocator *keyLocator)
{
    return (keyLocator->type == CCNxKeyLocatorType_Key);
}

bool
ccnxKeyLocator_IsKeyLink(const CCNxKeyLocator *keyLocator)
{
    return (keyLocator->type == CCNxKeyLocatorType_Link);
}

char *
ccnxKeyLocator_ToString(const CCNxKeyLocator *keyLocator)
{
    char *result;

    if (keyLocator == NULL) {
        char *nullString = "NULL";
        result = parcMemory_StringDuplicate(nullString, strlen(nullString));
    } else {
        char *locator;
        if (ccnxKeyLocator_IsKey(keyLocator)) {
            locator = parcKey_ToString(keyLocator->locator.key);
        } else if (ccnxKeyLocator_IsKeyLink(keyLocator)) {
            locator = ccnxLink_ToString(keyLocator->locator.keyLink);
        } else {
            char *initString = "not initialised.";
            locator = parcMemory_StringDuplicate(initString, strlen(initString));
        }
        char *string;
        int failure = asprintf(&string, "KeyLocator { %s }", locator);
        assertTrue(failure > -1, "Error asprintf");

        parcMemory_Deallocate((void **) &locator);

        result = parcMemory_StringDuplicate(string, strlen(string));
        free(string);
    }

    return result;
}

CCNxLink *
ccnxKeyLocator_GetKeyLink(const CCNxKeyLocator *keyLocator)
{
    assertTrue(keyLocator->type == CCNxKeyLocatorType_Link, "Key Locator must be of type CCNxKeyLocatorType_Link");

    if (keyLocator->type == CCNxKeyLocatorType_Link) {
        return keyLocator->locator.keyLink;
    }
    return NULL;
}

PARCKey *
ccnxKeyLocator_GetKey(const CCNxKeyLocator *keyLocator)
{
    assertTrue(keyLocator->type == CCNxKeyLocatorType_Key, "Key Locator must be of type CCNxKeyLocatorType_Key");

    if (keyLocator->type == CCNxKeyLocatorType_Key) {
        return keyLocator->locator.key;
    }
    return NULL;
}

void
ccnxKeyLocator_AssertValid(const CCNxKeyLocator *keyLocator)
{
    assertNotNull(keyLocator, "CCNxKeyLocator pointer was null");
    if (ccnxKeyLocator_IsKey(keyLocator)) {
        parcKey_AssertValid(keyLocator->locator.key);
    } else if (ccnxKeyLocator_IsKeyLink(keyLocator)) {
        ccnxLink_AssertValid(keyLocator->locator.keyLink);
    } else {
        assertTrue(0, "KeyLocator is not one of %s.", "Key or Key Link");
    }
}
