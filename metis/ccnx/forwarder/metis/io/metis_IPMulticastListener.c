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
#include <arpa/inet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/io/metis_IPMulticastListener.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/tlv/metis_Tlv.h>
#include <ccnx/forwarder/metis/core/metis_Message.h>

MetisListenerOps *metisIPMulticastListener_CreateInet6(MetisForwarder *metis, struct sockaddr_in6 sin6);
MetisListenerOps *metisIPMulticastListener_CreateInet(MetisForwarder *metis, struct sockaddr_in sin);
