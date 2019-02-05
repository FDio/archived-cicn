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
 * @file parc_ContainerEncoding.h
 * @ingroup security
 * @brief A Encoding specifying a certificate.
 *
 */
#ifndef libparc_parc_Encoding_h
#define libparc_parc_Encoding_h

typedef enum {
    PARCContainerEncoding_PEM,
    PARCContainerEncoding_DER,
    PARCContainerEncoding_PKCS12,
    PARCContainerEncoding_Invalid
} PARCContainerEncoding;

/**
 * Convert the `PARCContainerEncoding` value to a human-readable string representation.
 *
 * @param [in] Encoding A `PARCContainerEncoding` value
 *
 * @return A static, null-terminated string.
 *
 * Example:
 * @code
 * {
 *     PARCContainerEncoding encoding = PARCContainerEncoding_X509;
 *     const char *stringRep = parcContainerEncoding_ToString(encoding);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
const char *parcContainerEncoding_ToString(PARCContainerEncoding Encoding);

/**
 * Convert a string representation value of a `PARCContainerEncoding` to an actual value.
 *
 * @param [in] name A string representation of a `PARCContainerEncoding` value.
 *
 * @return A `PARCContainerEncoding` value.
 *
 * Example:
 * @code
 * {
 *     const char stringRep[17] = "PARCContainerEncoding_PEM";
 *     PARCContainerEncoding encoding = parcContainerEncoding_FromString(stringRep);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
PARCContainerEncoding parcContainerEncoding_FromString(const char *name);
#endif // libparc_parc_Encoding_h
