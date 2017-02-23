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
 * @file interest_nameA_badcrc32c.h
 * @brief Interest with CRC validation
 *
 * Has incorrect CRC32C payload
 *
 */

#ifndef testdata_interest_nameA_badcrc32c_h
#define testdata_interest_nameA_badcrc32c_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_InterestSchema.h>

/**
 * A well formed interest with only a name
 */

__attribute__((unused))
static uint8_t v1_interest_nameA_badcrc32c[] = {
    0x01, 0x00, 0x00, 41,       // ver = 1, type = interest, length = 65
    0x20, 0x00, 0x00, 24,       // HopLimit = 31, reserved = 0, header length = 24
    // ------------------------
    0x00, 0x03, 0x00, 0x0C,     // Interest Fragment
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,     // fragment 0x0102030405060708
    0x05, 0xDC, 0x00, 0x00,     // MTU 1500, fragcnt 0, fragnum 0
    // ------------------------
    0x00, 0x01, 0x00, 0x15,     // type = interest, length = 21
    // ------------------------
    0x00, 0x00, 0x00, 0x11,     // type = name, length = 17
    0x00, 0x03, 0x00, 0x05,     // type = binary, length = 5
    'h',  'e',  'l',  'l',      // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04,     // type = app, length = 4
    'o',  'u',  'c',  'h',      // "ouch"
    // ------------------------
    0x00, 0x03, 0x00, 4,        // validation alg, length = 4
    0x00, 0x02, 0x00, 0x00,     // CRC32C
    // ------------------------
    0x00, 0x04, 0x00, 4,        // validation payload
    0x00, 0x00, 0x00, 0x00      // invalid CRC32C
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_interest_nameA_badcrc32c)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_E2EFRAG,           .bodyManifest = false, .extent = { 12, 12 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_INTEREST,          .bodyManifest = true,  .extent = { 20, 21 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_NAME,              .bodyManifest = true,  .extent = { 24, 17 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_ValidationAlg,     .bodyManifest = true,  .extent = { 45, 4  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_ValidationPayload, .bodyManifest = true,  .extent = { 53, 4  } },
    { .wellKnownType = false, .indexOrKey = T_INVALID,                         .extent       = { 0,   0 } },
};

#define v1_interest_nameA_badcrc32c_truthTable TABLEENTRY(v1_interest_nameA_badcrc32c, TLV_ERR_NO_ERROR)

#define v1_interest_nameA_badcrc32c_URI "lci:/3=hello/0xf000=ouch"

#endif
