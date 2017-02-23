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
 * @file metis_TestDataV1
 * @brief v1 schema test packets
 *
 * Test vectors of V1 schema packets.
 *
 */

#ifndef Metis_metis_TestDataV1_h
#define Metis_metis_TestDataV1_h

// =======================
// Interests

__attribute__((unused))
static uint8_t metisTestDataV1_Interest_AllFields[] = {
    0x01, 0x00, 0x00, 100, // ver = 1, type = interest, length = 100
    0x20, 0x00, 0x11, 14,  // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,   // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 82,  // type = interest, length = 82
    // ------------------------
    0x00, 0x00, 0x00, 8,   // type = name, length = 8
    0x00, 0x02, 0x00, 4,   // type = binary, length = 4
    'c',  'o',  'o',  'l', // "cool"
    // ------------------------
    0x00, 0x02, 0x00, 16,  // type = keyid restriction, length = 16
    0xa0, 0xa1, 0xa2, 0xa3,
    0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab,
    0xac, 0xad, 0xae, 0xaf,
    // ------------------------
    0x00, 0x03, 0x00, 32,  // type = hash restriction, length = 32
    0xb0, 0xb1, 0xb2, 0xb3,
    0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb,
    0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb,
    0xcc, 0xcd, 0xce, 0xcf,
    // ------------------------
    0x00, 0x04, 0x00, 1,   // Interest payload method (1 byte)
    0x00,
    // ------------------------
    0x00, 0x01, 0x00, 5,   // type = payload, length = 5
    0xD0, 0xD1, 0xD2, 0xD3,
    0xD4,
};

__attribute__((unused))
static uint8_t metisTestDataV1_Interest_NameA_Crc32c[] = {
    0x01, 0x00, 0x00, 65,  // ver = 1, type = interest, length = 65
    0x20, 0x00, 0x00, 24,  // HopLimit = 32, reserved = 0, header length = 24
    // ------------------------
    0x00, 0x03, 0x00, 12,  // Interest Fragment
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, // fragment 0x0102030405060708
    0x05, 0xDC, 0x00, 0x00, // MTU 1500, fragcnt 0, fragnum 0
    // ------------------------
    0x00, 0x01, 0x00, 0x15, // type = interest, length = 21
    // ------------------------
    0x00, 0x00, 0x00, 0x11, // type = name, length = 17
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l', // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h', // "ouch"
    // ------------------------
    0x00, 0x03, 0x00, 4,   // validation alg, length = 4
    0x00, 0x02, 0x00, 0x00, // CRC32C
    // ------------------------
    0x00, 0x04, 0x00, 4,   // validation payload
    0x6A, 0xD7, 0xB1, 0xF2      // 6AD7B1F2
};

__attribute__((unused))
static uint8_t metisTestDataV1_Interest_NameAAndKeyId[] = {
    0x01, 0x00, 0x00, 89,  // ver = 1, type = interest, length = 73
    0x20, 0x00, 0x11, 14,  // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,   // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 71,  // type = interest, length = 55
    // ------------------------
    0x00, 0x00, 0x00, 0x11, // type = name, length = 17
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l', // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h', // "ouch"
    // ------------------------
    0x00, 0x02, 0x00, 32,  // type = keyid restriction, length = 32
    0x5c, 0x23, 0x4c, 0x28,
    0x50, 0xda, 0x20, 0x7b,
    0x88, 0x25, 0x8b, 0xf3,
    0x62, 0x61, 0x96, 0xd8,
    0xf0, 0x60, 0x76, 0x38,
    0xa2, 0xd4, 0xe0, 0xe2,
    0x49, 0xb2, 0xa9, 0xaf,
    0xce, 0xb8, 0x85, 0x59,
    // ------------------------
    0x00, 0x04, 0x00, 1,   // Interest payload method (1 byte)
    0x00,
    // ------------------------
    0x00, 0x01, 0x00, 5,   // type = payload, length = 5
    0xD0, 0xD1, 0xD2, 0xD3,
    0xD4,
};


// ===========================
// Content Objects

__attribute__((unused))
static uint8_t metisTestDataV1_ContentObject_NameA_Crc32c[] = {
    0x01, 0x01, 0x00, 85,  // ver = 1, type = content object, length = 85
    0x00, 0x00, 0x00, 44,  // HopLimit = 31, reserved = 0, header length = 44
    // ------------------------
    0x00, 0x04, 0x00, 20,  // ContentObject Fragment, length = 20
    0x12, 0x23, 0x34, 0x45,
    0x56, 0x67, 0x78, 0x89, // fragid 0x1223344556677889
    0x05, 0xDC, 0x01, 0x00, // MTU 1500, fragcnt 1, fragnum 0
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, // interest fragment 0x0102030405060708
    // ------------------------
    0x00, 0x02, 0x00, 8,   // Recommended Cache Time
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x6D, 0xDD, 0x00, // 2 hours (0x6DDD00 milli seconds)
    // ------------------------
    0x00, 0x02, 0x00, 21,  // type = content object, length = 21
    // ------------------------
    0x00, 0x00, 0x00, 0x11, // type = name, length = 17
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l', // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h', // "ouch"
    // ------------------------
    0x00, 0x03, 0x00, 4,   // validation alg, length = 4
    0x00, 0x02, 0x00, 0x00, // CRC32C
    // ------------------------
    0x00, 0x04, 0x00, 4,   // validation payload
    0x2C, 0x3C, 0xC0, 0xAF      // 2C3CC0AF
};

__attribute__((unused))
static uint8_t metisTestDataV1_ContentObject_NameA_KeyId1_RsaSha256[] = {
    0x01, 0x01, 0x01, 0xB4, // ver = 1, type = content object, length = 436
    0x00, 0x00, 0x00, 32,  // HopLimit = 0, reserved = 0, header length = 32
    // ------------------------
    0x00, 0x04, 0x00, 20,  // ContentObject Fragment, length = 20
    0x12, 0x23, 0x34, 0x45,
    0x56, 0x67, 0x78, 0x89, // fragid 0x1223344556677889
    0x05, 0xDC, 0x01, 0x00, // MTU 1500, fragcnt 1, fragnum 0
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, // interest fragment 0x0102030405060708
    // ------------------------
    0x00, 0x02, 0x00, 58,  // type = content object, length = 58
    // ------------------------
    0x00, 0x00, 0x00, 17,  // type = name, length = 17
    0x00, 0x02, 0x00, 0x05, // type = binary, length = 5
    'h',  'e',  'l',  'l', // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04, // type = app, length = 4
    'o',  'u',  'c',  'h', // "ouch"
    // ------------------------
    0x00, 0x05, 0x00, 1,   // PayloadType
    1,    // type 1 = key
    0x00, 0x06, 0x00, 0x08, // expiry time in msec
    0x00, 0x00, 0x01, 0x43, // 1,388,534,400,000 msec
    0x4B, 0x19, 0x84, 0x00,
    0x00, 0x19, 0x00, 4,   // end chunk number
    0x06, 0x05, 0x04, 0x03,
    // ------------------------
    0x00, 0x01, 0x00, 8,   // paylaod, length = 8
    0x73, 0x75, 0x72, 0x70,
    0x72, 0x69, 0x73, 0x65,
    // ------------------------
    0x00, 0x03, 0x00, 206, // validation alg, length = 206
    0x00, 0x06, 0x00, 202, // RSA-SHA256, length = 162 + 4 + 32 + 4 = 202
    0x00, 0x09, 0x00, 32,  // type = keyid, length = 32
    0x5c, 0x23, 0x4c, 0x28,
    0x50, 0xda, 0x20, 0x7b,
    0x88, 0x25, 0x8b, 0xf3,
    0x62, 0x61, 0x96, 0xd8,
    0xf0, 0x60, 0x76, 0x38,
    0xa2, 0xd4, 0xe0, 0xe2,
    0x49, 0xb2, 0xa9, 0xaf,
    0xce, 0xb8, 0x85, 0x59,
    0x00, 0x0B, 0x00, 162, // public key, length = 162
    0x30, 0x81, 0x9f, 0x30,0x0d,  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x01, 0x01,0x05,  0x00, 0x03, 0x81, 0x8d, 0x00, 0x30, 0x81,
    0x89, 0x02, 0x81, 0x81,0x00,  0xa7, 0xd6, 0x93, 0xc5, 0xf1, 0x73, 0xc6,
    0x12, 0xb5, 0xfa, 0x79,0xc5,  0xef, 0x18, 0x8b, 0xef, 0xaa, 0xcd, 0x04,
    0x01, 0x8f, 0x83, 0x77,0x3f,  0xb1, 0x66, 0xa1, 0x5a, 0xb9, 0x39, 0x2a,
    0xb2, 0x3e, 0x30, 0xfb,0x11,  0xfc, 0x57, 0xfc, 0x9d, 0xd1, 0x2f, 0x77,
    0xf0, 0xab, 0x77, 0xad,0x66,  0x2c, 0x26, 0xc8, 0x9b, 0x51, 0x6f, 0x69,
    0xbf, 0x26, 0x10, 0x06,0x29,  0xee, 0xcb, 0xb0, 0x2c, 0x5e, 0x91, 0x0e,
    0x24, 0x42, 0x8d, 0xe6,0x75,  0xa0, 0x4e, 0x04, 0x43, 0x0c, 0x3e, 0x4c,
    0x06, 0x74, 0x67, 0xad,0x84,  0xc3, 0xe8, 0xf4, 0xc5, 0x94, 0x73, 0xc4,
    0x9f, 0x25, 0xa8, 0x42,0x06,  0xbf, 0x45, 0x19, 0xe2, 0x98, 0x1c, 0x36,
    0xa1, 0x43, 0x4d, 0x9b,0x71,  0x08, 0xe1, 0x82, 0xdf, 0xe2, 0x4f, 0x2a,
    0x3c, 0x3d, 0x0f, 0x35,0x92,  0xae, 0x70, 0x27, 0xfb, 0xd2, 0xe4, 0x1e,
    0x27, 0x02, 0x03, 0x01,0x00,  0x01,
    // ------------------------
    0x00, 0x04, 0x00, 128, // validation payload, length = 128
    0x03, 0x46, 0xee, 0xb7,0x30,  0x1c, 0xea, 0x13, 0x0c, 0xce, 0x83, 0x5b,
    0x7b, 0x4f, 0xf5, 0x83,0x37,  0x08, 0x7f, 0xe0, 0xe1, 0xc9, 0x70, 0x09,
    0x5e, 0xc2, 0x1c, 0xd3,0x74,  0xbb, 0xbd, 0x72, 0x35, 0xa4, 0x1b, 0x0f,
    0x3d, 0x04, 0x5e, 0xf7,0xc1,  0xdf, 0xea, 0xc3, 0x50, 0x47, 0x14, 0xf9,
    0xb7, 0xbb, 0x42, 0xf9,0x3e,  0xaa, 0x49, 0xd2, 0x9f, 0xd1, 0xab, 0xf6,
    0xda, 0x32, 0x4a, 0xb1,0xb9,  0x69, 0x91, 0x57, 0x43, 0x5d, 0x06, 0xcf,
    0x1d, 0x9f, 0x7c, 0x28,0xee,  0x35, 0xaa, 0xd0, 0xb2, 0x8d, 0x34, 0x09,
    0xcd, 0xdb, 0x01, 0xf7,0xda,  0xe8, 0x59, 0x98, 0x4e, 0x59, 0xfa, 0x13,
    0xd0, 0xd1, 0x54, 0x8e,0x64,  0x8c, 0xc6, 0xd7, 0x6b, 0xc5, 0x89, 0xeb,
    0x37, 0x8f, 0x53, 0x04,0xba,  0x03, 0x05, 0xb4, 0x67, 0x73, 0xe1, 0x51,
    0x59, 0x12, 0xbc, 0x25,0xaa,  0xa2, 0xc1, 0x18
};

__attribute__((unused))
static uint8_t metisTestDataV1_CPI_AddRoute_Crc32c[] = "\x01\xA4\x00\xB7"
                                                       "\x00\x00\x00\x08"
                                                       "\xBE\xEF\x00\x9A"
                                                       "{\"CPI_REQUEST\":{\"SEQUENCE\":22,\"REGISTER\":{\"PREFIX\":\"lci:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}"
                                                       "\x00\x03\x00\x04"
                                                       "\x00\x02\x00\x00"
                                                       "\x00\x04\x00\x04"
                                                       "\x78\xfd\x92\x6a";

// ===============================================
// HopByHop Fragments
// These fragments together make a small Interest packet.

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_Begin[] = {
    0x01, 4,    0x00, 20,       // ver = 1, type = hop-by-hop frag, length = 20
    0x40, 0x00, 0x01, 8,        // B, seqnum = 1, header length = 8
    // ------------------------
    0x00, 0x05, 0x00, 8,        // Interest Fragment
    // ------------------------
    0x01, 0x00, 0x00, 24,       // ver = 1, type = interest, length = 24
    0x20, 0x00, 0x11, 8,        // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_Begin_Fragment[] = {
    0x01, 0x00, 0x00, 24,       // ver = 1, type = interest, length = 24
    0x20, 0x00, 0x11, 8,        // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_Middle[] = {
    0x01, 4,    0x00, 16,       // ver = 1, type = hop-by-hop frag, length = 16
    0x00, 0x00, 0x02, 8,        // no flag, seqnum = 2, header length = 8
    // ------------------------
    0x00, 0x05, 0x00, 4,        // Interest Fragment
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_Middle_Fragment[] = {
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_End[] = {
    0x01, 4,    0x00, 24,       // ver = 1, type = hop-by-hop frag, length = 24
    0x20, 0x00, 0x03, 8,        // E, seqnum = 3, header length = 8
    // ------------------------
    0x00, 0x05, 0x00, 12,       // Interest Fragment
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_End_Fragment[] = {
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_BeginEnd[] = {
    0x01, 4,    0x00, 36,       // ver = 1, type = hop-by-hop frag, length = 36
    0x60, 0x00, 0x04, 8,        // B, seqnum = 4, header length = 8
    // ------------------------
    0x00, 0x05, 0x00, 24,       // Interest Fragment
    // ------------------------
    0x01, 0x00, 0x00, 24,       // ver = 1, type = interest, length = 24
    0x20, 0x00, 0x11, 8,        // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_BeginEnd_Fragment[] = {
    0x01, 0x00, 0x00, 24,       // ver = 1, type = interest, length = 24
    0x20, 0x00, 0x11, 8,        // HopLimit = 32, reserved = 0, flags = 0x11, header length = 8
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

__attribute__((unused))
static uint8_t metisTestDataV1_HopByHopFrag_Idle[] = {
    0x01, 4,    0x00, 8,        // ver = 1, type = hop-by-hop frag, length = 16
    0x10, 0x00, 0x05, 8,        // B, seqnum = 1, header length = 8
};

// ===============================================
// Ethernet padded interest

__attribute__((unused))
static uint8_t metisTestDataV1_InterestWithEthernetPadding[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // dmac
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55,   // smac
    0x08, 0x01, // ethertype
    0x01, 0x00, 0x00, 0x18, // ver = 1, len = 24
    0xfe, 0x00, 0x00, 0x08, // hoplimit = 254, header length = 8
    0x00, 0x01, 0x00, 0x0c, // interest
    0x00, 0x00, 0x00, 0x08, // name
    0x00, 0x01, 0x00, 0x04, // name component
    0x64, 0x61, 0x74, 0x65, // "date"
    0x00, 0x00, 0x00, 0x00, // ethernet padding
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

__attribute__((unused))
static uint8_t metisTestDataV1_InterestWithEthernetPaddingStripped[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // dmac
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55,   // smac
    0x08, 0x01, // ethertype
    0x01, 0x00, 0x00, 0x18, // ver = 1, len = 24
    0xfe, 0x00, 0x00, 0x08, // hoplimit = 254, header length = 8
    0x00, 0x01, 0x00, 0x0c, // interest
    0x00, 0x00, 0x00, 0x08, // name
    0x00, 0x01, 0x00, 0x04, // name component
    0x64, 0x61, 0x74, 0x65, // "date"
};

// ===============================================
// Error frames for coding violation testing

// Less than fixed header size
__attribute__((unused))
static uint8_t metisTestDataV1_Runt[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11
};

// Version Unknown
__attribute__((unused))
static uint8_t metisTestDataV1_BadVersion[] = {
    0xFF, 0x00, 0x00, 30,       // ver = 255 (bad), type = interest, length = 30
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// PacketType Unknown
__attribute__((unused))
static uint8_t metisTestDataV1_BadPacketType[] = {
    0x01, 0x77, 0x00, 30,       // ver = 1, type = 77 (bad), length = 30
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// packet length less than fixed header length
__attribute__((unused))
static uint8_t metisTestDataV1_PacketLengthLessFixedHeaderLength[] = {
    0x01, 0x00, 0x00, 6,        // ver = 1, type = interest, length = 6 (too short)
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// packet length less than fixed header length
__attribute__((unused))
static uint8_t metisTestDataV1_PacketLengthLessBeyondEnd[] = {
    0x01, 0x00, 0x00, 40,       // ver = 1, type = interest, length = 40 (beyond end of frame)
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};


// header length less than fixed header length
__attribute__((unused))
static uint8_t metisTestDataV1_HeaderLengthLessFixedHeaderLength[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 7,        // HopLimit = 32, reserved = 0, flags = 0x11, header length = 7
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// header length less than actual header length
__attribute__((unused))
static uint8_t metisTestDataV1_HeaderLengthTooShort[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 13,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 13 (1 byte too short)
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// header length less than actual header length
__attribute__((unused))
static uint8_t metisTestDataV1_HeaderLengthTooLong[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 16,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 16 (2 bytes too long)
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// header length swallows whole message
__attribute__((unused))
static uint8_t metisTestDataV1_HeaderLengthWholePacket[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 30,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 30
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};

// header length beyond end of frame
__attribute__((unused))
static uint8_t metisTestDataV1_HeaderLengthBeyondEnd[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 40,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 40
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};


// packet length less than optional header length
__attribute__((unused))
static uint8_t metisTestDataV1_PacketLengthLessHeaderLength[] = {
    0x01, 0x00, 0x00, 12,       // ver = 1, type = interest, length = 12 (past fixed header, less than optional header)
    0x20, 0x00, 0x11, 14,       // HopLimit = 32, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 12,       // type = interest, length = 12
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4
    'c',  'o',  'o',  'l',      // "cool"
};


// T_INTEREST length too long
__attribute__((unused))
static uint8_t metisTestDataV1_Interest_MessageLengthTooLong[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 14,       // HopLimit = 31, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 13,       // type = interest, length = 13 (1 byte too far)
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4 (this goes beyond the T_INTEREST length)
    'c',  'o',  'o',  'l',      // "cool"
};


// T_INTEREST length too short
__attribute__((unused))
static uint8_t metisTestDataV1_Interest_MessageLengthTooShort[] = {
    0x01, 0x00, 0x00, 30,       // ver = 1, type = interest, length = 30
    0x20, 0x00, 0x11, 14,       // HopLimit = 31, reserved = 0, flags = 0x11, header length = 14
    // ------------------------
    0x00, 0x01, 0x00, 2,        // Interest Lifetime (2 bytes)
    0xEA, 0xEB,
    // ------------------------
    0x00, 0x01, 0x00, 11,       // type = interest, length = 11 (1 byte too short)
    // ------------------------
    0x00, 0x00, 0x00, 8,        // type = name, length = 8
    0x00, 0x02, 0x00, 4,        // type = binary, length = 4 (this goes beyond the T_INTEREST length)
    'c',  'o',  'o',  'l',      // "cool"
};

#define FRAMEPAIR(array) { .frame = array, .length = sizeof(array) }

__attribute__((unused))
struct metisTestDataV1_frame_array {
    const uint8_t *frame;
    size_t length;
} metisTestDataV1_ErrorFrames[] = {
    FRAMEPAIR(metisTestDataV1_Runt),                             // 0
    FRAMEPAIR(metisTestDataV1_BadVersion),
    FRAMEPAIR(metisTestDataV1_BadPacketType),
    FRAMEPAIR(metisTestDataV1_PacketLengthLessFixedHeaderLength),
    FRAMEPAIR(metisTestDataV1_PacketLengthLessBeyondEnd),
    FRAMEPAIR(metisTestDataV1_HeaderLengthLessFixedHeaderLength), // 5
    FRAMEPAIR(metisTestDataV1_HeaderLengthTooShort),
    FRAMEPAIR(metisTestDataV1_HeaderLengthTooLong),
    FRAMEPAIR(metisTestDataV1_HeaderLengthWholePacket),
    FRAMEPAIR(metisTestDataV1_HeaderLengthBeyondEnd),
    FRAMEPAIR(metisTestDataV1_PacketLengthLessHeaderLength),
    FRAMEPAIR(metisTestDataV1_Interest_MessageLengthTooLong),
    FRAMEPAIR(metisTestDataV1_Interest_MessageLengthTooShort),
    { .frame = NULL,                                             .length= 0 }
};


#endif // Metis_metis_TestDataV1_h
