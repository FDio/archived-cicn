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
 * @brief A structure of functions to enable accessing a CCNxTlvDictionary as a
 *        wire format object.
 *
 */

#ifndef __CCNx_Common__ccnx_WireFormatMessage__
#define __CCNx_Common__ccnx_WireFormatMessage__

#include <ccnx/common/internal/ccnx_WireFormatMessageInterface.h>
#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <parc/security/parc_CryptoHash.h>

typedef CCNxTlvDictionary CCNxWireFormatMessage;

/**
 * Creates a new CCNxWireFormatMessage instance from the `wireFormat` buffer passed in.
 * The schema version and the message type are determined from `wireFormat`.
 *
 * @param wireFormat - a buffer, in wire format, representing the message to created.
 * @return A new instance of CCNxWireFormatMessage that must eventually be released by calling
 *          {@link ccnxWireFormatMessage_Release}.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 * @see ccnxWireFormatMessage_Release
 */
CCNxWireFormatMessage *ccnxWireFormatMessage_Create(const PARCBuffer *wireFormat);

/**
 * Creates dictionary of Interest type from the wire format
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxWireFormatMessage *ccnxWireFormatMessage_FromInterestPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat);

/**
 * Creates dictionary of Interest type from the wire format
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxWireFormatMessage *ccnxWireFormatMessage_FromInterestPacketTypeIoVec(const CCNxTlvDictionary_SchemaVersion schemaVersion, const CCNxCodecNetworkBufferIoVec *vec);

/**
 * Creates a dictionary of ContentObject type from the wire format
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
CCNxWireFormatMessage *ccnxWireFormatMessage_FromContentObjectPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat);

/**
 * Creates a dictionary of Control type from the wire format
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
CCNxWireFormatMessage *ccnxWireFormatMessage_FromControlPacketType(const CCNxTlvDictionary_SchemaVersion schemaVersion, const PARCBuffer *wireFormat);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxWireFormatMessage_OptionalAssertValid(_instance_)
#else
#  define ccnxWireFormatMessage_OptionalAssertValid(_instance_) ccnxWireFormatMessage_AssertValid(_instance_)
#endif

/**
 * Assert that the given `CCNxWireFormatMessage` instance is valid.
 *
 * @param [in] instance A pointer to a valid CCNxWireFormatMessage instance.
 *
 * Example:
 * @code
 * {
 *     CCNxWireFormatMessage *a = ccnxWireFormatMessage_Create();
 *
 *     ccnxWireFormatMessage_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     ccnxWireFormatMessage_Release(&b);
 * }
 * @endcode
 */
void ccnxWireFormatMessage_AssertValid(const CCNxWireFormatMessage *message);

/**
 * Returns the PARCBuffer that wraps the entire wireformat representation
 *
 * May be NULL if there is no wire format yet (e.g. you're going down the stack before the codec)
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCBuffer *ccnxWireFormatMessage_GetWireFormatBuffer(const CCNxWireFormatMessage *dictionary);

/**
 * Returns the CCNxCodecNetworkBufferIoVec that wraps the entire wireformat representation
 *
 * May be NULL if there is no wire format yet (e.g. you're going down the stack before the codec), or it
 * may be NULL because the wireformat is wrapped in a PARCBuffer instead.
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxWireFormatMessage_GetIoVec(const CCNxWireFormatMessage *dictionary);


/**
 * Sets the wire format buffer in a Dictionary
 *
 * You can only put the wire format once.  Will store its down reference to the wire format, so
 * caller should release their copy when done with it.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true Value was put in the dictionary
 * @return false Value not put, likely already existed
 *
 * Example:
 * @code
 * {
 *
 * }
 * @endcode
 */
bool ccnxWireFormatMessage_PutWireFormatBuffer(CCNxWireFormatMessage *dictionary, PARCBuffer *buffer);

/**
 * Sets the wire format buffer in a Dictionary
 *
 * You can only put the wire format once.  Will store its down reference to the wire format, so
 * caller should release their copy when done with it.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true Value was put in the dictionary
 * @return false Value not put, likely already existed
 *
 * Example:
 * @code
 * {
 *
 * }
 * @endcode
 */
bool ccnxWireFormatMessage_PutIoVec(CCNxWireFormatMessage *dictionary, CCNxCodecNetworkBufferIoVec *vec);

/**
 * Writes the wire format to the specified file
 *
 * The file will be truncated to 0.  If there is no wire format, the file will remain at 0 bytes.
 *
 * You can view the packet with the Wireshark plugin.  First convert it to a TCP enapsulation
 * (the program text2pcap is included as part of WireShark).  The part "- filename.pcap" means that
 * text2pcap will read from stdin (the "-") and write to "filename.pcap".  Text2pcap requires a
 * text format derived from "od" or from "hexdump".
 *
 *     od -Ax -tx1 -v filename | text2pcap -T 9695,9695 - filename.pcap
 *     tshark -r filename.pcap -V
 *
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
void ccnxWireFormatMessage_WriteToFile(const CCNxWireFormatMessage *dictionary, const char *filename);

/**
 * Write to the dictionary the start of the protection region
 *
 * The protected region is the byte range that is protected by the signature (validation payload).
 * This sets the validation region in the dictionary.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return false A validation region is already set, cannot overwrite
 * @return true The validation region was set in the dictionary
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxWireFormatMessage_SetProtectedRegionStart(CCNxWireFormatMessage *dictionary, size_t startPosition);

/**
 * Write to the dictionary the length of the protection region
 *
 * The protected region is the byte range that is protected by the signature (validation payload).
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return false A validation region is already set, cannot overwrite
 * @return true The validation region was set in the dictionary
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxWireFormatMessage_SetProtectedRegionLength(CCNxWireFormatMessage *dictionary, size_t length);

/**
 * Runs a hasher over the protected part of the wire format message
 *
 * The protected part of the wire format message is from the start of the CCNx Message
 * through the end of the Validation Algorithm.
 *
 * If no validation algorithm block exists, then there is no protection region and
 * we cannot compute the hash.
 *
 * The protection region is computed by the CCNxCodecTlvDecoder and stored in the dictionary.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return null There is no protection region
 * @return non-null The computed hash
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCCryptoHash *ccnxWireFormatMessage_HashProtectedRegion(const CCNxWireFormatMessage *dictionary, PARCCryptoHasher *hasher);

/**
 * Calculates the ContentObject Hash, which is the SHA256 hash of the protected part of the wire format message.
 *
 * The protected part of the wire format message is from the start of the CCNx Message
 * through the end of the Validation Algorithm.
 *
 * The protection region is computed by the CCNxCodecTlvDecoder and stored in the dictionary.
 *
 * This function must only be called on a dictionary that contains a ContentObject and a WireFormat buffer.
 *
 * @param [in] dictionary The ContentObject message on which to calculate its hash.
 *
 * @return null if we could not calculate the hash. This could mean the protected region wasn't set, or it wasn't a ContentObject.
 * @return non-null The computed hash
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

PARCCryptoHash *ccnxWireFormatMessage_CreateContentObjectHash(CCNxWireFormatMessage *dictionary);

/**
 * Returns a pointer to the CCNxTlvDictionary underlying the specified CCNxWireFormatMessage.
 *
 * @param message - The CCNxWireFormatMessage instance from which to retrieve the CCNxTlvDictionary.
 * @return A pointer to the underlying CCNxTlvDictionary
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxTlvDictionary *ccnxWireFormatMessage_GetDictionary(const CCNxWireFormatMessage *message);

/**
 * Increase the number of references to a `CCNxWireFormatMessage`.
 *
 * Note that a new `CCNxWireFormatMessage` is not created,
 * only that the given `CCNxWireFormatMessage` reference count is incremented.
 * Discard the reference by invoking {@link ccnxWireFormatMessage_Release}.
 *
 * @param [in] message A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *
 *     CCNxWireFormat *reference = ccnxWireFormatMessage_Acquire(message);
 *
 *     ...
 *
 *     ccnxWireFormatMessage_Release(&reference);
 *
 * }
 * @endcode
 *
 * @see {@link ccnxWireFormatMessage_Release}
 */
CCNxWireFormatMessage *ccnxWireFormatMessage_Acquire(const CCNxWireFormatMessage *message);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] messageP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxWireFormatMessage *reference = ccnxWireFormatMessage_Acquire(message);
 *
 *     ...
 *
 *     ccnxWireFormatMessage_Release(&reference);
 *
 * }
 * @endcode
 *
 * @see {@link ccnxWireFormatMessage_Acquire}
 */
void ccnxWireFormatMessage_Release(CCNxWireFormatMessage **messageP);

/**
 * Write a hoplimit to a messages attached wire format io vectors or buffers
 *
 * @param [in] message
 * @param [in] hoplimit
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxWireFormatMessage_SetHopLimit(CCNxWireFormatMessage *message, uint32_t hoplimit);

/**
 * Given an Interest (as a CCNxWireFormatMessage), convert it to an InterestReturn and
 * set the return code of the InterestReturn. This does not create a new instance, but
 * simply modifies the supplied Interest in place.
 *
 * @param [in] message - the Interest (as a CCNxWireFormatMessage) to convert.
 * @param [in] returnCode - the InterestReturn code to embed in the message.
 *
 * Example:
 * @code
 * {
 *     CCNxWireFormatMessage *interest = <...>;
 *
 *     CCNxWireFormatMessage *interestReturn =
 *         ccnxWireFormatMessage_ConvertInterestToInterestReturn(interest,
 *             CCNxInterestReturn_ReturnCode_HopLimitExceeded);
 *
 * }
 * @endcode
 */
bool ccnxWireFormatMessage_ConvertInterestToInterestReturn(CCNxWireFormatMessage *message, uint8_t returnCode);
#endif /* defined(__CCNx_Common__ccnx_WireFormatMessage__) */
