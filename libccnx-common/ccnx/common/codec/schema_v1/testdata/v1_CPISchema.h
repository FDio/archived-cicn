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
 * Defines the truthtable manifest constants used by the version 1 test vectors.
 *
 */

/**
 * Schema for Control Plane Interface packets.  These packets are a TLV wrapping
 * the control JSON.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

#ifndef Libccnx_v1_CPISchema_h
#define Libccnx_v1_CPISchema_h

#include <ccnx/common/codec/testdata/tlv_Schema.h>

// -----------------------------------------------------
// these are the array indicies used to store the TlvExtent for the item
typedef enum {
    V1_MANIFEST_CPI_PAYLOAD = 0,
    V1_MANIFEST_CPI_SIGBITS = 5,            // the payload of the signature
    V1_MANIFEST_CPI_ValidationAlg = 6,      // start of validation algorithm
    V1_MANIFEST_CPI_ValidationPayload = 7,  // start of validation payload
    V1_MANIFEST_CPI_BODYEND = 8
} V1_ManifestCPIBody;

typedef enum {
    V1_MANIFEST_CPI_HEADEND = 0
} V1_ManifestCPIHeaders;

#endif // Libccnx_tlv_CPISchema_h
