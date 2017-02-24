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
 * <#Detailed Description#>
 *
 */

#ifndef CCNx_Common_v1_interest_all_fields_h
#define CCNx_Common_v1_interest_all_fields_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_InterestSchema.h>

/**
 * A well formed interest with all allowed Interest fields
 */
__attribute__((unused))
static uint8_t v1_interest_all_fields[] = {
    0x01, 0x00, 0x00, 156,      // ver = 1, type = interest, length = 156
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 138,      // type = interest, length = 138
    // ------------------------
    0x00, 0x00, 0x00, 45,       // type = name, length = 45
    0x00, 0x03, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
    // ----- segment 2 --------
    0x00, 0x02, 0x00, 33,       // type = payload id, length = 33
    0x01, // payloadID type = sha256
    0x89, 0x87, 0x69, 0xfc,     // hash bytes based on payload
    0x8c, 0xff, 0x16, 0xff,
    0x3d, 0xfc, 0xe7, 0xfa,
    0x02, 0xd2, 0x6d, 0x26,
    0xf0, 0x91, 0x86, 0x27,
    0xcf, 0x18, 0xc1, 0x9b,
    0x0b, 0x5f, 0xe3, 0x93,
    0xce, 0x1a, 0xa3, 0x56,
    // ------------------------
    0x00, 0x02, 0x00, 36,       // type = keyid restriction, length = 36
    0x00, 0x01, 0x00, 0x20,     // SHA256 hash, length 32
    0xa0, 0xa1, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf,
    0xa0, 0xa1, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf,
    // ------------------------
    0x00, 0x03, 0x00, 36,       // type = hash restriction, length = 36
    0x00, 0x01, 0x00, 0x20,     // SHA256 hash, length 32
    0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb,
    0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb,
    0xcc, 0xcd, 0xce, 0xcf,
    // ------------------------
    0x00, 0x01, 0x00, 5,        // type = payload, length = 5
    0xD0, 0xD1, 0xD2, 0xD3,
    0xD4,
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_interest_all_fields)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_LIFETIME, .bodyManifest = false, .extent = { 12,  2   } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_INTEREST, .bodyManifest = true,  .extent = { 18,  138 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_NAME,     .bodyManifest = true,  .extent = { 22,  45  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_KEYID,    .bodyManifest = true,  .extent = { 71,  36  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_OBJHASH,  .bodyManifest = true,  .extent = { 111, 36  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_INT_PAYLOAD,  .bodyManifest = true,  .extent = { 151, 5   } },
    { .wellKnownType = false, .indexOrKey = T_INVALID,                .extent       = { 0,   0 } },
};

#define v1_interest_all_fields_truthTable TABLEENTRY(v1_interest_all_fields, TLV_ERR_NO_ERROR)

#define v1_interest_all_fields_URI "lci:/3=cool/2=\x01\x89\x87\x69\xfc\x8c\xff\x16\xff\x3d\xfc\xe7\xfa\x02\xd2\x6d\x26\xf0\x91\x86\x27\xcf\x18\xc1\x9b\x0b\x5f\xe3\x93\xce\x1a\xa3\x56"
#define v1_interest_all_fields_Lifetime 0xEAEB
#endif
