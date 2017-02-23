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
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <ccnx/forwarder/metis/io/metis_UdpTunnel.h>
#include <ccnx/forwarder/metis/io/metis_UdpConnection.h>

#include <LongBow/runtime.h>

MetisIoOperations *
metisUdpTunnel_CreateOnListener(MetisForwarder *metis, MetisListenerOps *localListener, const CPIAddress *remoteAddress)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(localListener, "Parameter localListener must be non-null");
    assertNotNull(remoteAddress, "Parameter remoteAddress must be non-null");

    MetisLogger *logger = metisForwarder_GetLogger(metis);

    MetisIoOperations *ops = NULL;
    if (localListener->getEncapType(localListener) == METIS_ENCAP_UDP) {
        const CPIAddress *localAddress = localListener->getListenAddress(localListener);
        CPIAddressType localType = cpiAddress_GetType(localAddress);
        CPIAddressType remoteType = cpiAddress_GetType(remoteAddress);

        if (localType == remoteType) {
            MetisAddressPair *pair = metisAddressPair_Create(localAddress, remoteAddress);
            bool isLocal = false;
            int fd = localListener->getSocket(localListener);
            ops = metisUdpConnection_Create(metis, fd, pair, isLocal);
            
            metisAddressPair_Release(&pair);
        } else {
            if (metisLogger_IsLoggable(logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
                metisLogger_Log(logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                                "Local listener of type %s and remote type %s, cannot establish tunnel",
                                cpiAddress_TypeToString(localType),
                                cpiAddress_TypeToString(remoteType));
            }
        }
    } else {
        if (metisLogger_IsLoggable(logger, MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            metisLogger_Log(logger, MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "Local listener %p is not type UDP, cannot establish tunnel", (void *) localListener);
        }
    }

    return ops;
}

/*
 * wrapper for metisUdpTunnel_CreateOnListener.  Lookup to see if we have a listener on the local address.
 * If so, call metisUdpTunnel_CreateOnListener, otherwise return NULL
 */
MetisIoOperations *
metisUdpTunnel_Create(MetisForwarder *metis, const CPIAddress *localAddress, const CPIAddress *remoteAddress)
{
    MetisListenerSet *set = metisForwarder_GetListenerSet(metis);
    MetisListenerOps *listener = metisListenerSet_Find(set, METIS_ENCAP_UDP, localAddress);
    MetisIoOperations *ops = NULL;
    if (listener) {
        ops = metisUdpTunnel_CreateOnListener(metis, listener, remoteAddress);
    } else {
        if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error)) {
            char *str = cpiAddress_ToString(localAddress);
            metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_IO, PARCLogLevel_Error, __func__,
                            "Could not find listener to match address %s", str);
            parcMemory_Deallocate((void **) &str);
        }
    }
    return ops;
}

