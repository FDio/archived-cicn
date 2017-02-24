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
 * @file ccnxCodec_Error.h
 * @brief Wraps an error condition in the Tlv codec
 *
 * <#Detailed Description#>
 *
 */
#ifndef libccnx_ccnxCodec_Error_h
#define libccnx_ccnxCodec_Error_h

#include <stdlib.h>
#include <ccnx/common/codec/ccnxCodec_ErrorCodes.h>

struct ccnx_codec_error;
typedef struct ccnx_codec_error CCNxCodecError;

/**
 * <#One Line Description#>
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
const char *ccnxCodecError_ErrorMessage(CCNxCodecErrorCodes code);

/**
 * <#One Line Description#>
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
CCNxCodecError *ccnxCodecError_Create(CCNxCodecErrorCodes code, const char *func, int line, size_t byteOffset);

/**
 * Returns a reference counted copy of the error
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return non-null A reference counted copy
 * @return null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecError *ccnxCodecError_Acquire(CCNxCodecError *error);

/**
 * Releases a reference count
 *
 * When the reference count reaches 0, the object is destroyed.
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
void ccnxCodecError_Release(CCNxCodecError **errorPtr);

/**
 * The byte offset of the error
 *
 *   Primarily for decoding errors.  It will contain the byte offset of the first byte
 *   of the field causing the error.  For encoding, it will be the byte offer of the
 *   partially-encoded buffer, but the error is usually in the native format, not the
 *   partially encoded buffer.
 *
 * @param <#param1#>
 * @return The byte offset of the error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t ccnxCodecError_GetByteOffset(const CCNxCodecError *error);


/**
 * If there was a decode error, return the error code
 *
 *   A text message is available from <code>tlvErrors_ErrorMessage()</code>.
 *
 * @param <#param1#>
 * @return Returns the error code, or TVL_ERR_NO_ERROR for successful decode
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxCodecErrorCodes ccnxCodecError_GetErrorCode(const CCNxCodecError *error);

/**
 * The function where the error occured
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
const char *ccnxCodecError_GetFunction(const CCNxCodecError *error);

/**
 * The line where the error occured
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
int ccnxCodecError_GetLine(const CCNxCodecError *error);

/**
 * Descriptive error message
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return A static text string
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *ccnxCodecError_GetErrorMessage(const CCNxCodecError *error);

/**
 * A string representation of the entire error
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return An internally allocated string, do not destroy it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *ccnxCodecError_ToString(CCNxCodecError *error);
#endif // libccnx_ccnx_TlvError_h
