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
 * @file parc_InputStream.h
 * @ingroup inputoutput
 * @brief Generalized Input Stream
 *
 * a `PARCFileInputStream` is a kind of `PARCInputStream`
 *
 */
#ifndef libparc_parc_InputStream_h
#define libparc_parc_InputStream_h

#include <parc/algol/parc_Buffer.h>


/**
 * @typedef `PARCInputStream`
 */
struct parc_input_stream;
typedef struct parc_input_stream PARCInputStream;

/**
 * @typedef PARCInputStreamInterface
 */

typedef struct parc_input_stream_interface {
    size_t (*Read)(PARCInputStream *inputStream, PARCBuffer *buffer);

    PARCInputStream *(*Acquire)(const PARCInputStream * instance);

    void (*Release)(PARCInputStream **instancePtr);
} PARCInputStreamInterface;

/**
 * Create an instance of a `PARCInputStream` given a pointer to an instance and interface.
 *
 *
 * @param [in] instance A pointer to a structure suitable for the given `PARCInputStreamInterface`.
 * @param [in] interface A pointer to a `PARCInputStreamInterface`
 *
 * @return non-NULL A pointer to a valid PARCInputStream
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 */
PARCInputStream *parcInputStream(void *instance, const PARCInputStreamInterface *interface);

/**
 * Read a `PARCInputStream` into a {@link PARCBuffer}.
 *
 * The contents of the `PARCBuffer` are filled from the current position to the limit.
 * When this function returns the position is set to the end of the last successfully read byte of data.
 *
 * @param [in] inputStream The `PARCInputStream` to read.
 * @param [in] buffer The `PARCBuffer` to fill, from the current position of the buffer to its limit.
 *
 * @return number of bytes read / filled.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcInputStream_Read(PARCInputStream *inputStream, PARCBuffer *buffer);

/**
 * Acquire a new reference to an instance of `PARCInputStream`.
 *
 * The reference count to the instance is incremented.
 *
 * @param [in] instance The instance of `PARCInputStream` to which to refer.
 *
 * @return The same value as the input parameter @p instance
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCInputStream *parcInputStream_Acquire(const PARCInputStream *instance);

/**
 * Release a `PARCInputStream` reference.
 *
 * Only the last invocation where the reference count is decremented to zero,
 * will actually destroy the `PARCInputStream`.
 *
 * @param [in,out] instancePtr is a pointer to the `PARCInputStream` reference.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcInputStream_Release(PARCInputStream **instancePtr);
#endif // libparc_parc_InputStream_h
