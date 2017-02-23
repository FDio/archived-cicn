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
 * @file ccnxCodecSchemaV1_CryptoSuite.h
 * @brief Translates between PARC CryptoSuite values and the wire encoding
 *
 * <#Detailed Description#>
 *
 */

#include <stdbool.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/security/parc_SigningAlgorithm.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

/**
 * Converts a PARC Crypto Suite to its TLV value
 *
 * Looks up the PARC cryptosuite value and returns the corresponding TLV wire format value.
 * If no match is found, returns false and outputSuite is not modified.
 *
 *
 * @param [in] parcSuite The PARC cryptosuite
 * @param [out] outputValue The wire encoding equivalent
 *
 * @retval true if supported suite and outputValue set.
 * @retval false if not supported.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1CryptoSuite_ParcToTlv(PARCCryptoSuite parcSuite, CCNxCodecSchemaV1TlvDictionary_CryptoSuite *outputValue);

/**
 * Converts a wire format cryptosuite value to the PARC cryptosuite
 *
 * Looks up the TLV wire format value and returns the corresponding PARC cryptosuite.
 * If no match is found, returns false and outputSuite is not modified.
 *
 * @param [in] tlvValue The wire format value
 * @param [out] outputValue The PARC equivalent
 *
 * @return true if match found and parcSuite set.
 * @return false if no match found.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1CryptoSuite_TlvToParc(CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvValue, PARCCryptoSuite *outputSuite);

/**
 * Lookup a signing algorithm and hash type and convert to a wire format value
 *
 * Based on a PARCSigner's algorithm and hash type, find the corresponding wire format crypto suite.
 *
 * @param [in] signAlgorithm The signing algorithm
 * @param [in] hashType The hash used by the signing algorithm
 * @param [out] outputValue The wire format value
 *
 * @retval true if supported suite and outputValue set.
 * @retval false if not supported.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv(PARCSigningAlgorithm signAlgorithm, PARCCryptoHashType hashType, CCNxCodecSchemaV1TlvDictionary_CryptoSuite *outputValue);
