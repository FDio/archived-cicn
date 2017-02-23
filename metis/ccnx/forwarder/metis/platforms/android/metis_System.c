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
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#include <errno.h>
#include <string.h>

//#define __USE_MISC
#include <net/if.h>

// to get the list of arp types
#include <net/if_arp.h>

// for the mac address
#include <netpacket/packet.h>

#include <ccnx/api/control/cpi_InterfaceSet.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

#include <LongBow/runtime.h>

#include "ifaddrs.h"

/**
 * Returns the MTU for a named interface
 *
 * On linux, we get the MTU by opening a socket and reading SIOCGIFMTU
 *
 * @param [in] ifname Interface name (e.g. "eth0")
 *
 * @retval number The MTU in bytes
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static int
getMtu(const char *ifname)
{
    struct ifreq ifr;
    int fd;

    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(ifr.ifr_name, ifname);
    ioctl(fd, SIOCGIFMTU, &ifr);

    close(fd);
    return ifr.ifr_mtu;
}

CPIInterfaceSet *
metisSystem_Interfaces(MetisForwarder *metis)
{
    CPIInterfaceSet *set = cpiInterfaceSet_Create();

    MetisLogger *logger = metisForwarder_GetLogger(metis);

    // this is the dynamically allocated head of the list
    struct ifaddrs *ifaddr;
    int failure = getifaddrs(&ifaddr);
    assertFalse(failure, "Error getifaddrs: (%d) %s", errno, strerror(errno));

    struct ifaddrs *next;
    for (next = ifaddr; next != NULL; next = next->ifa_next) {
        if ((next->ifa_addr == NULL) || ((next->ifa_flags & IFF_UP) == 0)) {
            continue;
        }

        CPIInterface *iface = cpiInterfaceSet_GetByName(set, next->ifa_name);
        if (iface == NULL) {
            unsigned mtu = (unsigned) getMtu(next->ifa_name);

            iface = cpiInterface_Create(next->ifa_name,
                                        metisForwarder_GetNextConnectionId(metis),
                                        next->ifa_flags & IFF_LOOPBACK,
                                        next->ifa_flags & IFF_MULTICAST,
                                        mtu);

            cpiInterfaceSet_Add(set, iface);
        }

        int family = next->ifa_addr->sa_family;
        switch (family) {
            case AF_INET: {
                CPIAddress *address = cpiAddress_CreateFromInet((struct sockaddr_in *) next->ifa_addr);
                cpiInterface_AddAddress(iface, address);
                break;
            }

            case AF_INET6: {
                CPIAddress *address = cpiAddress_CreateFromInet6((struct sockaddr_in6 *) next->ifa_addr);
                cpiInterface_AddAddress(iface, address);
                break;
            }

            case AF_PACKET: {
                struct sockaddr_ll *addr_ll = (struct sockaddr_ll *) next->ifa_addr;

                if (metisLogger_IsLoggable(logger, MetisLoggerFacility_IO, PARCLogLevel_Debug)) {
                    metisLogger_Log(logger, MetisLoggerFacility_IO, PARCLogLevel_Debug, __func__,
                                    "sockaddr_ll family %d proto %d ifindex %d hatype %d pkttype %d halen %d",
                                    addr_ll->sll_family,
                                    addr_ll->sll_protocol,
                                    addr_ll->sll_ifindex,
                                    addr_ll->sll_hatype,
                                    addr_ll->sll_pkttype,
                                    addr_ll->sll_halen);
                }

                switch (addr_ll->sll_hatype) {
                    // list of the ARP hatypes we can extract a MAC address from
                    case ARPHRD_ETHER:
                    // fallthrough
                    case ARPHRD_IEEE802: {
                        CPIAddress *address = cpiAddress_CreateFromLink((uint8_t *) addr_ll->sll_addr, addr_ll->sll_halen);
                        cpiInterface_AddAddress(iface, address);
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return set;
}

CPIAddress *
metisSystem_GetMacAddressByName(MetisForwarder *metis, const char *interfaceName)
{
    CPIAddress *linkAddress = NULL;

    CPIInterfaceSet *interfaceSet = metisSystem_Interfaces(metis);
    CPIInterface *interface = cpiInterfaceSet_GetByName(interfaceSet, interfaceName);

    if (interface) {
        const CPIAddressList *addressList = cpiInterface_GetAddresses(interface);

        size_t length = cpiAddressList_Length(addressList);
        for (size_t i = 0; i < length && !linkAddress; i++) {
            const CPIAddress *a = cpiAddressList_GetItem(addressList, i);
            if (cpiAddress_GetType(a) == cpiAddressType_LINK) {
                linkAddress = cpiAddress_Copy(a);
            }
        }
    }

    cpiInterfaceSet_Destroy(&interfaceSet);

    return linkAddress;
}

unsigned
metisSystem_InterfaceMtu(MetisForwarder *metis, const char *interfaceName)
{
    unsigned mtu = 0;

    CPIInterfaceSet *interfaceSet = metisSystem_Interfaces(metis);
    CPIInterface *interface = cpiInterfaceSet_GetByName(interfaceSet, interfaceName);

    if (interface) {
        mtu = cpiInterface_GetMTU(interface);
    }

    cpiInterfaceSet_Destroy(&interfaceSet);

    return mtu;
}
