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
 * @file metis_PIT.h
 * @brief The Pending Interest Table interface
 *
 * Interface for implementing a PIT table
 *
 */

#ifndef Metis_metis_PIT_h
#define Metis_metis_PIT_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/core/metis_NumberSet.h>
#include <ccnx/forwarder/metis/processor/metis_PitEntry.h>
#include <ccnx/forwarder/metis/processor/metis_PITVerdict.h>

struct metis_pit;
typedef struct metis_pit MetisPIT;

struct metis_pit {
    void (*release)(MetisPIT **pitPtr);
    MetisPITVerdict (*receiveInterest)(MetisPIT *pit, MetisMessage *interestMessage);
    MetisNumberSet * (*satisfyInterest)(MetisPIT * pit, const MetisMessage * objectMessage);
    void (*removeInterest)(MetisPIT *pit, const MetisMessage *interestMessage);
    MetisPitEntry * (*getPitEntry)(const MetisPIT * pit, const MetisMessage * interestMessage);
    void *closure;
};

void *metisPIT_Closure(const MetisPIT *pit);

/**
 * Destroys the PIT table and all entries contained in it.
 *
 * PIT entries are reference counted, so if the user has stored one outside the PIT table
 * it will still be valid.
 *
 * @param [in,out] pitPtr Double pointer to PIT table, will be NULLed
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisPIT_Release(MetisPIT **pitPtr);

/**
 * @function metisPit_ReceiveInterest
 * @abstract Receives an interest and adds to PIT table
 * @discussion
 *   If not present, adds entry to the PIT table and returns PIT_VERDICT_NEW_ENTRY.
 *   If present and aggregated, returns PIT_VERDICT_EXISTING_ENTRY.
 *
 *   Some aggregated interests may return PIT_VERDICT_NEW_ENTRY if the interest needs
 *   to be forwarded again (e.g. the lifetime is extended).
 *
 *   If the PIT stores the message in its table, it will store a reference counted copy.
 *
 * @param <#param1#>
 * @return Verdict of receiving the interest
 */
MetisPITVerdict metisPIT_ReceiveInterest(MetisPIT *pit, MetisMessage *interestMessage);

/**
 * @function metisPit_SatisfyInterest
 * @abstract Tries to satisfy PIT entries based on the message, returning where to send message
 * @discussion
 *     If matching interests are in the PIT, will return the set of reverse paths to use
 *     to forward the content object.
 *
 *     The return value is allocated and must be destroyed.
 *
 * @param <#param1#>
 * @return Set of ConnectionTable id's to forward the message, may be empty or NULL.  Must be destroyed.
 */
MetisNumberSet *metisPIT_SatisfyInterest(MetisPIT *pit, const MetisMessage *objectMessage);

/**
 * @function metisPit_RemoveInterest
 * @abstract Unconditionally remove the interest from the PIT
 * @discussion
 *   The PIT may store a specific name in several tables.  This function will
 *   remove the interest from the specific table it lives it.  It will not
 *   remove PIT entries in different tables with the same name.
 *
 *   The different tables index interests based on their matching criteria,
 *   such as by name, by name and keyid, etc.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisPIT_RemoveInterest(MetisPIT *pit, const MetisMessage *interestMessage);

/**
 * @function metisPit_GetPitEntry
 * @abstract Retrieve the best matching PIT entry for the message.
 * @discussion
 *   Returns a reference counted copy of the entry, must call <code>metisPitEntry_Destory()</code> on it.
 *
 * @param <#param1#>
 * @return NULL if not in table, otherwise a reference counted copy of the entry
 */
MetisPitEntry *metisPIT_GetPitEntry(const MetisPIT *pit, const MetisMessage *interestMessage);
#endif // Metis_metis_PIT_h
