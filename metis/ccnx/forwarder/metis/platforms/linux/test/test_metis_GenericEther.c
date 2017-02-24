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
#include "../metis_GenericEther.c"

#include <ifaddrs.h>
#include <poll.h>

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>

#include <ccnx/forwarder/metis/testdata/metis_TestDataV0.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

/*
 * This is no longer gobally exported, so reproduce here for tests
 */
typedef struct __attribute__ ((__packed__)) metis_tlv_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t payloadLength;
    uint16_t reserved;
    uint16_t headerLength;
} _MetisTlvFixedHeaderV0;

static char *
getInterfaceName(void)
{
    // Lookup the MAC address of an interface that is up, then ask for it.  Don't use loopback.
    struct ifaddrs *ifaddr;
    int failure = getifaddrs(&ifaddr);
    assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

    char *ifname = NULL;

    struct ifaddrs *next;
    for (next = ifaddr; next != NULL && ifname == NULL; next = next->ifa_next) {
        if ((next->ifa_addr == NULL) || ((next->ifa_flags & IFF_UP) == 0)) {
            continue;
        }

        if (next->ifa_flags & IFF_LOOPBACK) {
            continue;
        }

        if (next->ifa_addr->sa_family == AF_PACKET) {
            ifname = strdup(next->ifa_name);
        }
    }
    freeifaddrs(ifaddr);
    return ifname;
}

static char *interfaceName = NULL;

// =====================================


LONGBOW_TEST_RUNNER(linux_Ethernet)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

/*
 * If we cannot open a raw socket, we cannot run any of these tests.
 */
static bool
_checkForRawAbility(void)
{
    bool success = false;
    uint16_t ethertype = 0x0801;
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ethertype));
    if (fd > 0) {
        success = true;
        close(fd);
    }

    return success;
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(linux_Ethernet)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    if (_checkForRawAbility()) {
        interfaceName = getInterfaceName();
        return LONGBOW_STATUS_SUCCEEDED;
    }

    fprintf(stderr, "%s failed to open an AF_PACKET SOCK_RAW socket, cannot execute tests\n", __func__);
    // exit here with autoconf SKIP code until LongBow exits that way for skipped tests
    exit(77);
    return LONGBOW_STATUS_SETUP_SKIPTESTS;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(linux_Ethernet)
{
    free(interfaceName);

    return LONGBOW_STATUS_SUCCEEDED;
}

static uint8_t *
createEtherFrame(uint32_t frameLength)
{
    uint8_t *frame = parcMemory_AllocateAndClear(frameLength);

    for (int i = 0; i < frameLength; i++) {
        frame[i] = i * frameLength;
    }

    // Create a proper header
    size_t messageLength = frameLength - ETHER_HDR_LEN;
    size_t payloadLength = messageLength - metisTlv_FixedHeaderLength();

    _MetisTlvFixedHeaderV0 *fixedHeader = (_MetisTlvFixedHeaderV0 *) (frame + ETHER_HDR_LEN);
    fixedHeader->version = 0;
    fixedHeader->packetType = 1;
    fixedHeader->payloadLength = payloadLength;
    fixedHeader->headerLength = 0;

    return frame;
}

static PARCBuffer *
createInterestFrame(size_t extrabytes)
{
    size_t totalLength = sizeof(metisTestDataV0_EncodedInterest) + extrabytes + ETHER_HDR_LEN;
    uint8_t *frame = createEtherFrame(totalLength);

    memcpy(frame + ETHER_HDR_LEN, metisTestDataV0_EncodedInterest, sizeof(metisTestDataV0_EncodedInterest));

    PARCBuffer *buffer = parcBuffer_Allocate(totalLength);
    parcBuffer_PutArray(buffer, totalLength, frame);
    parcBuffer_Flip(buffer);
    parcMemory_Deallocate((void **) &frame);

    return buffer;
}

// ==================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_Create);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_Create_BadEtherType);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_Release);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_GetDescriptor);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_ReadNextFrame);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_ReadNextFrame_WithPadding);
    LONGBOW_RUN_TEST_CASE(Global, metisGenericEther_SendFrame);
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

LONGBOW_TEST_CASE(Global, metisGenericEther_Create)
{
    uint16_t ethertype = 0x0801;
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    assertNotNull(ether, "Got null ether");
    assertTrue(ether->ethertype == ethertype, "Wrong ethertype, got %x expected %x", ether->ethertype, ethertype);
    assertTrue(ether->etherSocket > 0, "Invalid etherSocket, got %d", ether->etherSocket);

    metisGenericEther_Release(&ether);
}

LONGBOW_TEST_CASE(Global, metisGenericEther_Create_BadEtherType)
{
    uint16_t ethertype = 0x0000;
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    assertNull(ether, "Should have gotten NULL for bad ethertype");
}


LONGBOW_TEST_CASE(Global, metisGenericEther_Release)
{
    uint16_t ethertype = 0x0801;
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);
    metisGenericEther_Release(&ether);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after release");
    assertNull(ether, "release did not null the pointer");
}

LONGBOW_TEST_CASE(Global, metisGenericEther_GetDescriptor)
{
    uint16_t ethertype = 0x0801;
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);
    int fd = metisGenericEther_GetDescriptor(ether);
    assertTrue(fd == ether->etherSocket, "Returned wrong descriptor");
    metisGenericEther_Release(&ether);
}

static void
assertFrameEquals(uint8_t *frame, PARCEventBuffer *test, size_t caplen)
{
    assertTrue(parcEventBuffer_GetLength(test) == caplen, "Wrong length, got %zu expected %zu", parcEventBuffer_GetLength(test), caplen);

    uint8_t *linear = parcEventBuffer_Pullup(test, -1);
    assertTrue(memcmp(linear, frame, caplen) == 0, "Buffers do not compare");
}

LONGBOW_TEST_CASE(Global, metisGenericEther_ReadNextFrame)
{
    uint16_t ethertype = 0x0801;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    // swap out the PF_PACKET socket for a socket pair
    close(ether->etherSocket);

    int fd[2];
    socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);

    int localSocket = fd[0];
    ether->etherSocket = fd[1];
    _linuxEthernet_SetNonBlocking(ether);

    size_t length_a = 129;
    uint8_t *frame_a = createEtherFrame(length_a);

    size_t length_b = 777;
    uint8_t *frame_b = createEtherFrame(length_b);

    ssize_t nwritten = write(localSocket, frame_a, length_a);
    assertTrue(nwritten == length_a, "Error on write, expected %zu got %zd", length_a, nwritten);

    nwritten = write(localSocket, frame_b, length_b);
    assertTrue(nwritten == length_b, "Error on write, expected %zu got %zd", length_b, nwritten);


    // wait for it to become available
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = ether->etherSocket;
    pfd.events = POLLIN | POLLERR;

    poll(&pfd, 1, 10);

    // Something is ready to read

    bool success;

    PARCEventBuffer *output = parcEventBuffer_Create();

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame A");
    assertFrameEquals(frame_a, output, length_a);

    // clear the buffer before next packet
    parcEventBuffer_Read(output, NULL, -1);

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame B");
    assertFrameEquals(frame_b, output, length_b);

    close(localSocket);

    parcMemory_Deallocate((void **) &frame_a);
    parcMemory_Deallocate((void **) &frame_b);
    parcEventBuffer_Destroy(&output);
    metisGenericEther_Release(&ether);
}

LONGBOW_TEST_CASE(Global, metisGenericEther_ReadNextFrame_WithPadding)
{
    uint16_t ethertype = 0x0801;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    // swap out the PF_PACKET socket for a socket pair
    close(ether->etherSocket);

    int fd[2];
    socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);

    int localSocket = fd[0];
    ether->etherSocket = fd[1];
    _linuxEthernet_SetNonBlocking(ether);

    ssize_t nwritten = write(localSocket, metisTestDataV1_InterestWithEthernetPadding, sizeof(metisTestDataV1_InterestWithEthernetPadding));
    assertTrue(nwritten == sizeof(metisTestDataV1_InterestWithEthernetPadding),
               "Error on write, expected %zu got %zd",
               sizeof(metisTestDataV1_InterestWithEthernetPadding), nwritten);

    // wait for it to become available
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = ether->etherSocket;
    pfd.events = POLLIN | POLLERR;

    poll(&pfd, 1, 10);

    // Something is ready to read

    bool success;

    PARCEventBuffer *output = parcEventBuffer_Create();

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame A");
    assertFrameEquals(metisTestDataV1_InterestWithEthernetPaddingStripped, output, sizeof(metisTestDataV1_InterestWithEthernetPaddingStripped));

    close(localSocket);

    parcEventBuffer_Destroy(&output);
    metisGenericEther_Release(&ether);
}

LONGBOW_TEST_CASE(Global, metisGenericEther_SendFrame)
{
    char *interfaceName = getInterfaceName();
    uint16_t ethertype = 0x0801;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
    char dataBuffer[1024 * 1024];
    parcEventBuffer_Append(parcEventBuffer, dataBuffer, 16);

    bool result = metisGenericEther_SendFrame(ether, parcEventBuffer);
    assertTrue(result, "metisGenericEther_Sendframe failed to send smallest packet");

    parcEventBuffer_Append(parcEventBuffer, dataBuffer, 1024 * 1024);

    result = metisGenericEther_SendFrame(ether, parcEventBuffer);
    assertFalse(result, "metisGenericEther_Sendframe should have failed to send packet larger than our MTU");

    parcEventBuffer_Destroy(&parcEventBuffer);

    metisGenericEther_Release(&ether);

    free(interfaceName);
}

// ==================================================================

typedef struct test_data {
    MetisGenericEther *ether;
} TestData;

static void
commonSetup(const LongBowTestCase *testCase, const char *device, uint16_t ethertype)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    data->ether = metisGenericEther_Create(metis, interfaceName, ethertype);
    metisForwarder_Destroy(&metis);

    longBowTestCase_SetClipBoardData(testCase, data);
}

static void
commonTeardown(const LongBowTestCase *testCase)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    metisGenericEther_Release(&data->ether);
    parcMemory_Deallocate((void **) &data);
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_SetInterfaceIndex);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_SetInterfaceAddress);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_Bind);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_SetNonBlocking);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_SetupSocket);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_TrimBuffer_Length_OK);
    LONGBOW_RUN_TEST_CASE(Local, _linuxEthernet_TrimBuffer_Length_Trim);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    commonSetup(testCase, interfaceName, 0x0801);
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    commonTeardown(testCase);
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_SetInterfaceIndex)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_Bind)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_SetNonBlocking)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_SetupSocket)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_SetInterfaceAddress)
{
    PARCBuffer *addr = NULL;
    char ifname[1024];

    // Lookup the MAC address of the interface we chose back in the fixture setup
    struct ifaddrs *ifaddr;
    int failure = getifaddrs(&ifaddr);
    assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

    struct ifaddrs *next;
    for (next = ifaddr; next != NULL && addr == NULL; next = next->ifa_next) {
        if (strcmp(interfaceName, next->ifa_name) == 0) {
            if (next->ifa_addr->sa_family == AF_PACKET) {
                strcpy(ifname, next->ifa_name);

                struct sockaddr_ll *addr_ll = (struct sockaddr_ll *) next->ifa_addr;

                switch (addr_ll->sll_hatype) {
                    // list of the ARP hatypes we can extract a MAC address from
                    case ARPHRD_ETHER:
                    // fallthrough
                    case ARPHRD_IEEE802: {
                        addr = parcBuffer_Allocate(addr_ll->sll_halen);
                        parcBuffer_PutArray(addr, addr_ll->sll_halen, (uint8_t *) addr_ll->sll_addr);
                        parcBuffer_Flip(addr);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
    freeifaddrs(ifaddr);

    if (addr) {
        uint16_t ethertype = 0x0801;
        MetisForwarder *metis = metisForwarder_Create(NULL);
        metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
        MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, ethertype);
        metisForwarder_Destroy(&metis);

        assertTrue(parcBuffer_Equals(addr, ether->macAddress), "Addresses do not match")
        {
            parcBuffer_Display(addr, 0);
            parcBuffer_Display(ether->macAddress, 0);
        }

        parcBuffer_Release(&addr);
        metisGenericEther_Release(&ether);
    }
}

static void
trimBufferTest(const LongBowTestCase *testCase, size_t extraBytes)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    PARCEventBuffer *buffer = parcEventBuffer_Create();

    PARCBuffer *frameBuffer = createInterestFrame(extraBytes);
    size_t expectedSize = parcBuffer_Remaining(frameBuffer) - extraBytes;

    parcEventBuffer_Append(buffer, parcBuffer_Overlay(frameBuffer, 0), parcBuffer_Remaining(frameBuffer));

    _linuxEthernet_TrimBuffer(data->ether, buffer);

    assertTrue(parcEventBuffer_GetLength(buffer) == expectedSize,
               "Buffer incorrect size got %zu expected %zu",
               parcEventBuffer_GetLength(buffer), expectedSize);

    parcBuffer_Release(&frameBuffer);
    parcEventBuffer_Destroy(&buffer);
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_TrimBuffer_Length_OK)
{
    trimBufferTest(testCase, 0);
}

LONGBOW_TEST_CASE(Local, _linuxEthernet_TrimBuffer_Length_Trim)
{
    trimBufferTest(testCase, 4);
}


// ==================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(linux_Ethernet);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
