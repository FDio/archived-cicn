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
 * @file parc_JSONArray.h
 * @brief A JSON Array stores an array of JSON objects.
 * @ingroup inputoutput
 *
 */
#ifndef libparc_parc_JSONArray_h
#define libparc_parc_JSONArray_h

struct parcJSONArray;
typedef struct parcJSONArray PARCJSONArray;

#include <parc/algol/parc_JSONValue.h>

/**
 * Create an empty `PARCJSONArray` instance.
 *
 * @return A pointer to an empty `PARCJSONArray` instance.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *x = parcJSONArray_Create();
 *     parcJSONArray_Release(&x);
 * }
 * @endcode
 *
 * @see parcJSONArray_AddValue
 */
PARCJSONArray *parcJSONArray_Create(void);

/**
 * Increase the number of references to a `PARCJSONArray`.
 *
 * Note that new `PARCJSONArray` is not created,
 * only that the given `PARCJSONArray` reference count is incremented.
 * Discard the reference by invoking {@link parcJSONArray_Release}.
 *
 * @param [in] array A pointer to a `PARCJSONArray` instance.
 *
 * @return The input `PARCJSONArray` pointer.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *x = parcJSONArray_Create();
 *
 *     PARCJSONArray *x_2 = parcJSONArray_Acquire(x);
 *
 *     parcJSONArray_Release(&x);
 *     parcJSONArray_Release(&x_2);
 * }
 * @endcode
 */
PARCJSONArray *parcJSONArray_Acquire(const PARCJSONArray *array);

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
 * @param [in,out] arrayPtr A pointer to a pointer to the instance of `PARCJSONArray` to release.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *x = parcJSONArray_Create();
 *
 *     parcJSONArray_Release(&x);
 * }
 * @endcode
 */
void parcJSONArray_Release(PARCJSONArray **arrayPtr);

/**
 * Assert that an instance of `PARCJSONArray` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] array A pointer to a `PARCJSONArray` instance.
 */
void parcJSONArray_AssertValid(const PARCJSONArray *array);

/**
 * Determine if two `PARCJSONArray` instances are equal.
 *
 * The following equivalence relations on non-null `PARCJSONArray` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcJSONArray_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcJSONArray_Equals(x, y)` must return true if and only if
 *        `parcJSONArray_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcJSONArray_Equals(x, y)` returns true and
 *        `parcJSONArray_Equals(y, z)` returns true,
 *        then `parcJSONArray_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcJSONArray_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcJSONArray_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a `PARCJSONArray` instance.
 * @param [in] y A pointer to a `PARCJSONArray` instance.
 *
 * @return true `PARCJSONArray` x and y are equal.
 * @return false `PARCJSONArray` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *x = parcJSONArray_Create();
 *     PARCJSONArray *y = parcJSONArray_Create();
 *
 *     if (parcJSONArray_Equals(x, y)) {
 *         printf("Arrays are equal.\n");
 *     }  else {
 *         printf("Arrays are NOT equal.\n");
 *     }
 *
 *     parcJSONArray_Release(&x);
 *     parcJSONArray_Release(&y);
 * }
 * @endcode
 */
bool parcJSONArray_Equals(const PARCJSONArray *x, const PARCJSONArray *y);

/**
 * Add {@link PARCJSONValue} instance to the given `PARCJSONArray`.
 *
 * A new reference to the `PARCJSONValue` is acquired by this call.
 *
 * @param [in,out] array A pointer to a `PARCJSONArray` instance.
 * @param [in] value A pointer to a `PARCJSONValue` instance.
 *
 * @return A pointer to the @p array.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     parcJSONArray_AddValue(array, value);
 *     parcJSONValue_Release(&value);
 *
 *     parcJSONArray_Release(&array);
 * }
 * @endcode
 *
 */
PARCJSONArray *parcJSONArray_AddValue(PARCJSONArray *array, PARCJSONValue *value);

/**
 * Get the length of the given `PARCJSONArray` instance.
 *
 * @param [in] array A pointer to a `PARCJSONArray` instance.
 *
 * @return The number of elements in the array.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     parcJSONArray_AddValue(array, value);
 *     parcJSONValue_Release(&value);
 *
 *     parcJSONValue_GetLength(array);
 *
 *     parcJSONArray_Release(&array);
 * }
 * @endcode
 */
size_t parcJSONArray_GetLength(const PARCJSONArray *array);

/**
 * Get the {@link PARCJSONValue} stored at the given index in the `PARCJSONArray`.
 * A new reference is not acquired.
 * The caller must acquire its own reference if needed.
 *
 * @param [in] array A pointer to a `PARCJSONArray` instance.
 * @param [in] index The index of the `PARCJSONValue` to get.
 *
 * @return The pointer to the requested `PARCJSONValue`, or NULL if the index exceeded the length of the array.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     parcJSONArray_AddValue(array, value);
 *     parcJSONValue_Release(&value);
 *
 *     PARCJSONValue *actualValue = parcJSONValue_GetValue(array, 0);
 *
 *     parcJSONArray_Release(&array);
 * }
 * @endcode
 */
PARCJSONValue *parcJSONArray_GetValue(const PARCJSONArray *array, size_t index);

/**
 * Append a representation of the specified `PARCJSONArray` instance to the given
 * {@link PARCBufferComposer}.
 *
 * @param [in] array A pointer to the `PARCJSONArray` instance.
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
 *     parcJSONArray_BuildString(instance, result);
 *
 *     PARCBuffer *string = parcBufferComposer_FinalizeBuffer(result);
 *     printf("JSON Array: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcJSONArray_BuildString(const PARCJSONArray *array, PARCBufferComposer *composer, bool compact);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] array A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     parcJSONArray_AddValue(array, value);
 *     parcJSONValue_Release(&value);
 *
 *     const char *string = parcJSONValue_ToString(array);
 *
 *     parcMemory_Deallocate((void **) &string);
 *
 *     parcJSONArray_Release(&array);
 * }
 * @endcode
 *
 * @see parcJSONArray_Display
 */
char *parcJSONArray_ToString(const PARCJSONArray *array);

/**
 * Produce a null-terminated compact string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] array A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSONArray_Create();
 *
 *     PARCJSONValue *value = parcJSONValue_CreateFromInteger(31415);
 *     parcJSONArray_AddValue(array, value);
 *     parcJSONValue_Release(&value);
 *
 *     const char *string = parcJSONValue_ToCompactString(array);
 *
 *     parcMemory_Deallocate((void **) &string);
 *
 *     parcJSONArray_Release(&array);
 * }
 * @endcode
 *
 * @see parcJSONArray_Display
 */
char *parcJSONArray_ToCompactString(const PARCJSONArray *array);

/**
 * Pretty print the given `PARCJSONArray` instance.
 *
 * @param [in] array The `PARCJSONArray` instance to be printed.
 * @param [in] indentation The amount of indentation to prefix each line of output
 *
 * Example:
 * @code
 * {
 *     PARCJSONArray *array = parcJSON_ParseString("{ \"string\" : \"xyzzy\" }");
 *     parcJSONArray_Display(array, 0);
 * }
 * @endcode
 */
void parcJSONArray_Display(const PARCJSONArray *array, int indentation);
#endif // libparc_parc_JSONArray_h
