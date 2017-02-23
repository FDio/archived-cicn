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

#include "../parc_URIAuthority.c"
#include <stdint.h>

#include <LongBow/unit-test.h>

#include <parc/algol/parc_URI.h>

#include "_test_parc_URI.h"

#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

LONGBOW_TEST_RUNNER(parcURIAuthority)
{
    LONGBOW_RUN_TEST_FIXTURE(parcURIAuthority);
}

LONGBOW_TEST_RUNNER_SETUP(parcURIAuthority)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parcURIAuthority)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(parcURIAuthority)
{
    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_Parse);

    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_Acquire);

    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_Equals);

    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_GetUserInfo);
    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_GetHostName);
    LONGBOW_RUN_TEST_CASE(parcURIAuthority, parcURIAuthority_GetPort);
}

LONGBOW_TEST_FIXTURE_SETUP(parcURIAuthority)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(parcURIAuthority)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_Parse)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);

    PARCURIAuthority *authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertEqualStrings(parcURIAuthority_GetUserInfo(authority), URI_AUTHORITY_USERINFO);

    parcURIAuthority_Release(&authority);

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);

    // URI without the port
    uriString = URI_SCHEME "://" URI_AUTHORITY_USERINFO "@" URI_AUTHORITY_HOSTNAME "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    uri = parcURI_Parse(uriString);
    authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertEqualStrings(parcURIAuthority_GetUserInfo(authority), URI_AUTHORITY_USERINFO);

    parcURIAuthority_Release(&authority);
    parcURI_Release(&uri);

    // URI with literal V4 address
    uriString = URI_SCHEME "://" URI_AUTHORITY_LITERAL_HOST "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    uri = parcURI_Parse(uriString);
    authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertEqualStrings(parcURIAuthority_GetHostName(authority), URI_AUTHORITY_LITERAL_HOSTNAME);

    parcURIAuthority_Release(&authority);
    parcURI_Release(&uri);

    // URI with literal V6 address
    uriString = URI_SCHEME "://" URI_AUTHORITY_LITERAL_HOST6 "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    uri = parcURI_Parse(uriString);
    authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertEqualStrings(parcURIAuthority_GetHostName(authority), URI_AUTHORITY_LITERAL_HOSTNAME6);

    parcURIAuthority_Release(&authority);
    parcURI_Release(&uri);

    // URI with full literal V6 address
    uriString = URI_SCHEME "://" URI_AUTHORITY_LITERAL_HOST6_2 "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    uri = parcURI_Parse(uriString);
    authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertEqualStrings(parcURIAuthority_GetHostName(authority), URI_AUTHORITY_LITERAL_HOSTNAME6_2);

    parcURIAuthority_Release(&authority);
    parcURI_Release(&uri);

    parcMemory_Deallocate((void **) &uriString);
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_Acquire)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);

    PARCURIAuthority *authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));
    PARCURIAuthority *handle = parcURIAuthority_Acquire(authority);

    assertTrue(parcURIAuthority_Equals(authority, handle), "URI Authorities should be equal since they refer to the same object.");

    parcURIAuthority_Release(&authority);
    parcURIAuthority_Release(&handle);

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_Equals)
{
    char *uriString1 = URI_SCHEME "://" URI_AUTHORITY;
    uriString1 = parcMemory_StringDuplicate(uriString1, strlen(uriString1));
    PARCURI *uri1 = parcURI_Parse(uriString1);
    PARCURIAuthority *x = parcURIAuthority_Parse(parcURI_GetAuthority(uri1));

    char *uriString2 = URI_SCHEME "://" URI_AUTHORITY;
    uriString2 = parcMemory_StringDuplicate(uriString2, strlen(uriString2));
    PARCURI *uri2 = parcURI_Parse(uriString2);
    PARCURIAuthority *y = parcURIAuthority_Parse(parcURI_GetAuthority(uri2));

    char *uriString3 = URI_SCHEME "://" URI_AUTHORITY;
    uriString3 = parcMemory_StringDuplicate(uriString3, strlen(uriString3));
    PARCURI *uri3 = parcURI_Parse(uriString3);
    PARCURIAuthority *z = parcURIAuthority_Parse(parcURI_GetAuthority(uri3));

    char *differentUriString = URI_SCHEME "://" URI_AUTHORITY_USERINFO;
    differentUriString = parcMemory_StringDuplicate(differentUriString, strlen(differentUriString));
    PARCURI *unequalUri = parcURI_Parse(differentUriString);
    PARCURIAuthority *u = parcURIAuthority_Parse(parcURI_GetAuthority(unequalUri));

    char *uriString5 = URI_SCHEME "://" URI_AUTHORITY_DIFFERENT_PORT;
    uriString5 = parcMemory_StringDuplicate(uriString5, strlen(uriString5));
    PARCURI *unequalUri5 = parcURI_Parse(uriString5);
    PARCURIAuthority *u5 = parcURIAuthority_Parse(parcURI_GetAuthority(unequalUri5));

    char *uriString4 = URI_SCHEME "://" URI_AUTHORITY_DIFFERENT_USER;
    uriString4 = parcMemory_StringDuplicate(uriString4, strlen(uriString4));
    PARCURI *unequalUri4 = parcURI_Parse(uriString4);
    PARCURIAuthority *u4 = parcURIAuthority_Parse(parcURI_GetAuthority(unequalUri4));

    parcObjectTesting_AssertEqualsFunction(parcURIAuthority_Equals, x, y, z, u);

    assertFalse(parcURIAuthority_Equals(x, u4), "Expected URI authorities with different user info to be unequal");
    assertFalse(parcURIAuthority_Equals(x, u5), "Expected URI authorities with different hsot names to be unequal");

    parcURIAuthority_Release(&x);
    parcURIAuthority_Release(&y);
    parcURIAuthority_Release(&z);
    parcURIAuthority_Release(&u);
    parcURIAuthority_Release(&u4);
    parcURIAuthority_Release(&u5);

    parcMemory_Deallocate((void **) &uriString1);
    parcMemory_Deallocate((void **) &uriString2);
    parcMemory_Deallocate((void **) &uriString3);
    parcMemory_Deallocate((void **) &uriString4);
    parcMemory_Deallocate((void **) &uriString5);
    parcMemory_Deallocate((void **) &differentUriString);

    parcURI_Release(&uri1);
    parcURI_Release(&uri2);
    parcURI_Release(&uri3);
    parcURI_Release(&unequalUri);
    parcURI_Release(&unequalUri4);
    parcURI_Release(&unequalUri5);
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_GetUserInfo)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    PARCURI *uri = parcURI_Parse(uriString);
    PARCURIAuthority *authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertTrue(strcmp(URI_AUTHORITY_USERINFO, parcURIAuthority_GetUserInfo(authority)) == 0, "URI Authority user info should be equal: %s - %s", URI_AUTHORITY_USERINFO, parcURIAuthority_GetUserInfo(authority));

    parcURIAuthority_Release(&authority);

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_GetHostName)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    PARCURI *uri = parcURI_Parse(uriString);
    PARCURIAuthority *authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertTrue(strcmp(URI_AUTHORITY_HOSTNAME, parcURIAuthority_GetHostName(authority)) == 0, "URI Authority host name should be equal: %s - %s", URI_AUTHORITY_HOSTNAME, parcURIAuthority_GetHostName(authority));

    parcURIAuthority_Release(&authority);

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURIAuthority, parcURIAuthority_GetPort)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));
    PARCURI *uri = parcURI_Parse(uriString);
    PARCURIAuthority *authority = parcURIAuthority_Parse(parcURI_GetAuthority(uri));

    assertTrue(atol(URI_AUTHORITY_PORT_1) == parcURIAuthority_GetPort(authority),
               "URI Authority host name should be equal: %ld - %ld", atol(URI_AUTHORITY_PORT_1), parcURIAuthority_GetPort(authority));

    parcURIAuthority_Release(&authority);

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcURIAuthority);
    int status = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(status);
}
