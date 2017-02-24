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

#include <config.h>
#include <LongBow/unit-test.h>
#include <fcntl.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_SafeMemory.h"
#include "../parc_File.c"

#define PATH_SEGMENT "A"

LONGBOW_TEST_RUNNER(parc_File)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Local);
    LONGBOW_RUN_TEST_FIXTURE(AcquireRelease);
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_File)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_File)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(AcquireRelease)
{
    LONGBOW_RUN_TEST_CASE(AcquireRelease, parcFile_AcquireRelease);
}

LONGBOW_TEST_FIXTURE_SETUP(AcquireRelease)
{
    longBowClipBoard_SetInt(testClipBoard, "initalAllocations", parcMemory_Outstanding());

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(AcquireRelease)
{
    uint64_t initialAllocations = longBowClipBoard_GetAsInt(testClipBoard, "initalAllocations");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (initialAllocations < outstandingAllocations) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(AcquireRelease, parcFile_AcquireRelease)
{
    char *root = "/tmp/test_parc_File";

    PARCFile *file = parcFile_Create(root);

    PARCFile *reference = parcFile_Acquire(file);

    parcFile_Release(&reference);

    parcFile_AssertValid(file);

    parcFile_Release(&file);
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcFile_CreateChild);
    LONGBOW_RUN_TEST_CASE(Global, parcFile_CreateDeleteNewFile);
    LONGBOW_RUN_TEST_CASE(Global, parcFile_CreateDelete_Directory);
    LONGBOW_RUN_TEST_CASE(Global, parcFile_Exists);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    longBowClipBoard_SetInt(testClipBoard, "initalAllocations", parcMemory_Outstanding());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint64_t initialAllocations = longBowClipBoard_GetAsInt(testClipBoard, "initalAllocations");

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (initialAllocations < outstandingAllocations) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parcFile_Exists)
{
    char *root = "/tmp";

    PARCFile *parent = parcFile_Create(root);

    char *child = "foo";
    PARCFile *file = parcFile_CreateChild(parent, child);

    parcFile_CreateNewFile(file);

    bool actual = parcFile_Exists(file);

    assertTrue(actual, "Expected the file to exist.");

    parcFile_Release(&file);
    parcFile_Release(&parent);
}

LONGBOW_TEST_CASE(Global, parcFile_CreateChild)
{
    char *root = "/tmp";

    PARCFile *parent = parcFile_Create(root);

    char *child = "foo";
    PARCFile *file = parcFile_CreateChild(parent, child);

    char *actual = parcFile_ToString(file);

    assertTrue(strcmp("/tmp/foo", actual) == 0,
               "Expected %s, actual %s", "/tmp/foo", actual);

    parcMemory_Deallocate((void **) &actual);
    parcFile_Release(&file);
    parcFile_Release(&parent);
}

LONGBOW_TEST_CASE(Global, parcFile_CreateDeleteNewFile)
{
    char *name = "/tmp/test_parc_File";

    PARCFile *file = parcFile_Create(name);

    parcFile_CreateNewFile(file);

    bool actual = parcFile_Delete(file);
    assertTrue(actual, "Expected parcFile_Delete to return true.");

    parcFile_Release(&file);
}

LONGBOW_TEST_CASE(Global, parcFile_CreateDelete_Directory)
{
    char *name = "/tmp/test_parc_File_directory";

    PARCFile *directory = parcFile_Create(name);

    parcFile_Mkdir(directory);

    char *fileName = "foo";
    PARCFile *file = parcFile_CreateChild(directory, fileName);

    bool success = parcFile_CreateNewFile(file);
    assertTrue(success, "Expected parcFile_CreateNewFile success");

    bool actual = parcFile_Delete(directory);
    assertTrue(actual, "Expected parcFile_Delete to return true.");

    parcFile_Release(&file);
    parcFile_Release(&directory);
}

LONGBOW_TEST_FIXTURE(Local)
{
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_File);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
