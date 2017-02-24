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
#include <parc/algol/parc_Time.h>

#include <ccnx/forwarder/metis/config/metisControl_ListRoutes.h>

#include <ccnx/api/control/cpi_ManageLinks.h>
#include <ccnx/api/control/cpi_Forwarding.h>

static MetisCommandReturn _metisControlListRoutes_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlListRoutes_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandListRoutes = "list routes";
static const char *_commandListRoutesHelp = "help list routes";

// ====================================================

MetisCommandOps *
metisControlListRoutes_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListRoutes, NULL, _metisControlListRoutes_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlListRoutes_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandListRoutesHelp, NULL, _metisControlListRoutes_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlListRoutes_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("command: list routes\n");
    printf("\n");
    printf("This command will fetch the prefix routing table.  For each route, it will list:\n");
    printf("   iface:    interface\n");
    printf("   protocol: the routing protocol, such as STATIC, CONNECTED, etc.\n");
    printf("   type:     LMP or EXACT (longest matching prefix or exact match)\n");
    printf("   cost:     The route cost, lower being preferred\n");
    printf("   next:     List of next hops by interface id\n");
    printf("   prefix:   The CCNx name prefix\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlListRoutes_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    if (parcList_Size(args) != 2) {
        _metisControlListRoutes_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    MetisControlState *state = ops->closure;

    CCNxControl *routeListRequest = ccnxControl_CreateRouteListRequest();

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(routeListRequest);

    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("reponse:\n%s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    CPIRouteEntryList *list = cpiForwarding_RouteListFromControlMessage(response);

    printf("%6.6s %9.9s %7.7s %8.8s %20.20s %s\n", "iface", "protocol", "route", "cost", "next", "prefix");

    for (size_t i = 0; i < cpiRouteEntryList_Length(list); i++) {
        CPIRouteEntry *route = cpiRouteEntryList_Get(list, i);

        PARCBufferComposer *composer = parcBufferComposer_Create();

        parcBufferComposer_Format(composer, "%6d %9.9s %7.7s %8u ",
                                  cpiRouteEntry_GetInterfaceIndex(route),
                                  cpiNameRouteProtocolType_ToString(cpiRouteEntry_GetRouteProtocolType(route)),
                                  cpiNameRouteType_ToString(cpiRouteEntry_GetRouteType(route)),
                                  cpiRouteEntry_GetCost(route));

        if (cpiRouteEntry_GetNexthop(route) != NULL) {
            cpiAddress_BuildString(cpiRouteEntry_GetNexthop(route), composer);
        } else {
            parcBufferComposer_PutString(composer, "---.---.---.---/....");
        }

        if (cpiRouteEntry_HasLifetime(route)) {
            char *timeString = parcTime_TimevalAsString(cpiRouteEntry_GetLifetime(route));
            parcBufferComposer_PutString(composer, timeString);
            parcMemory_Deallocate((void **) &timeString);
        } else {
            parcBufferComposer_PutString(composer, " ");
        }

        char *ccnxName = ccnxName_ToString(cpiRouteEntry_GetPrefix(route));
        parcBufferComposer_PutString(composer, ccnxName);
        parcMemory_Deallocate((void **) &ccnxName);

        PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
        char *result = parcBuffer_ToString(tempBuffer);
        parcBuffer_Release(&tempBuffer);

        puts(result);
        parcMemory_Deallocate((void **) &result);
        parcBufferComposer_Release(&composer);
        cpiRouteEntry_Destroy(&route);
    }

    cpiRouteEntryList_Destroy(&list);
    ccnxMetaMessage_Release(&rawResponse);
    ccnxControl_Release(&routeListRequest);

    printf("Done\n\n");

    return MetisCommandReturn_Success;
}
