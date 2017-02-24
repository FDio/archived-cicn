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
 * The TCP out-bound tunnel is almost identical to the in-bound tunnel.
 * We use MetisStreamConneciton for the out-bound tunnels too.  We call a different
 * constructor than the in-bound so the MetisStreamConneciton knows that it is starting
 * unconnected and needs to wait for the Connected event before putting it in the UP state.
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <ccnx/forwarder/metis/io/metis_TcpTunnel.h>
#include <ccnx/forwarder/metis/io/metis_StreamConnection.h>

#include <LongBow/runtime.h>

MetisIoOperations *
metisTcpTunnel_Create(MetisForwarder *metis, const CPIAddress *localAddress, const CPIAddress *remoteAddress)
{
    MetisAddressPair *pair = metisAddressPair_Create(localAddress, remoteAddress);

    bool isLocal = false;

    // this takes ownership of the address pair
    return metisStreamConnection_OpenConnection(metis, pair, isLocal);
}
