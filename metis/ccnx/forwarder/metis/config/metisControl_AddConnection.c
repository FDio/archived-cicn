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
#include <ccnx/forwarder/metis/config/metisControl_AddConnection.h>

// ===================================================

static void _metisControlAddConnection_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlAddConnection_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddConnection_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

// ===================================================

static MetisCommandReturn _metisControlAddConnection_TcpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddConnection_TcpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static MetisCommandReturn _metisControlAddConnection_UdpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddConnection_UdpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static MetisCommandReturn _metisControlAddConnection_McastHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddConnection_McastExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static MetisCommandReturn _metisControlAddConnection_EtherHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlAddConnection_EtherExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

// ===================================================

static const char *_commandAddConnection = "add connection";
static const char *_commandAddConnectionTcp = "add connection tcp";
static const char *_commandAddConnectionUdp = "add connection udp";
static const char *_commandAddConnectionMcast = "add connection mcast";
static const char *_commandAddConnectionEther = "add connection ether";
static const char *_commandAddConnectionHelp = "help add connection";
static const char *_commandAddConnectionTcpHelp = "help add connection tcp";
static const char *_commandAddConnectionUdpHelp = "help add connection udp";
static const char *_commandAddConnectionMcastHelp = "help add connection mcast";
static const char *_commandAddConnectionEtherHelp = "help add connection ether";

// ===================================================

MetisCommandOps *
metisControlAddConnection_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnection, _metisControlAddConnection_Init,
                                  _metisControlAddConnection_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlAddConnection_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionHelp, NULL,
                                  _metisControlAddConnection_HelpExecute, metisCommandOps_Destroy);
}

// ===================================================

static MetisCommandOps *
_metisControlAddConnection_TcpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionTcp, NULL,
                                  _metisControlAddConnection_TcpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_UdpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionUdp, NULL,
                                  _metisControlAddConnection_UdpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_McastCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionMcast, NULL,
                                  _metisControlAddConnection_McastExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_EtherCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionEther, NULL,
                                  _metisControlAddConnection_EtherExecute, metisCommandOps_Destroy);
}

// ===================================================

static MetisCommandOps *
_metisControlAddConnection_TcpHelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionTcpHelp, NULL,
                                  _metisControlAddConnection_TcpHelpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_UdpHelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionUdpHelp, NULL,
                                  _metisControlAddConnection_UdpHelpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_McastHelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionMcastHelp, NULL,
                                  _metisControlAddConnection_McastHelpExecute, metisCommandOps_Destroy);
}

static MetisCommandOps *
_metisControlAddConnection_EtherHelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandAddConnectionEtherHelp, NULL,
                                  _metisControlAddConnection_EtherHelpExecute, metisCommandOps_Destroy);
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

// ===================================================

static MetisCommandReturn
_metisControlAddConnection_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("Available commands:\n");
    printf("   %s\n", _commandAddConnectionTcp);
    printf("   %s\n", _commandAddConnectionUdp);
    printf("   %s\n", _commandAddConnectionMcast);
    printf("   %s\n", _commandAddConnectionEther);
    printf("\n");
    return MetisCommandReturn_Success;
}

static void
_metisControlAddConnection_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, _metisControlAddConnection_TcpHelpCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_UdpHelpCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_McastHelpCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_EtherHelpCreate(state));

    metisControlState_RegisterCommand(state, _metisControlAddConnection_TcpCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_UdpCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_McastCreate(state));
    metisControlState_RegisterCommand(state, _metisControlAddConnection_EtherCreate(state));
}

static MetisCommandReturn
_metisControlAddConnection_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlAddConnection_HelpExecute(parser, ops, args);
}

// ===================================================
// functions general to all connection types

/**
 * Create a tunnel in the forwarder based on the CPI addresses
 *
 * Caller retains ownership of memory.
 * The symbolic name will be used to refer to this connection. It must be unqiue otherwise
 * the forwarder will reject this commend.
 *
 * @param [in] parser An allocated MetisCommandParser
 * @param [in] ops Allocated MetisCommandOps (needed to extract MetisControlState)
 * @param [in] localAddress the local IP and port.  The port may be the wildcard value.
 * @param [in] remoteAddress The remote IP and port (both must be specified)
 * @param [in] tunnelType The tunneling protocol
 * @param [in] symbolic The symbolic name for the connection (must be unique)
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      struct sockaddr_in *anyAddress = parcNetwork_SockInet4AddressAny();
 *      struct sockaddr_in *remote     = parcNetwork_SockInet4Address("192.168.1.2", 9695);
 *
 *      CPIAddress *localAddress = cpiAddress_CreateFromInet(anyAddress);
 *      CPIAddress *remoteAddress = cpiAddress_CreateFromInet(remote);
 *
 *      metisControl_CreateTunnel(state, localAddress, remoteAddress, IPTUN_TCP, "conn7");
 *
 *      cpiAddress_Destroy(&localAddress);
 *      cpiAddress_Destroy(&remoteAddress);
 *      parcMemory_Deallocate((void **)&remote);
 *      parcMemory_Deallocate((void **)&anyAddress);
 * }
 * @endcode
 */
static void
_metisControlAddConnection_CreateTunnel(MetisCommandParser *parser, MetisCommandOps *ops, CPIAddress *localAddress, CPIAddress *remoteAddress, CPIInterfaceIPTunnelType tunnelType, const char *symbolic)
{
    MetisControlState *state = ops->closure;
    CPIAddress *remoteAddressCopy = cpiAddress_Copy(remoteAddress);
    CPIAddress *localAddressCopy = cpiAddress_Copy(localAddress);

    // a request like this always has an interface index of 0
    unsigned int interfaceIndex = 0;
    CPIInterfaceIPTunnel *ipTunnel = cpiInterfaceIPTunnel_Create(interfaceIndex, localAddressCopy, remoteAddressCopy, tunnelType, symbolic);
    PARCJSON *cpiMessage = cpiLinks_CreateIPTunnel(ipTunnel);
    CCNxControl *controlMessage = ccnxControl_CreateCPIRequest(cpiMessage);
    parcJSON_Release(&cpiMessage);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(controlMessage);

    // Write it, and get the response.
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
}

static CPIAddress *
_metisControlAddConnection_ConvertStringsToCpiAddress(const char *ip_string, const char *port_string)
{
    int port = atoi(port_string);
    struct sockaddr *addr = parcNetwork_SockAddress(ip_string, port);

    if (addr == NULL) {
        printf("Error converting address '%s' port '%s' to socket address\n", ip_string, port_string);
        return NULL;
    }

    CPIAddress *remote_cpi_address = NULL;
    switch (addr->sa_family) {
        case PF_INET:
        {
            remote_cpi_address = cpiAddress_CreateFromInet((struct sockaddr_in *) addr);
            break;
        }

        case PF_INET6:
        {
            remote_cpi_address = cpiAddress_CreateFromInet6((struct sockaddr_in6 *) addr);
            break;
        }
        default:
        {
            printf("Error converting address '%s' port '%s' to socket address, unsupported address family %d\n",
                   ip_string, port_string, addr->sa_family);
            break;
        }
    }
    parcMemory_Deallocate((void **) &addr);
    return remote_cpi_address;
}

/**
 * Parse a standard format command-line to a remote address and a local adress
 *
 * Command-line format:
 *   aaa bbb ccc <symbolic> <remote_ip|hostname> <remote_port> [<local_ip|hostname> [<local_port>]]
 *
 * where aaa, bbb, and ccc are don't care.
 *
 * @param [in] args The command line
 * @param [out] remoteAddressPtr The remote address, use `parcMemory_Deallocate()` on it, or null if error
 * @param [out] localAddressPtr The remote address, use `parcMemory_Deallocate()` on it, or null if error
 * @param [out] symbolicPtr The symbolic name (points to string in args)
 *
 * @return MetisCommandReturn_Success if valid IP address command-line
 * @return MetisCommandReturn_Failure if an error in the IP address command-line
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static MetisCommandReturn
_metisControlAddConnection_ParseIPCommandLine(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args,
                                              CPIAddress **remoteAddressPtr, CPIAddress **localAddressPtr, char **symbolicPtr)
{
    *remoteAddressPtr = NULL;
    *localAddressPtr = NULL;

    if (parcList_Size(args) < 6 || parcList_Size(args) > 8) {
        _metisControlAddConnection_TcpHelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    char *symbolic = parcList_GetAtIndex(args, 3);

    if (_validateSymbolicName(symbolic)) {
        char *remote_ip = parcList_GetAtIndex(args, 4);
        char *remote_port = parcList_GetAtIndex(args, 5);

        CPIAddress *remote_addr = _metisControlAddConnection_ConvertStringsToCpiAddress(remote_ip, remote_port);
        if (remote_addr == NULL) {
            return MetisCommandReturn_Failure;
        }

        char *local_ip = "0.0.0.0";
        char *local_port = "0";

        if (parcList_Size(args) > 6) {
            local_ip = parcList_GetAtIndex(args, 6);
        }

        if (parcList_Size(args) > 7) {
            local_port = parcList_GetAtIndex(args, 7);
        }

        CPIAddress *local_addr = _metisControlAddConnection_ConvertStringsToCpiAddress(local_ip, local_port);
        if (local_addr == NULL) {
            cpiAddress_Destroy(&remote_addr);
            return MetisCommandReturn_Failure;
        }

        if (cpiAddress_GetType(local_addr) != cpiAddress_GetType(remote_addr)) {
            char *local_str = cpiAddress_ToString(local_addr);
            char *remote_str = cpiAddress_ToString(remote_addr);
            printf("Error: local address %s not same type as remote address %s\n",
                   local_str, remote_str);
            parcMemory_Deallocate((void **) &local_str);
            parcMemory_Deallocate((void **) &remote_str);
            cpiAddress_Destroy(&remote_addr);
            cpiAddress_Destroy(&local_addr);
            return MetisCommandReturn_Failure;
        }

        *symbolicPtr = symbolic;
        *remoteAddressPtr = remote_addr;
        *localAddressPtr = local_addr;
        return MetisCommandReturn_Success;
    } else {
        printf("Invalid symbolic name.  Must begin with alpha and contain only alphanum.\n");
        return MetisCommandReturn_Failure;
    }
}

static MetisCommandReturn
_metisControlAddConnection_IpHelp(MetisCommandParser*parser,
                                  MetisCommandOps*ops, PARCList*args, const char*protocol)
{
    printf("add connection %s <symbolic> <remote_ip|hostname> <remote_port> [<local_ip|hostname> [<local_port>]]\n",
           protocol);
    printf("  <symbolic>              : symbolic name, e.g. 'conn1' (must be unique, start with alpha)\n");
    printf("  <remote_ip | hostname>  : the IPv4 or IPv6 or hostname of the remote system\n");
    printf("  <remote_port>           : the remote TCP port\n");
    printf("  <local_ip>              : optional local IP address to bind to\n");
    printf("  <local_port>            : optional local TCP port, random if not specified\n");
    printf("\n");
    printf("Examples:\n");
    printf("   add connection %s conn1 1.1.1.1 1200\n", protocol);
    printf("      opens a connection to IP address 1.1.1.1 port 1200 using the best local\n");
    printf("      interface and random local port.");
    printf("\n");
    printf("   add connection %s barney2 fe80::aa20:66ff:fe00:314a 1300\n", protocol);
    printf("     opens connection to IPv6 address on port 1300.\n");
    printf("\n");
    printf("   add connection %s conn0 1.1.1.1 1200 2.2.2.2 1300\n", protocol);
    printf("     opens a connection to 1.1.1.1 on port 1200 from the local address 2.2.2.2 port 1300\n");
    printf("\n");
    printf("   add connection %s conn3 ccn.parc.com 9695\n", protocol);
    printf("     opens a connection to the host 'ccn.parc.com' on port 9695.\n");
    printf("     Maybe an IPv4 or IPv6 connection as the name is resolved and connectivity permits.\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

// ===================================================

static MetisCommandReturn
_metisControlAddConnection_TcpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    _metisControlAddConnection_IpHelp(parser, ops, args, "tcp");
    printf("A TCP connection will not be usable until the remote peer accepts the connection.\n");
    printf("\n");
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlAddConnection_TcpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    char *symbolic = NULL;
    CPIAddress *remote_addr;
    CPIAddress *local_addr;
    if (_metisControlAddConnection_ParseIPCommandLine(parser, ops, args, &remote_addr, &local_addr, &symbolic) == MetisCommandReturn_Success) {
        _metisControlAddConnection_CreateTunnel(parser, ops, local_addr, remote_addr, IPTUN_TCP, symbolic);

        cpiAddress_Destroy(&remote_addr);
        cpiAddress_Destroy(&local_addr);
        return MetisCommandReturn_Success;
    }

    return MetisCommandReturn_Failure;
}

// ===================================================

static MetisCommandReturn
_metisControlAddConnection_UdpHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    _metisControlAddConnection_IpHelp(parser, ops, args, "udp");
    printf("A UDP connection will be usable immediately, even if the remote side has not accepted.\n");
    printf("\n");

    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlAddConnection_UdpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    CPIAddress *remote_addr;
    CPIAddress *local_addr;
    char *symbolic = NULL;
    if (_metisControlAddConnection_ParseIPCommandLine(parser, ops, args, &remote_addr, &local_addr, &symbolic) == MetisCommandReturn_Success) {
        _metisControlAddConnection_CreateTunnel(parser, ops, local_addr, remote_addr, IPTUN_UDP, symbolic);

        cpiAddress_Destroy(&remote_addr);
        cpiAddress_Destroy(&local_addr);
        return MetisCommandReturn_Success;
    }

    return MetisCommandReturn_Failure;
}

// ===================================================

static MetisCommandReturn
_metisControlAddConnection_McastHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("%s help", ops->command);
    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlAddConnection_McastExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    printf("ERROR: command not implemented\n\n");
    return MetisCommandReturn_Failure;
}

// ===================================================

/**
 * Parse a standard format command-line to a remote address and a local adress
 *
 * Command-line format:
 *   aaa bbb ccc <symbolic> <destination_mac> <local_interface>
 *
 * where aaa, bbb, and ccc are don't care.
 *
 * @param [in] args The command line
 * @param [out] remoteAddressPtr The remote address, or null if error
 * @param [out] localAddressPtr The local interface name as a LINK address, or null if error
 * @param [out] etherEncapType The ethertype (host byte order)
 * @param [out] symbolic The symbolic name (points to string in args)
 *
 * @retval MetisCommandReturn_Success if valid IP address command-line
 * @retval MetisCommandReturn_Failure if an error in the IP address command-line
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static MetisCommandReturn
_metisControl_ParseEtherCommandLine(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args,
                                    CPIAddress **remoteAddressPtr, char **localAddressPtr, uint16_t *etherEncapType, char **symbolicPtr)
{
    //    MetisControlState *state = ops->closure;
    *remoteAddressPtr = NULL;

    if (parcList_Size(args) < 5) {
        _metisControlAddConnection_EtherHelpExecute(parser, ops, args);
        return MetisCommandReturn_Failure;
    }

    char *symbolic = parcList_GetAtIndex(args, 3);

    if (_validateSymbolicName(symbolic)) {
        char *remoteMacString = parcList_GetAtIndex(args, 4);
        char *localInterface = parcList_GetAtIndex(args, 5);

        if (parcList_Size(args) > 6) {
            // TODO : Parse for ether_type = [ value ]
            *etherEncapType = 0x801;
        } else {
            *etherEncapType = 0x801;
        }
        // This will over-allocate the buffer
        PARCBuffer *remoteMacBuffer = parcBuffer_Allocate(strnlen(remoteMacString, 24));

        bool success = false;
        if (strlen(localInterface) > 0) {
            if (parcNetwork_ParseMAC48Address(remoteMacString, remoteMacBuffer)) {
                parcBuffer_Flip(remoteMacBuffer);
                *remoteAddressPtr = cpiAddress_CreateFromLink(parcBuffer_Overlay(remoteMacBuffer, 0), parcBuffer_Remaining(remoteMacBuffer));
                *localAddressPtr = localInterface;
                *symbolicPtr = symbolic;
                success = true;
            }
        }

        parcBuffer_Release(&remoteMacBuffer);

        return (success ? MetisCommandReturn_Success : MetisCommandReturn_Failure);
    } else {
        printf("Invalid symbolic name.  Must begin with alpha and contain only alphanum.\n");
        return MetisCommandReturn_Failure;
    }
}

static MetisCommandReturn
_metisControlAddConnection_EtherHelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    // ethertype not currently supported

    printf("add connection ether <symbolic> <destination_mac> <local_interface>\n");
    printf("  <symbolic>         : symbolic name, e.g. 'conn1' (must be unique, start with alpha)\n");
    printf("  <destination_mac>  : destination MAC address in hex (optional \":\" or \"-\" separators)\n");
    printf("  <local_interface>  : the name of the local interface (e.g. \"en0\")\n");
    printf("\n");
    printf("Examples:\n");
    printf("   add connection ether conn7 e8-06-88-cd-28-de em3\n");
    printf("      Creates a connection to e8-06-88-cd-28-de on interface em3, ethertype = 0x0801\n");
    printf("\n");
    printf("   add connection ether hal2 00:1c:42:00:00:08 eth0\n");
    printf("     Creates a connection to 00:1c:42:00:00:08 on interface eth0, ethertype = 0x0801\n");
    printf("\n");
    printf("   add connection ether bcast0 FFFFFFFFFFFF eth0\n");
    printf("     Creates a broadcast connection on eth0 with ethertype = 0x0801\n");
    printf("\n");

    return MetisCommandReturn_Success;
}

static MetisCommandReturn
_metisControlAddConnection_EtherExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    CPIAddress *remote_addr;
    char       *local_addr = NULL;
    char       *symbolic = NULL;
    uint16_t ether_EncapType;
    MetisControlState *metis_State = ops->closure;


    if (_metisControl_ParseEtherCommandLine(parser, ops, args, &remote_addr, &local_addr, &ether_EncapType, &symbolic) == MetisCommandReturn_Success) {
        CPIConnectionEthernet *ether_Conn = cpiConnectionEthernet_Create(local_addr, remote_addr, ether_EncapType, symbolic);
        CCNxControl *control_Message = cpiConnectionEthernet_CreateAddMessage(ether_Conn);

        CCNxMetaMessage *msg = ccnxMetaMessage_CreateFromControl(control_Message);
        CCNxMetaMessage *control_Response = metisControlState_WriteRead(metis_State, msg);
        ccnxMetaMessage_Release(&msg);

        if (metisControlState_GetDebug(metis_State)) {
            char *str = parcJSON_ToString(ccnxControl_GetJson(control_Message));
            printf("reponse:\n%s\n", str);
            parcMemory_Deallocate((void **) &str);
        }

        ccnxMetaMessage_Release(&control_Response);
        ccnxControl_Release(&control_Message);
        cpiConnectionEthernet_Release(&ether_Conn);
        cpiAddress_Destroy(&remote_addr);
        return MetisCommandReturn_Success;
    }

    return MetisCommandReturn_Failure;
}

// ===================================================
