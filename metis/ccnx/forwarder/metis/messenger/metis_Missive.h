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
 * @file metis_Missive.h
 * @brief A Missive is a status message sent over a broadcast channel inside Metis
 *
 * Recipients use {@link metisMessenger_Register} to receive missives.  They are
 * broadcast to all recipients.
 *
 */
#ifndef Metis_metis_missive_h
#define Metis_metis_missive_h

#include <ccnx/forwarder/metis/messenger/metis_MissiveType.h>

struct metis_missive;
typedef struct metis_missive MetisMissive;

/**
 * Creates a Missive and sets the reference count to 1
 *
 * A Missive may be sent to listeners of the MetisMessenger to inform them of events on a connection id.
 *
 * @param [in] MetisMissiveType The event type
 * @param [in] connectionid The relevant conneciton id
 *
 * @return non-null A message
 * @retrun null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMissive *metisMissive_Create(MetisMissiveType missiveType, unsigned connectionid);

/**
 * Acquire a reference counted copy
 *
 * Increases the reference count by 1 and returns the original object.
 *
 * @param [in] missive An allocated missive
 *
 * @return non-null The original missive with increased reference count
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMissive *metisMissive_Acquire(const MetisMissive *missive);

/**
 * Releases a reference counted copy.
 *
 *  If it is the last reference, the missive is freed.
 *
 * @param [in,out] missivePtr Double pointer to a missive, will be nulled.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMissive_Release(MetisMissive **missivePtr);

/**
 * Returns the type of the missive
 *
 * Returns the type of event the missive represents
 *
 * @param [in] missive An allocated missive
 *
 * @return MetisMissiveType The event type
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisMissiveType metisMissive_GetType(const MetisMissive *missive);

/**
 * Returns the connection ID of the missive
 *
 * An event is usually associated with a connection id (i.e. the I/O channel
 * that originaged the event).
 *
 * @param [in] missive An allocated missive
 *
 * @return number The relevant connection id.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
unsigned metisMissive_GetConnectionId(const MetisMissive *missive);
#endif // Metis_metis_missive_h
