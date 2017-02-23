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

#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>


#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvSchemaV0.h>
#include <ccnx/forwarder/metis/tlv/metis_TlvSchemaV1.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

// a reasonably large enough number that we capture name parsing without
// needing to re-alloc.  Not a big deal if this is wrong, it just means
// we have to do one pass to count and another pass to fillin, and waste
// one malloc and free.
static const size_t _initialLengthForNameExtents = 24;


// -----------------------------

size_t
metisTlv_FixedHeaderLength(void)
{
    // at some point this will no longer be true and we will have to refactor
    return 8;
}


size_t
metisTlv_TotalHeaderLength(const uint8_t *packet)
{
    size_t length = 0;
    uint8_t version = packet[0];
    switch (version) {
        case 0:
            length = MetisTlvSchemaV0_Ops.totalHeaderLength(packet); // Deprecated
            break;

        case 1:
            length = MetisTlvSchemaV1_Ops.totalHeaderLength(packet);
            break;

        default:
            break;
    }
    return length;
}

size_t
metisTlv_TotalPacketLength(const uint8_t *packet)
{
    size_t length = 0;
    uint8_t version = packet[0];
    switch (version) {
        case 0:
            length = MetisTlvSchemaV0_Ops.totalPacketLength(packet); // Deprecated
            break;

        case 1:
            length = MetisTlvSchemaV1_Ops.totalPacketLength(packet);
            break;

        default:
            break;
    }
    return length;
}

PARCBuffer *
metisTlv_EncodeControlPlaneInformation(const CCNxControl *cpiControlMessage)
{
    PARCBuffer *encoded = NULL;
    CCNxTlvDictionary_SchemaVersion version = ccnxTlvDictionary_GetSchemaVersion(cpiControlMessage);
    switch (version) {
        case CCNxTlvDictionary_SchemaVersion_V0:
            encoded = MetisTlvSchemaV0_Ops.encodeControlPlaneInformation(cpiControlMessage);
            break;

        case CCNxTlvDictionary_SchemaVersion_V1:
            encoded = MetisTlvSchemaV1_Ops.encodeControlPlaneInformation(cpiControlMessage);
            break;

        default:
            break;
    }
    return encoded;
}

/**
 * @function metisTlv_ParseName
 * @abstract Parse a name into the provided output array, ensuring it does not exceed outputLength
 * @discussion
 *   <#Discussion#>
 *
 * @param outputArray may be NULL to count the number of name elements.
 * @para outputLength is the maximum number of name segments to parse in to outputArray
 * @return The number of name elements parsed
 */
static size_t
_metisTlv_ParseName(uint8_t *name, size_t nameLength, MetisTlvExtent *outputArray, size_t outputLength)
{
    size_t offset = 0;
    size_t count = 0;
    const size_t tl_length = 4;
    while (offset < nameLength) {
        MetisTlvType *tlv = (MetisTlvType *) (name + offset);
        uint16_t v_length = htons(tlv->length);

        if (count < outputLength) {
            outputArray[count].offset = offset;
            outputArray[count].length = tl_length + v_length;
        }

        // skip past the TL and V
        offset += tl_length + v_length;
        count++;
    }
    return count;
}

void
metisTlv_NameSegments(uint8_t *name, size_t nameLength, MetisTlvExtent **outputArrayPtr, size_t *outputLengthPtr)
{
    // allocate an array that's kind of big.  if name does not fit, we'll need to re-alloc.
    MetisTlvExtent *output = parcMemory_Allocate(_initialLengthForNameExtents * sizeof(MetisTlvExtent));
    assertNotNull(output, "parcMemory_Allocate(%zu) returned NULL", _initialLengthForNameExtents * sizeof(MetisTlvExtent));

    size_t actualLength = _metisTlv_ParseName(name, nameLength, output, _initialLengthForNameExtents);
    if (actualLength > _initialLengthForNameExtents) {
        // Oops, do over
        parcMemory_Deallocate((void **) &output);
        output = parcMemory_Allocate(actualLength * sizeof(MetisTlvExtent));
        assertNotNull(output, "parcMemory_Allocate(%zu) returned NULL", actualLength * sizeof(MetisTlvExtent));
        _metisTlv_ParseName(name, nameLength, output, actualLength);
    }

    *outputArrayPtr = output;
    *outputLengthPtr = actualLength;
}

bool
metisTlv_ExtentToVarInt(const uint8_t *packet, const MetisTlvExtent *extent, uint64_t *output)
{
    assertNotNull(packet, "Parameter buffer must be non-null");
    assertNotNull(extent, "Parameter output must be non-null");

    bool success = false;
    if (extent->length >= 1 && extent->length <= 8) {
        uint64_t value = 0;
        for (int i = 0; i < extent->length; i++) {
            value = value << 8 | packet[extent->offset + i];
        }
        *output = value;
        success = true;
    }
    return success;
}

