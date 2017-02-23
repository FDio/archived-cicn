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
 * @file metis_TlvSchemaV1.h
 * @brief API to handle the v1 packet format
 *
 * Defines the operations for parsing a V1 schema name
 *
 */

#ifndef Metis_metis_TlvSchemaV1
#define Metis_metis_TlvSchemaV1

#include <ccnx/forwarder/metis/tlv/metis_TlvOps.h>

/**
 * Defines the TLV Operations for the V1 Schema
 *
 * Example:
 * @code
 * {
 *    uint8_t *packet = // read a packet from the network
 *    MetisTlvSkeleton skeleton;
 *    bool success = MetisTlvSchemaV1_Ops.parseSkeleton(packet, &skeleton);
 *    if (success) {
 *       if (MetisTlvSchemaV1_Ops.isPacketTypeInterest(packet)) {
 *       // parse interest
 *    }
 * }
 * @endcode
 */
extern const MetisTlvOps MetisTlvSchemaV1_Ops;


#endif // Metis_metis_TlvSchemaV1

