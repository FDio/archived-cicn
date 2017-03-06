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

#ifndef ccnxPingCommon_h
#define ccnxPingCommon_h

#include <stdint.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

/**
 * The `CCNxName` prefix for the server.
 */
#define ccnxPing_DefaultPrefix "ccnx:/localhost"

/**
 * The default client receive timeout (in microseconds).
 */
extern const size_t ccnxPing_DefaultReceiveTimeoutInUs;

/**
 * The default size of a content object payload.
 */
extern const size_t ccnxPing_DefaultPayloadSize;

/**
 * The maximum size of a content object payload.
 * 64KB is the limit imposed by the packet structure.
 * Here we limit the max Payload Size to 1400 bytes.
 */
#define ccnxPing_MaxPayloadSize 1400

/**
 * A default "medium" number of messages to send.
 */
extern const size_t mediumNumberOfPings;

/**
 * A default "small" number of messages to send.
 */
extern const size_t smallNumberOfPings;

/**
 * Initialize and return a new instance of CCNxPortalFactory. A randomly generated identity is
 * used to initialize the factory. The returned instance must eventually be released by calling
 * ccnxPortalFactory_Release().
 *
 * @param [in] keystoreName The name of the file to save the new identity.
 * @param [in] keystorePassword The password of the file holding the identity.
 * @param [in] subjectName The name of the owner of the identity.
 *
 * @return A new instance of a CCNxPortalFactory initialized with a randomly created identity.
 */
CCNxPortalFactory *ccnxPingCommon_SetupPortalFactory(const char *keystoreName,
                                                     const char *keystorePassword,
                                                     const char *subjectName);
#endif // ccnxPingCommon_h.h
