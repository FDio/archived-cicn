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
 * @file metis_Configuration.h
 * @brief Metis configuration, such as in-band commands or CLI
 *
 * Manages all user configuration of the system, such as from the CLI or web interface
 * It remembers the user commands and will be able to write out a config file.
 *
 */

#ifndef Metis_metis_Configuration_h
#define Metis_metis_Configuration_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_configuration;
typedef struct metis_configuration MetisConfiguration;

/**
 * <#One Line Description#>
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
MetisConfiguration *metisConfiguration_Create(MetisForwarder *metis);

/**
 * <#One Line Description#>
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
void metisConfiguration_Destroy(MetisConfiguration **configPtr);

void metisConfiguration_SetupAllListeners(MetisConfiguration *config, uint16_t port, const char *localPath);

/**
 * Receive a CPI control message from the user encapsulated in a MetisMessage
 *
 *   Takes ownership of the message, and will destroy it as needed.
 *
 * @param message is of type CCNX_MSG_CPI.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConfiguration_Receive(MetisConfiguration *config, MetisMessage *message);

/**
 * Receives a CPI control message from the user
 *
 * Processes the message and generates the CPI control response.  The response should always
 * be non-null and must be released by the caller.
 *
 * @param [in] config Allocated MetisConfiguration
 * @param [in] request The CPI Request to process
 * @param [in] ingressId The ingress connection ID, used to track in logging messages
 *
 * @retval CCNxControl The response control message (an ACK, NACK, or Response).
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxControl *metisConfiguration_ReceiveControl(MetisConfiguration *config, CCNxControl *request, unsigned ingressId);

/**
 * <#One Line Description#>
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
PARCJSON *metisConfiguration_GetVersion(MetisConfiguration *config);

/**
 * <#One Line Description#>
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
void metisConfiguration_StartCLI(MetisConfiguration *config, uint16_t port);

/**
 * Returns the configured size of the content store
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
size_t metisConfiguration_GetObjectStoreSize(MetisConfiguration *config);

/**
 * Sets the size of the content store (in objects, not bytes)
 *
 * Must be set before starting the forwarder
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void   metisConfiguration_SetObjectStoreSize(MetisConfiguration *config, size_t maximumContentObjectCount);

const char * metisConfiguration_GetForwarginStrategy(MetisConfiguration *config, const CCNxName * prefix);

/**
 * Returns the MetisForwarder that owns the MetisConfiguration
 *
 * Returns the Metis Forwarder.  Used primarily by associated classes in the
 * configuration group.
 *
 * @param [in] config An allocated MetisConfiguration
 *
 * @return non-null The owning MetisForwarder
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
MetisForwarder *metisConfiguration_GetForwarder(const MetisConfiguration *config);

/**
 * Returns the logger used by the Configuration subsystem
 *
 * Returns the logger specified when the MetisConfiguration was created.
 *
 * @param [in] config An allocated MetisConfiguration
 *
 * @retval non-null The logger
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisLogger *metisConfiguration_GetLogger(const MetisConfiguration *config);
#endif // Metis_metis_Configuration_h
