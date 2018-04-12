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
 * @file parc_KeyType.h
 * @ingroup security
 * @brief A type specifying a key.
 *
 */
#ifndef libparc_parc_KeyType_h
#define libparc_parc_KeyType_h

typedef enum {
    PARCKeyType_RSA,
    PARCKeyType_EC,
    PARCKeyType_Invalid
} PARCKeyType;

/**
 * Convert the `PARCKeyType` value to a human-readable string representation.
 *
 * @param [in] type A `PARCKeyType` value
 *
 * @return A static, null-terminated string.
 *
 * Example:
 * @code
 * {
 *     PARCKeyType type = PARCKeyType_RSA;
 *     const char *stringRep = parcKeyType_ToString(type);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
const char *parcKeyType_ToString(PARCKeyType type);

/**
 * Convert a string representation value of a `PARCKeyType` to an actual value.
 *
 * @param [in] name A string representation of a `PARCKeyType` value.
 *
 * @return A `PARCKeyType` value.
 *
 * Example:
 * @code
 * {
 *     const char stringRep[17] = "PARCKeyType_RSA";
 *     PARCKeyType type = parcKeyType_FromString(stringRep);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
PARCKeyType parcKeyType_FromString(const char *name);
#endif // libparc_parc_KeyType_h
