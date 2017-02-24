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
 * @file ccnxValidation_CRC32C.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef CCNx_Common_ccnxValidation_CRC32C_h
#define CCNx_Common_ccnxValidation_CRC32C_h

#include <parc/security/parc_Signer.h>
#include <parc/security/parc_Verifier.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Sets the Validation algorithm to RSA-SHA256
 *
 * Sets the validation algorithm to be RSA with a SHA-256 digest.  Optionally includes
 * a KeyId and KeyLocator with the message.
 *
 * @param [in] message The message dictionary
 *
 * @return `true` success
 * @return `false` failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationCRC32C_Set(CCNxTlvDictionary *message);

/**
 * Determines if the validation algorithm is RSA-SHA256 *
 * @param [in] message The message to check
 *
 * @return `true` The validation algorithm in the dictionary is this one
 * @return `false` The validaiton algorithm in the dictionary is something else or not present
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationCRC32C_Test(const CCNxTlvDictionary *message);

/**
 * Creates a signer to compute a CRC32C
 *
 * @return non-null An allocated signer
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSigner *ccnxValidationCRC32C_CreateSigner(void);

/**
 * Creates a verifier to check a CRC32C "signature"
 *
 * @return non-null An allocated verifier
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCVerifier *ccnxValidationCRC32C_CreateVerifier(void);
#endif // CCNx_Common_ccnxValidation_CRC32C_h
