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
 * @file parc_JSONParser.h
 * @brief A JSON parser
 * @ingroup inputoutput
 *
 */
#ifndef PARC_Library_parc_JSONParser_h
#define PARC_Library_parc_JSONParser_h

struct parc_buffer_parser;
typedef struct parc_buffer_parser PARCJSONParser;

#include <parc/algol/parc_Buffer.h>

/**
 * @def parcJSONValue_OptionalAssertValid
 * Optional validation of the given instance.
 *
 * Define `PARCLibrary_DISABLE_VALIDATION` to nullify validation.
 */
#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcJSONParser_OptionalAssertValid(_instance_)
#else
#  define parcJSONParser_OptionalAssertValid(_instance_) parcJSONParser_AssertValid(_instance_)
#endif


/**
 * Create a new `PARCJSONParser`.
 *
 * @param [in] buffer A pointer to a {@link PARCBuffer} containing the data to parse.
 *
 * @return non-NULL A pointer to a valid `PARCJSONParser`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCJSONParser *parcJSONParser_Create(PARCBuffer *buffer);

/**
 * Assert that an instance of `PARCJSONParser` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue()}
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 */
void parcJSONParser_AssertValid(const PARCJSONParser *parser);

/**
 * Increase the number of references to a `PARCJSONParser`.
 *
 * Note that new `PARCJSONParser` is not created,
 * only that the given `PARCJSONParser` reference count is incremented.
 * Discard the reference by invoking {@link parcJSONParser_Release}.
 *
 * @param parser A pointer to the original instance.
 * @return The value of the input parameter @p parser.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONParser *x2 = parcJSONParser_Acquire(parser);
 *
 *     parcJSONParser_Release(&parser);
 *     parcJSONParser_Release(&x2);
 * }
 * @endcode
 *
 * @see parcJSONParser_Release
 */
PARCJSONParser *parcJSONParser_Acquire(const PARCJSONParser *parser);

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
 * The contents of the dealloced memory used for the PARC object are undefined.
 * Do not reference the object after the last release.
 *
 * @param [in,out] parserPtr A pointer to a pointer to the instance of `PARCJSONParser` to release.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
void parcJSONParser_Release(PARCJSONParser **parserPtr);

/**
 * Advance the parser, skipping any ignored characters.
 *
 * Ignored characters are space, tab and new-line.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     parcJSONParser_SkipIgnored(parser);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
void parcJSONParser_SkipIgnored(PARCJSONParser *parser);

/**
 * Get the next character from the parser.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 *
 * @return The next character
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     char c = parcJSONParser_NextChar(parser);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
char parcJSONParser_NextChar(PARCJSONParser *parser);

/**
 * Get the next character from the parser, returning true or false if successful.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 * @param [out] value A pointer to a `char` to contain the value if successful.
 *
 * @return true If successful
 * @return false If unsuccessful
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     bool success = parcJSONParser_Next(parser, &c);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
bool parcJSONParser_Next(PARCJSONParser *parser, char *value);

/**
 * Get the next character that the parser will process, but do not process it nor advance the parser.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 *
 * @return The next character that the parser will process.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     char c = parcJSONParser_PeekNextChar(parser);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
char parcJSONParser_PeekNextChar(PARCJSONParser *parser);

/**
 * Advance the position of the parser forward or backward by the given number of bytes.
 *
 * To advance forward, bytes is a positive value.
 * To advance backwards, bytes is a negative value.
 *
 * @param [in] parser A pointer to a valid `PARCJSONParser`.
 * @param [in] bytes The number of bytes to move forward or backward.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("abcdef");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     parcJSONParser_Advance(parser, 2);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
void parcJSONParser_Advance(PARCJSONParser *parser, long bytes);

/**
 * Get the number of characters remaining to be parsed.
 *
 * @param [in] parser A pointer to a valid `PARCJSONParser` instance
 *
 * @return The number of characters remaining to be parsed.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("true);
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     size_t remaining = parcJSONParser_Remaining(parser);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
size_t parcJSONParser_Remaining(const PARCJSONParser *parser);

/**
 * Require the fixed string to appear in the current position of the parser.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 * @param [in] string A pointer to a null-terminated C-string that must appear at the current position of the parser.
 *
 * @return true If the string appears.
 * @return false If the string does not appear
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("true");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     bool result = parcJSONParser_RequireString(parser, "true");
 *
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
bool parcJSONParser_RequireString(PARCJSONParser *parser, const char *string);

/**
 * Parse a JSON string returning a {@link PARCBuffer} containing the parsed string.
 *
 * A JSON string begins and ends with a non-escaped double-quote character.
 *
 * @param [in] parser A pointer to a `PARCJSONParser` instance.
 *
 * @return non-NULL A pointer to a valid `PARCBuffer`.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString("\"name\" : 123");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCBuffer *theName = parcJSONParser_ParseString(parser);
 *
 *     parcJSONParser_Release(&parser);
 * }
 * @endcode
 */
PARCBuffer *parcJSONParser_ParseString(PARCJSONParser *parser);

#endif
