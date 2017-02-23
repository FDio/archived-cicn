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


#ifndef Metis_metis_ContentStore_h
#define Metis_metis_ContentStore_h

#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/core/metis_Logger.h>

struct metis_contentstore;
typedef struct metis_contentstore MetisContentStore;

MetisContentStore *metisContentStore_Create(size_t objectCapacity, MetisLogger *logger);
void metisContentStore_Destroy(MetisContentStore **storePtr);

/**
 * @function metisContentStore_Save
 * @abstract Saves content object in the store
 * @discussion
 *   Will make a reference counted copy of the message, so caller retains ownership of original message.
 *
 * @param <#param1#>
 * @return True if saved, false othewise
 */

bool metisContentStore_Save(MetisContentStore *store, MetisMessage *objectMessage);

/**
 * @function metisContentStore_Fetch
 * @abstract Fetch a content object from the store that matches the interest message
 * @discussion
 *   Returns a reference counted copy, caller must Destroy it.
 *
 * @param <#param1#>
 * @return May be NULL if no match
 */
MetisMessage *metisContentStore_Fetch(MetisContentStore *store, MetisMessage *interestMessage);

#endif // Metis_metis_ContentStore_h
