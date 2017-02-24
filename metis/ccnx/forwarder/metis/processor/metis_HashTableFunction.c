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
#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Hash.h>

#include <ccnx/forwarder/metis/processor/metis_HashTableFunction.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>

#include <LongBow/runtime.h>

// ======================================================================
// Hash table key functions
// We use a MetisMessage as the key data type

bool
metisHashTableFunction_MessageNameEquals(const void *keyA, const void *keyB)
{
    const MetisMessage *a = (const MetisMessage  *) keyA;
    const MetisMessage *b = (const MetisMessage  *) keyB;

    return metisTlvName_Equals(metisMessage_GetName(a), metisMessage_GetName(b));
}

HashCodeType
metisHashTableFunction_MessageNameHashCode(const void *keyA)
{
    const MetisMessage *message = (const MetisMessage *) keyA;
    MetisTlvName *name = metisMessage_GetName(message);

    // we want the cumulative hash for the whole name
    uint32_t hash = metisTlvName_HashCode(name);

    return hash;
}

bool
metisHashTableFunction_MessageNameAndKeyIdEquals(const void *keyA, const void *keyB)
{
    const MetisMessage *a = (const MetisMessage  *) keyA;
    const MetisMessage *b = (const MetisMessage  *) keyB;

    if (metisMessage_KeyIdEquals(a, b)) {
        if (metisTlvName_Equals(metisMessage_GetName(a), metisMessage_GetName(b))) {
            return true;
        }
    }
    return false;
}

HashCodeType
metisHashTableFunction_MessageNameAndKeyIdHashCode(const void *keyA)
{
    const MetisMessage *message = (const MetisMessage  *) keyA;

    uint32_t keyIdHash;

    bool hasKeyId = metisMessage_GetKeyIdHash(message, &keyIdHash);
    assertTrue(hasKeyId, "Called NameAndKeyIdHashCode for a message without a keyid");

    // we want the cumulative hash for the whole name
    MetisTlvName *name = metisMessage_GetName(message);
    uint32_t nameHash = metisTlvName_HashCode(name);

    // now combine the two hashes.  The KeyId hash is mixed in to the name hash.
    uint32_t hash = parcHash32_Data_Cumulative(&keyIdHash, sizeof(keyIdHash), nameHash);
    return hash;
}

bool
metisHashTableFunction_MessageNameAndObjectHashEquals(const void *keyA, const void *keyB)
{
    const MetisMessage *a = (const MetisMessage  *) keyA;
    const MetisMessage *b = (const MetisMessage  *) keyB;

    // due to lazy calculation of hash in content objects, need non-const
    if (metisMessage_ObjectHashEquals((MetisMessage  *) a, (MetisMessage  *) b)) {
        if (metisTlvName_Equals(metisMessage_GetName(a), metisMessage_GetName(b))) {
            return true;
        }
    }
    return false;
}

HashCodeType
metisHashTableFunction_MessageNameAndObjectHashHashCode(const void *keyA)
{
    const MetisMessage *message = (const MetisMessage  *) keyA;

    uint32_t contentObjectHashHash;

    bool hasObjectHash = metisMessage_GetContentObjectHashHash((MetisMessage *) message, &contentObjectHashHash);
    assertTrue(hasObjectHash, "Called metisPit_NameAndObjectHashHashCode for an interest without a ContentObjectHash restriction");

    // we want the cumulative hash for the whole name
    MetisTlvName *name = metisMessage_GetName(message);
    uint32_t nameHash = metisTlvName_HashCode(name);

    // now combine the two hashes
    uint32_t hash = parcHash32_Data_Cumulative(&contentObjectHashHash, sizeof(contentObjectHashHash), nameHash);
    return hash;
}

// ======================================================================
// TlvName variety

bool
metisHashTableFunction_TlvNameEquals(const void *keyA, const void *keyB)
{
    const MetisTlvName *a = (const MetisTlvName  *) keyA;
    const MetisTlvName *b = (const MetisTlvName  *) keyB;

    return metisTlvName_Equals(a, b);
}

int
metisHashTableFunction_TlvNameCompare(const void *keyA, const void *keyB)
{
    const MetisTlvName *a = (const MetisTlvName  *) keyA;
    const MetisTlvName *b = (const MetisTlvName  *) keyB;

    return metisTlvName_Compare(a, b);
}

HashCodeType
metisHashTableFunction_TlvNameHashCode(const void *keyA)
{
    MetisTlvName *name = (MetisTlvName *) keyA;

    // we want the cumulative hash for the whole name
    uint32_t hash = metisTlvName_HashCode(name);

    return hash;
}
