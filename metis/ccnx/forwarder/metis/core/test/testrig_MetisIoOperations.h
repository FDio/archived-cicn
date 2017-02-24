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

#ifndef Metis_testrig_MetisIoOperations_h
#define Metis_testrig_MetisIoOperations_h

/**
 * Setup a test rig around a MetisIoOperation so we have visibility in to
 * what the connection table is doing
 *
 * Usage: Use <code>mockIoOperationsData_Create()</code> or <code>mockIoOperationsData_CreateSimple()</code>
 *        to create the MetisIoOperations.  You can then inspect the TestData inside the context
 *        by mapping <code>TestData *data = (TestData *) metisIoOperations_GetClosure(ops)</code>.
 *
 * IMPORTANT: ops->destroy(&ops) will not destroy the test rig.  It will increment a counter.
 *            you must call <code>testdata_Destroy(&ops)</code> yourself.  You should call this
 *            as the very last thing, even after <code>metisForwarder_Destroy()</code>, if you put
 *            the MetisIoOpereations in the connection table.
 */
static bool                     mockIoOperations_Send(MetisIoOperations *ops, const CPIAddress *nexthop, MetisMessage *message);
static const CPIAddress        *mockIoOperations_GetRemoteAddress(const MetisIoOperations *ops);
static const MetisAddressPair  *mockIoOperations_GetAddressPair(const MetisIoOperations *ops);
static bool                     mockIoOperations_IsUp(const MetisIoOperations *ops);
static bool                     mockIoOperations_IsLocal(const MetisIoOperations *ops);
static unsigned                 mockIoOperations_GetConnectionId(const MetisIoOperations *ops);
static void                     mockIoOperations_Destroy(MetisIoOperations **opsPtr);
static CPIConnectionType        mockIoOperations_GetConnectionType(const MetisIoOperations *ops);
static const void *mockIoOperations_Class(const MetisIoOperations *ops);

static MetisIoOperations mockIoOperationsTemplate = {
    .closure           = NULL,
    .send              = &mockIoOperations_Send,
    .getRemoteAddress  = &mockIoOperations_GetRemoteAddress,
    .getAddressPair    = &mockIoOperations_GetAddressPair,
    .isUp              = &mockIoOperations_IsUp,
    .isLocal           = &mockIoOperations_IsLocal,
    .getConnectionId   = &mockIoOperations_GetConnectionId,
    .destroy           = &mockIoOperations_Destroy,
    .getConnectionType = &mockIoOperations_GetConnectionType,
    .class             = &mockIoOperations_Class
};

typedef struct mock_io_operations_data {
    // counters for each call
    unsigned sendCount;
    unsigned getRemoteAddressCount;
    unsigned getAddressPairCount;
    unsigned isUpCount;
    unsigned isLocalCount;
    unsigned getConnectionIdCount;
    unsigned destroyCount;
    unsigned getConnectionTypeCount;
    unsigned classCount;

    MetisMessage *lastMessage;
    MetisAddressPair *addressPair;
    unsigned id;
    bool isUp;
    bool isLocal;
    bool sendResult;                // what to return when send() called
    CPIConnectionType connType;
} MockIoOperationsData;

/**
 * @function testdata_Create
 * @abstract Creates a data set for testing MetisIoOperations
 * @discussion
 *   Caller must explicitly use <code>testdata_Destroy()</code> when done.  Calling the destroyer through
 *   the io operations only increments counters, it does not destroy the object.
 *
 * @param <#param1#>
 * @return <#return#>
 */
static MetisIoOperations *
mockIoOperationsData_Create(MetisAddressPair *pair, unsigned id, bool isUp, bool sendResult, bool isLocal, CPIConnectionType connType)
{
    MockIoOperationsData *data = parcMemory_AllocateAndClear(sizeof(MockIoOperationsData));
    data->addressPair = pair;
    data->id = id;
    data->isUp = isUp;
    data->sendResult = sendResult;
    data->lastMessage = NULL;
    data->isLocal = isLocal;
    data->connType = connType;

    MetisIoOperations *ops = parcMemory_AllocateAndClear(sizeof(MetisIoOperations));
    memcpy(ops, &mockIoOperationsTemplate, sizeof(MetisIoOperations));
    ops->closure = data;

    return ops;
}

/**
 * @function testdata_CreateSimple
 * @abstract Creates a data set for testing MetisIoOperations
 * @discussion
 *   Caller must explicitly use <code>testdata_Destroy()</code> when done.  Calling the destroyer through
 *   the io operations only increments counters, it does not destroy the object.
 *
 * @param <#param1#>
 * @return <#return#>
 */
static MetisIoOperations *
mockIoOperationsData_CreateSimple(unsigned addressLocal, unsigned addressRemote, unsigned id, bool isUp, bool sendResult, bool isLocal)
{
    CPIAddress *local = cpiAddress_CreateFromInterface(addressLocal);
    CPIAddress *remote = cpiAddress_CreateFromInterface(addressRemote);
    MetisAddressPair *pair = metisAddressPair_Create(local, remote);
    MetisIoOperations *ops = mockIoOperationsData_Create(pair, id, isUp, sendResult, isLocal, cpiConnection_UDP);
    cpiAddress_Destroy(&local);
    cpiAddress_Destroy(&remote);
    return ops;
}

static void
mockIoOperationsData_Destroy(MetisIoOperations **opsPtr)
{
    MetisIoOperations *ops = *opsPtr;
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);

    metisAddressPair_Release(&data->addressPair);
    if (data->lastMessage) {
        metisMessage_Release(&data->lastMessage);
    }
    parcMemory_Deallocate((void **) &data);
    ops->closure = NULL;
    parcMemory_Deallocate((void **) &ops);
    *opsPtr = NULL;
}

static bool
mockIoOperations_Send(MetisIoOperations *ops, const CPIAddress *nexthop, MetisMessage *message)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->sendCount++;

    if (message) {
        if (data->lastMessage) {
            metisMessage_Release(&data->lastMessage);
        }

        data->lastMessage = metisMessage_Acquire(message);
    }

    return data->sendResult;
}

static const CPIAddress *
mockIoOperations_GetRemoteAddress(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->getRemoteAddressCount++;
    return metisAddressPair_GetRemote(data->addressPair);
}

static const MetisAddressPair *
mockIoOperations_GetAddressPair(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->getAddressPairCount++;
    return data->addressPair;
}

static bool
mockIoOperations_IsUp(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->isUpCount++;
    return data->isUp;
}

static bool
mockIoOperations_IsLocal(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->isLocalCount++;
    return data->isLocal;
}


static unsigned
mockIoOperations_GetConnectionId(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->getConnectionIdCount++;
    return data->id;
}

static void
mockIoOperations_Destroy(MetisIoOperations **opsPtr)
{
    MockIoOperationsData *data = (MockIoOperationsData *) (*opsPtr)->closure;
    data->destroyCount++;
    *opsPtr = NULL;
}

static CPIConnectionType
mockIoOperations_GetConnectionType(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->getConnectionTypeCount++;
    return data->connType;
}

static const void *
mockIoOperations_Class(const MetisIoOperations *ops)
{
    MockIoOperationsData *data = (MockIoOperationsData *) metisIoOperations_GetClosure(ops);
    data->classCount++;
    return __FILE__;
}
#endif // Metis_testrig_MetisIoOperations_h
