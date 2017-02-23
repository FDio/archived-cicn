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
 * @file metis_StandardPIT.h
 * @brief The Pending Interest Table
 *
 * Implements the standard Pending Interest Table.
 *
 */

#ifndef Metis_metis_StandardPIT_h
#define Metis_metis_StandardPIT_h

#include <ccnx/forwarder/metis/processor/metis_PIT.h>

/**
 * Creates a PIT table
 *
 * Creates and allocates an emtpy PIT table.  The MetisForwarder reference is
 * used for logging and for time functions.
 *
 * @param [in] metis The releated MetisForwarder
 *
 * @return non-null a PIT table
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisPIT *metisStandardPIT_Create(MetisForwarder *metis);
#endif // Metis_metis_PIT_h
