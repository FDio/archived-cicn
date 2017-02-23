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


#ifndef Metis_metis_WebInterface_h
#define Metis_metis_WebInterface_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_web;
typedef struct metis_web MetisWeb;

/**
 * Creates a Web on the given port.
 *
 *   A http interface.  The Web interface is created in the STOPPED mode, so
 *   you need to start it for it to be usable.
 *
 *   Create will bind the port, but callbacks in the dispatcher will not be
 *   enabled until it is started.
 *
 * @param port the command port, in host byte order
 * @return NULL if cannot be created on the port
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisWeb *metisWeb_Create(MetisForwarder *metis, uint16_t port);

/**
 * Stops and destroys the web interface.  Existing sessions are destroyed.
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisWeb_Destroy(MetisWeb **WebPtr);

/**
 * Enables the web interface in the event dispatcher.
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisWeb_Start(MetisWeb *web);

/**
 * Disables callback in the event dispatcher.  Existing connections unaffected.
 *
 *   Stopping it only disable accepting new connections.
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisWeb_Stop(MetisWeb *web);
#endif // Metis_metis_WebInterface_h
