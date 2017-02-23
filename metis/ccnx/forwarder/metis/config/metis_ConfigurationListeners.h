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
 * @file metis_ConfigurationListeners.h
 * @brief Configuration routines related to Listeners
 *
 * Adding and removing listeners.
 *
 */

#ifndef Metis__metis_ConfigurationListeners_h
#define Metis__metis_ConfigurationListeners_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>

#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_ControlMessage.h>

/**
 * Setup udp, tcp, and local listeners
 *
 *   Will bind to all available IP protocols on the given port.
 *   Does not add Ethernet listeners.
 *
 * @param port is the UPD and TCP port to use
 * @param localPath is the AF_UNIX path to use, if NULL no AF_UNIX listener is setup
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConfigurationListeners_SetupAll(const MetisConfiguration *config, uint16_t port, const char *localPath);

bool metisConfigurationListeners_Add(const MetisConfiguration *config, const CCNxControl *control, unsigned ingressId);
bool metisConfigurationListeners_Remove(const MetisConfiguration *config, const CCNxControl *control, unsigned ingressId);


#endif /* defined(Metis__metis_ConfigurationListeners_h) */
