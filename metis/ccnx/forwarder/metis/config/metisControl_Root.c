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

#include <ccnx/forwarder/metis/config/metisControl_Root.h>
#include <ccnx/forwarder/metis/config/metisControl_Add.h>
#include <ccnx/forwarder/metis/config/metisControl_List.h>
#include <ccnx/forwarder/metis/config/metisControl_Quit.h>
#include <ccnx/forwarder/metis/config/metisControl_Remove.h>
#include <ccnx/forwarder/metis/config/metisControl_Set.h>
#include <ccnx/forwarder/metis/config/metisControl_Unset.h>
#include <ccnx/forwarder/metis/config/metisControl_Cache.h>

static void _metisControlRoot_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlRoot_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlRoot_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandRoot = "";
static const char *_commandRootHelp = "help";

// ====================================================

MetisCommandOps *
metisControlRoot_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRoot, _metisControlRoot_Init, _metisControlRoot_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlRoot_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRootHelp, NULL, _metisControlRoot_HelpExecute, metisCommandOps_Destroy);
}

// ===================================================

static MetisCommandReturn
_metisControlRoot_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("Command-line execution:\n");
    printf("   metis_control [--keystore <keystorepath>] [--password <password>] command\n");
    printf("\n");
    printf("Interactive execution:\n");
    printf("   metis_control [--keystore <keystorepath>] [--password <password>]\n");
    printf("\n");
    printf("If the keystore is not specified, the default path is used. Keystore must exist prior to running program.\n");
    printf("If the password is not specified, the user will be prompted.\n");
    printf("\n");

    MetisCommandOps *ops_help_add = metisControlAdd_CreateHelp(NULL);
    MetisCommandOps *ops_help_list = metisControlList_HelpCreate(NULL);
    MetisCommandOps *ops_help_quit = metisControlQuit_HelpCreate(NULL);
    MetisCommandOps *ops_help_remove = metisControlRemove_HelpCreate(NULL);
    MetisCommandOps *ops_help_set = metisControlSet_HelpCreate(NULL);
    MetisCommandOps *ops_help_unset = metisControlUnset_HelpCreate(NULL);
    MetisCommandOps *ops_help_cache = metisControlCache_HelpCreate(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_help_add->command);
    printf("   %s\n", ops_help_list->command);
    printf("   %s\n", ops_help_quit->command);
    printf("   %s\n", ops_help_remove->command);
    printf("   %s\n", ops_help_set->command);
    printf("   %s\n", ops_help_unset->command);
    printf("   %s\n", ops_help_cache->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_help_add);
    metisCommandOps_Destroy(&ops_help_list);
    metisCommandOps_Destroy(&ops_help_quit);
    metisCommandOps_Destroy(&ops_help_remove);
    metisCommandOps_Destroy(&ops_help_set);
    metisCommandOps_Destroy(&ops_help_unset);
    metisCommandOps_Destroy(&ops_help_cache);

    return MetisCommandReturn_Success;
}

static void
_metisControlRoot_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;

    metisControlState_RegisterCommand(state, metisControlAdd_CreateHelp(state));
    metisControlState_RegisterCommand(state, metisControlList_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlQuit_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlRemove_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlSet_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlUnset_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlCache_HelpCreate(state));

    metisControlState_RegisterCommand(state, metisControlAdd_Create(state));
    metisControlState_RegisterCommand(state, metisControlList_Create(state));
    metisControlState_RegisterCommand(state, metisControlQuit_Create(state));
    metisControlState_RegisterCommand(state, metisControlRemove_Create(state));
    metisControlState_RegisterCommand(state, metisControlSet_Create(state));
    metisControlState_RegisterCommand(state, metisControlUnset_Create(state));
    metisControlState_RegisterCommand(state, metisControlCache_Create(state));
}

static MetisCommandReturn
_metisControlRoot_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return MetisCommandReturn_Success;
}

// ======================================================================
