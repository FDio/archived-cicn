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


#include <config.h>

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include <LongBow/runtime.h>

#include <parc/security/parc_Security.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/config/metisControl_List.h>
#include <ccnx/forwarder/metis/config/metisControl_ListConnections.h>
#include <ccnx/forwarder/metis/config/metisControl_ListInterfaces.h>
#include <ccnx/forwarder/metis/config/metisControl_ListRoutes.h>

static void _metisControlList_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlList_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlList_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandList = "list";
static const char *_commandListHelp = "help list";

MetisCommandOps *
metisControlList_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandList, _metisControlList_Init, _metisControlList_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlList_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListHelp, NULL, _metisControlList_HelpExecute, metisCommandOps_Destroy);
}

// =====================================================

static MetisCommandReturn
_metisControlList_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandOps *ops_list_connections = metisControlListConnections_HelpCreate(NULL);
    MetisCommandOps *ops_list_interfaces = metisControlListInterfaces_HelpCreate(NULL);
    MetisCommandOps *ops_list_routes = metisControlListRoutes_HelpCreate(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_list_connections->command);
    printf("   %s\n", ops_list_interfaces->command);
    printf("   %s\n", ops_list_routes->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_list_connections);
    metisCommandOps_Destroy(&ops_list_interfaces);
    metisCommandOps_Destroy(&ops_list_routes);

    return MetisCommandReturn_Success;
}

static void
_metisControlList_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlListConnections_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlListInterfaces_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlListRoutes_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlListConnections_Create(state));
    metisControlState_RegisterCommand(state, metisControlListInterfaces_Create(state));
    metisControlState_RegisterCommand(state, metisControlListRoutes_Create(state));
}

static MetisCommandReturn
_metisControlList_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlList_HelpExecute(parser, ops, args);
}

// ======================================================================
