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

/*
 *
 */
#include "../parc_URI.c"
#include <LongBow/unit-test.h>

#include <stdint.h>

#include <parc/algol/parc_Hash.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/testing/parc_ObjectTesting.h>

#include <parc/algol/parc_Hash.h>

#include "_test_parc_URI.h"

LONGBOW_TEST_RUNNER(parcURI)
{
    LONGBOW_RUN_TEST_FIXTURE(parcURI);
}

LONGBOW_TEST_RUNNER_SETUP(parcURI)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(parcURI)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("Tests leak memory by %d allocations\n", outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(parcURI)
{
    LONGBOW_RUN_TEST_CASE(parcURI, parseScheme);
    LONGBOW_RUN_TEST_CASE(parcURI, parseScheme_Only);
    LONGBOW_RUN_TEST_CASE(parcURI, parseScheme_BadScheme);
    LONGBOW_RUN_TEST_CASE(parcURI, parseScheme_EmptyScheme);

    LONGBOW_RUN_TEST_CASE(parcURI, parseAuthority);
    LONGBOW_RUN_TEST_CASE(parcURI, parseAuthority_NoAuthority);
    LONGBOW_RUN_TEST_CASE(parcURI, parseAuthority_NoPath);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Acquire);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_AuthorityEquals_SamePointer);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_AuthorityEquals_NullPointers);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_QueryEquals_SamePointer);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_QueryEquals_NullPointers);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_FragmentEquals_SamePointer);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_FragmentEquals_NullPointers);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SchemeEquals_SamePointer);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SchemeEquals_NullPointers);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Parse);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Parse_NoScheme);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Parse_SchemeOnly);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetScheme);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetScheme_Reset);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetScheme_Resetting);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetFragment);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetFragment_Reset);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetFragment_Resetting);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetQuery);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetQuery_Reset);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetQuery_Resetting);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetAuthority);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetAuthority_Reset);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_SetAuthority_Resetting);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Equals_Contract);

    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_GetPath);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_GetQuery);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_GetFragment);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_Copy);
    LONGBOW_RUN_TEST_CASE(parcURI, parcURI_ToString_Full);

    LONGBOW_RUN_TEST_CASE(parcURI, PARCURI_ToString_SchemeOnly);
    LONGBOW_RUN_TEST_CASE(parcURI, PARCURI_ToString_NoAuthority);
}

LONGBOW_TEST_FIXTURE_SETUP(parcURI)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(parcURI)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(parcURI, parcURI_Acquire)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);

    PARCURI *handle = parcURI_Acquire(uri);

    assertTrue(parcURI_Equals(uri, handle), "Expected URI and acquired handle to be equal");

    parcURI_Release(&handle);
    parcURI_Release(&uri);
    parcMemory_Deallocate((void **) &uriString);
}

LONGBOW_TEST_CASE(parcURI, parcURI_AuthorityEquals_SamePointer)
{
    char *authority = "authority@auth";
    assertTrue(_parcURI_AuthorityEquals(authority, authority),
               "Expected authorities to be equal since they're the same pointers: %p - %p",
               (void *) authority, (void *) authority);
}

LONGBOW_TEST_CASE(parcURI, parcURI_AuthorityEquals_NullPointers)
{
    char *authority = "authority@auth";
    assertFalse(_parcURI_AuthorityEquals(NULL, authority), "Expected authorities to not be equal since one is NULL");
    assertFalse(_parcURI_AuthorityEquals(authority, NULL), "Expected authorities to not be equal since one is NULL");
    assertTrue(_parcURI_AuthorityEquals(NULL, NULL), "Expected authorities to be equal since both are NULL");
}

LONGBOW_TEST_CASE(parcURI, parcURI_QueryEquals_SamePointer)
{
    char *query = "query";
    assertTrue(_parcURI_QueryEquals(query, query),
               "Expected queries to be equal since they're the same pointers: %p - %p",
               (void *) query, (void *) query);
}

LONGBOW_TEST_CASE(parcURI, parcURI_QueryEquals_NullPointers)
{
    char *query = "query";
    assertFalse(_parcURI_QueryEquals(NULL, query), "Expected queries to not be equal since one is NULL");
    assertFalse(_parcURI_QueryEquals(query, NULL), "Expected queries to not be equal since one is NULL");
    assertTrue(_parcURI_QueryEquals(NULL, NULL), "Expected queries to be equal since both are NULL");
}

LONGBOW_TEST_CASE(parcURI, parcURI_FragmentEquals_SamePointer)
{
    char *fragment = "fragment";
    assertTrue(_parcURI_FragmentEquals(fragment, fragment),
               "Expected fragments to be equal since they're the same pointers: %p - %p",
               (void *) fragment, (void *) fragment);
}

LONGBOW_TEST_CASE(parcURI, parcURI_FragmentEquals_NullPointers)
{
    char *fragment = "fragment";
    assertFalse(_parcURI_FragmentEquals(NULL, fragment), "Expected fragments to not be equal since one is NULL");
    assertFalse(_parcURI_FragmentEquals(fragment, NULL), "Expected fragments to not be equal since one is NULL");
    assertTrue(_parcURI_FragmentEquals(NULL, NULL), "Expected fragments to be equal since both are NULL");
}

LONGBOW_TEST_CASE(parcURI, parcURI_SchemeEquals_SamePointer)
{
    char *scheme = "scheme";
    assertTrue(_parcURI_SchemeEquals(scheme, scheme),
               "Expected schemes to be equal since they're the same pointers: %p - %p",
               (void *) scheme, (void *) scheme);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SchemeEquals_NullPointers)
{
    char *scheme = "scheme";
    assertFalse(_parcURI_SchemeEquals(NULL, scheme), "Expected schemes to not be equal since one is NULL");
    assertFalse(_parcURI_SchemeEquals(scheme, NULL), "Expected schemes to not be equal since one is NULL");
    assertTrue(_parcURI_SchemeEquals(NULL, NULL), "Expected schemes to be equal since both are NULL");
}

LONGBOW_TEST_CASE(parcURI, parseScheme_EmptyScheme)
{
    const char *pointer;
    char *actual = _parseScheme(":", &pointer); // empty string
    assertNull(actual, "Parsed scheme should be NULL since the input string was empty.");
    actual = _parseScheme("", &pointer); // empty string
    assertNull(actual, "Parsed scheme should be NULL since the input string was empty.");
}

LONGBOW_TEST_CASE(parcURI, parcURI_Parse)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);
    assertNotNull(uri, "Expected non-null result for '%s'", uriString);

    memset(uriString, 0, strlen(uriString));

    assertTrue(strcmp(URI_SCHEME, parcURI_GetScheme(uri)) == 0,
               "Expected '%s', actual '%s'", URI_SCHEME, parcURI_GetScheme(uri));
    assertTrue(strcmp(URI_AUTHORITY, parcURI_GetAuthority(uri)) == 0,
               "Expected '%s', actual '%s'", URI_AUTHORITY, parcURI_GetAuthority(uri));
    assertTrue(strcmp(URI_QUERY, parcURI_GetQuery(uri)) == 0,
               "Expected '%s', actual '%s'", URI_QUERY, parcURI_GetQuery(uri));
    assertTrue(strcmp(URI_FRAGMENT, parcURI_GetFragment(uri)) == 0,
               "Expected '%s', actual '%s'", URI_FRAGMENT, parcURI_GetFragment(uri));

    parcMemory_Deallocate((void **) &uriString);

    parcURI_Release(&uri);
    assertNull(uri, "Expected parcURI_Release to null the pointer.");
}

LONGBOW_TEST_CASE(parcURI, parcURI_Parse_NoScheme)
{
    char *uriString = "/" URI_PATH_SEGMENT;

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);
    assertNull(uri,
               "Expected null result for '%s'", uriString);
    parcMemory_Deallocate((void **) &uriString);
}

LONGBOW_TEST_CASE(parcURI, parcURI_Parse_SchemeOnly)
{
    char *uriString = URI_SCHEME ":";

    uriString = parcMemory_StringDuplicate(uriString, strlen(uriString));

    PARCURI *uri = parcURI_Parse(uriString);
    assertNotNull(uri,
                  "Expected non-null result for '%s'", uriString);

    memset(uriString, 0, strlen(uriString));

    assertTrue(strcmp(URI_SCHEME, parcURI_GetScheme(uri)) == 0,
               "Expected '%s', actual '%s'", URI_SCHEME, parcURI_GetScheme(uri));
    assertNull(parcURI_GetAuthority(uri),
               "Expected NULL, actual '%s'", parcURI_GetAuthority(uri));
    assertNull(parcURI_GetQuery(uri),
               "Expected NULL, actual '%s'", parcURI_GetQuery(uri));
    assertNull(parcURI_GetFragment(uri),
               "Expected NULL, actual '%s'", parcURI_GetFragment(uri));

    parcMemory_Deallocate((void **) &uriString);
    parcURI_Release(&uri);
    assertNull(uri,
               "Expected parcURI_Release to null the pointer.");
}

LONGBOW_TEST_CASE(parcURI, parseScheme)
{
    const char *pointer;
    char *actual = _parseScheme(URI_FULL, &pointer);
    assertTrue(strcmp(URI_SCHEME, actual) == 0,
               "Expected '%s' actual '%s'", URI_SCHEME, actual);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, parseScheme_Only)
{
    const char *pointer;
    char *actual = _parseScheme(URI_SCHEME ":", &pointer);
    assertTrue(strcmp(URI_SCHEME, actual) == 0,
               "Expected '%s' actual '%s'", URI_SCHEME, actual);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, parseScheme_BadScheme)
{
    const char *pointer;
    char *actual = _parseScheme("", &pointer);
    assertNull(actual,
               "Expected NULL actual '%s'", actual);
}

LONGBOW_TEST_CASE(parcURI, PARCURI_GetAuthority)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    const char *actual = parcURI_GetAuthority(uri);
    assertTrue(strcmp(TEST_URI_AUTHORITY, actual) == 0,
               "Expected '%s' actual '%s'", URI_AUTHORITY, actual);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_GetQuery)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    const char *actual = parcURI_GetQuery(uri);
    assertTrue(strcmp(URI_QUERY, actual) == 0,
               "Expected '%s' actual '%s'", URI_FRAGMENT, actual);

    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_GetFragment)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    const char *actual = parcURI_GetFragment(uri);
    assertTrue(strcmp(URI_FRAGMENT, actual) == 0,
               "Expected '%s' actual '%s'", URI_FRAGMENT, actual);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_GetPath)
{
    char *uriString = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    PARCURI *uri = parcURI_Parse(uriString);
    PARCURIPath *actual = parcURI_GetPath(uri);

    char *string = parcURIPath_ToString(actual);

    char *expected = URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    assertTrue(strcmp(expected, string) == 0, "Expected '%s' actual '%s'", expected, string);

    parcMemory_Deallocate((void **) &string);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_Copy)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    PARCURI *uri2 = parcURI_Copy(uri);

    char *expected = parcURI_ToString(uri);
    char *actual = parcURI_ToString(uri2);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &expected);
    parcMemory_Deallocate((void **) &actual);
    parcURI_Release(&uri);
    parcURI_Release(&uri2);
}

LONGBOW_TEST_CASE(parcURI, parcURI_Equals_Contract)
{
    PARCURI *x = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    PARCURI *y = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    PARCURI *z = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    PARCURI *u = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "?" URI_QUERY);

    parcObjectTesting_AssertEqualsFunction(parcURI_Equals, x, y, z, u);

    parcURI_Release(&x);
    parcURI_Release(&y);
    parcURI_Release(&z);
    parcURI_Release(&u);
}

LONGBOW_TEST_CASE(parcURI, PARCURI_GetScheme)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    const char *actual = parcURI_GetScheme(uri);
    assertTrue(strcmp(TEST_URI_SCHEME, actual) == 0,
               "Expected '%s' actual '%s'", TEST_URI_SCHEME, actual);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, PARCURI_Parse)
{
    PARCURI *uri = parcURI_Parse(URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT);
    assertNotNull(uri,
                  "Expected a non-null result.");

    assertTrue(strcmp(URI_SCHEME, parcURI_GetScheme(uri)) == 0,
               "Expected '%s', actual '%s'", URI_SCHEME, parcURI_GetScheme(uri));
    assertTrue(strcmp(URI_AUTHORITY, parcURI_GetAuthority(uri)) == 0,
               "Expected '%s', actual '%s'", URI_AUTHORITY, parcURI_GetAuthority(uri));
    assertTrue(strcmp(URI_QUERY, parcURI_GetQuery(uri)) == 0,
               "Expected '%s', actual '%s'", URI_QUERY, parcURI_GetQuery(uri));
    assertTrue(strcmp(URI_FRAGMENT, parcURI_GetFragment(uri)) == 0,
               "Expected '%s', actual '%s'", URI_FRAGMENT, parcURI_GetFragment(uri));

    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_ToString_Full)
{
    char *expected = URI_SCHEME "://" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;

    PARCURI *uri = parcURI_Parse(expected);
    char *actual = parcURI_ToString(uri);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcMemory_Deallocate((void **) &actual);
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, PARCURI_ToString_SchemeOnly)
{
    char *expected = URI_SCHEME ":" "/";
    PARCURI *uri = parcURI_Parse(expected);
    char *actual = parcURI_ToString(uri);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    parcURI_Release(&uri);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, PARCURI_ToString_NoAuthority)
{
    char *expected = URI_SCHEME ":" "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT "?" URI_QUERY "#" URI_FRAGMENT;
    PARCURI *uri = parcURI_Parse(expected);
    char *actual = parcURI_ToString(uri);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);

    parcURI_Release(&uri);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, parseAuthority)
{
    char *authority = "//" URI_AUTHORITY "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;
    char *expected = URI_AUTHORITY;

    const char *pointer;
    char *actual = _parseAuthority(authority, &pointer);
    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", expected, actual);
    assertTrue(*pointer == '/',
               "Expected '/' actual '%c'", *pointer);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, parseAuthority_NoAuthority)
{
    char *string = "/" URI_PATH_SEGMENT "/" URI_PATH_SEGMENT;

    const char *pointer;
    char *actual = _parseAuthority(string, &pointer);
    assertTrue(actual == NULL,
               "Expected NULL actual '%s'", actual);
    assertTrue(*pointer == '/',
               "Expected '/' actual '%c'", *pointer);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetScheme)
{
    char *scheme = "scheme";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetScheme(uri, scheme);

    assertTrue(strcmp(scheme, parcURI_GetScheme(uri)) == 0,
               "Expected %s actual %s", scheme, parcURI_GetScheme(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetScheme_Resetting)
{
    char *scheme = "scheme";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetScheme(uri, scheme);
    _parcURI_SetScheme(uri, scheme);

    assertTrue(strcmp(scheme, parcURI_GetScheme(uri)) == 0,
               "Expected %s actual %s", scheme, parcURI_GetScheme(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetScheme_Reset)
{
    char *scheme = "scheme";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetScheme(uri, scheme);
    _parcURI_SetScheme(uri, NULL);

    assertNull(parcURI_GetScheme(uri),
               "Expected NULL actual %s", parcURI_GetScheme(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetFragment)
{
    char *fragment = "fragment";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetFragment(uri, fragment);

    assertTrue(strcmp(fragment, parcURI_GetFragment(uri)) == 0,
               "Expected %s actual %s", fragment, parcURI_GetFragment(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetFragment_Resetting)
{
    char *fragment = "fragment";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetFragment(uri, fragment);
    _parcURI_SetFragment(uri, fragment);

    assertTrue(strcmp(fragment, parcURI_GetFragment(uri)) == 0,
               "Expected %s actual %s", fragment, parcURI_GetFragment(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetFragment_Reset)
{
    char *fragment = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetFragment(uri, fragment);
    _parcURI_SetFragment(uri, NULL);

    assertNull(parcURI_GetFragment(uri),
               "Expected NULL actual %s", parcURI_GetFragment(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetQuery)
{
    char *query = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetQuery(uri, query);

    assertTrue(strcmp(query, parcURI_GetQuery(uri)) == 0,
               "Expected %s actual %s", query, parcURI_GetQuery(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetQuery_Resetting)
{
    char *query = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetQuery(uri, query);
    _parcURI_SetQuery(uri, query);

    assertTrue(strcmp(query, parcURI_GetQuery(uri)) == 0,
               "Expected %s actual %s", query, parcURI_GetQuery(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetQuery_Reset)
{
    char *query = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetQuery(uri, query);
    _parcURI_SetQuery(uri, NULL);

    assertNull(parcURI_GetQuery(uri),
               "Expected NULL actual %s", parcURI_GetQuery(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetAuthority)
{
    char *authority = "authority@auth";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetAuthority(uri, authority);

    assertTrue(strcmp(authority, parcURI_GetAuthority(uri)) == 0,
               "Expected %s actual %s", authority, parcURI_GetAuthority(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetAuthority_Resetting)
{
    char *authority = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetAuthority(uri, authority);
    _parcURI_SetAuthority(uri, authority);

    assertTrue(strcmp(authority, parcURI_GetAuthority(uri)) == 0,
               "Expected %s actual %s", authority, parcURI_GetAuthority(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parcURI_SetAuthority_Reset)
{
    char *query = "query";
    PARCURI *uri = parcURI_Create();
    _parcURI_SetAuthority(uri, query);
    _parcURI_SetAuthority(uri, NULL);

    assertNull(parcURI_GetAuthority(uri),
               "Expected NULL actual %s", parcURI_GetAuthority(uri));
    parcURI_Release(&uri);
}

LONGBOW_TEST_CASE(parcURI, parseAuthority_NoPath)
{
    char *authority = "//" URI_AUTHORITY;
    char *expected = URI_AUTHORITY;

    const char *pointer;
    char *actual = _parseAuthority(authority, &pointer);

    assertTrue(strcmp(expected, actual) == 0,
               "Expected '%s' actual '%s'", authority, actual);
    assertTrue(*pointer == 0,
               "Expected null actual '%c'", *pointer);
    parcMemory_Deallocate((void **) &actual);
}

LONGBOW_TEST_CASE(parcURI, parseFragment)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(parcURI, parsePath)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(parcURI, parseQuery)
{
    testUnimplemented("This test is unimplemented");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parcURI);
    int status = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(status);
}
