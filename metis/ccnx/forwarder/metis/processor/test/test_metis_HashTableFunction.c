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


// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../metis_HashTableFunction.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(metis_HashTableFunction)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(metis_HashTableFunction)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(metis_HashTableFunction)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameAndKeyIdEquals);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameAndKeyIdHashCode);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameAndObjectHashEquals);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameAndObjectHashHashCode);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameEquals);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_MessageNameHashCode);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_TlvNameCompare);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_TlvNameEquals);
    LONGBOW_RUN_TEST_CASE(Global, metisHashTableFunction_TlvNameHashCode);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameAndKeyIdEquals)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameAndKeyIdHashCode)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameAndObjectHashEquals)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameAndObjectHashHashCode)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameEquals)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_MessageNameHashCode)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_TlvNameCompare)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_TlvNameEquals)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, metisHashTableFunction_TlvNameHashCode)
{
    testUnimplemented("");
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
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(metis_HashTableFunction);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
