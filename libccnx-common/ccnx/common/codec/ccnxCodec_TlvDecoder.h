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
 * @file ccnxCodec_TlvDecoder.h
 * @ingroup networking
 * @brief TLV codec for messages
 *
 * TLV decoder
 *
 * Terminology:
 *   type     = a field that labels a value
 *   length   = the byte lenth of the value
 *   value    = the data
 *   header   = type + length
 *   container= a value that contains TLVs
 *
 * For example, in this structure, the "type 1" TLV is a container that holds a second TLV
 * The second TLV is a terminal, and holds an opaque value.
 *
 *  { .type = 1, .length = 20, .value = { .type = 2, .length = 16, .value ="It was a dark a " } }
 *
 * To decode the above example, we would use the decoder like this:
 *
 * @code
 * {
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(container);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      if (type == 3) {
 *          size_t end = ccnxCodecTlvDecoder_GetPosition(decoder) + length;
 *          while ( ccnxCodecTlvDecoder_GetPosition(decoder) < end ) {
 *              type = ccnxCodecTlvDecoder_GetType(decoder);
 *              length = ccnxCodecTlvDecoder_GetLength(decoder);
 *              if (type == 1) {
 *                  PARCBuffer *name = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use name, then release it
 *              } else if (type == 2) {
 *                  PARCBuffer *address = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use address, then release it
 *              }
 *          }
 *      }
 *      PARCReadOnlyBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 * }
 * @endcode
 *
 * Another way to do the same parsing without having to use `ccnxCodecTlvDecoder_GetPosition' is to
 * recursively parse each value:
 *
 * @code
 * {
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(container);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      PARCBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *      if (type == 3) {
 *          CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_Create(value);
 *          while ( ! ccnxCodecTlvDecoder_IsEmpty(innerDecoder) ) {
 *              type = ccnxCodecTlvDecoder_GetType(decoder);
 *              length = ccnxCodecTlvDecoder_GetLength(decoder);
 *              if (type == 1) {
 *                  PARCBuffer *name = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use name, then release it
 *              } else if (type == 2) {
 *                  PARCBuffer *address = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use address, then release it
 *              }
 *          }
 *          ccnxCodecTlvDecoder_Destroy(&innerDecoder);
 *      }
 *      PARCReadOnlyBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 * }
 * @endcode
 *
 * And an even cleaner way is to use ccnxCodecTlvDecoder_GetContainer to pull out sub-decoders
 *
 * @code
 * {
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(container);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      if (type == 3) {
 *          CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_GetContainer(decoder, length);
 *          while ( ! ccnxCodecTlvDecoder_IsEmpty(innerDecoder) ) {
 *              type = ccnxCodecTlvDecoder_GetType(decoder);
 *              length = ccnxCodecTlvDecoder_GetLength(decoder);
 *              if (type == 1) {
 *                  PARCBuffer *name = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use name, then release it
 *              } else if (type == 2) {
 *                  PARCBuffer *address = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *                  // use address, then release it
 *              }
 *          }
 *          ccnxCodecTlvDecoder_Destroy(&innerDecoder);
 *      }
 *      PARCReadOnlyBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 * }
 * @endcode
 *
 */

#ifndef libccnx_ccnx_TlvDecoder_h
#define libccnx_ccnx_TlvDecoder_h

#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>
#include <parc/security/parc_Signer.h>
#include <parc/security/parc_Signature.h>

#include <ccnx/common/codec/ccnxCodec_Error.h>


struct ccnx_codec_tlv_decoder;
typedef struct ccnx_codec_tlv_decoder CCNxCodecTlvDecoder;

/**
 * Decodes a TLV-encoded buffer to individual buffers for each Value
 *
 * Walks through a TLV-encoded buffer returning buffer slices of the
 * original.  These are 0-copy operations.
 *
 * The decoder should be based on the CCNxCodecNetworkBufferIoVec, see case 1009
 *
 * @param [in] buffer The buffer to parse, must be ready to read.
 *
 * @return non-null A TLV decoder
 * @return null An error
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      ccnxCodecTlvDecoder_Destroy(&decoder);
 * }
 * @endcode
 */
CCNxCodecTlvDecoder *ccnxCodecTlvDecoder_Create(PARCBuffer *buffer);

/**
 * Releases the tlv decoder.
 *
 * Buffers that have been previously returned remain acquired.  The internal
 * reference to the input buffer will be released.
 *
 * @param [in] decoderPtr Pointer to the decoder to destroy
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      ccnxCodecTlvDecoder_Destroy(&decoder);
 * }
 * @endcode
 */
void ccnxCodecTlvDecoder_Destroy(CCNxCodecTlvDecoder **decoderPtr);

/**
 * Tests if there is anything left to decode
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true There are more bytes available
 * @return false At the end of the buffer
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvDecoder_IsEmpty(CCNxCodecTlvDecoder *decoder);

/**
 * Checks if there are at least `bytes' bytes remaining in the buffer
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true There are at least `bytes' bytes left
 * @return false Buffer underrun
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvDecoder_EnsureRemaining(CCNxCodecTlvDecoder *decoder, size_t bytes);

/**
 * Returns the bytes remaining in the decoder
 *
 * The remaining bytes to be decoded
 *
 * @param [in] decoder An allocated decoder
 *
 * @retval number The bytes left to decode
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecTlvDecoder_Remaining(const CCNxCodecTlvDecoder *decoder);

/**
 * Gets the next bytes as the TLV type
 *
 * The buffer is advanced the width of the type
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      // GetType                                       |--------|
 *      // GetLength                                                 |--------|
 *      // GetValue                                                              |--------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      PARCBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *      // value = 0x01020304
 * }
 * @endcode
 */
uint16_t ccnxCodecTlvDecoder_GetType(CCNxCodecTlvDecoder *decoder);

/**
 * Returns the TLV Type but does not advance the decoder
 *
 * At the current position, decode the next bytes as the TLV type
 *
 * @param [in] decoder The Decoder object
 *
 * @return number The TLV type
 *
 * Example:
 * @code
 * {
 *      // PeekType                                      |--------|
 *      // GetType                                       |--------|
 *      // GetLength                                                 |--------|
 *      // GetValue                                                              |--------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      unsigned type   = ccnxCodecTlvDecoder_PeekType(decoder);
 *      if( type == 0xAABB ) {
 *         (void) ccnxCodecTlvDecoder_GetType(decoder);
 *         unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *          PARCBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *          // value = 0x01020304
 *      }
 * }
 * @endcode
 */
uint16_t ccnxCodecTlvDecoder_PeekType(CCNxCodecTlvDecoder *decoder);

/**
 * Gets the next bytes as the TLV length
 *
 * The buffer is advanced the width of the length
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      // GetType                                       |--------|
 *      // GetLength                                                 |--------|
 *      // GetValue                                                              |--------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      PARCBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *      // value = 0x01020304
 * }
 * @endcode
 */
uint16_t ccnxCodecTlvDecoder_GetLength(CCNxCodecTlvDecoder *decoder);

/**
 * Returns the next `length' bytes as a value
 *
 * The buffer is advanced `length' bytes.  The returned value is ready for reading.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *      // GetType                                       |--------|
 *      // GetLength                                                 |--------|
 *      // GetValue                                                              |--------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *      unsigned type   = ccnxCodecTlvDecoder_GetType(decoder);
 *      unsigned length = ccnxCodecTlvDecoder_GetLength(decoder);
 *      PARCBuffer * value = ccnxCodecTlvDecoder_GetValue(decoder, length);
 *      // value = 0x01020304
 * }
 * @endcode
 */
PARCBuffer *ccnxCodecTlvDecoder_GetValue(CCNxCodecTlvDecoder *decoder, uint16_t length);

/**
 * Ensure the current position is of type `type', then return a buffer of the value
 *
 * If the buffer points to a type of `type', the function will create a buffer of
 * the specified length and return the value in a buffer.
 *
 * The function will return NULL if the types don't match or if there is a
 * a decoder underrun (its not as long as the type specifies), or if the length would
 * take the end of the input buffer.
 *
 * @param [in] decoder The TLV decoder object
 * @param [in] type The type type to validate
 *
 * @return non-null A conforming buffer
 * @return null An error
 *
 * Example:
 * @code
 * {
 *      // GetBuffer                                     |--------------------------------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 8, 0, 8);
 *
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *
 *      PARCBuffer *buffer = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
 *      // buffer contains { 0x01, 0x02, 0x03, 0x04 }
 * }
 * @endcode
 */
PARCBuffer *ccnxCodecTlvDecoder_GetBuffer(CCNxCodecTlvDecoder *decoder, uint16_t type);

/**
 * The current location is a TLV container (a value that is TLVs)
 *
 * Returns a TLV decoder that represents the "slice" of the input buffer from
 * the current position up to the current position plus `length'.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null A new sub-decoder
 * @return null An error, such as input underrun
 *
 * Example:
 * @code
 * {
 *      // GetType                                       |--------|
 *      // GetLength                                                 |--------|
 *      // GetContainer                                                          |--------------------------------------------|
 *      // GetBuffer                                                             |--------------------------------------------|
 *      PARCBuffer *input = parcBuffer_Wrap((uint8_t[]) {0x00, 0x01, 0x00, 0x08, 0xAA, 0xBB, 0x00, 0x04, 0x01, 0x02, 0x03, 0x04}, 12, 0, 12);
 *
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(input);
 *
 *      uint16_t containerType = ccnxCodecTlvDecoder_GetType(decoder);
 *      size_t containerLength = ccnxCodecTlvDecoder_GetLength(decoder);
 *      CCNxCodecTlvDecoder *innerDecoder = ccnxCodecTlvDecoder_GetContainer(decoder, containerLength);
 *      PARCBuffer *buffer = ccnxCodecTlvDecoder_GetBuffer(decoder, 0xAABB);
 *
 *      ccnxCodecTlvDecoder_Destroy(&innerDecoder);
 *      ccnxCodecTlvDecoder_Destroy(&decoder);
 *
 *      // buffer contains { 0x01, 0x02, 0x03, 0x04 }
 * }
 * @endcode
 */
CCNxCodecTlvDecoder *ccnxCodecTlvDecoder_GetContainer(CCNxCodecTlvDecoder *decoder, uint16_t length);

/**
 * Decodes the current location as a type, length, and uint8_t value.
 *
 * Ensures the type is `type' and returns the value as a uint8_t.  If the type
 * does not match or there is buffer underflow, the function will return false and
 * the output will be unchanged.  If the TLV length is not "1", it will also return false.
 * Otherwise, it returns true and the decoded value.
 *
 * On success, the decoder is advanced, on failure the decoder will remain at the
 * current position.
 *
 * @param [in] decoder The decoder object
 * @param [in] type The TLV type to validate
 * @param [out] output The output value
 *
 * @return true output parameter is valid
 * @return false on error (type did not match or buffer underflow)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x20, 0x00, 0x01, 0xFF }, 5, 0, 5 );
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *      uint8_t value;
 *      bool success = ccnxCodecTlvDecoder_GetUint8(decoder, 0x1020, &value);
 *      assert(success && value == 0xFF);
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_GetUint8(CCNxCodecTlvDecoder *decoder, uint16_t type, uint8_t *output);

/**
 * Decodes the current location as a type, length, and uint16_t value.
 *
 * Ensures the type is `type' and returns the value as a uint16_t.  If the type
 * does not match or there is buffer underflow, the function will return false and
 * the output will be unchanged.  If the TLV length is not "2", it will also return false.
 * Otherwise, it returns true and the decoded value.
 *
 * On success, the decoder is advanced, on failure the decoder will remain at the
 * current position.
 *
 * @param [in] decoder The decoder object
 * @param [in] type The TLV type to validate
 * @param [out] output The output value
 *
 * @return true output parameter is valid
 * @return false on error (type did not match or buffer underflow)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x21, 0x00, 0x02, 0xFF, 0xAA }, 6, 0, 6 );
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *      uint16_t value;
 *      bool success = ccnxCodecTlvDecoder_GetUint16(decoder, 0x1021, &value);
 *      assert(success && value == 0xFFAA);
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_GetUint16(CCNxCodecTlvDecoder *decoder, uint16_t type, uint16_t *output);

/**
 * Decodes the current location as a type, length, and uint32_t value.
 *
 * Ensures the type is `type' and returns the value as a uint32_t.  If the type
 * does not match or there is buffer underflow, the function will return false and
 * the output will be unchanged.  If the TLV length is not "4", it will also return false.
 * Otherwise, it returns true and the decoded value.
 *
 * On success, the decoder is advanced, on failure the decoder will remain at the
 * current position.
 *
 * @param [in] decoder The decoder object
 * @param [in] type The TLV type to validate
 * @param [out] output The output value
 *
 * @return true output parameter is valid
 * @return false on error (type did not match or buffer underflow)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x22, 0x00, 0x04, 0xFF, 0xAA, 0xBB, 0xCC }, 8, 0, 8 );
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *      uint32_t value;
 *      bool success = ccnxCodecTlvDecoder_GetUint32(decoder, 0x1022, &value);
 *      assert(success && value == 0xFFAABBCC);
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_GetUint32(CCNxCodecTlvDecoder *decoder, uint16_t type, uint32_t *output);

/**
 * Decodes the current location as a type, length, and uint64_t value.
 *
 * Ensures the type is `type' and returns the value as a uint64_t.  If the type
 * does not match or there is buffer underflow, the function will return false and
 * the output will be unchanged.  If the TLV length is not "8", it will also return false.
 * Otherwise, it returns true and the decoded value.
 *
 * On success, the decoder is advanced, on failure the decoder will remain at the
 * current position.
 *
 * @param [in] decoder The decoder object
 * @param [in] type The TLV type to validate
 * @param [out] output The output value
 *
 * @return true output parameter is valid
 * @return false on error (type did not match or buffer underflow)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00, 0x08, 0xFF, 0xAA, 0xBB, 0xCC, 0x00, 0x00, 0x00, 0x00 }, 12, 0, 12 );
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *      uint64_t value;
 *      bool success = ccnxCodecTlvDecoder_GetUint64(decoder, 0x1023, &value);
 *      assert(success && value == 0xFFAABBCC00000000ULL);
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_GetUint64(CCNxCodecTlvDecoder *decoder, uint16_t type, uint64_t *output);

/**
 * Returns the current byte position of the buffer
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
size_t ccnxCodecTlvDecoder_Position(CCNxCodecTlvDecoder *decoder);

/**
 * Advance the decoder a given number of bytes
 *
 * Advance the decoder, throwing away a given number of bytes.
 * If there are not enough bytes left in the decoder, no action is taken
 *
 * @param [in] decoder The decoder to advance
 * @param [in] length The number of bytes to skip
 *
 * @return true Advanced the buffer
 * @return false Error, likely a buffer underrun (not enough bytes)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvDecoder_Advance(CCNxCodecTlvDecoder *decoder, uint16_t length);

/**
 * Decode the current position as a VarInt
 *
 * A VarInt may be 1 to 8 bytes long.  It is interpreted as an unsigned
 * integer in network byte order.
 *
 * @param [in] decoder The TLV decoder
 * @param [in] length The number of bytes in the varint
 * @param [out] output The value of the varint
 *
 * @return true Successful decode
 * @return fale Error (length too long, too short, or not enough bytes in decoder)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00 }, 3, 0, 3 );
 *      CCNxCodecTlvDecoder *decoder = ccnxCodecTlvDecoder_Create(buffer);
 *      uint64_t value;
 *      ccnxCodecTlvDecoder_GetVarInt(decoder, 3, &value);
 *      // value = 0x0000000000102300
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_GetVarInt(CCNxCodecTlvDecoder *decoder, uint16_t length, uint64_t *output);


/**
 * Decode the current position of the Buffer as a VarInt out to 'length' bytes
 *
 * A VarInt may be 1 to 8 bytes long.  It is interpreted as an unsigned
 * integer in network byte order.  The buffer must have at least 'length' bytes remaining.
 * The buffer is advanced.
 *
 * @param [in] decoder The TLV decoder
 * @param [in] length The number of bytes in the varint
 * @param [out] output The value of the varint
 *
 * @return true Successful decode
 * @return fale Error (length too long, too short, or not enough bytes in decoder)
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *buffer = parcBuffer_Wrap((uint8_t[]) { 0x10, 0x23, 0x00 }, 3, 0, 3 );
 *      uint64_t value;
 *      ccnxCodecTlvDecoder_BufferToVarInt(buffer, 3, &value);
 *      // value = 0x0000000000102300
 * }
 * @endcode
 */
bool ccnxCodecTlvDecoder_BufferToVarInt(PARCBuffer *buffer, uint16_t length, uint64_t *output);

/**
 * Determines if the TLV Encoder has an error condition set
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
bool ccnxCodecTlvDecoder_HasError(const CCNxCodecTlvDecoder *decoder);

/**
 * Sets an error condition.  Only one error condition may be set.
 *
 * Stores a reference counted copy of the CCNxCodecError.  If an error is already set,
 * this function returns false and does not store a reference count.  The previous error
 * stays as the current error.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true Error condition set
 * @return false Error already set, you must clear it first
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvDecoder_SetError(CCNxCodecTlvDecoder *decoder, CCNxCodecError *error);

/**
 * Clears the error condition, if any
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
void ccnxCodecTlvDecoder_ClearError(CCNxCodecTlvDecoder *decoder);

/**
 * Retrieves the error message
 *
 * Retrieves the error condition, if any.  If no error is set, will return NULL.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null The error condition set
 * @return null No error condition is set
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecError *ccnxCodecTlvDecoder_GetError(const CCNxCodecTlvDecoder *encoder);
#endif // libccnx_ccnx_TlvDecoder_h
