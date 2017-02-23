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
#include "../strategy_All.c"
#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(strategy_All)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(strategy_All)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(strategy_All)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, strategyAll_Create);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, strategyAll_Create)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, __llvm_gcov_flush);
    LONGBOW_RUN_TEST_CASE(Local, __llvm_gcov_init);
    LONGBOW_RUN_TEST_CASE(Local, __llvm_gcov_writeout);
    LONGBOW_RUN_TEST_CASE(Local, strategyAll_AddNexthop);
    LONGBOW_RUN_TEST_CASE(Local, strategyAll_ImplDestroy);
    LONGBOW_RUN_TEST_CASE(Local, strategyAll_LookupNexthop);
    LONGBOW_RUN_TEST_CASE(Local, strategyAll_ReceiveObject);
    LONGBOW_RUN_TEST_CASE(Local, strategyAll_RemoveNexthop);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, __llvm_gcov_flush)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, __llvm_gcov_init)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, __llvm_gcov_writeout)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, strategyAll_AddNexthop)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, strategyAll_ImplDestroy)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, strategyAll_LookupNexthop)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, strategyAll_ReceiveObject)
{
    testUnimplemented("This test is unimplemented");
}

LONGBOW_TEST_CASE(Local, strategyAll_RemoveNexthop)
{
    testUnimplemented("This test is unimplemented");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(strategy_All);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
