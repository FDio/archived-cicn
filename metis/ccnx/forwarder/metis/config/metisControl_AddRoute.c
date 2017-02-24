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
#include <ctype.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/api/control/cpi_NameRouteProtocolType.h>
#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/api/control/cpi_Forwarding.h>

#include <ccnx/forwarder/metis/config/metisControl_AddRoute.h>

static MetisCommandReturn _metisControlAddRoute_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddRoute_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandAddRoute = "add route";
static const char *_commandAddRouteHelp = "help add route";

MetisCommandOps *
metisControlAddRoute_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddRoute, NULL, _metisControlAddRoute_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlAddRoute_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddRouteHelp, NULL, _metisControlAddRoute_HelpExecute, metisCommandOps_Destroy);
}

/**
 * Return true if string is purely an integer
 */
static bool
_isNumber(const char *string)
{
    size_t len = strlen(string);
    for (size_t i = 0; i < len; i++) {
        if (!isdigit(string[i])) {
            return false;
        }
    }
    return true;
}

/**
 * A symbolic name must be at least 1 character and must begin with an alpha.
 * The remainder must be an alphanum.
 */
static bool
_validateSymbolicName(const char *symbolic)
{
    bool success = false;
    size_t len = strlen(symbolic);
    if (len > 0) {
        if (isalpha(symbolic[0])) {
            success = true;
            for (size_t i = 1; i < len; i++) {
                if (!isalnum(symbolic[i])) {
                    success = false;
                    break;
                }
            }
        }
    }
    return success;
}

// ====================================================

static MetisCommandReturn
_metisControlAddRoute_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("commands:\n");
    printf("   add route <symbolic | connid> <prefix> <cost>\n");
    printf("\n");
    printf("   symbolic:  The symbolic name for an exgress\n");
    printf("   connid:    The egress connection id (see 'help list connections')\n");
    printf("   prefix:    The CCNx name as a URI (e.g. lci:/foo/bar)\n");
    printf("   cost:      positive integer representing cost\n");
    printf("   nexthop:   Optional network endpoint on the connection\n");
    printf("   seconds:   Create a route that will expire if not refresed within the lifetime\n");
    printf("\n");
    printf("Examples:\n");
    printf("   add route 7 lci:/foo/bar 1\n");
    printf("      adds route to prefix '/foo/bar' on egress connection 7 with cost 1\n");
    printf("   add route tun3 lci:/foo/bar 1\n");
    printf("      adds route to prefix '/foo/bar' on egress connection 'tun3' with cost 1\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlAddRoute_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisControlState *state = ops->closure;

    if (parcList_Size(args) != 5) {
        _metisControlAddRoute_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    const char *symbolicOrConnid = parcList_GetAtIndex(args, 2);

    if (_validateSymbolicName(symbolicOrConnid) || _isNumber(symbolicOrConnid)) {
        const char *prefixString = parcList_GetAtIndex(args, 3);
        unsigned cost = atoi(parcList_GetAtIndex(args, 4));

        if (cost == 0) {
            printf("ERROR: cost must be positive integer, got %u from '%s'\n", cost, (char *) parcList_GetAtIndex(args, 4));
            return MetisCommandReturn_Failure;
        }

        CCNxName *prefix = ccnxName_CreateFromCString(prefixString);
        if (prefix == NULL) {
            printf("ERROR: could not parse prefix '%s'\n", prefixString);
            return MetisCommandReturn_Failure;
        }

        char *protocolTypeAsString = "static";

        CPINameRouteProtocolType protocolType = cpiNameRouteProtocolType_FromString(protocolTypeAsString);
        CPINameRouteType routeType = cpiNameRouteType_LONGEST_MATCH;
        CPIAddress *nexthop = NULL;

        struct timeval *lifetime = NULL;

        CPIRouteEntry *route = NULL;

        if (_isNumber(symbolicOrConnid)) {
            unsigned connid = (unsigned) strtold(symbolicOrConnid, NULL);
            route = cpiRouteEntry_Create(prefix, connid, nexthop, protocolType, routeType, lifetime, cost);
        } else {
            route = cpiRouteEntry_CreateSymbolic(prefix, symbolicOrConnid, protocolType, routeType, lifetime, cost);
        }

        CCNxControl *addRouteRequest = ccnxControl_CreateAddRouteRequest(route);

        cpiRouteEntry_Destroy(&route);

        if (metisControlState_GetDebug(state)) {
            char *str = parcJSON_ToString(ccnxControl_GetJson(addRouteRequest));
            printf("request: %s\n", str);
            parcMemory_Deallocate((void **) &str);
        }

        CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(addRouteRequest);
        CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
        ccnxMetaMessage_Release(&message);

        ccnxControl_Release(&addRouteRequest);

        CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

        if (metisControlState_GetDebug(state)) {
            char *str = parcJSON_ToString(ccnxControl_GetJson(response));
                printf("response: %s\n", str);
            parcMemory_Deallocate((void **) &str);
        }

        ccnxMetaMessage_Release(&rawResponse);

        return MetisCommandReturn_Success;
    } else {
        printf("ERROR: Invalid symbolic or connid.  Symbolic name must begin with an alpha followed by alphanum.  connid must be an integer\n");
        return MetisCommandReturn_Failure;
    }
}
