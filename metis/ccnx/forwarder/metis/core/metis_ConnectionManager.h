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
 * @file metis_ConnectionManager.h
 * @brief The connection manager handles connection events, such as going down
 *
 * The connection manager listens to the event notification system.  Based on those
 * events, the connection manager will take specific actions.  This is expected
 * to be a singleton instantiated by metis_Forwarder.
 *
 *   METIS_CONN_UP:
 *      - send a notification to appropriate local applications that want to
 *        known when connections come up.
 *
 *   METIS_CONN_DOWN:
 *      - Tear down the connection
 *      - Send a notification to local applications
 *
 */

#ifndef Metis_metis_ConnectionManager_h
#define Metis_metis_ConnectionManager_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_connection_manager;
typedef struct metis_connection_manager MetisConnectionManager;

MetisConnectionManager *metisConnectionManager_Create(MetisForwarder *metis);

void metisConnectionManager_Destroy(MetisConnectionManager **managerPtr);
#endif // Metis_metis_ConnectionManager_h
