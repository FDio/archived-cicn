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
 * @file parc_CertificateType.h
 * @ingroup security
 * @brief A type specifying a certificate.
 *
 */
#ifndef libparc_parc_CertificateType_h
#define libparc_parc_CertificateType_h

typedef enum {
    PARCCertificateType_X509,
    PARCCertificateType_Invalid
} PARCCertificateType;

/**
 * Convert the `PARCCertificateType` value to a human-readable string representation.
 *
 * @param [in] type A `PARCCertificateType` value
 *
 * @return A static, null-terminated string.
 *
 * Example:
 * @code
 * {
 *     PARCCertificateType type = PARCCertificateType_X509;
 *     const char *stringRep = parcCertificateType_ToString(type);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
const char *parcCertificateType_ToString(PARCCertificateType type);

/**
 * Convert a string representation value of a `PARCCertificateType` to an actual value.
 *
 * @param [in] name A string representation of a `PARCCertificateType` value.
 *
 * @return A `PARCCertificateType` value.
 *
 * Example:
 * @code
 * {
 *     const char stringRep[17] = "PARCCertificateType_X509";
 *     PARCCertificateType type = parcCertificateType_FromString(stringRep);
 *     // use stringRep as necessary, and then free
 * }
 * @endcode
 */
PARCCertificateType parcCertificateType_FromString(const char *name);
#endif // libparc_parc_CertificateType_h
