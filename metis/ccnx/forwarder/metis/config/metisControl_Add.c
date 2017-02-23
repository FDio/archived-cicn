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

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/config/metisControl_Add.h>
#include <ccnx/forwarder/metis/config/metisControl_AddConnection.h>
#include <ccnx/forwarder/metis/config/metisControl_AddRoute.h>
#include <ccnx/forwarder/metis/config/metisControl_AddListener.h>

// ===================================================

static void _metisControlAdd_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlAdd_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAdd_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

// ===================================================

static const char *command_add = "add";
static const char *help_command_add = "help add";

MetisCommandOps *
metisControlAdd_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, command_add, _metisControlAdd_Init, _metisControlAdd_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlAdd_CreateHelp(MetisControlState *state)
{
    return metisCommandOps_Create(state, help_command_add, NULL, _metisControlAdd_HelpExecute, metisCommandOps_Destroy);
}

// ===================================================

static MetisCommandReturn
_metisControlAdd_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{

    printf("Available commands:\n");
    printf("   %s\n", command_add);
    printf("   %s\n", help_command_add);
    printf("\n");
    return MetisCommandReturn_Success;
}

static void
_metisControlAdd_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlAddListener_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlAddListener_Create(state));
    metisControlState_RegisterCommand(state, metisControlAddConnection_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlAddRoute_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlAddConnection_Create(state));
    metisControlState_RegisterCommand(state, metisControlAddRoute_Create(state));
}

static MetisCommandReturn
_metisControlAdd_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlAdd_HelpExecute(parser, ops, args);
}
