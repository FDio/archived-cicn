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
 * @header metis_MatchingRulesTable
 * @abstract A generic table (void *) that matches a MetisMessage according to the CCNx 1.0 rules
 * @discussion
 *     Matching is done based on Name, Name + KeyId, or Name + ContentObjectHash.
 *     The table key is always a MetisMessage.
 *
 *     When used in the PIT, one calls <code>metisMatchingRulesTable_AddToBestTable()</code> to
 *     add an interest to the "best" (i.e. most restrictive match) table, then calls
 *     <code>metisMatchingRulesTable_GetUnion()</code> on a content object to match against
 *     all of them.
 *
 *     When used in a ContentStore, one calls <code>metisMatchingRulesTable_AddToAllTables()</code>
 *     to index a Content Object in all the tables.  one then calls <code>metisMatchingRulesTable_Get()</code>
 *     with an Interest to do the "best" matching (i.e by hash first, then keyid, then just by name).
 *
 */

#ifndef Metis_metis_MatchingRulesTable_h
#define Metis_metis_MatchingRulesTable_h

#include <parc/algol/parc_HashCodeTable.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <parc/algol/parc_ArrayList.h>

struct metis_matching_rules_table;
typedef struct metis_matching_rules_table MetisMatchingRulesTable;

/**
 * Creates a MetisMatchigRulesTable and specifies the function to call to de-allocate an entry
 *
 * The datadestroyer will be called when an entry is removed from a table. It may be NULL.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMatchingRulesTable *metisMatchingRulesTable_Create(PARCHashCodeTable_Destroyer dataDestroyer);

/**
 * Destroys the table and removes all stored elements.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMatchingRulesTable_Destroy(MetisMatchingRulesTable **tablePtr);

/**
 * @function metisMatchingRulesTable_Get
 * @abstract Returns the data item that best matches the message.
 * @discussion
 *   Indexed by NameAndContentObjectHash, NameAndKeyId, and Name, in that order.
 *
 * @param <#param1#>
 * @return NULL if nothing matches, otherwise the stored value
 */
void *metisMatchingRulesTable_Get(const MetisMatchingRulesTable *table, const MetisMessage *message);

/**
 * @function metisMatchingRulesTable_GetUnion
 * @abstract Returns matching data items from all index tables.
 * @discussion
 *   The PARCArrayList does not have an item destructor, so destroying it will not affect
 *   the underlying data.
 *
 *
 * @param <#param1#>
 * @return Will not be NULL, but may be empty
 */
PARCArrayList *metisMatchingRulesTable_GetUnion(const MetisMatchingRulesTable *table, const MetisMessage *message);

/**
 * @function metisMatchingRulesTable_Add
 * @abstract Adds the data to the best table
 * @discussion
 *   The key must be derived from the data and destroyed when the data is destroyed.  Only the data
 *   destroyer is called.
 *
 *   No duplicates are allowed, will return false if not added.
 *
 * @param <#param1#>
 * @return true if unique key and added, false if duplicate and no action taken.
 */
bool metisMatchingRulesTable_AddToBestTable(MetisMatchingRulesTable *rulesTable, MetisMessage *key, void *data);

/**
 * @function metisMatchingRulesTable_AddToAllTables
 * @abstract Adds the key and data to all tables
 * @discussion
 *   duplicates are not added
 *
 * @param <#param1#>
 */
void metisMatchingRulesTable_AddToAllTables(MetisMatchingRulesTable *rulesTable, MetisMessage *key, void *data);

/**
 * @function metisMatchingRulesTable_Remove
 * @abstract Removes the matching entry from the best match table, calling the destroyer on the data.
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisMatchingRulesTable_RemoveFromBest(MetisMatchingRulesTable *rulesTable, const MetisMessage *message);

/**
 * @function metisMatchingRulesTable_RemoveFromAll
 * @abstract Removes the message from all tables
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMatchingRulesTable_RemoveFromAll(MetisMatchingRulesTable *rulesTable, const MetisMessage *message);
#endif // Metis_metis_MatchingRulesTable_h
