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

#include <ccnx/forwarder/metis/config/metisControl_Remove.h>
#include <ccnx/forwarder/metis/config/metisControl_RemoveConnection.h>
#include <ccnx/forwarder/metis/config/metisControl_RemoveRoute.h>

static void _metisControlRemove_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlRemove_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlRemove_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandRemove = "remove";
static const char *_commandRemoveHelp = "help remove";

// ====================================================

MetisCommandOps *
metisControlRemove_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemove, _metisControlRemove_Init, _metisControlRemove_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlRemove_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemoveHelp, NULL, _metisControlRemove_HelpExecute, metisCommandOps_Destroy);
}

// ==============================================

static MetisCommandReturn
_metisControlRemove_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandOps *ops_remove_connection = metisControlRemoveConnection_Create(NULL);
    MetisCommandOps *ops_remove_route = metisControlRemoveRoute_Create(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_remove_connection->command);
    printf("   %s\n", ops_remove_route->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_remove_connection);
    metisCommandOps_Destroy(&ops_remove_route);
    return MetisCommandReturn_Success;
}

static void
_metisControlRemove_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlRemoveConnection_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlRemoveRoute_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlRemoveConnection_Create(state));
    metisControlState_RegisterCommand(state, metisControlRemoveRoute_Create(state));
}

static MetisCommandReturn
_metisControlRemove_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlRemove_HelpExecute(parser, ops, args);
}
