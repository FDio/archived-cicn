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
 * @file metis_CommandLineInterface.h
 * @brief A telnet-like server for management interface
 *
 *
 * We do not start the CLI until metisCommandLineInterface_Start() is called.  This allows us to always create it in
 * metisForwarder_Create(), but not bind the port until needed.  Binding the port in metisForwarder_Create()
 * causes severe issues in rapid execution of unit tests.
 *
 *
 */

#ifndef Metis_metis_CommandLineInterface_h
#define Metis_metis_CommandLineInterface_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_command_line_interface;
typedef struct metis_command_line_interface MetisCommandLineInterface;

/**
 * Creates a CLI on the given port.
 *
 *   A telnet-style interface.  Creating it will not bind the port or start
 *   the service.  You need to call <code>metisCli_Start()</code>
 *
 * @param port the command port, in host byte order
 *
 * @return NULL if cannot be created on the port
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisCommandLineInterface *metisCommandLineInterface_Create(MetisForwarder *metis, uint16_t port);

/**
 * Stops and destroys the CLI.  Existing sessions are destroyed.
 *
 *   <#Discussion#>
 *
 * @param cliPtr
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisCommandLineInterface_Destroy(MetisCommandLineInterface **cliPtr);

/*
 * Binds the port and starts the CLI service
 *
 *   <#Discussion#>
 *
 * @param cli
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisCommandLineInterface_Start(MetisCommandLineInterface *cli);
#endif // Metis_metis_CommandLineInterface_h
