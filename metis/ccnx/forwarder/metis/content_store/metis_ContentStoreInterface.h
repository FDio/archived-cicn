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


#ifndef Metis_metis_ContentStoreInterface_h
#define Metis_metis_ContentStoreInterface_h

#include <stdio.h>

#include <ccnx/forwarder/metis/core/metis_Message.h>

typedef struct metis_contentstore_config {
    size_t objectCapacity;
} MetisContentStoreConfig;

typedef struct metis_contentstore_interface MetisContentStoreInterface;

struct metis_contentstore_interface {
    /**
     * Place a MetisMessage representing a ContentObject into the ContentStore. If necessary to make room,
     * remove expired content or content that has exceeded the Recommended Cache Time.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     * @param content - a pointer to a `MetisMessage` to place in the store.
     * @param currentTimeTicks - the current time, in metis ticks, since the UTC epoch.
     */
    bool (*putContent)(MetisContentStoreInterface *storeImpl, MetisMessage *content, uint64_t currentTimeTicks);

    /**
     * The function to call to remove content from the ContentStore.
     * It will Release any references that were created when the content was placed into the ContentStore.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     * @param content - a pointer to a `MetisMessage` to remove from the store.
     */
    bool (*removeContent)(MetisContentStoreInterface *storeImpl, MetisMessage *content);

    /**
     * Given a MetisMessage that represents and Interest, try to find a matching ContentObject. Matching is
     * done on a most-restrictive basis.
     *
     *  a) If the interest has a ContentObjectHash restriction, it will match on the Name and the Object Hash.
     *  b) If it has a KeyId, it will match on the Name and the KeyId
     *  c) otherwise, it matches by Name
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     * @param interest - a pointer to a `MetisMessage` representing the Interest to match.
     *
     * @return a pointer to a MetisMessage containing the matching ContentObject
     * @return NULL if no matching ContentObject was found
     */
    MetisMessage * (*matchInterest)(MetisContentStoreInterface*storeImpl, MetisMessage *interest);

    /**
     * Return the maximum number of ContentObjects that can be stored in this ContentStore. This is a raw
     * count, not based on memory size.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     *
     * @return the maximum number of ContentObjects that can be stored
     */
    size_t (*getObjectCapacity)(MetisContentStoreInterface *storeImpl);

    /**
     * Return the number of ContentObjects currently stored in the ContentStore.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     *
     * @return the current number of ContentObjects in the ContentStore
     */
    size_t (*getObjectCount)(MetisContentStoreInterface *storeImpl);

    /**
     * Loga ContentStore implementation specific version of store-related information.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     */
    void (*log)(MetisContentStoreInterface *storeImpl);

    /**
     * Acquire a new reference to the specified ContentStore instance. This reference will eventually need
     * to be released by calling {@link metisContentStoreInterface_Release}.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     */
    MetisContentStoreInterface *(*acquire)(const MetisContentStoreInterface *storeImpl);

    /**
     * Release the ContentStore, which will also Release any references held by it.
     *
     * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
     */
    void (*release)(MetisContentStoreInterface **storeImpl);

    /**
     * A pointer to opaque private data used by the ContentStore instance represented by this instance of
     * MetisContentStoreInterface.
     */
    void *_privateData;
};

/**
 * Place a MetisMessage representing a ContentObject into the ContentStore. If necessary to make room,
 * remove expired content or content that has exceeded the Recommended Cache Time.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 * @param content - a pointer to a `MetisMessage` to place in the store.
 *
 * @param currentTimeTicks - the current time, in metis ticks, since the UTC epoch.
 */
bool metisContentStoreInterface_PutContent(MetisContentStoreInterface *storeImpl, MetisMessage *content, uint64_t currentTimeTicks);

/**
 * The function to call to remove content from the ContentStore.
 * It will Release any references that were created when the content was placed into the ContentStore.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 * @param content - a pointer to a `MetisMessage` to remove from the store.
 */
bool metisContentStoreInterface_RemoveContent(MetisContentStoreInterface *storeImpl, MetisMessage *content);

/**
 * Given a MetisMessage that represents and Interest, try to find a matching ContentObject. Matching is
 * done on a most-restrictive basis.
 *
 *  a) If the interest has a ContentObjectHash restriction, it will match on the Name and the Object Hash.
 *  b) If it has a KeyId, it will match on the Name and the KeyId
 *  c) otherwise, it matches by Name
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 * @param interest - a pointer to a `MetisMessage` representing the Interest to match.
 *
 * @return a pointer to a MetisMessage containing the matching ContentObject
 * @return NULL if no matching ContentObject was found
 */
MetisMessage *metisContentStoreInterface_MatchInterest(MetisContentStoreInterface*storeImpl, MetisMessage *interest);

/**
 * Return the maximum number of ContentObjects that can be stored in this ContentStore. This is a raw
 * count, not based on memory size.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 *
 * @return the maximum number of ContentObjects that can be stored
 */
size_t metisContentStoreInterface_GetObjectCapacity(MetisContentStoreInterface *storeImpl);

/**
 * Return the number of ContentObjects currently stored in the ContentStore.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 *
 * @return the current number of ContentObjects in the ContentStore
 */
size_t metisContentStoreInterface_GetObjectCount(MetisContentStoreInterface *storeImpl);

/**
 * Loga ContentStore implementation specific version of store-related information.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 */
void metisContentStoreInterface_Log(MetisContentStoreInterface *storeImpl);

/**
 * Acquire a new reference to the specified ContentStore instance. This reference will eventually need
 * to be released by calling {@link metisContentStoreInterface_Release}.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 */
MetisContentStoreInterface *metisContentStoreInterface_Aquire(const MetisContentStoreInterface *storeImpl);

/**
 * Release the ContentStore, which will also Release any references held by it.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 */
void metisContentStoreInterface_Release(MetisContentStoreInterface **storeImplPtr);

/**
 * Return a pointer to the data private to this implementation of the ContentStore interface.
 *
 * @param storeImpl - a pointer to this MetisContentStoreInterface instance.
 */
void *metisContentStoreInterface_GetPrivateData(MetisContentStoreInterface *storeImpl);
#endif // Metis_metis_ContentStoreInterface_h
