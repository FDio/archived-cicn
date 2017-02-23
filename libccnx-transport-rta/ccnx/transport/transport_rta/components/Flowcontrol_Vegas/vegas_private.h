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
 * @file vegas_private.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_vegas_private_h
#define Libccnx_vegas_private_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>

typedef uint64_t segnum_t;

struct vegas_session;
typedef struct vegas_session VegasSession;

struct vegas_connection_state;
typedef struct vegas_connection_state VegasConnectionState;

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] fc An allocated Vegas flow controller
 * @param [in] conn The RTA connection owning the flow
 * @param [in] basename The name without a chunk number
 * @param [in] begin The chunk number to begin requesting at
 * @param [in] interestInterface The {@link CCNxInterestInterface} to use to generate new Interests
 * @param [in] lifetime The default lifetime, in milli-seconds, to use for generated Interests
 * @param [in] keyIdRestriction The KeyIdRestriction, if any, from the originating Interest
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
VegasSession *vegasSession_Create(VegasConnectionState *fc, RtaConnection *conn, CCNxName *basename,
                                  segnum_t begin, CCNxInterestInterface *interestInterface, uint32_t lifetime,
                                  PARCBuffer *keyIdRestriction);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void vegasSession_Destroy(VegasSession **sessionPtr);
/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int vegasSession_Start(VegasSession *session);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int vegasSession_Pause(VegasSession *session);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int vegasSession_Resume(VegasSession *session);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int vegasSession_Seek(VegasSession *session, segnum_t absolutePosition);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
int vegasSession_ReceiveContentObject(VegasSession *session, TransportMessage *tm);


/**
 * Tell a session that there was a state change in its connection
 *
 * The caller should ensure that the session's connection is the right one by
 * using {@link vegasSession_GetConnectionId}.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void vegasSession_StateChanged(VegasSession *session);

/**
 * Returns the connection id used by the session
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
unsigned vegasSession_GetConnectionId(VegasSession *session);


/**
 * <#One Line Description#>
 *
 *  Called by a session when it is done
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void vegas_EndSession(VegasConnectionState *fc, VegasSession *session);
#endif // Libccnx_vegas_private_h
