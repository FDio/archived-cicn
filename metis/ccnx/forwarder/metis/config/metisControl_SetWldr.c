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

#include <ccnx/api/control/cpi_ManageWldr.h>
#include <ccnx/api/control/cpi_Forwarding.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/config/metisControl_SetDebug.h>


static MetisCommandReturn _metisControlSetWldr_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlSetWldr_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandSetWldr = "set wldr";
static const char *_commandSetWldrHelp = "help set wldr";

// ====================================================

MetisCommandOps *
metisControlSetWldr_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSetWldr, NULL, _metisControlSetWldr_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlSetWldr_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSetWldrHelp, NULL, _metisControlSetWldr_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlSetWldr_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("set wldr <on|off> <connection_id>\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlSetWldr_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisControlState *state = ops->closure;

    if (parcList_Size(args) != 4) {
        _metisControlSetWldr_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    if (((strcmp(parcList_GetAtIndex(args, 0), "set") != 0) || (strcmp(parcList_GetAtIndex(args, 1), "wldr") != 0))) {
        _metisControlSetWldr_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    const char *activeStr = parcList_GetAtIndex(args, 2);
    bool active;
    if(strcmp(activeStr, "on") == 0){
        active = true;
    }else if(strcmp(activeStr, "off") == 0){
        active = false;
    }else{
        _metisControlSetWldr_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    const char *connId = parcList_GetAtIndex(args, 3);

    CPIManageWldr *cpiWldr = cpiManageWldr_Create(active, (char *) connId);

    CCNxControl *setWldrRequest = ccnxControl_CreateSetWldrRequest(cpiWldr);

    cpiManageWldr_Destroy(&cpiWldr);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(setWldrRequest));
        printf("request: %s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(setWldrRequest);
    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
    ccnxMetaMessage_Release(&message);

    ccnxControl_Release(&setWldrRequest);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("response: %s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    if(ccnxControl_IsNACK(response)){
        printf("command set wldr failed");
        ccnxMetaMessage_Release(&rawResponse);
        return MetisCommandReturn_Failure;
    }

    ccnxMetaMessage_Release(&rawResponse);

    return MetisCommandReturn_Success;
}
