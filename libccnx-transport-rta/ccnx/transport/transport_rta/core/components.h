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

//
//  components.h
//  Libccnx
//


#ifndef Libccnx_components_h
#define Libccnx_components_h

// Every component in the system must be defined here
// These must correspond to array indicies.
typedef enum {
    API_CONNECTOR = 0,
    FC_NONE = 1,
    FC_VEGAS = 2,
    FC_PIPELINE = 3,
    // vacant          = 4,
    // vacant          = 5,
    // vacant          = 6,
    CODEC_NONE = 7,
    CODEC_UNSPEC = 8,
    CODEC_TLV = 9,
    // vacant          = 10,
    // vacant          = 11,
    FWD_NONE = 12,
    FWD_LOCAL = 13,
    // vacant          = 14,
    // vacant          = 15,
    TESTING_UPPER = 16,
    TESTING_LOWER = 17,
    FWD_METIS = 19,
    LAST_COMPONENT = 20,      // MUST ALWAYS BE LAST
    UNKNOWN_COMPONENT         // MUST BE VERY LAST
} RtaComponents;


// This is defied in rta_ProtocolStack.c and should be kept
// in sync with RtaComponents
extern const char *RtaComponentNames[LAST_COMPONENT];

#endif
