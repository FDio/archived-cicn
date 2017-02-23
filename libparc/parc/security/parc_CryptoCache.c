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
 * @header parc_CryptoCache.c
 * <#Abstract#>
 *
 *     <#Discussion#>
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
/*
 * This should be updated to make a reference counted copy of PARCKey and store that (along with its KeyId)
 * instead of using a direct copy of the user data.  That way, there's no issues about destroying the entry
 * but the user retaining a (now invalid) reference to it.
 *
 */

#include <config.h>
#include <stdio.h>
#include <string.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_CryptoCache.h>
#include <parc/algol/parc_HashCodeTable.h>

struct parc_crypto_cache {
    PARCHashCodeTable *keyid_table;
};

// =====================================================================
// Translations from void* to typed pointer for use in HashCodeTable

static bool
_keyidEquals(const void *ptrA, const void *ptrB)
{
    return parcKeyId_Equals((const PARCKeyId *) ptrA, (const PARCKeyId *) ptrB);
}

static void
_dataDestroy(void **voidPtr)
{
    PARCKey **keyPtr = (PARCKey **) voidPtr;
    parcKey_Release(keyPtr);
}

// =====================================================================

PARCCryptoCache *
parcCryptoCache_Create()
{
    PARCCryptoCache *cache = parcMemory_AllocateAndClear(sizeof(PARCCryptoCache));
    assertNotNull(cache, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCCryptoCache));

    // KeyIdDestroyer is NULL because we get the keyid out of the key, and it will be destroyed
    // when the key is destroyed.
    cache->keyid_table = parcHashCodeTable_Create(_keyidEquals, parcKeyId_HashCodeFromVoid, NULL, _dataDestroy);

    return cache;
}

/**
 * Destroys the cache and all internal buffers.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
parcCryptoCache_Destroy(PARCCryptoCache **cryptoCachePtr)
{
    assertNotNull(cryptoCachePtr, "Parameter must be non-null double pointer");
    assertNotNull(*cryptoCachePtr, "Parameter must dereference to non-null pointer");

    PARCCryptoCache *cache = *cryptoCachePtr;
    parcHashCodeTable_Destroy(&cache->keyid_table);
    parcMemory_Deallocate((void **) cryptoCachePtr);
    *cryptoCachePtr = NULL;
}

/**
 * Adds the specified key to the keycache.
 *
 * Parameters must be non-null
 * Returns true if added or false if keyid alredy existed and was a different than <code>key</code>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool
parcCryptoCache_AddKey(PARCCryptoCache *cache, PARCKey *original_key)
{
    assertNotNull(cache, "Parameter cache must be non-null");
    assertNotNull(original_key, "Parameter key must be non-null");

    PARCKey *key = parcKey_Copy(original_key);
    PARCKeyId *keyid = parcKey_GetKeyId(key);

    return parcHashCodeTable_Add(cache->keyid_table, keyid, key);
}

/**
 * Fetches the Key.  The user must node modify or destroy the key.
 *
 * Returns NULL if the keyid is not found.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const PARCKey *
parcCryptoCache_GetKey(PARCCryptoCache *cache, const PARCKeyId *keyid)
{
    assertNotNull(cache, "Parameter cache must be non-null");
    assertNotNull(keyid, "Parameter keyid must be non-null");

    return parcHashCodeTable_Get(cache->keyid_table, keyid);
}

/**
 * Removes the keyid and key.  The internal buffers are destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
parcCryptoCache_RemoveKey(PARCCryptoCache *cache, const PARCKeyId *keyid)
{
    assertNotNull(cache, "Parameter cache must be non-null");
    assertNotNull(keyid, "Parameter keyid must be non-null");

    parcHashCodeTable_Del(cache->keyid_table, keyid);
}
