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
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <LongBow/runtime.h>

#include <ccnx/common/internal/ccnx_ChunkingFacadeV1.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

static void
_assertInvariants(const CCNxTlvDictionary *dictionary)
{
    assertNotNull(dictionary, "Parameter contentObjectDictionary must be non-null");
    trapIllegalValueIf(ccnxTlvDictionary_GetSchemaVersion(dictionary) != CCNxTlvDictionary_SchemaVersion_V1,
                       "Wrong schema version, expected %d got %d",
                       CCNxTlvDictionary_SchemaVersion_V1,
                       ccnxTlvDictionary_GetSchemaVersion(dictionary));
}

bool
ccnxChunkingFacadeV1_HasEndChunkNumber(const CCNxTlvDictionary *contentObjectDictionary)
{
    _assertInvariants(contentObjectDictionary);
    return ccnxTlvDictionary_IsValueInteger(contentObjectDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
}

uint64_t
ccnxChunkingFacadeV1_GetEndChunkNumber(const CCNxTlvDictionary *contentObjectDictionary)
{
    _assertInvariants(contentObjectDictionary);
    return ccnxTlvDictionary_GetInteger(contentObjectDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT);
}

bool
ccnxChunkingFacadeV1_SetEndChunkNumber(CCNxTlvDictionary *contentObjectDictionary, uint64_t endChunkNumber)
{
    _assertInvariants(contentObjectDictionary);
    trapIllegalValueIf(!ccnxTlvDictionary_IsContentObject(contentObjectDictionary), "Dictionary is not a ContentObject");
    return ccnxTlvDictionary_PutInteger(contentObjectDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT, endChunkNumber);
}
