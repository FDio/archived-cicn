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
 * @file ccnxValidation_EcSecp256K1.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef CCNx_Common_ccnxValidation_EcSecp256K1_h
#define CCNx_Common_ccnxValidation_EcSecp256K1_h

#include <stdbool.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/ccnx_KeyLocator.h>

/**
 *
 * Sets the validation algorithm to be Elliptical Curve with SECP-256K1 parameters.
 * Optionally includes a KeyId and KeyLocator with the message.
 *
 * @param [in] message The message dictionary
 * @param [in] keyid (Optional) The KEYID to include the the message
 * @param [in] keyLocator (Optional) The KEY LOCATOR to include in the message
 *
 * @return true success
 * @return false failure
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxValidationEcSecp256K1_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid, const CCNxKeyLocator *keyLocator);

/**
 * Determines if the validation algorithm is Elliptical Curve with SECP-256K1 parameters.
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
bool ccnxValidationEcSecp256K1_Test(const CCNxTlvDictionary *message);

/**
 * Returns the KeyId associated with the validation algorithm
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] message The message to check
 *
 * @return non-NULL the keyid
 * @return null An error or no keyid or no validation algorithm in the message
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxValidationEcSecp256K1_GetKeyId(const CCNxTlvDictionary *message);

/**
 * Returns the KeyName associated with the validation algorithm
 *
 * This should return a LINK, see case 1018
 *
 * @param [in] message The message to check
 *
 * @return non-NULL the KeyName
 * @return null An error or no keyid or no validation algorithm in the message
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxName   *ccnxValidationEcSecp256K1_GetKeyLocatorName(const CCNxTlvDictionary *message);

/**
 * Returns the PublicKey associated with the validation algorithm
 *
 * @param [in] message The message to check
 *
 * @return non-NULL the PublicKey (DER encoded)
 * @return null An error or no public key or no validation algorithm in the message
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxValidationEcSecp256K1_GetKeyLocatorPublicKey(const CCNxTlvDictionary *message);

/**
 * Returns the Certificate associated with the validation algorithm
 *
 * @param [in] message The message to check
 *
 * @return non-NULL the Certificate (DER encoded)
 * @return null An error or no certificate or no validation algorithm in the message
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBuffer *ccnxValidationEcSecp256K1_GetKeyLocatorCertificate(const CCNxTlvDictionary *message);
#endif // CCNx_Common_ccnxValidation_EcSecp256K1_h
