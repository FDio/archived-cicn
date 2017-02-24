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
 * @file parc_JSON.h
 * @ingroup inputoutput
 * @brief JSON A JSON pair consists of a name and a value separated by a colon.
 *
 */
#ifndef libparc_parc_JSONPair_h
#define libparc_parc_JSONPair_h

#include <stdbool.h>

struct parcJSONPair;
typedef struct parcJSONPair PARCJSONPair;

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_List.h>

#include <parc/algol/parc_JSONArray.h>
#include <parc/algol/parc_JSONValue.h>
#include <parc/algol/parc_JSONParser.h>

/**
 * Create a new JSON Pair
 *
 * @param [in] name A pointer to a {@link PARCBuffer} instance containing the name for the JSON Pair.
 * @param [in] value A pointer to a {@link PARCJSONValue} instance containing the value for the JSON Pair.
 * @return A pointer to a new `PARCJSONPair`, or NULL if an error occured.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *name = parcBuffer_AllocateCString("myname");
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     PARCJSONPair *pair = parcJSONPair_Create(name, value);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see {@link parcJSONPair_CreateFromString}
 * @see {@link parcJSONPair_CreateFromNULL}
 * @see {@link parcJSONPair_CreateFromBoolean}
 * @see {@link parcJSONPair_CreateFromInteger}
 * @see {@link parcJSONPair_CreateFromDouble}
 * @see {@link parcJSONPair_CreateFromJSONArray}
 * @see {@link parcJSONPair_CreateFromJSON}
 */
PARCJSONPair *parcJSONPair_Create(const PARCBuffer *name, PARCJSONValue *value);

/**
 * Create a `PARCJSONPair` consisting of the given name and value represented as null-terminated C strings.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value A pointer to a null-terminated C string for the value of the `PARCJSONPair`.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromString("name", "value");
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromString(const char *name, const char *value);

/**
 * Create a `PARCJSONPair` consisting of the given name and `PARCJSONValue`.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value A pointer to a `PARCJSONValue` for the value of the `PARCJSONPair`.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromJSONValue("name", "value");
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromJSONValue(const char *name, PARCJSONValue *value);

/**
 * Create a `PARCJSONPair` consisting of the given name and the null value.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromNULL("name");
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromNULL(const char *name);

/**
 * Create a `PARCJSONPair` consisting of the given name and a JSON boolean of true or false as the value.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value Either `true` or `false`.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromBoolean("name", true);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromBoolean(const char *name, bool value);

/**
 * Create a `PARCJSONPair` consisting of the given name and a JSON Integer.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value An integer value.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromInteger("name", 314159);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromInteger(const char *name, int64_t value);

/**
 * Create a `PARCJSONPair` consisting of the given name and a JSON floating point value.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value A double value.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromDouble(const char *name, double value);

/**
 * Create a `PARCJSONPair` consisting of the given name and a JSON Array.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value A pointer to a {@link PARCJSONArray} instance for the value.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *     PARCJSONPair *pair = parcJSONPair_CreateFromJSONArray("name", array);
 *     parcJSONArray_Release(&array);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromJSONArray(const char *name, PARCJSONArray *value);

/**
 * Create a `PARCJSONPair` consisting of the given name and a JSON Object.
 *
 * @param [in] name A pointer to a null-terminated C string for the name of the `PARCJSONPair`.
 * @param [in] value A pointer to a {@link PARCJSON} instance.
 *
 * @return A pointer to a `PARCJSONPair` instance, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json parcJSON_Create();
 *     PARCJSONPair *pair = parcJSONPair_CreateFromJSON("name", json);
 *     parcJSON_Release(&json);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONPair *parcJSONPair_CreateFromJSON(const char *name, PARCJSON *value);

/**
 * Increase the number of references to a `PARCJSONPair` instance.
 *
 * A new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the acquired reference by invoking {@link parcJSONPair_Release}.
 *
 * @param [in] pair A pointer to a `PARCJSONPair` instance.
 * @return A pointer to the original instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *
 *     PARJSON *reference = parcJSONPair_Acquire(pair);
 *
 *     parcJSONPair_Release(&pair);
 *     parcJSONPair_Release(&reference);
 * }
 * @endcode
 */
PARCJSONPair *parcJSONPair_Acquire(const PARCJSONPair *pair);

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
 * The contents of the deallocated memory used for the PARC object are undefined.
 * Do not reference the object after the last release.
 *
 * @param [in,out] pairPtr A pointer to a pointer to the `PARCJSONPair` instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 */
void parcJSONPair_Release(PARCJSONPair **pairPtr);

/**
 * Get the {@link PARCBuffer} containing the name of the given `PARCJSONPair`.
 *
 * A new reference to the `PARCBuffer` is not created.
 * The caller must create a new reference, if it retains a reference to the buffer.
 *
 * @param [in] pair A pointer to a `PARCJSONPair` instance.
 *
 * @return A pointer to a `PARCBuffer` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *
 *     const char *name = parcJSONPair_GetName(pair);
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCBuffer *parcJSONPair_GetName(const PARCJSONPair *pair);

/**
 * Print a human readable representation of the given `PARCJSONPair`.
 *
 * @param [in] pair A pointer to the instance to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *     parcJSONPair_Display(pair, 0);
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 */
void parcJSONPair_Display(const PARCJSONPair *pair, int indentation);

/**
 * Get the {@link PARCJSONValue} containing the value of the given `PARCJSONPair`.
 *
 * A new reference to the `PARCJSONValue` is not created.
 * The caller must create a new reference, if it retains a reference to the value instance.
 *
 * @param [in] pair A pointer to a `PARCJSONPair` instance.
 *
 * @return A pointer to a `PARCJSONValue` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *pair = parcJSONPair_CreateFromDouble("name", 3.14159);
 *     PARCJSONValue *value = parcJSONPair_GetValue(pair);
 *     parcJSONPair_Release(&pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSONValue *parcJSONPair_GetValue(const PARCJSONPair *pair);

/**
 * Determine if two `PARCJSONPair` instances are equal.
 *
 * Two `PARCJSONPair` instances are equal if, and only if, their names and values are equal.
 *
 * The following equivalence relations on non-null `PARCJSONPair` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcJSONPair_Equals(x, x)`
 *       must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y,
 *     `parcJSONPair_Equals(x, y)` must return true if and only if
 *        `parcJSONPair_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcJSONPair_Equals(x, y)` returns true and
 *        `parcJSONPair_Equals(y, z)` returns true,
 *        then  `parcJSONPair_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple
 *       invocations of `parcJSONPair_Equals(x, y)` consistently return true or
 *       consistently return false.
 *
 *   * For any non-null reference value x, `parcJSONPair_Equals(x, NULL)` must
 *       return false.
 *
 * @param [in] x A pointer to a `PARCJSONPair` instance.
 * @param [in] y A pointer to a `PARCJSONPair` instance.
 * @return true if the two `PARCJSONPair` instances are equal.
 *
 * Example:
 * @code
 * {
 *     PARCJSONPair *a = parcJSONPair_Equals();
 *     PARCJSONPair *b = parcJSONPair_Equals();
 *
 *     if (parcJSONPair_Equals(a, b)) {
 *         // true
 *     } else {
 *         // false
 *     }
 * }
 * @endcode
 */
bool parcJSONPair_Equals(const PARCJSONPair *x, const PARCJSONPair *y);

/**
 * Produce a null-terminated string representation of the specified instance of `PARCJSONPair`.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] pair A pointer to the `PARCJSONPair` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(123456);
 *     PARCJSONPair *instance = parcJSONPair_Create(parcBuffer_Wrap("Hello", 5, 0, 5), value);
 *     parcJSONValue_Release(&value);
 *
 *     char *string = parcJSONPair_ToString(instance);
 *
 *     if (string != NULL) {
 *         printf("%s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcJSONPair_Release(&instance);
 * }
 * @endcode
 *
 * @see parcJSONPair_BuildString
 * @see parcJSONPair_Display
 */
char *parcJSONPair_ToString(const PARCJSONPair *pair);

/**
 * Append a representation of the specified instance to the given
 * {@link PARCBufferComposer}.
 *
 * @param [in] pair A pointer to the `PARCJSONPair` instance.
 * @param [in,out] composer A pointer to the `PARCBufferComposer` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The given `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcJSONPair_BuildString(instance, result);
 *
 *     PARCBuffer *string = parcBufferComposer_FinalizeBuffer(result);
 *     printf("Hello: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcJSONPair_BuildString(const PARCJSONPair *pair, PARCBufferComposer *composer, bool compact);

/**
 * Parse a complete JSON pair
 *
 * A pair consists of a name and a value separated by a colon.
 *
 * @param [in] parser A pointer to a {@link PARCJSONParser} instance.
 *
 * @return non-NULL A pointer to a valid `PARCJSONPair`
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_AllocateCString("\"name\" : \"value\"");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONPair *pair = parcJSONPair_Parser(parser);
 *
 *     parcJSONPair_Release(&pair);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCJSONPair *parcJSONPair_Parser(PARCJSONParser *parser);
#endif // libparc_parc_JSONPair_h
