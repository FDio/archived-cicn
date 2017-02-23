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

#include <config.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include <linux/if_arp.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/forwarder/metis/io/metis_GenericEther.h>
#include <ccnx/forwarder/metis/core/metis_System.h>
#include <parc/algol/parc_Object.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/io/metis_Ethernet.h>

struct metis_generic_ether {
    uint16_t ethertype;
    int etherSocket;

    int linuxInterfaceIndex;

    PARCBuffer *macAddress;
    MetisLogger *logger;

    // MTU set on interface when we are created
    unsigned mtu;
};

static bool _linuxEthernet_SetupSocket(MetisGenericEther *ether, const char *devstr);

static void
_metisGenericEther_Destroy(MetisGenericEther **etherPtr)
{
    MetisGenericEther *ether = *etherPtr;

    if (ether->etherSocket > 0) {
        close(ether->etherSocket);
    }

    if (ether->macAddress) {
        parcBuffer_Release(&ether->macAddress);
    }

    metisLogger_Release(&ether->logger);
}

parcObject_ExtendPARCObject(MetisGenericEther, _metisGenericEther_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(metisGenericEther, MetisGenericEther);

parcObject_ImplementRelease(metisGenericEther, MetisGenericEther);

// =========================
// PUBLIC API
// =========================

MetisGenericEther *
metisGenericEther_Create(MetisForwarder *metis, const char *deviceName, uint16_t etherType)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(deviceName, "Parameter deviceName must be non-null");

    MetisGenericEther *ether = NULL;

    if (metisEthernet_IsValidEthertype(etherType)) {
        ether = parcObject_CreateInstance(MetisGenericEther);
        ether->ethertype = etherType;
        ether->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
        ether->mtu = metisSystem_InterfaceMtu(metis, deviceName);

        ether->etherSocket = -1; // invalid valid
        ether->macAddress = NULL;

        bool success = _linuxEthernet_SetupSocket(ether, deviceName);

        if (success) {
            if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                char *str = parcBuffer_ToHexString(ether->macAddress);
                metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                "GenericEther %p created on device %s (%s) for ethertype 0x%04x fd %d ifindex %u mtu %u",
                                (void *) ether, deviceName, str, etherType, ether->etherSocket, ether->linuxInterfaceIndex,
                                ether->mtu);
                parcMemory_Deallocate((void **) &str);
            }
        } else {
            if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error)) {
                metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                                "GenericEther failed to created on device %s for ethertype 0x%04x",
                                deviceName, etherType);
            }

            // this will also null ether
            metisGenericEther_Release(&ether);
        }
    } else {
        if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "GenericEther failed to created on device %s for ethertype 0x%04x, invalid ethertype",
                            deviceName, etherType);
        }
    }

    return ether;
}

int
metisGenericEther_GetDescriptor(const MetisGenericEther *ether)
{
    assertNotNull(ether, "Parameter ether must be non-null");
    return ether->etherSocket;
}

/**
 * Based on the fixed header, trim the buffer
 *
 * Some platforms do not strip the ethernet CRC from the raw packet.  Trim the buffer to
 * the right sized based on the fixed header.
 *
 * @param [in] ether An allocated ethernet interface
 * @param [in] readBuffer The buffer to trim
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_linuxEthernet_TrimBuffer(MetisGenericEther *ether, PARCEventBuffer *readBuffer)
{
    // read the fixed header
    uint8_t *etherHeader = parcEventBuffer_Pullup(readBuffer, ETHER_HDR_LEN + metisTlv_FixedHeaderLength());

    if (etherHeader) {
        uint8_t *fixedHeader = etherHeader + ETHER_HDR_LEN;
        size_t totalLength = metisTlv_TotalPacketLength(fixedHeader) + ETHER_HDR_LEN;

        if (parcEventBuffer_GetLength(readBuffer) > totalLength) {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                "%s buffer length %zu, actual length %zu (ether header + ccnx packet), trimming %zu bytes",
                                __func__, parcEventBuffer_GetLength(readBuffer), totalLength,
                                parcEventBuffer_GetLength(readBuffer) - totalLength);
            }

            PARCEventBuffer *temp = parcEventBuffer_Create();

            // there's no way to drain from the end, so we move to a tempororary buffer,
            // drain the unwanted part, then move back.

            int movedBytes = parcEventBuffer_ReadIntoBuffer(readBuffer, temp, totalLength);
            assertTrue(movedBytes == totalLength, "Failed to move all the bytes, got %d expected %zu", movedBytes, totalLength);

            // flush all the bytes out of the read buffer
            parcEventBuffer_Read(readBuffer, NULL, -1);

            // now put back what we want
            int failure = parcEventBuffer_AppendBuffer(temp, readBuffer);
            assertFalse(failure, "parcEventBuffer_AppendBuffer failed");

            parcEventBuffer_Destroy(&temp);
        }
    }
}

/*
 * Reading a raw socket, on some systems, may include the FCS trailer
 */
bool
metisGenericEther_ReadNextFrame(MetisGenericEther *ether, PARCEventBuffer *readBuffer)
{
    assertNotNull(ether, "Parameter ether must be non-null");
    assertNotNull(readBuffer, "Parameter readBuffer must be non-null");

    bool success = false;

    int evread_length = parcEventBuffer_ReadFromFileDescriptor(readBuffer, ether->etherSocket, (int) -1);

    if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "%s read length %d", __func__, evread_length);
    }

    if (evread_length > 0) {
        _linuxEthernet_TrimBuffer(ether, readBuffer);
        success = true;
    }

    return success;
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
    assertNotNull(ether, "Parameter ether must be non-null");
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

// ==================
// PRIVATE API

static bool
_linuxEthernet_SetInterfaceIndex(MetisGenericEther *ether, const char *devstr)
{
    // get the interface index of the desired device
    bool success = false;

    struct ifreq if_idx;
    memset(&if_idx, 0, sizeof(if_idx));
    strncpy(if_idx.ifr_name, devstr, IFNAMSIZ - 1);
    if (!ioctl(ether->etherSocket, SIOCGIFINDEX, &if_idx)) {
        ether->linuxInterfaceIndex = if_idx.ifr_ifindex;
        success = true;
    } else {
        if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "ioctl SIOCGIFINDEX error: (%d) %s", errno, strerror(errno));
        }
    }

    return success;
}

static bool
_linuxEthernet_SetInterfaceAddress(MetisGenericEther *ether, const char *devstr)
{
    bool success = false;

    assertNull(ether->macAddress, "Should only be called once with null macAddress");

    // get the MAC address of the device
    struct ifreq if_mac;
    memset(&if_mac, 0, sizeof(if_mac));
    strncpy(if_mac.ifr_name, devstr, IFNAMSIZ - 1);
    if (!ioctl(ether->etherSocket, SIOCGIFHWADDR, &if_mac)) {
        if (if_mac.ifr_hwaddr.sa_family == ARPHRD_ETHER) {
            ether->macAddress = parcBuffer_Allocate(ETHER_ADDR_LEN);
            parcBuffer_PutArray(ether->macAddress, ETHER_ADDR_LEN, (uint8_t *) if_mac.ifr_hwaddr.sa_data);
            parcBuffer_Flip(ether->macAddress);
            success = true;
        } else {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                                "Device %s does not have an Ethernet hardware address", devstr);
            }
        }
    }
    return success;
}

static bool
_linuxEthernet_Bind(MetisGenericEther *ether)
{
    bool success = false;

    // we need to bind to the ethertype to receive packets
    struct sockaddr_ll my_addr;
    my_addr.sll_family = PF_PACKET;
    my_addr.sll_protocol = htons(ether->ethertype);
    my_addr.sll_ifindex = ether->linuxInterfaceIndex;

    if (!bind(ether->etherSocket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_ll))) {
        success = true;
    } else {
        if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "bind error: (%d) %s", errno, strerror(errno));
        }
    }
    return success;
}

static bool
_linuxEthernet_SetNonBlocking(MetisGenericEther *ether)
{
    bool success = false;
    uint32_t on = 1;
    if (!ioctl(ether->etherSocket, FIONBIO, &on)) {
        success = true;
    }
    return success;
}

static bool
_linuxEthernet_SetupSocket(MetisGenericEther *ether, const char *devstr)
{
    bool success = false;
    ether->etherSocket = socket(AF_PACKET, SOCK_RAW, htons(ether->ethertype));
    if (ether->etherSocket > 0) {
        if (_linuxEthernet_SetInterfaceIndex(ether, devstr)) {
            if (_linuxEthernet_SetInterfaceAddress(ether, devstr)) {
                if (_linuxEthernet_Bind(ether)) {
                    // set non-blocking
                    if (_linuxEthernet_SetNonBlocking(ether)) {
                        success = true;
                    }
                }
            }
        }
    }

    if (!success) {
        if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "setup socket error: (%d) %s", errno, strerror(errno));
        }
    }

    return success;
}



