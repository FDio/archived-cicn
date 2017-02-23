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

#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_Memory.h>
#include <parc/logging/parc_LogReporterTextStdout.h>
#include <ccnx/forwarder/metis/testdata/metis_TestDataV1.h>

typedef struct __attribute__ ((__packed__)) metis_tlv_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t packetLength;
    uint8_t interestHopLimit;
    uint8_t returnCode;
    uint8_t flags;
    uint8_t headerLength;
} _MetisTlvFixedHeaderV1;

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

        if (next->ifa_addr->sa_family == AF_INET) {
            ifname = strdup(next->ifa_name);
        }
    }
    freeifaddrs(ifaddr);
    return ifname;
}

LONGBOW_TEST_RUNNER(darwin_Ethernet)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

static bool
_testPermissions(void)
{
    int fd = _darwinEthernet_OpenBfpDevice();
    if (fd > 0) {
        close(fd);
        return true;
    }
    return false;
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(darwin_Ethernet)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);

    if (!_testPermissions()) {
        fprintf(stderr, "Could not open a /dev/bpf device.  Check permissions.\n");
        exit(77);
        return LONGBOW_STATUS_SETUP_SKIPTESTS;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(darwin_Ethernet)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

/*
 * Creates a BPF-encapsulated ethernet frame.
 *
 * @param [in] frameLength The capture length (ether header + ccnx packet)
 * @param [out] totalLengthPtr The total read length, including BPF header and padding
 *
 * @return the parcMemory allocted byte array
 */
static uint8_t *
createBpfFrame(uint32_t frameLength, size_t *totalLengthPtr)
{
    uint16_t hdrlen = BPF_WORDALIGN(sizeof(struct bpf_hdr));
    uint32_t actualLength = BPF_WORDALIGN(frameLength + hdrlen);
    uint32_t caplen = frameLength;

    uint8_t *buffer = parcMemory_AllocateAndClear(actualLength);

    struct bpf_hdr *hdr = (struct bpf_hdr *) buffer;
    uint8_t *frame = buffer + hdrlen;

    memset(hdr, 0, hdrlen);
    hdr->bh_hdrlen = hdrlen;
    hdr->bh_caplen = caplen;

    for (int i = 0; i < caplen; i++) {
        frame[i] = i * frameLength;
    }

    // we need a valid FixedHeader length
    struct ether_header *etherHeader = (struct ether_header *) frame;
    _MetisTlvFixedHeaderV1 *fixedHeader = (_MetisTlvFixedHeaderV1 *)(frame + sizeof(struct ether_header));

    etherHeader->ether_type = htons(0x0801);
    fixedHeader->version = 1;
    fixedHeader->packetLength = htons(caplen - sizeof(struct ether_header));

    *totalLengthPtr = actualLength;

    return buffer;
}

/*
 * Create a BPF frame from a given Ethernet frame
 *
 * @param [out] totalLengthPtr The total frame length including the BPF header
 */
static uint8_t *
createBpfFrameFromEthernet(uint32_t length, const uint8_t etherframe[length], size_t *totalLengthPtr)
{
    uint16_t hdrlen = BPF_WORDALIGN(sizeof(struct bpf_hdr));
    uint32_t actualLength = BPF_WORDALIGN(length + hdrlen);
    uint32_t caplen = length;

    uint8_t *buffer = parcMemory_AllocateAndClear(actualLength);

    struct bpf_hdr *hdr = (struct bpf_hdr *) buffer;
    uint8_t *frame = buffer + hdrlen;

    memset(hdr, 0, hdrlen);
    hdr->bh_hdrlen = hdrlen;
    hdr->bh_caplen = caplen;

    memcpy(frame, etherframe, length);

    *totalLengthPtr = actualLength;

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
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0x0801;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
    metisForwarder_Destroy(&metis);

    assertNotNull(ether, "Got null ether");
    assertTrue(ether->ethertype == ethertype, "Wrong ethertype, got %x expected %x", ether->ethertype, ethertype);
    assertNotNull(ether->workBuffer, "Work buffer is null");
    assertTrue(ether->etherSocket > 0, "Invalid etherSocket, got %d", ether->etherSocket);

    metisGenericEther_Release(&ether);
}

LONGBOW_TEST_CASE(Global, metisGenericEther_Create_BadEtherType)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
    metisForwarder_Destroy(&metis);

    assertNull(ether, "Should have gotten NULL for bad ethertype");
}

LONGBOW_TEST_CASE(Global, metisGenericEther_Release)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0x0801;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
    metisForwarder_Destroy(&metis);

    metisGenericEther_Release(&ether);
    assertTrue(parcMemory_Outstanding() == 0, "Memory imbalance after release");
    assertNull(ether, "release did not null the pointer");
}

LONGBOW_TEST_CASE(Global, metisGenericEther_GetDescriptor)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0x0801;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
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
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0x0801;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
    metisForwarder_Destroy(&metis);

    size_t length_a;
    uint8_t *buffer_a = createBpfFrame(129, &length_a);
    struct bpf_hdr *hdr_a = (struct bpf_hdr *) buffer_a;
    uint8_t *frame_a = buffer_a + hdr_a->bh_hdrlen;

    parcEventBuffer_Append(ether->workBuffer, buffer_a, length_a);

    size_t length_b;
    uint8_t *buffer_b = createBpfFrame(777, &length_b);
    struct bpf_hdr *hdr_b = (struct bpf_hdr *) buffer_b;
    uint8_t *frame_b = buffer_b + hdr_b->bh_hdrlen;

    parcEventBuffer_Append(ether->workBuffer, buffer_b, length_b);

    bool success;

    PARCEventBuffer *output = parcEventBuffer_Create();

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame A");
    assertFrameEquals(frame_a, output, hdr_a->bh_caplen);

    // clear the buffer before next packet
    parcEventBuffer_Read(output, NULL, -1);

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame B");
    assertFrameEquals(frame_b, output, hdr_b->bh_caplen);


    parcMemory_Deallocate((void **) &buffer_a);
    parcMemory_Deallocate((void **) &buffer_b);
    parcEventBuffer_Destroy(&output);
    metisGenericEther_Release(&ether);
}

LONGBOW_TEST_CASE(Global, metisGenericEther_ReadNextFrame_WithPadding)
{
    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    uint16_t ethertype = 0x0801;
    MetisGenericEther *ether = metisGenericEther_Create(metis, NULL, ethertype);
    metisForwarder_Destroy(&metis);

    size_t totalLength = 0;
    uint8_t *bpf = createBpfFrameFromEthernet(sizeof(metisTestDataV1_InterestWithEthernetPadding), metisTestDataV1_InterestWithEthernetPadding, &totalLength);
    parcEventBuffer_Append(ether->workBuffer, bpf, totalLength);

    PARCEventBuffer *output = parcEventBuffer_Create();

    bool success;

    success = metisGenericEther_ReadNextFrame(ether, output);
    assertTrue(success, "Failed to read frame A");
    assertFrameEquals(metisTestDataV1_InterestWithEthernetPaddingStripped, output, sizeof(metisTestDataV1_InterestWithEthernetPaddingStripped));

    parcMemory_Deallocate((void **) &bpf);
    parcEventBuffer_Destroy(&output);
    metisGenericEther_Release(&ether);
}


LONGBOW_TEST_CASE(Global, metisGenericEther_SendFrame)
{
    char *interfaceName = getInterfaceName();
    uint16_t etherType = 0x0801;

    MetisForwarder *metis = metisForwarder_Create(NULL);
    metisLogger_SetLogLevel(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug);
    MetisGenericEther *ether = metisGenericEther_Create(metis, interfaceName, etherType);
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
    data->ether = metisGenericEther_Create(metis, device, ethertype);
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
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_SetupReceive);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_SetFilter);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_SetDeviceOptions);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_OpenBfpDevice);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_ReadSocket_True);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_ReadSocket_False);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_ReadWorkBuffer);
    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_ReadWorkBuffer_Short);

    LONGBOW_RUN_TEST_CASE(Local, _darwinEthernet_SetInterfaceAddress);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    commonSetup(testCase, NULL, 0x0801);
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

LONGBOW_TEST_CASE(Local, _darwinEthernet_SetupReceive)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_SetFilter)
{
    testUnimplemented("");
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_SetDeviceOptions)
{
    int fd = _darwinEthernet_OpenBfpDevice();
    bool success = _darwinEthernet_SetDeviceOptions(fd, NULL);
    assertTrue(success, "Error setting device options");
    close(fd);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_OpenBfpDevice)
{
    int fd = _darwinEthernet_OpenBfpDevice();
    assertTrue(fd > -1, "Error opening device");
    close(fd);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_ReadSocket_True)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Create a socket pair and pump some test data in
    int fd[2];
    int failure = socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);
    assertFalse(failure, "Error on socketpair");

    // Set non-blocking flag
    int flags = fcntl(fd[1], F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    failure = fcntl(fd[1], F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // swap out the file descriptor from the BPF to our sockpair
    close(data->ether->etherSocket);
    data->ether->etherSocket = fd[1];

    size_t totalLength;
    uint8_t *buffer = createBpfFrame(129, &totalLength);

    ssize_t bytesWritten = write(fd[0], buffer, totalLength);
    assertTrue(bytesWritten == totalLength, "Error on write, got %zd bytes expected %zu bytes", bytesWritten, totalLength);

    bool success = _darwinEthernet_ReadSocket(data->ether);
    assertTrue(success, "Did not read buffer even though we put data in socket");

    // The buffer should contain the whole frame with the BPF header
    assertTrue(parcEventBuffer_GetLength(data->ether->workBuffer) == totalLength, "Wrong lenght, got %zu expected %zu", parcEventBuffer_GetLength(data->ether->workBuffer), totalLength);

    uint8_t *test = parcEventBuffer_Pullup(data->ether->workBuffer, -1);
    assertTrue(memcmp(test, buffer, totalLength) == 0, "Buffers do not match");

    parcMemory_Deallocate((void **) &buffer);
    close(fd[0]);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_ReadSocket_False)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    // Create a socket pair and pump some test data in
    int fd[2];
    int failure = socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);
    assertFalse(failure, "Error on socketpair");

    // Set non-blocking flag
    int flags = fcntl(fd[1], F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    failure = fcntl(fd[1], F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // swap out the file descriptor from the BPF to our sockpair
    close(data->ether->etherSocket);
    data->ether->etherSocket = fd[1];

    bool success = _darwinEthernet_ReadSocket(data->ether);
    assertFalse(success, "Should have failed to read when no data present");

    close(fd[0]);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_ReadWorkBuffer)
{
    // Put a BFP packet in to the work buffer and make sure it is read correctly
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    size_t totalLength;
    uint8_t *buffer = createBpfFrame(129, &totalLength);
    struct bpf_hdr *hdr = (struct bpf_hdr *) buffer;
    uint8_t *frame = buffer + hdr->bh_hdrlen;

    parcEventBuffer_Append(data->ether->workBuffer, buffer, totalLength);

    // now read the work buffer and make sure we get the right frame
    PARCEventBuffer *output = parcEventBuffer_Create();
    _ReadWorkBufferResult result = _darwinEthernet_ReadWorkBuffer(data->ether, output);
    assertTrue(result == ReadWorkBufferResult_Ok, "Failed on ReadWorkBuffer");

    uint8_t *test = parcEventBuffer_Pullup(output, -1);
    assertTrue(memcmp(test, frame, hdr->bh_caplen) == 0, "Frames do not match");

    parcMemory_Deallocate((void **) &buffer);
    parcEventBuffer_Destroy(&output);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_ReadWorkBuffer_Short)
{
    // put an incomplete frame in to the work buffer
    TestData *data = longBowTestCase_GetClipBoardData(testCase);

    size_t totalLength;
    uint8_t *buffer = createBpfFrame(129, &totalLength);

    parcEventBuffer_Append(data->ether->workBuffer, buffer, 5);

    // now read the work buffer and make sure we get the right frame
    PARCEventBuffer *output = parcEventBuffer_Create();
    _ReadWorkBufferResult result = _darwinEthernet_ReadWorkBuffer(data->ether, output);
    assertTrue(result == ReadWorkBufferResult_Empty, "Failed on ReadWorkBuffer");

    parcMemory_Deallocate((void **) &buffer);
    parcEventBuffer_Destroy(&output);
}

LONGBOW_TEST_CASE(Local, _darwinEthernet_SetInterfaceAddress)
{
    PARCBuffer *addr = NULL;
    char ifname[1024];

    // Lookup the MAC address of an interface that is up, then ask for it.  Don't use loopback.
    struct ifaddrs *ifaddr;
    int failure = getifaddrs(&ifaddr);
    assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

    struct ifaddrs *next;
    for (next = ifaddr; next != NULL; next = next->ifa_next) {
        if ((next->ifa_addr == NULL) || ((next->ifa_flags & IFF_UP) == 0)) {
            continue;
        }

        if (next->ifa_flags & IFF_LOOPBACK) {
            continue;
        }

        if (next->ifa_addr->sa_family == AF_LINK) {
            strcpy(ifname, next->ifa_name);

            struct sockaddr_dl *addr_dl = (struct sockaddr_dl *) next->ifa_addr;

            // addr_dl->sdl_data[12] contains the interface name followed by the MAC address, so
            // need to offset in to the array past the interface name.
            addr = parcBuffer_Allocate(addr_dl->sdl_alen);
            parcBuffer_PutArray(addr, addr_dl->sdl_alen, (uint8_t *) &addr_dl->sdl_data[ addr_dl->sdl_nlen]);
            parcBuffer_Flip(addr);
            break;
        }
    }
    freeifaddrs(ifaddr);


    // If we could find an address, try to get it
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    _darwinEthernet_SetInterfaceAddress(data->ether, ifname);

    assertTrue(parcBuffer_Equals(addr, data->ether->macAddress), "Addresses do not match")
    {
        parcBuffer_Display(addr, 0);
        parcBuffer_Display(data->ether->macAddress, 0);
    }

    parcBuffer_Release(&addr);
}


// ==================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(darwin_Ethernet);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
