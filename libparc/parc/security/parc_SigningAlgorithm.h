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
 * @file parc_SigningAlgorithm.h
 * @ingroup security
 * @brief This module encapsulates information about the types of available signing algorithms.
 *
 * Both asymmetric digital signature algorithms, e.g., RSA and DSA, and symmetric Message Authentication
 * Codes (MACS), e.g., HMAC, are supported. This module exposes the functionality necessary to map between
 * enum and human-readable string representations of these algorithms.
 *
 */
#ifndef libparc_parc_SigningAlgorithm_h
#define libparc_parc_SigningAlgorithm_h

typedef enum {
    PARCSigningAlgorithm_UNKNOWN = -1,
    PARCSigningAlgorithm_RSA = 1,
    PARCSigningAlgorithm_DSA = 2,
    PARCSigningAlgorithm_HMAC = 3,
    PARCSigningAlgorithm_ECDSA = 4,
    PARCSigningAlgorithm_NULL = 5,
} PARCSigningAlgorithm;

/**
 * Return a human readable string representation of the specified signing algorithm.
 *
 * @param [in] algorithm A pointer to a PARCSigningAlgorithm as the target signing algorithm.
 * @return A static, null-terminated string.
 *
 * Example:
 * @code
 * {
 *     PARCSigningAlgorithm alg = PARCSigningAlgorithm_RSA;
 *     const char *stringRep = parcSigningAlgorithm_ToString(alg);
 *     // do something with stringRep
 * }
 * @endcode
 *
 * @see parcSigningAlgorithm_FromString
 */
const char *parcSigningAlgorithm_ToString(PARCSigningAlgorithm algorithm);

/**
 * Get the `PARCSigningAlgorithm` enum from a corresponding human-readable string representation
 * of a signing algorithm.
 *
 * @param [in] name A nul-terminate C-string representation of signing algorithm.
 * @return A valid `PARCSigningAlgorithm` enum.
 *
 * Example:
 * @code
 * {
 *     const char *stringRep = "PARCSigningAlgortihm_NULL";
 *     PARCSigningAlgorithm alg = parcSigningAlgorithm_FromString(stringRep);
 *     // do something with alg
 * }
 * @endcode
 *
 * @see parcSigningAlgorithm_ToString
 */
PARCSigningAlgorithm parcSigningAlgorithm_FromString(const char *name);

#endif // libparc_parc_SigningAlgorithm_h
