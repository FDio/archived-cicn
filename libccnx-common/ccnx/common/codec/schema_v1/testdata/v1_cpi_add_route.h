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
 * @file v1_cpi_add_route.h
 * @brief A hand-encoded CPI packet to add a route
 *
 * The v1 old-style control packet is a fixed header plus a tlv container 0xBEEF with a "value" of the CPI JSON string.
 * The packet type is 0xA4.
 *
 */

#ifndef TransportRTA_v1_cpi_AddRoute_h
#define TransportRTA_v1_cpi_AddRoute_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_CPISchema.h>

__attribute__((unused))
static uint8_t v1_cpi_add_route[] = "\x01\xA4\x00\xA7"
                                    "\x00\x00\x00\x08"
                                    "\xBE\xEF\x00\x9A"
                                    "{\"CPI_REQUEST\":{\"SEQUENCE\":22,\"REGISTER\":{\"PREFIX\":\"lci:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_cpi_add_route)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_CPI_PAYLOAD, .bodyManifest = true, .extent = { 12, 155 } },
    { .wellKnownType = false, .indexOrKey = T_INVALID,               .extent       = { 0,  0 } },
};

#define v1_cpi_add_route_truthTable TABLEENTRY(v1_cpi_add_route, TLV_ERR_NO_ERROR)

#define v1_cpi_add_route_PrefixUri "lci:/howdie/stranger"
#define v1_cpi_add_route_Sequence  22
#define v1_cpi_add_route_Interface 55
#endif // TransportRTA_cpi_AddRoute_h
