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
 * @brief A complete JSON encoding and decoding library.
 *
 * # Parsing #
 * Parse a null-terminated C string containing JSON via {@link parcJSON_ParseString}.
 *
 * # Printing #
 * Print a JSON object via {@link parcJSON_ToString}.
 *
 * # Composing #
 * Compose JSON objects via {@link parcJSON_Create} and add members via {@link parcJSON_Add}.
 * Compose members as JSON Pairs consisting of a name and value.  See functions named `parcJSONPair_Create*`
 *
 */
#ifndef libparc_parc_JSON_h
#define libparc_parc_JSON_h

#include <stdbool.h>

struct parc_json;
typedef struct parc_json PARCJSON;

#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_HashCode.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_PathName.h>

#include <parc/algol/parc_List.h>
#include <parc/algol/parc_JSONPair.h>
#include <parc/algol/parc_JSONValue.h>

/**
 * Create a new JSON object.
 *
 * The JSON new object contains no members.
 *
 * @return A pointer to a `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *     ...
 *     parcJSONValue_Release(&json);
 * }
 * @endcode
 *
 * @see {@link parcJSON_Add}
 */
PARCJSON *parcJSON_Create(void);

/**
 * Create a deep copy of a JSON object. Call parcJSON_Release to free the object when done with it.
 *
 * @return A pointer to a `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *jsonSrc = parcJSON_Create();
 *     ...
 *     PARCJSON *jsonCopy = parcJSON_Copy(jsonSrc);
 *     ...
 *     parcJSONValue_Release(&jsonSrc);
 *     ...
 *     parcJSONValue_Release(&jsonCopy);
 * }
 * @endcode
 *
 */
PARCJSON *parcJSON_Copy(const PARCJSON *src);

/**
 * Increase the number of references to a `PARCJSON` instance.
 *
 * Note that a new `PARCJSON` is not created,
 * only that the given `PARCJSON` reference count is incremented.
 * Discard the reference by invoking {@link parcJSON_Release}.
 *
 * @param [in] json A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *x = parcJSON_Create();
 *
 *     PARCJSON *x2 = parcJSON_Acquire(x);
 *
 *     parcJSON_Release(&x);
 *     parcJSON_Release(&x2);
 * }
 * @endcode
 *
 * @see {@link parcJSON_Release}
 */
PARCJSON *parcJSON_Acquire(const PARCJSON *json);

/**
 * Determine if two `PARCJSON` instances are equal.
 *
 * Two `PARCJSON` instances are equal if, and only if,
 * they contain the equal members, in the same order.
 *
 * The following equivalence relations on non-null `PARCJSON` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcJSON_Equals(x, x)`
 *       must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y,
 *     `parcJSON_Equals(x, y)` must return true if and only if
 *        `parcJSON_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcJSON_Equals(x, y)` returns true and
 *        `parcJSON_Equals(y, z)` returns true,
 *        then  `parcJSON_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple
 *       invocations of `parcJSON_Equals(x, y)` consistently return true or
 *       consistently return false.
 *
 *   * For any non-null reference value x, `parcJSON_Equals(x, NULL)` must
 *       return false.
 *
 * @param [in] x A pointer to a `PARCJSON` instance.
 * @param [in] y A pointer to a `PARCJSON` instance.
 * @return true if the two `PARCJSON` instances are equal.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *a = parcJSON_Create();
 *     PARCJSON *b = parcJSON_Create();
 *
 *     if (parcJSON_Equals(a, b)) {
 *         // true
 *     } else {
 *         // false
 *     }
 * }
 * @endcode
 */
bool parcJSON_Equals(const PARCJSON *x, const PARCJSON *y);

/**
 * Add a JSON Pair to the members of a JSON Object.
 *
 * @param [in,out] json A pointer to a `PARCJSON` instance.
 * @param [in] pair A pointer to a `PARCJSONPair` instance.
 *
 * @return The pointer to the `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *
 *     PARCJSONPair *pair = parcJSONPair_CreateFromInteger("pi", 314159);
 *     parcJSON_AddPair(json, pair);
 * }
 * @endcode
 *
 * @see parcJSONPair_Create
 */
PARCJSON *parcJSON_AddPair(PARCJSON *json, PARCJSONPair *pair);

/**
 * Pretty print the given `PARCJSON` instance.
 *
 * @param [in] json The `PARCJSON` instance to be printed.
 * @param [in] indentation The amount of indentation to prefix each line of output
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
 *     parcJSON_Display(json, 0);
 * }
 * @endcode
 */
void parcJSON_Display(const PARCJSON *json, int indentation);

/**
 * Get the list of members of the given `PARCJSON` instance.
 *
 * A new reference to the {@link PARCList} is not created.
 * The caller must create a new reference, if it retains a reference to the buffer.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @return A pointer to a `PARCList` instance containing the members.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
 *     PARCList *members = parcJSON_GetMembers(json);
 * }
 * @endcode
 */
PARCList *parcJSON_GetMembers(const PARCJSON *json);

/**
 * Get the PARCJSONPair at the index in the given `PARCJSON` instance.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @param [in] index The index value of the desired element.
 * @return A pointer to a `PARCJSONPair` instance containing or NULL if there is nothing at the specified index.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
 *     PARCJSONPair *pair = parcJSON_GetPairByIndex(json, 0);
 * }
 * @endcode
 */
PARCJSONPair *parcJSON_GetPairByIndex(const PARCJSON *json, size_t index);

/**
 * Get the PARCJSONValue at the index in the given `PARCJSON` instance.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @param [in] index The index value of the desired element.
 * @return A pointer to a `PARCJSONValue` instance containing or NULL if there is nothing at the specified index.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
 *     PARCJSONValue *pair = parcJSON_GetValueByIndex(json, 0);
 * }
 * @endcode
 */
PARCJSONValue *parcJSON_GetValueByIndex(const PARCJSON *json, size_t index);

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
 * @param [in,out] jsonPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
void parcJSON_Release(PARCJSON **jsonPtr);

/**
 * Parse a null-terminated C string into a `PARCJSON` instance.
 *
 * Only 8-bit characters are parsed.
 *
 * @param [in] string A null-terminated C string containing a well-formed JSON object.
 *
 * @return A pointer to a `PARCJSON` instance with one reference, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"key\" : 1, \"array\" : [1, 2, 3] }");
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
PARCJSON *parcJSON_ParseString(const char *string);

/**
 * Parse a null-terminated C string into a `PARCJSON` instance.
 *
 * Only 8-bit characters are parsed.
 *
 * @param [in] buffer A pointer to a valid PARCBuffer instance.
 *
 * @return A pointer to a `PARCJSON` instance with one reference, or NULL if an error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBufer *buffer = parcBuffer_WrapCString("{ \"key\" : 1, \"array\" : [1, 2, 3] }");
 *     PARCJSON *json = parcJSON_ParseBuffer(buffer);
 *
 *     parcBuffer_Release(&buffer);
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
PARCJSON *parcJSON_ParseBuffer(PARCBuffer *buffer);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] json A pointer to the `PARCJSON` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *
 *     char *string = parcJSON_ToString(json);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 * @see {@link parcJSONPair_BuildString}
 * @see {@link parcJSONPair_Display}
 */
char *parcJSON_ToString(const PARCJSON *json);

/**
 * Produce a null-terminated compact (minimally escaped and formated) string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] json A pointer to the `PARCJSON` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *
 *     char *string = parcJSON_ToCompactString(json);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 * @see {@link parcJSONPair_BuildString}
 * @see {@link parcJSONPair_Display}
 */
char *parcJSON_ToCompactString(const PARCJSON *json);

/**
 * Produce a PARCHashCode for the JSON object.
 *
 * @param [in] json A pointer to the `PARCJSON` instance.
 *
 * @return PARCHashCode The object's hash-code.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_Create();
 *     ...
 *     PARCHashCode hashCode = parcJSON_HashCode(json);
 *     ....
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 */
PARCHashCode parcJSON_HashCode(const PARCJSON *json);

/**
 * Get the {@link PARCJSONPair} with the given key name.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @param [in] name A null-terminated C string containing the name of the pair to return.
 *
 * @return A pointer to the named `PARCJSONPair`.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"key\" : 1, "array" : [1, 2, 3] }");
 *
 *     PARCJSONPair *arrayPair = parcJSON_GetPairByName(json, "array");
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 * @see parcJSON_Add
 */
const PARCJSONPair *parcJSON_GetPairByName(const PARCJSON *json, const char *name);

/**
 * Get the {@link PARCJSONValue} with the given key name.
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @param [in] name A null-terminated C string containing the name of the pair to return.
 *
 * @return A pointer to the named `PARCJSONValue`.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"key\" : 1, "array" : [1, 2, 3] }");
 *
 *     PARCJSONValue *arrayValue = parcJSON_GetValueByName(json, "array");
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 *
 * @see parcJSON_Add
 */
PARCJSONValue *parcJSON_GetValueByName(const PARCJSON *json, const char *name);

/**
 * Get the JSON pair named by the given '/' separated path name.
 *
 * Using a '/' separated list of JSON pair names, return the {@link PARCJSONPair} named by the path.
 * This function currently returns the Value, not the Pair specified by the `PARCPathName`
 *
 * @param [in] json A pointer to a `PARCJSON` instance.
 * @param [in] path A pointer to a null-terminated C string containing the full path of the `PARCJSONPair`.
 *
 * @return A pointer to the {@link PARCJSONValue} named by the path.
 *
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"key\" : 1, "array" : [1, 2, 3] }");
 *
 *     PARCJSONValue *key = parcJSON_GetByPath(json, "/key");
 *
 *     PARCJSONValue *array_1 = parcJSON_GetByPath(json, "/array/1");
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
const PARCJSONValue *parcJSON_GetByPath(const PARCJSON *json, const char *path);

/**
 * Get the JSON pair named by the given {@link PARCPathName}.
 * This function currently returns the Value, not the Pair specified by the `PARCPathName`
 *
 * @param [in] pathNode A pointer to a {@link PARCJSONValue} instance.
 * @param [in] pathName A pointer to valid `PARCPathName` instance.
 *
 * @return A pointer to the `PARCJSONValue` named by the path.
 * Example:
 * @code
 * {
 *     PARCJSON *json = parcJSON_ParseString("{ \"key\" : 1, "array" : [1, 2, 3] }");
 *
 *     PARCPathName *keyPath = parcPathName_Create("/key");
 *     PARCPathName *array1Path = parcPathName_Create("/array/1");
 *
 *     PARCJSONValue *key = parcJSON_GetByPathName(json, keyPath);
 *
 *     PARCJSONValue *array_1 = parcJSON_GetByPathName(json, array1Path);
 *
 *     parcJSON_Release(&json);
 * }
 * @endcode
 */
const PARCJSONValue *parcJSON_GetByPathName(const PARCJSONValue *pathNode, const PARCPathName *pathName);

/**
 * Append a representation of the specified {@link PARCJSON} instance to the given {@link PARCBufferComposer}.
 *
 * @param [in] json A pointer to the `PARCJSON` instance.
 * @param [in,out] composer A `PARCBufferComposer` to append to this URI segment.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The given `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcJSON_BuildString(instance, result);
 *
 *     PARCBuffer *string = parcBufferComposer_FinalizeBuffer(result);
 *     printf("JSON String: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcJSON_BuildString(const PARCJSON *json, PARCBufferComposer *composer, bool compact);

/**
 * Create and add a JSON string pair to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value A pointer to a nul-terminated C string containing the value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddString(PARCJSON *json, const char *name, const char *value);

/**
 * Create and add a JSON object pair to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value A pointer to a valid `PARCJON` value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddObject(PARCJSON *json, const char *name, PARCJSON *value);

/**
 * Create and add a pair with an array for the value to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] array A pointer to a valid `PARCJONArray` value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddArray(PARCJSON *json, const char *name, PARCJSONArray *array);

/**
 * Create and add a pair with a PARCJSONValue for the value to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value A pointer to a valid `PARCJONValue` value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddValue(PARCJSON *json, const char *name, PARCJSONValue *value);

/**
 * Create and add an integer pair to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value An integer value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddInteger(PARCJSON *json, const char *name, int64_t value);

/**
 * Create and add a boolean pair to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value An boolean value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddBoolean(PARCJSON *json, const char *name, bool value);

/**
 * Create and add a boolean pair to a PARCJSON object.
 *
 * @param [in] json A pointer to a valid `PARCJSON` instance.
 * @param [in] name A pointer to a nul-terminated C string containing the name of the pair.
 * @param [in] value An boolean value.
 *
 * @return A pointer to the updated `PARCJSON` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *parcJSON_AddArray(PARCJSON *json, const char *name, PARCJSONArray *value);
#endif // libparc_parc_JSON_h
