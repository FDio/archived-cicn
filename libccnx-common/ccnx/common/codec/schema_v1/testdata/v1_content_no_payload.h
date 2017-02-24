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
 * @file v1_content_no_payload.h
 * @brief Content Object without a payload TLV
 *
 *
 */


#ifndef CCNx_Common_v1_content_no_payload_h
#define CCNx_Common_v1_content_no_payload_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_ContentObjectSchema.h>

__attribute__((unused))
static uint8_t v1_content_no_payload[] = {
    0x01, 0x01, 0x00, 0x21,
    0x00, 0x00, 0x00, 0x08,
    // -- content object
    0x00, 0x02, 0x00, 0x15,
    // -- name
    0x00, 0x00, 0x00, 0x11,
    0x00, 0x03, 0x00, 0x02,
    0x6e, 0x6f,
    0x00, 0x03, 0x00, 0x07,
    0x70, 0x61, 0x79, 0x6c,
    0x6f, 0x61, 0x64
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_content_no_payload)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_CONTENTOBJECT, .bodyManifest = true, .extent = { 12, 21 } },
    { .wellKnownType = false, .indexOrKey = T_INVALID,                     .extent       = { 0,  0 } },
};

#define v1_content_no_payload_truthTable TABLEENTRY(v1_content_no_payload, TLV_ERR_NO_ERROR)

#endif
