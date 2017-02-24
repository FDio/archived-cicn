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
 * Implements the platform-specific code for working with an Ethernet interface.
 *
 * Uses the Berkeley Packet Filter (BPF) approach to reading the Ethernet device.
 */

#include <config.h>
#include <stdio.h>
#include <net/bpf.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <ifaddrs.h>

#include <net/ethernet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/forwarder/metis/io/metis_GenericEther.h>
#include <ccnx/forwarder/metis/core/metis_System.h>
#include <parc/algol/parc_Object.h>
#include <ccnx/forwarder/metis/io/metis_Ethernet.h>

struct metis_generic_ether {
    uint16_t ethertype;
    int etherSocket;

    // what size do the read buffers need to be?  ioctl BIOCGBLEN will tell us.
    unsigned etherBufferLength;

    // MTU set on interface when we are created
    unsigned mtu;

    PARCEventBuffer *workBuffer;

    PARCBuffer *macAddress;
    MetisLogger *logger;
};

static void
_metisGenericEther_Destroy(MetisGenericEther **etherPtr)
{
    MetisGenericEther *ether = *etherPtr;

    if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "GenericEther %p destroyed", (void *) ether);
    }

    if (ether->etherSocket > 0) {
        close(ether->etherSocket);
    }

    if (ether->macAddress) {
        parcBuffer_Release(&ether->macAddress);
    }

    metisLogger_Release(&ether->logger);
    parcEventBuffer_Destroy(&ether->workBuffer);
}


// =========================
// PRIVATE API
// =========================

/*
 * Returns the total legth of the good data: the ethernet header plus ccnx packet.
 * If there is a FCS, it will be excluded.
 *
 * PRECONDITION: You have drained off any BPF headers and the first byte
 * of the work buffer points to the first byte of the Ethernet header
 */
static uint16_t
_getFrameLengthFromWorkBuffer(MetisGenericEther *ether)
{
    uint16_t frameLength = 0;

    // read the fixed header
    uint8_t *etherHeader = parcEventBuffer_Pullup(ether->workBuffer, ETHER_HDR_LEN + metisTlv_FixedHeaderLength());

    if (etherHeader) {
        uint8_t *fixedHeader = etherHeader + ETHER_HDR_LEN;
        frameLength = metisTlv_TotalPacketLength(fixedHeader) + ETHER_HDR_LEN;
    }

    return frameLength;
}

/*
 * An attempt to read from the workbuffer to the readbuffer can succeed (ok), fail because
 * the work buffer does not have enough bytes (empty), or an error causes a frame to be
 * discarded (tryagain).
 */
typedef enum {
    ReadWorkBufferResult_Ok,
    ReadWorkBufferResult_Empty,
    ReadWorkBufferResult_TryAgain
} _ReadWorkBufferResult;

/**
 * Parses the work buffer to extract packets
 *
 * The work buffer should be filled in with a set of tuples (bh_hdrlen, frame, pad).
 * The pad extends each packet out to BPF_WORDALIGN.
 *
 * If the CCNxMessage PacketLength says it is larger than the read capture length (caplen),
 * then this is an invalid packet and it will be discarded.  This error will result in a
 * ReadWorkBufferResult_TryAgain condition.
 *
 * struct bpf_hdr {
 *    struct BPF_TIMEVAL bh_tstamp;	 // time stamp
 *    bpf_u_int32        bh_caplen;	 // length of captured portion
 *    bpf_u_int32        bh_datalen; // original length of packet
 *    u_short		     bh_hdrlen;	 // length of bpf header (this struct plus alignment padding)
 * }
 *
 * @param [in] ether An allocated Darwin ethernet.
 * @param [in] readbuffer A user-provided read buffer.
 *
 * @retval ReadWorkBufferResult_Ok A frame was moved from workbuffer to readbuffer
 * @retval ReadWorkBufferResult_Empty There's not enough bytes in the workbuffer
 * @retval ReadWorkBufferResult_TryAgain (likely discard) caused this call to fail, but you should try again
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static _ReadWorkBufferResult
_darwinEthernet_ReadWorkBuffer(MetisGenericEther *ether, PARCEventBuffer *readbuffer)
{
    _ReadWorkBufferResult result = ReadWorkBufferResult_Empty;

    // Make sure we have linear memory for the BPF header
    struct bpf_hdr *bpfHeader = (struct bpf_hdr *) parcEventBuffer_Pullup(ether->workBuffer, sizeof(struct bpf_hdr));

    // make sure we have enough bytes to process the frame
    // bpfHeader may be NULL if there are not sizeof(struct bpf_hdr) bytes available.
    if (bpfHeader && parcEventBuffer_GetLength(ether->workBuffer) >= bpfHeader->bh_hdrlen + bpfHeader->bh_caplen) {
        // (0) Save the needed fields from the bpf header
        // (1) pop off the bpf header
        // (2) move the iovec from work buffer to readBuffer.
        // (3) remove any BPF_WORDALIGN padding to the start of the next packet

        // (0) Save the needed fields from the bpf header
        uint16_t hdrlen = bpfHeader->bh_hdrlen;
        uint32_t caplen = bpfHeader->bh_caplen;

        // (1) pop off the bpf header
        parcEventBuffer_Read(ether->workBuffer, NULL, hdrlen);

        // (1a) Determine the packet length from the fixed header and only transfer that many bytes
        uint16_t packetlen = _getFrameLengthFromWorkBuffer(ether);

        if (packetlen <= caplen) {
            // (2) move the iovec from work buffer to readBuffer.
            parcEventBuffer_ReadIntoBuffer(ether->workBuffer, readbuffer, packetlen);

            // (2a) drain off any trailer (i.e. FCS)
            parcEventBuffer_Read(ether->workBuffer, NULL, caplen - packetlen);

            result = ReadWorkBufferResult_Ok;
        } else {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning)) {
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                                "%s reading fd %d discard packetlen %u greater than caplen %u",
                                __func__, ether->etherSocket, packetlen, caplen);
            }

            // discard all of caplen
            parcEventBuffer_Read(ether->workBuffer, NULL, caplen);

            // tell the caller that this read failed, but they could try again.
            result = ReadWorkBufferResult_TryAgain;
        }

        // (3) remove any BPF_WORDALIGN padding to the start of the next packet
        size_t alignedLength = BPF_WORDALIGN(hdrlen + caplen);
        size_t pad = alignedLength - hdrlen - caplen;
        parcEventBuffer_Read(ether->workBuffer, NULL, pad);
    }
    return result;
}

/**
 * Reads at most a single frame from the BPF queue in the workbuffer
 *
 * Reads at most a single frame from the BPF queue in the workbuffer to the user-provided
 * readbuffer.  Returns true if one frame moved to readbuffer or false otherwise.
 *
 * The frame will be appended to the readbuffer.  If return failure, the readbuffer will
 * be unmodified.
 *
 * The returned frame will not contain a FCS, even if one was read.
 *
 * @param [in] ether An allocated Darwin ethernet.
 * @param [in] readbuffer A user-provided read buffer.
 *
 * @retval true A frame was added to the readbuffer
 * @retval false No frame is available at this time
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_darwinEthernet_WorkBufferToReadBuffer(MetisGenericEther *ether, PARCEventBuffer *readbuffer)
{
    _ReadWorkBufferResult result = ReadWorkBufferResult_Empty;
    do {
        result = _darwinEthernet_ReadWorkBuffer(ether, readbuffer);
    } while (result == ReadWorkBufferResult_TryAgain);

    return (result == ReadWorkBufferResult_Ok);
}

/**
 * Reads from the socket to fill in the work buffer
 *
 * Reads one or more packets from the socket to the work buffer  It will append to the work buffer.
 * The BFP socket is non-blocking.  The BPF interface may return multiple packets in one read
 * that need to be parsed as in _darwinEthernet_ReadWorkBuffer().
 *
 * @param [in] ether The Darwin ethernet interface
 *
 * @retval true We added something to the work buffer
 * @retval false Nothing was read
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_darwinEthernet_ReadSocket(MetisGenericEther *ether)
{
    bool success = false;

    if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "%s reading fd %d bufferLength %u", __func__, ether->etherSocket, ether->etherBufferLength);
    }

    // The buffer we're reading must be exactly ether->etherBufferLength
    // TODO: Fix the parcEventBuffer_ReadFromFileDescriptor call to reserve that space so it's all there.

    uint8_t tempBuffer[ether->etherBufferLength];
    ssize_t read_length = read(ether->etherSocket, tempBuffer, ether->etherBufferLength);
    if (read_length > 0) {
        parcEventBuffer_Append(ether->workBuffer, tempBuffer, read_length);
        if (read_length > 0) {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                "%s read %zd bytes from fd %d",
                                __func__,
                                read_length,
                                ether->etherSocket);
            }
            success = true;
        }
    }

    return success;
}


// open the first available /dev/bpf. Returns
// the fd or -1 on none found
static int
_darwinEthernet_OpenBfpDevice(void)
{
    for (int i = 0; i < 255; i++) {
        char bpfstr[255];
        snprintf(bpfstr, sizeof(bpfstr), "/dev/bpf%d", i);

        int fd = open(bpfstr, O_RDWR);
        if (fd > -1) {
            return fd;
        }

        if (errno == EBUSY) {
            continue;
        }

        if (errno == EBADF) {
            continue;
        }

        // its an error
        return -1;
    }

    errno = ENOENT;
    return -1;
}

/**
 * Set the ioctrl for the BPF socket
 *
 * (a) bind to a specific name
 * (b) The kernel will fill in the source MAC address
 * (c) We will see out-going packets [turn off?]
 * (d) immediate read() [don't wait to queue multiple packets]
 * (e) Include the buffer length
 *
 * Settings BIOCIMMEDIATE means that read() calls will not wait for several packets
 * to accumulate.  It does not, however, guarantee only one packet per read().
 *
 * @param [in] fd BPF socket
 * @param [in] devstr The device name
 *
 * @retval true Set all needed ioctrl
 * @retval false An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_darwinEthernet_SetDeviceOptions(int fd, const char *devstr)
{
    struct ifreq ifr;
    uint32_t on = 1;

    bool success = false;

    // Always null terminate.  If devstr did not fit in ifr_name, then
    // the ioctl call will fail anyway.
    if (devstr != NULL) {
        (void) strncpy(ifr.ifr_name, devstr, IF_NAMESIZE - 1);
        ifr.ifr_name[IF_NAMESIZE - 1] = 0;

        if (ioctl(fd, BIOCSETIF, &ifr)) {
            return false;
        }
    }

    // on = immediate read on packet reception
    if (!ioctl(fd, BIOCIMMEDIATE, &on)) {
        // on = request the buffer length prepended to each packet
        if (!ioctl(fd, BIOCGBLEN, &on)) {
            // set non-blocking
            if (!ioctl(fd, FIONBIO, &on)) {
                success = true;
            }
        }
    }

    return success;
}

/**
 * Create the berkeley packet filter for our ethertype
 *
 * Creates a BPF for our ether type.
 *
 * @param [in,out] ether The MetisGenericEther to modify
 *
 * @retval true success
 * @retval false failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_darwinEthernet_SetFilter(MetisGenericEther *ether)
{
    struct bpf_program filterCode = { 0 };

    // BPF instructions:
    // Load 12 to accumulator (offset of ethertype)
    // Jump Equal to netbyteorder_ethertype 0 instructions, otherwise 1 instruction
    // 0: return a length of -1 (meaning whole packet)
    // 1: return a length of 0 (meaning skip packet)
    struct bpf_insn instructions[] = {
        BPF_STMT(BPF_LD + BPF_H + BPF_ABS,  12),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0801,      0,  1),
        BPF_STMT(BPF_RET + BPF_K,           (u_int) - 1),
        BPF_STMT(BPF_RET + BPF_K,           0),
    };
    //        BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ether->ethertype, 0, 1),

    /* Set the filter */
    filterCode.bf_len = sizeof(instructions) / sizeof(struct bpf_insn);
    filterCode.bf_insns = &instructions[0];

    if (ioctl(ether->etherSocket, BIOCSETF, &filterCode) < 0) {
        return false;
    }

    return true;
}

static bool
_darwinEthernet_SetupReceive(MetisGenericEther *ether, const char *devstr)
{
    // If we cannot open the Ethernet BPF (likely due to permissions), return
    // a soft error so we can move past this failure.
    if ((ether->etherSocket = _darwinEthernet_OpenBfpDevice()) < 0) {
        return false;
    }

    if (!_darwinEthernet_SetDeviceOptions(ether->etherSocket, devstr)) {
        trapUnrecoverableState("error setting options: %s", strerror(errno));
    }


    if (!_darwinEthernet_SetFilter(ether)) {
        trapUnrecoverableState("error setting filter: %s", strerror(errno));
    }

    if (ioctl(ether->etherSocket, BIOCGBLEN, &ether->etherBufferLength)) {
        trapUnrecoverableState("error getting buffer length: %s", strerror(errno));
    }

    return true;
}

/**
 * If the user specified a device name, set the MAC address in ether->macAddress
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] ether An allocated MetisGenericEther
 * @param [in] devstr A C-String of the device name
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_darwinEthernet_SetInterfaceAddress(MetisGenericEther *ether, const char *devstr)
{
    if (devstr) {
        struct ifaddrs *ifaddr;
        int failure = getifaddrs(&ifaddr);
        assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

        struct ifaddrs *next;
        for (next = ifaddr; next != NULL; next = next->ifa_next) {
            if (strcmp(next->ifa_name, devstr) == 0) {
                if (next->ifa_addr->sa_family == AF_LINK) {
                    struct sockaddr_dl *addr_dl = (struct sockaddr_dl *) next->ifa_addr;

                    // addr_dl->sdl_data[12] contains the interface name followed by the MAC address, so
                    // need to offset in to the array past the interface name.
                    PARCBuffer *addr = parcBuffer_Allocate(addr_dl->sdl_alen);
                    parcBuffer_PutArray(addr, addr_dl->sdl_alen, (uint8_t *) &addr_dl->sdl_data[ addr_dl->sdl_nlen]);
                    parcBuffer_Flip(addr);
                    ether->macAddress = addr;

                    // break out of loop and freeifaddrs
                    break;
                }
            }
        }
        freeifaddrs(ifaddr);
    }
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

    // The Darwin generic ether allows a NULL device name, it is used in the unit tests.

    MetisGenericEther *ether = NULL;

    if (metisEthernet_IsValidEthertype(etherType)) {
        ether = parcObject_CreateInstance(MetisGenericEther);
        ether->ethertype = etherType;
        ether->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
        ether->etherSocket = -1; // invalid valid
        ether->workBuffer = parcEventBuffer_Create();
        ether->macAddress = NULL;
        ether->mtu = metisSystem_InterfaceMtu(metis, deviceName);

        _darwinEthernet_SetInterfaceAddress(ether, deviceName);

        bool success = _darwinEthernet_SetupReceive(ether, deviceName);

        if (success) {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Info)) {
                char *str = parcBuffer_ToHexString(ether->macAddress);
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Info, __func__,
                                "GenericEther %p created on device %s (%s) for ethertype 0x%04x fd %d bufferLength %u mtu %u",
                                (void *) ether, deviceName, str, etherType, ether->etherSocket, ether->etherBufferLength, ether->mtu);
                parcMemory_Deallocate((void **) &str);
            }
        } else {
            if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
                metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                                "GenericEther failed to created on device %s for ethertype 0x%04x",
                                deviceName, etherType);
            }

            // this will also null ether
            metisGenericEther_Release(&ether);
        }

        assertTrue(ether->etherBufferLength < 65536, "Buffer length way too big, expected less than 65536 got %u", ether->etherBufferLength);
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


/*
 * Reading a BPF packet will include the BPF header.  Frame may include the FCS.
 */
bool
metisGenericEther_ReadNextFrame(MetisGenericEther *ether, PARCEventBuffer *readbuffer)
{
    assertNotNull(ether, "Parameter ether must be non-null");
    assertNotNull(readbuffer, "Parameter readbuffer must be non-null");

    bool success = false;

    if (metisLogger_IsLoggable(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
        metisLogger_Log(ether->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Workbuffer length %zu", __func__, parcEventBuffer_GetLength(ether->workBuffer));
    }

    // Do we already have something in our work buffer?  If not, try to read something.
    if (parcEventBuffer_GetLength(ether->workBuffer) == 0) {
        _darwinEthernet_ReadSocket(ether);
    }

    success = _darwinEthernet_WorkBufferToReadBuffer(ether, readbuffer);
    return success;
}

bool
metisGenericEther_SendFrame(MetisGenericEther *ether, PARCEventBuffer *buffer)
{
    assertNotNull(ether, "Parameter ether must be non-null");

    size_t length = parcEventBuffer_GetLength(buffer);
    int written = parcEventBuffer_WriteToFileDescriptor(buffer, ether->etherSocket, -1);
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

