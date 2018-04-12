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

typedef enum {
    PARCCryptoSuite_RSA_SHA256,
    PARCCryptoSuite_DSA_SHA256,
    PARCCryptoSuite_RSA_SHA512,
    PARCCryptoSuite_HMAC_SHA256,
    PARCCryptoSuite_HMAC_SHA512,
    PARCCryptoSuite_NULL_CRC32C,
    PARCCryptoSuite_ECDSA_SHA256,
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

#endif // libparc_parc_CryptoSuite_h
