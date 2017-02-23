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
#include <ccnx/forwarder/metis/config/metisControl_SetDebug.h>

static MetisCommandReturn _metisControlSetStrategy_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlSetStrategy_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandSetStrategy = "set strategy";
static const char *_commandSetStrategyHelp = "help set strategy";

// ====================================================

MetisCommandOps *
metisControlSetStrategy_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSetStrategy, NULL, _metisControlSetStrategy_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlSetStrategy_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandSetStrategyHelp, NULL, _metisControlSetStrategy_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandReturn
_metisControlSetStrategy_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("set strategy <prefix> <strategy>\n");
    printf("available strateies:\n");
    printf("    random\n");
    printf("    loadbalancer\n");
    printf("    random_per_dash_segment\n");
    printf("    loadbalancer_with_delay\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlSetStrategy_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisControlState *state = ops->closure;

    if (parcList_Size(args) != 4) {
        _metisControlSetStrategy_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    if (((strcmp(parcList_GetAtIndex(args, 0), "set") != 0) || (strcmp(parcList_GetAtIndex(args, 1), "strategy") != 0))) {
        _metisControlSetStrategy_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    const char *prefixString = parcList_GetAtIndex(args, 2);
    const char *strategy = parcList_GetAtIndex(args, 3);
    CCNxName *prefix = ccnxName_CreateFromCString(prefixString);
    if (prefix == NULL) {
        printf("ERROR: could not parse prefix '%s'\n", prefixString);
        return MetisCommandReturn_Failure;
    }

    CPIForwardingStrategy *fwdStrategy = cpiForwardingStrategy_Create(prefix, (char *) strategy);

    CCNxControl *setFwdStrategyRequest = ccnxControl_CreateSetStrategyRequest(fwdStrategy);

    cpiForwardingStrategy_Destroy(&fwdStrategy);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(setFwdStrategyRequest));
        printf("request: %s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(setFwdStrategyRequest);
    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
    ccnxMetaMessage_Release(&message);

    ccnxControl_Release(&setFwdStrategyRequest);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("response: %s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    ccnxMetaMessage_Release(&rawResponse);

    return MetisCommandReturn_Success;
}
