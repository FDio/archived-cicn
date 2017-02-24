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

/**
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashMap.h>
#include <parc/algol/parc_String.h>

#include <ccnx/api/control/cpi_InterfaceSet.h>
#include <ccnx/api/control/cpi_ConnectionEthernet.h>
#include <ccnx/api/control/cpi_Listener.h>
#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnelList.h>
#include <ccnx/api/control/cpi_ForwardingStrategy.h>

#include <ccnx/forwarder/metis/config/metis_CommandLineInterface.h>
#include <ccnx/forwarder/metis/config/metis_SymbolicNameTable.h>
#include <ccnx/forwarder/metis/config/metis_ConfigurationListeners.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_System.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionTable.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>

#include <ccnx/forwarder/metis/io/metis_TcpTunnel.h>
#include <ccnx/forwarder/metis/io/metis_UdpTunnel.h>
#include <ccnx/forwarder/metis/io/metis_UdpConnection.h>
#include <ccnx/forwarder/metis/io/metis_EtherListener.h>
#include <ccnx/forwarder/metis/io/metis_EtherConnection.h>

#include <ccnx/forwarder/metis/metis_About.h>

#define ETHERTYPE 0x0801

struct metis_configuration {
    MetisForwarder *metis;
    MetisLogger *logger;
    MetisCommandLineInterface *cli;

    size_t maximumContentObjectStoreSize;

    // map from prefix to strategy
    PARCHashMap *strategy_map;

    // translates between a symblic name and a connection id
    MetisSymbolicNameTable *symbolicNameTable;
};


// ========================================================================================

MetisConfiguration *
metisConfiguration_Create(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    MetisConfiguration *config = parcMemory_AllocateAndClear(sizeof(MetisConfiguration));
    assertNotNull(config, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisConfiguration));
    config->metis = metis;
    config->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));
    config->cli = NULL;
    config->maximumContentObjectStoreSize = 100000;
    config->strategy_map = parcHashMap_Create();
    config->symbolicNameTable = metisSymbolicNameTable_Create();

    return config;
}

void
metisConfiguration_Destroy(MetisConfiguration **configPtr)
{
    assertNotNull(configPtr, "Parameter must be non-null double poitner");
    assertNotNull(*configPtr, "Parameter must dereference to non-null pointer");

    MetisConfiguration *config = *configPtr;
    metisLogger_Release(&config->logger);
    if (config->cli != NULL) {
        metisCommandLineInterface_Destroy(&config->cli);
    }
    parcHashMap_Release(&(config->strategy_map));
    metisSymbolicNameTable_Destroy(&config->symbolicNameTable);
    parcMemory_Deallocate((void **) &config);
    *configPtr = NULL;
}

void
metisConfiguration_StartCLI(MetisConfiguration *config, uint16_t port)
{
    assertNull(config->cli, "You cannot start more than one CLI");
    config->cli = metisCommandLineInterface_Create(config->metis, port);
    metisCommandLineInterface_Start(config->cli);
}

PARCJSON *
metisConfiguration_GetVersion(MetisConfiguration *config)
{
    PARCJSON *fwd = parcJSON_Create();
    parcJSON_AddString(fwd, "NAME", metisAbout_Name());
    parcJSON_AddString(fwd, "COPYRIGHT", metisAbout_MiniNotice());
    parcJSON_AddString(fwd, "VERSION", metisAbout_Version());
    return fwd;
}

static void
metisConfiguration_SendResponse(MetisConfiguration *config, CCNxControl *response, unsigned egressId)
{
    PARCBuffer *responseBuffer = metisTlv_EncodeControlPlaneInformation(response);
    MetisMessage *tlvEncodedResponse = metisMessage_CreateFromParcBuffer(responseBuffer, 0, metisForwarder_GetTicks(config->metis), metisForwarder_GetLogger(config->metis));

    parcBuffer_Release(&responseBuffer);

    MetisConnectionTable *connectionTable = metisForwarder_GetConnectionTable(config->metis);
    const MetisConnection *conn = metisConnectionTable_FindById(connectionTable, egressId);
    assertNotNull(conn, "Got null connection for control message we just received");

    metisConnection_Send(conn, tlvEncodedResponse);
    metisMessage_Release(&tlvEncodedResponse);
}

static CCNxControl *
_createNack(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    PARCJSON *json = ccnxControl_GetJson(control);

    PARCJSON *jsonNack = cpiAcks_CreateNack(json);

    CCNxControl *response = ccnxControl_CreateCPIRequest(jsonNack);
    parcJSON_Release(&jsonNack);
    return response;
}

static CCNxControl *
_createAck(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    PARCJSON *json = ccnxControl_GetJson(control);
    PARCJSON *jsonAck = cpiAcks_CreateAck(json);

    CCNxControl *response = ccnxControl_CreateCPIRequest(jsonAck);
    parcJSON_Release(&jsonAck);
    return response;
}


static CCNxControl *
metisConfiguration_ProcessForwarderVersion(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    PARCJSON *fwd = metisConfiguration_GetVersion(config);

    // this process of getting to a MetisMesage needs streamlining

    CCNxControl *response = cpi_CreateResponse(request, fwd);
    return response;
}

static CCNxControl *
metisConfiguration_ProcessInterfaceList(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    CPIInterfaceSet *set = metisSystem_Interfaces(config->metis);
    PARCJSON *setJson = cpiInterfaceSet_ToJson(set);

    CCNxControl *response = cpi_CreateResponse(request, setJson);
    parcJSON_Release(&setJson);
    cpiInterfaceSet_Destroy(&set);
    return response;
}

static bool
_symbolicRegisterPrefix(MetisConfiguration *config, CPIRouteEntry *route)
{
    bool success = false;

    const char *symbolic = cpiRouteEntry_GetSymbolicName(route);
    unsigned ifidx = metisSymbolicNameTable_Get(config->symbolicNameTable, symbolic);
    if (ifidx != UINT32_MAX) {
        cpiRouteEntry_SetInterfaceIndex(route, ifidx);
        if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug)) {
            metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug, __func__,
                            "Add route resolve name '%s' to connid %u",
                            symbolic, ifidx);
        }

        success = metisForwarder_AddOrUpdateRoute(config->metis, route);
    } else {
        if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
            metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                            "Add route symbolic name '%s' could not be resolved", symbolic);
        }
        // this is a failure
    }
    return success;
}

static CCNxControl *
metisConfiguration_ProcessRegisterPrefix(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    CPIRouteEntry *route = cpiForwarding_RouteFromControlMessage(control);

    bool success = false;

    // if it has a symbolic name set the interface index
    if (cpiRouteEntry_GetSymbolicName(route) != NULL) {
        success = _symbolicRegisterPrefix(config, route);
    } else {
        if (cpiRouteEntry_GetInterfaceIndex(route) == CPI_CURRENT_INTERFACE) {
            // We want to use the ingress interface
            cpiRouteEntry_SetInterfaceIndex(route, ingressId);
        }
        success = metisForwarder_AddOrUpdateRoute(config->metis, route);
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, control, ingressId);
    } else {
        response = _createNack(config, control, ingressId);
    }

    cpiRouteEntry_Destroy(&route);
    return response;
}

static CCNxControl *
metisConfiguration_ProcessUnregisterPrefix(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    CPIRouteEntry *route = cpiForwarding_RouteFromControlMessage(control);

    bool success = false;

    if (cpiRouteEntry_GetSymbolicName(route) != NULL) {
        const char *symbolic = cpiRouteEntry_GetSymbolicName(route);
        unsigned ifidx = metisSymbolicNameTable_Get(config->symbolicNameTable, symbolic);
        if (ifidx != UINT32_MAX) {
            cpiRouteEntry_SetInterfaceIndex(route, ifidx);
            success = metisForwarder_RemoveRoute(config->metis, route);
        } else {
            // this is a failure
            success = false;
        }
    } else {
        if (cpiRouteEntry_GetInterfaceIndex(route) == CPI_CURRENT_INTERFACE) {
            // We want to use the ingress interface
            cpiRouteEntry_SetInterfaceIndex(route, ingressId);
        }
        success = metisForwarder_RemoveRoute(config->metis, route);
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, control, ingressId);
    } else {
        response = _createNack(config, control, ingressId);
    }

    cpiRouteEntry_Destroy(&route);
    return response;
}

static CCNxControl *
metisConfiguration_ProcessRegistrationList(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    MetisFibEntryList *fibList = metisForwarder_GetFibEntries(config->metis);

    CPIRouteEntryList *routeEntryList = cpiRouteEntryList_Create();
    for (size_t i = 0; i < metisFibEntryList_Length(fibList); i++) {
        const MetisFibEntry *fibEntry = metisFibEntryList_Get(fibList, i);
        MetisTlvName *prefix = metisFibEntry_GetPrefix(fibEntry);
        const MetisNumberSet *nexthops = metisFibEntry_GetNexthops(fibEntry);

        for (int j = 0; j < metisNumberSet_Length(nexthops); j++) {
            CPIRouteEntry *route = cpiRouteEntry_Create(metisTlvName_ToCCNxName(prefix),
                                                        metisNumberSet_GetItem(nexthops, j),
                                                        NULL,
                                                        cpiNameRouteProtocolType_STATIC,
                                                        cpiNameRouteType_LONGEST_MATCH,
                                                        NULL, // lifetime
                                                        1);   // cost
            cpiRouteEntryList_Append(routeEntryList, route);
        }

        metisTlvName_Release(&prefix);
    }
    PARCJSON *entryListJson = cpiRouteEntryList_ToJson(routeEntryList);
    CCNxControl *response = cpi_CreateResponse(request, entryListJson);
    parcJSON_Release(&entryListJson);
    cpiRouteEntryList_Destroy(&routeEntryList);
    metisFibEntryList_Destroy(&fibList);
    return response;
}

static void
_logProcessCreateTunnelMessage(MetisConfiguration *config, CPIInterfaceIPTunnel *iptun, PARCLogLevel logLevel, const char *message)
{
    if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Info)) {
        const CPIAddress *source = cpiInterfaceIPTunnel_GetSourceAddress(iptun);
        const CPIAddress *destination = cpiInterfaceIPTunnel_GetDestinationAddress(iptun);

        const char *symbolicName = cpiInterfaceIPTunnel_GetSymbolicName(iptun);

        char *sourceString = cpiAddress_ToString(source);
        char *remoteString = cpiAddress_ToString(destination);

        metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Info, __func__,
                        "Add connection %s on %s to %s: %s",
                        symbolicName,
                        sourceString,
                        remoteString,
                        message);

        parcMemory_Deallocate((void **) &sourceString);
        parcMemory_Deallocate((void **) &remoteString);
    }
}

/**
 * Add an IP-based tunnel.
 *
 * The call cal fail if the symbolic name is a duplicate.  It could also fail if there's an problem creating
 * the local side of the tunnel (i.e. the local socket address is not usable).
 *
 * @return true Tunnel added
 * @return false Tunnel not added (an error)
 */
static CCNxControl *
metisConfiguration_ProcessCreateTunnel(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    bool success = false;

    CPIInterfaceIPTunnel *iptun = cpiLinks_CreateIPTunnelFromControlMessage(control);

    const char *symbolicName = cpiInterfaceIPTunnel_GetSymbolicName(iptun);

    if (!metisSymbolicNameTable_Exists(config->symbolicNameTable, symbolicName)) {
        const CPIAddress *source = cpiInterfaceIPTunnel_GetSourceAddress(iptun);
        const CPIAddress *destination = cpiInterfaceIPTunnel_GetDestinationAddress(iptun);

        MetisIoOperations *ops = NULL;
        switch (cpiInterfaceIPTunnel_GetTunnelType(iptun)) {
            case IPTUN_TCP:
                ops = metisTcpTunnel_Create(config->metis, source, destination);
                break;
            case IPTUN_UDP:
                ops = metisUdpTunnel_Create(config->metis, source, destination);
                break;
            case IPTUN_GRE:
                metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                                "Unsupported tunnel protocol: GRE");
                break;
            default:
                metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                                "Unsupported tunnel protocol: %d",
                                cpiInterfaceIPTunnel_GetTunnelType(iptun));
                break;
        }


        if (ops != NULL) {
            MetisConnection *conn = metisConnection_Create(ops);

            const char *suffix = "wldr";
            size_t suffix_len = 4;
            size_t name_len = strlen(symbolicName);
            if (suffix_len < name_len && (strncmp(symbolicName + name_len - suffix_len, suffix, suffix_len) == 0)) {
                printf("WARNING: WLDR initialized with legacy code. please use the command set wldr <on|off> <connId>\n");
                printf("see metis_control help for more information\n");
                metisConnection_EnableWldr(conn);
            }

            metisConnectionTable_Add(metisForwarder_GetConnectionTable(config->metis), conn);
            metisSymbolicNameTable_Add(config->symbolicNameTable, symbolicName, metisConnection_GetConnectionId(conn));

            success = true;

            _logProcessCreateTunnelMessage(config, iptun, PARCLogLevel_Info, "success");
        } else {
            _logProcessCreateTunnelMessage(config, iptun, PARCLogLevel_Warning, "failed, could not create IoOperations");
        }
    } else {
        _logProcessCreateTunnelMessage(config, iptun, PARCLogLevel_Warning, "failed, symbolic name exists");
    }

    // send the ACK or NACK
    CCNxControl *response;
    if (success) {
        response = _createAck(config, control, ingressId);
    } else {
        response = _createNack(config, control, ingressId);
    }

    cpiInterfaceIPTunnel_Release(&iptun);

    return response;
}

static bool
_metisConfiguration_AddConnectionEthernet(MetisConfiguration *config, CPIConnectionEthernet *etherConn, CPIAddress *linkAddress, MetisListenerOps *listenerOps)
{
    bool success = false;

    const char *symbolic = cpiConnectionEthernet_GetSymbolicName(etherConn);
    if (!metisSymbolicNameTable_Exists(config->symbolicNameTable, symbolic)) {
        const CPIAddress *remote = cpiConnectionEthernet_GetPeerLinkAddress(etherConn);
        MetisAddressPair *pair = metisAddressPair_Create(linkAddress, remote);

        MetisGenericEther *ether = metisEtherListener_GetGenericEtherFromListener(listenerOps);

        if (ether) {
            MetisIoOperations *ops = metisEtherConnection_Create(config->metis, ether, pair);

            if (ops) {
                MetisConnection *conn = metisConnection_Create(ops);
                assertNotNull(conn, "Failed to create connection");

                metisConnectionTable_Add(metisForwarder_GetConnectionTable(config->metis), conn);
                metisSymbolicNameTable_Add(config->symbolicNameTable, symbolic, metisConnection_GetConnectionId(conn));

                success = true;

                if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug)) {
                    char *peerAddressString = cpiAddress_ToString(remote);
                    metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug, __func__,
                                    "Add connection %s on %s to %s, connid %u",
                                    symbolic,
                                    cpiConnectionEthernet_GetInterfaceName(etherConn),
                                    peerAddressString,
                                    metisConnection_GetConnectionId(conn));
                    parcMemory_Deallocate((void **) &peerAddressString);
                }
            }
        } else {
            metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                            "Could not get MetisGenericEther for listener %p",
                            listenerOps);
        }

        metisAddressPair_Release(&pair);
    } else {
        if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
            const CPIAddress *remote = cpiConnectionEthernet_GetPeerLinkAddress(etherConn);
            char *peerAddressString = cpiAddress_ToString(remote);
            metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                            "Add connection %s on %s to %s failed, symbolic name exists",
                            symbolic,
                            cpiConnectionEthernet_GetInterfaceName(etherConn),
                            peerAddressString);
            parcMemory_Deallocate((void **) &peerAddressString);
        }
    }

    return success;
}


static CCNxControl *
metisConfiguration_ProcessAddConnectionEthernet(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    bool success = false;

    CPIConnectionEthernet *etherConn = cpiConnectionEthernet_FromControl(control);
    assertNotNull(etherConn, "Control message did not parse to CPIConnectionEthernet");

    if (cpiConnectionEthernet_GetEthertype(etherConn) == ETHERTYPE) {
        CPIAddress *linkAddress = metisSystem_GetMacAddressByName(config->metis, cpiConnectionEthernet_GetInterfaceName(etherConn));
        if (linkAddress != NULL) {
            MetisListenerSet *listenerSet = metisForwarder_GetListenerSet(config->metis);
            MetisListenerOps *listenerOps = metisListenerSet_Find(listenerSet, METIS_ENCAP_ETHER, linkAddress);

            if (listenerOps) {
                // Now add the connection
                success = _metisConfiguration_AddConnectionEthernet(config, etherConn, linkAddress, listenerOps);
            } else {
                char *str = cpiAddress_ToString(linkAddress);
                metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                                "Could not find listener for interface '%s' addr %s ethertype 0x%04x",
                                cpiConnectionEthernet_GetInterfaceName(etherConn),
                                str,
                                cpiConnectionEthernet_GetEthertype(etherConn));
                parcMemory_Deallocate((void **) &str);
            }


            cpiAddress_Destroy(&linkAddress);
        } else {
            metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                            "Could not resolve interface '%s' to a MAC address",
                            cpiConnectionEthernet_GetInterfaceName(etherConn));
        }
    } else {
        metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                        "Control message asked for ethertype %04x, only %04x supported",
                        cpiConnectionEthernet_GetEthertype(etherConn), ETHERTYPE);
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, control, ingressId);
    } else {
        response = _createNack(config, control, ingressId);
    }

    cpiConnectionEthernet_Release(&etherConn);
    return response;
}

static CCNxControl *
metisConfiguration_ProcessRemoveConnectionEthernet(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    printf("%s not implemented\n", __func__);
    return _createNack(config, control, ingressId);
}


static CCNxControl *
metisConfiguration_ProcessRemoveTunnel(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    CPIInterfaceIPTunnel *iptun = cpiLinks_CreateIPTunnelFromControlMessage(control);
    const char *symbolic = cpiInterfaceIPTunnel_GetSymbolicName(iptun);
    unsigned ifidx = metisSymbolicNameTable_Get(config->symbolicNameTable, symbolic);
    if (ifidx != UINT32_MAX) {
        //remove connection from the FIB
        metisForwarder_RemoveConnectionIdFromRoutes(config->metis, ifidx);
        MetisConnectionTable *table = metisForwarder_GetConnectionTable(config->metis);
        //remove connection from connection table
        metisConnectionTable_RemoveById(table, ifidx);
        //remove connection from symbolicNameTable
        metisSymbolicNameTable_Remove(config->symbolicNameTable, symbolic);
        //and ... this is it!
        CCNxControl *response = _createAck(config, control, ingressId);
        cpiInterfaceIPTunnel_Release(&iptun);
        return response;
    } else  {
        CCNxControl *response = _createNack(config, control, ingressId);
        cpiInterfaceIPTunnel_Release(&iptun);
        return response;
    }
}

static CCNxControl *
metisConfiguration_ProcessConnectionList(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    CPIConnectionList *tunnelList = cpiConnectionList_Create();

    MetisConnectionTable *table = metisForwarder_GetConnectionTable(config->metis);
    MetisConnectionList *connList = metisConnectionTable_GetEntries(table);

    for (size_t i = 0; i < metisConnectionList_Length(connList); i++) {
        // Don't release original, we're not storing it
        MetisConnection *original = metisConnectionList_Get(connList, i);
        const MetisAddressPair *addressPair = metisConnection_GetAddressPair(original);
        CPIAddress *localAddress = cpiAddress_Copy(metisAddressPair_GetLocal(addressPair));
        CPIAddress *remoteAddress = cpiAddress_Copy(metisAddressPair_GetRemote(addressPair));

        CPIConnectionType type = metisIoOperations_GetConnectionType(metisConnection_GetIoOperations(original));

        CPIConnection *cpiConn = cpiConnection_Create(metisConnection_GetConnectionId(original),
                                                      localAddress,
                                                      remoteAddress,
                                                      type);

        cpiConnection_SetState(cpiConn, metisConnection_IsUp(original) ? CPI_IFACE_UP : CPI_IFACE_DOWN);
        cpiConnectionList_Append(tunnelList, cpiConn);
    }

    PARCJSON *connectListJson = cpiConnectionList_ToJson(tunnelList);
    CCNxControl *response = cpi_CreateResponse(request, connectListJson);
    parcJSON_Release(&connectListJson);
    cpiConnectionList_Destroy(&tunnelList);
    metisConnectionList_Destroy(&connList);

    return response;
}

static CCNxControl *
metisConfiguration_ProcessCacheStoreOn(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    bool success = false;

    metisForwarder_SetChacheStoreFlag(config->metis, true);
    if (metisForwarder_GetChacheStoreFlag(config->metis)) {
        success = true;
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, request, ingressId);
    } else {
        response = _createNack(config, request, ingressId);
    }
    return response;
}

static CCNxControl *
metisConfiguration_ProcessCacheStoreOff(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    bool success = false;

    metisForwarder_SetChacheStoreFlag(config->metis, false);
    if (!metisForwarder_GetChacheStoreFlag(config->metis)) {
        success = true;
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, request, ingressId);
    } else {
        response = _createNack(config, request, ingressId);
    }
    return response;
}

static CCNxControl *
metisConfiguration_ProcessCacheServeOn(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    bool success = true;

    metisForwarder_SetChacheServeFlag(config->metis, true);
    if (metisForwarder_GetChacheServeFlag(config->metis)) {
        success = true;
    }

    CCNxControl *response;
    if (success) {
        response = _createAck(config, request, ingressId);
    } else {
        response = _createNack(config, request, ingressId);
    }
    return response;
}

static CCNxControl *
metisConfiguration_ProcessCacheServeOff(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    bool success = true;

    metisForwarder_SetChacheServeFlag(config->metis, false);
    if (!metisForwarder_GetChacheServeFlag(config->metis)) {
        success = true;
    }


    CCNxControl *response;
    if (success) {
        response = _createAck(config, request, ingressId);
    } else {
        response = _createNack(config, request, ingressId);
    }
    return response;
}

static CCNxControl *
metisConfiguration_ProcessCacheClear(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    metisForwarder_ClearCache(config->metis);

    CCNxControl *response = _createAck(config, request, ingressId);

    return response;
}


size_t
metisConfiguration_GetObjectStoreSize(MetisConfiguration *config)
{
    return config->maximumContentObjectStoreSize;
}

static CCNxControl *
metisConfiguration_SetForwarginStrategy(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    CPIForwardingStrategy *fwdStrategy = cpiForwarding_ForwardingStrategyFromControlMessage(request);

    const CCNxName *prefix = cpiForwardingStrategy_GetPrefix(fwdStrategy);
    const char *strategy = cpiForwardingStrategy_GetStrategy(fwdStrategy);
    const char *existingFwdStrategy = metisConfiguration_GetForwarginStrategy(config, prefix);

    if ((existingFwdStrategy == NULL) || (strcmp(strategy, existingFwdStrategy) != 0)) {
        PARCString *fwdStrategy = parcString_Create(strategy);
        parcHashMap_Put(config->strategy_map, prefix, fwdStrategy);
        metisForwarder_SetStrategy(config->metis, (CCNxName *) prefix, (char *) strategy);
    }

    cpiForwardingStrategy_Destroy(&fwdStrategy);
    CCNxControl *response = _createAck(config, request, ingressId);
    return response;
}

static CCNxControl *
metisConfiguration_SetWldr(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    CPIManageWldr *cpiWldr = cpiLinks_ManageWldrFromControlMessage(request);
    
    const char * connId = cpiManageWldr_GetConnection(cpiWldr);
    bool active = cpiManageWldr_IsActive(cpiWldr);
   
    bool success = false;
    unsigned ifidx = metisSymbolicNameTable_Get(config->symbolicNameTable, connId);
    if (ifidx != UINT32_MAX) {
        MetisConnectionTable *table = metisForwarder_GetConnectionTable(config->metis);
        MetisConnection *conn = (MetisConnection *) metisConnectionTable_FindById(table, ifidx);
        if(conn == NULL){
            success = false;
        } else {
            if(active){
                metisConnection_EnableWldr(conn);
            } else {
                metisConnection_DisableWldr(conn);
            }
            success = true;
        }
    } else  {
        success = false;
    }
    
    CCNxControl *response;
    if(success){
        response = _createAck(config, request, ingressId);
    } else {
        response = _createNack(config, request, ingressId);
    }

    cpiManageWldr_Destroy(&cpiWldr);
    return response;
}

const char *
metisConfiguration_GetForwarginStrategy(MetisConfiguration *config, const CCNxName *prefix)
{
    const PARCString *val = parcHashMap_Get(config->strategy_map, prefix);
    if (val == NULL) {
        return NULL;
    } else  {
        return (const char *) parcString_ToString(val);
    }
}

void
metisConfiguration_SetObjectStoreSize(MetisConfiguration *config, size_t maximumObjectCount)
{
    config->maximumContentObjectStoreSize = maximumObjectCount;

    metisForwarder_SetContentObjectStoreSize(config->metis, config->maximumContentObjectStoreSize);
}

MetisForwarder *
metisConfiguration_GetForwarder(const MetisConfiguration *config)
{
    return config->metis;
}

MetisLogger *
metisConfiguration_GetLogger(const MetisConfiguration *config)
{
    return config->logger;
}

// ===========================
// Main functions that deal with receiving commands, executing them, and sending ACK/NACK

static CCNxControl *
metisConfiguration_DispatchCommandOldStyle(MetisConfiguration *config, CCNxControl *control, unsigned ingressId)
{
    CCNxControl *response = NULL;
    switch (cpi_GetMessageOperation(control)) {
        case CPI_FORWARDER_VERSION:
            response = metisConfiguration_ProcessForwarderVersion(config, control, ingressId);
            break;

        case CPI_INTERFACE_LIST:
            response = metisConfiguration_ProcessInterfaceList(config, control, ingressId);
            break;

        case CPI_PREFIX_REGISTRATION_LIST:
            response = metisConfiguration_ProcessRegistrationList(config, control, ingressId);
            break;

        case CPI_REGISTER_PREFIX:
            response = metisConfiguration_ProcessRegisterPrefix(config, control, ingressId);
            break;

        case CPI_UNREGISTER_PREFIX:
            response = metisConfiguration_ProcessUnregisterPrefix(config, control, ingressId);
            break;

        case CPI_CREATE_TUNNEL:
            response = metisConfiguration_ProcessCreateTunnel(config, control, ingressId);
            break;

        case CPI_REMOVE_TUNNEL:
            response = metisConfiguration_ProcessRemoveTunnel(config, control, ingressId);
            break;

        case CPI_CONNECTION_LIST:
            response = metisConfiguration_ProcessConnectionList(config, control, ingressId);
            break;

        case CPI_PAUSE:
            break;

        case CPI_CACHE_STORE_ON:
            response = metisConfiguration_ProcessCacheStoreOn(config, control, ingressId);
            break;

        case CPI_CACHE_STORE_OFF:
            response = metisConfiguration_ProcessCacheStoreOff(config, control, ingressId);
            break;

        case CPI_CACHE_SERVE_ON:
            response = metisConfiguration_ProcessCacheServeOn(config, control, ingressId);
            break;

        case CPI_CACHE_SERVE_OFF:
            response = metisConfiguration_ProcessCacheServeOff(config, control, ingressId);
            break;

        case CPI_CACHE_CLEAR:
            response = metisConfiguration_ProcessCacheClear(config, control, ingressId);
            break;

        case CPI_SET_FORWARDING_STRATEGY:
            response = metisConfiguration_SetForwarginStrategy(config, control, ingressId);
            break;
        
        case CPI_SET_WLDR:
            response = metisConfiguration_SetWldr(config, control, ingressId);
            break;

        default:
            break;
    }

    return response;
}

static CCNxControl *
_processControl(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    CCNxControl *response = NULL;

    switch (cpi_GetMessageType(request)) {
        case CPI_REQUEST: {
            if (cpiConnectionEthernet_IsAddMessage(request)) {
                response = metisConfiguration_ProcessAddConnectionEthernet(config, request, ingressId);
            } else if (cpiConnectionEthernet_IsRemoveMessage(request)) {
                response = metisConfiguration_ProcessRemoveConnectionEthernet(config, request, ingressId);
            } else if (cpiListener_IsAddMessage(request)) {
                bool success = metisConfigurationListeners_Add(config, request, ingressId);
                if (success) {
                    response = _createAck(config, request, ingressId);
                } else {
                    response = _createNack(config, request, ingressId);
                }
            } else if (cpiListener_IsRemoveMessage(request)) {
                bool success = metisConfigurationListeners_Remove(config, request, ingressId);
                if (success) {
                    response = _createAck(config, request, ingressId);
                } else {
                    response = _createNack(config, request, ingressId);
                }
            } else {
                response = metisConfiguration_DispatchCommandOldStyle(config, request, ingressId);
            }
            break;
        }

        default:
            break;
    }

    assertNotNull(response, "Got null CCNxControl response");
    return response;
}

CCNxControl *
metisConfiguration_ReceiveControl(MetisConfiguration *config, CCNxControl *request, unsigned ingressId)
{
    assertNotNull(config, "Parameter config must be non-null");
    assertNotNull(request, "Parameter request must be non-null");

    CCNxControl *response = _processControl(config, request, ingressId);
    return response;
}

void
metisConfiguration_Receive(MetisConfiguration *config, MetisMessage *message)
{
    assertNotNull(config, "Parameter config must be non-null");
    assertNotNull(message, "Parameter message must be non-null");
    assertTrue(metisMessage_GetType(message) == MetisMessagePacketType_Control,
               "Message must be type CPI, expected %02x got %02x",
               MetisMessagePacketType_Control, metisMessage_GetType(message));

    CCNxControl *control = metisMessage_CreateControlMessage(message);
    unsigned ingressId = metisMessage_GetIngressConnectionId(message);

    if (metisLogger_IsLoggable(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug)) {
        char *str = parcJSON_ToCompactString(ccnxControl_GetJson(control));
        metisLogger_Log(config->logger, MetisLoggerFacility_Config, PARCLogLevel_Debug, __func__,
                        "%s received %s\n", __func__, str);
        parcMemory_Deallocate((void **) &str);
    }

    CCNxControl *response = _processControl(config, control, ingressId);
    metisConfiguration_SendResponse(config, response, ingressId);
    ccnxControl_Release(&response);

    ccnxControl_Release(&control);
    metisMessage_Release(&message);
}


