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
 * This test rig sets up a mock RTA Framework for testing Components and Connectors.
 *
 *
 */

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_Commands.c>
#include <ccnx/transport/transport_rta/core/rta_Framework_NonThreaded.h>

#ifndef MAXPATH
#define MAXPATH 1024
#endif

typedef struct mock_framework {
    PARCRingBuffer1x1*commandRingBuffer;
    PARCNotifier     *commandNotifier;
    RtaFramework     *framework;

    int stackId;
    RtaProtocolStack *stack;

    int connection_fds[2];
    RtaConnection    *connection;

    CCNxTransportConfig  *transport_config;
} MockFramework;

static MockFramework *
mockFramework_Create(CCNxTransportConfig *config)
{
    MockFramework *mock = parcMemory_AllocateAndClear(sizeof(MockFramework));
    assertNotNull(mock, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MockFramework));

    mock->transport_config = ccnxTransportConfig_Copy(config);
    assertNotNull(mock->transport_config, "%s got null params from createParams\n", __func__);

    mock->commandRingBuffer = parcRingBuffer1x1_Create(128, NULL);
    mock->commandNotifier = parcNotifier_Create();
    mock->framework = rtaFramework_Create(mock->commandRingBuffer, mock->commandNotifier);

    // Create the protocol stack

    mock->stackId = 1;
    RtaCommandCreateProtocolStack *createStack =
        rtaCommandCreateProtocolStack_Create(mock->stackId, ccnxTransportConfig_GetStackConfig(mock->transport_config));
    _rtaFramework_ExecuteCreateStack(mock->framework, createStack);
    rtaCommandCreateProtocolStack_Release(&createStack);

    // peek inside and get the protocol stack reference
    FrameworkProtocolHolder *fph = rtaFramework_GetProtocolStackByStackId(mock->framework, mock->stackId);
    mock->stack = fph->stack;

    int error = socketpair(AF_UNIX, SOCK_STREAM, 0, mock->connection_fds);
    assertFalse(error, "Error creating socket pair: (%d) %s", errno, strerror(errno));

    RtaCommandOpenConnection *openConnection = rtaCommandOpenConnection_Create(mock->stackId, mock->connection_fds[0], mock->connection_fds[1],
                                                                               ccnxConnectionConfig_GetJson(ccnxTransportConfig_GetConnectionConfig(mock->transport_config)));
    _rtaFramework_ExecuteOpenConnection(mock->framework, openConnection);
    rtaCommandOpenConnection_Release(&openConnection);

    mock->connection = rtaConnectionTable_GetByApiFd(mock->framework->connectionTable, mock->connection_fds[0]);

    // Uses the non-threaded forwarder, make sure we step at least once
    rtaFramework_NonThreadedStep(mock->framework);

    return mock;
}

static void
mockFramework_Destroy(MockFramework **mockPtr)
{
    MockFramework *mock = *mockPtr;

    rtaFramework_Teardown(mock->framework);

    parcRingBuffer1x1_Release(&mock->commandRingBuffer);
    parcNotifier_Release(&mock->commandNotifier);

    rtaFramework_Destroy(&mock->framework);
    ccnxTransportConfig_Destroy(&mock->transport_config);

    parcMemory_Deallocate((void **) &mock);
}
