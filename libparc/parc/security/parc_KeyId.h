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
 * @file parc_KeyId.h
 * @ingroup security
 * @brief Represent a key by an octet string
 *
 * A KeyId is a hash digest used to identify a key. These are used as key entries in hash-table
 * based key stores that cache raw keys. Instead of transferring raw keys, parties may exchange
 * KeyIds used to index into key stores for constant-time key retrieval. This exchange
 * expects that the raw key will be present in the key store. If not, the lookup will fail.
 * Consequently, KeyIds are not used to encapsulate or transfer raw keys.
 *
 */
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_BufferComposer.h>

#ifndef libparc_parc_KeyId_h
#define libparc_parc_KeyId_h

struct parc_keyid;
/**
 * @typedef PARCKeyId
 * @brief A KeyId is a hash digest used to identify a key.
 */

typedef struct parc_keyid PARCKeyId;

/**
 * @def parcKeyId_OptionalAssertValid
 * Optional validation of the given instance.
 *
 * Define `PARCLibrary_DISABLE_VALIDATION` to disable validation.
 */
#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcKeyId_OptionalAssertValid(_instance_)
#else
#  define parcKeyId_OptionalAssertValid(_instance_) parcKeyId_AssertValid(_instance_)
#endif

/**
 * Create a `PARCKeyId` from the given pre-computed-key identifier.
 *
 * A reference to the given identifer is created and the caller is responsible for releasing the remaining references.
 *
 * @param [in] preComputedKeyId A pointer to a `PARCBuffer` instance containing the pre-computed-key identifier.
 * @return A pointer to an allocated `PARCKeyId` that must be released via `parcKeyId_Release`.
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *keybits = parcBuffer_Wrap("Hello", 5, 0, 5);
 *      PARCKeyId *keyid = parcKeyId_Create(keybits);
 *      parcBuffer_Release(&keybits);
 *      // do something with keyid
 *      parcKeyId_Release(&keyid);
 * }
 * @endcode
 */
PARCKeyId *parcKeyId_Create(PARCBuffer *preComputedKeyId);

/**
 * Increase the number of references to a `PARCKeyId`.
 *
 * Note that new `PARCKeyId` is not created,
 * only that the given `PARCKeyId` reference count is incremented.
 * Discard the reference by invoking `parcKeyId_Release`.
 *
 * @param [in] instance A pointer to the instance of `PARCKeyId`.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Wrap("Hello", 5, 0, 5);
 *     PARCKeyId *instance = parcKeyId_CreateFromByteBuffer(buffer);
 *     parcBuffer_Release(&buffer);
 *
 *     PARCKeyId *reference = parcKeyId_Acquire(instance);
 *
 *     parcKeyId_Release(&instance);
 *     parcKeyId_Release(&reference);
 * }
 * @endcode
 *
 * @see parcKeyId
 */
PARCKeyId *parcKeyId_Acquire(const PARCKeyId *instance);

/**
 * Assert a valid PARCKeyId instance.
 *
 * @param [in] keyId A pointer to a `PARCKeyId` instance.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Wrap("Hello", 5, 0, 5);
 *     PARCKeyId *instance = parcKeyId_CreateFromByteBuffer(buffer);
 *     parcBuffer_Release(&buffer);
 *
 *     parcKeyId_AssertValid(&instance);
 *     parcKeyId_Release(&instance);
 * }
 * @endcode
 * @see parcKeyId_OptionalAssertValid
 */
void parcKeyId_AssertValid(const PARCKeyId *keyId);

/**
 * Create a copy of the KeyId from the specified `PARCKeyId` instance.
 *
 * A deep copy is performed, acquiring handles to objects when needed.
 *
 * @param [in] original A pointer to a `PARCKeyId` instance containing the pre-computerd-key identifier.
 * @return A pointer to a newly allocated `PARCKeyId` that must be released via `parcKeyId_Release`.
 *
 * Example:
 * @code
 * {
 *      PARCBuffer *keybits = parcBuffer_Wrap("Hello", 5, 0, 5);
 *      PARCKeyId *keyid = parcKeyId_Create(keybits);
 *      parcBuffer_Release(&keybits);
 *      // do something with keyid
 *      parcKeyId_Release(&keyid);
 * }
 * @endcode
 */
PARCKeyId *parcKeyId_Copy(const PARCKeyId *original);

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
 * @param [in,out] object A pointer to a pointer to the `PARCKeyId` instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Wrap("Hello", 5, 0, 5);
 *     PARCKeyId *instance = parcKeyId_CreateFromByteBuffer(buffer);
 *     parcBuffer_Release(&buffer);
 *
 *     parcKeyId_Release(&instance);
 * }
 * @endcode
 */
void parcKeyId_Release(PARCKeyId **object);

/**
 * Determine if two `PARCKeyId` instances are equal.
 *
 * Two `PARCKeyId` instances are equal if, and only if,
 *
 * The following equivalence relations on non-null `PARCKeyId` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `parcKeyId_Equals(x, x)`
 *       must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y,
 *     `parcKeyId_Equals(x, y)` must return true if and only if
 *        `parcKeyId_Equals(y, x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcKeyId_Equals(x, y)` returns true and
 *        `parcKeyId_Equals(y, z)` returns true,
 *        then  `parcKeyId_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple
 *       invocations of `parcKeyId_Equals(x, y)` consistently return true or
 *       consistently return false.
 *
 *   * For any non-null reference value x, `parcKeyId_Equals(x, NULL)` must
 *       return false.
 *
 * @param a A pointer to a `PARCKeyId` instance.
 * @param b A pointer to a `PARCKeyId` instance.
 * @return true if the two `PARCKeyId` instances are equal.
 *
 * Example:
 * @code
 * {
 *     PARCKeyId *a = parcKeyId_Create(...);
 *     PARCKeyId *b = parcKeyId_Create(...);
 *
 *     if (parcKeyId_Equals(a, b)) {
 *         // equal
 *     } else {
 *         // unequal
 *     }
 * }
 * @endcode
 */
bool parcKeyId_Equals(const PARCKeyId *a, const PARCKeyId *b);

/**
 * Get the digest bytes of a `PARCKeyId` instance;
 *
 * @param [in] keyid A pointer to a `PARCKeyId` instance.
 *
 * @return A pointer to a (shared) {@link PARCBuffer} instance containing the digest bytes of the `PARCKeyId`.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *buffer = parcBuffer_Wrap("Hello", 5, 0, 5);
 *     PARCKeyId *instance = parcKeyId_CreateFromByteBuffer(buffer);
 *     PARCBuffer *bytes = parcKeyId_GetKeyId(instance);
 *     // use or display the bytes buffer as needed
 * }
 * @endcode
 */
const PARCBuffer *parcKeyId_GetKeyId(const PARCKeyId *keyid);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `parcKeyId_HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an
 * execution of an application,
 * the {@link parcKeyId_HashCode} function must consistently return the same value,
 * provided no information used in a corresponding {@link parcKeyId_Equals}
 * comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to
 * another execution of the same application.
 * If two instances are equal according to the {@link parcKeyId_Equals} function,
 * then calling the {@link parcKeyId_HashCode} function on each of the two instances must
 * produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcKeyId_Equals} function,
 * then calling the {@link parcKeyId_HashCode}
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] keyid A pointer to the `PARCKeyId` instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCKeyId *x = parcKeyId_Create(...);
 *     uint32_t hashValue = parcKeyId_HashCode(array);
 *     parcKeyId_Release(&x);
 * }
 * @endcode
 */
PARCHashCode parcKeyId_HashCode(const PARCKeyId *keyid);

/**
 * Compute a non-cryptographic hash of a `PARCKeyId` instance from a void pointer.
 *
 * This is useful for use in {@link PARCHashCodeTable}.
 *
 * @param [in] keyId A pointer to the instance to be hashed
 *
 * @return A 32-bit non-cryptographic hash of the instance.
 *
 * Example:
 * @code
 * {
 *     PARCKeyId *x = parcKeyId_Create(...);
 *     uint32_t hash = parcKeyId_HashCode((void*)x);
 *     // use the hash to index into a table, or something else
 * }
 * @endcode
 *
 * @see parcKeyId_HashCode
 */
PARCHashCode parcKeyId_HashCodeFromVoid(const void *keyId);

/**
 * Append a representation of the specified instance to the given
 * {@link PARCBufferComposer}.
 *
 * @param [in] keyId A pointer to a `PARCKeyId` instance.
 * @param [in] composer A pointer to a `PARCBufferComposer` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The given `PARCBufferComposer`.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *
 *     parcKeyId_BuildString(instance, result);
 *
 *     char *string = parcBuffer_ToString(parcBufferComposer_ProduceBuffer(result));
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate((void **)&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcKeyId_BuildString(const PARCKeyId *keyId, PARCBufferComposer *composer);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated,
 *         null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCKeyId *instance = parcKeyId_Create(...);
 *
 *     char *string = parcKeyId_ToString(instance);
 *
 *     printf("Hello: %s\n", string);
 *     parcMemory_Deallocate((void **)&string);
 *
 *     parcKeyId_Release(&instance);
 * }
 * @endcode
 *
 * @see parcKeyId_BuildString
 */
char *parcKeyId_ToString(const PARCKeyId *instance);
#endif // libparc_parc_KeyId_h
