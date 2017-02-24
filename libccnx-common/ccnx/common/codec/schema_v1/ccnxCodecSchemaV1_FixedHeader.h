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
 * @file ccnxCodecSchemaV1_FixedHeader.h
 * @brief common definitions and functions for the FixedHeader
 *
 * See ccnxCodecSchemaV1_Packet.h for an overview of the version 1 codec
 *
 * This is the one file you need to include for all FixedHeader operations.  It will
 * include all the Decoders and Encoders.
 *
 */

#ifndef TransportRTA_ccnxCodecSchemaV1_FixedHeader_h
#define TransportRTA_ccnxCodecSchemaV1_FixedHeader_h

typedef struct __attribute__ ((__packed__)) rta_tlv_schema_v1_fixed_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t packetLength;
    uint8_t reserved[3];
    uint8_t headerLength;
} CCNxCodecSchemaV1FixedHeader;

typedef struct __attribute__ ((__packed__)) rta_tlv_schema_v1_interest_header {
    uint8_t version;
    uint8_t packetType;
    uint16_t packetLength;
    uint8_t hopLimit;
    uint8_t returnCode;
    uint8_t flags;
    uint8_t headerLength;
} CCNxCodecSchemaV1InterestHeader;


#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderDecoder.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_FixedHeaderEncoder.h>
#endif // TransportRTA_ccnxCodecSchemaV1_FixedHeader_h
