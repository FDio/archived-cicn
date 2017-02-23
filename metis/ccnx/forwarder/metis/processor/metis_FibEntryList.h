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
 * @file metis_FibEntryList.h
 * @brief A typesafe list of MetisFibEntry
 *
 * <#Detailed Description#>
 *
 */

#ifndef Metis_metis_FibEntryList_h
#define Metis_metis_FibEntryList_h

#include <ccnx/forwarder/metis/processor/metis_FibEntry.h>

struct metis_fib_entry_list;
typedef struct metis_fib_entry_list MetisFibEntryList;

/**
 * Creates an emtpy FIB entry list
 *
 * Must be destroyed with metisFibEntryList_Destroy.
 *
 * @retval non-null An allocated MetisFibEntryList
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisFibEntryList *metisFibEntryList_Create(void);

/**
 * @function MetisFibEntryList_Detroy
 * @abstract Destroys the list and all entries.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisFibEntryList_Destroy(MetisFibEntryList **listPtr);

/**
 * @function metisFibEntryList_Append
 * @abstract Will store a reference counted copy of the entry.
 * @discussion
 *   Will create and store a reference counted copy.  You keep ownership
 *   of the parameter <code>fibEntry</code>.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisFibEntryList_Append(MetisFibEntryList *list, MetisFibEntry *fibEntry);

/**
 * Returns the number of entries in the list
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] list An allocated MetisFibEntryList
 *
 * @retval number The number of entries in the list
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t metisFibEntryList_Length(const MetisFibEntryList *list);

/**
 * @function metisFibEntryList_Get
 * @abstract Gets an element.  This is the internal reference, do not destroy.
 * @discussion
 *   Returns an internal reference from the list.  You must not destroy it.
 *   Will assert if you go off the end of the list.
 *
 * @param <#param1#>
 * @return <#return#>
 */
const MetisFibEntry *metisFibEntryList_Get(const MetisFibEntryList *list, size_t index);
#endif // Metis_metis_FibEntryList_h
