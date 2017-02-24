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
 * @file metis_PitEntry.h
 * @brief The embodiment of a PIT entry
 *
 * Embodies a PIT entry
 *
 */

#ifndef Metis_metis_PitEntry_h
#define Metis_metis_PitEntry_h

#include <ccnx/forwarder/metis/core/metis_Ticks.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>
#include <ccnx/forwarder/metis/core/metis_NumberSet.h>
#include <ccnx/forwarder/metis/processor/metis_FibEntry.h>

struct metis_pit_entry;
typedef struct metis_pit_entry MetisPitEntry;

/**
 * @function metisPitEntry_Create
 * @abstract Takes ownership of the message inside the PitEntry
 * @discussion
 *   When the PIT entry is destroyed, will call <code>metisMessage_Release()</code> on the message.
 *
 * @param <#param1#>
 * @return <#return#>
 */
MetisPitEntry *metisPitEntry_Create(MetisMessage *message, MetisTicks expiryTime, MetisTicks CreationTime);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] pitEntryPtr A pointer to a MetisPitEntry instance pointer, which will be set to zero on return.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
void metisPitEntry_Release(MetisPitEntry **pitEntryPtr);

/**
 * @function metisPitEntry_Acquire
 * @abstract Returns a reference counted copy
 * @discussion
 *   A reference counted copy that shares the same state as the original.
 *   Caller must use <code>metisPitEntry_Release()</code> on it when done.
 *
 * @return A reference counted copy, use Destroy on it.
 */
MetisPitEntry *metisPitEntry_Acquire(MetisPitEntry *original);

/**
 * @function metisPitEntry_AddIngressId
 * @abstract Add an ingress connection id to the list of reverse paths
 * @discussion
 *   A PitEntry has two NumberSets.  The first is the set of ingress ports, which
 *   make up the reverse path.  The second is the set of egress ports, which make up
 *   its forward path.
 *
 *   This function tracks which reverse paths have sent us the interest.
 *
 * @param ingressId the reverse path
 */
void  metisPitEntry_AddIngressId(MetisPitEntry *pitEntry, unsigned ingressId);

/**
 * @function metisPitEntry_AddEgressId
 * @abstract Add an egress connection id to the list of attempted paths
 * @discussion
 *   A PitEntry has two NumberSets.  The first is the set of ingress ports, which
 *   make up the reverse path.  The second is the set of egress ports, which make up
 *   its forward path.
 *
 *   This function tracks which forward paths we've tried for the interest.
 *
 * @param egressId the forwarded path
 */
void  metisPitEntry_AddEgressId(MetisPitEntry *pitEntry, unsigned egressId);

void metisPitEntry_AddFibEntry(MetisPitEntry *pitEntry, MetisFibEntry *fibEntry);
MetisFibEntry *metisPitEntry_GetFibEntry(MetisPitEntry *pitEntry);

/**
 * @function metisPitEntry_GetIngressSet
 * @abstract The Ingress connection id set
 * @discussion
 *   You must acquire a copy of the number set if you will store the result.  This is
 *   the internal reference.
 *
 * @param <#param1#>
 * @return May be empty, will not be null.  Must be destroyed.
 */
const MetisNumberSet *metisPitEntry_GetIngressSet(const MetisPitEntry *pitEntry);

/**
 * @function metisPitEntry_GetEgressSet
 * @abstract The Egress connection id set
 * @discussion
 *   You must acquire a copy of the number set if you will store the result.  This is
 *   the internal reference.
 *
 * @param <#param1#>
 * @return May be empty, will not be null.  Must be destroyed.
 */
const MetisNumberSet *metisPitEntry_GetEgressSet(const MetisPitEntry *pitEntry);

/**
 * @function metisPitEntry_GetMessage
 * @abstract Gets the interest underpinning the PIT entry
 * @discussion
 *   A reference counted copy, call <code>MetisMessage_Release()</code> on it.
 *
 * @param <#param1#>
 * @return A reference counted copy, call <code>MetisMessage_Release()</code> on it.
 */
MetisMessage *metisPitEntry_GetMessage(const MetisPitEntry *pitEntry);

/**
 * Returns the time (in ticks) at which the PIT entry is no longer valid
 *
 * The ExpiryTime is computed when the PIT entry is added (or via metisPitEntry_SetExpiryTime).
 * It is the aboslute time (in Ticks) at which the Pit entry is no longer valid.
 *
 * @param [in] MetisPitEntry An allocated PIT entry
 *
 * @retval number The abosolute time (in Ticks) of the Expiry
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisTicks metisPitEntry_GetExpiryTime(const MetisPitEntry *pitEntry);

MetisTicks metisPitEntry_GetCreationTime(const MetisPitEntry *pitEntry);
/**
 * Sets the ExpriyTime of the PIT entry to the given value
 *
 * It is probalby an error to set the expiryTime to a smaller value than currently set to, but
 * this is not enforced.  PIT entries use lazy delete.
 *
 * @param [in] pitEntry The allocated PIT entry to modify
 * @param [in] expiryTime The new expiryTime (UTC in forwarder Ticks)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisPitEntry_SetExpiryTime(MetisPitEntry *pitEntry, MetisTicks expiryTime);

#endif // Metis_metis_PitEntry_h
