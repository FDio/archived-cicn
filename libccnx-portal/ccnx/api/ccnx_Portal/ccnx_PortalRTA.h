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
 * @file ccnx_PortalRTA.h
 * @brief The CCNxPortalStack representation of an "RTA" Transport Stack
 *
 */
#ifndef __CCNx_Portal_API__ccnx_PortalRTA__
#define __CCNx_Portal_API__ccnx_PortalRTA__

#include <ccnx/api/ccnx_Portal/ccnx_PortalAttributes.h>
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

/**
 * Specification for an "RTA" Tranport Stack configured for Message-by-Message interaction.
 *
 * @param [in] factory A pointer to a valid {@link CCNxPortalFactory} instance.
 * @param [in] attributes A pointer to a valid {@link CCNxPortalAttributes} instance.
 *
 * @return non-NULL A pointer to a valid {@link CCNxPortal} instance bound to the specified Transport Stack.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Message);
 * }
 * @endcode
 */
CCNxPortal *ccnxPortalRTA_Message(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes);

/**
 * Specification for an "RTA" Tranport Stack configured for Chunked interaction.
 *
 * The flow of inbound Content Objects is initiated by the first `CCNxInterest` specifying a valid Chunked protocol.
 *
 * @param [in] factory A pointer to a valid {@link CCNxPortalFactory} instance.
 * @param [in] attributes A pointer to a valid {@link CCNxPortalAttributes} instance.
 *
 * @return non-NULL A pointer to a valid {@link CCNxPortal} instance bound to the specified Transport Stack.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Chunked);
 * }
 * @endcode
 */
CCNxPortal *ccnxPortalRTA_Chunked(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes);

/**
 * Specification for an "RTA" Tranport Stack configured for a loopback, Message-by-Message interaction.
 *
 * The loopback causes all messages sent to be reflected back to be received.
 *
 * @param [in] factory A pointer to a valid {@link CCNxPortalFactory} instance.
 * @param [in] attributes A pointer to a valid {@link CCNxPortalAttributes} instance.
 *
 * @return non-NULL A pointer to a valid {@link CCNxPortal} instance bound to the specified Transport Stack.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 * }
 * @endcode
 */
CCNxPortal *ccnxPortalRTA_LoopBack(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes);

#endif /* defined(__CCNx_Portal_API__ccnx_PortalRTA__) */
