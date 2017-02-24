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
 * @file ccnxCodec_TlvEncoder.h
 * @ingroup networking
 * @brief TLV codec for messages
 *
 * TLV encoder
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
 * Example to encode a container that wraps a name and an address:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *nameAndAddress = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *
 *      // Initialize starts a new buffer
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 3, nameAndAddress);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *container = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *
 *      parcBuffer_Destroy(&nameAndAddress);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 *
 *      // now use the container for something...
 * }
 * @endcode
 *
 * The TLV will look something like this:
 * @code
 *    { .type = 3, .length = L1 + L2 + overhead, .value = {
 *          {.type = 1, .length = L1, .value = name},
 *          {.type = 2, .length = L2, .value = address}
 *          }
 *     }
 * @endcode
 *
 * An alternative way to encode it that does not require recursive encoder is to use Position:
 * @code
 * {
 *      // Creates {{T=3, L=length, V={{T=1, L=..., V=name}, {T=2, L=..., V=address}}}}
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      size_t offset = ccnxCodecTlvEncoder_Position(encoder);
 *      ccnxCodecTlvEncoder_AppendContainer(encoder, 3, 0);
 *      size_t length = 0;
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 1, name);
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, address);
 *      ccnxCodecTlvEncoder_SetContainerLength(encoder, offset, length);
 * }
 * @endcode
 *
 */

#ifndef libccnx_ccnx_TlvEncoder_h
#define libccnx_ccnx_TlvEncoder_h

#include <parc/algol/parc_Buffer.h>
#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>
#include <parc/security/parc_Signer.h>
#include <parc/security/parc_Signature.h>

#include <ccnx/common/codec/ccnxCodec_Error.h>

// ======================================================

struct ccnx_codec_tlv_encoder;
typedef struct ccnx_codec_tlv_encoder CCNxCodecTlvEncoder;

/**
 * Creates a TLV encoder
 *
 * The encoder is re-usable, as the state is reset for each Initialize.
 *
 * @return non-null A TLV encoder
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecTlvEncoder *ccnxCodecTlvEncoder_Create(void);

/**
 * Destroys the TLV encoder and all internal state
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void ccnxCodecTlvEncoder_Destroy(CCNxCodecTlvEncoder **encoderPtr);

/**
 * Initialize the encoder to start a new encoding
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] tlv <#description#>
 *
 * @return non-null The encoder
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
CCNxCodecTlvEncoder *ccnxCodecTlvEncoder_Initialize(CCNxCodecTlvEncoder *encoder);

/**
 * Appends a TL container and a PARCBuffer to the enoder
 *
 * The type is from the parameter `type', the length is from the remaining size
 * of the value buffer, and the value comes from `value'.
 *
 * @param [in] encoder The encoder to append to
 * @param [in] type    The TLV type
 * @param [in] value   The length is the remaining buffer size, the position is advanced.
 *
 * @return number The total bytes of the TLV, including the T and L.
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendBuffer(CCNxCodecTlvEncoder *encoder, uint16_t type, PARCBuffer *value);

/**
 * Appends a "TL" container then the bytes of the array
 *
 * Writes a Type of 'type' and Length of 'length' then appends 'length' bytes from
 * the array to the encoder.
 *
 * @param [in] encoder An allocated CCnxCodecTlvEncoder
 * @param [in] type The TLV type
 * @param [in] length The TLV length
 * @param [in] array The bytes to append
 *
 * @return number The number of bytes appended (including the T and L)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendArray(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t length, const uint8_t *array);

/**
 * Appends a TL without a V.
 *
 * Appends a T and L without a V.  Useful for doing a heirarchical encoding where the V
 * will be more TLVs.  You will either need to know L or go back and fix it up.
 *
 * @param [in] encoder The encoder to modify
 * @param [in] type The TLV type
 * @param [in] length The TLV length, use 0 if you do not know.
 *
 * @return bytes The bytes appended (the size of the T and L)
 *
 * Example:
 * @code
 * {
 *      // Creates {{ T=99, L=.., V=SequenceNumber }, {T=1, L=length, V={{T=2, L=..., V=name}, {T=3, L=..., V=address}}}}
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendBuffer(encoder, 99, sequenceNumber);
 *      size_t offset = ccnxCodecTlvEncoder_Position(encoder);
 *      ccnxCodecTlvEncoder_AppendContainer(encoder, 1, 0);
 *      size_t length = 0;
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, name);
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 3, address);
 *      ccnxCodecTlvEncoder_SetContainerLength(encoder, offset, length);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendContainer(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t length);

/**
 * Adds a TLV with an 8-bit value
 *
 * Uses network byte order for the value.  The returned size includes
 * the type and length lengths plus the value length.
 *
 * @param [in] encoder The TLV encoder
 * @param [in] type The Type value to use for the container
 * @param [in] value The value to encode
 *
 * @return number The number of bytes appended to the encoded (5)
 *
 * Example:
 * @code
 * {
 *      // encoded buffer with be { .type = 1, .length = 1, .value = 0x19 }
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendUint8(encoder, 1, (uint8_t) 25);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendUint8(CCNxCodecTlvEncoder *encoder, uint16_t type, uint8_t value);

/**
 * Adds a TLV with a 16-bit value
 *
 * Uses network byte order for the value.  The returned size includes
 * the type and length lengths plus the value length.
 *
 * @param [in] encoder The TLV encoder
 * @param [in] type The Type value to use for the container
 * @param [in] value The value to encode
 *
 * @return number The number of bytes appended to the encoded (6)
 *
 * Example:
 * @code
 * {
 *      // encoded buffer with be { .type = 1, .length = 2, .value = 0x1234 }
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendUint16(encoder, 1, (uint16_t) 0x1234);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendUint16(CCNxCodecTlvEncoder *encoder, uint16_t type, uint16_t value);

/**
 * Adds a TLV with a 32-bit value
 *
 * Uses network byte order for the value.  The returned size includes
 * the type and length lengths plus the value length.
 *
 * @param [in] encoder The TLV encoder
 * @param [in] type The Type value to use for the container
 * @param [in] value The value to encode
 *
 * @return number The number of bytes appended to the encoded (8)
 *
 * Example:
 * @code
 * {
 *      // encoded buffer with be { .type = 1, .length = 4, .value = 0x12345678 }
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendUint32(encoder, 1, (uint32_t) 0x12345678);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendUint32(CCNxCodecTlvEncoder *encoder, uint16_t type, uint32_t value);


/**
 * Adds a TLV with a 64-bit value
 *
 * Uses network byte order for the value.  The returned size includes
 * the type and length lengths plus the value length.
 *
 * @param [in] encoder The TLV encoder
 * @param [in] type The Type value to use for the container
 * @param [in] value The value to encode
 *
 * @return number The number of bytes appended to the encoded (8)
 *
 * Example:
 * @code
 * {
 *      // encoded buffer with be { .type = 1, .length = 8, .value = 0x0000000012345678 }
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendUint64(encoder, 1, (uint64_t) 0x12345678);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendUint64(CCNxCodecTlvEncoder *encoder, uint16_t type, uint64_t value);

/**
 * Returns the current encoding buffer position
 *
 * This is useful if you need to backtrack to fill in a length you did not know before.
 *
 * @param [in] encoder The Tlv encoder object
 *
 * @return number The byte offset of the encode buffer
 *
 * Example:
 * @code
 * {
 *      // Creates {{ T=99, L=.., V=SequenceNumber }, {T=1, L=length, V={{T=2, L=..., V=name}, {T=3, L=..., V=address}}}}
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendBuffer(encoder, 99, sequenceNumber);
 *      size_t offset = ccnxCodecTlvEncoder_Position(encoder);
 *      ccnxCodecTlvEncoder_AppendContainer(encoder, 1, 0);
 *      size_t length = 0;
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, name);
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 3, address);
 *      ccnxCodecTlvEncoder_SetContainerLength(encoder, offset, length);
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_Position(CCNxCodecTlvEncoder *encoder);

/**
 * Used to rewind and erase
 *
 * For example, you called ccnxCodecTlvEncoder_AppendContainer() then found out that
 * the container was empty.  If you rewind to just before you added the container, it
 * is as if the container were never added.
 *
 * @param [in] position Must be no more than ccnxCodecTlvEncoder_Position()
 *
 * @return number The position after calling SetPosition
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecTlvEncoder_SetPosition(CCNxCodecTlvEncoder *encoder, size_t position);

/**
 * Sets the length field of the container at the given offset
 *
 * User `ccnxCodecTlvEncoder_Position' before `ccnxCodecTlvEncoder_AppendContainer' to get the container's
 * offset.  You can then set that container's length with this function.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 * @param [in] offset position of the container in the Tlv Encoder
 * @param [in] length The container length to set
 *
 * Example:
 * @code
 * {
 *      // Creates {{ T=99, L=.., V=SequenceNumber }, {T=1, L=length, V={{T=2, L=..., V=name}, {T=3, L=..., V=address}}}}
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_AppendBuffer(encoder, 99, sequenceNumber);
 *      size_t offset = ccnxCodecTlvEncoder_Position(encoder);
 *      ccnxCodecTlvEncoder_AppendContainer(encoder, 1, 0);
 *      size_t length = 0;
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 2, name);
 *      length += ccnxCodecTlvEncoder_AppendBuffer(encoder, 3, address);
 *      ccnxCodecTlvEncoder_SetContainerLength(encoder, offset, length);
 * }
 * @endcode
 */
void ccnxCodecTlvEncoder_SetContainerLength(CCNxCodecTlvEncoder *encoder, size_t offset, uint16_t length);

/**
 * Finalizes the encoding and returns the encoded buffer
 *
 * The buffer is ready for reading.  In specific, it will truncate the buffer at the
 * current position, setting the Limit to that location.  This will cut off any
 * writes to the buffer that have been "erased" by setting the position to an
 * earlier location.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
void ccnxCodecTlvEncoder_Finalize(CCNxCodecTlvEncoder *encoder);

/**
 * Creates a PARCBuffer from position 0 to the Limit.
 *
 * Returns a PARCBuffer from position 0 to the Limit.  If the user has called
 * ccnxCodecTlvEncoder_SetPosition() to rewind the buffer, the user should likely call
 * ccnxCodecTlvEncoder_Finalize() to trim the Limit, otherwise there may be unexpected
 * bytes at the end.
 *
 * The PARCBuffer representation is not the native form of the buffer and will result
 * in a deep copy of the buffer.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return non-null An allocated PARCBuffer, use parcBuffer_Release() on it
 * @return null An error.
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      PARCBuffer *encoded = ccnxCodecTlvEncoder_CreateBuffer(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
PARCBuffer *ccnxCodecTlvEncoder_CreateBuffer(CCNxCodecTlvEncoder *encoder);

/**
 * Creates a vectored I/O representation of the encoder
 *
 * Returns a CCNxCodecNetworkBufferIoVec from position 0 to the Limit.  If the user has called
 * ccnxCodecTlvEncoder_SetPosition() to rewind the buffer, the user should likely call
 * ccnxCodecTlvEncoder_Finalize() to trim the Limit, otherwise there may be unexpected
 * bytes at the end.
 *
 * The CCNxCodecNetworkBufferIoVec is the native form of the memory and does not involve any copies.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return non-null An allocated CCNxCodecNetworkBufferIoVec, use CCNxCodecNetworkBufferIoVec_Release() on it
 * @return null An error.
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Initialize(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_Finalize(encoder);
 *      CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvEncoder_CreateIoVec(encoder);
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxCodecTlvEncoder_CreateIoVec(CCNxCodecTlvEncoder *encoder);

/**
 * Marks the current position as the start of the signature
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);
 *
 *      // The signature is calcualted over the TLV field of the "address"
 *      PARCSignature *sig = ccnxCodecTlvEncoder_ComputeSignature(encoder, signer);
 *
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
void ccnxCodecTlvEncoder_MarkSignatureStart(CCNxCodecTlvEncoder *encoder);

/**
 * Marks the current position as the end (non-inclusive) of the Signature
 * Example:
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);
 *
 *      // The signature is calcualted over the TLV field of the "address"
 *      PARCSignature *sig = ccnxCodecTlvEncoder_ComputeSignature(encoder, signer);
 *
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
void ccnxCodecTlvEncoder_MarkSignatureEnd(CCNxCodecTlvEncoder *encoder);

/**
 * Computes the cryptographic signature over the designated area.
 * If both a Start and End have not been set, function will assert
 *
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @retval non-null An allocated PARCSignature
 * @retval null An error
 *
 * Example:
 * @code
 * {
 *      CCNxCodecTlvEncoder *encoder = ccnxCodecTlvEncoder_Create();
 *      ccnxCodecTlvEncoder_Append(encoder, 1, name);
 *      ccnxCodecTlvEncoder_MarkSignatureStart(encoder);
 *      ccnxCodecTlvEncoder_Append(encoder, 2, address);
 *      ccnxCodecTlvEncoder_MarkSignatureEnd(encoder);
 *
 *      // The signature is calcualted over the TLV field of the "address"
 *      ccnxCodecTlvEncoder_SetSigner(signer);
 *      PARCSignature *sig = ccnxCodecTlvEncoder_ComputeSignature(encoder);
 *
 *      ccnxCodecTlvEncoder_Destroy(&encoder);
 * }
 * @endcode
 */
PARCSignature *ccnxCodecTlvEncoder_ComputeSignature(CCNxCodecTlvEncoder *encoder);

/**
 * Puts a uint8_t at the specified position.
 *
 * Does not modify the current position of the Tlv Encoder
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return number The number of bytes put
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_PutUint8(CCNxCodecTlvEncoder *encoder, size_t offset, uint8_t value);

/**
 * Puts a uint16_t at the specified position.
 *
 * Does not modify the current position of the Tlv Encoder
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return number The number of bytes put
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_PutUint16(CCNxCodecTlvEncoder *encoder, size_t offset, uint16_t value);

/**
 * Writes an array to the current position.  No "TL" container is written.
 *
 * Unlike ccnxCodecTlvEncoder_AppendArray, this does not append a Type and Length container for the array.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 * @param [in] length The number of bytes from array to write
 * @param [in] array The source array
 *
 * @return number The number of bytes appended to the encoder
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendRawArray(CCNxCodecTlvEncoder *encoder, size_t length, uint8_t *array);


/**
 * Determines if the TLV Encoder has an error condition set
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @retval true An error condition is set
 * @retval false No error condition is set
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvEncoder_HasError(const CCNxCodecTlvEncoder *encoder);

/**
 * Sets an error condition.  Only one error condition may be set.
 *
 * Stores a reference counted copy of the CCNxCodecError.  If an error is already set,
 * this function returns false and does not store a reference to the error.  The previous error
 * stays as the current error.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return true Error condition set
 * @return false Error already set, you must clear it first
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxCodecTlvEncoder_SetError(CCNxCodecTlvEncoder *encoder, CCNxCodecError *error);

/**
 * Clears the error condition, if any
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void ccnxCodecTlvEncoder_ClearError(CCNxCodecTlvEncoder *encoder);

/**
 * Retrieves the error message
 *
 * Retrieves the error condition, if any.  If no error is set, will return NULL.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return non-null The error condition set
 * @return null No error condition is set
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecError *ccnxCodecTlvEncoder_GetError(const CCNxCodecTlvEncoder *encoder);


/**
 * Associates a signer with the encoder for producing signatures or MACs
 *
 * Stores a reference counted copy of the signer.  The reference will be released with the
 * CCNxCodecTlvEncoder is released or if this function is called multiple times.
 *
 * It is allowed to set a NULL singer.
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 * @param [in] singer Assocaited a PARCSigner with the encoder, storing a reference to it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void ccnxCodecTlvEncoder_SetSigner(CCNxCodecTlvEncoder *encoder, PARCSigner *signer);

/**
 * Returns the PARCSigner associated with the encoder, if any
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] encoder An allocated CCNxCodecTlvEncoder
 *
 * @return non-null The signer assocated with the encoder
 * @return null There is no singer associated with the encoder
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCSigner *ccnxCodecTlvEncoder_GetSigner(const CCNxCodecTlvEncoder *encoder);

/**
 * Appends a TLV container holding the value as a VarInt
 *
 * A VarInt may be 1 to 8 bytes long.  It is interpreted as an unsigned
 * integer in network byte order.  The value of "0" is encoded as a single
 * byte of "0".
 *
 * @param [in] decoder The TLV decoder
 * @param [in] type The Type value to use for the container
 * @param [in] value The value of the varint
 *
 * @return number The number of bytes appended
 *
 * Example:
 * @code
 * {
 *      uint64_t value = 0x0000000000102300
 *      size_t length = ccnxCodecTlvEncoder_AppendVarInt(encoder, 12, value);
 *      // length = 7
 *      // appedned { 0x00, 0x0C, 0x00, 0x03, 0x10, 0x23, 0x00 }
 * }
 * @endcode
 */
size_t ccnxCodecTlvEncoder_AppendVarInt(CCNxCodecTlvEncoder *encoder, uint16_t type, uint64_t value);

#endif // libccnx_ccnx_TlvEncoder_h
