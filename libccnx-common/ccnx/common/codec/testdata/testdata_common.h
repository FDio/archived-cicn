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


/*
 *  testdata_common.h
 *  TransportRTA
 */

#ifndef TransportRTA_testdata_common_h
#define TransportRTA_testdata_common_h

#include <ccnx/common/codec/ccnxCodec_Error.h>

typedef struct tlv_extent {
    uint16_t offset;
    uint16_t length;
} TlvExtent;

// Equal to { 0xFFFF, 0xFFFF }
extern const TlvExtent TlvExtentNotFound;

/**
 * Determine if two TlvExtent instances are equal.
 *
 * The following equivalence relations on non-null `TlvExtent` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `TlvExtent_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `tlvExtent_Equals(x, y)` must return true if and only if
 *        `tlvExtent_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `tlvExtent_Equals(x, y)` returns true and
 *        `tlvExtent_Equals(y, z)` returns true,
 *        then  `tlvExtent_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `tlvExtent_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `tlvExtent_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `TlvExtent` instance.
 * @param b A pointer to a `TlvExtent` instance.
 * @return true if the two `TlvExtent` instances are equal.
 *
 * Example:
 * @code
 * {
 *    TlvExtent *a = tlvExtent_Create();
 *    TlvExtent *b = tlvExtent_Create();
 *
 *    if (tlvExtent_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool tlvExtent_Equals(const TlvExtent *a, const TlvExtent *b);

#define TRUTHTABLENAME(NAME) NAME ## _truthTableEntries

// easy way to generate table entries, so long as you use the standard naming
// convention for the the TruthTableEntry array
#define TABLEENTRY(NAME, ERROR) { .testname = #NAME, .packet = NAME, .length = sizeof(NAME), .expectedError = ERROR, .entry = TRUTHTABLENAME(NAME) }

typedef struct testrig_truth_table_entry {
    bool wellKnownType;

    // is the wellKnownType in the body manifest?  or the header?
    bool bodyManifest;

    // if its a well known type, this is the manifest array index
    // otherwise, its the unknown type value
    int indexOrKey;

    TlvExtent extent;
} TruthTableEntry;


typedef struct testrig_truth_table {
    const char *testname;
    uint8_t *packet;
    size_t length;

    CCNxCodecErrorCodes expectedError;

    // the array is terminated by a T_INVALID value
    // for "arrayIndexOrTypeKey"
    TruthTableEntry *entry;
} TruthTable;

#endif
