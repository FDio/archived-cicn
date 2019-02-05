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
 * @brief A JSON value. One of a Boolean, String, Number, JSON Array, JSON Object, or Null.
 *
 */
#ifndef libparc_parc_JSONValue_h
#define libparc_parc_JSONValue_h

#include <stdbool.h>
#include <stdint.h>

struct parc_json_value;
typedef struct parc_json_value PARCJSONValue;

#include <parc/algol/parc_Object.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_JSONParser.h>
#include <parc/algol/parc_JSONPair.h>
#include <parc/algol/parc_JSONArray.h>

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_List.h>

/**
 * @def parcJSONValue_OptionalAssertValid
 * Optional validation of the given instance.
 *
 * Define `PARCLibrary_DISABLE_VALIDATION` to nullify validation.
 */
#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcJSONValue_OptionalAssertValid(_instance_)
#else
#  define parcJSONValue_OptionalAssertValid(_instance_) parcJSONValue_AssertValid(_instance_)
#endif

/**
 * Print a human readable representation of the given `PARCJSONValue`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] value A pointer to the instance of `PARCJSONValue` to display.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *instance = parcJSONValue_CreateFromNull();
 *
 *     parcJSONValue_Display(instance, 0);
 *
 *     parcJSONValue_Release(&instance);
 * }
 * @endcode
 *
 */
void parcJSONValue_Display(const PARCJSONValue *value, int indentation);

/**
 * Determine if @p value is a JSON Null.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON Null.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromNULL();
 *
 *     if (parcJSONValue_IsNull(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_CreateFromNULL
 */
bool parcJSONValue_IsNull(const PARCJSONValue *value);

/**
 * Determine if @p value is a JSON Boolean.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON Boolean.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromBoolean(true);
 *
 *     if (parcJSONValue_IsBoolean(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_CreateFromNULL
 */
bool parcJSONValue_IsBoolean(const PARCJSONValue *value);

/**
 * Determine if @p value is a JSON Number.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON Number.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromFloat(3.14);
 *
 *     if (parcJSONValue_IsNumber(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_GetInteger
 * @see parcJSONValue_GetFloat
 */
bool parcJSONValue_IsNumber(const PARCJSONValue *value);

/**
 * Determine if the pointer to a `PARCJSONValue` is valid.
 *
 * @param [in] value A pointer to a `PARCJSONValue`.
 *
 * @return true The `PARCJSONValue` is valid.
 * @return false The `PARCJSONValue` is invalid.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create());
 *     PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
 *     parcJSON_Release(&json);
 *
 *     if (parcJSONValue_IsValid(value)) {
 *         printf("Valid!\n");
 *     } else {
 *         printf("Invalid!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
bool parcJSONValue_IsValid(const PARCJSONValue *value);

/**
 * Assert that an instance of `PARCJSONValue` is valid.
 *
 * If the instance is not valid, terminate via {@link parcTrapIllegalValue()}
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] value A pointer to a `PARCJSONValue` instance.
 */
void parcJSONValue_AssertValid(const PARCJSONValue *value);

/**
 * Determine if @p value is a JSON Object.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON Object.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create());
 *     PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
 *     parcJSON_Release(&json);
 *
 *     if (parcJSONValue_IsObject(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_CreateFromJSON
 */
bool parcJSONValue_IsJSON(const PARCJSONValue *value);

/**
 * Determine if @p value is a JSON String.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON String.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *string = parcBuffer_Wrap("Hello", 0, 5);
 *     PARCJSONValue *value = parcJSONValue_CreateFromString(string);
 *     parcBuffer_Release(&string);
 *
 *     if (parcJSONValue_IsString(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_CreateFromString
 */
bool parcJSONValue_IsString(const PARCJSONValue *value);

/**
 * Determine if @p value is a JSON Array.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return True if @p value is a JSON Array.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *     PARCJSONValue *value = parcJSONValue_CreateFromJSONArray(array);
 *     parcJSONArray_Release(&array);
 *
 *     if (parcJSONValue_IsArray(value)) {
 *         printf("Success!\n");
 *     }
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 * @see parcJSONValue_CreateFromJSONArray
 */
bool parcJSONValue_IsArray(const PARCJSONValue *value);

/**
 * Get the value of the given `PARCJSONValue` as a {@link PARCJSONArray} instance.
 *
 * The value must be a JSON Array.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return A pointer to a `PARCJSONArray` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" [ ]");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     PARCJSONArray *array = parcJSONValue_GetArray(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *    parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_IsArray
 */
PARCJSONArray *parcJSONValue_GetArray(const PARCJSONValue *value);

/**
 * Get the given value is a `bool`
 *
 * The value must be a JSON Array of the type {@link PARCJSONValueType_Boolean}
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return A `bool` representation.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" true");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     bool result = parcJSONValue_GetBoolean(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see {link parcJSONValue_IsBoolean}
 */
bool parcJSONValue_GetBoolean(const PARCJSONValue *value);

/**
 * Get the JSON float value is a `long double`
 *
 *
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return The value as a C double.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" 3.1415");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     long double result = parcJSONValue_GetFloat(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_IsNumber
 */
long double parcJSONValue_GetFloat(const PARCJSONValue *value);

/**
 * Get the given value as an integer.
 *
 * The value must be a JSON Number value.
 * The result must be expressible in a 64-bit integer (`int64_t`).
 * Overflow is not detected.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return The value as a C `int64_t`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" 31415");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     int64_t result = parcJSONValue_GetInteger(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_IsNumber
 */
int64_t parcJSONValue_GetInteger(const PARCJSONValue *value);

/**
 * Get the given value is a null-terminated C string.
 *
 * The value must be a JSON Array of the type {@link PARCJSONValueType_String}
 *
 * A new reference to the return value is not created.
 * The caller must create a new reference, if it retains a reference to the buffer.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return A pointer to a {@link PARCBuffer} instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" \"a string\"");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     PARCBuffer *result = parcJSONValue_GetString(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_IsString
 */
PARCBuffer *parcJSONValue_GetString(const PARCJSONValue *value);

/**
 * Get the value of a JSON object.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 *
 * @return The value as a pointer to a {@link PARCJSON} instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : \"a string\" }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     PARCJSON *result = parcJSONValue_GetJSON(value);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_IsJSON
 */
PARCJSON *parcJSONValue_GetJSON(const PARCJSONValue *value);

/**
 * Convenience function to fill a timeval struct from a PARCJSONValue.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 * @param [out] timeval A pre-allocated timeval struct to fill.
 *
 * @return A pointer to the filled timeval struct.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"seconds\" : 10, \"micros\" : 0 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     parcBuffer_Release(&buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *     parcJSONParser_Release(&parser);
 *
 *     struct timeval timeval;
 *     parcJSONValue_GetTimeval(value, &timeval);
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 */
struct timeval *parcJSONValue_GetTimeval(const PARCJSONValue *value, struct timeval *timeval);

/**
 * Convenience function to fill a timespec struct from a PARCJSONValue.
 *
 * @param [in] value A pointer to a `JSONValue` instance.
 * @param [out] timespec A pre-allocated timespec struct to fill.
 *
 * @return A pointer to the filled timespec struct.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"seconds\" : 10, \"nanos\" : 0 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     parcBuffer_Release(&buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *     parcJSONParser_Release(&parser);
 *
 *     struct timespec timespec;
 *     parcJSONValue_GetTimespec(value, &timespec);
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 */
struct timespec *parcJSONValue_GetTimespec(const PARCJSONValue *value, struct timespec *timespec);

/**
 * Create a NULL JSON value.
 *
 * @return A pointer to the new `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromNULL();
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 *
 */
PARCJSONValue *parcJSONValue_CreateFromNULL(void);

/**
 * Create a boolean value.
 *
 * @param [in] value Either `true` or `false.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromBoolean(true);
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromBoolean(bool value);

/**
 * Create a float value.
 *
 * The resulting PARCJSONValue is a number (see {@link parcJSONValue_IsNumber}).
 *
 * @param [in] value A `long double` value.
 * @return A pointer to the new `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromFloat(3.14);
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromFloat(long double value);

/**
 * Create a JSON integer value.
 *
 * The resulting PARCJSONValue is a number (see {@link parcJSONValue_IsNumber} ).
 *
 * @param [in] integer An `int64_t` value.
 * @return A pointer to the new `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(3);
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromInteger(int64_t integer);

/**
 * Create a string value from the contents of a {@link PARCBuffer}.
 *
 * @param [in] string A pointer to a `PARCBuffer` instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *string = parcBuffer_AllocateCString("hello"):
 *     PARCJSONValue *value = parcJSONValue_CreateFromString(string);
 *
 *     parcJSONValue_Release(&value);
 *     parcBuffer_Release(&string);
 * }
 * @endcode
 * @see parcJSONValue_CreateFromCString
 */
PARCJSONValue *parcJSONValue_CreateFromString(PARCBuffer *string);

/**
 * Create a string value from a null-terminated C string.
 *
 * @param [in] string A pointer to a {@link PARCBuffer} instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *value = parcJSONValue_CreateFromCString("hello");
 *
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 * @see parcJSONValue_CreateFromString
 */
PARCJSONValue *parcJSONValue_CreateFromCString(const char *string);

/**
 * Create a JSON Array value.
 *
 * @param [in] array A pointer to a {@link PARCJSONArray} instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *     PARCJSONValue *value = parcJSONValue_CreateFromPARCArray(array);
 *
 *     PARCJSONArray_Release(&array);
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromJSONArray(PARCJSONArray *array);

/**
 * Create a JSON Value containing a JSON Object.
 *
 * A new reference to the given {@link PARCJSON} instance is acquired.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *     PARCJSONValue *value = parcJSONValue_CreateFromJSON(json);
 *
 *     parcJSON_Release(&json);
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromJSON(PARCJSON *json);

/**
 * A convenience function to create a JSON Value containing a JSON Object representing a timeval.
 *
 * A new reference to the given {@link PARCJSON} instance is acquired. The json keys for the
 * timespec fields are are "seconds" & "micros".
 *
 * @param [in] timeval A pointer to a timeval instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     struct timeval timeval = { .tv_sec = 1, .tv_usec = 1 };
 *     PARCJSONValue *value = parcJSONValue_CreateFromTimeval(&timeval);
 *
 *     parcJSON_Release(&json);
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromTimeval(const struct timeval *timeval);

/**
 * A convenience function to create a JSON Value containing a JSON Object representing a timespec.
 *
 * A new reference to the given {@link PARCJSON} instance is acquired. The json keys for the
 * timespec fields are are "seconds" & "nanos".
 *
 * @param [in] timeval A pointer to a timespec instance.
 * @return A pointer to the new  `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     struct timespec timespec = { .tv_sec = 10, .tv_nsec = 0 };
 *     PARCJSONValue *value = parcJSONValue_CreateFromTimespec(&timespec);
 *
 *     parcJSON_Release(&json);
 *     parcJSONValue_Release(&value);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_CreateFromTimespec(const struct timespec *timespec);

/**
 * Increase the number of references to a `PARCJSONValue`.
 *
 * Note that new `PARCJSONValue` is not created,
 * only that the given `PARCJSONValue` reference count is incremented.
 * Discard the reference by invoking {@link parcJSONValue_Release}.
 *
 * @param [in] value A pointer to the original instance.
 *
 * @return The value of the input parameter @p value.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *x = parcJSONValue_CreateFromNull();
 *
 *     PARCJSONValue *x2 = parcJSONValue_Acquire(x);
 *
 *     parcJSONValue_Release(&x);
 *     parcJSONValue_Release(&x2);
 * }
 * @endcode
 *
 * @see parcJSONValue_Release
 */
PARCJSONValue *parcJSONValue_Acquire(const PARCJSONValue *value);

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
 * @param [in,out] valuePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *x = parcJSONValue_CreateFromNull();
 *
 *     parcJSONValue_Release(&x);
 * }
 * @endcode
 */
void parcJSONValue_Release(PARCJSONValue **valuePtr);

/**
 * Determine if two `PARCJSONValue` instances are equal.
 *
 *
 * The following equivalence relations on non-null `PARCJSONValue` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcJSONValue_Equals(x, x)`
 *       must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y,
 *     `parcJSONValue_Equals(x, y)` must return true if and only if
 *        `parcJSONValue_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcJSONValue_Equals(x, y)` returns true and
 *        `parcJSONValue_Equals(y, z)` returns true,
 *        then  `parcJSONValue_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple
 *       invocations of `parcJSONValue_Equals(x, y)` consistently return true or
 *       consistently return false.
 *
 *   * For any non-null reference value x, `parcJSONValue_Equals(x, NULL)` must
 *       return false.
 *
 * @param [in] x A pointer to a `PARCJSONValue` instance.
 * @param [in] y A pointer to a `PARCJSONValue` instance.
 * @return true if the two `PARCJSONValue` instances are equal.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *x = parcJSONValue_CreateFromBoolean(true);
 *     PARCJSONValue *y = parcJSONValue_CreateFromBoolean(true);
 *
 *     if (parcJSONValue_Equals(x, y)) {
 *         // true
 *     } else {
 *         // false
 *     }
 * }
 * @endcode
 */
bool parcJSONValue_Equals(const PARCJSONValue *x, const PARCJSONValue *y);

/**
 * Produce a null-terminated string representation of the specified instance of `PARCJSONValue`.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] value A pointer to the `PARCJSONValue` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *instance = parcJSONValue_CreateFromString("Hello World");
 *
 *     char *string = parcJSONValue_ToString(instance);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcJSONValue_Release(&instance);
 * }
 * @endcode
 *
 * @see parcJSONValue_BuildString
 * @see parcJSONValue_Display
 */
char *parcJSONValue_ToString(const PARCJSONValue *value);

/**
 * Produce a null-terminated "compact" (minimal escaping and formatting) string representation
 * of the specified instance of `PARCJSONValue`.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] value A pointer to the `PARCJSONValue` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSONValue *instance = parcJSONValue_CreateFromString("Hello World");
 *
 *     char *string = parcJSONValue_ToCompactString(instance);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcJSONValue_Release(&instance);
 * }
 * @endcode
 *
 * @see parcJSONValue_BuildString
 * @see parcJSONValue_Display
 */
char *parcJSONValue_ToCompactString(const PARCJSONValue *value);

/**
 * Append a representation of the specified instance to the given
 * {@link PARCBufferComposer}.
 *
 * @param [in] value A pointer to a {@link PARCJSONParser} instance.
 * @param [in,out] composer A pointer to the {@link PARCBufferComposer} instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The given `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcJSONValue_BuildString(instance, result);
 *
 *     PARCBuffer *string = parcBufferComposer_FinalizeBuffer(result);
 *     printf("JSON Value: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcJSONValue_BuildString(const PARCJSONValue *value, PARCBufferComposer *composer, bool compact);

/**
 * Parse an arbitrary JSON value.
 *
 * The value may be any valid JSON value, consisting of strings, numbers, objects, arrays, `true`, `false`, or `null`
 * in their properly encoded string format.
 *
 * @param [in] parser A pointer to a {@link PARCJSONParser} instance.
 *
 * @return non-NULL A pointer to a valid `PARCJSONValue` instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" 123");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_Parser(parser);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 *
 * @see parcJSONValue_NumberParser
 */
PARCJSONValue *parcJSONValue_Parser(PARCJSONParser *parser);


/**
 * Parse a JSON Object value.
 *
 * @param [in] parser A pointer to a {@link PARCJSONParser} instance.
 *
 * @return non-NULL A pointer to a valid `PARCJSONValue` instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_WrapCString(" { \"name\" : 123 }");
 *
 *     PARCJSONParser *parser = parcJSONParser_Create(buffer);
 *     PARCJSONValue *value = parcJSONValue_ObjectParser(parser);
 *
 *     parcJSONValue_Release(&value);
 *     parcJSONParser_Release(&parser);
 *     parcBuffer_Release(&buffer);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONValue_ObjectParser(PARCJSONParser *parser);
#endif // libparc_parc_JSONValue_h
