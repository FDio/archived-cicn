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
#include <string.h>

#include <parc/security/parc_Security.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Network.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_TreeRedBlack.h>
#include <parc/algol/parc_Time.h>

#include <ccnx/forwarder/metis/config/metis_ControlState.h>
#include <ccnx/forwarder/metis/config/metisControl_Root.h>
#include <ccnx/forwarder/metis/config/metis_CommandParser.h>

struct metis_control_state {
    MetisCommandParser *parser;
    bool debugFlag;

    void *userdata;
    CCNxMetaMessage * (*writeRead)(void *userdata, CCNxMetaMessage *msg);
};

MetisControlState *
metisControlState_Create(void *userdata, CCNxMetaMessage * (*writeRead)(void *userdata, CCNxMetaMessage * msg))
{
    MetisControlState *state = parcMemory_AllocateAndClear(sizeof(MetisControlState));
    assertNotNull(state, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisControlState));
    state->parser = metisCommandParser_Create();

    state->userdata = userdata;
    state->writeRead = writeRead;
    state->debugFlag = false;

    return state;
}

void
metisControlState_Destroy(MetisControlState **statePtr)
{
    assertNotNull(statePtr, "Parameter statePtr must be non-null");
    assertNotNull(*statePtr, "Parameter statePtr must dereference t non-null");
    MetisControlState *state = *statePtr;
    metisCommandParser_Destroy(&state->parser);

    parcMemory_Deallocate((void **) &state);
    *statePtr = NULL;
}

void
metisControlState_SetDebug(MetisControlState *state, bool debugFlag)
{
    assertNotNull(state, "Parameter state must be non-null");
    state->debugFlag = debugFlag;
    metisCommandParser_SetDebug(state->parser, debugFlag);
}

bool
metisControlState_GetDebug(MetisControlState *state)
{
    assertNotNull(state, "Parameter state must be non-null");
    return state->debugFlag;
}

void
metisControlState_RegisterCommand(MetisControlState *state, MetisCommandOps *ops)
{
    assertNotNull(state, "Parameter state must be non-null");
    metisCommandParser_RegisterCommand(state->parser, ops);
}

CCNxMetaMessage *
metisControlState_WriteRead(MetisControlState *state, CCNxMetaMessage *msg)
{
    assertNotNull(state, "Parameter state must be non-null");
    assertNotNull(msg, "Parameter msg must be non-null");

    return state->writeRead(state->userdata, msg);
}

static PARCList *
_metisControlState_ParseStringIntoTokens(const char *originalString)
{
    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    char *token;

    char *tofree = parcMemory_StringDuplicate(originalString, strlen(originalString) + 1);
    char *string = tofree;

    while ((token = strsep(&string, " \t\n")) != NULL) {
        if (strlen(token) > 0) {
            parcList_Add(list, strdup(token));
        }
    }

    parcMemory_Deallocate((void **) &tofree);

    return list;
}

MetisCommandReturn
metisControlState_DispatchCommand(MetisControlState *state, PARCList *args)
{
    assertNotNull(state, "Parameter state must be non-null");
    return metisCommandParser_DispatchCommand(state->parser, args);
}

int
metisControlState_Interactive(MetisControlState *state)
{
    assertNotNull(state, "Parameter state must be non-null");
    char *line = NULL;
    size_t linecap = 0;
    MetisCommandReturn controlReturn = MetisCommandReturn_Success;

    while (controlReturn != MetisCommandReturn_Exit && !feof(stdin)) {
        fputs("> ", stdout); fflush(stdout);
        ssize_t failure = getline(&line, &linecap, stdin);
        assertTrue(failure > -1, "Error getline");

        PARCList *args = _metisControlState_ParseStringIntoTokens(line);
        controlReturn = metisControlState_DispatchCommand(state, args);
        parcList_Release(&args);
    }
    return 0;
}
