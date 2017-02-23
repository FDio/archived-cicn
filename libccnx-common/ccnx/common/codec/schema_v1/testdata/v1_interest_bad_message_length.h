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
 * @file v1_interest_all_fields.h
 * @brief A hand-encoded v1 interest in wireformat with all Interest fields.
 *
 * The Interest TLV length goes beyond the end of the packet
 *
 */

#ifndef CCNx_Common_v1_interest_bad_message_length_h
#define CCNx_Common_v1_interest_bad_message_length_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_InterestSchema.h>

/**
 * A well formed interest with all allowed Interest fields
 */
__attribute__((unused))
static uint8_t v1_interest_bad_message_length[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 14,       // HopLimit = 31, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 13,       // type = interest, length = 13 (1 byte too far)
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x03, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o', 'l',       // "cool"
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_interest_bad_message_length)[] =
{
    { .wellKnownType = false, .indexOrKey = T_INVALID, .extent = { 0, 0 } },
};

#define v1_interest_bad_message_length_truthTable TABLEENTRY(v1_interest_bad_message_length, TLV_ERR_TOO_LONG)

#endif
