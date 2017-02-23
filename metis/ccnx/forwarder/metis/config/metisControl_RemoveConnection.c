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
#include <parc/algol/parc_Network.h>
#include <ccnx/api/control/cpi_Address.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>
#include <ccnx/api/control/cpi_ManageLinks.h>
#include <ccnx/api/control/cpi_ConnectionEthernet.h>

#include <ccnx/forwarder/metis/config/metisControl_RemoveConnection.h>

static void  _metisControlRemoveConnection_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlRemoveConnection_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlRemoveConnection_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

// ===================================================

//TODO: implement this also for TCP and ethernet

static MetisCommandReturn _metisControlRemoveConnection_UdpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlRemoveConnection_UdpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

// ===================================================


static const char *_commandRemoveConnection = "remove connection";
static const char *_commandRemoveConnectionUdp = "remove connection udp";
static const char *_commandRemoveConnectionHelp = "help remove connection";
static const char *_commandRemoveConnectionUdpHelp = "help remove connection udp";

// ====================================================

MetisCommandOps *
metisControlRemoveConnection_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemoveConnection, _metisControlRemoveConnection_Init, _metisControlRemoveConnection_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlRemoveConnection_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemoveConnectionHelp, NULL, _metisControlRemoveConnection_HelpExecute, metisCommandOps_Destroy);
}

// ====================================================

static MetisCommandOps *
_metisControlRemoveConnection_UdpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemoveConnectionUdp, NULL,
                                  _metisControlRemoveConnection_UdpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlRemoveConnection_UdpHelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandRemoveConnectionUdpHelp, NULL,
                                  _metisControlRemoveConnection_UdpHelpExecute, metisCommandOps_Destroy);
}

// ====================================================

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
_metisControlRemoveConnection_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("Available commands:\n");
    printf("    %s\n", _commandRemoveConnectionUdp);
    return MetisCommandReturn_Success;
}

static void
_metisControlRemoveConnection_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, _metisControlRemoveConnection_UdpHelpCreate(state));

    metisControlState_RegisterCommand(state, _metisControlRemoveConnection_UdpCreate(state));
}


static MetisCommandReturn
_metisControlRemoveConnection_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlRemoveConnection_HelpExecute(parser, ops, args);
}

// ==================================================

static bool
_parseMessage(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args, char **symbolicPtr)
{
    if (parcList_Size(args) != 4) {
        _metisControlRemoveConnection_UdpHelpExecute(parser, ops, args);
        return false;
    }

    if ((strcmp(parcList_GetAtIndex(args, 0), "remove") != 0) ||
        (strcmp(parcList_GetAtIndex(args, 1), "connection") != 0) ||
        (strcmp(parcList_GetAtIndex(args, 2), "udp") != 0)) {
        _metisControlRemoveConnection_UdpHelpExecute(parser, ops, args);
        return false;
    }

    char *symbolic = parcList_GetAtIndex(args, 3);
    if (_validateSymbolicName(symbolic)) {
        *symbolicPtr = symbolic;
        return true;
    }
    return false;
}

static void
_removeUdpConnection(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args, char *symbolic)
{
    MetisControlState *state = ops->closure;
    struct sockaddr_in *local = parcNetwork_SockInet4AddressAny(); //parcNetwork_SockInet4Address("192.168.56.27", 12346);
    struct sockaddr_in *remote = parcNetwork_SockInet4AddressAny(); //parcNetwork_SockInet4Address("192.168.62.10", 19695);
    CPIAddress *localAddress = cpiAddress_CreateFromInet(local);
    CPIAddress *remoteAddress = cpiAddress_CreateFromInet(remote);

    CPIInterfaceIPTunnel *ipTunnel = cpiInterfaceIPTunnel_Create(0, localAddress, remoteAddress, IPTUN_UDP, symbolic);

    PARCJSON *cpiMessage = cpiLinks_RemoveIPTunnel(ipTunnel);
    CCNxControl *controlMessage = ccnxControl_CreateCPIRequest(cpiMessage);
    parcJSON_Release(&cpiMessage);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(controlMessage);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(message));
        printf("request: %s\n", str);
        parcMemory_Deallocate((void **) &str);
    }


    CCNxMetaMessage *rawResponse = metisControlState_WriteRead(state, message);
    ccnxMetaMessage_Release(&message);

    CCNxControl *response = ccnxMetaMessage_GetControl(rawResponse);

    if (metisControlState_GetDebug(state)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(response));
        printf("reponse:\n%s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    ccnxControl_Release(&controlMessage);
    ccnxMetaMessage_Release(&rawResponse);
    cpiInterfaceIPTunnel_Release(&ipTunnel);
    parcMemory_Deallocate((void **) &remote);
    parcMemory_Deallocate((void **) &local);
}

static MetisCommandReturn
_metisControlRemoveConnection_UdpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("command:\n");
    printf("    remove connection upd <symbolic>\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlRemoveConnection_UdpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    char *symbolic = NULL;
    if (!_parseMessage(parser, ops, args, &symbolic)) {
        return MetisCommandReturn_Success;
    }

    _removeUdpConnection(parser, ops, args, symbolic);

    return MetisCommandReturn_Success;
}


