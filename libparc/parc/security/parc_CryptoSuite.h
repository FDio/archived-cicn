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
 * @file parc_CryptoSuite.h
 * @ingroup security
 * @brief Represent a cryptographic suite, a set of corresponding hash and signing/MAC/CRC algorithms.
 *
 * A cryptographic suite encapsulates the method by which (public key) digital signatures and
 * (private key) MACs. For example, a digital signature suite might combine SHA-256 as the hash
 * digest algorithm and RSA as the signature generation/verification algorithm. Such a suite
 * would would have the PARCCryptoSuite value PARCCryptoSuite_RSA_SHA256.
 *
 */
#ifndef libparc_parc_CryptoSuite_h
#define libparc_parc_CryptoSuite_h

#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_SigningAlgorithm.h>

typedef enum {
    PARCCryptoSuite_DSA_SHA256,
    PARCCryptoSuite_DSA_SHA512,
    PARCCryptoSuite_RSA_SHA256,
    PARCCryptoSuite_RSA_SHA512,
    PARCCryptoSuite_HMAC_SHA256,
    PARCCryptoSuite_HMAC_SHA512,
    PARCCryptoSuite_ECDSA_SHA256,
    PARCCryptoSuite_ECDSA_SHA512,
    PARCCryptoSuite_NULL_CRC32C,
    PARCCryptoSuite_UNKNOWN
} PARCCryptoSuite;

/**
 * Given a PARCCryptoSuite value, return the corresponding cryptographic hash as a `PARCCryptoHashType`.
 *
 * @param [in] suite A PARCCryptoSuite value.
 *
 * @return A PARCCryptoHashType value
 *
 * Example:
 * @code
 * {
 *     PARCryptoHashType hash = parcCryptoSuite_GetCryptoHash(PARCCryptoSuite_RSA_SHA256);
 * }
 * @endcode
 */
PARCCryptoHashType parcCryptoSuite_GetCryptoHash(PARCCryptoSuite suite);

/**
 * Given a PARCCryptoSuite value and the key length, return the expected length in bits of the signature.
 * For ECDSA the result is the maximum length
 *
 * @param [in] suite A PARCCryptoSuite value.
 *
 * @return A PARCCryptoHashType value
 *
 * Example:
 * @code
 * {
 *     int bits = parcCryptoSuite_GetSignatureSizeBits(PARCCryptoSuite_RSA_SHA256, 1024);
 * }
 * @endcode
 */
int parcCryptoSuite_GetSignatureSizeBits(PARCCryptoSuite suite, int keyLengthBits);

/**
 * Given a PARCCryptoSuite value and the key length, return the expected length in bytes of the signature.
 * For ECDSA the result is the maximum length
 *
 * @param [in] suite A PARCCryptoSuite value.
 *
 * @return A PARCCryptoHashType value
 *
 * Example:
 * @code
 * {
 *     int bytes = parcCryptoSuite_GetSignatureSizeBits(PARCCryptoSuite_RSA_SHA256, 1024);
 * }
 * @endcode
 */
int parcCryptoSuite_GetSignatureSizeBytes(PARCCryptoSuite suite, int keyLengthBits);

/**
 * Given a PARCSigningAlgorithm value and a PARCCryptoHashType value, return the corresponding `PARCCryptoSuite`.
 *
 * @param [in] suite A PARCSigningAlgorithm value and a PARCCryptoHashType value
 *
 * @return A PARCCryptoSuite value
 *
 * Example:
 * @code
 * {
 *     PARCryptoSuite suite = parcCryptoSuite_GetFromSigningHash(PARCSigningAlgorihtm_RSA, PARCCryptoHashType_SHA256);
 * }
 * @endcode
 */
PARCCryptoSuite parcCryptoSuite_GetFromSigningHash(PARCSigningAlgorithm signAlgo, PARCCryptoHashType hash);

/**
 * Get the `PARCSigningAlgorithm` type associated with the specified `PARCCryptoSuite` type.
 *
 * PARCCryptoSuite types combine hash and signing algorithms to be used to signature and/or MAC generation.
 * Therefore, a PARCCryptoSuite type of PARCCryptoSuite_DSA_SHA256, for example, uses the
 * PARCSigningAlgorithm_DSA type of signing algorithm. This function serves to determine the
 * signing algorithm type from the suite.
 *
 * @param [in] suite The type of cryptographic suite used for signature and/or MAC generation.
 * @return A valid `PARCSigningAlgorithm` enum associated with the specified `PARCCryptoSuite` type.
 *
 * Example:
 * @code
 * {
 *     PARCCryptoSuite suite = PARCCryptoSuite_RSA_SHA256;
 *     PARCSigningAlgorithm alg = parcSigningAlgorithm_GetSigningAlgorithm(suite);
 *     // do something with alg
 * }
 * @endcode
 */
PARCSigningAlgorithm parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite suite);


#endif // libparc_parc_CryptoSuite_h
