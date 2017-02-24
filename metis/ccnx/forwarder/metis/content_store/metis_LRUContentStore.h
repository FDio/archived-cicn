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


#ifndef Metis_metis_LRUContentStore_h
#define Metis_metis_LRUContentStore_h

#include <stdio.h>
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreInterface.h>
#include <ccnx/forwarder/metis/core/metis_Logger.h>

/**
 * Create and Initialize an instance of MetisLRUContentStore. A newly allocated {@link MetisContentStoreInterface}
 * object is initialized and returned. It must eventually be released by calling {@link metisContentStoreInterface_Release}.
 *
 *
 * @param config An instance of `MetisContentStoreConfig`, specifying options to be applied
 *               by the underlying MetisLRUContentStore instance.
 * @param logger An instance of a {@link MetisLogger} to use for logging content store events.
 *
 * @return a newly created MetisLRUContentStore instance.
 *
 * Example:
 * @code
 * {
 *     MetisContentStoreConfig config = {
 *         .objectCapacity = 10
 *     };
 *
 *     MetisContentStoreInterface *store = metisLRUContentStore_Create(&config, logger);
 *     assertTrue(status, "Expected to init a content store");
 *
 *     store->Display(&store);
 *     metisContentStoreInterface_Release(&store);
 * }
 * @endcode
 * @see MetisContentStoreInterface
 * @see metisContentStoreInterface_Release
 */
MetisContentStoreInterface *metisLRUContentStore_Create(MetisContentStoreConfig *config, MetisLogger *logger);
#endif // Metis_metis_LRUContentStore_h
