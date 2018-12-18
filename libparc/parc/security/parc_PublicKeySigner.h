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
 * @file parc_PublicKeySigner.h
 * @brief An entity to produce digital signatures based on public/private key stores.
 *
 */
#ifndef PARCLibrary_parc_PublicKeySigner
#define PARCLibrary_parc_PublicKeySigner
#include <stdbool.h>

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_CryptoHash.h>
#include <parc/security/parc_CryptoSuite.h>
#include <parc/security/parc_SigningAlgorithm.h>
#include <parc/security/parc_Signer.h>

struct PARCPublicKeySigner;
typedef struct PARCPublicKeySigner PARCPublicKeySigner;

extern PARCSigningInterface *PARCPublicKeySignerAsSigner;

/**
 * Increase the number of references to a `PARCPublicKeySigner` instance.
 *
 * Note that new `PARCPublicKeySigner` is not created,
 * only that the given `PARCPublicKeySigner` reference count is incremented.
 * Discard the reference by invoking `parcPublicKeySigner_Release`.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     PARCPublicKeySigner *b = parcPublicKeySigner_Acquire();
 *
 *     parcPublicKeySigner_Release(&a);
 *     parcPublicKeySigner_Release(&b);
 * }
 * @endcode
 */
PARCPublicKeySigner *parcPublicKeySigner_Acquire(const PARCPublicKeySigner *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcPublicKeySigner_OptionalAssertValid(_instance_)
#else
#  define parcPublicKeySigner_OptionalAssertValid(_instance_) parcPublicKeySigner_AssertValid(_instance_)
#endif

/**
 * Assert that the given `PARCPublicKeySigner` instance is valid.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     parcPublicKeySigner_AssertValid(a);
 *
 *     printf("Instance is valid.\n");
 *
 *     parcPublicKeySigner_Release(&b);
 * }
 * @endcode
 */
void parcPublicKeySigner_AssertValid(const PARCPublicKeySigner *instance);

/**
 * Create an instance of PARCPublicKeySigner
 *
 * <#Paragraphs Of Explanation#>
 *
 * @return non-NULL A pointer to a valid PARCPublicKeySigner instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     parcPublicKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCPublicKeySigner *parcPublicKeySigner_Create(PARCKeyStore *keyStore, PARCCryptoSuite suite);

/**
 * Compares @p instance with @p other for order.
 *
 * Returns a negative integer, zero, or a positive integer as @p instance
 * is less than, equal to, or greater than @p other.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 * @param [in] other A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return <0 Instance is less than @p other.
 * @return 0 Instance a and instance b compare the same.
 * @return >0 Instance a is greater than instance b.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *     PARCPublicKeySigner *b = parcPublicKeySigner_Create();
 *
 *     if (parcPublicKeySigner_Compare(a, b) == 0) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcPublicKeySigner_Release(&a);
 *     parcPublicKeySigner_Release(&b);
 * }
 * @endcode
 *
 * @see parcPublicKeySigner_Equals
 */
int parcPublicKeySigner_Compare(const PARCPublicKeySigner *instance, const PARCPublicKeySigner *other);

/**
 * Create an independent copy the given `PARCBuffer`
 *
 * A new buffer is created as a complete copy of the original.
 *
 * @param [in] original A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a new `PARCPublicKeySigner` instance.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     PARCPublicKeySigner *copy = parcPublicKeySigner_Copy(&b);
 *
 *     parcPublicKeySigner_Release(&b);
 *     parcPublicKeySigner_Release(&copy);
 * }
 * @endcode
 */
PARCPublicKeySigner *parcPublicKeySigner_Copy(const PARCPublicKeySigner *original);

/**
 * Determine if two `PARCPublicKeySigner` instances are equal.
 *
 * The following equivalence relations on non-null `PARCPublicKeySigner` instances are maintained: *
 *   * It is reflexive: for any non-null reference value x, `parcPublicKeySigner_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `parcPublicKeySigner_Equals(x, y)` must return true if and only if
 *        `parcPublicKeySigner_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `parcPublicKeySigner_Equals(x, y)` returns true and
 *        `parcPublicKeySigner_Equals(y, z)` returns true,
 *        then `parcPublicKeySigner_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `parcPublicKeySigner_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `parcPublicKeySigner_Equals(x, NULL)` must return false.
 *
 * @param [in] x A pointer to a valid PARCPublicKeySigner instance.
 * @param [in] y A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return true The instances x and y are equal.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *     PARCPublicKeySigner *b = parcPublicKeySigner_Create();
 *
 *     if (parcPublicKeySigner_Equals(a, b)) {
 *         printf("Instances are equal.\n");
 *     }
 *
 *     parcPublicKeySigner_Release(&a);
 *     parcPublicKeySigner_Release(&b);
 * }
 * @endcode
 * @see parcPublicKeySigner_HashCode
 */
bool parcPublicKeySigner_Equals(const PARCPublicKeySigner *x, const PARCPublicKeySigner *y);

/**
 * Returns a hash code value for the given instance.
 *
 * The general contract of `HashCode` is:
 *
 * Whenever it is invoked on the same instance more than once during an execution of an application,
 * the `HashCode` function must consistently return the same value,
 * provided no information used in a corresponding comparisons on the instance is modified.
 *
 * This value need not remain consistent from one execution of an application to another execution of the same application.
 * If two instances are equal according to the {@link parcPublicKeySigner_Equals} method,
 * then calling the {@link parcPublicKeySigner_HashCode} method on each of the two instances must produce the same integer result.
 *
 * It is not required that if two instances are unequal according to the
 * {@link parcPublicKeySigner_Equals} function,
 * then calling the `parcPublicKeySigner_HashCode`
 * method on each of the two objects must produce distinct integer results.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return The hashcode for the given instance.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     PARCHashCode hashValue = parcPublicKeySigner_HashCode(buffer);
 *     parcPublicKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCHashCode parcPublicKeySigner_HashCode(const PARCPublicKeySigner *instance);

/**
 * Determine if an instance of `PARCPublicKeySigner` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     if (parcPublicKeySigner_IsValid(a)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcPublicKeySigner_Release(&a);
 * }
 * @endcode
 *
 */
bool parcPublicKeySigner_IsValid(const PARCPublicKeySigner *instance);

/**
 * Release a previously acquired reference to the given `PARCPublicKeySigner` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     parcPublicKeySigner_Release(&a);
 * }
 * @endcode
 */
void parcPublicKeySigner_Release(PARCPublicKeySigner **instancePtr);

/**
 * Create a `PARCJSON` instance (representation) of the given object.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return NULL Memory could not be allocated to contain the `PARCJSON` instance.
 * @return non-NULL An allocated C string that must be deallocated via parcMemory_Deallocate().
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     PARCJSON *json = parcPublicKeySigner_ToJSON(a);
 *
 *     printf("JSON representation: %s\n", parcJSON_ToString(json));
 *     parcJSON_Release(&json);
 *
 *     parcPublicKeySigner_Release(&a);
 * }
 * @endcode
 */
PARCJSON *parcPublicKeySigner_ToJSON(const PARCPublicKeySigner *instance);

/**
 * Produce a null-terminated string representation of the specified `PARCPublicKeySigner`.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to a valid PARCPublicKeySigner instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCPublicKeySigner *a = parcPublicKeySigner_Create();
 *
 *     char *string = parcPublicKeySigner_ToString(a);
 *
 *     parcPublicKeySigner_Release(&a);
 *
 *     parcMemory_Deallocate(&string);
 * }
 * @endcode
 *
 * @see parcPublicKeySigner_Display
 */
char *parcPublicKeySigner_ToString(const PARCPublicKeySigner *instance);
#endif
