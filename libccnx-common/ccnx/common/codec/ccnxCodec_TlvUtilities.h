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
 * @file ccnxCodec_TlvUtilities.h
 * @brief Utility functions common to all the codecs
 *
 * <#Detailed Description#>
 *
 */
#ifndef TransportRTA_ccnxCodec_TlvUtilities_h
#define TransportRTA_ccnxCodec_TlvUtilities_h

#include <stdbool.h>
#include <stdint.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>
#include <ccnx/common/codec/ccnxCodec_TlvDecoder.h>

/**
 * Decodes a list of TLV entries
 *
 * The decoder should point to the first byte of a "type".  This function will iterate over all the TLVs
 * and call the user function 'typeDecoder' for each type-length.
 *
 * It is the responsibility of typeDecoder to advance the decoder by 'length' bytes.  It should return false
 * if it does not consume exactly 'length' bytes.
 *
 * The function will proceed until it can no longer parse a TLV header (4 bytes).  If the function consumes all
 * the bytes in the decoder without error, it will return true.  If it encounters an error from 'typeDecoder' it
 * will return false at that point.  If there is an underflow (i.e. 1, 2, or 3 bytes) left in the decoder at the end
 * it will return false.
 *
 * @param [in] decoder The TLV decoder that should point to the start of the TLV list
 * @param [in] packetDictionary The dictionary to use to store packet fields
 * @param [in] typeDecoder the user-supplied function to call for each TLV found in the container
 *
 * @return true There were no errors returned by 'typeDecoder' and we consumed the entire decoder buffer
 * @return false There was an error or we did not consume the entire decoder buffer.
 *
 * Example:
 * @code
 * {
 *    static bool
 *    testTypeDecoder(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
 *    {
 *    switch (type) {
 *       case 0x000C: // fallthrough
 *       case 0x000D:
 *          ccnxCodecTlvDecoder_Advance(decoder, length);
 *          return true;
 *       default:
 *          return false;
 *       }
 *    }
 *
 *    void foo(void)
 *    {
 *       // A list of 2 TLV containers (types 0x000C and 0x000D)
 *       uint8_t metadataContainer[] = {
 *          0x00, 0x0C, 0x00, 0x01,     // Object Type, length = 1
 *          0x04,                       // LINK
 *          0x00, 0x0D, 0x00,    8,     // Creation Time
 *          0x00, 0x00, 0x01, 0x43,     // 1,388,534,400,000 msec
 *          0x4B, 0x19, 0x84, 0x00,
 *       };
 *
 *       PARCBuffer *buffer = parcBuffer_Wrap(metadataContainer, sizeof(metadataContainer), 0, sizeof(metadataContainer) );
 *
 *       // now decode that snippit
 *       CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *       CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(10,10);
 *
 *       bool success = ccnxCodecTlvUtilities_DecodeContainer(decoder, dictionary, testTypeDecoder);
 *
 *       ccnxTlvDictionary_Release(&dictionary);
 *       ccnxCodecTlvDecoder_Destroy(&decoder);
 *       parcBuffer_Release(&buffer);
 *
 *       assertTrue(success, "The TLV types were known to us");
 *    }
 * @endcode
 */
bool ccnxCodecTlvUtilities_DecodeContainer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, bool (*typeDecoder)(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length));

/**
 * Creates an inner decoder of for decoding a subcontainer
 *
 * The decoder should point at the first byte of the "value", which is known to be a subcontainer listing other
 * TLVs.  This function will create an inner decoder and then call 'ccnxCodecTlvUtilities_DecodeContainer' with it to
 * decode the inner TLVs.
 *
 * @param [in] decoder The decoder that points to the fist byte of a list of TLVs.
 * @param [in] packetDictionary Where to put the results
 * @param [in] key NOT USED
 * @param [in] length The length of the subcontainer.  The inner decoder will end after this may bytes.
 * @param [in] subcontainerDecoder The function to pass to 'ccnxCodecTlvUtilities_DecodeContainer' for the inner decoder
 *
 * @return true There were no errors and consumed 'length' bytes
 * @return false An error or did not consume 'length' bytes
 *
 * Example:
 * @code
 * {
 *   // The KeyLocator field is known to be a subcontainer containing its own TLV fields.  When we encounter that
 *   // TLV type, we parse the 'value' of it as a subcontainer
 *   //
 *   static bool
 *   rtaTlvSchemaV0NameAuth_DecodeType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
 *   {
 *      bool success = false;
 *      switch (type) {
 *      case CCNxCodecSchemaV0_NameAuthKeys_KeyLocator:
 *         success = ccnxCodecTlvUtilities_DecodeSubcontainer(decoder, packetDictionary, type, length, rtaTlvSchemaV0KeyLocator_Decode);
 *         break;
 *
 *      case CCNxCodecSchemaV0_NameAuthKeys_CryptoSuite:
 *         success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_CRYPTO_SUITE);
 *         break;
 *
 *      case CCNxCodecSchemaV0_NameAuthKeys_KeyId:
 *         success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_KEYID);
 *         break;
 *
 *      default:
 *         // if we do not know the TLV type, put it in this container's unknown list
 *         success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectLists_NAMEAUTH_LIST);
 *         break;
 *      }
 *      return success;
 * }
 * @endcode
 */
bool
ccnxCodecTlvUtilities_DecodeSubcontainer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t key, uint16_t length,
                                         bool (*subcontainerDecoder)(CCNxCodecTlvDecoder *, CCNxTlvDictionary *));

/**
 * Decodes 'length' bytes from the decoder and puts it in the dictionary
 *
 * Reads the next 'length' bytes from the decoder and wraps it in a PARCBuffer.  The buffer is saved in the packetDictionary
 * under the key 'arrayKey'.
 *
 * It is an error if there are not 'length' bytes remaining in the decoder.
 *
 * @param [in] decoder The input to read
 * @param [in] packetDictionary The output dictionary to save the buffer in
 * @param [in] key The TLV key of the value being read (NOT USED)
 * @param [in] length The byte length to read
 * @param [in] dictionaryKey The key to use in the packetDictionary
 *
 * @return true 'length' bytes were read and saved in the packetDictionary
 * @return false An error
 *
 * Example:
 * @code
 *    {
 *       // A list of 2 TLV containers (types 0x000C and 0x000D)
 *       uint8_t metadataContainer[] = {
 *          0x00, 0x0C, 0x00, 0x01,     // Object Type, length = 1
 *          0x04,                       // LINK
 *          0x00, 0x0D, 0x00,    8,     // Creation Time
 *          0x00, 0x00, 0x01, 0x43,     // 1,388,534,400,000 msec
 *          0x4B, 0x19, 0x84, 0x00,
 *       };
 *
 *       PARCBuffer *buffer = parcBuffer_Wrap(metadataContainer, sizeof(metadataContainer), 0, sizeof(metadataContainer) );
 *
 *       // now decode that snippit
 *       CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *       CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(10,10);
 *
 *       uint16_t tlvtype = ccnxCodecTlvDecoder_GetType(decoder);
 *       uint16_t tlvlength = ccnxCodecTlvDecoder_GetLength(decoder);
 *
 *       // The buffer will contain the one byte 0x04.
 *       bool success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, dictionary, tlvtype, tlvlength, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_OBJ_TYPE);
 *
 *       ccnxTlvDictionary_Release(&dictionary);
 *       ccnxCodecTlvDecoder_Destroy(&decoder);
 *       parcBuffer_Release(&buffer);
 *
 *       assertTrue(success, "There was an unknown TLV at position %zu", ccnxCodecTlvDecoder_Position(decoder));
 *    }
 * @endcode
 */
bool
ccnxCodecTlvUtilities_PutAsBuffer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int dictionaryKey);

/**
 * Decodes a `PARCCryptoHash` value of 'length' bytes from the decoder and puts it in the dictionary.
 *
 * It is an error if there are not 'length' bytes remaining in the decoder.
 *
 * @param [in] decoder The input to read
 * @param [in] packetDictionary The output dictionary to save the buffer in
 * @param [in] key The TLV key of the value being read (NOT USED)
 * @param [in] length The byte length to read
 * @param [in] dictionaryKey The key to use in the packetDictionary
 *
 * @return true 'length' bytes were read and saved in the packetDictionary
 * @return false An error
 *
 * Example:
 * @code
 *    {
 *       // A list of 2 TLV containers (types 0x000C and 0x000D)
 *       uint8_t hashContainer[] = {
 *          0x00, 0x01, 0x00, 0x20,     // SHA256 hash, length = 0x20
 *          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *       };
 *
 *       PARCBuffer *buffer = parcBuffer_Wrap(hashContainer, sizeof(hashContainer), 0, sizeof(hashContainer) );
 *
 *       // now decode that snippit
 *       CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *       CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(10,10);
 *
 *       uint16_t tlvtype = ccnxCodecTlvDecoder_GetType(decoder);
 *       uint16_t tlvlength = ccnxCodecTlvDecoder_GetLength(decoder);
 *
 *       bool success = ccnxCodecTlvUtilities_PutAsHash(decoder, dictionary, tlvtype, tlvlength, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_OBJHASH_RESTRICTION);
 *
 *       ccnxTlvDictionary_Release(&dictionary);
 *       ccnxCodecTlvDecoder_Destroy(&decoder);
 *       parcBuffer_Release(&buffer);
 *    }
 * @endcode
 */
bool
ccnxCodecTlvUtilities_PutAsHash(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int dictionaryKey);

/**
 * Decodes the value as a VarInt and saves it as an Integer in the Dictionary
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool
ccnxCodecTlvUtilities_PutAsInteger(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int dictionaryKey);


/**
 * Decodes 'length' bytes from the decoder and puts it in the dictionary as a CCNxName
 *
 * Reads the next 'length' bytes from the decoder and wraps it in a CCNxName.  The name is saved in the packetDictionary
 * under the key 'arrayKey'.
 *
 * It is an error if there are not 'length' bytes remaining in the decoder.
 *
 * @param [in] decoder The input to read
 * @param [in] packetDictionary The output dictionary to save the name in
 * @param [in] key The TLV key of the value being read (NOT USED)
 * @param [in] length The byte length to read
 * @param [in] dictionaryKey The key to use in the packetDictionary
 *
 * @return true 'length' bytes were read and saved in the packetDictionary
 * @return false An error
 *
 * Example:
 * @code
 *    {
 *       // A list of 2 TLV containers (types 0x000C and 0x000D)
 *       uint8_t metadataContainer[] = {
 *          0x00, 0x00, 0x00,    9,     // type = name, length = 9
 *          0x00, 0x02, 0x00,    5,     // type = binary, length = 5
 *          'h',  'e',  'l',  'l',      // "hello"
 *          'o',
 *       };
 *
 *       PARCBuffer *buffer = parcBuffer_Wrap(metadataContainer, sizeof(metadataContainer), 0, sizeof(metadataContainer) );
 *
 *       // now decode that snippit
 *       CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *       CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(10,10);
 *
 *       uint16_t tlvtype = ccnxCodecTlvDecoder_GetType(decoder);
 *       uint16_t tlvlength = ccnxCodecTlvDecoder_GetLength(decoder);
 *
 *       // Saves "lci:/hello"
 *       bool success = ccnxCodecTlvUtilities_PutAsName(decoder, dictionary, tlvtype, tlvlength, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_NAME);
 *
 *       ccnxTlvDictionary_Release(&dictionary);
 *       ccnxCodecTlvDecoder_Destroy(&decoder);
 *       parcBuffer_Release(&buffer);
 *
 *       assertTrue(success, "The Name failed to decode or some other error");
 *    }
 * @endcode
 */
bool
ccnxCodecTlvUtilities_PutAsName(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int arrayKey);

/**
 * Reads 'length' bytes from the decoder and appends a PARCBuffer to a list in packetDictionary
 *
 * Saves a buffer as part of a List in the packet dictionary.  This is primarily used for unknown TLV types that
 * do not have a specific decoder.
 *
 * @param [in] decoder The decoder to read
 * @param [in] packetDictionary The dictionary to append the buffer in
 * @param [in] type The TLV type of the buffer (saved as part of the list entry)
 * @param [in] length The length to wrap in the buffer
 * @param [in] listKey The list key in packetDictionary
 *
 * @return true Success
 * @return false Failure or error
 *
 * Example:
 * @code
 * {
 *   // If we exhaust all the known keys in the Name Authenticator, the default case will save the TLV in
 *   // the container's list in the packet dictionary.
 *   //
 *   static bool
 *   rtaTlvSchemaV0NameAuth_DecodeType(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length)
 *   {
 *      bool success = false;
 *      switch (type) {
 *      case CCNxCodecSchemaV0_NameAuthKeys_KeyLocator:
 *         success = ccnxCodecTlvUtilities_DecodeSubcontainer(decoder, packetDictionary, type, length, rtaTlvSchemaV0KeyLocator_Decode);
 *         break;
 *
 *      case CCNxCodecSchemaV0_NameAuthKeys_CryptoSuite:
 *         success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_CRYPTO_SUITE);
 *         break;
 *
 *      case CCNxCodecSchemaV0_NameAuthKeys_KeyId:
 *         success = ccnxCodecTlvUtilities_PutAsBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_KEYID);
 *         break;
 *
 *      default:
 *         // if we do not know the TLV type, put it in this container's unknown list
 *         success = ccnxCodecTlvUtilities_PutAsListBuffer(decoder, packetDictionary, type, length, CCNxCodecSchemaV0TlvDictionary_ContentObjectLists_NAMEAUTH_LIST);
 *         break;
 *      }
 *      return success;
 * }
 * @endcode
 */
bool
ccnxCodecTlvUtilities_PutAsListBuffer(CCNxCodecTlvDecoder *decoder, CCNxTlvDictionary *packetDictionary, uint16_t type, uint16_t length, int listKey);

/**
 * Encodes a nested TLV container (the opposite of ccnxCodecTlvUtilities_DecodeSubcontainer)
 *
 * Appends a TLV header (4 bytes) to the encoder using 'nestedType' as the TLV type.  It then calls 'nestedEncoderFunction' to
 * encode the 'value' of the container.  If 'nestedEncoderFunction' returns positive bytes it will go back and fill in the proper TLV length.
 * If 'nestedEncoderFunction' returns 0 or negative bytes, it rewinds the encoder to the original position before appending the TLV header.
 *
 * @param [in] outerEncoder The encoder to append to
 * @param [in] packetDictionary The dictionary to read from
 * @param [in] nestedType the TLV type to use for the nested value
 * @param [in] nestedEncoderFunction The function to call to write the inner value
 *
 * @return non-negative The total number of bytes appended to 'outerEncoder'
 * @return -1 An error
 *
 * Example:
 * @code
 * {
 *    // If the dictionary contains a KeyName Name, then encode the KeyName continer using 'rtaTlvSchemaV0KeyName_Encode'.  Use
 *    // the value 'CCNxCodecSchemaV0_KeyLocatorKeys_KeyName' as the TLV type for the subcontainer.
 *    //
 *    if (ccnxTlvDictionary_IsValueBuffer(packetDictionary, CCNxCodecSchemaV0TlvDictionary_ContentObjectFastArray_KEYNAME_NAME)) {
 *       keynameLength = ccnxCodecTlvUtilities_NestedEncode(keyLocatorEncoder, packetDictionary, CCNxCodecSchemaV0_KeyLocatorKeys_KeyName, rtaTlvSchemaV0KeyName_Encode);
 *    }
 * }
 * @endcode
 */
ssize_t
ccnxCodecTlvUtilities_NestedEncode(CCNxCodecTlvEncoder *outerEncoder, CCNxTlvDictionary *packetDictionary, uint32_t nestedType,
                                   ssize_t (*nestedEncoderFunction)(CCNxCodecTlvEncoder *innerEncoder, CCNxTlvDictionary *packetDictionary));

/**
 * Reads the list 'listKey' from the dictionary and encodes them all as TLV entries
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] encoder The encoder to append to
 * @param [in] packetDictionary The dictionary to read from
 * @param [in] listKey The list key to read from packetDictionary
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
ssize_t
ccnxCodecTlvUtilities_EncodeCustomList(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary, int listKey);


/**
 * Parses the input buffer as a VarInt
 *
 * Parses the bytes of the input buffer as a network byte order variable length integer.
 * Between 1 and 'length' bytes will be parses, where 'length' must be from 1 to 8.
 * The buffer will be advanced as the bytes are read.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00 }, 3, 0, 3 );
 *      uint64_t value;
 *      ccnxCodecTlvUtilities_GetVarInt(buffer, 3, &value);
 *      // value = 0x0000000000102300
 * }
 * @endcode
 */
bool ccnxCodecTlvUtilities_GetVarInt(PARCBuffer *input, size_t length, uint64_t *output);

#endif // TransportRTA_ccnxCodec_TlvUtilities_h
