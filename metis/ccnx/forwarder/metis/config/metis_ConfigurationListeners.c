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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/api/control/cpi_InterfaceSet.h>
#include <ccnx/api/control/cpi_Listener.h>
#include <ccnx/forwarder/metis/core/metis_System.h>

#include <ccnx/forwarder/metis/config/metis_ConfigurationListeners.h>
#include <ccnx/forwarder/metis/io/metis_TcpListener.h>
#include <ccnx/forwarder/metis/io/metis_UdpListener.h>
#include <ccnx/forwarder/metis/io/metis_LocalListener.h>
#include <ccnx/forwarder/metis/io/metis_EtherListener.h>

static bool
_setupTcpListenerOnInet(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    bool success = false;
    struct sockaddr_in addr_sin;
    cpiAddress_GetInet(address, &addr_sin);
    addr_sin.sin_port = htons(port);

    MetisListenerOps *ops = metisTcpListener_CreateInet(metis, addr_sin);
    if (ops) {
        success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        assertTrue(success, "Failed to add TCP listener on %s to ListenerSet", cpiAddress_ToString(ops->getListenAddress(ops)));
    }
    return success;
}

static bool
_setupUdpListenerOnInet(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    bool success = false;
    struct sockaddr_in addr_sin;
    cpiAddress_GetInet(address, &addr_sin);
    addr_sin.sin_port = htons(port);

    MetisListenerOps *ops = metisUdpListener_CreateInet(metis, addr_sin);
    if (ops) {
        success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        assertTrue(success, "Failed to add UDP listener on %s to ListenerSet", cpiAddress_ToString(ops->getListenAddress(ops)));
    }
    return success;
}

static bool
_setupTcpListenerOnInet6(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    bool success = false;
    struct sockaddr_in6 addr_sin6;
    cpiAddress_GetInet6(address, &addr_sin6);
    addr_sin6.sin6_port = htons(port);

    MetisListenerOps *ops = metisTcpListener_CreateInet6(metis, addr_sin6);
    if (ops) {
        success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        assertTrue(success, "Failed to add TCP6 listener on %s to ListenerSet", cpiAddress_ToString(ops->getListenAddress(ops)));
    }
    return success;
}

static bool
_setupUdpListenerOnInet6(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    bool success = false;
    struct sockaddr_in6 addr_sin6;
    cpiAddress_GetInet6(address, &addr_sin6);
    addr_sin6.sin6_port = htons(port);

    MetisListenerOps *ops = metisUdpListener_CreateInet6(metis, addr_sin6);
    if (ops) {
        success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        assertTrue(success, "Failed to add UDP6 listener on %s to ListenerSet", cpiAddress_ToString(ops->getListenAddress(ops)));
    }
    return success;
}

static bool
_setupLocalListener(MetisForwarder *metis, const char *path)
{
    bool success = false;
    MetisListenerOps *ops = metisLocalListener_Create(metis, path);
    if (ops) {
        success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        assertTrue(success, "Failed to add Local listener on %s to ListenerSet", path);
    }
    return success;
}

static MetisListenerOps *
_setupEthernetListenerOnLink(MetisForwarder *metis, const CPIAddress *address, const char *interfaceName, uint16_t ethertype)
{
    MetisListenerOps *ops = metisEtherListener_Create(metis, interfaceName, ethertype);
    if (ops) {
        bool success = metisListenerSet_Add(metisForwarder_GetListenerSet(metis), ops);
        if (!success) {
            metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                            "Failed to add Ethernet listener on %s ethertype 0x%04x to ListenerSet (likely already one on interface)", interfaceName, ethertype);

            // this will null ops for the return value
            ops->destroy(&ops);
        }
    } else {
        metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                        "Could not start Ethernet listener on interface %s\n", interfaceName);
    }
    return ops;
}

static void
_setupListenersOnInet(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    _setupTcpListenerOnInet(metis, address, port);
    _setupUdpListenerOnInet(metis, address, port);
}

static void
_setupListenersOnInet6(MetisForwarder *metis, const CPIAddress *address, uint16_t port)
{
    _setupTcpListenerOnInet6(metis, address, port);
    _setupUdpListenerOnInet6(metis, address, port);
}

static void
_setupListenersOnAddress(MetisForwarder *metis, const CPIAddress *address, uint16_t port, const char *interfaceName)
{
    CPIAddressType type = cpiAddress_GetType(address);
    switch (type) {
        case cpiAddressType_INET:
            _setupListenersOnInet(metis, address, port);
            break;

        case cpiAddressType_INET6:
            _setupListenersOnInet6(metis, address, port);
            break;

        case cpiAddressType_LINK:
            // not used
            break;

        default:
            // dont' know how to handle this, so no listeners
            break;
    }
}

void
metisConfigurationListeners_SetupAll(const MetisConfiguration *config, uint16_t port, const char *localPath)
{
    MetisForwarder *metis = metisConfiguration_GetForwarder(config);
    CPIInterfaceSet *set = metisSystem_Interfaces(metis);

    size_t interfaceSetLength = cpiInterfaceSet_Length(set);
    for (size_t i = 0; i < interfaceSetLength; i++) {
        CPIInterface *iface = cpiInterfaceSet_GetByOrdinalIndex(set, i);

        const CPIAddressList *addresses = cpiInterface_GetAddresses(iface);
        size_t addressListLength = cpiAddressList_Length(addresses);

        for (size_t j = 0; j < addressListLength; j++) {
            const CPIAddress *address = cpiAddressList_GetItem(addresses, j);

            // Do not start on link address
            if (cpiAddress_GetType(address) != cpiAddressType_LINK) {
                _setupListenersOnAddress(metis, address, port, cpiInterface_GetName(iface));
            }
        }
    }

    if (localPath != NULL) {
        _setupLocalListener(metis, localPath);
    }

    cpiInterfaceSet_Destroy(&set);
}


static bool
_addEther(const MetisConfiguration *config, const CPIListener *cpiListener, unsigned ingressId)
{
    bool success = false;
    CPIAddress *mac = metisSystem_GetMacAddressByName(metisConfiguration_GetForwarder(config), cpiListener_GetInterfaceName(cpiListener));
    if (mac) {
        MetisListenerOps *listener = _setupEthernetListenerOnLink(metisConfiguration_GetForwarder(config), mac, cpiListener_GetInterfaceName(cpiListener), cpiListener_GetEtherType(cpiListener));
        success = (listener != NULL);
        cpiAddress_Destroy(&mac);
    }
    return success;
}

static bool
_addIP(const MetisConfiguration *config, const CPIListener *cpiListener, unsigned ingressId)
{
    bool success = false;
    CPIAddress *localAddress = cpiListener_GetAddress(cpiListener);

    switch (cpiAddress_GetType(localAddress)) {
        case cpiAddressType_INET: {
            // The CPI address, in this case, has the port inside it, so use that
            struct sockaddr_in sin;
            cpiAddress_GetInet(localAddress, &sin);
            if (cpiListener_IsProtocolUdp(cpiListener)) {
                success = _setupUdpListenerOnInet(metisConfiguration_GetForwarder(config), localAddress, htons(sin.sin_port));
            } else if (cpiListener_IsProtocolTcp(cpiListener)) {
                success = _setupTcpListenerOnInet(metisConfiguration_GetForwarder(config), localAddress, htons(sin.sin_port));
            }
            break;
        }

        case cpiAddressType_INET6: {
            // The CPI address, in this case, has the port inside it, so use that
            struct sockaddr_in6 sin6;
            cpiAddress_GetInet6(localAddress, &sin6);
            if (cpiListener_IsProtocolUdp(cpiListener)) {
                success = _setupUdpListenerOnInet6(metisConfiguration_GetForwarder(config), localAddress, htons(sin6.sin6_port));
            } else if (cpiListener_IsProtocolTcp(cpiListener)) {
                success = _setupTcpListenerOnInet6(metisConfiguration_GetForwarder(config), localAddress, htons(sin6.sin6_port));
            }
            break;
        }

        default:
            if (metisLogger_IsLoggable(metisConfiguration_GetLogger(config), MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
                char *str = cpiAddress_ToString(localAddress);
                metisLogger_Log(metisConfiguration_GetLogger(config), MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                                "Unsupported address type for IP encapsulation ingress id %u: %s",
                                ingressId,
                                str);
                parcMemory_Deallocate((void **) &str);
            }
            break;
    }

    if (success) {
        if (metisLogger_IsLoggable(metisConfiguration_GetLogger(config), MetisLoggerFacility_Config, PARCLogLevel_Info)) {
            char *str = cpiAddress_ToString(localAddress);
            metisLogger_Log(metisConfiguration_GetLogger(config), MetisLoggerFacility_Config, PARCLogLevel_Info, __func__,
                            "Setup listener on address %s",
                            str);
            parcMemory_Deallocate((void **) &str);
        }
    }

    return success;
}

bool
metisConfigurationListeners_Add(const MetisConfiguration *config, const CCNxControl *control, unsigned ingressId)
{
    bool success = false;
    CPIListener *cpiListener = cpiListener_FromControl(control);
    if (cpiListener) {
        if (cpiListener_IsEtherEncap(cpiListener)) {
            success = _addEther(config, cpiListener, ingressId);
        } else if (cpiListener_IsIPEncap(cpiListener)) {
            success = _addIP(config, cpiListener, ingressId);
        } else {
            MetisLogger *logger = metisConfiguration_GetLogger(config);
            if (metisLogger_IsLoggable(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
                PARCJSON *json = ccnxControl_GetJson(control);
                char *str = parcJSON_ToCompactString(json);
                metisLogger_Log(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                                "Unsupported encapsulation ingress %u control %s",
                                ingressId,
                                str);
                parcMemory_Deallocate((void **) &str);
            }
        }

        cpiListener_Release(&cpiListener);
    } else {
        MetisLogger *logger = metisConfiguration_GetLogger(config);
        if (metisLogger_IsLoggable(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
            PARCJSON *json = ccnxControl_GetJson(control);
            char *str = parcJSON_ToCompactString(json);
            metisLogger_Log(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                            "Could not parse control message ingress %u control %s",
                            ingressId,
                            str);
            parcMemory_Deallocate((void **) &str);
        }
    }
    return success;
}

bool
metisConfigurationListeners_Remove(const MetisConfiguration *config, const CCNxControl *control, unsigned ingressId)
{
    MetisLogger *logger = metisConfiguration_GetLogger(config);
    if (metisLogger_IsLoggable(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning)) {
        PARCJSON *json = ccnxControl_GetJson(control);
        char *str = parcJSON_ToCompactString(json);
        metisLogger_Log(logger, MetisLoggerFacility_Config, PARCLogLevel_Warning, __func__,
                        "Removing a listener not supported: ingress %u control %s",
                        ingressId,
                        str);
        parcMemory_Deallocate((void **) &str);
    }

    return false;
}

