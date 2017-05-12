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
 * @file v1_content_nameless_nosig.h
 * @brief A v1 content object without a name
 *
 */

#ifndef v1_content_nameless_nosig_h
#define v1_content_nameless_nosig_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_ContentObjectSchema.h>

/**
 * A well formed nameless V1 Content Object
 */
__attribute__((unused))
static uint8_t v1_content_nameless_nosig[] = {
    0x01, 0x01, 0x00, 0x31,         // ver = 1, type = content object, length = 0x31
    0x00, 0x00, 0x00, 0x08,         // HopLimit = 0, reserved = 0, header length = 8
    // ------------------------
    0x00, 0x02, 0x00, 37,           // type = content object, length = 37
    // ------------------------
    0x00, 0x05, 0x00, 1,            // PayloadType
    1,    // type 1 = key
    0x00, 0x06, 0x00, 0x08,         // expiry time in msec
    0x00, 0x00, 0x01, 0x43,         // 1,388,534,400,000 msec
    0x4B, 0x19, 0x84, 0x00,
    0x00, 0x19, 0x00, 4,            // end chunk number
    0x06, 0x05, 0x04, 0x03,
    // ------------------------
    0x00, 0x01, 0x00, 8,            // payload, length = 8
    0x73, 0x75, 0x72, 0x70,
    0x72, 0x69, 0x73, 0x65,
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_content_nameless_nosig)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_CONTENTOBJECT, .bodyManifest = true, .extent = { 12, 37 } }, //12 //17
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_PAYLOADTYPE,   .bodyManifest = true, .extent = { 17, 1  } }, //17 //22
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_EXPIRY_TIME,   .bodyManifest = true, .extent = { 22, 8  } }, //22 //27
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_ENDSEGMENT,    .bodyManifest = true, .extent = { 30, 4  } }, //30 //35
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_PAYLOAD,       .bodyManifest = true, .extent = { 41, 8  } }, //41 //46
    { .wellKnownType = false, .indexOrKey = T_INVALID,                     .extent       = { 0,  0 } },
};

#define v1_content_nameless_nosig_truthTable TABLEENTRY(v1_content_nameless_nosig, TLV_ERR_NO_ERROR)

#endif // v1_content_nameless_nosig_h
