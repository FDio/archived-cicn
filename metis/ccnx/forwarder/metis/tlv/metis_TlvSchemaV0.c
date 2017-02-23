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

/**
 * THIS IS A DEPRECATED CLASS. V0 IS NO LONGER IN USE.
 */

#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvExtent.h>

#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_CryptoHasher.h>

#include <ccnx/forwarder/metis/tlv/metis_TlvSchemaV0.h>

typedef struct __attribute__ ((__packed__)) metis_tlv_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t payloadLength;
    uint16_t reserved;
    uint16_t headerLength;
} _MetisTlvFixedHeaderV0;

#define FIXED_HEADER_LEN 8


#define TotalPacketLength(fhPtr) (htons((fhPtr)->payloadLength) + htons((fhPtr)->headerLength) + FIXED_HEADER_LEN)


#define METIS_PACKET_TYPE_INTEREST 0x01
#define METIS_PACKET_TYPE_CONTENT  0x02

// The message type for a Metis control packet
#define METIS_PACKET_TYPE_CONTROL 0xA4

// -----------------------------
// in host byte order

#define T_NAME     0x0000

#define T_HOPLIMIT 0x0002
#define T_INTFRAG  0x0003
#define T_OBJFRAG  0x0004

// inside interest
#define T_KEYID    0x0001
#define T_OBJHASH  0x0002
#define T_SCOPE    0x0003
#define T_INTLIFE  0x0005

// inside an object
#define T_NAMEAUTH 0x0002
#define T_CONTENTS 0x0004
#define T_SIGBLOCK 0x0005
#define T_SIGBITS  0x000E

// inside a CPI
#define T_CPI      0xBEEF

// -----------------------------
// Internal API

static void
_parsePerHopV0(const uint8_t *packet, size_t offset, size_t endHeaders, MetisTlvSkeleton *skeleton)
{
    int foundCount = 0;
    const size_t tl_length = 4;

    // we only parse to the end of the per-hop headers or until we've found
    // the 1 header we want is hoplimit.  Others ignored.
    while (offset < endHeaders && foundCount < 1) {
        MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        uint16_t type = htons(tlv->type);
        uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += tl_length;

        switch (type) {
            case T_HOPLIMIT:
                metisTlvSkeleton_SetHopLimit(skeleton, offset, v_length);
                foundCount++;
                break;

            default:
                break;
        }

        offset += v_length;
    }
}

static void
_parseNameAuth(const uint8_t *packet, size_t offset, size_t endSection, MetisTlvSkeleton *skeleton)
{
    const size_t tl_length = 4;

    while (offset < endSection) {
        MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        uint16_t type = htons(tlv->type);
        uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += tl_length;

        switch (type) {
            case T_KEYID:
                metisTlvSkeleton_SetKeyId(skeleton, offset, v_length);
                return;

            default:
                break;
        }

        offset += v_length;
    }
}

static void
_parseObjectV0(const uint8_t *packet, size_t offset, size_t endMessage, MetisTlvSkeleton *skeleton)
{
    int foundCount = 0;
    const size_t tl_length = 4;

    // skip the opending content object TLV
    offset += 4;

    // parse to the end or until we find the two things we need (name, keyid)
    while (offset < endMessage && foundCount < 2) {
        MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        uint16_t type = htons(tlv->type);
        uint16_t v_length = htons(tlv->length);

        // move past the TL header
        offset += tl_length;

        switch (type) {
            case T_NAME:
                metisTlvSkeleton_SetName(skeleton, offset, v_length);
                foundCount++;
                break;

            case T_NAMEAUTH:
                _parseNameAuth(packet, offset, offset + v_length, skeleton);
                foundCount++;
                break;

            default:
                break;
        }

        offset += v_length;
    }
}

static void
_parseInterestV0(const uint8_t *packet, size_t offset, size_t endMessage, MetisTlvSkeleton *skeleton)
{
    int foundCount = 0;
    const size_t tl_length = sizeof(MetisTlvType);

    // skip the Interest wrapper
    offset += 4;

    // parse to the end or until we find all 5 things (name, keyid, objecthash, scope, interest lifetime)
    while (offset < endMessage && foundCount < 5) {
        MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        uint16_t type = htons(tlv->type);
        uint16_t v_length = htons(tlv->length);

        // skip past the TLV header
        offset += tl_length;

        switch (type) {
            case T_NAME:
                metisTlvSkeleton_SetName(skeleton, offset, v_length);
                foundCount++;
                break;

            case T_KEYID:
                metisTlvSkeleton_SetKeyId(skeleton, offset, v_length);
                foundCount++;
                break;

            case T_OBJHASH:
                metisTlvSkeleton_SetObjectHash(skeleton, offset, v_length);
                foundCount++;
                break;

            case T_INTLIFE:
                metisTlvSkeleton_SetInterestLifetime(skeleton, offset, v_length);
                foundCount++;
                break;

            default:
                break;
        }

        offset += v_length;
    }
}


static void
_parseControlPlaneInterface(const uint8_t *packet, size_t offset, size_t endMessage, MetisTlvSkeleton *skeleton)
{
    int foundCount = 0;
    const size_t tl_length = 4;

    // parse to the end or until we find all 5 things (name, keyid, objecthash, scope, interest lifetime)
    while (offset < endMessage && foundCount < 1) {
        MetisTlvType *tlv = (MetisTlvType *) (packet + offset);
        uint16_t type = htons(tlv->type);
        uint16_t v_length = htons(tlv->length);

        // skip past the TLV header
        offset += tl_length;

        switch (type) {
            case T_CPI:
                metisTlvSkeleton_SetCPI(skeleton, offset, v_length);
                foundCount++;
                break;

            default:
                break;
        }

        offset += v_length;
    }
}

static PARCCryptoHash *
_computeHash(const uint8_t *packet, size_t offset, size_t endMessage)
{
    PARCCryptoHasher *hasher = parcCryptoHasher_Create(PARCCryptoHashType_SHA256);
    parcCryptoHasher_Init(hasher);
    parcCryptoHasher_UpdateBytes(hasher, packet + offset, endMessage - offset);
    PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);
    parcCryptoHasher_Release(&hasher);
    return hash;
}

// ==================
// TlvOps functions

static PARCBuffer *
_encodeControlPlaneInformation(const CCNxControl *cpiControlMessage)
{
    PARCJSON *json = ccnxControl_GetJson(cpiControlMessage);
    char *str = parcJSON_ToCompactString(json);

    // include +1 because we need the NULL byte
    size_t len = strlen(str) + 1;

    size_t packetLength = sizeof(_MetisTlvFixedHeaderV0) + sizeof(MetisTlvType) + len;
    PARCBuffer *packet = parcBuffer_Allocate(packetLength);

    _MetisTlvFixedHeaderV0 hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = 0;
    hdr.packetType = METIS_PACKET_TYPE_CONTROL;
    hdr.payloadLength = htons(len + sizeof(MetisTlvType));

    parcBuffer_PutArray(packet, sizeof(hdr), (uint8_t *) &hdr);

    MetisTlvType tlv = { .type = htons(T_CPI), .length = htons(len) };
    parcBuffer_PutArray(packet, sizeof(tlv), (uint8_t *) &tlv);

    parcBuffer_PutArray(packet, len, (uint8_t *) str);

    parcMemory_Deallocate((void **) &str);
    return parcBuffer_Flip(packet);
}


static PARCCryptoHash *
_computeContentObjectHash(const uint8_t *packet)
{
    assertNotNull(packet, "Parameter packet must be non-null");

    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;
    if (hdr->packetType == METIS_PACKET_TYPE_CONTENT) {
        size_t headerLength = htons(hdr->headerLength);
        size_t endHeaders = FIXED_HEADER_LEN + headerLength;
        size_t endPacket = TotalPacketLength(hdr);

        return _computeHash(packet, endHeaders, endPacket);
    }

    return NULL;
}

static bool
_isPacketTypeInterest(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_INTEREST);
}

static bool
_isPacketTypeInterestReturn(const uint8_t *packet)
{
    return false;
}

static bool
_isPacketTypeContentObject(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_CONTENT);
}

static bool
_isPacketTypeControl(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;
    return (hdr->packetType == METIS_PACKET_TYPE_CONTROL);
}

static bool
_isPacketTypeHopByHopFragment(const uint8_t *packet)
{
    // does not exist for version 0 packets
    return false;
}

static size_t
_fixedHeaderLength(const uint8_t *packet)
{
    return sizeof(_MetisTlvFixedHeaderV0);
}

static size_t
_totalHeaderLength(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;

    return htons(hdr->headerLength) + sizeof(_MetisTlvFixedHeaderV0);
}

static size_t
_totalPacketLength(const uint8_t *packet)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) packet;

    return TotalPacketLength(hdr);
}

static bool
_parse(MetisTlvSkeleton *skeleton)
{
    _MetisTlvFixedHeaderV0 *hdr = (_MetisTlvFixedHeaderV0 *) metisTlvSkeleton_GetPacket(skeleton);
    size_t headerLength = htons(hdr->headerLength);

    size_t endHeaders = FIXED_HEADER_LEN + headerLength;
    size_t endPacket = TotalPacketLength(hdr);

    trapUnexpectedStateIf(hdr->version != 0, "Version not 0");

    switch (hdr->packetType) {
        case METIS_PACKET_TYPE_INTEREST:
            _parsePerHopV0(metisTlvSkeleton_GetPacket(skeleton), FIXED_HEADER_LEN, endHeaders, skeleton);
            _parseInterestV0(metisTlvSkeleton_GetPacket(skeleton), endHeaders, endPacket, skeleton);
            break;

        case METIS_PACKET_TYPE_CONTENT:
            _parsePerHopV0(metisTlvSkeleton_GetPacket(skeleton), FIXED_HEADER_LEN, endHeaders, skeleton);
            _parseObjectV0(metisTlvSkeleton_GetPacket(skeleton), endHeaders, endPacket, skeleton);
            break;

        case METIS_PACKET_TYPE_CONTROL:
            _parseControlPlaneInterface(metisTlvSkeleton_GetPacket(skeleton), endHeaders, endPacket, skeleton);
            break;

        default:
            break;
    }

    return true;
}

const MetisTlvOps MetisTlvSchemaV0_Ops = {
    .parse                         = _parse,
    .computeContentObjectHash      = _computeContentObjectHash,
    .encodeControlPlaneInformation = _encodeControlPlaneInformation,
    .fixedHeaderLength             = _fixedHeaderLength,
    .totalHeaderLength             = _totalHeaderLength,
    .totalPacketLength             = _totalPacketLength,
    .isPacketTypeInterest          = _isPacketTypeInterest,
    .isPacketTypeContentObject     = _isPacketTypeContentObject,
    .isPacketTypeInterestReturn    = _isPacketTypeInterestReturn,
    .isPacketTypeControl           = _isPacketTypeControl,
    .isPacketTypeHopByHopFragment  = _isPacketTypeHopByHopFragment,
};

