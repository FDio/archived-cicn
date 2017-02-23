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
 * @file ccnxValidation_HmacSha256.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef CCNx_Common_ccnxValidation_HmacSha256_h
#define CCNx_Common_ccnxValidation_HmacSha256_h

#include <parc/security/parc_Signer.h>
#include <parc/security/parc_Verifier.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Sets the Validation algorithm to HMAC-SHA256
 *
 * Sets the validation algorithm to be HMAC with a SHA-256 digest.  Optionally includes a KeyId
 *
 * @param [in] message The message dictionary
 * @param [in] keyid (Optional) The KEYID to include the the message
 *
 * @return true success
 * @return false failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationHmacSha256_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid);

/**
 * Determines if the validation algorithm is RSA-SHA256
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message The message to check
 *
 * @return true The validation algorithm in the dictionary is this one
 * @return false The validaiton algorithm in the dictionary is something else or not present
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationHmacSha256_Test(const CCNxTlvDictionary *message);

/**
 * Creates a signer using a specified secret key
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] secretKey The key to use as the authenticator
 *
 * @return non-null An allocated signer
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSigner *ccnxValidationHmacSha256_CreateSigner(PARCBuffer *secretKey);

/**
 * Creates a verifier to check a CRC32C "signature"
 *
 * Once the Verifier is created, you can add more keys using
 * parcVerifier_AddKey().  If you provide a secretKey in the call, it will
 * be added to the verifier automatically.
 *
 * @param [in] secretKey (Optional) The key to use as the authenticator, or NULL.
 *
 * @return non-null An allocated verifier
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCVerifier *ccnxValidationHmacSha256_CreateVerifier(PARCBuffer *secretKey);
#endif // CCNx_Common_ccnxValidation_HmacSha256_h
