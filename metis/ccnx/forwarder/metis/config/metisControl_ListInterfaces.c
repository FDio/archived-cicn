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

#include <ccnx/forwarder/metis/config/metisControl_ListInterfaces.h>

#include <ccnx/api/control/cpi_ManageLinks.h>
#include <ccnx/api/control/cpi_Forwarding.h>

static MetisCommandReturn _metisControlListInterfaces_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlListInterfaces_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandListInterfaces = "list interfaces";
static const char *_commandListInterfacesHelp = "help list interfaces";

// ====================================================

MetisCommandOps *
metisControlListInterfaces_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListInterfaces, NULL, _metisControlListInterfaces_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlListInterfaces_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListInterfacesHelp, NULL, _metisControlListInterfaces_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlListInterfaces_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("list interfaces\n");
    printf("\n");

    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlListInterfaces_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    if (parcList_Size(args) != 2) {
        _metisControlListInterfaces_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    MetisControlState *state = ops->closure;
    CCNxControl *listRequest = ccnxControl_CreateInterfaceListRequest();

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(listRequest);
    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
    ccnxMetaMessage_Release(&message);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("reponse:\n%s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    CPIInterfaceSet *set = cpiLinks_InterfacesFromControlMessage(response);

    //"%3u %10s %1s%1s %8u "
    printf("%3.3s %10.10s %1.1s%1.1s %8.8s \n", "interface", "name", "loopback", "multicast", "MTU");
    for (size_t i = 0; i < cpiInterfaceSet_Length(set); i++) {
        CPIInterface *interface = cpiInterfaceSet_GetByOrdinalIndex(set, i);
        char *string = cpiInterface_ToString(interface);
        puts(string);
        parcMemory_Deallocate((void **) &string);
    }

    cpiInterfaceSet_Destroy(&set);

    ccnxMetaMessage_Release(&rawResponse);

    return MetisCommandReturn_Success;
}
