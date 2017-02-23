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
 * @file transport_private.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_transport_private_h
#define Libccnx_transport_private_h

#include <ccnx/transport/common/transport_MetaMessage.h>
#include <ccnx/transport/common/ccnx_TransportConfig.h>

struct transport_operations {
    void* (*Create)(void);
    int (*Open)(void *ctx, CCNxTransportConfig *transportConfig);
    int (*Send)(void *ctx, int desc, CCNxMetaMessage *msg, const struct timeval *timeout);
    TransportIOStatus (*Recv)(void *ctx, int desc, CCNxMetaMessage **msg, const struct timeval *timeout);
    int (*Close)(void *ctx, int desc);
    int (*Destroy)(void **ctx);
    int (*PassCommand)(void *ctx, void *command);
};
#endif // Libccnx_transport_private_h
