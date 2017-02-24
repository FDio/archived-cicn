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

#include "../component_Codec_Tlv.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <ccnx/api/control/cpi_ControlFacade.h>

#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_Security.h>
#include <ccnx/transport/transport_rta/config/config_All.h>

#include <ccnx/transport/test_tools/traffic_tools.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_cpi_add_route_crc32c.h>


#include <ccnx/common/ccnx_WireFormatMessage.h>

#include "testrig_MockFramework.c"

typedef struct test_data {
    MockFramework *mock;
    char keystore_filename[MAXPATH];
    char keystore_password[MAXPATH];
} TestData;

static CCNxTransportConfig *
codecTlv_CreateParams(const char *keystore_filename, const char *keystore_password)
{
    assertNotNull(keystore_filename, "Got null keystore name\n");
    assertNotNull(keystore_password, "Got null keystore passwd\n");

    CCNxStackConfig *stackConfig = ccnxStackConfig_Create();

    apiConnector_ProtocolStackConfig(stackConfig);
    testingUpper_ProtocolStackConfig(stackConfig);
    tlvCodec_ProtocolStackConfig(stackConfig);
    testingLower_ProtocolStackConfig(stackConfig);
    protocolStack_ComponentsConfigArgs(stackConfig, apiConnector_GetName(), testingUpper_GetName(), tlvCodec_GetName(), testingLower_GetName(), NULL);

    CCNxConnectionConfig *connConfig = apiConnector_ConnectionConfig(ccnxConnectionConfig_Create());
    testingUpper_ConnectionConfig(connConfig);
    tlvCodec_ConnectionConfig(connConfig);
    testingLower_ConnectionConfig(connConfig);

    unlink(keystore_filename);

    bool success = parcPkcs12KeyStore_CreateFile(keystore_filename, keystore_password, "alice", 1024, 30);
    assertTrue(success, "parcPkcs12KeyStore_CreateFile() failed.");

    publicKeySigner_ConnectionConfig(connConfig, keystore_filename, keystore_password);

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

LONGBOW_TEST_RUNNER(component_Codec_Tlv)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Dictionary);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(component_Codec_Tlv)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(component_Codec_Tlv)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ==================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
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

// ==================================================================================

static TransportMessage *
sendDown(TestData *data, TransportMessage *tm_going_down)
{
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, tm_going_down);
    // turn the handle enough times, the message will pass all the way out the bottom
    rtaFramework_NonThreadedStepCount(data->mock->framework, 10);
    return rtaComponent_GetMessage(out);
}

static TransportMessage *
sendUp(TestData *data, TransportMessage *tm_going_down)
{
    PARCEventQueue *out = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_UPPER, RTA_DOWN);
    PARCEventQueue *in = rtaProtocolStack_GetPutQueue(data->mock->stack, TESTING_LOWER, RTA_UP);

    rtaComponent_PutMessage(in, tm_going_down);
    // turn the handle enough times, the message will pass all the way out the bottom
    rtaFramework_NonThreadedStepCount(data->mock->framework, 10);
    return rtaComponent_GetMessage(out);
}


// ==================================================================================

LONGBOW_TEST_FIXTURE(Dictionary)
{
    LONGBOW_RUN_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Interest);
    LONGBOW_RUN_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Control);
    LONGBOW_RUN_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Raw);

    LONGBOW_RUN_TEST_CASE(Dictionary, component_Codec_Tlv_Upcall_Read_Control);
}

LONGBOW_TEST_FIXTURE_SETUP(Dictionary)
{
    longBowTestCase_SetClipBoardData(testCase, _commonSetup());
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Dictionary)
{
    _commonTeardown(longBowTestCase_GetClipBoardData(testCase));

    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * Makes sure an interest going down the stack gets encoded.  Does not test
 * the actual wire format -- that's the job of the tlv unit tests.
 */
LONGBOW_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Interest)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryInterest(data->mock->connection, CCNxTlvDictionary_SchemaVersion_V1);
    TransportMessage *test_tm = sendDown(data, tm);
    CCNxCodecNetworkBufferIoVec *vec =
        ccnxTlvDictionary_GetIoVec(transportMessage_GetDictionary(test_tm), CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_WireFormat);
    assertNotNull(vec, "Output of coded did not have a raw format message");
    transportMessage_Destroy(&test_tm);
}

/**
 * control message should be passed through
 */
LONGBOW_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Control)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryControl(data->mock->connection,
                                                                                    CCNxTlvDictionary_SchemaVersion_V1);
    TransportMessage *test_tm = sendDown(data, tm);
    PARCJSON *json = ccnxControlFacade_GetJson(transportMessage_GetDictionary(test_tm));
    assertNotNull(json, "Output of codec did not have a control message");
    transportMessage_Destroy(&test_tm);
}

LONGBOW_TEST_CASE(Dictionary, component_Codec_Tlv_Downcall_Read_Raw)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    TransportMessage *tm = trafficTools_CreateTransportMessageWithDictionaryRaw(data->mock->connection,
                                                                                CCNxTlvDictionary_SchemaVersion_V1);
    TransportMessage *test_tm = sendDown(data, tm);
    PARCBuffer *buffer = ccnxWireFormatMessage_GetWireFormatBuffer(transportMessage_GetDictionary(test_tm));
    assertNotNull(buffer, "Output of codec did not have a raw format message");
    transportMessage_Destroy(&test_tm);
}

LONGBOW_TEST_CASE(Dictionary, component_Codec_Tlv_Upcall_Read_Control)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCBuffer *wireFormat = parcBuffer_Wrap(v1_cpi_add_route_crc32c, sizeof(v1_cpi_add_route_crc32c),
                                             0, sizeof(v1_cpi_add_route_crc32c));
    CCNxTlvDictionary *dictionary = ccnxWireFormatMessage_FromControlPacketType(CCNxTlvDictionary_SchemaVersion_V1, wireFormat);
    parcBuffer_Release(&wireFormat);

    // We have not set the message type or schema
    TransportMessage *tm = transportMessage_CreateFromDictionary(dictionary);
    transportMessage_SetInfo(tm, data->mock->connection, NULL);
    ccnxTlvDictionary_Release(&dictionary);

    // ------
    // Now do the actual test of sending the transport message up the stack

    TransportMessage *test_tm = sendUp(data, tm);

    // It should now be parsed into an control message
    CCNxTlvDictionary *testdict = transportMessage_GetDictionary(test_tm);
    assertNotNull(testdict, "Failed to get dictionary from the transport message");

    assertTrue(ccnxTlvDictionary_IsControl(testdict), "Dictionary says it is not a Control");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(testdict) == CCNxTlvDictionary_SchemaVersion_V1,
               "Wrong schema, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(testdict), CCNxTlvDictionary_SchemaVersion_V1);

    transportMessage_Destroy(&tm);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(component_Codec_Tlv);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
