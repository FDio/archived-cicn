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
 * @file parc_CryptoCache.h
 * @ingroup security
 * @brief In-memory cache of keys or certificates.
 *
 * Not sure how to differentiate between keys and certs at the moment.  The current API
 * is thus built around keys.
 *
 */
#include <parc/security/parc_Key.h>

#ifndef libparc_parc_CryptoCache_h
#define libparc_parc_CryptoCache_h

struct parc_crypto_cache;
typedef struct parc_crypto_cache PARCCryptoCache;

PARCCryptoCache *parcCryptoCache_Create(void);

/**
 * Destroys the cache and all internal buffers.
 *
 * @param [in,out] cryptoCachePtr A pointer to a pointer to a `PARCCryptoCache` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcCryptoCache_Destroy(PARCCryptoCache **cryptoCachePtr);

/**
 * Adds the specified key to the keycache.
 *
 * Parameters must be non-null
 * Returns true if added or false if keyid alredy existed and was a different than <code>key</code>
 * This will store its own reference to the key, so the caller must free key.
 *
 * @param [in] cache A pointer to a PARCCryptoCache instance.
 *
 * Example:
 * @code
 * {
 *      PARCKey *key = ....;
 *      PARCCryptoCache *cache = parcCryptoCache_Create();
 *      parcCryptoCache_AddKey(cache, key);
 *      parcKey_release(&key);
 *      // do stuff with the crypto cache
 *      parcCryptoCache_Destroy(&cache);
 * }
 * @endcode
 */
bool parcCryptoCache_AddKey(PARCCryptoCache *cache, PARCKey *key);

/**
 * Fetches the Key.  The user must not modify or destroy the key.
 *
 * Returns NULL if the keyid is not found.
 *
 * @param [in] cache A pointer to a PARCCryptoCache instance.
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const PARCKey *parcCryptoCache_GetKey(PARCCryptoCache *cache, const PARCKeyId *keyid);

/**
 * Removes the keyid and key.  The internal buffers are destroyed.
 *
 * @param [in] cache A pointer to a PARCCryptoCache instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcCryptoCache_RemoveKey(PARCCryptoCache *cache, const PARCKeyId *keyid);
#endif // libparc_parc_CryptoCache_h
