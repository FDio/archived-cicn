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

#include <ccnx/forwarder/metis/config/metisControl_Quit.h>

static MetisCommandReturn _metisControlQuit_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlQuit_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandQuit = "quit";
static const char *_commandQuitHelp = "help quit";

// ====================================================

MetisCommandOps *
metisControlQuit_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandQuit, NULL, _metisControlQuit_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlQuit_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandQuitHelp, NULL, _metisControlQuit_HelpExecute, metisCommandOps_Destroy);
}

// ==============================================

static MetisCommandReturn
_metisControlQuit_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("Exits the interactive control program\n\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlQuit_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("exiting interactive shell\n");
    return MetisCommandReturn_Exit;
}
