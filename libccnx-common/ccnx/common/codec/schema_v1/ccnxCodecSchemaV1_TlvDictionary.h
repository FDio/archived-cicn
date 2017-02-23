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
 * @file ccnxCodecSchemaV1_TlvDictionary.h
 * @brief <#Brief Description#>
 *
 * Used as keys to the CCNxTlvDictionary for the version 1 schema
 *
 */

#ifndef libccnx_ccnx_TlvDictionary_SchemaV1_h
#define libccnx_ccnx_TlvDictionary_SchemaV1_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * @typedef CCNxCodecSchemaV1TlvDictionary_CryptoSuite
 * @abstract The ValidationAlgorithm Type.
 * @constant <#name#> <#description#>
 * @discussion These are the wire-format values for the ValidationAlgorithm type.  The values
 *      specified follow from the CCNx Messages RFC.
 *
 *      It is not the same as the value stored in CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE,
 *      which is of type PARCCryptoSuite.
 */
typedef enum rta_tlv_schema_v1_crypto_suite {
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C = 2,
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256 = 4,
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256 = 6,
    CCNxCodecSchemaV1TlvDictionary_CryptoSuite_EcSecp256K1 = 7,
} CCNxCodecSchemaV1TlvDictionary_CryptoSuite;

/**
 * @typedef <#CCNBHeaderType#>
 * @abstract <#Abstract#>
 * @constant CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_WireFormat The off-the-wire packet or a pre-encoded packet
 * @discussion
 *   The WireFormat header is a ficticious header for holding the off-the-wire packet received
 *   from the network or to send a pre-encoded packet down through the stack.
 *
 *   The Forwarder header is a ficticious header for holding special forwarder control block.  The
 *   forwarder control block, on ingress, contains information about where a packet arrived.  On
 *   egress, it contains information about how the packet should be transmitted, such as restricting
 *   it to a specific egress interface.
 *
 *   The protected region extent is used to determine they byte range used for verification.
 */


typedef enum rta_tlv_schema_v1_headers_fastarray {
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_TransportStack = 0,        /**< Array element 0 is used by RTA Transport stack */
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_FixedHeader = 1,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_INTFRAG = 2,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_OBJFRAG = 3,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_WireFormat = 4,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_Forwarder = 5,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestLifetime = 6,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_RecommendedCacheTime = 7,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedStart = 8,        /**< Fictitious header for Protected Region Extent */
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ProtectedLength = 9,       /**< Fictitious header for Protected Region length */
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ContentObjectHashRegionStart = 10,  /**< Fictitious header for CO Hash Region Extent */
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_ContentObjectHashRegionLength = 11, /**< Fictitious header for CO Hash Region length */
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode = 12,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel = 13,
    CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END = 14
} CCNxCodecSchemaV1TlvDictionary_HeadersFastArray;

/**
 * The ValidationFastArray are fields that may appear in the Validation Algorithm and the Validation Payload field.
 *
 * Note that the ValidationFastArray_CRYPTO_SUITE is always expressed in terms of PARCCryptoSuite.
 */
typedef enum rta_tlv_schema_v1_validation_fastarray {
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 0,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 1,   // PARCCryptoSuite value
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEY = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 2,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CERT = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 3,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_NAME = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 4,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_KEYID = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 5,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYNAME_OBJHASH = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 6,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_PAYLOAD = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 7,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_SIGNTIME = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 8,
    CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END = CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_END + 9
} CCNxCodecSchemaV1TlvDictionary_ValidationFastArray;


/**
 * The MessageFastArray are fields that may appear in the body of a CCNx message (Interest, Content Object, Control).
 *
 * The Hop Limit is part of the MessageFastArray even though it appears in the FixedHeader.  It is treated like a property
 * of the Interest.
 */
typedef enum rta_tlv_schema_v1_message_fastarray {
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 0,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_KEYID_RESTRICTION = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 1,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 2,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOAD = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 4,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HOPLIMIT = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 5,            /***< Virtual field */
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_PAYLOADTYPE = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 6,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_EXPIRY_TIME = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 7,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_ENDSEGMENT = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 8,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_HASH_GROUP = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 9,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_DATA_POINTER = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 10,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_MANIFEST_POINTER = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 11,
    CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END = CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_END + 12
} CCNxCodecSchemaV1TlvDictionary_MessageFastArray;

//const int CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_PathLabel = CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END + 1;

/**
 * Each TLV container can have custom types in it, so each container has a "list"
 * Organization Extensions go here.
 */
typedef enum rta_tlv_schema_v1_lists {
    CCNxCodecSchemaV1TlvDictionary_Lists_HEADERS = 0,
    CCNxCodecSchemaV1TlvDictionary_Lists_MESSAGE_LIST = 1,
    CCNxCodecSchemaV1TlvDictionary_Lists_VALIDATION_ALG_LIST = 4,
    CCNxCodecSchemaV1TlvDictionary_Lists_VALIDATION_PAYLOAD_LIST = 5,
    CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST = 6,
    CCNxCodecSchemaV1TlvDictionary_Lists_END = 7
} CCNxCodecSchemaV1TlvDictionary_Lists;

/**
 * Creates an empty Interest dictionary
 *
 * The dictionary schema will be V1 and the dictionary type will be Interest.  No other
 * fields are pre-populated.
 *
 * @retval non-null An allocated Dictionary of type Interest
 * @retval null An error (likely no memory)
 *
 * Example:
 * @code
 * {
 *     // in a decoder
 *     if (messageType == _DecoderTlvType_Interest) {
 *         CCNxTlvDictionary *interest = ccnxCodecSchemaV1TlvDictionary_CreateInterest();
 *         // decode the rest of the packet
 *         return interest;
 *     }
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecSchemaV1TlvDictionary_CreateInterest(void);

/**
 * Creates an empty Content Object dictionary
 *
 * The dictionary schema will be V1 and the dictionary type will be Content Object.  No other
 * fields are pre-populated.
 *
 * @retval non-null An allocated Dictionary of type Content Object
 * @retval null An error (likely no memory)
 *
 * Example:
 * @code
 * {
 *     // in a decoder
 *     if (messageType == _DecoderTlvType_ContentObject) {
 *         CCNxTlvDictionary *object = ccnxCodecSchemaV1TlvDictionary_CreateContentObject();
 *         // decode the rest of the packet
 *         return object;
 *     }
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecSchemaV1TlvDictionary_CreateContentObject(void);

/**
 * Creates an empty Manifest dictionary
 *
 * The dictionary schema will be V1 and the dictionary type will be Content Object. The
 * PayloadType will be set to CCNxPayloadType_MANIFEST. No other fields are pre-populated.
 *
 * @retval non-null An allocated Dictionary of type Manifest
 * @retval null An error (likely no memory)
 *
 * Example:
 * @code
 * {
 *     // in a decoder
 *     if (messageType == _DecoderTlvType_Manifest) {
 *         CCNxTlvDictionary *object = ccnxCodecSchemaV1TlvDictionary_Manifest();
 *         // decode the rest of the packet
 *         return object;
 *     }
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecSchemaV1TlvDictionary_CreateManifest(void);

/**
 * Creates an empty Control dictionary
 *
 * The dictionary schema will be V1 and the dictionary type will be Control.  No other
 * fields are pre-populated.
 *
 * @retval non-null An allocated Dictionary of type Control
 * @retval null An error (likely no memory)
 *
 * Example:
 * @code
 * {
 *     // in a decoder
 *     if (messageType == _DecoderTlvType_Control) {
 *         CCNxTlvDictionary *control = ccnxCodecSchemaV1TlvDictionary_CreateControl();
 *         // decode the rest of the packet
 *         return control;
 *     }
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecSchemaV1TlvDictionary_CreateControl(void);

/**
 * Creates an empty InterestReturn dictionary
 *
 * The dictionary schema will be V1 and the dictionary type will be InterestReturn.  No other
 * fields are pre-populated.
 *
 * @retval non-null An allocated Dictionary of type InterestReturn
 * @retval null An error (likely no memory)
 *
 * Example:
 * @code
 * {
 *     // in a decoder
 *     if (messageType == _DecoderTlvType_InterestReturn) {
 *         CCNxTlvDictionary *interestReturn = ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn();
 *         // decode the rest of the packet
 *         return interestReturn;
 *     }
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn(void);
#endif // libccnx_ccnx_TlvDictionary_SchemaV1_h
