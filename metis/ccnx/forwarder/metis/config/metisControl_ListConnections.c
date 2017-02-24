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

#include <ccnx/forwarder/metis/config/metisControl_ListConnections.h>

#include <ccnx/api/control/cpi_ManageLinks.h>
#include <ccnx/api/control/cpi_Forwarding.h>

static MetisCommandReturn _metisControlListConnections_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlListConnections_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandListConnections = "list connections";
static const char *_commandListConnectionsHelp = "help list connections";

MetisCommandOps *
metisControlListConnections_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListConnections, NULL, _metisControlListConnections_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlListConnections_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListConnectionsHelp, NULL, _metisControlListConnections_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlListConnections_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("list connections: displays a 1-line summary of each connection\n");
    printf("\n");
    printf("The columns are:\n");
    printf("   connection id : an integer index for the connection\n");
    printf("   state         : UP or DOWN\n");
    printf("   local address : the local network address associated with the connection\n");
    printf("   remote address: the remote network address associated with the connection\n");
    printf("   protocol      : the network protocol (tcp, udp, gre, mcast, ether)\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlListConnections_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    if (parcList_Size(args) != 2) {
        _metisControlListConnections_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    MetisControlState *state = ops->closure;

    CCNxControl *connectionListRequest = ccnxControl_CreateConnectionListRequest();

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(connectionListRequest);
    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
    ccnxMetaMessage_Release(&message);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("reponse:\n%s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    CPIConnectionList *list = cpiLinks_ConnectionListFromControlMessage(response);
    //
    //    //"%3u %10s %1s%1s %8u "
    //    //    printf("%3.3s %10.10s %1.1s%1.1s %8.8s \n", "interface", "name", "loopback", "multicast", "MTU");
    for (size_t i = 0; i < cpiConnectionList_Length(list); i++) {
        CPIConnection *connection = cpiConnectionList_Get(list, i);
        char *string = cpiConnection_ToString(connection);
        puts(string);
        parcMemory_Deallocate((void **) &string);
        cpiConnection_Release(&connection);
    }
    cpiConnectionList_Destroy(&list);
    ccnxMetaMessage_Release(&rawResponse);

    return MetisCommandReturn_Success;
}
