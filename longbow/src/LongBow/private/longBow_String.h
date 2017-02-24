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
 * @file longBow_String.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef __LongBow__longBow_String__
#define __LongBow__longBow_String__

#include <stdbool.h>
#include <stdio.h>

#include <LongBow/private/longBow_ArrayList.h>

struct longbow_String;
typedef struct longbow_string LongBowString;

/**
 * Create a LongBowString
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] initialSize The initial buffer size to allocate for the string.
 *
 * @return non-NULL A pointer to a valid LongBowString instance.
 * @return NULL Memory could not be allocated.
 *
 */
LongBowString *longBowString_Create(const size_t initialSize);

/**
 * Create a `LongBowString` instance containing the formatted result of the given format string and parameters.
 *
 * @param [in] format A pointer to a valid LongBowString instance.
 *
 * @return The a LongBowString instance that must be deallocated via longBowString_Deallocate.
 */
LongBowString *longBowString_CreateFormat(const char *format, ...) __attribute__((format(printf, 1, 2)));

/**
 * Destroy a LongBowString instance.
 *
 * The pointer will be set to zero on return.
 *
 * @param [in,out] stringPtr A pointer to a valid LongBowString instance.
 */
void longBowString_Destroy(LongBowString **stringPtr);

/**
 * Append to the given LongBowString instance the formatted result of the given format string and parameters.
 *
 * @param [in] string A pointer to a valid LongBowString instance.
 *
 * @return The value of @p string
 */
LongBowString *longBowString_Format(LongBowString *string, const char *format, ...) __attribute__((format(printf, 2, 3)));

/**
 * Determine if a string begins with a specific prefix.
 *
 * @param [in] string A nul-terminated C string.
 * @param [in] prefix A nul-terminated C string.
 *
 * @return true The value of @p string starts with @p prefix.
 * @return false The value of @p string does not start with @p prefix.
 *
 * Example:
 * @code
 * {
 *     bool result = longBowString_StartsWith("Hello World", "Hello");
 * }
 * @endcode
 */
bool longBowString_StartsWith(const char *string, const char *prefix);

/**
 * Determine if a nul-terminated C string is equal to another.
 *
 * @param [in] string A nul-terminated C string.
 * @param [in] other A nul-terminated C string.
 *
 * @return true The value of @p string starts with @p prefix.
 * @return false The value of @p string does not start with @p prefix.
 *
 * Example:
 * @code
 * {
 *     bool result = longBowString_StartsWith("Hello World", "Hello");
 * }
 * @endcode
 */
bool longBowString_Equals(const char *string, const char *other);

/**
 * Produce a LongBowArrayList containing the tokens for the given @p string
 * where each token is separated by characters in the string @p separators.
 *
 * @param [in] string A nul-terminated C string.
 * @param [in] separators A nul-terminated C string containing the characters that separate the tokens.
 *
 * @return non-NULL A valid LongBowArrayList containing the tokens of the string.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     LongBowArrayList *result = longBowString_Tokenise("Hello World", " ");
 *         ....
 *     longBowArrayList_Destroy(&result);
 * }
 * @endcode
 */
LongBowArrayList *longBowString_Tokenise(const char *string, const char *separators);

/**
 * Produce a nul-terminated C string from the given LongBowString instance.
 *
 * @param [in] string A pointer to a valid LongBowString instance.
 *
 * @return non-NULL A pointer to a nul-terminated C string that must be deallocated via free(3).
 */
char *longBowString_ToString(const LongBowString *string);

/**
 * Write the contents of the given LongBowString instance to the specified FILE output stream.
 *
 * @param [in] string A pointer to a valid LongBowString instance.
 * @param [in] fp A pointer to a valid FILE instance.
 *
 * @return true All of the string was successfully written.
 * @return false All of the string was not successfully written.
 */
bool longBowString_Write(const LongBowString *string, FILE *fp);

#endif /* defined(__LongBow__longBow_String__) */
