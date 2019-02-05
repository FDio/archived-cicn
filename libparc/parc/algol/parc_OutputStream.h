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
 * @file parc_OutputStream.h
 * @ingroup inputoutput
 * @brief A polymophic interface to specific implementations of modules that implement the
 *        output stream capabilities.
 *
 *
 *
 */
#ifndef libparc_parc_OutputStream_h
#define libparc_parc_OutputStream_h

#include <parc/algol/parc_Buffer.h>

struct parc_output_stream;
typedef struct parc_output_stream PARCOutputStream;

typedef struct parc_output_stream_interface {
    size_t (*Write)(PARCOutputStream *stream, PARCBuffer *buffer);

    PARCOutputStream *(*Acquire)(PARCOutputStream * stream);

    void (*Release)(PARCOutputStream **streamPtr);
} PARCOutputStreamInterface;

/**
 * Create an valid PARCOutputStream instance from the given pointers to a properly
 * initialized `PARCOutputStreamInterface`
 * and specific instance structure that will be supplied to the underlying interface.
 *
 * @param [in] instance A pointer to a `PARCObject` that will be the parameter to the functions specifed by @p interface.
 * @param [in] interface A pointer to a `PARCOutputStreamInterface`.
 *
 * @return non-NULL A pointer to a valid PARCOutputStream instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 * }
 * @endcode
 */
PARCOutputStream *parcOutputStream_Create(void *instance, const PARCOutputStreamInterface *interface);

/**
 * Write the contents of the given `PARCBuffer` to the output stream.
 *
 * The position of the `PARCBuffer` will be set to its limit as a side-effect.
 *
 * @param [in] stream A pointer to a valid `PARCOutputStream` instance.
 * @param [in] buffer A pointer to the `PARCBuffer` whose contents should be written to @p stream.
 *
 * @return The number of bytes written
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     parcOutputStream_Write(output, buffer);
 *     parcOutputStream_Release(&output);
 * }
 * @endcode
 */
size_t parcOutputStream_Write(PARCOutputStream *stream, PARCBuffer *buffer);

/**
 * Write a nul-terminated C string to the given `PARCOutputStream`.
 *
 * @param [in] stream A pointer to a valid `PARCOutputStream` instance.
 * @param [in] string A nul-terminated C string.
 *
 * @return The number of bytes written.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 *
 *     parcOutputStream_WriteCStrings(output, "Hello", " ", "World", NULL);
 *
 *     parcOutputStream_Release(&output);
 * }
 * @endcode
 */
size_t parcOutputStream_WriteCString(PARCOutputStream *stream, const char *string);

/**
 * Write one or more nul-terminated C strings to the given `PARCOutputStream`.
 *
 * @param [in] stream A pointer to a valid `PARCOutputStream` instance.
 * @param [in] ... A NULL terminated variable argument list of nul-terminated C strings.
 *
 * @return The number of bytes written.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 *
 *     parcOutputStream_WriteCStrings(output, "Hello", " ", "World", NULL);
 *
 *     parcOutputStream_Release(&output);
 * }
 * @endcode
 */
size_t parcOutputStream_WriteCStrings(PARCOutputStream *stream, ...);

/**
 * Increase the number of references to a `PARCOutputStream`.
 *
 * Note that new `PARCOutputStream` is not created,
 * only that the given `PARCOutputStream` reference count is incremented.
 * Discard the reference by invoking `parcOutputStream_Release`.
 *
 * @param [in] stream A pointer to a `PARCOutputStream` instance.
 *
 * @return The input `PARCOutputStream` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     parcOutputStream_Write(output, buffer);
 *
 *     PARCOutputStream *x = parcOutputStream_Acquire(output);
 *
 *     parcBuffer_Release(&x);
 *     parcOutputStream_Release(&output);
 * }
 * @endcode
 * @see parcOutputStream_Release
 */
PARCOutputStream *parcOutputStream_Acquire(const PARCOutputStream *stream);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's interface will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in] streamPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(1);
 *     PARCOutputStream *output = parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream),
 *                                                        PARCFileOutputStreamAsPARCInputStream);
 *     PARCBuffer *buffer = parcBuffer_WrapCString("Hello World");
 *     parcOutputStream_Write(output, buffer);
 *     parcOutputStream_Release(&output);
 * }
 * @endcode
 * @see parcOutputStream_Acquire
 * @see parcOutputStream_Create
 */
void parcOutputStream_Release(PARCOutputStream **streamPtr);
#endif // libparc_parc_OutputStream_h
