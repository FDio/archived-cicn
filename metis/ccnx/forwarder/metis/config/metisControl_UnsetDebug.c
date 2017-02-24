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

#include <ccnx/api/control/cpi_ManageLinks.h>
#include <ccnx/api/control/cpi_Forwarding.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/config/metisControl_UnsetDebug.h>

static MetisCommandReturn _metisControlUnsetDebug_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlUnsetDebug_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandUnsetDebug = "unset debug";
static const char *_commandUnsetDebugHelp = "help unset debug";

// ====================================================

MetisCommandOps *
metisControlUnsetDebug_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandUnsetDebug, NULL, _metisControlUnsetDebug_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlUnsetDebug_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandUnsetDebugHelp, NULL, _metisControlUnsetDebug_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlUnsetDebug_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("unset debug: will disable the debug flag\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlUnsetDebug_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    if (parcList_Size(args) != 2) {
        _metisControlUnsetDebug_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    MetisControlState *state = ops->closure;
    metisControlState_SetDebug(state, false);
    printf("Debug flag cleared\n\n");

    return MetisCommandReturn_Success;
}
