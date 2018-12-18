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
 * @file parc_ElasticString.h
 * @ingroup memory
 * @brief An elastic C string.
 *
 * An elastic string is a dynamic array of characters which are readily expressed as a
 * nul-terminated C string.
 *
 */
#ifndef libparc_parc_ElasticString_h
#define libparc_parc_ElasticString_h

#include <stdarg.h>

#include <parc/algol/parc_Buffer.h>

struct parc_elastic_string;
typedef struct parc_elastic_string PARCElasticString;

/**
 * Perform validation on a pointer to a `PARCElasticString`.
 *
 * If invalid, this function will abort the running process.
 *
 * @param *  string A pointer to a `PARCElasticString` to validate.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     parcElasticString_AssertValid(string);
 * }
 * @endcode
 */
void parcElasticString_AssertValid(const PARCElasticString *string);

/**
 * Create an empty `PARCElasticString` instance.
 *
 *   The instance will be empty upon initialization (i.e., `parcElasticString_ToString()` will
 *   return an empty string), but characters and strings may be inserted/appended to
 *   the instance to produce usable content,
 *
 * @return A pointer to an allocated `PARCElasticString` that must be freed with `parcElasticString_Release()`.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     // use the string as necessary
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
PARCElasticString *parcElasticString_Create(void);

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
 * @param *  string A pointer to a pointer to the instance to release.
 *
 * @return The number of remaining references to the object.
 *
 * Example:
 * @code
 * {
 *     PARCElasticString *buffer = parcElasticString_Create();
 *
 *     parcElasticString_Release(&pathName);
 * }
 * @endcode
 */
void parcElasticString_Release(PARCElasticString **string);

/**
 * Retrieve the number of remaining bytes between the current position
 * in the string and its (flexible) limit.
 *
 * @param *  string A pointer to an `PARCElasticString` instance.
 *
 * @return The non-negative number of characters remaining between the position and limit.
 *
 * Example:
 * @code
 * {
 *     char *inputString = "Hello World";
 *     size_t inputLength = strlen(input);
 *     PARElasticString *string = parcElasticString_Create();
 *     parcElasticString_PutString(string, inputString);
 *     parcElasticString_Flip(string);
 *     size_t numRemaining = parcElasticString_Remaining(string);
 *
 *     // numRemaining == inputLength
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
size_t parcElasticString_Remaining(const PARCElasticString *string);

/**
 * Set the limit to the current position, then the position to zero.
 * If the mark is defined, it is invalidated,
 * and any subsequent operation that requires the mark will abort until the mark
 * is set again via `parcElasticString_Mark`.
 *
 * @param *  string A pointer to an `PARCElasticString` instance.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     char *inputString = "Hello World";
 *     size_t inputLength = strlen(input);
 *     PARElasticString *string = parcElasticString_Create();
 *     parcElasticString_PutString(string, inputString);
 *     parcElasticString_Flip(string);
 *     size_t numRemaining = parcElasticString_Remaining(string);
 *
 *     // numRemaining == inputLength
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
PARCElasticString *parcElasticString_Flip(PARCElasticString *string);

/**
 * Return the given `PARCElasticString`'s position.
 *
 * A buffer's position is the index of the next element to be read or written.
 * A buffer's position is never negative and is never greater than its limit.
 *
 * @param *  string A pointer to a `PARCElasticString` instance.
 *
 * @return The given `PARCElasticString`'s position.
 *
 * Example:
 * @code
 * {
 *     size_t currentPosition = parcBuffer_Position(buffer);
 * }
 * @endcode
 *
 * @see parcElasticString_SetPosition
 */
size_t parcElasticString_Position(const PARCElasticString *string);

/**
 * Set the position in the given `PARCElasticString`.
 *
 * If the mark is defined and larger than the new position then it is invalidated.
 *
 * @param *  string A pointer to a `PARCElasticString` instance.
 * @param *  newPosition The new position for the `PARCElasticString`.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *     parcElasticString_PutString(string, "Hello World");
 *
 *     parcElasticString_SetPosition(string, 0);
 *
 *     // position is now at 0, instead of at the end of "Hello World"
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_Position
 */
PARCElasticString *parcElasticString_SetPosition(PARCElasticString *string, size_t newPosition);

/**
 * Append an array with the specified number of bytes to the end of this `PARCElasticString` instance.
 *
 * The position of the string is advanced by the length of the array.
 *
 * @param *  string A pointer to a `PARCElasticString` instance.
 * @param *  array A pointer to the array containing the bytes to append.
 * @param *  length The length of the input array.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     uint8_t * appendArray = { 0x00, 0x01, 0x02, 0x03, 0x04 };
 *     parcElasticString_PutArray(string, appendArray, 5);
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_PutString
 */
PARCElasticString *parcElasticString_PutArray(PARCElasticString *string, const char *array, size_t length);

/**
 * Append a C-string to the end of this `PARCElasticString` instance.
 *
 * The position of the string is advanced by the length of the string.
 *
 * @param *  string A pointer to a `PARCElasticString` instance.
 * @param *  cString A pointer to a nul-terminated C string to append to this `PARCElasticString`.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     parcElasticString_PutString(string, "Hello World");
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_PutArray
 */
PARCElasticString *parcElasticString_PutString(PARCElasticString *string, const char *cString);

/**
 * Append the contents of a `PARCBuffer` instance to the end of the `PARCElasticString` instance.
 *
 * The position of the string is advanced by the length of the buffer.
 *
 * @param *  string A pointer to a `PARCElasticString` instance.
 * @param *  buffer A pointer to a `PARCBuffer` instance.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     uint8_t * array = { 'H', 'e', 'l', 'l', 'o' };
 *     PARCBuffer *endBuffer = parcBuffer_Allocate(10);
 *     parcBuffer_PutArray(endBuffer, 5, array);
 *     parcElasticString_PutBuffer(string, endBuffer);
 *
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_PutString
 */
PARCElasticString *parcElasticString_PutBuffer(PARCElasticString *string, PARCBuffer *buffer);

/**
 * Append a single character (byte) to the end of this string.
 *
 * The position of the string is advanced by one (1).
 *
 * @param *  string A pointer to `PARCElasticString`
 * @param *  character A `char` value to append.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     parcElasticString_PutChar(string, 'H');
 *     parcElasticString_PutChar(string, 'e');
 *     parcElasticString_PutChar(string, 'l');
 *     parcElasticString_PutChar(string, 'l');
 *     parcElasticString_PutChar(string, 'o');
 *
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_PutString
 */
PARCElasticString *parcElasticString_PutChar(PARCElasticString *string, const char character);

/**
 * Put a variable number of characters into the `PARCElasticString`.
 *
 * @param *  string The `PARCElasticString` to receive the characters.
 * @param *  count The number of characters to insert into the `PARCElasticString`
 * @param *  ...  The characters to insert into the `PARCElasticString`.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     parcElasticString_PutChar(string, 5, 'H', 'e', 'l', 'l', 'o');
 *
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 *
 * @see parcElasticString_PutString
 */
PARCElasticString *parcElasticString_PutChars(PARCElasticString *string, unsigned int count, ...);

/**
 * Append an arbitrary number of C-style strings to the given `PARCElasticString` instance.
 *
 * @param *  string A pointer to `PARCElasticString`
 * @param *  ... The nul-terminated, C-style strings to append to the given `PARCElasticString`.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *baseString = parcElasticString_Create();
 *     parcElasticString_PutString(baseString, "Hello");
 *     uint8_t * string1 = {' ', '\0'};
 *     uint8_t * string2 = {'W', 'o', 'r', 'l', 'd', '\0'};
 *
 *     parcElasticString_PutStrings(baseString, string1, string2);
 *
 *     printf("String = %s\n", parcElasticString_ToString(baseString));
 *
 *     parcElasticString_Release(&baseString);
 * }
 * @endcode
 */
PARCElasticString *parcElasticString_PutStrings(PARCElasticString *string, ...);

/**
 * Append a formatted nul-terminated, C-style string string to the given `PARCElasticString` instance.
 *
 * @param *  string A pointer to `PARCElasticString`.
 * @param *  format The format string
 * @param *  ... Remaining parameters used to format the string.
 *
 * @return The same pointer as the `string` parameter.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *
 *     parcElasticString_Format(string, "Hello %s\n", "World");
 *
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
PARCElasticString *parcElasticString_Format(PARCElasticString *string, const char *format, ...) \
    __attribute__((format(printf, 2, 3)));

/**
 * Retrieve a handle to the `PARCBuffer` instance. The reference count is not increased.
 *
 * @param *  string A pointer to `PARCElasticString`.
 *
 * @return The `PARCBuffer` instance used to encapsulate the `PARCElasticString` contents.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *     parcElasticString_PutString(string, "Hello World");
 *
 *     PARCBuffer *buffer = parcElasticString_ToBuffer(string);
 *     printf("String in hex = %s\n", parcBuffer_ToHexString(buffer));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
PARCBuffer *parcElasticString_ToBuffer(const PARCElasticString *string);

/**
 * Produce a C string representation of the given `PARCElasticString`.
 *
 * Produce an allocated, nul-terminated string representation of the given `PARCElasticString`.
 * The result must be freed by the caller via the `parcMemory_Deallocate()` function.
 *
 * @param *  string A pointer to `PARCElasticString`.
 *
 * @return A pointer to an allocated array containing a nul-terminated string the must be freed via `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     PARElasticString *string = parcElasticString_Create();
 *     parcElasticString_PutString(string, "Hello World");
 *
 *     printf("String = %s\n", parcElasticString_ToString(string));
 *
 *     parcElasticString_Release(&string);
 * }
 * @endcode
 */
char *parcElasticString_ToString(const PARCElasticString *string);
#endif // libparc_parc_ElasticString_h
