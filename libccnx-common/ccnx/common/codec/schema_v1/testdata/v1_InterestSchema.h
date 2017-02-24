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
 * This is from the version 0 codec.  All the test vectors in this directory (e.g. interest_nameA.h)
 * are encoded using these constants.  These are no longer used for any functional code, only to interpret the test vectors.
 *
 */

#ifndef Libccnx_v1_InterestSchema_h
#define Libccnx_v1_InterestSchema_h

#include <ccnx/common/codec/testdata/tlv_Schema.h>

// -----------------------------------------------------
// these are the array indicies used to store the TlvExtent for the item
typedef enum {
    V1_MANIFEST_INT_INTEREST = 0, // start of Interest body to end

    V1_MANIFEST_INT_NAME = 1,
    V1_MANIFEST_INT_KEYID = 2,
    V1_MANIFEST_INT_OBJHASH = 3,
    V1_MANIFEST_INT_PAYLOAD = 4,
    V1_MANIFEST_INT_IPIDM = 5,

    V1_MANIFEST_INT_ValidationAlg = 6,      // start of validation algorithm
    V1_MANIFEST_INT_ValidationPayload = 7,  // start of validation payload

    V1_MANIFEST_INT_BODYEND = 7
} ScheamV1ManifestInterestBody;

typedef enum {
    V1_MANIFEST_INT_OPTHEAD = 0,   // the start of the optional headers
    V1_MANIFEST_INT_LIFETIME = 1,
    V1_MANIFEST_INT_E2EFRAG = 2,
    V1_MANIFEST_INT_HEADEND = 3,
} ScheamV1ManifestInterestHeaders;


#define T_INTEREST 0x0001

#endif // Libccnx_tlv_InterestSchema_h
