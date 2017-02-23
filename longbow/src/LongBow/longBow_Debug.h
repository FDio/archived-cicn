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
 * @file longBow_Debug.h
 * @ingroup internals
 * @brief Support for LongBow and Application Debugging.
 *
 */
#ifndef LongBow_longBow_Debug_h
#define LongBow_longBow_Debug_h

#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include <LongBow/longBow_Location.h>

typedef struct longbow_debug_criteria LongBowDebugCriteria;

/**
 * Pretty print memory on standard output.
 *
 * @param [in] memory A pointer to memory.
 * @param [in] length The number of bytes to display.
 */
void longBowDebug_MemoryDump(const char *memory, size_t length);

/**
 * Generate and send a message to the specified location.
 *
 * @param [in] criteria Debug criteria.
 * @param [in] location The location to which the message is sent.
 * @param [in] format The format string for the message.
 * @param [in] ... Remaining arguments for the message string.
 */
void longBowDebug_Message(LongBowDebugCriteria *criteria, const LongBowLocation *location, const char *format, ...);

/**
 * Write data in an array to a file.
 *
 * Data is written to the specified file from the supplied array.
 *
 * @param [in] fileName The name of the to write to.
 * @param [in] data A pointer to an array of bytes to write.
 * @param [in] length The number of bytes to write.
 *
 * @return The number of bytes written.
 *
 * Example:
 * @code
 * {
 *     size_t numBytesWritten = longBowDebug_WriteFile("log.out", "error", 6);
 * }
 * @endcode
 *
 * @see longBowDebug_ReadFile
 */
ssize_t longBowDebug_WriteFile(const char *fileName, const char *data, size_t length);

/**
 * Read data from a file into an allocated array.
 *
 * Data is read from the specified file into an allocated byte array which must be deallocated by the caller via {stdlib free()}.
 *
 * For convenience the allocate buffer exceeds the size of the file by one byte, which is set to zero.
 * This permits using the result directly as a nul-terminated C string.
 *
 * @param [in] fileName The name of the file to read from.
 * @param [in] data A pointer to a character pointer that will be updated with the address of the read data.
 *
 * @return The number of bytes read, or -1 if there was an error.
 *
 * Example:
 * @code
 * {
 *     char *buffer = NULL;
 *     size_t numBytesRead = longBowDebug_ReadFile("log.out", &buffer);
 *     // use buffer as needed
 * }
 * @endcode
 *
 * @see longBowDebug_WriteFile
 */
ssize_t longBowDebug_ReadFile(const char *fileName, char **data);
#endif
