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
 * @file ccnx_PortalAPI.h
 * @brief A Portal Protocol Stack implementation that simulates a real network stack.
 *
 * This Portal Protocol Stack implementation simulates a network stack for the purposes of testing and development.
 *
 */
#ifndef __CCNx_Portal_API__ccnxPortalAPI__
#define __CCNx_Portal_API__ccnxPortalAPI__

#include <ccnx/api/ccnx_Portal/ccnx_PortalAttributes.h>
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

/**
 * Create a {@link CCNxPortal} instance from the given @p factory and @p attributes.
 *
 * @param [in] factory A pointer to a valid {@link CCNxPortalFactory} instance
 * @param [in] attributes A pointer to a valid {@link CCNxPortalAttributes} instance
 *
 * @return non-null A pointer to a valid `CCNxPortal` instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxPortal *ccnxPortalAPI_LoopBack(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes);

#endif /* defined(__CCNx_Portal_API__ccnxPortalAPI__) */
