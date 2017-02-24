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
// This permits internal static functions to be visible to this Test Runner.
#include "../ccnx_PortalRTA.c"

#include <LongBow/unit-test.h>

LONGBOW_TEST_RUNNER(ccnx_PortalRTA)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified here, but every test must be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Static);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_PortalRTA)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_PortalRTA)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalRTA_Chunked);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalRTA_LoopBack);
    LONGBOW_RUN_TEST_CASE(Global, ccnxPortalRTA_Message);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, ccnxPortalRTA_Chunked)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, ccnxPortalRTA_LoopBack)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Global, ccnxPortalRTA_Message)
{
    testUnimplemented("");
}

LONGBOW_TEST_FIXTURE(Static)
{
    LONGBOW_RUN_TEST_CASE(Static, _autowrap_destroy__CCNxPortalRTAContext);
    LONGBOW_RUN_TEST_CASE(Static, _blockingPortal);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalProtocol_RTALoopback);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalProtocol_RTAMetis);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTAContext_Create);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTAContext_Destroy);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTAContext_Release);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_CreatePortal);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_GetAttributes);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_GetFileId);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Ignore);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_IsConnected);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Listen);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Receive);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Send);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_SetAttributes);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Start);
    LONGBOW_RUN_TEST_CASE(Static, _ccnxPortalRTA_Stop);
    LONGBOW_RUN_TEST_CASE(Static, _createTransportConfig);
    LONGBOW_RUN_TEST_CASE(Static, _nonBlockingPortal);
}

LONGBOW_TEST_FIXTURE_SETUP(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Static)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Static, _autowrap_destroy__CCNxPortalRTAContext)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _blockingPortal)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalProtocol_RTALoopback)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalProtocol_RTAMetis)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTAContext_Create)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTAContext_Destroy)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTAContext_Release)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_CreatePortal)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_GetAttributes)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_GetFileId)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Ignore)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_IsConnected)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Listen)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Receive)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Send)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_SetAttributes)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Start)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _ccnxPortalRTA_Stop)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _createTransportConfig)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Static, _nonBlockingPortal)
{
    testUnimplemented("");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_PortalRTA);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
