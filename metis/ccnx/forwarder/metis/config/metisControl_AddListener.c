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
#include <inttypes.h>
#include <ctype.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Network.h>
#include <parc/algol/parc_Memory.h>
#include <ccnx/api/control/cpi_Listener.h>

#include <ccnx/forwarder/metis/config/metisControl_AddListener.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_Acks.h>

static MetisCommandReturn _metisControlAddListener_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddListener_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *command_add_listener = "add listener";
static const char *command_help_add_listener = "help add listener";

MetisCommandOps *
metisControlAddListener_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, command_add_listener, NULL, _metisControlAddListener_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlAddListener_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, command_help_add_listener, NULL, _metisControlAddListener_HelpExecute, metisCommandOps_Destroy);
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

static const int _indexProtocol = 2;
static const int _indexSymbolic = 3;
static const int _indexAddress = 4;
static const int _indexPort = 5;

static MetisCommandReturn
_metisControlAddListener_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("commands:\n");
    printf("   add listener <protocol> <symbolic> <localAddress> <PortOrEtherType>\n");
    printf("\n");
    printf("   symbolic:        User defined name for listener, must start with alpha and be alphanum\n");
    printf("   protocol:        tcp | udp | ether\n");
    printf("   localAddress:    IPv4 or IPv6 or hostname or interface name (see examples)\n");
    printf("   PortOrEtherType: TCP/UDP port or EtherType (base 10 or use 0x for base 16)\n");
    printf("\n");
    printf("Notes:\n");
    printf("   The local address must be on the system (see 'help list interfaces' command).\n");
    printf("   For Ethernet, the broadcast and CCNx group address will also be added.\n");
    printf("   The symblic name must be unique or the forwarder will reject it.\n");
    printf("\n");
    printf("Examples:\n");
    printf("   Listens to 192.168.1.7 on tcp port 9695\n");
    printf("      add listener tcp homenet 192.168.1.7 9695\n");
    printf("\n");
    printf("   Listens to IPv6 localhost on udp port 9695\n");
    printf("      add listener udp localhost6 ::1 9695\n");
    printf("\n");
    printf("   Listens to interface 'en0' on ethertype 0x0801\n");
    printf("      add listener ether nic0 en0 0x0801\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static CPIAddress *
_convertStringsToCpiAddress(const char *ip_string, const char *port_string)
{
    int port = atoi(port_string);
    struct sockaddr *addr = parcNetwork_SockAddress(ip_string, port);

    if (addr == NULL) {
        printf("Error converting address '%s' port '%s' to socket address\n", ip_string, port_string);
        return NULL;
    }

    CPIAddress *remote_cpi_address = NULL;
    switch (addr->sa_family) {
        case PF_INET: {
            remote_cpi_address = cpiAddress_CreateFromInet((struct sockaddr_in *) addr);
            break;
        }

        case PF_INET6: {
            remote_cpi_address = cpiAddress_CreateFromInet6((struct sockaddr_in6 *) addr);
            break;
        }

        default: {
            printf("Error converting address '%s' port '%s' to socket address, unsupported address family %d\n",
                   ip_string, port_string, addr->sa_family);
            break;
        }
    }
    parcMemory_Deallocate((void **) &addr);
    return remote_cpi_address;
}

static MetisCommandReturn
_sendAndVerify(MetisControlState *metis_State, CCNxControl *control)
{
    MetisCommandReturn result = MetisCommandReturn_Failure;
    uint64_t seqnum = cpi_GetSequenceNumber(control);

    CCNxMetaMessage *requestMessage = ccnxMetaMessage_CreateFromControl(control);
    CCNxMetaMessage *responseMessage = metisControlState_WriteRead(metis_State, requestMessage);
    ccnxMetaMessage_Release(&requestMessage);

    if (metisControlState_GetDebug(metis_State)) {
        char *str = parcJSON_ToString(ccnxControl_GetJson(responseMessage));
        printf("reponse:\n%s\n", str);
        parcMemory_Deallocate((void **) &str);
    }

    if (ccnxMetaMessage_IsControl(responseMessage)) {
        CCNxControl *responseControl = ccnxMetaMessage_GetControl(responseMessage);
        if (ccnxControl_IsACK(responseControl)) {
            uint64_t ackedSeqnum = cpiAcks_GetAckOriginalSequenceNumber(ccnxControl_GetJson(responseControl));
            if (ackedSeqnum == seqnum) {
                result = MetisCommandReturn_Success;
            } else {
                printf("Error: received wrong seqnum expected %" PRIu64 " got %" PRIu64 "\n", seqnum, ackedSeqnum);
            }
        }
    }

    ccnxMetaMessage_Release(&responseMessage);
    return result;
}

static MetisCommandReturn
_createTcpListener(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandReturn result = MetisCommandReturn_Failure;

    const char *symbolic = parcList_GetAtIndex(args, _indexSymbolic);
    const char *host = parcList_GetAtIndex(args, _indexAddress);
    const char *port = parcList_GetAtIndex(args, _indexPort);

    CPIAddress *socket = _convertStringsToCpiAddress(host, port);
    if (socket) {
        CPIListener *listener = cpiListener_CreateIP(IPTUN_TCP, socket, symbolic);
        CCNxControl *control = cpiListener_CreateAddMessage(listener);

        MetisControlState *metis_State = ops->closure;
        result = _sendAndVerify(metis_State, control);
        ccnxControl_Release(&control);
        cpiListener_Release(&listener);
        cpiAddress_Destroy(&socket);
    }

    return result;
}

static MetisCommandReturn
_createUdpListener(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandReturn result = MetisCommandReturn_Failure;

    const char *symbolic = parcList_GetAtIndex(args, _indexSymbolic);
    const char *host = parcList_GetAtIndex(args, _indexAddress);
    const char *port = parcList_GetAtIndex(args, _indexPort);

    CPIAddress *socket = _convertStringsToCpiAddress(host, port);
    if (socket) {
        CPIListener *listener = cpiListener_CreateIP(IPTUN_UDP, socket, symbolic);
        CCNxControl *control = cpiListener_CreateAddMessage(listener);

        MetisControlState *metis_State = ops->closure;
        result = _sendAndVerify(metis_State, control);
        ccnxControl_Release(&control);
        cpiListener_Release(&listener);
        cpiAddress_Destroy(&socket);
    }

    return result;
}

static MetisCommandReturn
_createEtherListener(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandReturn result = MetisCommandReturn_Failure;

    const char *symbolic = parcList_GetAtIndex(args, _indexSymbolic);
    const char *ifname = parcList_GetAtIndex(args, _indexAddress);
    uint16_t ethertype = (uint16_t) strtoul(parcList_GetAtIndex(args, _indexPort), NULL, 0);

    {
        CPIListener *listener = cpiListener_CreateEther(ifname, (uint16_t) ethertype, symbolic);
        CCNxControl *control = cpiListener_CreateAddMessage(listener);

        MetisControlState *metis_State = ops->closure;
        result = _sendAndVerify(metis_State, control);
        ccnxControl_Release(&control);
        cpiListener_Release(&listener);
    }

    return result;
}


static MetisCommandReturn
_metisControlAddListener_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    if (parcList_Size(args) != 6) {
        _metisControlAddListener_HelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    MetisCommandReturn result = MetisCommandReturn_Failure;

    const char *symbolic = parcList_GetAtIndex(args, _indexSymbolic);
    if (_validateSymbolicName(symbolic)) {
        const char *protocol = parcList_GetAtIndex(args, _indexProtocol);
        if (strcasecmp("tcp", protocol) == 0) {
            result = _createTcpListener(parser, ops, args);
        } else if (strcasecmp("udp", protocol) == 0) {
            result = _createUdpListener(parser, ops, args);
        } else if (strcasecmp("ether", protocol) == 0) {
            result = _createEtherListener(parser, ops, args);
        } else {
            printf("Error: unrecognized protocol '%s'\n", protocol);
        }
    } else {
        printf("Error: symbolic name must begin with an alpha and be alphanum after\n");
    }


    return result;
}



