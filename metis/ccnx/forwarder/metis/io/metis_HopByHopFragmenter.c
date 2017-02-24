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
 * See https://www.ietf.org/proceedings/interim/2015/03/22/icnrg/slides/slides-interim-2015-icnrg-2-3.pptx
 *
 * B flag - indicates the start of a fragment
 * E flag - indicates the end of a fragment (may be in same frame as B frame)
 * I flag - an Idle frame (may only occur between E and B frames).
 * X flag - extended format (not supported)
 *
 * in the basic protocol that we implement, there is a 20-bit sequence number
 *
 */

#include <config.h>
#include <stdio.h>

#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/io/metis_HopByHopFragmenter.h>

#include <parc/algol/parc_EventBuffer.h>

#include <parc/concurrent/parc_RingBuffer_1x1.h>

/*
 * Complete header for the Basic Encoding in a V1 FixedHeader. The blob[3] array
 * holds the protocol header fields.  See the macros below for accessing specific fields.
 * The tlvType and tlvLength begin the container to hold the fragment payload.
 */
typedef struct hopbyhop_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t packetLength;
    uint8_t blob[3];
    uint8_t headerLength;
    uint16_t tlvType;
    uint16_t tlvLength;
} __attribute__((packed)) _HopByHopHeader;

// These two values are also defined in metis_TlvShcemaV1.c
#define METIS_PACKET_TYPE_HOPFRAG 4
#define T_HOPFRAG_PAYLOAD  0x0005

/*
 * Mask a uint32_t down to the 20-bit sequence number
 */
#define SEQNUM_MASK ((uint32_t) (0x000FFFFF))

/*
 * This will right-pad the seqnum out to 32 bits.  By filling up a uint32_t it allows
 * us to use 2's compliment math to compare two sequence numbers rather than the cumbersome
 * multiple branches required by the method outlined in RFC 1982.
 * We use a 20-bit sequence number, so need to shift 12 bits to the left.
 */
#define SEQNUM_SHIFT 12

/*
 * The X bit value in the top byte of the header
 */
#define XMASK 0x80

/*
 * The B bit value in the top byte of the header
 */
#define BMASK 0x40

/*
 * The E bit value in the top byte of the header
 */
#define EMASK 0x20

/*
 * The I bit value in the top byte of the header
 */
#define IMASK 0x10

/*
 * Mask out the flags from the top byte of the header
 */
#define _hopByHopHeader_GetFlags(header) ((header)->blob[0] & 0xF0)

/*
 * non-zero if the X flag is set
 */
#define _hopByHopHeader_GetXFlag(header) ((header)->blob[0] & XMASK)

/*
 * non-zero if the B flag is set
 */
#define _hopByHopHeader_GetBFlag(header) ((header)->blob[0] & BMASK)

/*
 * non-zero if the E flag is set
 */
#define _hopByHopHeader_GetEFlag(header) ((header)->blob[0] & EMASK)

/*
 * non-zero if the I flag is set
 */
#define _hopByHopHeader_GetIFlag(header) ((header)->blob[0] & IMASK)

/*
 * Sets the X flag in the header
 */
#define _hopByHopHeader_SetXFlag(header) ((header)->blob[0] |= XMASK)

/*
 * Sets the B flag in the header
 */
#define _hopByHopHeader_SetBFlag(header) ((header)->blob[0] |= BMASK)

/*
 * Sets the E flag in the header
 */
#define _hopByHopHeader_SetEFlag(header) ((header)->blob[0] |= EMASK)

/*
 * Sets the I flag in the header
 */
#define _hopByHopHeader_SetIFlag(header) ((header)->blob[0] |= IMASK)

typedef enum {
    _ParserState_Idle,  // not parsing anything
    _ParserState_Busy,  // we have received a B but not an E
} _ParserState;

struct metis_hopbyhop_fragment {
    MetisLogger *logger;
    unsigned mtu;

    // The next expected sequence number (i.e. compare then increment)
    uint32_t nextReceiveFragSequenceNumber;

    // The next seqnum to use in out-going message (i.e. use then increment)
    uint32_t nextSendFragSequenceNumber;

    unsigned receiveQueueCapacity;
    unsigned sendQueueCapacity;
    PARCRingBuffer1x1 *receiveQueue;
    PARCRingBuffer1x1 *sendQueue;

    // We are only ever reassembling one packet at a time
    PARCEventBuffer *currentReceiveBuffer;

    // these two are set from the "B" fragment so a reassembled frame
    // will have the time and ingress port of the first fragment.
    MetisTicks currentReceiveBufferStartTicks;
    unsigned currentReceiveBufferIngressId;

    // Determines if we are currently reassembling a fragment
    _ParserState parserState;
};

static uint32_t
_hopByHopHeader_GetSeqnum(const _HopByHopHeader *header)
{
    uint32_t seqnum = ((uint32_t) header->blob[0] & 0x0F) << 16 | (uint32_t) header->blob[1] << 8 | header->blob[2];
    return seqnum;
}

static void __attribute__((unused))
_hopByHopHeader_SetSeqnum(_HopByHopHeader *header, uint32_t seqnum)
{
    header->blob[2] = seqnum & 0xFF;
    header->blob[1] = (seqnum >> 8) & 0xFF;

    header->blob[0] &= 0xF0;
    header->blob[0] |= (seqnum >> 16) & 0x0F;
}

static void
_ringBufferDestroyer(void **ptr)
{
    MetisMessage *message = *ptr;
    metisMessage_Release(&message);
    *ptr = NULL;
}

/**
 * Compares sequence numbers as per RFC 1982
 *
 * Handles wrap-around using the 1/2 buffer rule as per RFC 1982.  The indefinate state
 * at exactly the middle is handled by having 2^(N-1)-1 greater than and 2^(N-1) less than.
 *
 * @param [in] a The first sequence number
 * @param [in] b The second sequence number
 *
 * @return negative If a < b
 * @return 0 If a == b
 * @return positive if a > b
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static int
_compareSequenceNumbers(uint32_t a, uint32_t b)
{
    // shift the numbers so they take up a full 32-bits and then use 2's compliment
    // arithmatic to determine the ordering

    a <<= SEQNUM_SHIFT;
    b <<= SEQNUM_SHIFT;

    int32_t c = (int32_t) (a - b);
    return c;
}

static uint32_t
_incrementSequenceNumber(const uint32_t seqnum, const uint32_t mask)
{
    uint32_t result = (seqnum + 1) & mask;
    return result;
}

static uint32_t
_nextSendSequenceNumber(MetisHopByHopFragmenter *fragmenter)
{
    uint32_t seqnum = fragmenter->nextSendFragSequenceNumber;
    fragmenter->nextSendFragSequenceNumber = _incrementSequenceNumber(fragmenter->nextSendFragSequenceNumber, SEQNUM_MASK);
    return seqnum;
}

// ===================================================
// RECEIVE PROCESS

static void
_resetParser(MetisHopByHopFragmenter *fragmenter)
{
    // throw away the re-assembly buffer and reset state to Idle
    parcEventBuffer_Read(fragmenter->currentReceiveBuffer, NULL, UINT32_MAX);
    fragmenter->parserState = _ParserState_Idle;
}

/**
 * Apply the sequence number rules
 *
 * a) If the sequence number is in order, no action.
 * b) If the sequence number is out of order, reset the parser.
 * c) Update the next expected sequence number to be this packet's seqnum + 1.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 * @param [in] fixedHeader The packet's fixed header
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_applySequenceNumberRules(MetisHopByHopFragmenter *fragmenter, const _HopByHopHeader *fixedHeader)
{
    uint32_t segnum = _hopByHopHeader_GetSeqnum(fixedHeader);

    int compare = _compareSequenceNumbers(segnum, fragmenter->nextReceiveFragSequenceNumber);

    if (compare == 0) {
        // In order
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Fragmenter %p in-order seqnum %u",
                        (void *) fragmenter, segnum);
    } else if (compare < 0) {
        // it is an old sequence number
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Info, __func__,
                        "Fragmenter %p out-of-order seqnum %u expecting %u",
                        (void *) fragmenter, segnum, fragmenter->nextReceiveFragSequenceNumber);
        _resetParser(fragmenter);
    } else if (compare > 0) {
        // lost packets
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Info, __func__,
                        "Fragmenter %p out-of-order seqnum %u expecting %u",
                        (void *) fragmenter, segnum, fragmenter->nextReceiveFragSequenceNumber);
        _resetParser(fragmenter);
    }

    // the next seqnum we expect will be 1 after what we just received.  For example, if we lost packets
    // this will put us back in line with the new series.
    fragmenter->nextReceiveFragSequenceNumber = _incrementSequenceNumber(segnum, SEQNUM_MASK);
}

/*
 * We've reach the END fragment of the reassembly buffer.
 * 1) Make a metis message out of the reassembly buffer,
 * 2) put the message in the receive queue (discard if queue full)
 * 3) allocate a new reassembly buffer
 * 4) reset the parser
 */
static void
_finalizeReassemblyBuffer(MetisHopByHopFragmenter *fragmenter)
{
    // This takes ownership of fragmenter->currentReceiveBuffer
    MetisMessage *reassembled = metisMessage_CreateFromBuffer(fragmenter->currentReceiveBufferIngressId,
                                                              fragmenter->currentReceiveBufferStartTicks,
                                                              fragmenter->currentReceiveBuffer,
                                                              fragmenter->logger);

    if (reassembled) {
        bool success = parcRingBuffer1x1_Put(fragmenter->receiveQueue, reassembled);
        if (success) {
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                            "Fragmenter %p putting reassembed message %p in receive queue",
                            (void *) fragmenter, (void *) reassembled);
        } else {
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "Fragmenter %p failed to put reassembled message in receive queue, dropping",
                            (void *) fragmenter);

            metisMessage_Release(&reassembled);
        }

        fragmenter->currentReceiveBuffer = parcEventBuffer_Create();
    } else {
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                        "Fragmenter %p failed to parse reassembled packet to MetisMessage, dropping",
                        (void *) fragmenter);
    }

    _resetParser(fragmenter);
}

static void
_appendFragmentToReassemblyBuffer(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message)
{
    size_t length = metisMessage_AppendFragmentPayload(message, fragmenter->currentReceiveBuffer);
    metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                    "Fragmenter %p append %zu bytes to reassembly buffer",
                    (void *) fragmenter, length);
}

/*
 * Parser is in Idle state.  We can only accept a B or BE frame.
 * 1) If B frame:
 * 1a) append to current receive buffer
 * 1b) set parser state to Busy
 * 1c) set the currentReceiveBufferStartTicks
 * 1d) set the currentReceiveBufferIngressId
 * 2) If BE frame, do B actions and finalize it (side effect: will reset state to Idle)
 * 3) Otherwise ignore it
 *
 * Precondition: You know that the parser is in the Idle state
 */
static void
_receiveInIdleState(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message, const _HopByHopHeader *fixedHeader)
{
    trapUnexpectedStateIf(fragmenter->parserState != _ParserState_Idle,
                          "Parser in wrong state, expected %d got %d",
                          _ParserState_Idle, fragmenter->parserState);

    if (_hopByHopHeader_GetBFlag(fixedHeader)) {
        // start a new packet
        fragmenter->currentReceiveBufferStartTicks = metisMessage_GetReceiveTime(message);
        fragmenter->currentReceiveBufferIngressId = metisMessage_GetIngressConnectionId(message);
        fragmenter->parserState = _ParserState_Busy;

        _appendFragmentToReassemblyBuffer(fragmenter, message);

        if (_hopByHopHeader_GetEFlag(fixedHeader)) {
            // it is also the last fragment
            _finalizeReassemblyBuffer(fragmenter);
        }
    } else if (_hopByHopHeader_GetIFlag(fixedHeader)) {
        // nothing to do
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Fragmenter %p idle frame, ignorning",
                        (void *) fragmenter);
    } else {
        // nothing we can do with this frame
        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                        "Fragmenter %p received bad header flags (0x%02X), ignorning",
                        (void *) fragmenter, _hopByHopHeader_GetFlags(fixedHeader));
    }
}

/*
 * In the Busy state, we can only accept a packet with no flag (middle) or end flag (end of packet).
 * Anything else is an error and will cause the parser to be reset.
 *
 * 1) If no flags
 * 1a) append to reassembly buffer
 * 2) If E flag
 * 2a) append to reassembly buffer
 * 2b) finalize the buffer (side effect: will reset the parser and place in receive queue)
 * 3) Otherwise, its an error, reset the parser
 *
 * PRECONDITION: you know the packet is in-order relative to the assembly buffer.
 * This is handled by calling _applySequenceNumberRules() before this function.
 * PRECONDITION: you know the parser is in the Busy state.
 */
static void
_receiveInBusyState(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message, const _HopByHopHeader *fixedHeader)
{
    trapUnexpectedStateIf(fragmenter->parserState != _ParserState_Busy,
                          "Parser in wrong state, expected %d got %d",
                          _ParserState_Busy, fragmenter->parserState);

    if (_hopByHopHeader_GetFlags(fixedHeader) == 0) {
        // It's a middle packet

        _appendFragmentToReassemblyBuffer(fragmenter, message);
    } else if (_hopByHopHeader_GetEFlag(fixedHeader)) {
        // It is the last fragment

        _appendFragmentToReassemblyBuffer(fragmenter, message);

        _finalizeReassemblyBuffer(fragmenter);
    } else {
        // nothing we can do with this frame, and it's a state machine error

        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                        "Fragmenter %p received invalid headers (0x%02X) in Busy state, resetting",
                        (void *) fragmenter, _hopByHopHeader_GetFlags(fixedHeader));
        _resetParser(fragmenter);
    }
}

/**
 * Receives a fragment and applies the protocol algorithm
 *
 * 1) A receiver maintains one reassembly queue per peer.
 * 2) Discard Idle fragments.
 * 3) Discard fragments until a 'B' fragment is received.  Store the received sequence number for this sender.
 * 4) If an out-of-order fragment is received next, discard the reassembly buffer and go to step (2).
 * 5) Continue receiving in-order fragments until the first 'E’ fragment.  At this time, the fragmented
 *    packet is fully re-assembled and may be passed on to the next layer.
 * 6) The receiver cannot assume it will receive the 'E' fragment or a subsequence 'I' frame, so it should
 *    use a timeout mechanism appropriate to the link to release allocated memory resources.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 * @param [in] message The fragment packet
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static void
_receiveFragment(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message)
{
    const _HopByHopHeader *fixedHeader = (const _HopByHopHeader *) metisMessage_FixedHeader(message);

    _applySequenceNumberRules(fragmenter, fixedHeader);

    // ======
    // Now apply the receiver rules


    switch (fragmenter->parserState) {
        case _ParserState_Idle:
            _receiveInIdleState(fragmenter, message, fixedHeader);
            break;

        case _ParserState_Busy:
            _receiveInBusyState(fragmenter, message, fixedHeader);
            break;

        default:
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "Fragmenter %p Invalid parser state, discarding current buffer and resetting to Idle: %d",
                            (void *) fragmenter, fragmenter->parserState);
            break;
    }
}

// ===================================================
// SEND PROCESS

/**
 * Fragments a message and puts all the fragments in the send queue
 *
 * Splits up the message in to fragments.  The frist fragment will have the B flag and
 * the last fragment will have the E flag.  If the message fits in one fragment, it will
 * have both the BE flags.  Middle messages have no flags.
 *
 * @param [in] fragmenter An allocated MetisHopByHopFragmenter
 * @param [in] message The message to fragment down to MTU size
 *
 * @return true Message was fragmented and all fragments put on send queue
 * @return false Error durring fragmentation (likley full send queue)
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static bool
_sendFragments(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message)
{
    const size_t length = metisMessage_Length(message);
    size_t offset = 0;

    const size_t maxPayload = fragmenter->mtu - sizeof(_HopByHopHeader);

    _HopByHopHeader header;
    memset(&header, 0, sizeof(header));
    _hopByHopHeader_SetBFlag(&header);

    while (offset < length) {
        size_t payloadLength = maxPayload;
        const size_t remaining = length - offset;

        if (remaining < maxPayload) {
            payloadLength = remaining;
            _hopByHopHeader_SetEFlag(&header);
        }

        const size_t packetLength = sizeof(_HopByHopHeader) + payloadLength;

        header.version = 1;
        header.packetType = METIS_PACKET_TYPE_HOPFRAG;
        header.packetLength = htons(packetLength);
        header.headerLength = 8;
        header.tlvType = htons(T_HOPFRAG_PAYLOAD);
        header.tlvLength = htons(payloadLength);

        uint32_t seqnum = _nextSendSequenceNumber(fragmenter);
        _hopByHopHeader_SetSeqnum(&header, seqnum);

        MetisMessage *fragment = metisMessage_Slice(message, offset, payloadLength, sizeof(header), (uint8_t *) &header);
        bool goodput = parcRingBuffer1x1_Put(fragmenter->sendQueue, fragment);
        if (!goodput) {
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                            "Fragmenter %p message %p send queue full offset %zu length %zu",
                            (void *) fragmenter, (void *) message, offset, payloadLength);
            metisMessage_Release(&fragment);
            break;
        }

        metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                        "Fragmenter %p message %p send queue fragment %p offset %zu length %zu",
                        (void *) fragmenter, (void *) message, (void *) fragment, offset, payloadLength);

        offset += payloadLength;

        memset(&header, 0, sizeof(header));
    }

    return (offset == length);
}

// ===================================================

MetisHopByHopFragmenter *
metisHopByHopFragmenter_Create(MetisLogger *logger, unsigned mtu)
{
    MetisHopByHopFragmenter *fragmenter = parcMemory_Allocate(sizeof(MetisHopByHopFragmenter));
    if (fragmenter) {
        fragmenter->nextReceiveFragSequenceNumber = 0;
        fragmenter->nextSendFragSequenceNumber = 0;

        fragmenter->logger = metisLogger_Acquire(logger);
        fragmenter->mtu = mtu;
        fragmenter->receiveQueueCapacity = 128;  // this is a many-to-one queue, so not too big
        fragmenter->sendQueueCapacity = 2048;    // this is a one-to-many queue, so bigger (e.g. 64 KB in to 1KB payloads)
        fragmenter->receiveQueue = parcRingBuffer1x1_Create(fragmenter->receiveQueueCapacity, _ringBufferDestroyer);
        fragmenter->sendQueue = parcRingBuffer1x1_Create(fragmenter->sendQueueCapacity, _ringBufferDestroyer);
        fragmenter->currentReceiveBuffer = parcEventBuffer_Create();
        fragmenter->parserState = _ParserState_Idle;
    }
    return fragmenter;
}

void
metisHopByHopFragmenter_Release(MetisHopByHopFragmenter **fragmenterPtr)
{
    MetisHopByHopFragmenter *fragmenter = *fragmenterPtr;
    parcEventBuffer_Destroy(&fragmenter->currentReceiveBuffer);
    parcRingBuffer1x1_Release(&fragmenter->sendQueue);
    parcRingBuffer1x1_Release(&fragmenter->receiveQueue);
    metisLogger_Release(&fragmenter->logger);
    parcMemory_Deallocate((void **) fragmenterPtr);
}

bool
metisHopByHopFragmenter_Receive(MetisHopByHopFragmenter *fragmenter, const MetisMessage *message)
{
    if (metisMessage_GetType(message) == MetisMessagePacketType_HopByHopFrag) {
        _receiveFragment(fragmenter, message);
    } else {
        // put the whole thing on the output queue
        MetisMessage *copy = metisMessage_Acquire(message);
        bool putSuccess = parcRingBuffer1x1_Put(fragmenter->receiveQueue, copy);
        if (!putSuccess) {
            metisMessage_Release(&copy);
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                            "Failed to add message %p to receive queue", (void *) message);
        } else {
            if (metisLogger_IsLoggable(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                "Add message %p to receive queue", (void *) message);
            }
        }
    }

    // the maximum remaining is its capacity - 1
    return (parcRingBuffer1x1_Remaining(fragmenter->receiveQueue) < fragmenter->receiveQueueCapacity - 1);
}

MetisMessage *
metisHopByHopFragmenter_PopReceiveQueue(MetisHopByHopFragmenter *fragmenter)
{
    MetisMessage *message = NULL;
    parcRingBuffer1x1_Get(fragmenter->receiveQueue, (void **) &message);
    return message;
}

bool
metisHopByHopFragmenter_Send(MetisHopByHopFragmenter *fragmenter, MetisMessage *message)
{
    bool success = false;

    // If the packet will fit in the MTU without fragmentation, do not use fragmentation
    if (metisMessage_Length(message) > fragmenter->mtu) {
        success = _sendFragments(fragmenter, message);
    } else {
        MetisMessage *copy = metisMessage_Acquire(message);
        success = parcRingBuffer1x1_Put(fragmenter->sendQueue, copy);
        if (!success) {
            metisMessage_Release(&copy);
            metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Warning, __func__,
                            "Failed to add message %p to send queue", (void *) message);
        } else {
            if (metisLogger_IsLoggable(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                metisLogger_Log(fragmenter->logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                "Add message %p to send queue", (void *) message);
            }
        }
    }

    return success;
}

MetisMessage *
metisHopByHopFragmenter_PopSendQueue(MetisHopByHopFragmenter *fragmenter)
{
    MetisMessage *message = NULL;
    parcRingBuffer1x1_Get(fragmenter->sendQueue, (void **) &message);
    return message;
}

