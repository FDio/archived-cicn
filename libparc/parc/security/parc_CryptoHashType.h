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
 * @file parc_CryptoHashType.h
 * @ingroup security
 * @brief A type specifying a cryptographic hash (or CRC check) algorithm.
 *
 * This type is overloaded to support both cryptographic hash digest algorithms and cyclical-reduncancy
 * check (CRC) algorithms. See the available `PARCCryptoHashType` enum types for an exhaustive
 * list of the supported algorithms.
 *
 */
#ifndef libparc_parc_CryptoHashType_h
#define libparc_parc_CryptoHashType_h

typedef enum {
    PARCCryptoHashType_SHA256,
    PARCCryptoHashType_SHA512,
    PARCCryptoHashType_CRC32C,
    PARCCryptoHashType_NULL
} PARCCryptoHashType;

/**
 * Convert the `PARCCryptoHashType` value to a human-readable string representation.
 *
 * @param [in] type A `PARCCryptoHashType` value
 *
 * @return A static, null-terminated string.
 *
 * Example:
 * @code
 * {
 *     PARCCryptoHashType type = PARCCryptoHashType_SHA256;
 *     const char *stringRep = parcCryptoHashType_ToString(type);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
const char *parcCryptoHashType_ToString(PARCCryptoHashType type);

/**
 * Convert a string representation value of a `PARCCryptoHashType` to an actual value.
 *
 * @param [in] name A string representation of a `PARCCryptoHashType` value.
 *
 * @return A `PARCCryptoHashType` value.
 *
 * Example:
 * @code
 * {
 *     const char stringRep[17] = "PARCCryptoHashType_SHA256";
 *     PARCCryptoHashType type = parcCryptoHashType_FromString(stringRep);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
PARCCryptoHashType parcCryptoHashType_FromString(const char *name);
#endif // libparc_parc_CryptoHashType_h
