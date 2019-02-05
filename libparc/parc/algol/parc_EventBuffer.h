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
 * @file parc_EventBuffer.h
 * @ingroup events
 * @brief Buffered event management
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_EventBuffer_h
#define libparc_parc_EventBuffer_h

#include <parc/algol/parc_EventQueue.h>

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcEventBuffer_OptionalAssertValid(_instance_)
#else
#  define parcEventBuffer_OptionalAssertValid(_instance_) parcEventBuffer_AssertValid(_instance_)
#endif

/**
 * @typedef PARCEventBuffer
 * @brief A structure containing private libevent state data variables
 */
typedef struct PARCEventBuffer PARCEventBuffer;

/**
 * Create an event buffer instance.
 *
 * @returns A pointer to a new PARCEventBuffer instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventBuffer *parcEventBuffer = parcEventBuffer_Create();
 * }
 * @endcode
 *
 */
PARCEventBuffer *parcEventBuffer_Create(void);

/**
 * Destroy a network parcEventBuffer instance.
 *
 * The address of the event instance is passed in.
 *
 * @param [in] parcEventBuffer - The address of the instance to destroy.
 *
 * Example:
 * @code
 * {
 *     parcEventBuffer_Destroy(&parcEventBuffer)
 * }
 * @endcode
 *
 */
void parcEventBuffer_Destroy(PARCEventBuffer **parcEventBuffer);

/**
 * Determine if an instance of `PARCEventBuffer` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] eventBuffer A pointer to a `PARCEventBuffer` instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCEventBuffer *instance = parcEventBuffer_Create();
 *
 *     if (parcEventBuffer_IsValid(instance)) {
 *         printf("Instance is valid.\n");
 *     }
 * }
 * @endcode
 */
bool parcEventBuffer_IsValid(const PARCEventBuffer *eventBuffer);

/**
 * Assert that the given `PARCEventBuffer` instance is valid.
 *
 * @param [in] eventBuffer A pointer to a valid PARCEventBuffer instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventBuffer *a = parcEventBuffer_Create();
 *
 *     parcEventBuffer_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcEventBuffer_Release(&b);
 * }
 * @endcode
 */
void parcEventBuffer_AssertValid(const PARCEventBuffer *eventBuffer);

/**
 * Get the input parcEventBuffer instance from the Queue
 *
 * @param [in] parcEventQueue - The queue to obtain the input parcEventBuffer from
 * @returns A pointer to the a PARCEventBuffer instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventBuffer *parcEventBuffer = parcEventBuffer_GetQueueBufferInput(eventQueue);
 * }
 * @endcode
 *
 */
PARCEventBuffer *parcEventBuffer_GetQueueBufferInput(PARCEventQueue *parcEventQueue);

/**
 * Get the output parcEventBuffer instance from the Queue
 *
 * @param [in] parcEventQueue - The queue to obtain the output parcEventBuffer from
 * @returns A pointer to the a PARCEventBuffer instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventBuffer *parcEventBuffer = parcEventBuffer_GetQueueBufferOutput(parcEventQueue);
 * }
 * @endcode
 *
 */
PARCEventBuffer *parcEventBuffer_GetQueueBufferOutput(PARCEventQueue *parcEventQueue);

/**
 * Get the data length of the associated buffer
 *
 * @param [in] parcEventBuffer - The buffer to obtain the length from
 * @returns The length of data within the buffer, 0 if the internal buffer has been freed
 *
 * Example:
 * @code
 * {
 *     int length = parcEventBuffer_GetLength(parcEventBuffer);
 * }
 * @endcode
 *
 */
size_t parcEventBuffer_GetLength(PARCEventBuffer *parcEventBuffer);

/**
 * Consolidate data in the associated parcEventBuffer
 *
 * @param [in] parcEventBuffer - The buffer to consolidate
 * @param [in] size - The length of data to consolidate, -1 linearizes the entire buffer
 * @returns A pointer to the first byte in the buffer.
 *
 * Example:
 * @code
 * {
 *     unsigned char *ptr = parcEventBuffer_Pullup(parcEventBuffer, size);
 * }
 * @endcode
 *
 */
uint8_t *parcEventBuffer_Pullup(PARCEventBuffer *parcEventBuffer, ssize_t size);

/**
 * Read data from the associated parcEventBuffer
 *
 * @param [in] parcEventBuffer - The parcEventBuffer to read from
 * @param [in] data - The memory to read into, NULL to discard data
 * @param [in] length - The number of bytes of data to be read
 * @returns Number of bytes read, -1 on failure.
 *
 * Example:
 * @code
 * {
 *     int bytesRead = parcEventBuffer_Read(parcEventBuffer, destination, bytesToRead);
 * }
 * @endcode
 *
 */
int parcEventBuffer_Read(PARCEventBuffer *parcEventBuffer, void *data, size_t length);

/**
 * Read data from the associated parcEventBuffer without delete them from the buffer.
 * Data can not be NULL
 *
 * @param [in] parcEventBuffer - The parcEventBuffer to read from
 * @param [in] data - The memory to read into
 * @param [in] length - The number of bytes of data to be read
 * @returns Number of bytes read, -1 on failure.
 *
 */
int parcEventBuffer_copyOut(PARCEventBuffer *readBuffer, void *data_out, size_t length);


/**
 * Read from a file descriptor into the end of a buffer
 *
 * @param [in] parcEventBuffer - The buffer to read from
 * @param [in] fd - The file descriptor to read from
 * @param [in] length - The number of bytes of data to be read
 * @returns The number of bytes read, 0 on EOF and -1 on error.
 *
 * Example:
 * @code
 * {
 *     int bytesTransfered = parcEventBuffer_ReadFromFileDescriptor(parcEventBuffer, fd, length);
 * }
 * @endcode
 *
 */
int parcEventBuffer_ReadFromFileDescriptor(PARCEventBuffer *parcEventBuffer, int fd, size_t length);

/**
 * Write to a file descriptor from a buffer
 *
 * @param [in] parcEventBuffer - The buffer to read from
 * @param [in] fd - The file descriptor to write to
 * @param [in] length - The number of bytes of data to be read (-1 for all).
 * @returns The number of bytes read, 0 on EOF and -1 on error.
 *
 * Example:
 * @code
 * {
 *     int bytesTransfered = parcEventBuffer_WriteToFileDescriptor(parcEventBuffer, fd, length);
 * }
 * @endcode
 *
 */
int parcEventBuffer_WriteToFileDescriptor(PARCEventBuffer *parcEventBuffer, int fd, ssize_t length);

/**
 * Append one PARCEventBuffer to another
 *
 * @param [in] source - The buffer to append
 * @param [in] destination - The buffer to append to
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventBuffer_AppendBuffer(source, destination);
 * }
 * @endcode
 *
 */
int parcEventBuffer_AppendBuffer(PARCEventBuffer *source, PARCEventBuffer *destination);

/**
 * Append bytes to a parcEventBuffer
 *
 * @param [in] parcEventBuffer - The buffer to write into
 * @param [in] sourceData - The data to append
 * @param [in] length - The number of bytes of data to be added
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventBuffer_Append(parcEventBuffer, sourceData, length);
 * }
 * @endcode
 *
 */
int parcEventBuffer_Append(PARCEventBuffer *parcEventBuffer, void *sourceData, size_t length);

/**
 * Prepend data to the associated parcEventBuffer
 *
 * @param [in] parcEventBuffer - The parcEventBuffer to prepend to
 * @param [in] sourceData - The data to prepend
 * @param [in] length - The number of bytes of data to be added
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventBuffer_Read(parcEventBuffer, sourceData, length);
 * }
 * @endcode
 *
 */
int parcEventBuffer_Prepend(PARCEventBuffer *parcEventBuffer, void *sourceData, size_t length);

/**
 * Move data from one buffer to another
 *
 * @param [in] sourceEventBuffer - The buffer to read from
 * @param [in] destinationEventBuffer - The buffer to move into
 * @param [in] length - The number of bytes of data to be moved
 * @returns The number of bytes moved on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int bytesMoved = parcEventBuffer_Create(sourceEventBuffer, destinationEventBuffer, length);
 * }
 * @endcode
 *
 */
int parcEventBuffer_ReadIntoBuffer(PARCEventBuffer *sourceEventBuffer, PARCEventBuffer *destinationEventBuffer, size_t length);

/**
 * Read a text line terminated by an optional carriage return, followed by a single linefeed
 *
 * @param [in] parcEventBuffer - The buffer to read from
 * @param [in] length - The number of bytes of data to be moved
 * @returns A newly allocated null terminated string, which must be freed by the caller
 *
 * Example:
 * @code
 * {
 *     char *text = parcEventBuffer_ReadLine(parcEventBuffer, length);
 *     ...
 *     parcMemory_Deallocate((void *)&text);
 * }
 * @endcode
 *
 */
char *parcEventBuffer_ReadLine(PARCEventBuffer *parcEventBuffer, size_t *length);

/**
 * Free a text line returned from parcEventBuffer_ReadLine
 *
 * @param [in] parcEventBuffer - The buffer to read from
 * @param [in] line address of returned value from previous call to parcEventBuffer_ReadLine
 *
 * Example:
 * @code
 * {
 *     char *text = parcEventBuffer_ReadLine(parcEventBuffer, length);
 *     ...
 *     parcEventBuffer_FreeLine(parcEventBuffer, &text);
 * }
 * @endcode
 *
 */
void parcEventBuffer_FreeLine(PARCEventBuffer *parcEventBuffer, char **line);

/**
 * Turn on debugging flags and messages
 *
 * @param [in] logger - the log to write debug messages to
 *
 * Example:
 * @code
 * {
 *     parcEventBuffer_EnableDebug(logger);
 * }
 * @endcode
 *
 */
void parcEventBuffer_EnableDebug(PARCLog *logger);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventBuffer_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEventBuffer_DisableDebug(void);
#endif // libparc_parc_EventBuffer_h
