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
 * @file parc_DiffieHellmanGroup.h
 * @ingroup security
 * @brief An enumeration of the supported Diffie Hellman key exchange mechanisms.
 *
 */
#ifndef libparc_parc_DiffieHellmanGroup_h
#define libparc_parc_DiffieHellmanGroup_h

typedef enum {
    PARCDiffieHellmanGroup_Prime256v1, // NIST Prime-Curve P-256
    PARCDiffieHellmanGroup_Secp521r1,  // NIST Prime-Curve P-521
    PARCDiffieHellmanGroup_Curve2559   // Curve2559
} PARCDiffieHellmanGroup;

#endif // libparc_parc_DiffieHellmanGroup_h
