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
 * @file parc_FileOutputStream.h
 * @ingroup inputoutput
 * @brief A file output stream is an output stream for writing data to a File or to a FileDescriptor.
 *
 * Whether or not a file is available or may be created depends upon the underlying platform.
 * Some platforms, in particular, allow a file to be opened for writing by only one FileOutputStream
 * (or other file-writing object) at a time. In such situations the constructors in this class will
 * fail if the file involved is already open.
 *
 */

#ifndef libparc_parc_FileOutputStream_h
#define libparc_parc_FileOutputStream_h

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_OutputStream.h>

struct parc_file_output_stream;
typedef struct parc_file_output_stream PARCFileOutputStream;

/**
 * The mapping of a `PARCFileOutputStream` to the generic `PARCInputStream`.
 *
 */
extern PARCOutputStreamInterface *PARCFileOutputStreamAsPARCOutputStream;

/**
 * Create a new output stream on a file descriptor.
 *
 * Caution: When the resulting `PARCFileOutputStream` is released, the file descriptor is closed.
 * If you wrap STDOUT_FILENO, for example, the standard output of the application will be closed
 * when this PARCFileOutputStream is released.
 * To avoid this, use dup(2) or dup2(2).
 *
 * @param [in] fileDescriptor The fileDescriptor for the file on which to create an output stream.
 *
 * @return A pointer to a new instance of `PARCFileOutputStream`
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCFileOutputStream *parcFileOutputStream_Create(int fileDescriptor);

/**
 * Convert an instance of `PARCFileOutputStream` to a `PARCOutputStream`.
 *
 * @param [in] fileOutputStream A pointer to a valid PARCFileOutputStream.
 *
 * @return  A pointer to a new instance of `PARCOutputStream`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCOutputStream *parcFileOutputStream_AsOutputStream(PARCFileOutputStream *fileOutputStream);

/**
 * Acquire a new reference to an instance of `PARCFileOutputStream`.
 *
 * The reference count to the instance is incremented.
 *
 * @param [in] stream The instance of `PARCFileOutputStream` to which to refer.
 *
 * @return The same value as the input parameter @p stream
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCFileOutputStream *parcFileOutputStream_Acquire(const PARCFileOutputStream *stream);

/**
 * Release a `PARCFileOutputStream` reference.
 *
 * Only the last invocation where the reference count is decremented to zero,
 * will actually destroy the `PARCFileOutputStream`.
 *
 * @param [in,out] streamPtr is a pointer to the `PARCFileOutputStream` reference.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcFileOutputStream_Release(PARCFileOutputStream **streamPtr);

/**
 * Write the contents of a {@link PARCBuffer} to the given `PARCFileOutputStream`.
 *
 * The contents of the `PARCBuffer` from the current position to the limit are written to the `PARCFileOutputStream`.
 * When this function returns the position is set to the end of the last successfully written byte of data.
 *
 * @param [in,out] outputStream The `PARCOutputStream` to write to.
 * @param [in] buffer The `PARCBuffer` to write, from the current position of the buffer to its limit.
 *
 * @return true The entire contents of the `PARCBuffer` were written.
 * @return false The entire contents of the `PARCBuffer` were not written.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *stream =
 *         parcFileOutputStream_Create(open("/tmp/test_parc_FileOutputStream", O_CREAT|O_WRONLY|O_TRUNC, 0600));
 *
 *     PARCBuffer *buffer = parcBuffer_Allocate(16 * 1024*1024);
 *
 *     parcFileOutputStream_Write(stream, buffer);
 *
 *     parcAssertFalse(parcBuffer_HasRemaining(buffer), "Expected the buffer to be emtpy");
 *
 *     parcBuffer_Release(&buffer);
 *
 *     parcFileOutputStream_Release(&stream);
 * }
 * @endcode
 */
bool parcFileOutputStream_Write(PARCFileOutputStream *outputStream, PARCBuffer *buffer);
#endif // libparc_parc_FileOutputStream_h
