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

#ifndef _WIN32
#include <unistd.h>
#endif

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <parc/assert/parc_Assert.h>
#include <parc/algol/parc_URIAuthority.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_SafeMemory.h>

struct parc_uri_authority {
    char *userinfo;
    char *hostName;
    long port;
};

static void
_parcURIAuthority_Finalize(PARCURIAuthority **authorityPtr)
{
    PARCURIAuthority *authority = *authorityPtr;
    parcMemory_SafeFree(authority->userinfo);
    parcMemory_SafeFree(authority->hostName);
}

parcObject_ExtendPARCObject(PARCURIAuthority, _parcURIAuthority_Finalize, NULL, NULL, parcURIAuthority_Equals, NULL, NULL, NULL);

PARCURIAuthority *
parcURIAuthority_Create(void)
{
    PARCURIAuthority *result = parcObject_CreateInstance(PARCURIAuthority);
    result->userinfo = 0;
    result->hostName = 0;
    result->port = 0;
    return result;
}

parcObject_ImplementAcquire(parcURIAuthority, PARCURIAuthority);

parcObject_ImplementRelease(parcURIAuthority, PARCURIAuthority);

PARCURIAuthority *
parcURIAuthority_Parse(const char *authority)
{
    PARCURIAuthority *result = parcURIAuthority_Create();

    char *atSign = strchr(authority, '@');
    if (atSign != NULL) {
        result->userinfo = parcMemory_StringDuplicate(authority, atSign - authority);
        authority = ++atSign;
    }
    // Support literal IPv6 address specifications (i.e. [::0]:80)
    char *rightBracket = strrchr(authority, ']');
    char *lastColon = strrchr(authority, ':');
    if (rightBracket != NULL) {
        result->hostName = parcMemory_StringDuplicate(authority, rightBracket - authority + 1);
        if ((lastColon - rightBracket) > 0) {
            result->port = (short) strtol(++lastColon, NULL, 10);
        }
    } else if (lastColon != NULL) {
        result->hostName = parcMemory_StringDuplicate(authority, lastColon - authority);
        result->port = (short) strtol(++lastColon, NULL, 10);
    } else {
        result->hostName = parcMemory_StringDuplicate(authority, strlen(authority));
    }

    return result;
}

const char *
parcURIAuthority_GetUserInfo(const PARCURIAuthority *authority)
{
    return authority->userinfo;
}

const char *
parcURIAuthority_GetHostName(const PARCURIAuthority *authority)
{
    return authority->hostName;
}

long
parcURIAuthority_GetPort(const PARCURIAuthority *authority)
{
    return authority->port;
}

bool
parcURIAuthority_Equals(const PARCURIAuthority *authA, const PARCURIAuthority *authB)
{
    if (authA == authB) {
        return true;
    }
    if (authA == NULL || authB == NULL) {
        return false;
    }

    if (strcmp(authA->hostName, authB->hostName) == 0) {
        if (strcmp(authA->userinfo, authB->userinfo) == 0) {
            if (authA->port == authB->port) {
                return true;
            }
            return false;
        }
        return false;
    }

    return false;
}
