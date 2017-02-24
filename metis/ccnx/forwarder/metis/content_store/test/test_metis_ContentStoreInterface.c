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

#include "../metis_ContentStoreInterface.c"

#include <LongBow/unit-test.h>

#include <parc/logging/parc_LogReporterTextStdout.h>

#include <parc/algol/parc_SafeMemory.h>

#include <ccnx/forwarder/metis/content_store/metis_LRUContentStore.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>

LONGBOW_TEST_RUNNER(metis_ContentStoreInterface)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_ContentStoreInterface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_ContentStoreInterface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_CreateRelease);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_PutContent);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_RemoveContent);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_MatchInterest);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_GetObjectCount);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_GetObjectCapacity);
    LONGBOW_RUN_TEST_CASE(Global, metisContentStoreInterface_Log);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static MetisLogger *
_createLogger(void)
{
    PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
    MetisLogger *result = metisLogger_Create(reporter, parcClock_Wallclock());
    parcLogReporter_Release(&reporter);
    metisLogger_SetLogLevel(result, MetisLoggerFacility_Processor, PARCLogLevel_Debug);

    return result;
}
static MetisContentStoreInterface *
_createContentStore(MetisLogger *logger)
{
    MetisContentStoreConfig config = {
        .objectCapacity = 1000,
    };

    MetisContentStoreInterface *result = metisLRUContentStore_Create(&config, logger);
    assertNotNull(result, "Expected to allocate a MetisContentStoreInterface");

    return result;
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_CreateRelease)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    assertNotNull(store, "Expected to allocate a MetisContentStoreInterface");

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_PutContent)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    MetisMessage *content = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisContentStoreInterface_PutContent(store, content, 1000);

    metisMessage_Release(&content);
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_RemoveContent)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    MetisMessage *content = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisContentStoreInterface_PutContent(store, content, 1000);

    bool wasRemoved = metisContentStoreInterface_RemoveContent(store, content);
    assertTrue(wasRemoved, "Expected to remove the previously stored MetisMessage");

    metisMessage_Release(&content);
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_MatchInterest)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    MetisMessage *content = metisMessage_CreateFromArray(metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject), 1, 2, logger);
    metisContentStoreInterface_PutContent(store, content, 1000);

    MetisMessage *interest = metisMessage_CreateFromArray(metisTestDataV0_InterestWithName, sizeof(metisTestDataV0_InterestWithName), 1, 2, logger);
    MetisMessage *match = metisContentStoreInterface_MatchInterest(store, interest);

    assertTrue(match == content, "Expected to retrieve the stored MetisMessage");

    metisMessage_Release(&interest);
    metisMessage_Release(&content);
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

static MetisMessage *
_createUniqueMetisMessage(MetisLogger *logger, int tweakNumber, uint8_t *template, size_t templateSize, int nameOffset)
{
    PARCBuffer *buffer = parcBuffer_Allocate(templateSize);
    memcpy(parcBuffer_Overlay(buffer, 0), template, templateSize);     // Copy the template to new memory

    // Tweak the encoded object's name so the name hash varies each time.
    uint8_t *bufPtr = parcBuffer_Overlay(buffer, 0);
    bufPtr[nameOffset] = 'a' + tweakNumber;

    MetisMessage *result = metisMessage_CreateFromArray(bufPtr, templateSize, 1, 2, logger);
    parcBuffer_Release(&buffer);

    return result;
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_GetObjectCount)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    for (int i = 1; i < 100; i++) {
        MetisMessage *content = _createUniqueMetisMessage(logger, i, metisTestDataV0_EncodedObject, sizeof(metisTestDataV0_EncodedObject),
                                                          metisTestDataV0_EncodedObject_name.offset + 4);
        metisContentStoreInterface_PutContent(store, content, 1000 + i);
        metisMessage_Release(&content);

        size_t count = metisContentStoreInterface_GetObjectCount(store);
        assertTrue(count == i, "Expected a count of %d items, got %zu", i, count);
    }

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_GetObjectCapacity)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreConfig config = {
        .objectCapacity = 1000,
    };

    MetisContentStoreInterface *store = metisLRUContentStore_Create(&config, logger);
    assertTrue(metisContentStoreInterface_GetObjectCapacity(store) == config.objectCapacity, "Expected to get back the capacity we set");
    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

LONGBOW_TEST_CASE(Global, metisContentStoreInterface_Log)
{
    MetisLogger *logger = _createLogger();
    MetisContentStoreInterface *store = _createContentStore(logger);

    metisContentStoreInterface_Log(store);

    metisContentStoreInterface_Release(&store);
    metisLogger_Release(&logger);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_ContentStoreInterface);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
