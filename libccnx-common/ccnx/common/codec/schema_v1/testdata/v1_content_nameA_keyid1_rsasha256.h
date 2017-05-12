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
 * Signature generated using "openssl sha -sign test_rsa_key.pem  -sha256 -binary y > sig", where "y" is
 * generated from the "xxd -r" of the message hex dump.  You need to manually edit "y" so it only
 * contains the parts begin signed.  Then use "xxd -i sig" to get the byte array.
 *
 * you can get the dump from the "write_packets" command.  here's the detailed steps:
 *
 * 1) execute ./write_packets
 * 2) xxd -r -c 8 v1_content_nameA_keyid1_rsasha256.txt > y
 * 3) vim -b y
 * 4) :%!xxd -p -c 16
 * 5) Delete the frist 32 bytes (2 lines).  The first line should now be:
 *     0002003a000000110002000568656c6c
 * 6) Delete the last 132 bytes (9 lines, 8 full lines plus the last 4 byte line)
 *    The last line in the file should now be:
 *     0f3592ae7027fbd2e41e270203010001
 *    What's left is the part to be signed.
 * 7) :%!xxd -r -p -c 16
 * 8) :wq
 * 9) Copy the PEM blocks below and put them in the file "key.pem".  Make sure to remove
 *    any leading whitespace.  If you get an error in the next command like
 *    "53999:error:0906D06C:PEM routines:PEM_read_bio:no start ... 648:Expecting: ANY PRIVATE KEY" then
 *    you most likely have leading whitespace.  Make sure all lines are flush left.
 * 10) openssl sha -sign key.pem  -sha256 -binary y > sig
 * 11) xxd -i sig
 * 12) Put the byte array from (11) in the Validation Payload.  Verify the length, it should be
 *     128 bytes.  If not, fixup the length of the ValidatonPayload and the PacketLength.
 *
 */

/*
 * -----BEGIN RSA PRIVATE KEY-----
 * MIICXAIBAAKBgQCn1pPF8XPGErX6ecXvGIvvqs0EAY+Ddz+xZqFauTkqsj4w+xH8
 * V/yd0S938Kt3rWYsJsibUW9pvyYQBinuy7AsXpEOJEKN5nWgTgRDDD5MBnRnrYTD
 * 6PTFlHPEnyWoQga/RRnimBw2oUNNm3EI4YLf4k8qPD0PNZKucCf70uQeJwIDAQAB
 * AoGAVOYPA/7aIGSQlu4IOKTDDG3qnM8pSEgG+PbAQgMVrspQ+TfXZj0ftLj++P3N
 * zpDw8P6BVUfBQs2FNG/ZwEhaiZVgJAl7cIAxJ9Ac+1oZYSgGyJfb3u9iWvkbMOoj
 * 83Inx5yyN+Qmk5zceH4pOC5D5cDAuGGZ740Euv4o2/2O3qECQQDTmWZw021PvEbA
 * r18O1YfZGxO3zFCwFXCpnHvtSMbP+MXAG5Gt47wZt4Vx1rX9k78beeCUitwqp3d3
 * ZI+YlUu3AkEAyw5wssQsJty/n2FL8DbJN3UzUhkcaCFYrKz3RtFye9wu+Bw0TxPC
 * 3jhFVcynm3nH3ZJN0JsnsPnHXuoQToShEQJATXC51hb6zZC5UDGel348fo9zUvP6
 * n8bo+ZoknL3izSBdtyYf1cUgBUVuGDCdYFWfPn4HXDXJx+6MQWzTRON21wJBAMZL
 * U8M/z94jtP3wBjiPR/Dggz2pSBRofDAkuVZvM13BqByjbnHK2oIocY1YTlWGl6fJ
 * ODR/UEODqS8HZOVIoAECQANcuvVnqDixSIl2ySZvydQytv4DKTbvE0nYSRroYIlJ
 * PTOBPy8ynIUkJwc2E1BsLl7V8gO62a5O0ntTwBMnPSQ=
 * -----END RSA PRIVATE KEY-----
 *
 * -----BEGIN PUBLIC KEY-----
 * MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCn1pPF8XPGErX6ecXvGIvvqs0E
 * AY+Ddz+xZqFauTkqsj4w+xH8V/yd0S938Kt3rWYsJsibUW9pvyYQBinuy7AsXpEO
 * JEKN5nWgTgRDDD5MBnRnrYTD6PTFlHPEnyWoQga/RRnimBw2oUNNm3EI4YLf4k8q
 * PD0PNZKucCf70uQeJwIDAQAB
 * -----END PUBLIC KEY-----
 */

#ifndef v1_content_nameA_keyid1_rsasha256_h
#define v1_content_nameA_keyid1_rsasha256_h

#include <ccnx/common/codec/testdata/testdata_common.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_ContentObjectSchema.h>

/**
 * A well formed interest with only a name
 */


__attribute__((unused))
static uint8_t v1_content_nameA_keyid1_rsasha256[] = {
    0x01, 0x01, 0x01, 0xB9,// ver = 1, type = content object, length = 441
    0x00, 0x00, 0x00, 37,  // HopLimit = 0, reserved = 0, header length = 37
    // ------------------------
    0x00, 0x05, 0x00, 20,  // ContentObject Fragment, length = 20
    0x12, 0x23, 0x34, 0x45,
    0x56, 0x67, 0x78, 0x89,// fragid 0x1223344556677889
    0x05, 0xDC, 0x01, 0x00,// MTU 1500, fragcnt 1, fragnum 0
    0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08,// interest fragment 0x0102030405060708
    //-----------------------
    0x00, 0x03, 0x00, 1, //path label, lenght = 1     //36 bytes
    0x00,
    // ------------------------
    0x00, 0x02, 0x00, 58,  // type = content object, length = 58
    // ------------------------
    0x00, 0x00, 0x00, 17,  // type = name, length = 17
    0x00, 0x03, 0x00, 0x05,// type = binary, length = 5
    'h',  'e',  'l',  'l', // "hello"
    'o',
    0xF0, 0x00, 0x00, 0x04,// type = app, length = 4
    'o',  'u',  'c',  'h', // "ouch"                //62 bytes
    // ------------------------
    0x00, 0x05, 0x00, 1,   // PayloadType
    1,    // type 1 = key
    0x00, 0x06, 0x00, 0x08,// expiry time in msec  //71 bytes
    0x00, 0x00, 0x01, 0x43,// 1,388,534,400,000 msec
    0x4B, 0x19, 0x84, 0x00,
    0x00, 0x19, 0x00, 4,   // end chunk number
    0x06, 0x05, 0x04, 0x03,                         //87 bytes
    // ------------------------
    0x00, 0x01, 0x00, 8,   // payload, length = 8   //91
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
static TruthTableEntry
TRUTHTABLENAME(v1_content_nameA_keyid1_rsasha256)[] =
{
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_E2EFRAG,       .bodyManifest = false, .extent = { 12,  20  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_CONTENTOBJECT, .bodyManifest = true,  .extent = { 41,  58  } }, //36 //41
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_NAME,          .bodyManifest = true,  .extent = { 40,  17  } },
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_PAYLOADTYPE,   .bodyManifest = true,  .extent = { 66,  1   } }, //61 //66
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_EXPIRY_TIME,   .bodyManifest = true,  .extent = { 71,  8   } }, //66 //71
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_ENDSEGMENT,    .bodyManifest = true,  .extent = { 83,  4   } }, //78 //83
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_PAYLOAD,       .bodyManifest = true,  .extent = { 91,  8   } }, //86 //91
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_ValidationAlg, .bodyManifest = true,  .extent = { 94,  206 } }, //94 //99
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_KEYID,         .bodyManifest = true,  .extent = { 111, 32  } }, //106 //111
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_PUBKEY,        .bodyManifest = true,  .extent = { 147, 162 } }, //142 //147
    { .wellKnownType = true,  .indexOrKey = V1_MANIFEST_OBJ_SIGBITS,       .bodyManifest = true,  .extent = { 313, 128 } }, //308 //313 
    { .wellKnownType = false, .indexOrKey = T_INVALID,                     .extent       = { 0,   0 } },
};

#define v1_content_nameA_keyid1_rsasha256_truthTable TABLEENTRY(v1_content_nameA_keyid1_rsasha256, TLV_ERR_NO_ERROR)

#define v1_content_nameA_keyid1_rsasha256_URI "lci:/3=hello/0xf000=ouch"

#endif
