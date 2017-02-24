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

#include "../component_Codec_Tlv.c"
#include <parc/algol/parc_SafeMemory.h>

#include <stdio.h>
#include <sys/un.h>
#include <strings.h>

#include <LongBow/unit-test.h>
#include <LongBow/runtime.h>

#define DEBUG_OUTPUT 0

#include <parc/security/parc_Security.h>
#include <parc/security/parc_SymmetricKeyStore.h>

#include <ccnx/transport/transport_rta/config/config_All.h>
#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_NonThreaded.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>

#include <ccnx/transport/test_tools/traffic_tools.h>

#include "testrig_MockFramework.c"

#ifndef MAXPATH
#define MAXPATH 1024
#endif

typedef struct test_data {
    MockFramework *mock;
    char keystore_filename[MAXPATH];
    char keystore_password[MAXPATH];
} TestData;

LONGBOW_TEST_RUNNER(System)
{
    LONGBOW_RUN_TEST_FIXTURE(Component_Codec_Tlv_Hmac);
}

LONGBOW_TEST_RUNNER_SETUP(System)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(System)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ================================

static CCNxTransportConfig *
codecTlv_CreateParams(const char *keystore_name, const char *keystore_passwd)
{
    assertNotNull(keystore_name, "Got null keystore name\n");
    assertNotNull(keystore_passwd, "Got null keystore passwd\n");

    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();

    apiConnector_ProtocolStackConfig(stackConfig);
    testingUpper_ProtocolStackConfig(stackConfig);
    tlvCodec_ProtocolStackConfig(stackConfig);
    testingLower_ProtocolStackConfig(stackConfig);
    protocolStack_ComponentsConfigArgs(stackConfig,
                                       apiConnector_GetName(),
                                       testingUpper_GetName(),
                                       tlvCodec_GetName(),
                                       testingLower_GetName(),
                                       NULL);

    CCNxConnectionConfig *connConfig = apiConnector_ConnectionConfig(ccnxConnectionConfig_Create());
    testingUpper_ConnectionConfig(connConfig);
    tlvCodec_ConnectionConfig(connConfig);
    testingLower_ConnectionConfig(connConfig);

    symmetricKeySigner_ConnectionConfig(connConfig, keystore_name, keystore_passwd);
    CCNxTransportConfig *result = ccnxTransportConfig_Create(stackConfig, connConfig);
    ccnxStackConfig_Release(&stackConfig);
    return result;
}

static TestData *
_commonSetup(void)
{
    parcSecurity_Init();

    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));

    sprintf(data->keystore_filename, "/tmp/alice_keystore.p12.XXXXXX");
    mktemp(data->keystore_filename);
    sprintf(data->keystore_password, "12345");

    unlink(data->keystore_filename);
    PARCBuffer *secret_key = parcSymmetricKeyStore_CreateKey(256);
    parcSymmetricKeyStore_CreateFile(data->keystore_filename, data->keystore_password, secret_key);
    parcBuffer_Release(&secret_key);

    CCNxTransportConfig *config = codecTlv_CreateParams(data->keystore_filename, data->keystore_password);
    data->mock = mockFramework_Create(config);
    ccnxTransportConfig_Destroy(&config);

    return data;
}

static void
_commonTeardown(TestData *data)
{
    mockFramework_Destroy(&data->mock);
    unlink(data->keystore_filename);
    parcMemory_Deallocate((void **) &data);

    parcSecurity_Fini();
}

// ======================================


LONGBOW_TEST_FIXTURE(Component_Codec_Tlv_Hmac)
{
    LONGBOW_RUN_TEST_CASE(Component_Codec_Tlv_Hmac, open_close);
    LONGBOW_RUN_TEST_CASE(Component_Codec_Tlv_Hmac, Component_Codec_Tlv_Hmac_Downcall_Read);
}

LONGBOW_TEST_FIXTURE_SETUP(Component_Codec_Tlv_Hmac)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Component_Codec_Tlv_Hmac)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Component_Codec_Tlv_Hmac, open_close)
{
    // dont actually do anything.  make sure no memory leaks in setup and teardown.
}

// ============================================
// TLV tests

static TransportMessage *
sendDown(TestData *data, TransportMessage *tm_going_down)
{
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, tm_going_down);
    // turn the handle enough times, the message will pass all the way out the bottom
    rtaFramework_NonThreadedStepCount(data->mock->framework, 5);
    return rtaComponent_GetMessage(out);
}

LONGBOW_TEST_CASE(Component_Codec_Tlv_Hmac, Component_Codec_Tlv_Hmac_Downcall_Read)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    TransportMessage *tm = trafficTools_CreateTransportMessageWithSignedContentObject(data->mock->connection);

    TransportMessage *test_tm = sendDown(data, tm);

    // we should now have a RawFormat message
    CCNxCodecNetworkBufferIoVec *vec = ccnxWireFormatMessage_GetIoVec(transportMessage_GetDictionary(test_tm));
    assertNotNull(vec, "Output of coded message did not have a raw format message")
    {
        ccnxTlvDictionary_Display(transportMessage_GetDictionary(test_tm), 0);
    }

    transportMessage_Destroy(&test_tm);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(System);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
