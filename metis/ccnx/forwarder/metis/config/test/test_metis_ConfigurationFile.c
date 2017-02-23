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


#include "../metis_ConfigurationFile.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

// =========================================================================

LONGBOW_TEST_RUNNER(metis_ConfigurationFile)
{
    LONGBOW_RUN_TEST_FIXTURE(Create);
    LONGBOW_RUN_TEST_FIXTURE(Process);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ConfigurationFile)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ConfigurationFile)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==============================================================================

LONGBOW_TEST_FIXTURE(Create)
{
    LONGBOW_RUN_TEST_CASE(Create, metisConfigurationFile_Create);
    LONGBOW_RUN_TEST_CASE(Create, metisConfigurationFile_Create_CantRead);
    LONGBOW_RUN_TEST_CASE(Create, metisConfigurationFile_Create_Missing);
}

LONGBOW_TEST_FIXTURE_SETUP(Create)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Create)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static void
_writeConfigFile(FILE *fh)
{
    ssize_t nwritten = fprintf(fh, "add listener udp conn0 127.0.0.1 9696\n");
    assertTrue(nwritten > 0, "Bad fprintf");
    fflush(fh);
}

LONGBOW_TEST_CASE(Create, metisConfigurationFile_Create)
{
    char template[] = "/tmp/test_metis_ConfigurationFile.XXXXXX";
    int fd = mkstemp(template);
    assertTrue(fd > -1, "Error creating temp file: (%d) %s", errno, strerror(errno));

    FILE *fh = fdopen(fd, "w");
    _writeConfigFile(fh);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(metis, template);

    assertNotNull(cf, "Should have returned non-null for good configuration file");

    metisConfigurationFile_Release(&cf);
    metisForwarder_Destroy(&metis);
    fclose(fh);
    unlink(template);
}

LONGBOW_TEST_CASE(Create, metisConfigurationFile_Create_CantRead)
{
    char template[] = "/tmp/test_metis_ConfigurationFile.XXXXXX";
    int fd = mkstemp(template);
    assertTrue(fd > -1, "Error creating temp file: (%d) %s", errno, strerror(errno));

    chmod(template, 0);

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(metis, template);

    chmod(template, 0600);
    unlink(template);

    uid_t uid = getuid(), euid = geteuid();
    if (uid <= 0 || uid != euid) {
	metisConfigurationFile_Release(&cf);
    } else {
	assertNull(cf, "Should have returned null configuration file for non-readable file");
    }

    metisForwarder_Destroy(&metis);
    close(fd);
}

LONGBOW_TEST_CASE(Create, metisConfigurationFile_Create_Missing)
{
    char template[] = "/tmp/test_metis_ConfigurationFile.ZZZZZZZZZ";

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(metis, template);

    assertNull(cf, "Should have returned null configuration file for missing file");

    metisForwarder_Destroy(&metis);
}

// ======================================================

typedef struct test_data {
    MetisForwarder *metis;
    char template[1024];
    int fd;
    FILE *fh;
} TestData;

LONGBOW_TEST_FIXTURE(Process)
{
    LONGBOW_RUN_TEST_CASE(Process, metisConfigurationFile_Process_NoErrors);
    LONGBOW_RUN_TEST_CASE(Process, metisConfigurationFile_Process_WithErrors);
    LONGBOW_RUN_TEST_CASE(Process, metisConfigurationFile_Process_WithComments);
    LONGBOW_RUN_TEST_CASE(Process, metisConfigurationFile_Process_Whitespace);
}

LONGBOW_TEST_FIXTURE_SETUP(Process)
{
    TestData *data = parcMemory_Allocate(sizeof(TestData));
    data->metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(data->metis), MetisLoggerFacility_Config, PARCLogLevel_Debug);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(data->metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);

    sprintf(data->template, "/tmp/test_metis_ConfigurationFile.XXXXXX");

    data->fd = mkstemp(data->template);
    assertTrue(data->fd > -1, "Error creating temp file: (%d) %s", errno, strerror(errno));

    data->fh = fdopen(data->fd, "w");

    longBowTestCase_SetClipBoardData(testCase, data);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Process)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    fclose(data->fh);
    unlink(data->template);
    metisForwarder_Destroy(&data->metis);
    parcMemory_Deallocate((void **) &data);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);

    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Process, metisConfigurationFile_Process_NoErrors)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _writeConfigFile(data->fh);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(data->metis, data->template);

    bool success = metisConfigurationFile_Process(cf);
    assertTrue(success, "Failed to execute configuration file.");
    assertTrue(cf->linesRead == 1, "Should have read 1 line, got %zu", cf->linesRead);

    metisConfigurationFile_Release(&cf);
}

LONGBOW_TEST_CASE(Process, metisConfigurationFile_Process_WithErrors)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _writeConfigFile(data->fh);

    ssize_t nwritten = fprintf(data->fh, "blah blah\n");
    assertTrue(nwritten > 0, "Bad write");

    // this should not be executed
    nwritten = fprintf(data->fh, "add listener conn3 tcp 127.0.0.1 9696\n");
    assertTrue(nwritten > 0, "Bad write");

    fflush(data->fh);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(data->metis, data->template);

    bool success = metisConfigurationFile_Process(cf);
    assertFalse(success, "Should have failed to execute configuration file.") {
        int res;
        res = system("netstat -an -p tcp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }
    assertTrue(cf->linesRead == 2, "Should have read 2 lines, got %zu", cf->linesRead);

    metisConfigurationFile_Release(&cf);
}

LONGBOW_TEST_CASE(Process, metisConfigurationFile_Process_WithComments)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _writeConfigFile(data->fh);

    ssize_t nwritten = fprintf(data->fh, "# ignore this\n");
    assertTrue(nwritten > 0, "Bad write");

    nwritten = fprintf(data->fh, "add listener tcp conn3 127.0.0.1 9696\n");
    assertTrue(nwritten > 0, "Bad write");

    fflush(data->fh);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(data->metis, data->template);

    bool success = metisConfigurationFile_Process(cf);
    assertTrue(success, "Should have failed to execute configuration file.") {
        int res;
        res = system("netstat -an -p tcp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }
    assertTrue(cf->linesRead == 3, "Should have read 3 lines, got %zu", cf->linesRead);

    metisConfigurationFile_Release(&cf);
}

LONGBOW_TEST_CASE(Process, metisConfigurationFile_Process_Whitespace)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _writeConfigFile(data->fh);

    ssize_t nwritten = fprintf(data->fh, "add listener tcp conn3 127.0.0.1 9696\n");
    assertTrue(nwritten > 0, "Bad write");

    fflush(data->fh);

    MetisConfigurationFile *cf = metisConfigurationFile_Create(data->metis, data->template);

    bool success = metisConfigurationFile_Process(cf);
    assertTrue(success, "Should have failed to execute configuration file.") {
        int res;
        res = system("netstat -an -p tcp");
        assertTrue(res != -1, "Error on system call");
        res = system("ps -el");
        assertTrue(res != -1, "Error on system call");
    }
    assertTrue(cf->linesRead == 2, "Should have read 2 lines, got %zu", cf->linesRead);

    metisConfigurationFile_Release(&cf);
}


// ==============================================================================


LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _stripLeadingWhitespace);
    LONGBOW_RUN_TEST_CASE(Local, _stripTrailingWhitespace);
    LONGBOW_RUN_TEST_CASE(Local, _trim);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

typedef struct test_vector {
    const char *input;
    const char *output;
    bool sentinel;
} TestVector;

LONGBOW_TEST_CASE(Local, _stripLeadingWhitespace)
{
    TestVector vectors[] = {
        { .input = "",       .output = "" },
        { .input = " ",      .output = "" },
        { .input = "\t",     .output = "" },
        { .input = "a",      .output = "a" },
        { .input = "abc",    .output = "abc" },
        { .input = " a c ",  .output = "a c " },
        { .input = " bc",    .output = "bc" },
        { .input = "\tbc",   .output = "bc" },
        { .input = " \tbc",  .output = "bc" },
        { .input = "\t\tbc ", .output = "bc " },
        { .input = NULL,     .output = NULL },
    };

    for (int i = 0; vectors[i].input != NULL; i++) {
        char *copy = parcMemory_StringDuplicate(vectors[i].input, strlen(vectors[i].input));
        char *test = _stripLeadingWhitespace(copy);
        assertTrue(strcmp(test, vectors[i].output) == 0, "Bad output index %d.  input = '%s' expected = '%s' actual = '%s'", i, vectors[i].input, vectors[i].output, test);
        parcMemory_Deallocate((void **) &copy);
    }
}

LONGBOW_TEST_CASE(Local, _stripTrailingWhitespace)
{
    TestVector vectors[] = {
        { .input = "",       .output = "" },
        { .input = " ",      .output = "" },
        { .input = "\t",     .output = "" },
        { .input = "a",      .output = "a" },
        { .input = "abc",    .output = "abc" },
        { .input = " a c ",  .output = " a c" },
        { .input = "bc ",    .output = "bc" },
        { .input = "bc\t",   .output = "bc" },
        { .input = "bc \t",  .output = "bc" },
        { .input = " bc\t\t", .output = " bc" },
        { .input = NULL,     .output = NULL },
    };

    for (int i = 0; vectors[i].input != NULL; i++) {
        char *copy = parcMemory_StringDuplicate(vectors[i].input, strlen(vectors[i].input));
        char *test = _stripTrailingWhitespace(copy);
        assertTrue(strcmp(test, vectors[i].output) == 0, "Bad output index %d.  input = '%s' expected = '%s' actual = '%s'", i, vectors[i].input, vectors[i].output, test);
        parcMemory_Deallocate((void **) &copy);
    }
}

LONGBOW_TEST_CASE(Local, _trim)
{
    TestVector vectors[] = {
        { .input = "",       .output = "" },
        { .input = " ",      .output = "" },
        { .input = "\t",     .output = "" },
        { .input = "a",      .output = "a" },
        { .input = "abc",    .output = "abc" },
        { .input = " a c ",  .output = "a c" },
        { .input = "bc ",    .output = "bc" },
        { .input = "bc\t",   .output = "bc" },
        { .input = "bc \t",  .output = "bc" },
        { .input = " bc\t\t", .output = "bc" },
        { .input = NULL,     .output = NULL },
    };

    for (int i = 0; vectors[i].input != NULL; i++) {
        char *copy = parcMemory_StringDuplicate(vectors[i].input, strlen(vectors[i].input));
        char *test = _trim(copy);
        assertTrue(strcmp(test, vectors[i].output) == 0, "Bad output index %d.  input = '%s' expected = '%s' actual = '%s'", i, vectors[i].input, vectors[i].output, test);
        parcMemory_Deallocate((void **) &copy);
    }
}

// ======================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ConfigurationFile);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
