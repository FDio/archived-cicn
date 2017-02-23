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
 * GenericEther mockup for testing metis_EtherListener.
 *
 */

#include <config.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_EventBuffer.h>

#include <ccnx/forwarder/metis/core/metis_System.h>
#include <ccnx/forwarder/metis/io/metis_GenericEther.h>

#include <LongBow/runtime.h>

struct metis_generic_ether {
    PARCDeque *inputQueue;
    int testSocket;
    int etherSocket;
    PARCBuffer *macAddress;
    uint16_t ethertype;
    unsigned refcount;
    MetisLogger *logger;
    unsigned mtu;
};

void
_metisGenericEther_Destroy(MetisGenericEther **etherPtr)
{
    MetisGenericEther *ether = *etherPtr;
    PARCBuffer *buffer;

    // the input queue is simple byte arrays of ethernet frames
    while ((buffer = parcDeque_RemoveFirst(ether->inputQueue))) {
        parcBuffer_Release(&buffer);
    }

    metisLogger_Release(&ether->logger);
    parcDeque_Release(&ether->inputQueue);
    parcBuffer_Release(&ether->macAddress);

    close(ether->testSocket);
    close(ether->etherSocket);
}

parcObject_ExtendPARCObject(MetisGenericEther, _metisGenericEther_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(metisGenericEther, MetisGenericEther);

parcObject_ImplementRelease(metisGenericEther, MetisGenericEther);

MetisGenericEther *
metisGenericEther_Create(MetisForwarder *metis, const char *deviceName, uint16_t etherType)
{
    assertNotNull(deviceName, "Parameter deviceName must be non-null");
    assertTrue(etherType >= 0x0600, "EtherType must be greater than or equal to 0x0600");

    MetisGenericEther *ether = parcObject_CreateInstance(MetisGenericEther);

    int fd[2];
    int failure = socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);
    assertFalse(failure, "Error on socketpair");

    ether->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
    ether->testSocket = fd[0];
    ether->etherSocket = fd[1];
    ether->ethertype = etherType;
    ether->inputQueue = parcDeque_Create();
    ether->mtu = 4000;

    // Set non-blocking flag
    int flags = fcntl(ether->testSocket, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    failure = fcntl(ether->testSocket, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);
    flags = fcntl(ether->etherSocket, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    failure = fcntl(ether->etherSocket, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

    // If we are passed a real interface name, use its MAC address otherwise make up something
    CPIAddress *realMacAddress = metisSystem_GetMacAddressByName(metis, deviceName);

    if (realMacAddress) {
        PARCBuffer *realMac = cpiAddress_GetLinkAddress(realMacAddress);
        ether->macAddress = parcBuffer_Copy(realMac);
        cpiAddress_Destroy(&realMacAddress);
    } else {
        uint8_t macAddress[] = { 1, 2, 3, 4, 5, 6 };

        // copy up to 6 bytes of the deviceName in to the mac address, for debugging
        size_t maxbytes = (strlen(deviceName) > 6) ? 6 : strlen(deviceName);
        memcpy(macAddress, deviceName, maxbytes);
        ether->macAddress = parcBuffer_Allocate(sizeof(macAddress));
        parcBuffer_PutArray(ether->macAddress, sizeof(macAddress), macAddress);
        parcBuffer_Flip(ether->macAddress);
    }

    return ether;
}

int
metisGenericEther_GetDescriptor(const MetisGenericEther *ether)
{
    return ether->etherSocket;
}

bool
metisGenericEther_ReadNextFrame(MetisGenericEther *ether, PARCEventBuffer *buffer)
{
    // read a byte off the etherSocket if available to clear the notification.
    // its non-blocking so no harm if nothing there.
    uint8_t one;
    ssize_t nread = read(ether->etherSocket, &one, 1);
    assertTrue(errno == EWOULDBLOCK || nread > -1, "Error on read");

    if (!parcDeque_IsEmpty(ether->inputQueue)) {
        PARCBuffer *frame = parcDeque_RemoveFirst(ether->inputQueue);
        uint8_t *bytes = parcBuffer_Overlay(frame, 0);
        size_t length = parcBuffer_Remaining(frame);

        parcEventBuffer_Append(buffer, bytes, length);
        parcBuffer_Release(&frame);
        return true;
    }
    return false;
}

bool
metisGenericEther_SendFrame(MetisGenericEther *ether, PARCEventBuffer *buffer)
{
    assertNotNull(ether, "Parameter ether must be non-null");

    // cannot use parcEventBuffer_WriteToFileDescriptor because we need to write the length in one go, not use the
    // iovec approach in parcEventBuffer_WriteToFileDescriptor.  It can cause problems on some platforms.

    uint8_t *linear = parcEventBuffer_Pullup(buffer, -1);
    size_t length = parcEventBuffer_GetLength(buffer);

    ssize_t written = write(ether->etherSocket, linear, length);
    if (written == length) {
        return true;
    }

    return false;
}

PARCBuffer *
metisGenericEther_GetMacAddress(const MetisGenericEther *ether)
{
    return ether->macAddress;
}

uint16_t
metisGenericEther_GetEtherType(const MetisGenericEther *ether)
{
    assertNotNull(ether, "Parameter ether must be non-null");
    return ether->ethertype;
}

unsigned
metisGenericEther_GetMTU(const MetisGenericEther *ether)
{
    return ether->mtu;
}

// =========
// Extra functions for testing

int
mockGenericEther_GetTestDescriptor(MetisGenericEther *ether)
{
    return ether->testSocket;
}

void
mockGenericEther_QueueFrame(MetisGenericEther *ether, PARCBuffer *ethernetFrame)
{
    parcDeque_Append(ether->inputQueue, parcBuffer_Acquire(ethernetFrame));
}

void
mockGenericEther_Notify(MetisGenericEther *ether)
{
    // notify the etherSocket by writing to the testSocket
    uint8_t one = 1;
    ssize_t nwritten = write(ether->testSocket, &one, 1);
    assertTrue(nwritten == 1, "Error on write, expected 1, got %zd", nwritten);
}

/*
 * Create an Ethernet frame enclosing the ccnxPacket.  does not include the FCS.
 */
PARCBuffer *
mockGenericEther_createFrame(size_t length, const uint8_t *ccnxPacket, const uint8_t dmac[ETHER_ADDR_LEN], const uint8_t smac[ETHER_ADDR_LEN], uint16_t ethertype)
{
    size_t totalLength = sizeof(struct ether_header) + length;
    PARCBuffer *buffer = parcBuffer_Allocate(totalLength);

    struct ether_header hdr;

    memcpy(hdr.ether_dhost, dmac, ETHER_ADDR_LEN);
    memcpy(hdr.ether_shost, smac, ETHER_ADDR_LEN);
    hdr.ether_type = htons(ethertype);

    parcBuffer_PutArray(buffer, sizeof(hdr), (uint8_t *) &hdr);

    parcBuffer_PutArray(buffer, length, ccnxPacket);

    parcBuffer_Flip(buffer);
    return buffer;
}

