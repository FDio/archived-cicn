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

#include <ccnx/forwarder/metis/config/metisControl_Set.h>
#include <ccnx/forwarder/metis/config/metisControl_SetDebug.h>
#include <ccnx/forwarder/metis/config/metisControl_SetStrategy.h>
#include <ccnx/forwarder/metis/config/metisControl_SetWldr.h>

static void _metisControlSet_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlSet_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlSet_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandSet = "set";
static const char *_commandSetHelp = "help set";

// ===========================================================

MetisCommandOps *
metisControlSet_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSet, _metisControlSet_Init, _metisControlSet_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlSet_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSetHelp, NULL, _metisControlSet_HelpExecute, metisCommandOps_Destroy);
}

// ===========================================================

static void
_metisControlSet_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlSetDebug_Create(state));
    metisControlState_RegisterCommand(state, metisControlSetDebug_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlSetStrategy_Create(state));
    metisControlState_RegisterCommand(state, metisControlSetStrategy_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlSetWldr_Create(state));
    metisControlState_RegisterCommand(state, metisControlSetWldr_HelpCreate(state));
}

static MetisCommandReturn
_metisControlSet_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandOps *ops_help_set_debug = metisControlSetDebug_HelpCreate(NULL);
    MetisCommandOps *ops_help_set_strategy = metisControlSetStrategy_HelpCreate(NULL);
    MetisCommandOps *ops_help_set_wldr = metisControlSetWldr_HelpCreate(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_help_set_debug->command);
    printf("   %s\n", ops_help_set_strategy->command);
    printf("   %s\n", ops_help_set_wldr->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_help_set_debug);
    metisCommandOps_Destroy(&ops_help_set_strategy);
    metisCommandOps_Destroy(&ops_help_set_wldr);
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlSet_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlSet_HelpExecute(parser, ops, args);
}
