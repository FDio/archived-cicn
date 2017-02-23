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

#include <ccnx/forwarder/metis/config/metisControl_Unset.h>
#include <ccnx/forwarder/metis/config/metisControl_UnsetDebug.h>

static void _metisControlUnset_Init(MetisCommandParser *parser, MetisCommandOps *ops);

static MetisCommandReturn _metisControlUnset_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlUnset_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandUnset = "unset";
static const char *_commandUnsetHelp = "help unset";

// ===========================================================

MetisCommandOps *
metisControlUnset_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandUnset, _metisControlUnset_Init, _metisControlUnset_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlUnset_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandUnsetHelp, NULL, _metisControlUnset_HelpExecute, metisCommandOps_Destroy);
}

// ===========================================================

static void
_metisControlUnset_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlUnsetDebug_Create(state));
    metisControlState_RegisterCommand(state, metisControlUnsetDebug_HelpCreate(state));
}

static MetisCommandReturn
_metisControlUnset_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandOps *ops_help_unset_debug = metisControlUnsetDebug_HelpCreate(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_help_unset_debug->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_help_unset_debug);
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlUnset_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlUnset_HelpExecute(parser, ops, args);
}
