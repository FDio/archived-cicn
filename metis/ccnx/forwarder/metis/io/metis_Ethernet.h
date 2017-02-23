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
 * @file metis_Ethernet.h
 * @brief Helpers for Ethernet frames
 *
 */

#ifndef Metis_metis_Ethernet_h
#define Metis_metis_Ethernet_h

/**
 * returns true is the ethertype is at least 0x0600 indicating
 * a type II frame (IEEE 802.3x-1997)
 *
 * @param [in] ethertype The ethertype in host byte order
 *
 * @retval true if the ethertype is at least 0x0600
 * @retval false Otherwise
 */
#define metisEthernet_IsValidEthertype(ethertype) ((ethertype) >= 0x0600)

#endif
