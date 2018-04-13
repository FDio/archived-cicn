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
// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnx_KeystoreUtilities.c"

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>

#include <errno.h>
#include <ftw.h>

typedef struct test_data {
    char dirname[1024];
} TestData;

static int
deleteEntry(const char *fpath, const struct stat *sb,
            int tflag, struct FTW *ftwbuf)
{
    if (tflag == FTW_DP) {
        // directory in post-order
        rmdir(fpath);
    } else {
        unlink(fpath);
    }
    return 0;           /* To tell nftw() to continue */
}

__attribute__((unused))
static void
recursiveDelete(const char *path)
{
    // only allow under tmp
    assertTrue(strncmp(path, "/tmp/", 5) == 0, "Path must begin with /tmp/: %s", path);
    // dont allow ".."
    assertNull(strstr(path, ".."), "Path cannot have .. in it: %s", path);

    // depth first, dont't follow symlinks
    int flags = FTW_DEPTH | FTW_PHYS;

    // maximum 20 fds open at a time
    int max_fd = 20;

    int failure = nftw(path, deleteEntry, max_fd, flags);
    assertFalse(failure, "Error on recursive delete: (%d) %s", errno, strerror(errno));
}

static TestData *
commonSetup(const char *testCaseName)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    sprintf(data->dirname, "/tmp/%s.%d", testCaseName, getpid());
    mkdir(data->dirname, S_IRWXU | S_IRWXG);
    setenv("HOME", data->dirname, 1);

    return data;
}

static void
commonTeardown(TestData **dataPtr)
{
    TestData *data = *dataPtr;

    recursiveDelete(data->dirname);
    parcMemory_Deallocate((void **) &data);
    *dataPtr = NULL;
}

LONGBOW_TEST_RUNNER(ccnx_KeystoreUtilities)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_KeystoreUtilities)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_KeystoreUtilities)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    TestData *data = commonSetup(longBowTestCase_GetName(testCase));
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    commonTeardown(&data);
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_ConstructPath);
    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_HomeDirectoryFromEnv);
    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_HomeDirectoryFromPasswd);

    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Missing);

    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Newfile);
    LONGBOW_RUN_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Oldfile);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    parcSecurity_Init();

    TestData *data = commonSetup(longBowTestCase_GetName(testCase));
    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    commonTeardown(&data);

    parcSecurity_Fini();

    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_ConstructPath)
{
    const char *dir = "/some/where";
    const char *file = "else";
    const char *truth = "/some/where/else";

    char *test = ccnxKeystoreUtilities_ConstructPath(dir, file);
    assertTrue(strcmp(truth, test) == 0, "Wrong path, expected %s got %s", truth, test);
    parcMemory_Deallocate((void **) &test);
}

LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_HomeDirectoryFromEnv)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    char *dir = ccnxKeystoreUtilities_HomeDirectoryFromEnv();
    assertNotNull(dir, "Did not get HOME variable from environment");
    assertTrue(strcmp(dir, data->dirname) == 0, "HOME directory not correct, expecting %s got %s\n", data->dirname, dir);
    parcMemory_Deallocate((void **) &dir);
}

LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_HomeDirectoryFromPasswd)
{
    char *dir = ccnxKeystoreUtilities_HomeDirectoryFromPasswd();
    assertNotNull(dir, "Did not get HOME variable from environment");
    parcMemory_Deallocate((void **) &dir);
}

/**
 * Create a temporary directory, set HOME to it, then try to open, but do
 * not create the keystore.  should return NULL.
 */
LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Missing)
{
    KeystoreParams *params = ccnxKeystoreUtilities_OpenFromHomeDirectory("abcd");
    assertNull(params, "Signer should have been null opening from non-existent keystore");
}


/**
 * Create a keystore with the old default name in the old location
 */
LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Oldfile)
{
    char *homedir = ccnxKeystoreUtilities_GetHomeDirectory();
    char *ccnxdir = ccnxKeystoreUtilities_ConstructPath(homedir, ".ccnx");
    mkdir(ccnxdir, 0700);
    char *path = ccnxKeystoreUtilities_ConstructPath(ccnxdir, ".ccnx_keystore");

    bool success = parcPkcs12KeyStore_CreateFile(path, "1234", "ccnxuser", PARCSigningAlgorithm_RSA, 1024, 365);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile() failed.");

    KeystoreParams *signer = ccnxKeystoreUtilities_OpenFromHomeDirectory("1234");
    assertNotNull(signer, "Signer should be non-null opening from a file we just created");
    keystoreParams_Destroy(&signer);

    parcMemory_Deallocate((void **) &path);
    parcMemory_Deallocate((void **) &ccnxdir);
    parcMemory_Deallocate((void **) &homedir);
}

/**
 * Create a keystore with the new default name in the old location
 */
LONGBOW_TEST_CASE(Local, ccnxKeystoreUtilities_OpenFromHomeDirectory_Newfile)
{
    char *homedir = ccnxKeystoreUtilities_GetHomeDirectory();
    char *ccnxdir = ccnxKeystoreUtilities_ConstructPath(homedir, ".ccnx");
    mkdir(ccnxdir, 0700);
    char *path = ccnxKeystoreUtilities_ConstructPath(ccnxdir, ".ccnx_keystore.p12");

    bool success = parcPkcs12KeyStore_CreateFile(path, "1234", "ccnxuser", PARCSigningAlgorithm_RSA, 1024, 365);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile() failed.");

    KeystoreParams *signer = ccnxKeystoreUtilities_OpenFromHomeDirectory("1234");
    assertNotNull(signer, "Signer should be non-null opening from a file we just created");
    keystoreParams_Destroy(&signer);

    parcMemory_Deallocate((void **) &path);
    parcMemory_Deallocate((void **) &ccnxdir);
    parcMemory_Deallocate((void **) &homedir);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_KeystoreUtilities);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
