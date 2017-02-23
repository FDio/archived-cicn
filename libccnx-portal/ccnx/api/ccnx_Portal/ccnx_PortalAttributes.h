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
 * @file ccnx_PortalAttributes.h
 * @brief Attributes related to the `CCNxPortal` instance.
 *
 */
#ifndef __CCNx_Portal_API__ccnx_PortalAttributes__
#define __CCNx_Portal_API__ccnx_PortalAttributes__

#include <stdbool.h>

struct ccnx_portal_attributes;
/**
 * @brief The attributes for the instance of {@link CCNxPortal}
 */
typedef struct ccnx_portal_attributes CCNxPortalAttributes;

/**
 * Non-blocking (reads)
 */
extern const CCNxPortalAttributes ccnxPortalAttributes_NonBlocking;

/**
 * Return `true` if the given attributes indicate Portal Logging is enabled.
 *
 * @param [in] attributes A pointer to a valid {@link CCNxPortalAttributes} instance.
 *
 * @return `true` If the given attributes indicate Portal Logging is enabled.
 * @return `false` If the given attributes do not indicate Portal Logging is enabled.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxPortalAttributes_IsLogging(const CCNxPortalAttributes *attributes);

#endif /* defined(__CCNx_Portal_API__ccnx_PortalAttributes__) */
