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
 * This is a mock for MetisListenerOps
 *
 */

/**
 * You should #include this C file in your unit test code
 */

// ===============================
// Setup a mock for the MetisListenerOps

typedef struct mock_listener_data {
    unsigned destroyCount;
    unsigned getInterfaceIndexCount;
    unsigned getListenAddressCount;
    unsigned getEncapTypeCount;

    // These values will be returned by the appropriate getter
    unsigned interfaceIndex;
    CPIAddress *listenAddress;
    MetisEncapType encapType;
} MockListenerData;

static void
mockListener_Destroy(MetisListenerOps **opsPtr)
{
    // Don't actually destroy the data, we want to keep the counts
    MetisListenerOps *ops = *opsPtr;
    MockListenerData *data = ops->context;
    data->destroyCount++;
    parcMemory_Deallocate((void **) &ops);
    *opsPtr = NULL;
}

static unsigned
mockListener_GetInterfaceIndex(const MetisListenerOps *ops)
{
    MockListenerData *data = ops->context;
    data->getInterfaceIndexCount++;
    return data->interfaceIndex;
}

static const CPIAddress *
mockListener_GetListenAddress(const MetisListenerOps *ops)
{
    MockListenerData *data = ops->context;
    data->getListenAddressCount++;
    return data->listenAddress;
}

static MetisEncapType
mockListener_GetEncapType(const MetisListenerOps *ops)
{
    MockListenerData *data = ops->context;
    data->getEncapTypeCount++;
    return data->encapType;
}

static MetisListenerOps
    mockListenerTemplate = {
    .context           = NULL,
    .destroy           = &mockListener_Destroy,
    .getInterfaceIndex = &mockListener_GetInterfaceIndex,
    .getListenAddress  = &mockListener_GetListenAddress,
    .getEncapType      = &mockListener_GetEncapType
};

MockListenerData *
mockListenData_Create(unsigned interfaceIndex, CPIAddress *listenAddress, MetisEncapType encapType)
{
    MockListenerData *data = parcMemory_AllocateAndClear(sizeof(MockListenerData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MockListenerData));
    memset(data, 0, sizeof(MockListenerData));
    data->encapType = encapType;
    data->interfaceIndex = interfaceIndex;
    data->listenAddress = cpiAddress_Copy(listenAddress);
    return data;
}

MetisListenerOps *
mockListener_Create(MockListenerData *data)
{
    MetisListenerOps *ops = parcMemory_AllocateAndClear(sizeof(MetisListenerOps));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisListenerOps));
    memcpy(ops, &mockListenerTemplate, sizeof(MetisListenerOps));
    ops->context = data;
    return ops;
}

void
mockListenerData_Destroy(MockListenerData **dataPtr)
{
    MockListenerData *data = *dataPtr;
    cpiAddress_Destroy(&data->listenAddress);
    parcMemory_Deallocate((void **) &data);
    *dataPtr = NULL;
}
