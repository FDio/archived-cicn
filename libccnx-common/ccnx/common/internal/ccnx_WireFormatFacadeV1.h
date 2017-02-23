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
 * @file ccnx_WireFormatFacadeV1.h
 * @ingroup Utility
 *
 * A WireFormat facade will set/get the wire format representation of a message from the
 * dictionary.
 *
 * One may also create a message dictionary only with a wire format, not specifying the actual message type.
 * This occurs mostly at the lowest layer that receives a network buffer and does not yet know what sort of message it holds.
 *
 * This facade is used by the Forwarder Connector to create the original dictionary
 * at the bottom of the stack on receive.  It is also used by the Codec component to set
 * the wireformat to encode a packet.
 *
 * If an application has a pre-encoded packet, it can create an empty dictionary and set the wire format
 * then send that down the stack.
 *
 */
#ifndef libccnx_ccnx_WireFormatFacadeV1_h
#define libccnx_ccnx_WireFormatFacadeV1_h


#include <ccnx/common/internal/ccnx_WireFormatMessageInterface.h>

extern CCNxWireFormatMessageInterface CCNxWireFormatFacadeV1_Implementation;
#endif // libccnx_ccnx_WireFormatFacadeV1_h
