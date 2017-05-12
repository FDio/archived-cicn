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
 * @file interest_nameA_crc32c.h
 * @brief Interest with CRC validation
 *
 * Ground truth set derived from CRC RevEng http://reveng.sourceforge.net
 * e.g. reveng -c  -m CRC-32C 313233343536373839 gives the canonical check value 0xe306928e
 *
 * You can also calcaulate CRC32C online at http://www.zorc.breitbandkatze.de/crc.html using
 * CRC polynomial 0x1EDC6F41, init 0xFFFFFFFF, final 0xFFFFFFFF, reverse data bytes (check),
 * and reverse CRC result before final XOR (check).
 *
 * you can get the packet dump from the "write_packets" command.  here's the detailed steps.
 * The -c size of 4 in steps 4 and 7 are chosen to make it easy to delete the right number of lines.
 * there's nothing magic about the "4".
 *
 *  1) execute ./write_packets
 *  2) xxd -r -c 8 v1_content_nameA_crc32c.txt > y
 *  3) vim -b y
 *  4) :%!xxd -p -c 4
 *  5) Delete the frist 44 bytes (11 lines).  The first line should now be:
 *     00020015
 *  6) Delete the last 8 bytes
 *     The last line two lines should be:
 *      04000200
 *      00
 *    What's left is the part to be signed.
 *  7) :%!xxd -r -p -c 4
 *  8) :wq
 *  9) dump the file to one long URL-escaped hex string with
 *                  xxd -p -c 256 y | sed 's/[0-9a-f]\{2\}/%&/g'
 * 10) Copy the hex string to the website and use the settings specified above (don't use 0x in front
 *     of any hex strings).  Click "compute!"
 * 11) The answer should be 2C3CC0Af
 * 12) Put the byte array from (11) in the Validation Payload.
 *
 */

#ifndef v1_content_nameA_crc32c_h
#define v1_content_nameA_crc32c_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_ContentObjectSchema.h>

/**
 * A well formed interest with only a name
 */

__attribute__((unused))
static uint8_t v1_content_nameA_crc32c[] = {
    0x01, 0x01, 0x00, 85,       // ver = 1, type = content object, length = 85
    0x00, 0x00, 0x00, 44,       // HopLimit = 31, reserved = 0, header length = 44
    // ------------------------
    0x00, 0x05, 0x00, 20,       // ContentObject Fragment, length = 20
    0x12, 0x23, 0x34, 0x45,
    0x56, 0x67, 0x78, 0x89,     // fragid 0x1223344556677889
    0x05, 0xDC, 0x01, 0x00,     // MTU 1500, fragcnt 1, fragnum 0
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,     // interest fragment 0x0102030405060708
    // ------------------------
    0x00, 0x02, 0x00, 8,        // Recommended Cache Time
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x6D, 0xDD, 0x00,     // 2 hours (0x6DDD00 milli seconds)
    // ------------------------
    0x00, 0x02, 0x00, 21,       // type = content object, length = 21
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
    0x2C, 0x3C, 0xC0, 0xAF      // 2C3CC0AF
};

__attribute__((unused))
static TruthTableEntry
TRUTHTABLENAME(v1_content_nameA_crc32c)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_E2EFRAG,              .bodyManifest = false, .extent = { 12, 20 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_RecommendedCacheTime, .bodyManifest = false, .extent = { 36, 8  } },

    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_CONTENTOBJECT,        .bodyManifest = true,  .extent = { 48, 21 } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_NAME,                 .bodyManifest = true,  .extent = { 52, 17 } },

    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_ValidationAlg,        .bodyManifest = true,  .extent = { 73, 4  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_SIGBITS,              .bodyManifest = true,  .extent = { 81, 4  } },
    { .wellKnownType = false, .indexOrKey = T_INVALID,                            .extent       = { 0,   0 } },
};

#define v1_content_nameA_crc32c_truthTable TABLEENTRY(v1_content_nameA_crc32c, TLV_ERR_NO_ERROR)

#define v1_content_nameA_crc32c_URI "lci:/3=hello/0xf000=ouch"

#endif
