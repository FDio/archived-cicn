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
 * This is from the version 1 codec.  All the test vectors in this directory (e.g. interest_nameA.h)
 * are encoded using these constants.  These are no longer used for any functional code, only to interpret the test vectors.
 *
 */

#ifndef Libccnx_v1_ContentObjectSchema_h
#define Libccnx_v1_ContentObjectSchema_h

#include <ccnx/common/codec/testdata/tlv_Schema.h>

#define T_CONTENTOBJECT 0x0002

// these are the array indicies used to store the TlvExtent for the item
typedef enum {
    // top level entities
    V1_MANIFEST_OBJ_NAME = 0,
    V1_MANIFEST_OBJ_CONTENTOBJECT = 1,    // the top container
    V1_MANIFEST_OBJ_NAMEAUTH = 2,
    V1_MANIFEST_OBJ_PAYLOADTYPE = 3,
    V1_MANIFEST_OBJ_PAYLOAD = 4,
    V1_MANIFEST_OBJ_SIGBITS = 5,

    // inside the name authenticator
    V1_MANIFEST_OBJ_KEYID = 6,
    V1_MANIFEST_OBJ_CRYPTO_SUITE = 7,
    V1_MANIFEST_OBJ_KEY = 8,
    V1_MANIFEST_OBJ_CERT = 9,
    V1_MANIFEST_OBJ_KEYNAME = 10,
    V1_MANIFEST_OBJ_KEYNAME_NAME = 11,
    V1_MANIFEST_OBJ_KEYNAME_OBJHASH = 12,

    // inside the protocol information
    V1_MANIFEST_OBJ_METADATA = 13,

    // inside metadata
    V1_MANIFEST_OBJ_OBJ_TYPE = 14,
    V1_MANIFEST_OBJ_CREATE_TIME = 15,
    V1_MANIFEST_OBJ_EXPIRY_TIME = 16,

    // inside signature block
    V1_MANIFEST_OBJ_ValidationPayload = 17,
    V1_MANIFEST_OBJ_ENDSEGMENT = 18,
    V1_MANIFEST_OBJ_PUBKEY = 19,

    V1_MANIFEST_OBJ_ValidationAlg = 20,
    V1_MANIFEST_OBJ_SigningTime = 21,
    V1_MANIFEST_OBJ_BODYEND = 22
} SchemaV1ManifestContentObjectBody;

typedef enum {
    V1_MANIFEST_OBJ_OPTHEAD = 0,
    V1_MANIFEST_OBJ_E2EFRAG = 2,
    V1_MANIFEST_OBJ_FIXEDHEADER = 3,
    V1_MANIFEST_OBJ_RecommendedCacheTime = 4,
    V1_MANIFEST_OBJ_HEADEND = 5
} SchemaV1ManifestContentObjectHeaders;

#endif // Libccnx_tlv_ContentObjectSchema_h
