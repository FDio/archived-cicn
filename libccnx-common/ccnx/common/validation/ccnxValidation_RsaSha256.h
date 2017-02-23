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
 * @file ccnxValidation_RsaSha256.h
 * @brief <#Brief Description#>
 *
 * The RsaSha256 validation algorithm uses standard locations for KeyId, PublicKey, Certificate, and KeyName,
 * so you should use ccnxValidationFacade getters to retrieve them.
 *
 */
#ifndef CCNx_Common_ccnxValidation_RsaSha256_h
#define CCNx_Common_ccnxValidation_RsaSha256_h

#include <stdbool.h>
#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/ccnx_KeyLocator.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Sets the Validation algorithm to RSA-SHA256
 *
 * Sets the validation algorithm to be RSA with a SHA-256 digest.  Optionally includes
 * a KeyId and KeyLocator with the message.
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
bool ccnxValidationRsaSha256_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid, const CCNxKeyLocator *keyLocator);

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
bool ccnxValidationRsaSha256_Test(const CCNxTlvDictionary *message);
#endif // CCNx_Common_ccnxValidation_RsaSha256_h
